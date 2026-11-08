/*
 * chunked_regression_probe.cpp — HTTP/1 POST chunked 全面回归探针（38 组）
 *
 * 与 h1_consume_probe.cpp 共用 run.sh 换链判据台，输出格式一致：
 *   每组一行 PASS:/FAIL:，末行 SUMMARY groups=N failures=M；全过退出码 0。
 *
 * 直接驱动真实 httpparse::process()，不起服务、不改产品源。覆盖四类：
 *   1) 单包：正常帧（单/多 chunk、chunk-ext、trailer、空 body、multipart、
 *      TE/Expect 的 OWS 与大小写、CL 回归）与畸形拒绝（40229/40230/40231/
 *      40232/40089/40098/40233/403 相关入口）；
 *   2) 分段：把报文拆成多段依次喂入，等价于多次 read_some，验证 CR/LF、chunk
 *      数据、终止块、trailer 跨包时状态机不丢字节；
 *   3) keep-alive：同一 parser 连续两条请求，验证 clear() 把 11 个 chunk 状态复位；
 *   4) 第三方审查复核：5 组 audit_* 判据钉住「看起来像 bug、实测不是」的边界
 *      读数（epilogue 计数口径、boundary 游标、size 行长度边界、截断 multipart、
 *      keep-alive 状态复位），每组注释里写明结论依据。
 *
 * 跑法：bash tests/http1_consume/run_chunked_regression.sh
 *   或  bash tests/http1_consume/run.sh chunkedreg "" tests/http1_consume/chunked_regression_probe.cpp
 *   也可由 all_arms.sh 的 chunkedreg 臂自动带上。
 */
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "http_parse.h"
#include "httppeer.h"
#include "server_localvar.h"

static int g_groups = 0;

static std::string esc(std::string_view s)
{
    std::string o;
    for (unsigned char c : s)
    {
        if (c == '\r') o += "\\r";
        else if (c == '\n') o += "\\n";
        else if (c >= 32 && c < 127) o.push_back((char)c);
        else { char b[8]; std::snprintf(b, sizeof b, "\\x%02x", c); o += b; }
    }
    return o;
}

static const std::string BD = "MPBOUNDARY9";
static const std::string MP =
    "--" + BD + "\r\n"
    "Content-Disposition: form-data; name=\"alpha\"\r\n"
    "\r\n"
    "1\r\n"
    "--" + BD + "--\r\n";

static std::string hexsize(size_t n)
{
    char b[32];
    std::snprintf(b, sizeof b, "%zx", n);
    return b;
}

// 头块。extra 追加额外头行；ver 可改 HTTP 版本；te 可替换 Transfer-Encoding 行
static std::string head(const std::string &ctype, const std::string &extra = "",
                        const std::string &te = "Transfer-Encoding: chunked\r\n",
                        const std::string &ver = "HTTP/1.1")
{
    return "POST /mp " + ver + "\r\n"
           "Host: 127.0.0.1\r\n"
           "Content-Type: " + ctype + "\r\n" + te + extra + "\r\n";
}

static std::string chunked(const std::string &ctype, const std::string &body,
                           const std::string &extra = "",
                           const std::string &te = "Transfer-Encoding: chunked\r\n",
                           const std::string &ver = "HTTP/1.1")
{
    return head(ctype, extra, te, ver) + hexsize(body.size()) + "\r\n" + body + "\r\n0\r\n\r\n";
}

// ---------------- 单包用例 ----------------

struct Case
{
    std::string name;
    std::string wire;
    unsigned int expect_error = 0;
    bool expect_finish        = true;
    bool check_full_consume    = true;
    long long expect_len       = -1;      // -1 不检查
    std::string expect_alpha;             // 空串不检查
    int expect_continue        = -1;       // -1 不检查 / 0 false / 1 true
    int expect_chunked         = -1;       // -1 不检查 / 0 false / 1 true
};

struct OneShot
{
    unsigned int error = 0;
    unsigned int consumed = 0;
    unsigned int bufsize = 0;
    bool finish = false;
    bool keepalive = false;
    bool expect_continue = false;
    bool request_chunked = false;
    unsigned int chunk_state = 0;
    bool payload_closed = false;
    unsigned long long content_length = 0;
    unsigned long long chunk_total = 0;
    std::string alpha;
    std::string beta;
    std::string leftover;
};

