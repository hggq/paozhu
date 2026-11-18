// HPACK 编码器动态表在线探针：原始 h2c 客户端打隔离实例，
// 按上线顺序用独立 MiniDec 解码服务端 HEADERS，验证动态表两端同步。
// 用法：test_hpack_live [port]（默认 18895）；由 tests/hpack_live/run.sh 编排起停。
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "http2_frame.h"
#include "http2_huffman.h"

using namespace http;
static int g_fail = 0, g_pass = 0;
static int g_port = 18895;
#define CHECK(cond, msg)                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (cond)                                                                                                      \
            ++g_pass;                                                                                                  \
        else                                                                                                           \
        {                                                                                                              \
            ++g_fail;                                                                                                  \
            std::printf("FAIL %d %s\n", __LINE__, msg);                                                                \
        }                                                                                                              \
    } while (0)

struct MiniDec
{
    std::deque<std::pair<std::string, std::string>> dyn;
    unsigned int size = 0, max_size = 4096;
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
        bool huff        = ((unsigned char)b[p] & 0x80) != 0;
        unsigned int len = rd_int(b, p, 7);
        std::string raw  = b.substr(p, len);
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

static int connect_to(const char *ip, int port)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port   = htons(port);
    inet_pton(AF_INET, ip, &a.sin_addr);
    if (::connect(fd, (sockaddr *)&a, sizeof(a)) != 0)
    {
        ::close(fd);
        return -1;
    }
    return fd;
}
static bool send_all(int fd, const std::string &b)
{
    size_t p = 0;
    while (p < b.size())
    {
        ssize_t n = ::send(fd, b.data() + p, b.size() - p, 0);
        if (n <= 0)
            return false;
        p += n;
    }
    return true;
}
static bool recv_all(int fd, std::string &out, size_t need, int timeout_sec)
{
    while (out.size() < need)
    {
        fd_set rs;
        FD_ZERO(&rs);
        FD_SET(fd, &rs);
        timeval tv{timeout_sec, 0};
        int r = ::select(fd + 1, &rs, nullptr, nullptr, &tv);
        if (r <= 0)
            return false;
        char buf[16384];
        ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n <= 0)
            return false;
        out.append(buf, n);
    }
    return true;
}
static std::string frame(unsigned char type, unsigned char flags, unsigned int sid, const std::string &payload)
{
    std::string f;
    unsigned int ln = (unsigned int)payload.size();
    f.push_back((ln >> 16) & 0xFF);
    f.push_back((ln >> 8) & 0xFF);
    f.push_back(ln & 0xFF);
    f.push_back(type);
    f.push_back(flags);
    f.push_back((sid >> 24) & 0x7F);
    f.push_back((sid >> 16) & 0xFF);
    f.push_back((sid >> 8) & 0xFF);
    f.push_back(sid & 0xFF);
    f.append(payload);
    return f;
}
// 请求头：全字面量不插表（H=0），:method/:scheme 用静态索引
static std::string req_headers(const std::string &path)
{
    std::string b;
    b.push_back((char)0x82);// :method GET
    b.push_back((char)0x86);// :scheme http
    b.push_back(0x04);      // :path 字面量不插表
    b.push_back((char)path.size());
    b.append(path);
    b.push_back(0x01);// :authority
    std::string auth = "127.0.0.1:" + std::to_string(g_port);
    b.push_back((char)auth.size());
    b.append(auth);
    return b;
}
struct Resp
{
    std::string hblock;
    std::string body;
    std::vector<std::pair<std::string, std::string>> decoded;
    bool end = false;
    bool headers_done = false;
    unsigned int first_headers_len = 0;
};
// 读到 streams 全部 END_STREAM；HEADERS 按上线顺序交给 dec 解码并记录
static bool read_responses(int fd, std::map<unsigned int, Resp> &resps, MiniDec &dec,
                           std::vector<unsigned int> &wire_order, int timeout_sec)
{
    std::string in;
    size_t p = 0;
    auto pending = [&]()
    {
        for (auto &[sid, r] : resps)
            if (!r.end)
                return true;
        return false;
    };
    while (pending())
    {
        if (in.size() - p < 9)
        {
            in.erase(0, p);
            p = 0;
            if (!recv_all(fd, in, 9, timeout_sec))
                return false;
        }
        unsigned int ln = ((unsigned char)in[p] << 16) | ((unsigned char)in[p + 1] << 8) | (unsigned char)in[p + 2];
        unsigned char type = in[p + 3], flags = in[p + 4];
        unsigned int sid = (((unsigned char)in[p + 5] & 0x7F) << 24) | ((unsigned char)in[p + 6] << 16) |
                           ((unsigned char)in[p + 7] << 8) | (unsigned char)in[p + 8];
        if (in.size() - p < 9 + ln)
        {
            in.erase(0, p);
            p = 0;
            if (!recv_all(fd, in, 9 + ln, timeout_sec))
                return false;
        }
        std::string payload = in.substr(p + 9, ln);
        p += 9 + ln;
        if (type == 0x07)
        {
            std::printf("GOAWAY received: %s\n", payload.substr(0, 8).c_str());
            return false;
        }
        if (type == 0x06 && !(flags & 0x01))
            send_all(fd, frame(0x06, 0x01, 0, payload));// PING ACK
        if (type == 0x04 && !(flags & 0x01))
            send_all(fd, frame(0x04, 0x01, 0, ""));// SETTINGS ACK
        auto it = resps.find(sid);
        if (it == resps.end())
            continue;
        Resp &r = it->second;
        if (type == 0x01)
        {
            if (r.first_headers_len == 0)
                r.first_headers_len = ln;
            // HEADERS 可能带 priority/padding，baseline 响应没有；CONTINUATION 累积
            r.hblock.append(payload);
            if (flags & 0x04)
            {
                r.headers_done = true;
                wire_order.push_back(sid);
                if (!dec.block(r.hblock))
                {
                    std::printf("decode fail stream %u\n", sid);
                    return false;
                }
                r.decoded = dec.out;
            }
            if (flags & 0x01)
                r.end = true;
        }
        else if (type == 0x09)
        {
            r.hblock.append(payload);
            if (flags & 0x04)
            {
                r.headers_done = true;
                wire_order.push_back(sid);
                if (!dec.block(r.hblock))
                    return false;
                r.decoded = dec.out;
            }
        }
        else if (type == 0x00)
        {
            r.body.append(payload);
            if (flags & 0x01)
                r.end = true;
            // 客户端接收窗口默认 65535：大正文必须随收随补（连接级 + 流级），
            // 否则服务端停等 WINDOW_UPDATE，探针超时假红。
            if (!payload.empty())
            {
                std::string wu;
                unsigned int n = (unsigned int)payload.size();
                wu.push_back((n >> 24) & 0xFF);
                wu.push_back((n >> 16) & 0xFF);
                wu.push_back((n >> 8) & 0xFF);
                wu.push_back(n & 0xFF);
                send_all(fd, frame(0x08, 0, 0, wu));
                send_all(fd, frame(0x08, 0, sid, wu));
            }
        }
        else if (type == 0x03)
        {
            std::printf("RST stream %u\n", sid);
            return false;
        }
    }
    return true;
}
static std::string field(const Resp &r, const std::string &name)
{
    for (auto &[n, v] : r.decoded)
        if (n == name)
            return v;
    return "<missing>";
}

