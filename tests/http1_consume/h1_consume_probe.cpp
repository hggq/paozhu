/*
 * h1_consume_probe.cpp — HTTP/1 解析器的字节消费口径离线单测
 *
 * 驱动真实 httpparse::process()，逐轮问「这一轮它吃掉了几个字节」，并按
 * vendor/httpserver/src/server.cpp 里 client_http1_loop 尾部的同一套算法把同一个 TCP 段里
 * 剩下的字节搬回缓冲头部继续解，验证 keepalive 复用、多条请求同包、以及请求被 TCP 拆包时
 * 既不丢字节也不多吃字节。末尾两组钉住单连接请求数上限的口径（计数初值 1、判定在自增之前
 * ⇒ 恰好应答 N 条）。
 *
 * 判据 = 每组一行 PASS/FAIL，末行 SUMMARY；全过退出码 0，任一失败退出码 1。
 *
 * 跑法：bash tests/http1_consume/run.sh（单臂）或 all_arms.sh（base + 四条改坏臂）。
 * 解析器的符号闭包是整个产品，所以不能用 tests/CMakeLists.txt 的 glob 目标，
 * run.sh 走 build/ 整档对象换链，探针二进制落 temp/h1_consume/，不写 bin/paozhu。
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "cost_define.h"
#include "http_parse.h"
#include "httppeer.h"
#include "server_localvar.h"

static int g_failures = 0;
static int g_groups   = 0;

static void group_result(const char *name, bool ok, const std::string &reading)
{
    g_groups++;
    if (ok)
    {
        std::cout << "PASS: " << name << " [" << reading << "]" << std::endl;
    }
    else
    {
        g_failures++;
        std::cout << "FAIL: " << name << " [" << reading << "]" << std::endl;
    }
}

static std::string rd(const std::string &k, unsigned long long v)
{
    return k + "=" + std::to_string(v);
}

// 读数里的值可能带着 \r\n（字段值被吞进整包时就是这样），不转义会把一行判据拆成几行
static std::string esc(std::string_view s)
{
    std::string o;
    for (unsigned char c : s)
    {
        if (c == '\r')
            o += "\\r";
        else if (c == '\n')
            o += "\\n";
        else if (c >= 32 && c < 127 && c != '"')
            o.push_back((char)c);
        else
        {
            char b[8];
            std::snprintf(b, sizeof b, "\\x%02x", c);
            o += b;
        }
    }
    return o;
}

static std::string rds(const std::string &k, const std::string &v)
{
    return k + "=" + esc(v);
}
static std::string rdb(const std::string &k, bool v)
{
    return k + "=" + std::to_string(v);
}

// —— 报文构造 ——
static std::string getreq(const std::string &path, const std::string &extra = "")
{
    return "GET " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\n" + extra + "\r\n";
}
static std::string postreq(const std::string &path, const std::string &body)
{
    return "POST " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\n"
           "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: " +
           std::to_string(body.size()) + "\r\n\r\n" + body;
}
// 只到头部为止的请求体长度：postreq() 的返回值含 body，这里剥出来
static std::string posthdr(const std::string &path, const std::string &body)
{
    return postreq(path, body).substr(0, postreq(path, body).size() - body.size());
}

// 一轮 process() 的读数，字段全是解析器与 peer 的真实状态
struct Rec
{
    unsigned int consumed = 0;   // readoffset：本轮真正吃掉的字节数
    unsigned int bufsize  = 0;   // 本轮交给 process() 的字节数
    unsigned int error    = 0;
    bool not_allowed      = false;
    bool unknown          = false;
    bool finished         = false;
    bool header_done      = false;
    bool state_keepalive  = false;
    unsigned char method  = 0;
    std::string url;
    std::string alpha;
    unsigned long long content_length = 0;
};

/*
 * 与 client_http1_loop 逐条对齐的连接模型：缓冲空了才"读包"（read_some 固定从下标 0 覆盖，
 * 所以有残留时不许读），请求完成后先过两条关闭判定，再记 consumed、clear、把残留搬到头部。
 * 业务派发（http1loop）不在这里，所以"已应答"等于"这一轮 getfinish() 为真"。
 */