static OneShot run_one(const std::string &wire)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse P;
    P.setpeer(peer);
    std::vector<unsigned char> buf(wire.begin(), wire.end());
    unsigned int n = (unsigned int)buf.size();
    P.process(buf.data(), n);

    OneShot r;
    r.error            = P.error;
    r.consumed          = P.readoffset;
    r.bufsize           = n;
    r.finish            = peer->isfinish;
    r.keepalive         = peer->state.keepalive;
    r.expect_continue  = P.expect_continue;
    r.request_chunked  = P.request_chunked;
    r.chunk_state      = P.chunk_state;
    r.payload_closed   = P.chunked_payload_closed;
    r.content_length   = peer->content_length;
    r.chunk_total      = P.chunk_total;
    r.alpha             = peer->post["alpha"].to_string();
    r.beta              = peer->post["beta"].to_string();
    if (r.consumed < n)
        r.leftover.assign((const char *)(buf.data() + r.consumed), n - r.consumed);
    return r;
}

static bool check_case(const Case &c, int &failures)
{
    OneShot r = run_one(c.wire);
    bool ok = true;
    if (r.error != c.expect_error) ok = false;
    if (r.finish != c.expect_finish) ok = false;
    if (c.expect_error == 0 && c.check_full_consume && r.consumed != r.bufsize) ok = false;
    if (c.expect_len >= 0 && r.content_length != (unsigned long long)c.expect_len) ok = false;
    if (!c.expect_alpha.empty() && r.alpha != c.expect_alpha) ok = false;
    if (c.expect_continue >= 0 && r.expect_continue != (bool)c.expect_continue) ok = false;
    if (c.expect_chunked >= 0 && r.request_chunked != (bool)c.expect_chunked) ok = false;

    ++g_groups;
    std::cout << (ok ? "PASS: " : "FAIL: ") << c.name << " ["
              << "error=" << r.error << "(exp " << c.expect_error << ")"
              << " finish=" << r.finish
              << " consumed=" << r.consumed << "/" << r.bufsize
              << " len=" << r.content_length
              << " alpha=\"" << esc(r.alpha) << "\""
              << " expect_cont=" << r.expect_continue
              << " chunked=" << r.request_chunked
              << " keepalive=" << r.keepalive
              << " leftover=\"" << esc(r.leftover) << "\""
              << "]" << std::endl;
    if (!ok) ++failures;
    return ok;
}

// ---------------- 分段（多次 read_some）用例 ----------------

struct SegOutcome
{
    int dispatch = 0;
    unsigned int error = 0;
    unsigned int last_consumed = 0;
    unsigned int last_size = 0;
    unsigned long long content_length = 0;
    std::string alpha;
    bool ever_wrong_consume = false;
};

static SegOutcome run_segments(const std::vector<std::string> &segs)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse P;
    P.setpeer(peer);
    SegOutcome out;
    for (const auto &s : segs)
    {
        std::vector<unsigned char> buf(s.begin(), s.end());
        unsigned int n = (unsigned int)buf.size();
        P.process(buf.data(), n);
        out.last_consumed = P.readoffset;
        out.last_size = n;
        if (P.error > 0) { out.error = P.error; return out; }
        if (!peer->isfinish && P.readoffset != n) out.ever_wrong_consume = true;
        if (peer->isfinish)
        {
            ++out.dispatch;
            out.content_length = peer->content_length;
            out.alpha = peer->post["alpha"].to_string();
            return out;
        }
    }
    return out;
}

static void report_seg(const char *name, const std::vector<std::string> &segs,
                       unsigned long long expect_len, const std::string &expect_alpha,
                       int &failures)
{
    SegOutcome o = run_segments(segs);
    bool ok = o.dispatch == 1 && o.error == 0 && !o.ever_wrong_consume &&
              o.last_consumed == o.last_size &&
              o.content_length == expect_len && o.alpha == expect_alpha;
    ++g_groups;
    std::cout << (ok ? "PASS: " : "FAIL: ") << name << " ["
              << "segs=" << segs.size()
              << " dispatch=" << o.dispatch
              << " error=" << o.error
              << " last=" << o.last_consumed << "/" << o.last_size
              << " len=" << o.content_length
              << " alpha=\"" << esc(o.alpha) << "\""
              << " wrong_consume=" << o.ever_wrong_consume
              << "]" << std::endl;
    if (!ok) ++failures;
}

// ---------------- keep-alive：同一 parser 连续两条请求 ----------------

