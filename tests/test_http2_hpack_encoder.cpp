// HPACK 编码器动态表对拍探针
// 真实编码函数（http2_frame.cpp）+ 独立迷你解码器 + RFC 7541 附录 C 向量。
#include <cstdio>
#include <cstring>
#include <string>
#include <deque>
#include <vector>
#include "http2_frame.h"
#include "http2_huffman.h"

using namespace http;

static int g_fail = 0;
static int g_pass = 0;
#define CHECK(cond, msg)                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (cond)                                                                                                      \
        {                                                                                                              \
            ++g_pass;                                                                                                  \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            ++g_fail;                                                                                                  \
            std::printf("FAIL %s:%d %s\n", __FILE__, __LINE__, msg);                                                    \
        }                                                                                                              \
    } while (0)

static std::string hex(const std::string &s)
{
    std::string r;
    char b[4];
    for (unsigned char c : s)
    {
        std::snprintf(b, sizeof(b), "%02x ", c);
        r += b;
    }
    return r;
}

// ---- 独立迷你 HPACK 解码器（与生产解码器无关，仅用于对拍）----
struct MiniDec
{
    std::deque<std::pair<std::string, std::string>> dyn;// front = 最新
    unsigned int size     = 0;
    unsigned int max_size = 4096;
    std::vector<std::pair<std::string, std::string>> out;

    static unsigned int rd_int(const std::string &b, size_t &p, unsigned int prefix)
    {
        unsigned int mask = (1u << prefix) - 1;
        unsigned int v    = (unsigned char)b[p++] & mask;
        if (v < mask)
            return v;
        unsigned int m = 0;
        while (p < b.size())
        {
            unsigned char c = (unsigned char)b[p++];
            v += (c & 0x7F) << m;
            m += 7;
            if (!(c & 0x80))
                break;
        }
        return v;
    }
    std::string rd_str(const std::string &b, size_t &p)
    {
        bool huff       = ((unsigned char)b[p] & 0x80) != 0;
        unsigned int len = rd_int(b, p, 7);
        std::string raw = b.substr(p, len);
        p += len;
        if (!huff)
            return raw;
        std::string dec;
        unsigned char state = 0, ending = 0;
        http_huffman_decode(&state, (unsigned char *)raw.data(), raw.size(), dec, ending);
        return dec;
    }
    void evict()
    {
        while (!dyn.empty() && size > max_size)
        {
            size -= (unsigned int)(dyn.back().first.size() + dyn.back().second.size()) + 32;
            dyn.pop_back();
        }
    }
    void insert(const std::string &n, const std::string &v)
    {
        dyn.emplace_front(n, v);
        size += (unsigned int)(n.size() + v.size()) + 32;
        evict();
    }
    bool lookup(unsigned int ix, std::string &n, std::string &v)
    {
        if (ix >= 1 && ix <= 61)
        {
            n = http2_header_static_table[ix].key;
            v = http2_header_static_table[ix].value;
            return true;
        }
        unsigned int d = ix - 62;
        if (d >= dyn.size())
            return false;
        n = dyn[d].first;
        v = dyn[d].second;
        return true;
    }
    bool block(const std::string &b)
    {
        out.clear();
        size_t p = 0;
        while (p < b.size())
        {
            unsigned char c = (unsigned char)b[p];
            if (c & 0x80)
            {
                unsigned int ix = rd_int(b, p, 7);
                std::string n, v;
                if (ix == 0 || !lookup(ix, n, v))
                    return false;
                out.emplace_back(n, v);
            }
            else if ((c & 0xC0) == 0x40)
            {
                unsigned int ni = rd_int(b, p, 6);
                std::string n, v;
                if (ni == 0)
                    n = rd_str(b, p);
                else if (!lookup(ni, n, v))
                    return false;
                v = rd_str(b, p);
                insert(n, v);
                out.emplace_back(n, v);
            }
            else if ((c & 0xE0) == 0x20)
            {
                max_size = rd_int(b, p, 5);
                evict();
            }
            else
            {
                unsigned int ni = rd_int(b, p, 4);
                std::string n, v;
                if (ni == 0)
                    n = rd_str(b, p);
                else if (!lookup(ni, n, v))
                    return false;
                v = rd_str(b, p);
                out.emplace_back(n, v);
            }
        }
        return true;
    }
};

static bool dec_eq(MiniDec &d, const std::vector<std::pair<std::string, std::string>> &want)
{
    return d.out == want;
}

// 模拟 make_http2_header 的头块前导：pending Size Update 在块头发射（生产代码在
// httppeer.cpp 那一层做，item3 本身不发）。缩了又抬时发两个：先最紧值再当前值。
static void emit_pending_update(hpack_enc_table &enc, std::string &b)
{
    if (!enc.size_update_pending)
        return;
    auto put = [&](unsigned int v)
    {
        if (v < 31)
        {
            b.push_back(static_cast<char>(0x20 | v));
        }
        else
        {
            b.push_back(0x3F);
            v -= 31;
            while (v >= 128)
            {
                b.push_back(static_cast<char>((v & 0x7F) | 0x80));
                v >>= 7;
            }
            b.push_back(static_cast<char>(v));
        }
    };
    put(enc.pending_size_update);
    if (enc.max_size != enc.pending_size_update)
        put(enc.max_size);
    enc.size_update_pending = false;
}

