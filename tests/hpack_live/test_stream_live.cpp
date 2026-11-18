// HTTP/2 流式发送在线探针：原始 h2c 客户端打隔离实例，验证内容环路径
// （send_make_*/send_chunk_* 同步+异步）：正文完整、DATA 分块上限、窗口停等与恢复、
// 中途 RST 死流丢弃、流式与锁内联响应交错后动态表仍同步。
// 用法：test_stream_live <port> <pdf 文件路径>；由 tests/hpack_live/run_stream.sh 编排起停。
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <fstream>
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
static std::string wu_payload(unsigned int n)
{
    std::string wu;
    wu.push_back((n >> 24) & 0xFF);
    wu.push_back((n >> 16) & 0xFF);
    wu.push_back((n >> 8) & 0xFF);
    wu.push_back(n & 0xFF);
    return wu;
}
struct Resp
{
    std::string hblock;
    std::string body;
    std::vector<std::pair<std::string, std::string>> decoded;
    bool end = false;
    bool headers_done = false;
    unsigned int max_data = 0;
    unsigned int data_frames = 0;
    bool end_on_data = false;
};
struct ReadCtl
{
    bool auto_wu = true;
    unsigned int server_init_window = 65535;
    bool got_rst = false;
    unsigned int rst_sid = 0;
    bool got_goaway = false;
};
// 读到 streams 全部 END_STREAM。auto_wu=false 时不补窗口（停等臂用）。
// 超时返回 false 但已收帧保留在 resps 里，调用方可检查部分进度。
static bool read_responses(int fd, std::map<unsigned int, Resp> &resps, MiniDec &dec, ReadCtl &ctl, int timeout_sec)
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
            ctl.got_goaway = true;
            std::printf("GOAWAY received\n");
            return false;
        }
        if (type == 0x06 && !(flags & 0x01))
            send_all(fd, frame(0x06, 0x01, 0, payload));// PING ACK
        if (type == 0x04 && !(flags & 0x01))
        {
            for (size_t i = 0; i + 6 <= payload.size(); i += 6)
            {
                unsigned int id = ((unsigned char)payload[i] << 8) | (unsigned char)payload[i + 1];
                unsigned int val = ((unsigned char)payload[i + 2] << 24) | ((unsigned char)payload[i + 3] << 16) |
                                   ((unsigned char)payload[i + 4] << 8) | (unsigned char)payload[i + 5];
                if (id == 0x04)
                    ctl.server_init_window = val;
            }
            send_all(fd, frame(0x04, 0x01, 0, ""));// SETTINGS ACK
        }
        if (type == 0x03)
        {
            ctl.got_rst = true;
            ctl.rst_sid = sid;
            std::printf("RST stream %u received\n", sid);
            auto it = resps.find(sid);
            if (it != resps.end())
                it->second.end = true;
            continue;
        }
        auto it = resps.find(sid);
        if (it == resps.end())
            continue;
        Resp &r = it->second;
        if (type == 0x01 || type == 0x09)
        {
            r.hblock.append(payload);
            if (flags & 0x04)
            {
                r.headers_done = true;
                if (!dec.block(r.hblock))
                {
                    std::printf("decode fail stream %u\n", sid);
                    return false;
                }
                r.decoded = dec.out;
            }
            if ((flags & 0x01) && type == 0x01)
                r.end = true;
        }
        else if (type == 0x00)
        {
            r.body.append(payload);
            if (ln > r.max_data)
                r.max_data = ln;
            ++r.data_frames;
            if (flags & 0x01)
            {
                r.end = true;
                r.end_on_data = true;
            }
            if (ctl.auto_wu && !payload.empty())
            {
                send_all(fd, frame(0x08, 0, 0, wu_payload(ln)));
                send_all(fd, frame(0x08, 0, sid, wu_payload(ln)));
            }
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
static std::string read_file(const char *path)
{
    std::ifstream f(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}
static const std::string kTextBody = "Hello! 这是通过 Content-Length 模式发送的文本内容。\n"
                                     "第一行\n第二行\n第三行\n";
static const std::string kChunkBody = "<html><body>\n<h1>Chunked Transfer 测试</h1>\n"
                                      "<p>第 1 行：这是通过 chunked transfer 发送的内容。</p>\n"
                                      "<p>第 2 行：这是通过 chunked transfer 发送的内容。</p>\n"
                                      "<p>第 3 行：这是通过 chunked transfer 发送的内容。</p>\n"
                                      "<p>第 4 行：这是通过 chunked transfer 发送的内容。</p>\n"
                                      "<p>第 5 行：这是通过 chunked transfer 发送的内容。</p>\n"
                                      "</body></html>";
static const std::string kChunkAsyncBody = "<html><body>\n<h1>Chunked Async 测试</h1>\n"
                                           "<p>异步第 1 行</p>\n<p>异步第 2 行</p>\n<p>异步第 3 行</p>\n"
                                           "</body></html>";

int main(int argc, char **argv)
{
    const char *ip = "127.0.0.1";
    if (argc > 1)
        g_port = std::atoi(argv[1]);
    const char *pdf_path = argc > 2 ? argv[2] : "docs/webpdf_tutorial.pdf";
    std::string pdf = read_file(pdf_path);
    if (pdf.size() < 100000)
    {
        std::printf("pdf fixture too small or missing: %s (%zu bytes)\n", pdf_path, pdf.size());
        return 2;
    }
    std::string preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";

    // ---- 连接 1：同步小文本 + 同步/异步大 PDF 顺序臂 ----
    {
        int fd = connect_to(ip, g_port);
        CHECK(fd >= 0, "connect 1");
        if (fd < 0)
            return 1;
        send_all(fd, preface + frame(0x04, 0, 0, ""));
        MiniDec dec;
        ReadCtl ctl;

        std::map<unsigned int, Resp> r1;
        r1[1] = Resp{};
        send_all(fd, frame(0x01, 0x05, 1, req_headers("/test_httppeersend_text")));
        CHECK(read_responses(fd, r1, dec, ctl, 10), "sync text complete");
        CHECK(field(r1[1], ":status") == "200", "sync text status");
        CHECK(r1[1].body == kTextBody, "sync text body exact");
        CHECK(field(r1[1], "content-length") == std::to_string(kTextBody.size()), "sync text CL");
        CHECK(r1[1].end_on_data, "sync text END_STREAM on DATA");

        std::map<unsigned int, Resp> r2;
        r2[3] = Resp{};
        send_all(fd, frame(0x01, 0x05, 3, req_headers("/test_httppeersend")));
        CHECK(read_responses(fd, r2, dec, ctl, 30), "sync pdf complete");
        CHECK(field(r2[3], ":status") == "200", "sync pdf status");
        CHECK(r2[3].body == pdf, "sync pdf bytes exact");
        CHECK(r2[3].max_data <= 15360, "sync pdf DATA chunk cap 15360");
        CHECK(r2[3].max_data > 0 && r2[3].end_on_data, "sync pdf END_STREAM on last DATA");
        CHECK(field(r2[3], "content-length") == std::to_string(pdf.size()), "sync pdf CL");

        std::map<unsigned int, Resp> r3;
        r3[5] = Resp{};
        send_all(fd, frame(0x01, 0x05, 5, req_headers("/test_httppeersend_async")));
        CHECK(read_responses(fd, r3, dec, ctl, 30), "async pdf complete");
        CHECK(field(r3[5], ":status") == "200", "async pdf status");
        CHECK(r3[5].body == pdf, "async pdf bytes exact");
        CHECK(r3[5].max_data <= 15360, "async pdf DATA chunk cap 15360");
        CHECK(r3[5].end_on_data, "async pdf END_STREAM on last DATA");

        // 流式跑过之后锁内联路径仍正常、动态表仍同步（第二个 /hello 必须 Indexed）
        std::map<unsigned int, Resp> r4;
        r4[7] = Resp{};
        send_all(fd, frame(0x01, 0x05, 7, req_headers("/hello")));
        CHECK(read_responses(fd, r4, dec, ctl, 10), "hello after streaming");
        CHECK(field(r4[7], ":status") == "200", "hello after streaming status");
        std::map<unsigned int, Resp> r5;
        r5[9] = Resp{};
        send_all(fd, frame(0x01, 0x05, 9, req_headers("/hello")));
        CHECK(read_responses(fd, r5, dec, ctl, 10), "hello indexed after streaming");
        CHECK(r5[9].headers_done && r5[9].end, "hello2 complete");
        CHECK(field(r5[9], ":status") == "200", "hello2 status");
        ::close(fd);
    }

    // ---- 连接 2：窗口停等臂——不补窗口，正文必须停在初始窗口；补窗后恢复 ----
    {
        int fd = connect_to(ip, g_port);
        CHECK(fd >= 0, "connect 2");
        if (fd < 0)
            return 1;
        send_all(fd, preface + frame(0x04, 0, 0, ""));
        MiniDec dec;
        ReadCtl ctl;
        ctl.auto_wu = false;
        std::map<unsigned int, Resp> r1;
        r1[1] = Resp{};
        send_all(fd, frame(0x01, 0x05, 1, req_headers("/test_httppeersend")));
        bool stalled = !read_responses(fd, r1, dec, ctl, 2);// 2s 内读不完 = 停等
        CHECK(stalled, "window stall: no END_STREAM without WU");
        CHECK(!ctl.got_rst && !ctl.got_goaway, "window stall: no RST/GOAWAY");
        // 探针 SETTINGS 为空 → 客户端通告的初始窗口取默认 65535，
        // 服务端每流发送窗口与连接级发送窗口都停在这个数上。
        CHECK(r1[1].body.size() == 65535,
              ("window stall: bytes == 65535, got " + std::to_string(r1[1].body.size())).c_str());
        CHECK(r1[1].max_data <= 15360, "window stall: chunk cap");
        // 再等 1s 确认一个字节都不多送
        std::string extra;
        fd_set rs;
        FD_ZERO(&rs);
        FD_SET(fd, &rs);
        timeval tv{1, 0};
        int sel = ::select(fd + 1, &rs, nullptr, nullptr, &tv);
        if (sel > 0)
        {
            char buf[4096];
            ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
            if (n > 0)
                extra.append(buf, n);
        }
        CHECK(extra.empty(), "window stall: zero extra bytes while stalled");
        // 连接级 + 流级一次补满，恢复自动补窗读完
        send_all(fd, frame(0x08, 0, 0, wu_payload((unsigned int)pdf.size())));
        send_all(fd, frame(0x08, 0, 1, wu_payload((unsigned int)pdf.size())));
        ctl.auto_wu = true;
        CHECK(read_responses(fd, r1, dec, ctl, 30), "window resume complete");
        CHECK(r1[1].body == pdf, "window resume bytes exact");
        CHECK(r1[1].end_on_data, "window resume END_STREAM on DATA");
        ::close(fd);
    }

    // ---- 连接 3：中途 RST 臂——死流丢弃，连接存活，后续请求正常 ----
    {
        int fd = connect_to(ip, g_port);
        CHECK(fd >= 0, "connect 3");
        if (fd < 0)
            return 1;
        send_all(fd, preface + frame(0x04, 0, 0, ""));
        MiniDec dec;
        ReadCtl ctl;
        std::map<unsigned int, Resp> r1;
        r1[1] = Resp{};
        send_all(fd, frame(0x01, 0x05, 1, req_headers("/test_httppeersend")));
        // 读到至少 2 个 DATA 帧（生产者已深入分块循环）
        std::string in;
        size_t p = 0;
        unsigned int data_seen = 0;
        while (data_seen < 2)
        {
            if (in.size() - p < 9)
            {
                in.erase(0, p);
                p = 0;
                CHECK(recv_all(fd, in, 9, 10), "rst arm read header");
                if (in.size() < 9)
                    break;
            }
            unsigned int ln = ((unsigned char)in[p] << 16) | ((unsigned char)in[p + 1] << 8) | (unsigned char)in[p + 2];
            unsigned char type = in[p + 3], flags = in[p + 4];
            unsigned int sid = (((unsigned char)in[p + 5] & 0x7F) << 24) | ((unsigned char)in[p + 6] << 16) |
                               ((unsigned char)in[p + 7] << 8) | (unsigned char)in[p + 8];
            if (in.size() - p < 9 + ln)
            {
                in.erase(0, p);
                p = 0;
                CHECK(recv_all(fd, in, 9 + ln, 10), "rst arm read payload");
                if (in.size() < 9 + ln)
                    break;
            }
            std::string payload = in.substr(p + 9, ln);
            p += 9 + ln;
            if (type == 0x04 && !(flags & 0x01))
                send_all(fd, frame(0x04, 0x01, 0, ""));
            if (sid == 1 && type == 0x00)
            {
                ++data_seen;
                r1[1].body.append(payload);
                send_all(fd, frame(0x08, 0, 0, wu_payload(ln)));
                send_all(fd, frame(0x08, 0, 1, wu_payload(ln)));
            }
        }
        CHECK(data_seen >= 2, "rst arm got 2 DATA frames");
        // 客户端主动 CANCEL；对端已 RST 的流服务端不得回 RST
        send_all(fd, frame(0x03, 0, 1, wu_payload(0x08)));
        // 排空 1s 内在途帧：不得出现 sid 1 的 RST，也不得 GOAWAY
        bool rst_back = false, goaway = false;
        for (;;)
        {
            fd_set rs2;
            FD_ZERO(&rs2);
            FD_SET(fd, &rs2);
            timeval tv2{1, 0};
            int sel = ::select(fd + 1, &rs2, nullptr, nullptr, &tv2);
            if (sel <= 0)
                break;
            char buf[16384];
            ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
            if (n <= 0)
                break;
            in.append(buf, n);
            while (in.size() - p >= 9)
            {
                unsigned int ln =
                    ((unsigned char)in[p] << 16) | ((unsigned char)in[p + 1] << 8) | (unsigned char)in[p + 2];
                if (in.size() - p < 9 + ln)
                    break;
                unsigned char type = in[p + 3];
                unsigned int sid = (((unsigned char)in[p + 5] & 0x7F) << 24) | ((unsigned char)in[p + 6] << 16) |
                                   ((unsigned char)in[p + 7] << 8) | (unsigned char)in[p + 8];
                if (type == 0x03 && sid == 1)
                    rst_back = true;
                if (type == 0x07)
                    goaway = true;
                p += 9 + ln;
            }
            in.erase(0, p);
            p = 0;
        }
        CHECK(!rst_back, "rst arm: no RST echoed to peer-reset stream");
        CHECK(!goaway, "rst arm: no GOAWAY");
        // 同一连接后续请求必须正常（死流帧被丢弃、窗口退还、消费者存活）
        std::map<unsigned int, Resp> r2;
        r2[3] = Resp{};
        send_all(fd, frame(0x01, 0x05, 3, req_headers("/hello")));
        CHECK(read_responses(fd, r2, dec, ctl, 10), "rst arm: hello after RST");
        CHECK(field(r2[3], ":status") == "200", "rst arm: hello status");
        CHECK(field(r2[3], "content-length") == std::to_string(r2[3].body.size()), "rst arm: hello CL");
        // 再打一次大 PDF：连接级窗口退还必须足额，否则这里会停等超时
        std::map<unsigned int, Resp> r3;
        r3[5] = Resp{};
        send_all(fd, frame(0x01, 0x05, 5, req_headers("/test_httppeersend_async")));
        CHECK(read_responses(fd, r3, dec, ctl, 30), "rst arm: pdf after RST");
        CHECK(r3[5].body == pdf, "rst arm: pdf bytes exact after RST");
        ::close(fd);
    }

    // ---- 连接 4：交错臂——同步/异步流式 + chunked + 锁内联并发，共享一张解码表 ----
    {
        int fd = connect_to(ip, g_port);
        CHECK(fd >= 0, "connect 4");
        if (fd < 0)
            return 1;
        send_all(fd, preface + frame(0x04, 0, 0, ""));
        MiniDec dec;
        ReadCtl ctl;
        std::map<unsigned int, Resp> rc;
        std::string burst;
        rc[1] = Resp{};
        burst += frame(0x01, 0x05, 1, req_headers("/test_httppeersend"));
        rc[3] = Resp{};
        burst += frame(0x01, 0x05, 3, req_headers("/test_httppeersend_async"));
        rc[5] = Resp{};
        burst += frame(0x01, 0x05, 5, req_headers("/test_chunked"));
        rc[7] = Resp{};
        burst += frame(0x01, 0x05, 7, req_headers("/test_chunked_async"));
        rc[9] = Resp{};
        burst += frame(0x01, 0x05, 9, req_headers("/test_chunkedfile"));
        rc[11] = Resp{};
        burst += frame(0x01, 0x05, 11, req_headers("/hello"));
        rc[13] = Resp{};
        burst += frame(0x01, 0x05, 13, req_headers("/"));
        send_all(fd, burst);
        CHECK(read_responses(fd, rc, dec, ctl, 60), "interleave all complete");
        CHECK(rc[1].body == pdf, "interleave sync pdf bytes");
        CHECK(rc[3].body == pdf, "interleave async pdf bytes");
        CHECK(rc[5].body == kChunkBody, "interleave chunked body");
        CHECK(rc[7].body == kChunkAsyncBody, "interleave chunked async body");
        CHECK(rc[9].body == pdf, "interleave chunked file bytes");
        // chunked/SSE 头块不得带 content-length（0 会让严格对端把 DATA 判 PROTOCOL_ERROR）
        CHECK(field(rc[5], "content-length") == "<missing>", "chunked no content-length");
        CHECK(field(rc[7], "content-length") == "<missing>", "chunked async no content-length");
        CHECK(field(rc[9], "content-length") == "<missing>", "chunked file no content-length");
        for (auto &[sid, r] : rc)
        {
            CHECK(r.headers_done && r.end, "interleave stream complete");
            CHECK(field(r, ":status") == "200", "interleave status 200");
            CHECK(r.max_data <= 15360, "interleave DATA chunk cap");
        }
        CHECK(field(rc[11], "content-length") == std::to_string(rc[11].body.size()), "interleave hello CL");
        // 交错之后再顺序两请求：动态表两端仍同步
        std::map<unsigned int, Resp> r2;
        r2[15] = Resp{};
        send_all(fd, frame(0x01, 0x05, 15, req_headers("/hello")));
        CHECK(read_responses(fd, r2, dec, ctl, 10), "post-interleave hello1");
        std::map<unsigned int, Resp> r3;
        r3[17] = Resp{};
        send_all(fd, frame(0x01, 0x05, 17, req_headers("/hello")));
        CHECK(read_responses(fd, r3, dec, ctl, 10), "post-interleave hello2");
        CHECK(field(r3[17], ":status") == "200", "post-interleave status");
        ::close(fd);
    }

    std::printf("STREAM LIVE PASS %d FAIL %d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}