static void report_keepalive(const char *name, const std::string &w1, const std::string &w2,
                             unsigned long long expect_len, const std::string &expect_alpha,
                             int &failures)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse P;
    P.setpeer(peer);

    std::vector<unsigned char> b1(w1.begin(), w1.end());
    P.process(b1.data(), (unsigned int)b1.size());
    unsigned int c1 = P.readoffset, e1 = P.error;
    bool f1 = peer->isfinish;

    // server.cpp:3259-3262：派发后两个对象都 clear，再复用同一 parser
    P.clear();
    peer->clear();

    std::vector<unsigned char> b2(w2.begin(), w2.end());
    P.process(b2.data(), (unsigned int)b2.size());
    unsigned int c2 = P.readoffset, e2 = P.error;
    bool f2 = peer->isfinish;
    unsigned long long l2 = peer->content_length;
    std::string a2 = peer->post["alpha"].to_string();

    bool ok = e1 == 0 && f1 && c1 == w1.size() &&
              e2 == 0 && f2 && c2 == w2.size() &&
              l2 == expect_len && a2 == expect_alpha;
    ++g_groups;
    std::cout << (ok ? "PASS: " : "FAIL: ") << name << " ["
              << "req1=" << c1 << "/" << w1.size() << " e=" << e1 << " f=" << f1
              << " req2=" << c2 << "/" << w2.size() << " e=" << e2 << " f=" << f2
              << " len2=" << l2 << " alpha2=\"" << esc(a2) << "\""
              << "]" << std::endl;
    if (!ok) ++failures;
}