int main(int argc, char **argv)
{
    const char *ip = "127.0.0.1";
    if (argc > 1)
        g_port = std::atoi(argv[1]);
    int port = g_port;
    std::string preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";

    // ---- 连接 1：默认表上限，顺序 + 并发 + 中途缩表 ----
    {
        int fd = connect_to(ip, port);
        CHECK(fd >= 0, "connect 1");
        if (fd < 0)
            return 1;
        send_all(fd, preface + frame(0x04, 0, 0, ""));
        MiniDec dec;
        std::vector<unsigned int> order;

        // 顺序两请求：第二个 HEADERS 必须缩成 Indexed
        std::map<unsigned int, Resp> r1;
        r1[1] = Resp{};
        send_all(fd, frame(0x01, 0x05, 1, req_headers("/hello")));
        CHECK(read_responses(fd, r1, dec, order, 5), "read resp1");
        CHECK(r1[1].headers_done && r1[1].end, "resp1 complete");
        CHECK(field(r1[1], ":status") == "200", "resp1 status 200");
        CHECK(field(r1[1], "server") == "Paozhu", "resp1 server");
        CHECK(r1[1].first_headers_len > 10, "resp1 literal headers");
        std::string cl1 = field(r1[1], "content-length");
        CHECK(cl1 == std::to_string(r1[1].body.size()), "resp1 CL matches body");

        std::map<unsigned int, Resp> r2;
        r2[3] = Resp{};
        send_all(fd, frame(0x01, 0x05, 3, req_headers("/hello")));
        CHECK(read_responses(fd, r2, dec, order, 5), "read resp2");
        CHECK(field(r2[3], ":status") == "200", "resp2 status");
        CHECK(field(r2[3], "content-type") == field(r1[1], "content-type"), "resp2 CT same");
        CHECK(r2[3].first_headers_len <= 6, ("resp2 indexed small got " + std::to_string(r2[3].first_headers_len)).c_str());

        // 并发 8 条流一次打出：上线顺序解码必须全部正确（E6 判别臂）
        std::map<unsigned int, Resp> rc;
        std::string burst;
        for (unsigned int sid = 5; sid <= 19; sid += 2)
        {
            rc[sid] = Resp{};
            burst += frame(0x01, 0x05, sid, req_headers(sid % 4 == 1 ? "/" : "/hello"));
        }
        send_all(fd, burst);
        CHECK(read_responses(fd, rc, dec, order, 8), "read concurrent");
        for (auto &[sid, r] : rc)
        {
            CHECK(r.headers_done && r.end, "concurrent complete");
            CHECK(field(r, ":status") == "200", "concurrent status");
            CHECK(field(r, "server") == "Paozhu", "concurrent server");
            CHECK(field(r, "content-length") == std::to_string(r.body.size()), "concurrent CL");
        }

        // 中途缩表到 60：下一个头块必须以 Size Update 开头
        std::string settings;
        settings.push_back(0x00);
        settings.push_back(0x01);
        settings.push_back(0x00);
        settings.push_back(0x00);
        settings.push_back(0x00);
        settings.push_back(60);
        send_all(fd, frame(0x04, 0, 0, settings));
        std::map<unsigned int, Resp> r3;
        r3[21] = Resp{};
        send_all(fd, frame(0x01, 0x05, 21, req_headers("/hello")));
        CHECK(read_responses(fd, r3, dec, order, 5), "read after shrink");
        CHECK((unsigned char)r3[21].hblock[0] == 0x3F || (unsigned char)r3[21].hblock[0] == 0x20,
              "shrink block starts with size update");
        CHECK(field(r3[21], ":status") == "200", "after shrink status");
        CHECK(dec.max_size == 60, "decoder max 60");
        ::close(fd);
    }

    // ---- 连接 2：SETTINGS_HEADER_TABLE_SIZE=0 → 退化不插表 ----
    {
        int fd = connect_to(ip, port);
        CHECK(fd >= 0, "connect 2");
        if (fd < 0)
            return 1;
        std::string settings;
        settings.push_back(0x00);
        settings.push_back(0x01);
        settings.push_back(0x00);
        settings.push_back(0x00);
        settings.push_back(0x00);
        settings.push_back(0x00);
        send_all(fd, preface + frame(0x04, 0, 0, settings));
        MiniDec dec;
        std::vector<unsigned int> order;
        std::map<unsigned int, Resp> r1;
        r1[1] = Resp{};
        send_all(fd, frame(0x01, 0x05, 1, req_headers("/hello")));
        CHECK(read_responses(fd, r1, dec, order, 5), "read max0 resp1");
        CHECK((unsigned char)r1[1].hblock[0] == 0x20, "max0 first block size update 0");
        CHECK(field(r1[1], ":status") == "200", "max0 status");
        std::map<unsigned int, Resp> r2;
        r2[3] = Resp{};
        send_all(fd, frame(0x01, 0x05, 3, req_headers("/hello")));
        CHECK(read_responses(fd, r2, dec, order, 5), "read max0 resp2");
        CHECK(field(r2[3], ":status") == "200", "max0 resp2 status");
        CHECK(dec.dyn.empty(), "max0 decoder table stays empty");
        CHECK(r2[3].first_headers_len > 10, "max0 no indexing benefit");
        ::close(fd);
    }

    // ---- 连接 3：动态表重点臂——混合路由并发 churn / 顺序 churn / 缩 0 再抬回 ----
    {
        int fd = connect_to(ip, port);
        CHECK(fd >= 0, "connect 3");
        if (fd < 0)
            return 1;
        send_all(fd, preface + frame(0x04, 0, 0, ""));
        MiniDec dec;
        std::vector<unsigned int> order;
        const char *paths[4] = {"/hello", "/", "/img1.jpg", "/index.html"};

        // 60 条流一次打出，混合动态/静态路由：静态响应带 date/etag/last-modified，
        // 值随时间变化 → 真实逐出 churn；全部按上线顺序共享一张解码表。
        std::map<unsigned int, Resp> rc;
        std::string burst;
        unsigned int idx = 0;
        for (unsigned int sid = 1; sid <= 119; sid += 2)
        {
            rc[sid] = Resp{};
            burst += frame(0x01, 0x05, sid, req_headers(paths[idx++ % 4]));
        }
        send_all(fd, burst);
        CHECK(read_responses(fd, rc, dec, order, 30), "read 60-stream churn");
        for (auto &[sid, r] : rc)
        {
            CHECK(r.headers_done && r.end, "churn complete");
            CHECK(field(r, ":status") == "200", "churn status");
            CHECK(field(r, "server") == "Paozhu", "churn server");
            CHECK(field(r, "content-length") == std::to_string(r.body.size()), "churn CL matches body");
        }

        // 顺序 30 请求继续 churn（逐出深度叠加）
        bool seq_ok = true;
        for (unsigned int i = 0; i < 30; ++i)
        {
            unsigned int sid = 121 + 2 * i;
            std::map<unsigned int, Resp> rs;
            rs[sid] = Resp{};
            send_all(fd, frame(0x01, 0x05, sid, req_headers(paths[i % 4])));
            if (!read_responses(fd, rs, dec, order, 15))
            {
                seq_ok = false;
                break;
            }
            Resp &r = rs[sid];
            if (!r.headers_done || !r.end || field(r, ":status") != "200" ||
                field(r, "content-length") != std::to_string(r.body.size()))
                seq_ok = false;
        }
        CHECK(seq_ok, "30-step sequential churn");

        // 缩到 0：信号 0x20 + 退化不插表
        std::string s0;
        s0.push_back(0x00);
        s0.push_back(0x01);
        s0.push_back(0x00);
        s0.push_back(0x00);
        s0.push_back(0x00);
        s0.push_back(0x00);
        send_all(fd, frame(0x04, 0, 0, s0));
        std::map<unsigned int, Resp> r0;
        r0[201] = Resp{};
        send_all(fd, frame(0x01, 0x05, 201, req_headers("/hello")));
        CHECK(read_responses(fd, r0, dec, order, 5), "read shrink-to-0");
        CHECK((unsigned char)r0[201].hblock[0] == 0x20, "shrink-to-0 signal");
        CHECK(dec.dyn.empty(), "shrink-to-0 table empty");
        CHECK(field(r0[201], ":status") == "200", "shrink-to-0 status");

        // 抬回 4096：同样发信号（4096），重新插表，下一请求恢复 Indexed
        std::string s4096;
        s4096.push_back(0x00);
        s4096.push_back(0x01);
        s4096.push_back(0x00);
        s4096.push_back(0x00);
        s4096.push_back(0x10);
        s4096.push_back(0x00);
        send_all(fd, frame(0x04, 0, 0, s4096));
        std::map<unsigned int, Resp> rg;
        rg[203] = Resp{};
        send_all(fd, frame(0x01, 0x05, 203, req_headers("/hello")));
        CHECK(read_responses(fd, rg, dec, order, 5), "read regrow");
        // 抬表也要发信号（解码器上限只跟信号走）：4096-31=4065 → 3F E1 1F
        CHECK((unsigned char)rg[203].hblock[0] == 0x3F, "regrow signals new max");
        CHECK(dec.max_size == 4096, "regrow decoder 4096");
        CHECK(!dec.dyn.empty(), "regrow reinserts");
        CHECK(field(rg[203], ":status") == "200", "regrow status");
        std::map<unsigned int, Resp> rg2;
        rg2[205] = Resp{};
        send_all(fd, frame(0x01, 0x05, 205, req_headers("/hello")));
        CHECK(read_responses(fd, rg2, dec, order, 5), "read regrow hit");
        CHECK(rg2[205].first_headers_len <= 6, "regrow second request indexed");
        CHECK(field(rg2[205], "content-length") == std::to_string(rg2[205].body.size()), "regrow CL");
        ::close(fd);
    }

    std::printf("LIVE PASS %d FAIL %d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}