int main()
{
    // ---- T1: 字节级向量（enc=nullptr 的 without-indexing + Huffman）----
    // 期望值取自 curl/nghttp2 实抓 HEADERS（curl_oracle.py），不是凭记忆的 RFC 附录。
    {
        std::string b;
        make_http2_headers_item3(b, std::string("custom-key"), std::string("custom-header"), nullptr);
        std::string want;
        want.push_back(0x00);
        want.push_back((char)0x88);
        want.append("\x25\xa8\x49\xe9\x5b\xa9\x7d\x7f", 8);// huffman("custom-key")
        want.push_back((char)0x89);
        want.append("\x25\xa8\x49\xe9\x5a\x72\x8e\x42\xd9", 9);// huffman("custom-header")
        CHECK(b == want, ("T1 bytes: " + hex(b)).c_str());
    }

    // ---- T2: RFC 7541 C.3 请求序列形状（响应侧同构）：首请求字面量插表，次请求 Indexed ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::vector<std::pair<std::string, std::string>> fields = {
            {":status", "200"}, {"content-length", "13"}, {"content-type", "text/plain"}, {"server", "Paozhu"}};
        for (int round = 0; round < 3; ++round)
        {
            std::string b;
            make_http2_headers_static(b, 200);
            make_http2_headers_item3(b, HTTP2_CODE_content_length, std::string("13"), &enc);
            make_http2_headers_item3(b, HTTP2_CODE_content_type, std::string("text/plain"), &enc);
            make_http2_headers_item3(b, std::string("server"), std::string("Paozhu"), &enc);
            CHECK(dec.block(b), "T2 decode ok");
            CHECK(dec_eq(dec, fields), "T2 fields match");
            if (round == 0)
            {
                CHECK(enc.entries.size() == 3, "T2 first round inserts 3");
                CHECK(b.size() > 10, "T2 first round literals");
            }
            else
            {
                // :status 0x88 + 3 个单字节 Indexed（索引 62..64 < 128）
                CHECK(b.size() == 4, ("T2 indexed round size=4 got " + std::to_string(b.size())).c_str());
                CHECK((unsigned char)b[0] == 0x88, "T2 status byte");
                for (int i = 1; i < 4; ++i)
                    CHECK(((unsigned char)b[i] & 0x80) != 0, "T2 indexed bytes");
            }
        }
        // 索引相对性：再插一条，旧条目索引全体 +1，引用仍要解对
        std::string b;
        make_http2_headers_item3(b, std::string("x-new"), std::string("v"), &enc);
        CHECK(dec.block(b), "T2 insert decode");
        std::string b2;
        make_http2_headers_item3(b2, std::string("server"), std::string("Paozhu"), &enc);
        CHECK(dec.block(b2), "T2 shifted decode");
        CHECK(dec_eq(dec, {{"server", "Paozhu"}}), "T2 shifted index correct");
        // 插入序 CL→CT→server→x-new：server 在距队头 1 位 → 索引 63
        CHECK(b2.size() == 1 && (unsigned char)b2[0] == (0x80 | 63), "T2 server now index 63");
    }

    // ---- T3: name-only 匹配（同 name 不同 value 用 name 索引 + 插表）----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string b1;
        make_http2_headers_item3(b1, HTTP2_CODE_content_type, std::string("text/plain"), &enc);
        CHECK(dec.block(b1), "T3 b1 decode");
        std::string b2;
        make_http2_headers_item3(b2, HTTP2_CODE_content_type, std::string("text/html"), &enc);
        CHECK(dec.block(b2), "T3 b2 decode");
        CHECK(dec_eq(dec, {{"content-type", "text/html"}}), "T3 name-only value");
        CHECK((unsigned char)b2[0] == (0x40 | 31), "T3 name index 31 in 6-bit prefix");
        CHECK(enc.entries.size() == 2, "T3 two entries");
    }

    // ---- T4: 索引 >=128 的多字节 Indexed（对端广告大表）----
    {
        hpack_enc_table enc;
        enc.set_max_size(65536);
        MiniDec dec;
        dec.max_size = 65536;
        for (int i = 0; i < 100; ++i)
        {
            std::string b;
            make_http2_headers_item3(b, std::string("x-k" + std::to_string(i)), std::string("v"), &enc);
            CHECK(dec.block(b), "T4 insert decode");
        }
        std::string b;
        make_http2_headers_item3(b, std::string("x-k0"), std::string("v"), &enc);// 最老，索引 161
        // 7-bit 前缀逃逸：0xFF + varint(161-127=34)，34<128 只占 1 字节
        CHECK(b.size() == 2, ("T4 multibyte size " + std::to_string(b.size())).c_str());
        CHECK((unsigned char)b[0] == 0xFF && (unsigned char)b[1] == 34, "T4 varint 161-127=34");
        CHECK(dec.block(b), "T4 multibyte decode");
        CHECK(dec_eq(dec, {{"x-k0", "v"}}), "T4 oldest entry value");
    }

    // ---- T5: SETTINGS 缩表 → 下一头块开头 Size Update（RFC 7541 §4.2）----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string b1;
        make_http2_headers_item3(b1, std::string("a"), std::string("1"), &enc);
        make_http2_headers_item3(b1, std::string("b"), std::string("2"), &enc);
        CHECK(dec.block(b1), "T5 b1 decode");
        enc.set_max_size(60);// 只容得下一条（33 字节/条）
        CHECK(enc.size_update_pending, "T5 pending flag");
        CHECK(enc.entries.size() == 1, "T5 evicted to 1");
        std::string b2;
        emit_pending_update(enc, b2);
        make_http2_headers_item3(b2, std::string("a"), std::string("1"), &enc);
        CHECK((unsigned char)b2[0] == 0x3F && (unsigned char)b2[1] == 29, "T5 size update escape 60-31=29");
        CHECK(!enc.size_update_pending, "T5 flag consumed");
        CHECK(dec.block(b2), "T5 b2 decode");
        CHECK(dec.max_size == 60, "T5 decoder max updated");
        CHECK(dec_eq(dec, {{"a", "1"}}), "T5 b2 fields");
        // 再缩到 0：信号 0x20，随后字面量不插表
        enc.set_max_size(0);
        std::string b3;
        emit_pending_update(enc, b3);
        make_http2_headers_item3(b3, std::string("a"), std::string("1"), &enc);
        CHECK((unsigned char)b3[0] == 0x20, "T5 zero update");
        CHECK(dec.block(b3), "T5 b3 decode");
        CHECK(dec.dyn.empty(), "T5 decoder table empty");
        CHECK(enc.entries.empty(), "T5 encoder table empty");
        std::string b4;
        make_http2_headers_item3(b4, std::string("a"), std::string("1"), &enc);
        CHECK((unsigned char)b4[0] == 0x00, "T5 max0 degrades to without-indexing");
        CHECK(dec.block(b4), "T5 b4 decode");
    }

    // ---- T6: 超大条目无条件入表 → 逐空全表（与解码侧口径一致）----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string b1;
        make_http2_headers_item3(b1, std::string("a"), std::string("1"), &enc);
        CHECK(dec.block(b1), "T6 b1 decode");
        std::string big(5000, 'x');
        std::string b2;
        make_http2_headers_item3(b2, std::string("big"), big, &enc);
        CHECK(dec.block(b2), "T6 big decode");
        CHECK(dec_eq(dec, {{"big", big}}), "T6 big value");
        CHECK(enc.entries.empty() && enc.size == 0, "T6 encoder emptied");
        CHECK(dec.dyn.empty() && dec.size == 0, "T6 decoder emptied");
        std::string b3;
        make_http2_headers_item3(b3, std::string("a"), std::string("1"), &enc);
        CHECK(dec.block(b3), "T6 after-empty decode");
        CHECK(dec_eq(dec, {{"a", "1"}}), "T6 reinsert");
    }

    // ---- T7: 静态表全匹配直接 Indexed，不插动态表 ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string b;
        make_http2_headers_item3(b, std::string("accept-encoding"), std::string("gzip, deflate"), &enc);
        CHECK(b.size() == 1 && (unsigned char)b[0] == (0x80 | 16), "T7 static full match index 16");
        CHECK(enc.entries.empty(), "T7 no insert");
        CHECK(dec.block(b), "T7 decode");
    }

    // ---- T8: 不插表块（流式路径 enc=nullptr）与插表块交错，两端表不失步 ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string b1;
        make_http2_headers_item3(b1, std::string("a"), std::string("1"), &enc);
        CHECK(dec.block(b1), "T8 b1");
        std::string b2;
        make_http2_headers_item3(b2, std::string("a"), std::string("1"), nullptr);
        make_http2_headers_item3(b2, std::string("stream"), std::string("yes"), nullptr);
        CHECK(dec.block(b2), "T8 neutral block decode");
        CHECK(dec_eq(dec, {{"a", "1"}, {"stream", "yes"}}), "T8 neutral fields");
        CHECK(dec.dyn.size() == 1, "T8 decoder table untouched");
        std::string b3;
        make_http2_headers_item3(b3, std::string("a"), std::string("1"), &enc);
        CHECK(b3.size() == 1 && (unsigned char)b3[0] == (0x80 | 62), "T8 indexed after neutral");
        CHECK(dec.block(b3), "T8 b3 decode");
    }

    // ---- T9: never-indexed（set-cookie 形状）不插表 ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string b1;
        make_http2_headers_item3(b1, std::string("a"), std::string("1"), &enc);
        CHECK(dec.block(b1), "T9 b1");
        std::string b2;
        make_http2_headers_item4(b2, HTTP2_CODE_set_cookie, std::string("sid=abc123"));
        // never-indexed 是 4-bit 前缀：索引 55 ≥ 16 → 0x1F + varint(55-15=40)
        CHECK((unsigned char)b2[0] == 0x1F && (unsigned char)b2[1] == 40, "T9 never-indexed prefix escape");
        CHECK(dec.block(b2), "T9 cookie decode");
        CHECK(dec_eq(dec, {{"set-cookie", "sid=abc123"}}), "T9 cookie value");
        CHECK(dec.dyn.size() == 1 && enc.entries.size() == 1, "T9 tables untouched");
    }

    // ---- T10: 长 value（>127 字节 Huffman 后）长度 varint 完整 ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string val(400, 'a');
        std::string b;
        make_http2_headers_item3(b, std::string("x-long"), val, &enc);
        CHECK(dec.block(b), "T10 long decode");
        CHECK(dec_eq(dec, {{"x-long", val}}), "T10 long value roundtrip");
        std::string b2;
        make_http2_headers_item4(b2, HTTP2_CODE_set_cookie, val);
        CHECK(dec.block(b2), "T10 long cookie decode");
        CHECK(dec_eq(dec, {{"set-cookie", val}}), "T10 long cookie roundtrip");
    }

    // ---- T11: 逐出 churn：300 条不同值持续逐出 + 80 次重复命中，记账不变式逐步核对 ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        auto invariant = [&](const char *tag)
        {
            unsigned int sum = 0;
            for (auto &e : enc.entries)
                sum += (unsigned int)(e.first.size() + e.second.size()) + 32;
            CHECK(enc.size == sum, tag);
            CHECK(enc.size <= enc.max_size, tag);
            CHECK(enc.entries == dec.dyn, tag);
        };
        for (int i = 0; i < 300; ++i)
        {
            std::string b;
            make_http2_headers_item3(b, std::string("h") + std::to_string(i % 40),
                                     std::to_string(i) + std::string(i % 7, 'v'), &enc);
            CHECK(dec.block(b), "T11 churn decode");
            invariant("T11 churn invariant");
        }
        CHECK(enc.entries.size() < 300, "T11 eviction happened");
        for (int i = 0; i < 80; ++i)
        {
            std::string name = "h" + std::to_string(i % 40);
            std::string val  = std::to_string(i % 40);
            std::string b;
            make_http2_headers_item3(b, name, val, &enc);
            CHECK(dec.block(b), "T11 hit decode");
            CHECK(dec_eq(dec, {{name, val}}), "T11 hit fields");
            if (i >= 40)
                CHECK(b.size() == 1, "T11 warm hit is 1-byte indexed");
            invariant("T11 hit invariant");
        }
    }

    // ---- T12: 条目尺寸边界：恰好等于上限 / 超 1 字节 / 带旧条目时超大入表 ----
    {
        hpack_enc_table enc;
        enc.set_max_size(100);
        MiniDec dec;
        dec.max_size = 100;
        std::string v67(67, 'x');// 1 + 67 + 32 = 100，恰好装下
        std::string b;
        make_http2_headers_item3(b, std::string("k"), v67, &enc);
        CHECK(dec.block(b), "T12 exact decode");
        CHECK(enc.entries.size() == 1 && enc.size == 100, "T12 exact fit");
        std::string v68(68, 'x');// 101 > 100 → 整表逐空
        std::string b2;
        make_http2_headers_item3(b2, std::string("k"), v68, &enc);
        CHECK(dec.block(b2), "T12 over decode");
        CHECK(dec_eq(dec, {{"k", v68}}), "T12 over field delivered");
        CHECK(enc.entries.empty() && enc.size == 0, "T12 over empties");
        CHECK(dec.dyn.empty() && dec.size == 0, "T12 decoder emptied");
        std::string b3;
        make_http2_headers_item3(b3, std::string("a"), std::string("1"), &enc);// 34B 先入
        make_http2_headers_item3(b3, std::string("k"), v68, &enc);              // 超大随后
        CHECK(dec.block(b3), "T12 mixed decode");
        CHECK(enc.entries.empty(), "T12 oversized evicts prior too");
    }

    // ---- T13: 索引边界 127 / 128 / 16446（7-bit 前缀逃逸与多字节 varint）----
    {
        hpack_enc_table enc;
        MiniDec dec;
        for (int i = 0; i < 66; ++i)
        {
            std::string b;
            make_http2_headers_item3(b, std::string("k") + std::to_string(i), std::string("v"), &enc);
            CHECK(dec.block(b), "T13 fill decode");
        }
        std::string b127;
        make_http2_headers_item3(b127, std::string("k0"), std::string("v"), &enc);// 62+65=127
        CHECK(b127.size() == 2 && (unsigned char)b127[0] == 0xFF && (unsigned char)b127[1] == 0,
              "T13 index 127 = FF 00");
        CHECK(dec.block(b127), "T13 127 decode");
        std::string bx;
        make_http2_headers_item3(bx, std::string("k66"), std::string("v"), &enc);
        CHECK(dec.block(bx), "T13 k66 decode");
        std::string b128;
        make_http2_headers_item3(b128, std::string("k0"), std::string("v"), &enc);// 128
        CHECK(b128.size() == 2 && (unsigned char)b128[0] == 0xFF && (unsigned char)b128[1] == 1,
              "T13 index 128 = FF 01");
        CHECK(dec.block(b128), "T13 128 decode");
        enc.set_max_size(1u << 20);
        dec.max_size = 1u << 20;
        for (int i = 67; i < 16385; ++i)
        {
            std::string b;
            make_http2_headers_item3(b, std::string("k") + std::to_string(i), std::string("v"), &enc);
            if (!dec.block(b))
            {
                CHECK(false, "T13 big fill decode");
                break;
            }
        }
        std::string bbig;
        make_http2_headers_item3(bbig, std::string("k0"), std::string("v"), &enc);// 62+16384=16446
        CHECK(bbig.size() == 3 && (unsigned char)bbig[0] == 0xFF && (unsigned char)bbig[1] == 0xBF &&
                  (unsigned char)bbig[2] == 0x7F,
              "T13 index 16446 = FF BF 7F");
        CHECK(dec.block(bbig), "T13 big decode");
        CHECK(dec_eq(dec, {{"k0", "v"}}), "T13 oldest still correct");
    }

    // ---- T14: 动态 name-only 匹配索引 ≥64（6-bit 前缀 0x7F 逃逸）----
    {
        hpack_enc_table enc;
        MiniDec dec;
        for (int i = 0; i < 5; ++i)
        {
            std::string b;
            make_http2_headers_item3(b, std::string("n") + std::to_string(i), std::string("v"), &enc);
            CHECK(dec.block(b), "T14 fill decode");
        }
        std::string b;
        make_http2_headers_item3(b, std::string("n0"), std::string("new"), &enc);// n0 最老 → 66
        CHECK((unsigned char)b[0] == 0x7F && (unsigned char)b[1] == 3, "T14 name index 66 = 7F 03");
        CHECK(dec.block(b), "T14 decode");
        CHECK(dec_eq(dec, {{"n0", "new"}}), "T14 name-only new value");
        CHECK(enc.entries.size() == 6, "T14 inserted");
    }

    // ---- T15: 重复条目：find 取最新；逐出一条后另一条仍可引用 ----
    // 编码器对同 name+value 第二次编码会命中 Indexed、不产生重复，所以第二条
    // 直接 insert + 手拼 literal-inc 块喂解码器，模拟对端/手工制造重复的场景。
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string b;
        make_http2_headers_item3(b, std::string("d"), std::string("x"), &enc);
        CHECK(dec.block(b), "T15 first insert decode");
        auto putstr = [&](std::string &out, const std::string &s)
        {
            std::string en;
            http_huffman_encode((unsigned char *)&s[0], (unsigned int)s.size(), en);
            out.push_back(static_cast<char>(0x80 | en.size()));
            out.append(en);
        };
        std::string bdup;
        bdup.push_back(0x40);
        putstr(bdup, "d");
        putstr(bdup, "x");
        enc.insert("d", "x");
        CHECK(dec.block(bdup), "T15 dup insert decode");
        CHECK(enc.entries.size() == 2 && dec.dyn.size() == 2, "T15 duplicates kept");
        b.clear();
        make_http2_headers_item3(b, std::string("d"), std::string("x"), &enc);
        CHECK(b.size() == 1 && (unsigned char)b[0] == (0x80 | 62), "T15 newest index 62");
        CHECK(dec.block(b), "T15 dup ref decode");
        enc.set_max_size(34);// 1+1+32=34，只容一条 → 逐掉老的重复
        CHECK(enc.entries.size() == 1, "T15 one evicted");
        std::string b2;
        emit_pending_update(enc, b2);// 34-31=3 → 3F 03，随后 Indexed 62
        make_http2_headers_item3(b2, std::string("d"), std::string("x"), &enc);
        CHECK(b2.size() == 3 && (unsigned char)b2[0] == 0x3F && (unsigned char)b2[1] == 0x03 &&
                  (unsigned char)b2[2] == (0x80 | 62),
              "T15 survivor signal + index 62");
        CHECK(dec.block(b2), "T15 survivor decode");
        CHECK(dec.dyn.size() == 1, "T15 decoder mirrored eviction");
    }

    // ---- T16: 多次缩表合并成一个信号；抬高也发信号 ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        enc.set_max_size(2000);
        enc.set_max_size(500);
        CHECK(enc.size_update_pending && enc.pending_size_update == 500, "T16 last shrink wins");
        std::string b;
        emit_pending_update(enc, b);
        make_http2_headers_item3(b, std::string("a"), std::string("1"), &enc);
        CHECK((unsigned char)b[0] == 0x3F && (unsigned char)b[1] == 0xD5 && (unsigned char)b[2] == 0x03,
              "T16 signal 500 = 3F D5 03");
        CHECK(dec.block(b), "T16 decode");
        CHECK(dec.max_size == 500, "T16 decoder 500");
        enc.set_max_size(8000);
        // 500 的信号已随上一块发出；这次是纯抬高，只发 8000
        CHECK(enc.size_update_pending && enc.pending_size_update == 8000, "T16 grow after emitted signal");
        std::string b2;
        emit_pending_update(enc, b2);
        make_http2_headers_item3(b2, std::string("a"), std::string("1"), &enc);
        // 8000-31=7969=0x1F21 → 3F A1 3E，随后 Indexed 62
        CHECK(b2.size() == 4 && (unsigned char)b2[0] == 0x3F && (unsigned char)b2[1] == 0xA1 &&
                  (unsigned char)b2[2] == 0x3E && (unsigned char)b2[3] == (0x80 | 62),
              "T16 grow emits single update");
        CHECK(dec.block(b2), "T16 b2 decode");
        CHECK(dec.max_size == 8000, "T16 decoder follows grow");
        CHECK(dec_eq(dec, {{"a", "1"}}), "T16 b2 fields");
        hpack_enc_table enc2;
        enc2.set_max_size(65536);
        CHECK(enc2.size_update_pending, "T16 fresh grow signals too");
        // 缩了又抬、中间没有头块：最紧值 500 的逐出必须镜像 → 两个信号都发
        hpack_enc_table enc3;
        MiniDec dec3;
        enc3.set_max_size(2000);
        enc3.set_max_size(500);
        enc3.set_max_size(8000);
        CHECK(enc3.pending_size_update == 500, "T16 tightest kept across coalesced changes");
        std::string b3;
        emit_pending_update(enc3, b3);
        make_http2_headers_item3(b3, std::string("a"), std::string("1"), &enc3);
        CHECK(b3.size() == 11 && (unsigned char)b3[0] == 0x3F && (unsigned char)b3[1] == 0xD5 &&
                  (unsigned char)b3[2] == 0x03 && (unsigned char)b3[3] == 0x3F &&
                  (unsigned char)b3[4] == 0xA1 && (unsigned char)b3[5] == 0x3E &&
                  (unsigned char)b3[6] == 0x40,
              "T16 coalesced shrink+grow emits both updates");
        CHECK(dec3.block(b3), "T16 b3 decode");
        CHECK(dec3.max_size == 8000, "T16 b3 decoder 8000");
        CHECK(dec_eq(dec3, {{"a", "1"}}), "T16 b3 fields");
    }

    // ---- T17: Size Update 多字节 varint（65536 → 1000）----
    {
        hpack_enc_table enc;
        enc.set_max_size(65536);
        enc.set_max_size(1000);
        MiniDec dec;
        dec.max_size = 65536;
        std::string b;
        emit_pending_update(enc, b);
        make_http2_headers_item3(b, std::string("a"), std::string("1"), &enc);
        CHECK((unsigned char)b[0] == 0x3F && (unsigned char)b[1] == 0xC9 && (unsigned char)b[2] == 0x07,
              "T17 signal 1000 = 3F C9 07");
        CHECK(dec.block(b), "T17 decode");
        CHECK(dec.max_size == 1000, "T17 decoder 1000");
    }

    // ---- T18: 缩表但无需逐出也要发信号 ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        std::string b;
        make_http2_headers_item3(b, std::string("a"), std::string("1"), &enc);// 34B
        CHECK(dec.block(b), "T18 b1 decode");
        enc.set_max_size(34);
        CHECK(enc.size_update_pending && enc.entries.size() == 1, "T18 pending, nothing evicted");
        std::string b2;
        emit_pending_update(enc, b2);
        make_http2_headers_item3(b2, std::string("a"), std::string("1"), &enc);
        CHECK((unsigned char)b2[0] == 0x3F && (unsigned char)b2[1] == 0x03, "T18 signal 34 = 3F 03");
        CHECK(dec.block(b2), "T18 b2 decode");
        CHECK(dec.max_size == 34 && dec.dyn.size() == 1, "T18 decoder mirrored");
    }

    // ---- T19: 超 max_frame_size 的头块切 CONTINUATION，表状态与信号不受切分影响 ----
    {
        hpack_enc_table enc;
        enc.set_max_size(100);
        MiniDec dec;
        std::string block;
        emit_pending_update(enc, block);// 3F 45（100-31=69）
        std::string bigval(30000, 'a');
        make_http2_headers_item3(block, std::string("big"), bigval, &enc);
        CHECK(enc.entries.empty(), "T19 oversized emptied encoder");
        std::string frame;
        frame.resize(9);
        frame[3] = 0x01;
        frame[4] = 0x04;
        frame[8] = 0x01;
        frame.append(block);
        set_http2_headers_size(frame, (unsigned int)block.size());
        http2_split_headers(frame, 16384);
        // 走帧链：首帧 HEADERS 不带 END_HEADERS，末帧 CONTINUATION 带；载荷拼回 == 原块
        std::string reasm;
        size_t p = 0;
        int nframes = 0;
        bool last_end_headers = false;
        while (p + 9 <= frame.size())
        {
            unsigned int ln = ((unsigned char)frame[p] << 16) | ((unsigned char)frame[p + 1] << 8) |
                              (unsigned char)frame[p + 2];
            unsigned char type = frame[p + 3], flags = frame[p + 4];
            if (nframes == 0)
            {
                CHECK(type == 0x01 && !(flags & 0x04), "T19 first frame HEADERS without END_HEADERS");
            }
            else
            {
                CHECK(type == 0x09, "T19 continuation type");
            }
            last_end_headers = (flags & 0x04) != 0;
            reasm.append(frame, p + 9, ln);
            p += 9 + ln;
            ++nframes;
        }
        CHECK(nframes >= 2, "T19 split happened");
        CHECK(last_end_headers, "T19 END_HEADERS on last");
        CHECK(reasm == block, "T19 reassembly byte-identical");
        CHECK(dec.block(reasm), "T19 decode across continuation");
        CHECK(dec_eq(dec, {{"big", bigval}}), "T19 big field intact");
        CHECK(dec.max_size == 100 && dec.dyn.empty(), "T19 decoder state mirrored");
    }

    // ---- T20: 1000 步伪随机操作（插入/缩/抬/不插表块），编码器解码器逐条镜像 ----
    {
        hpack_enc_table enc;
        MiniDec dec;
        unsigned int seed = 12345;
        auto rnd = [&]()
        {
            seed = seed * 1103515245u + 12345u;
            return (seed >> 8) & 0x7FFFFFFF;
        };
        bool ok = true;
        for (int step = 0; step < 1000 && ok; ++step)
        {
            unsigned int op = rnd() % 6;
            if (op <= 2)
            {
                std::string name = "n" + std::to_string(rnd() % 20);
                std::string val  = std::to_string(rnd() % 50) + std::string(rnd() % 300, 'x');
                std::string b;
                emit_pending_update(enc, b);
                make_http2_headers_item3(b, name, val, &enc);
                if (!dec.block(b) || !dec_eq(dec, {{name, val}}))
                    ok = false;
            }
            else if (op == 3)
            {
                enc.set_max_size(rnd() % 8192);
            }
            else if (op == 4)
            {
                enc.set_max_size(rnd() % 65536);
            }
            else
            {
                std::string b;
                make_http2_headers_item3(b, std::string("s") + std::to_string(rnd() % 10),
                                         std::string(rnd() % 100, 'y'), nullptr);
                if (!dec.block(b))
                    ok = false;
            }
            unsigned int sum = 0;
            for (auto &e : enc.entries)
                sum += (unsigned int)(e.first.size() + e.second.size()) + 32;
            if (enc.size != sum || enc.size > enc.max_size)
            {
                std::printf("T20 step %d op %u accounting: size=%u sum=%u max=%u\n", step, op, enc.size, sum,
                            enc.max_size);
                ok = false;
            }
            // 解码器上限只跟信号走：缩/抬之后、信号到达之前两表合法地不同；
            // 只在表块（op 0-2，信号已随块发出）之后逐条镜像比对。
            if (op <= 2 && (enc.entries != dec.dyn || dec.max_size != enc.max_size))
            {
                std::printf("T20 step %d op %u mirror: enc_n=%zu dec_n=%zu enc_max=%u dec_max=%u pending=%d\n",
                            step, op, enc.entries.size(), dec.dyn.size(), enc.max_size, dec.max_size,
                            (int)enc.size_update_pending);
                ok = false;
            }
        }
        CHECK(ok, "T20 1000-step random mirror");
    }

    // ---- T21: :status 状态行在两档位下都不碰动态表（302/403/429/503 回归）----
    // 线上事故形状：同一条 h2 连接先用几个 200 把动态表填热，再来一个 403。状态行若用
    // 0x48（Literal with Incremental Indexing），对端插一条而本端 hpack_enc 不插，两端表长
    // 从此错开一格，同一块里后面的 Indexed 引用解到别的字段（多一个 :status、少一个
    // content-type），Firefox 报 NS_ERROR_CORRUPTED_CONTENT、curl 报 CURLE_HTTP2。
    for (unsigned int code : {302u, 403u, 429u, 503u})
    {
        const std::string sval = std::to_string(code);
        // 档位甲：PAOZHU_HPACK_ENCODER_DYNAMIC_TABLE 开着，响应头字段走 enc
        {
            hpack_enc_table enc;
            MiniDec dec;
            for (int i = 0; i < 3; ++i)
            {
                std::string warm;
                make_http2_headers_static(warm, 200);
                make_http2_headers_item3(warm, HTTP2_CODE_content_length, std::string("84"), &enc);
                make_http2_headers_item3(warm, HTTP2_CODE_content_type, std::string("text/html; charset=utf-8"), &enc);
                make_http2_headers_item3(warm, std::string("server"), std::string("Paozhu"), &enc);
                CHECK(dec.block(warm), "T21a warm decode");
            }
            CHECK(enc.entries.size() == 3 && dec.dyn.size() == 3, "T21a warm tables in step");
            std::string b;
            make_http2_headers_static(b, code);
            CHECK((unsigned char)b[0] == 0x08 && b.size() == 5, ("T21a status bytes " + hex(b)).c_str());
            make_http2_headers_item3(b, HTTP2_CODE_content_length, std::string("84"), &enc);
            make_http2_headers_item3(b, HTTP2_CODE_content_type, std::string("text/html; charset=utf-8"), &enc);
            make_http2_headers_item3(b, std::string("server"), std::string("Paozhu"), &enc);
            // 三个字段此时全走 Indexed——正是被错位毁掉的那条路
            CHECK(dec.block(b), "T21a decode after warm table");
            CHECK(dec_eq(dec, {
                      {":status", sval},
                      {"content-length", "84"},
                      {"content-type", "text/html; charset=utf-8"},
                      {"server", "Paozhu"},
                  }),
                  ("T21a " + sval + " field set").c_str());
            CHECK(enc.entries == dec.dyn, "T21a tables still in step");
        }
        // 档位乙：宏关掉或流式路径（enc == nullptr），整块两端都不许插表
        {
            hpack_enc_table enc;
            MiniDec dec;
            std::string b;
            make_http2_headers_static(b, code);
            make_http2_headers_item3(b, HTTP2_CODE_content_length, std::string("84"), nullptr);
            make_http2_headers_item3(b, HTTP2_CODE_content_type, std::string("text/html; charset=utf-8"), nullptr);
            make_http2_headers_item3(b, std::string("server"), std::string("Paozhu"), nullptr);
            CHECK(dec.block(b), "T21b decode");
            CHECK(dec_eq(dec, {
                      {":status", sval},
                      {"content-length", "84"},
                      {"content-type", "text/html; charset=utf-8"},
                      {"server", "Paozhu"},
                  }),
                  ("T21b " + sval + " field set").c_str());
            CHECK(dec.dyn.empty() && enc.entries.empty(), "T21b neither side touches the table");
        }
    }

    // ---- T22: 旧编码件 item/item2 只发 without-indexing，不碰两端动态表 ----
    // 这两个重载的签名里没有 hpack_enc_table，改前却发 0x40（Incremental Indexing），错位成因
    // 与 T21 同一条。实现按用户要求保留作演变参考，故用本用例把表示形式钉住。
    {
        MiniDec dec;
        std::string b;
        make_http2_headers_item(b, (unsigned char)HTTP2_CODE_content_type, std::string("text/html"));
        // 槽号 31 > 4-bit 前缀 cap 15 → 两字节 varint 0x0F 0x10，随后 Huffman 值
        CHECK((unsigned char)b[0] == 0x0F && (unsigned char)b[1] == 0x10, ("T22 item bytes " + hex(b)).c_str());
        CHECK(dec.block(b), "T22 item decode");
        CHECK(dec_eq(dec, {{"content-type", "text/html"}}), "T22 item field");
        CHECK(dec.dyn.empty(), "T22 item inserts nothing");
    }
    {
        MiniDec dec;
        std::string b;
        make_http2_headers_item2(b, std::string("x-legacy"), std::string("1"));
        CHECK((unsigned char)b[0] == 0x00, ("T22 item2 name-literal byte " + hex(b)).c_str());
        CHECK(dec.block(b) && dec_eq(dec, {{"x-legacy", "1"}}), "T22 item2 name-literal field");
        CHECK(dec.dyn.empty(), "T22 item2 inserts nothing");
    }
    {
        MiniDec dec;
        std::string b;
        make_http2_headers_item2(b, (unsigned char)HTTP2_CODE_content_length, (unsigned long long)84);
        CHECK((unsigned char)b[0] == 0x0F && (unsigned char)b[1] == 0x0D, ("T22 item2 num bytes " + hex(b)).c_str());
        CHECK(dec.block(b) && dec_eq(dec, {{"content-length", "84"}}), "T22 item2 num field");
        CHECK(dec.dyn.empty(), "T22 item2 num inserts nothing");
    }
    {
        // 与热表交错：旧件插在两个 Indexed 引用之间，紧随其后的引用仍必须解对
        hpack_enc_table enc;
        MiniDec dec;
        for (int i = 0; i < 2; ++i)
        {
            std::string warm;
            make_http2_headers_item3(warm, std::string("server"), std::string("Paozhu"), &enc);
            CHECK(dec.block(warm), "T22 warm decode");
        }
        std::string leg;
        make_http2_headers_item2(leg, (unsigned char)HTTP2_CODE_content_type, std::string("text/html"));
        CHECK(dec.block(leg), "T22 legacy decode");
        CHECK(enc.entries == dec.dyn, "T22 legacy left tables in step");
        std::string next;
        make_http2_headers_item3(next, std::string("server"), std::string("Paozhu"), &enc);
        CHECK(dec.block(next) && dec_eq(dec, {{"server", "Paozhu"}}), "T22 indexed ref after legacy");
        CHECK(next.size() == 1 && (unsigned char)next[0] == (0x80 | 62), "T22 server still index 62");
    }

    std::printf("PASS %d  FAIL %d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}