class Loop
{
public:
    Loop()
    {
        peer_ = std::make_shared<http::httppeer>();
        pre_  = std::make_unique<http::httpparse>();
        pre_->setpeer(peer_);
    }

    void push(const std::string &segment)
    {
        queue_.push_back(segment);
    }

    void run(unsigned int max_steps = 4000)
    {
        for (unsigned int step = 0; step < max_steps; ++step)
        {
            if (readnum_ == 0)
            {
                if (qi_ >= queue_.size())
                {
                    closed_reason_ = "idle";
                    return;
                }
                const std::string &seg = queue_[qi_++];
                if (seg.size() > buf_.size())
                {
                    closed_reason_ = "segment_over_buffer";
                    return;
                }
                std::memcpy(buf_.data(), seg.data(), seg.size());
                readnum_ = static_cast<unsigned int>(seg.size());
            }

            pre_->process(buf_.data(), readnum_);

            Rec r;
            r.consumed        = pre_->readoffset;
            r.bufsize         = readnum_;
            r.error           = pre_->error;
            r.not_allowed     = pre_->method_not_allowed;
            r.unknown         = pre_->method_unknown;
            r.finished        = peer_->isfinish;
            r.header_done     = pre_->isfinish_header;
            r.state_keepalive = peer_->state.keepalive;
            r.method          = peer_->method;
            r.url             = peer_->url;
            r.alpha           = peer_->post["alpha"].to_string();
            r.content_length  = peer_->content_length;
            recs_.push_back(r);

            if (r.not_allowed)
            {
                closed_reason_ = "method_not_allowed";
                return;
            }
            if (r.unknown)
            {
                closed_reason_ = "method_unknown";
                return;
            }
            if (r.error > 0)
            {
                closed_reason_ = "bad_request";
                return;
            }
            if (!r.finished)
            {
                // 请求还没收完：这一轮交给它的字节它全吞下了，清零让下一次读包从 0 续写
                readnum_ = 0;
                continue;
            }
            if (!r.state_keepalive)
            {
                closed_reason_ = "keepalive_false";
                return;
            }
#ifndef BENCHMARK
            if (single_link_count >= CONST_HTTP1_KEEPALIVE_MAX_REQUESTS)
            {
                closed_reason_ = "request_cap";
                return;
            }
#endif
            unsigned int consumed = r.consumed;
            pre_->clear();
            peer_->clear();
            single_link_count++;
            if (readnum_ > consumed)
            {
                readnum_ -= consumed;
                std::memmove(buf_.data(), buf_.data() + consumed, readnum_);
            }
            else
            {
                readnum_ = 0;
            }
        }
        closed_reason_ = "step_limit";
    }

    const std::vector<Rec> &recs() const
    {
        return recs_;
    }
    const std::string &closed() const
    {
        return closed_reason_;
    }
    unsigned int answered() const
    {
        unsigned int n = 0;
        for (const Rec &r : recs_)
        {
            if (r.finished)
                n++;
        }
        return n;
    }
    // 连接收口时缓冲里还没被消费的字节：只允许在被判定为不再复用时非零
    unsigned int leftover() const
    {
        return readnum_;
    }

private:
    std::vector<unsigned char> buf_ = std::vector<unsigned char>(4096, 0);
    unsigned int readnum_           = 0;
    std::vector<std::string> queue_;
    size_t qi_                      = 0;
    unsigned int single_link_count  = 1;
    std::shared_ptr<http::httppeer> peer_;
    std::unique_ptr<http::httpparse> pre_;
    std::vector<Rec> recs_;
    std::string closed_reason_;
};

// 单独一次 process()：只问"这些字节被吃到哪"，不做残留搬运
static Rec once(const std::string &bytes)
{
    Loop L;
    L.push(bytes);
    L.run(1);
    const std::vector<Rec> &v = L.recs();
    return v.empty() ? Rec() : v[0];
}