// ---------------- 第三方审查 5 点复核判据 ----------------
// 这些读数钉住「看起来像 bug、实测不是」的边界，防止以后被误改或再次误判。
static void audit_result(const char *name, bool ok, const std::string &reading, int &failures)
{
    ++g_groups;
    std::cout << (ok ? "PASS: " : "FAIL: ") << name << " [" << reading << "]" << std::endl;
    if (!ok) ++failures;
}

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code ec;
    std::filesystem::create_directories("./temp/h1_consume/out/", ec);
    gv.temp_path = "./temp/h1_consume/out/";

    int failures = 0;
    const std::string URLENC = "application/x-www-form-urlencoded";
    const std::string MPCT = "multipart/form-data; boundary=" + BD;

    std::vector<Case> cases;

    // ---- 正常帧 ----
    cases.push_back({"ok_single_chunk_urlencoded", chunked(URLENC, "alpha=1"), 0, true, true, 7, "1", -1, 1});
    cases.push_back({"ok_empty_chunked_body",
                     head(URLENC) + "0\r\n\r\n", 0, true, true, 0, "", -1, 1});
    cases.push_back({"ok_multipart", chunked(MPCT, MP), 0, true, true, (long long)MP.size(), "1", -1, 1});
    cases.push_back({"ok_chunk_ext",
                     head(URLENC) + "7;foo=bar;baz=\"x y\"\r\nalpha=1\r\n0\r\n\r\n",
                     0, true, true, 7, "1", -1, 1});
    cases.push_back({"ok_trailer",
                     head(URLENC) + "7\r\nalpha=1\r\n0\r\nX-Checksum: abc\r\n\r\n",
                     0, true, true, 7, "1", -1, 1});
    cases.push_back({"ok_multi_chunk",
                     head(URLENC) + "3\r\nalp\r\n2\r\nha\r\n2\r\n=1\r\n0\r\n\r\n",
                     0, true, true, 7, "1", -1, 1});
    cases.push_back({"ok_TE_ows",
                     chunked(URLENC, "alpha=1", "", "Transfer-Encoding:  chunked \t\r\n"),
                     0, true, true, 7, "1", -1, 1});
    cases.push_back({"ok_TE_uppercase",
                     chunked(URLENC, "alpha=1", "", "TRANSFER-ENCODING: Chunked\r\n"),
                     0, true, true, 7, "1", -1, 1});
    cases.push_back({"ok_Expect_ows",
                     chunked(URLENC, "alpha=1", "Expect: 100-continue \t\r\n"),
                     0, true, true, 7, "1", 1, 1});
    cases.push_back({"ok_CL_regression",
                     "POST /mp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: " + URLENC +
                         "\r\nContent-Length: 7\r\n\r\nalpha=1",
                     0, true, true, 7, "1", 0, 0});

    // ---- 畸形 / 拒绝 ----
    cases.push_back({"err_invalid_chunk_size",
                     head(URLENC) + "Z\r\nalpha=1\r\n0\r\n\r\n", 40229, false, false});
    cases.push_back({"err_bare_lf_in_size_line",
                     head(URLENC) + "7\nalpha=1\r\n0\r\n\r\n", 40231, false, false});
    cases.push_back({"err_missing_cr_after_data",
                     head(URLENC) + "7\r\nalpha=1X0\r\n\r\n", 40230, false, false});
    cases.push_back({"err_declared_size_over_32m",
                     head(URLENC) + "FFFFFFF\r\n", 40089, false, false});
    cases.push_back({"err_TE_plus_CL",
                     chunked(URLENC, "alpha=1", "Content-Length: 7\r\n"), 40233, false, false});
    cases.push_back({"err_duplicate_TE",
                     chunked(URLENC, "alpha=1", "Transfer-Encoding: chunked\r\n"), 40233, false, false});
    cases.push_back({"err_TE_gzip",
                     chunked(URLENC, "alpha=1", "", "Transfer-Encoding: gzip\r\n"), 40098, false, false});
    cases.push_back({"err_http10_TE",
                     chunked(URLENC, "alpha=1", "", "Transfer-Encoding: chunked\r\n", "HTTP/1.0"),
                     40098, false, false});
    cases.push_back({"err_trailer_without_colon",
                     head(URLENC) + "7\r\nalpha=1\r\n0\r\nNoColonHere\r\n\r\n", 40232, false, false});
    cases.push_back({"err_trailer_bare_lf",
                     head(URLENC) + "7\r\nalpha=1\r\n0\r\nX-A: b\n\n", 40232, false, false});
    {
        std::string long_trailer(16385, 'a');
        cases.push_back({"err_trailer_over_16k",
                         head(URLENC) + "7\r\nalpha=1\r\n0\r\nX-A: " + long_trailer + "\r\n\r\n",
                         40232, false, false});
    }
    cases.push_back({"err_long_size_line",
                     head(URLENC) + "7;" + std::string(1100, 'x') + "\r\nalpha=1\r\n0\r\n\r\n",
                     40229, false, false});

    // GET 带 chunked body：派发但必须关连接，避免残留被当下一请求
    cases.push_back({"ok_GET_chunked_closes",
                     "GET /mp HTTP/1.1\r\nHost: 127.0.0.1\r\nTransfer-Encoding: chunked\r\n\r\n"
                     "7\r\nalpha=1\r\n0\r\n\r\n",
                     0, true, false});
    // 头值后面带空白但不含 ';'：不是合法 chunk-ext，按畸形拒绝
    cases.push_back({"err_chunk_size_bare_trailing_space",
                     head(URLENC) + "7 \r\nalpha=1\r\n0\r\n\r\n", 40229, false, false});
    // Expect 与空 chunked body 同包：不应影响正常收尾
    cases.push_back({"ok_Expect_with_empty_chunked",
                     head(URLENC, "Expect: 100-continue\r\n") + "0\r\n\r\n",
                     0, true, true, 0, "", 1, 1});

    std::cout << "=== 单包用例 ===" << std::endl;
    for (const auto &c : cases) check_case(c, failures);

    std::cout << "\n=== 分段用例（模拟多次 read_some） ===" << std::endl;
    {
        std::string w = chunked(MPCT, MP);
        unsigned int h = (unsigned int)head(MPCT).size();
        unsigned int szline = (unsigned int)hexsize(MP.size()).size();
        std::vector<std::string> segs = {
            w.substr(0, h),                     // 头块
            w.substr(h, szline),               // size 行的十六进制部分
            w.substr(h + szline, 2),           // \r\n
            w.substr(h + szline + 2, 20),      // body 前 20 字节
            w.substr(h + szline + 22)          // 剩余 body + 帧尾
        };
        report_seg("seg_multipart_5parts", segs, MP.size(), "1", failures);
    }
    {
        std::string w = chunked(URLENC, "alpha=1");
        std::vector<std::string> segs;
        for (char c : w) segs.push_back(std::string(1, c));
        report_seg("seg_urlencoded_bytewise", segs, 7, "1", failures);
    }
    {
        std::string w = head(URLENC) +
                        "3\r\nalp\r\n2\r\nha\r\n2\r\n=1\r\n0\r\nX-Checksum: abc\r\n\r\n";
        std::vector<std::string> segs = {
            w.substr(0, 20), w.substr(20, 30), w.substr(50, 25), w.substr(75)
        };
        report_seg("seg_multichunk_trailer", segs, 7, "1", failures);
    }

    std::cout << "\n=== keep-alive 连续请求（同一 parser 复用） ===" << std::endl;
    report_keepalive("ka_two_urlencoded_chunked",
                     chunked(URLENC, "alpha=1"), chunked(URLENC, "alpha=2"),
                     7, "2", failures);
    report_keepalive("ka_urlencoded_then_multipart",
                     chunked(URLENC, "alpha=1"), chunked(MPCT, MP),
                     MP.size(), "1", failures);
    report_keepalive("ka_multipart_then_urlencoded",
                     chunked(MPCT, MP), chunked(URLENC, "alpha=3"),
                     7, "3", failures);
    report_keepalive("ka_two_multipart",
                     chunked(MPCT, MP), chunked(MPCT, MP),
                     MP.size(), "1", failures);
    report_keepalive("ka_two_with_trailers",
                     head(URLENC) + "7\r\nalpha=1\r\n0\r\nX-A: b\r\n\r\n",
                     head(URLENC) + "7\r\nalpha=4\r\n0\r\nX-B: c\r\n\r\n",
                     7, "4", failures);

    std::cout << "\n=== 第三方审查复核判据（5 组，钉住实测读数） ===" << std::endl;

    // 审查点 5：http1pre 每连接复用，但每请求 clear()；chunk 状态不得串到下一条
    {
        auto peer = std::make_shared<http::httppeer>();
        http::httpparse P;
        P.setpeer(peer);
        std::string w1 = head(URLENC) + "7\r\nalpha=1\r\n0\r\n\r\n";
        std::vector<unsigned char> b1(w1.begin(), w1.end());
        P.process(b1.data(), (unsigned int)b1.size());
        bool req1 = P.error == 0 && peer->isfinish && peer->content_length == 7 && P.chunk_state == 5;
        P.clear();
        peer->clear();
        bool cleared = P.chunk_state == 0 && P.chunk_total == 0 && !P.chunked_payload_closed;
        std::string w2 = "GET /next HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
        std::vector<unsigned char> b2(w2.begin(), w2.end());
        P.process(b2.data(), (unsigned int)b2.size());
        bool req2 = P.error == 0 && peer->isfinish && peer->content_length == 0 &&
                    peer->url == "/next" && P.readoffset == w2.size();
        audit_result("audit_keepalive_state_reset", req1 && cleared && req2,
                     "req1{len=7 state=5} after_clear{state=0 total=0 closed=0} "
                     "req2{len=0 url=/next consumed=39/39}", failures);
    }

    // 审查点 1：multipart epilogue 属于 HTTP 消息体，chunked 与 CL 两条路径都必须计入
    {
        const unsigned long long expect_len = MP.size() + 5;
        std::string wc = head(MPCT) + hexsize(MP.size()) + "\r\n" + MP + "\r\n" +
                         "5\r\nXXXXX\r\n0\r\n\r\n";
        OneShot a = run_one(wc);
        std::string body = MP + "XXXXX";
        std::string wl = "POST /mp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: " + MPCT +
                        "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
        OneShot b = run_one(wl);
        // 同一 chunk 内、闭合 boundary 之后直接追加 epilogue 的等价形态
        std::string ws = head(MPCT) + hexsize(MP.size() + 5) + "\r\n" + MP + "XXXXX" +
                         "\r\n0\r\n\r\n";
        OneShot c = run_one(ws);
        bool ok = a.error == 0 && a.finish && a.consumed == a.bufsize &&
                  a.content_length == expect_len && a.chunk_total == expect_len &&
                  a.payload_closed && a.alpha == "1" &&
                  b.error == 0 && b.finish && b.content_length == expect_len && b.alpha == "1" &&
                  c.error == 0 && c.finish && c.content_length == expect_len && c.alpha == "1" &&
                  a.content_length == b.content_length && a.content_length == c.content_length;
        audit_result("audit_epilogue_counted_consistently", ok,
                     "extra_chunk_len=" + std::to_string(a.content_length) +
                         " same_chunk_len=" + std::to_string(c.content_length) +
                         " CL_len=" + std::to_string(b.content_length) +
                         " expect=" + std::to_string(expect_len) +
                         " payload_closed=" + std::to_string(a.payload_closed ? 1 : 0) +
                         " alpha=" + a.alpha, failures);
    }

    // 审查点 2：非终止 boundary 后 field_offset = j+1（少跳一个 \n），下游容忍，字段仍正确
    {
        std::string mp2 =
            "--" + BD + "\r\n"
            "Content-Disposition: form-data; name=\"alpha\"\r\n"
            "\r\n1\r\n"
            "--" + BD + "\r\n"
            "Content-Disposition: form-data; name=\"beta\"\r\n"
            "\r\n2\r\n"
            "--" + BD + "--\r\n";
        std::string cl = "POST /mp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: " + MPCT +
                        "\r\nContent-Length: " + std::to_string(mp2.size()) + "\r\n\r\n" + mp2;
        std::string ch = head(MPCT) + hexsize(mp2.size()) + "\r\n" + mp2 + "\r\n0\r\n\r\n";
        OneShot a = run_one(cl);
        OneShot b = run_one(ch);

        auto peer = std::make_shared<http::httppeer>();
        http::httpparse P;
        P.setpeer(peer);
        size_t cut = head(MPCT).size() + 10;
        std::vector<std::string> segs = {ch.substr(0, cut), ch.substr(cut, 100), ch.substr(cut + 100)};
        for (const auto &s : segs)
        {
            std::vector<unsigned char> bb(s.begin(), s.end());
            P.process(bb.data(), (unsigned int)bb.size());
        }
        std::string sa = peer->post["alpha"].to_string();
        std::string sb = peer->post["beta"].to_string();
        bool seg = P.error == 0 && peer->isfinish && sa == "1" && sb == "2";
        bool ok = a.error == 0 && a.alpha == "1" && a.beta == "2" &&
                  b.error == 0 && b.alpha == "1" && b.beta == "2" && seg;
        audit_result("audit_two_parts_boundary_cursor", ok,
                     "CL{alpha=" + a.alpha + " beta=" + a.beta + "} chunked{alpha=" + b.alpha +
                         " beta=" + b.beta + "} segmented{alpha=" + sa + " beta=" + sb + "}", failures);
    }

    // 审查点 3：CR/LF 不计入 chunk_line_bytes，实际边界比标称 1024 多 1~2 字节
    {
        bool ok = true;
        std::string reading;
        for (int n : {1021, 1022, 1023})
        {
            std::string w = head(URLENC) + "7;" + std::string(n, 'x') +
                            "\r\nalpha=1\r\n0\r\n\r\n";
            OneShot r = run_one(w);
            unsigned int expected = (n == 1023) ? 40229u : 0u;
            if (r.error != expected) ok = false;
            reading += "non_cr=" + std::to_string(n + 2) + ":error=" + std::to_string(r.error) + " ";
        }
        audit_result("audit_size_line_length_boundary", ok, reading, failures);
    }

    // 审查点 4：multipart 截断（无闭合 boundary）时 chunked 与 CL 行为完全一致
    {
        std::string trunc =
            "--" + BD + "\r\n"
            "Content-Disposition: form-data; name=\"alpha\"\r\n"
            "\r\n1\r\n";
        std::string wc = head(MPCT) + hexsize(trunc.size()) + "\r\n" + trunc + "\r\n0\r\n\r\n";
        std::string wl = "POST /mp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: " + MPCT +
                        "\r\nContent-Length: " + std::to_string(trunc.size()) + "\r\n\r\n" + trunc;
        OneShot a = run_one(wc);
        OneShot b = run_one(wl);
        bool ok = a.error == 0 && a.finish && a.content_length == trunc.size() && a.alpha.empty() &&
                  b.error == 0 && b.finish && b.content_length == trunc.size() && b.alpha.empty() &&
                  a.finish == b.finish && a.content_length == b.content_length;
        audit_result("audit_truncated_multipart_matches_cl", ok,
                     "chunked{finish=" + std::to_string(a.finish ? 1 : 0) +
                         " len=" + std::to_string(a.content_length) + " alpha=<empty>} CL{finish=" +
                         std::to_string(b.finish ? 1 : 0) + " len=" + std::to_string(b.content_length) +
                         " alpha=<empty>}", failures);
    }

    std::cout << "SUMMARY groups=" << g_groups << " failures=" << failures << std::endl;
    return failures == 0 ? 0 : 1;
}