static std::vector<std::string> carve(const std::string &all, size_t per)
{
    std::vector<std::string> segs;
    for (size_t i = 0; i < all.size(); i += per)
        segs.push_back(all.substr(i, per));
    return segs;
}

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code pec;
    const std::string outdir = "./temp/h1_consume/out/";
    std::filesystem::create_directories(outdir, pec);
    gv.temp_path = outdir;

#ifndef H1_ARM_NAME
#define H1_ARM_NAME "unset"
#endif
    std::cout << "ARM " H1_ARM_NAME << std::endl;
    std::cout << "BUILD benchmark_cap="
#ifdef BENCHMARK
              "on"
#else
              "off"
#endif
              << " CONST_HTTP1_KEEPALIVE_MAX_REQUESTS=" << CONST_HTTP1_KEEPALIVE_MAX_REQUESTS << std::endl;

    const std::string g1 = getreq("/a");

    // ===== 组 1：单条 GET 恰好消费完头部（含空行的 CRLF）=====
    {
        Rec r = once(g1);
        bool ok = r.finished && r.error == 0 && r.header_done && r.consumed == r.bufsize &&
                  r.consumed == (unsigned int)g1.size() && r.url == "/a" && r.state_keepalive;
        group_result("g1_get_consumes_exactly_header_block",
                     ok,
                     rd("consumed", r.consumed) + " " + rd("want", g1.size()) + " " + rds("url", r.url) + " " +
                         rd("err", r.error) + " " + rdb("keepalive", r.state_keepalive));
    }

    // ===== 组 2：头部被 TCP 拆成两包 ⇒ 第一轮整包吞下，第二轮才完成 =====
    {
        Loop L;
        L.push(g1.substr(0, 12));
        L.push(g1.substr(12));
        L.run();
        const std::vector<Rec> &v = L.recs();
        bool ok = v.size() == 2 && !v[0].finished && v[0].consumed == v[0].bufsize &&
                  v[1].finished && v[1].consumed == v[1].bufsize && v[1].url == "/a" && L.closed() == "idle";
        group_result("g2_header_split_keeps_every_byte",
                     ok,
                     rd("recs", v.size()) + " " + rd("p1", v.size() > 0 ? v[0].consumed : 0) + "/" +
                         std::to_string(v.size() > 0 ? v[0].bufsize : 0) + " " +
                         rdb("p1_finish", v.size() > 0 ? v[0].finished : true) + " " +
                         rd("p2", v.size() > 1 ? v[1].consumed : 0) + "/" +
                         std::to_string(v.size() > 1 ? v[1].bufsize : 0) + " " +
                         rds("p2_url", v.size() > 1 ? v[1].url : "<none>") + " " + rds("closed", L.closed()));
    }

    // ===== 组 3：空行的 "\r" 落在包尾、"\n" 在下一包 ⇒ 那条"补读"只吃掉 1 个字节 =====
    {
        const std::string one   = g1;
        const std::string two   = one + getreq("/b");
        Loop L;
        L.push(two.substr(0, one.size() - 1));   // 到空行的 "\r" 为止
        L.push(two.substr(one.size() - 1));      // "\n" + 第二条请求
        L.run();
        const std::vector<Rec> &v = L.recs();
        bool ok = v.size() == 3 && !v[0].finished && v[0].consumed == v[0].bufsize &&
                  v[1].finished && v[1].consumed == 1 && v[1].url == "/a" &&
                  v[2].finished && v[2].url == "/b" && v[2].consumed == v[2].bufsize && L.closed() == "idle";
        group_result("g3_cr_at_packet_tail_then_lf_only_pass",
                     ok,
                     rd("recs", v.size()) + " " + rd("p1", v.size() > 0 ? v[0].consumed : 0) + "/" +
                         std::to_string(v.size() > 0 ? v[0].bufsize : 0) + " " +
                         rd("p2", v.size() > 1 ? v[1].consumed : 0) + "/" +
                         std::to_string(v.size() > 1 ? v[1].bufsize : 0) + " " +
                         rds("p2_url", v.size() > 1 ? v[1].url : "<none>") + " " +
                         rd("p3", v.size() > 2 ? v[2].consumed : 0) + "/" +
                         std::to_string(v.size() > 2 ? v[2].bufsize : 0) + " " +
                         rds("p3_url", v.size() > 2 ? v[2].url : "<none>") + " " + rds("closed", L.closed()));
    }

    // ===== 组 4：同一个段里两条 GET ⇒ 两条都解出来，顺序对、字节不重不丢 =====
    {
        const std::string first  = getreq("/first");
        const std::string second = getreq("/second");
        Loop L;
        L.push(first + second);
        L.run();
        const std::vector<Rec> &v = L.recs();
        unsigned long long total  = 0;
        for (const Rec &r : v)
            total += r.consumed;
        bool ok = v.size() == 2 && v[0].consumed == (unsigned int)first.size() && v[0].url == "/first" &&
                  v[1].consumed == (unsigned int)second.size() && v[1].url == "/second" &&
                  total == first.size() + second.size() && L.closed() == "idle" && L.leftover() == 0;
        group_result("g4_two_get_in_one_segment_both_parsed",
                     ok,
                     rd("recs", v.size()) + " " + rd("c1", v.size() > 0 ? v[0].consumed : 0) + "/" +
                         std::to_string(first.size()) + " " + rds("u1", v.size() > 0 ? v[0].url : "<none>") + " " +
                         rd("c2", v.size() > 1 ? v[1].consumed : 0) + "/" + std::to_string(second.size()) + " " +
                         rds("u2", v.size() > 1 ? v[1].url : "<none>") + " " + rd("total", total) + " " +
                         rd("leftover", L.leftover()) + " " + rds("closed", L.closed()));
    }

    // ===== 组 5：POST 带完整 body + 尾随 GET ⇒ 停在 body 末尾，尾随那条照解 =====
    {
        const std::string body = "alpha=1&beta=2";
        const std::string hdr  = posthdr("/mp", body);
        const std::string all  = postreq("/mp", body) + getreq("/after");
        Loop L;
        L.push(all);
        L.run();
        const std::vector<Rec> &v = L.recs();
        bool ok = v.size() == 2 && v[0].finished && v[0].consumed == (unsigned int)all.size() - getreq("/after").size() &&
                  v[0].alpha == "1" && v[1].finished && v[1].url == "/after" &&
                  v[1].consumed == v[1].bufsize && L.closed() == "idle";
        group_result("g5_post_stops_at_content_length_then_trailing_get",
                     ok,
                     rd("hdr", hdr.size()) + " " + rd("body", body.size()) + " " +
                         rd("c1", v.size() > 0 ? v[0].consumed : 0) + " " +
                         rds("alpha", v.size() > 0 ? v[0].alpha : "<none>") + " " +
                         rd("c2", v.size() > 1 ? v[1].consumed : 0) + "/" +
                         std::to_string(v.size() > 1 ? v[1].bufsize : 0) + " " +
                         rds("u2", v.size() > 1 ? v[1].url : "<none>") + " " + rds("closed", L.closed()));
    }

    // ===== 组 6：POST 的 body 被拆成两包，第二包尾部还挂着一条 GET =====
    {
        const std::string body = "alpha=hello&beta=world";
        const std::string hdr  = posthdr("/split", body);
        const std::string all  = postreq("/split", body) + getreq("/next");
        unsigned int cut       = (unsigned int)hdr.size() + 6;   // body 中间切一刀
        Loop L;
        L.push(all.substr(0, cut));
        L.push(all.substr(cut));
        L.run();
        const std::vector<Rec> &v = L.recs();
        bool ok = v.size() == 3 && !v[0].finished && v[0].consumed == v[0].bufsize &&
                  v[1].finished && v[1].alpha == "hello" && v[2].finished && v[2].url == "/next" &&
                  v[2].consumed == v[2].bufsize && L.closed() == "idle";
        group_result("g6_body_split_across_packets_then_tail_get",
                     ok,
                     rd("recs", v.size()) + " " + rd("cut", cut) + " " + rdb("p1_finish", v.size() > 0 ? v[0].finished : true) + " " +
                         rds("alpha", v.size() > 1 ? v[1].alpha : "<none>") + " " +
                         rds("u3", v.size() > 2 ? v[2].url : "<none>") + " " + rds("closed", L.closed()));
    }

    // ===== 组 7：对端在 Content-Length 之外多塞 1 个字节 ⇒ 那一个字节不归这条请求 =====
    {
        const std::string hdr  = posthdr("/over", "alpha=1");   // 声明 7 字节
        const std::string tail = getreq("/x");
        Loop L;
        L.push(hdr + "alpha=1" + "Z" + tail);   // 实际 8 个字节：多出来的 "Z" 不属于本请求
        L.run();
        const std::vector<Rec> &v = L.recs();
        bool ok = v.size() == 2 && v[0].consumed == (unsigned int)(hdr.size() + 7) &&
                  v[0].alpha == "1" && v[1].finished == false && v[1].unknown == true &&
                  v[1].consumed == 18 && v[1].bufsize == 37 && L.closed() == "method_unknown";
        group_result("g7_declared_length_boundary_is_exact",
                     ok,
                     rd("hdr", hdr.size()) + " " + rd("c1", v.size() > 0 ? v[0].consumed : 0) + "/" +
                         std::to_string(hdr.size() + 7) + " " +
                         rds("alpha", v.size() > 0 ? v[0].alpha : "<none>") + " " +
                         rd("recs", v.size()) + " " +
                         rdb("unk2", v.size() > 1 ? v[1].unknown : false) + "/" +
                         rdb("fin2", v.size() > 1 ? v[1].finished : true) + " " +
                         std::to_string(v.size() > 1 ? v[1].consumed : 0) + ":" +
                         std::to_string(v.size() > 1 ? v[1].bufsize : 0) + " " + rds("closed", L.closed()));
    }

    // ===== 组 8：GET 带 Content-Length ⇒ 只吃声明的那几个字节，然后关连接（残留不搬运）=====
    {
        const std::string s = "GET /g HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 4\r\n\r\nabcd" + getreq("/y");
        size_t        hsz   = s.find("\r\n\r\n") + 4;   // 头部（含空行）字节数
        Loop L;
        L.push(s);
        L.run();
        const std::vector<Rec> &v = L.recs();
        bool ok = v.size() == 1 && v[0].finished && !v[0].state_keepalive &&
                  L.closed() == "keepalive_false" && v[0].consumed == (unsigned int)(hsz + 4) &&
                  v[0].content_length == 4;
        group_result("g8_get_with_content_length_closes_instead_of_refeeding",
                     ok,
                     rd("recs", v.size()) + " " + rd("consumed", v.size() > 0 ? v[0].consumed : 0) + "/" +
                         std::to_string(hsz + 4) + ":" + std::to_string(v.size() > 0 ? v[0].bufsize : 0) + " " +
                         rdb("keepalive", v.size() > 0 ? v[0].state_keepalive : true) + " " +
                         rds("closed", L.closed()));
    }

    // ===== 组 9：入站 chunked 在头部阶段就被拒，chunk 流不会被当成下一条请求解 =====
    {
        const std::string s = "POST /te HTTP/1.1\r\nHost: 127.0.0.1\r\nTransfer-Encoding: chunked\r\n\r\n"
                              "5\r\nhello\r\n0\r\n\r\n" +
                              getreq("/z");
        Loop L;
        L.push(s);
        L.run();
        const std::vector<Rec> &v = L.recs();
        bool ok = v.size() == 1 && v[0].error == 40098 && L.closed() == "bad_request";
        group_result("g9_transfer_encoding_refused_at_header_stage",
                     ok,
                     rd("recs", v.size()) + " " + rd("error", v.size() > 0 ? v[0].error : 0) + " " +
                         rdb("finish", v.size() > 0 ? v[0].finished : false) + " " + rds("closed", L.closed()));
    }

    // ===== 组 10：有意不支持的方法走 405 出口，不进残留逻辑 =====
    {
        int hit = 0;
        std::string read;
        const char *verbs[] = {"PUT", "DELETE", "TRACE", "CONNECT"};
        for (const char *verb : verbs)
        {
            Loop L;
            L.push(std::string(verb) + " /nv HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 3\r\n\r\nabc" +
                   getreq("/never"));
            L.run();
            const std::vector<Rec> &v = L.recs();
            if (v.size() == 1 && v[0].not_allowed && v[0].error == 0 && L.closed() == "method_not_allowed")
                hit++;
            read += std::string(verb) + ":" + std::to_string(v.size()) +
                    (v.empty() ? "" : (v[0].not_allowed ? "na" : "?")) +
                    (v.empty() || v[0].error == 0 ? "" : std::to_string(v[0].error)) + " ";
        }
        group_result("g10_unsupported_methods_take_405_exit", hit == 4, "hit=" + std::to_string(hit) + " " + read);
    }

    // ===== 组 10b：框架表里没有的方法名走 501 出口（同样关连接，请求不进业务）=====
    {
        int hit = 0;
        std::string read;
        const char *verbs[] = {"PATCH", "PROPFIND", "ZGET"};
        for (const char *verb : verbs)
        {
            Loop L;
            L.push(std::string(verb) + " /ui HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 3\r\n\r\nabc" +
                   getreq("/never2"));
            L.run();
            const std::vector<Rec> &v = L.recs();
            if (v.size() == 1 && v[0].unknown && !v[0].not_allowed && v[0].error == 0 &&
                L.closed() == "method_unknown")
                hit++;
            read += std::string(verb) + ":" + std::to_string(v.size()) +
                    (v.empty() ? "" : (v[0].unknown ? "uk" : (v[0].not_allowed ? "na" : "?"))) + " ";
        }
        // 方法名超过 8 个字符仍然按格式错误（400）处理，不落 501——钉住这条界值
        Loop L9;
        L9.push("BADMETHOD /ui HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n" + getreq("/never3"));
        L9.run();
        const std::vector<Rec> &v9 = L9.recs();
        bool len_gate = v9.size() == 1 && v9[0].error == 40058 && !v9[0].unknown && L9.closed() == "bad_request";
        group_result("g10b_unknown_methods_take_501_exit", hit == 3 && len_gate,
                     "hit=" + std::to_string(hit) + " len_gate=" + std::to_string(len_gate ? 1 : 0) + " " + read +
                         "9:" + std::to_string(v9.empty() ? 0u : v9[0].error) + "/" +
                         std::to_string(v9.empty() ? 0u : (v9[0].unknown ? 1u : 0u)));
    }

    // ===== 组 11：请求前一行裸 CRLF 被当成一条空请求（既有语义，钉住读数防漂移）=====
    {
        Loop L;
        L.push(std::string("\r\n") + g1);
        L.run();
        const std::vector<Rec> &v = L.recs();
        bool ok = v.size() == 1 && v[0].consumed == 2 && v[0].header_done && v[0].url.empty() &&
                  L.closed() == "keepalive_false";
        group_result("g11_leading_bare_crlf_parsed_as_empty_request",
                     ok,
                     rd("recs", v.size()) + " " + rd("consumed", v.size() > 0 ? v[0].consumed : 0) + " " +
                         rds("url", v.size() > 0 ? (v[0].url.empty() ? "<empty>" : v[0].url) : "<none>") + " " +
                         rds("closed", L.closed()));
    }

    // ===== 组 12：头部累计超过 16KB 上限必须报错，且报错后不再往下解 =====
    {
        std::string big = "GET /big HTTP/1.1\r\nHost: 127.0.0.1\r\n";
        const std::string pad(200, 'x');
        while (big.size() < (size_t)CONST_HTTP_HEADER_BODY_SIZE + 64)
            big += "X-Pad: " + pad + "\r\n";
        big += "\r\n";
        const std::string all = big + getreq("/after");
        Loop L;
        for (const std::string &s : carve(all, 4096))
            L.push(s);
        L.run();
        unsigned int err = 0;
        size_t at        = 0;
        for (size_t i = 0; i < L.recs().size(); ++i)
        {
            if (L.recs()[i].error > 0)
            {
                err = L.recs()[i].error;
                at  = i + 1;
                break;
            }
        }
        bool ok = err > 0 && at == L.recs().size() && L.closed() == "bad_request";
        group_result("g12_header_over_16k_is_refused",
                     ok,
                     rd("hdr", big.size()) + " " + rd("recs", L.recs().size()) + " " + rd("err", err) + " " +
                         rd("at_rec", at) + " " + rds("closed", L.closed()));
    }

    // ===== 组 13/14：单连接请求数上限的口径（每条请求独占一个段，界值与分段大小无关）=====
    {
#ifdef BENCHMARK
        group_result("g13_cap_answers_exactly_N", false, "BENCHMARK=defined: 上限不启用，本臂要在非压测口径跑");
        group_result("g14_below_cap_connection_stays", false, "BENCHMARK=defined");
#else
        const unsigned int N   = CONST_HTTP1_KEEPALIVE_MAX_REQUESTS;
        const std::string one  = getreq("/k");
        unsigned long long acc = 0;
        {
            Loop L;
            for (unsigned int i = 0; i < N + 50; ++i)
                L.push(one);
            L.run(N + 200);
            for (const Rec &r : L.recs())
                acc += r.consumed;
            bool ok = L.answered() == N && L.closed() == "request_cap" && acc == (unsigned long long)N * one.size();
            group_result("g13_cap_answers_exactly_N",
                         ok,
                         rd("N", N) + " " + rd("answered", L.answered()) + " " + rd("recs", L.recs().size()) + " " +
                             rd("bytes", acc) + "/" + std::to_string((unsigned long long)N * one.size()) + " " +
                             rds("closed", L.closed()));
        }
        {
            const unsigned int want = N - 2;
            Loop L;
            for (unsigned int i = 0; i < want; ++i)
                L.push(one);
            L.run(N + 200);
            bool ok = L.answered() == want && L.closed() == "idle" && L.leftover() == 0;
            group_result("g14_below_cap_connection_stays",
                         ok,
                         rd("want", want) + " " + rd("answered", L.answered()) + " " +
                             rd("leftover", L.leftover()) + " " + rds("closed", L.closed()));
        }
#endif
    }

    // ===== 组 15：逐 7 字节拆包的总不变量——只前进不后退、连接收口时字节全被消费 =====
    {
        const std::string all = postreq("/inv", "alpha=9&beta=8") + g1;
        Loop L;
        for (const std::string &s : carve(all, 7))
            L.push(s);
        L.run(400);
        bool bad = false;
        unsigned long long total = 0;
        std::string urls;
        for (const Rec &r : L.recs())
        {
            if (r.consumed > r.bufsize)
                bad = true;
            total += r.consumed;
            if (r.finished)
                urls += (urls.empty() ? "" : ",") + (r.url.empty() ? "<empty>" : r.url);
        }
        bool ok = !bad && total == all.size() && L.leftover() == 0 && L.closed() == "idle" &&
                  urls == "/inv,/a";
        group_result("g15_seven_byte_segmentation_no_loss_no_overshoot",
                     ok,
                     rd("segs", (all.size() + 6) / 7) + " " + rd("recs", L.recs().size()) + " " +
                         rdb("overshoot", bad) + " " + rd("total", total) + "/" + std::to_string(all.size()) + " " +
                         rd("leftover", L.leftover()) + " " + rds("urls", urls) + " " + rds("closed", L.closed()));
    }

    std::cout << "SUMMARY groups=" << g_groups << " failures=" << g_failures << std::endl;
    return g_failures == 0 ? 0 : 1;
}
