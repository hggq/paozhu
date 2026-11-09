// tests/multipart_part_headers/part_header_probe.cpp
//
// 单元探针：直接调用 h1 / h2 两侧的 post_multipart_itemcontent()，
// 把 multipart part 头解析的四个既有缺陷固化成可判定用例（不改产品代码）。
//
// 说明：
//  - 喂入的 field_item 就是 multipart 扫描器在真实请求中交给该函数的那段字节：
//    “part 头若干行 + 空行 + 正文”，不含前导 boundary 行、不含尾部 boundary。
//  - 文件域用 isfilefull=false 调用：正文同样落盘（fwrite 在 finalize 之前），
//    但临时文件名不会被清空，便于本探针直接读取原始字节。
//  - 文本域用 isfilefull=true 调用：函数才会把整段值写进 peer->post。
//  - 临时落盘目录由 argv[1] 指定（默认 /tmp/pz_mp_part_headers）。
//
// 输出：逐条 PASS/FAIL，失败打印期望 vs 实际（含前 48 字节 hexdump），
//       末尾 SUMMARY total=N pass=X fail=Y；有失败时进程退出码 1。

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <fstream>
#include <functional>

#include "http_parse.h"
#include "http2_parse.h"
#include "httppeer.h"
#include "server_localvar.h"

namespace fs = std::filesystem;

namespace
{
int g_total = 0;
int g_pass  = 0;
int g_fail  = 0;

// ---- 文本可视化与十六进制 -------------------------------------------------

std::string vis(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        switch (c)
        {
        case '\r': out += "\\r"; break;
        case '\n': out += "\\n"; break;
        case '\t': out += "\\t"; break;
        default: out.push_back(c); break;
        }
    }
    return out;
}

std::string hexdump(std::string_view s, size_t max = 48)
{
    std::string out;
    for (size_t i = 0; i < s.size() && i < max; ++i)
    {
        char buf[4];
        std::snprintf(buf, sizeof(buf), "%02x ", static_cast<unsigned char>(s[i]));
        out += buf;
    }
    if (!out.empty())
    {
        out.pop_back();
    }
    if (s.size() > max)
    {
        out += " ...";
    }
    return out;
}

std::string value_desc(const std::string &s)
{
    return "\"" + vis(s) + "\" (" + std::to_string(s.size()) + " bytes) hex=" + hexdump(s);
}

// ---- 结果记录 -------------------------------------------------------------

void report(const std::string &side, const std::string &name, bool ok,
            const std::string &expected, const std::string &actual)
{
    ++g_total;
    if (ok)
    {
        ++g_pass;
        std::printf("[PASS] %-2s %s\n", side.c_str(), name.c_str());
    }
    else
    {
        ++g_fail;
        std::printf("[FAIL] %-2s %s\n", side.c_str(), name.c_str());
        std::printf("       expected: %s\n", expected.c_str());
        std::printf("       actual  : %s\n", actual.c_str());
    }
    std::fflush(stdout);
}

std::string error_desc(unsigned int error, bool is_conn, bool show_conn)
{
    std::string out = "error=" + std::to_string(error);
    if (show_conn)
    {
        out += " error_is_conn=";
        out += (is_conn ? "1" : "0");
    }
    return out;
}

std::string actual_desc(const struct Result &r, bool show_conn);

// ---- 正文扫描（epilogue / preamble）用例的独立计数 -------------------------
int g2_total = 0;
int g2_pass  = 0;
int g2_fail  = 0;

void report2(const std::string &side, const std::string &name, bool ok,
             const std::string &expected, const std::string &actual)
{
    ++g2_total;
    if (ok)
    {
        ++g2_pass;
        std::printf("[PASS] %-2s %s\n", side.c_str(), name.c_str());
    }
    else
    {
        ++g2_fail;
        std::printf("[FAIL] %-2s %s\n", side.c_str(), name.c_str());
        std::printf("       expected: %s\n", expected.c_str());
        std::printf("       actual  : %s\n", actual.c_str());
    }
    std::fflush(stdout);
}

// ---- 执行一侧 -------------------------------------------------------------

struct Result
{
    unsigned int error         = 0;
    bool         error_is_conn = true;
    unsigned int h2_code       = 0;
    std::string  field_name;
    std::string  filename;
    std::string  mimetype;
    std::string  body;
    bool         body_available = false;
};

std::string read_file(const std::string &path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        return std::string();
    }
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

Result run_h1(const std::string &part_item, bool is_file)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse parser;
    parser.setpeer(peer);
    parser.temp_post_data             = std::make_unique<http::HTTP_POST_DATA_T>();
    parser.temp_post_data->field_item = part_item;

    // 文本域必须 isfilefull=true 才会写入 peer->post；文件域用 false 保留 temp_filename
    parser.post_multipart_itemcontent(!is_file);

    Result r;
    r.error      = parser.error;
    r.field_name = parser.temp_post_data->field_name;
    r.filename   = parser.temp_post_data->filename;
    r.mimetype   = parser.temp_post_data->mimetype;

    if (is_file)
    {
        if (parser.temp_post_data->fp)
        {
            std::fflush(parser.temp_post_data->fp.get());
        }
        if (!parser.temp_post_data->temp_filename.empty())
        {
            r.body            = read_file(parser.temp_post_data->temp_filename);
            r.body_available  = true;
        }
        parser.temp_post_data->fp.reset();
        if (!parser.temp_post_data->temp_filename.empty())
        {
            std::remove(parser.temp_post_data->temp_filename.c_str());
        }
        parser.temp_post_data->temp_filename.clear();
    }
    else
    {
        r.body           = peer->post["textfield"].as_string();
        r.body_available = true;
    }
    return r;
}

Result run_h2(const std::string &part_item, bool is_file)
{
    auto peer = std::make_shared<http::httppeer>();
    http::http2parse parser;
    http::HTTP2_POST_DATA_T pd;
    pd.peer       = peer;
    pd.stream_id  = 1;
    pd.field_item = part_item;

    parser.post_multipart_itemcontent(pd, !is_file);

    Result r;
    r.error         = parser.error;
    r.error_is_conn = parser.error_is_conn;
    r.h2_code       = parser.h2_error_code;
    r.field_name    = pd.field_name;
    r.filename      = pd.filename;
    r.mimetype      = pd.mimetype;

    if (is_file)
    {
        if (pd.fp)
        {
            std::fflush(pd.fp.get());
        }
        if (!pd.temp_filename.empty())
        {
            r.body           = read_file(pd.temp_filename);
            r.body_available = true;
        }
        pd.fp.reset();
        if (!pd.temp_filename.empty())
        {
            std::remove(pd.temp_filename.c_str());
        }
    }
    else
    {
        r.body           = peer->post["textfield"].as_string();
        r.body_available = true;
    }
    return r;
}

std::string actual_desc(const Result &r, bool show_conn)
{
    std::string out = error_desc(r.error, r.error_is_conn, show_conn);
    if (show_conn)
    {
        out += " h2_code=0x" + std::to_string(r.h2_code);
    }
    out += " name=\"" + r.field_name + "\"";
    out += " filename=\"" + r.filename + "\"";
    out += " mimetype=\"" + r.mimetype + "\"";
    out += " body=" + (r.body_available ? value_desc(r.body) : std::string("<unavailable>"));
    return out;
}

// ---- 正文扫描用例：驱动完整的 post_multipart_formdata 循环 -----------------
// 上面的用例只喂单个 part 给 post_multipart_itemcontent；epilogue / preamble
// 的问题出在外层扫描循环里，必须从 append_post_body（h1）与
// post_data_process（h2）进入，喂整段 body。

struct BodyResult
{
    unsigned int error           = 0;
    bool         error_is_conn   = true;
    unsigned int h2_code         = 0;
    std::string  post_json;
    // h1 的请求完成信号；h2 由 END_STREAM 决定，本探针不喂流事件，恒为 false 且不参与判定
    bool         isfinish        = false;
    bool         closed          = false;
    bool         waiting_closing = false;
    unsigned int dashes_seen     = 0;
};

BodyResult run_body_h1(const std::string &body)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse parser;
    parser.setpeer(peer);
    parser.posttype       = 2;// multipart
    parser.temp_post_data = std::make_unique<http::HTTP_POST_DATA_T>();
    parser.temp_post_data->boundary = "X";
    parser.append_post_body(reinterpret_cast<const unsigned char *>(body.data()),
                            static_cast<unsigned int>(body.size()), true);

    BodyResult r;
    r.error           = parser.error;
    r.post_json       = peer->post.to_json();
    r.isfinish        = peer->isfinish;
    r.closed          = parser.temp_post_data->multipart_closed;
    r.waiting_closing = parser.temp_post_data->boundary_hit_waiting_closing;
    r.dashes_seen     = parser.temp_post_data->closing_dashes_seen;
    parser.temp_post_data.reset();
    return r;
}

BodyResult run_body_h2(const std::string &body)
{
    auto peer = std::make_shared<http::httppeer>();
    http::http2parse parser;
    http::HTTP2_POST_DATA_T pd;
    pd.peer      = peer;
    pd.stream_id = 1;
    pd.posttype  = 2;// multipart
    pd.boundary  = "X";
    pd.content   = std::string_view(body);

    parser.post_data_process(pd, 1);

    BodyResult r;
    r.error           = parser.error;
    r.error_is_conn   = parser.error_is_conn;
    r.h2_code         = parser.h2_error_code;
    r.post_json       = peer->post.to_json();
    r.closed          = pd.multipart_closed;
    r.waiting_closing = pd.boundary_hit_waiting_closing;
    r.dashes_seen     = pd.closing_dashes_seen;
    pd.fp.reset();
    return r;
}

// h1 走 chunked 请求体时，外层循环在终止 boundary 处就 break 并置 chunked_payload_closed
BodyResult run_body_h1_chunked(const std::string &body)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse parser;
    parser.setpeer(peer);
    parser.request_chunked = true;
    parser.posttype        = 2;
    parser.temp_post_data  = std::make_unique<http::HTTP_POST_DATA_T>();
    parser.temp_post_data->boundary = "X";
    parser.append_post_body(reinterpret_cast<const unsigned char *>(body.data()),
                            static_cast<unsigned int>(body.size()), false);

    BodyResult r;
    r.error           = parser.error;
    r.post_json       = peer->post.to_json();
    r.isfinish        = peer->isfinish;
    r.closed          = parser.temp_post_data->multipart_closed;
    r.waiting_closing = parser.temp_post_data->boundary_hit_waiting_closing;
    r.dashes_seen     = parser.temp_post_data->closing_dashes_seen;
    parser.temp_post_data.reset();
    return r;
}

// h1 的两次喂入：闭合 boundary 跨缓冲时的真实场景
BodyResult run_body_h1_two(const std::string &b1, const std::string &b2)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse parser;
    parser.setpeer(peer);
    parser.posttype       = 2;// multipart
    parser.temp_post_data = std::make_unique<http::HTTP_POST_DATA_T>();
    parser.temp_post_data->boundary = "X";
    parser.append_post_body(reinterpret_cast<const unsigned char *>(b1.data()),
                            static_cast<unsigned int>(b1.size()), false);
    parser.append_post_body(reinterpret_cast<const unsigned char *>(b2.data()),
                            static_cast<unsigned int>(b2.size()), true);

    BodyResult r;
    r.error           = parser.error;
    r.post_json       = peer->post.to_json();
    r.isfinish        = peer->isfinish;
    r.closed          = parser.temp_post_data->multipart_closed;
    r.waiting_closing = parser.temp_post_data->boundary_hit_waiting_closing;
    r.dashes_seen     = parser.temp_post_data->closing_dashes_seen;
    parser.temp_post_data.reset();
    return r;
}

// h2 的两帧：终止 boundary 在第一帧、闭合 "--" 在第二帧
BodyResult run_body_h2_frames(const std::string &b1, const std::string &b2)
{
    auto peer = std::make_shared<http::httppeer>();
    http::http2parse parser;
    http::HTTP2_POST_DATA_T pd;
    pd.peer      = peer;
    pd.stream_id = 1;
    pd.posttype  = 2;
    pd.boundary  = "X";

    pd.content = std::string_view(b1);
    parser.post_data_process(pd, 0);
    pd.content = std::string_view(b2);
    parser.post_data_process(pd, 1);

    BodyResult r;
    r.error           = parser.error;
    r.error_is_conn   = parser.error_is_conn;
    r.h2_code         = parser.h2_error_code;
    r.post_json       = peer->post.to_json();
    r.closed          = pd.multipart_closed;
    r.waiting_closing = pd.boundary_hit_waiting_closing;
    r.dashes_seen     = pd.closing_dashes_seen;
    pd.fp.reset();
    return r;
}

// 只喂第一包（is_final=false / islast_pack=0），用来读挂起中的闭合判定状态
BodyResult run_body_h1_first(const std::string &b1)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse parser;
    parser.setpeer(peer);
    parser.posttype       = 2;
    parser.temp_post_data = std::make_unique<http::HTTP_POST_DATA_T>();
    parser.temp_post_data->boundary = "X";
    parser.append_post_body(reinterpret_cast<const unsigned char *>(b1.data()),
                            static_cast<unsigned int>(b1.size()), false);

    BodyResult r;
    r.error           = parser.error;
    r.post_json       = peer->post.to_json();
    r.isfinish        = peer->isfinish;
    r.closed          = parser.temp_post_data->multipart_closed;
    r.waiting_closing = parser.temp_post_data->boundary_hit_waiting_closing;
    r.dashes_seen     = parser.temp_post_data->closing_dashes_seen;
    parser.temp_post_data.reset();
    return r;
}

BodyResult run_body_h2_first(const std::string &b1)
{
    auto peer = std::make_shared<http::httppeer>();
    http::http2parse parser;
    http::HTTP2_POST_DATA_T pd;
    pd.peer      = peer;
    pd.stream_id = 1;
    pd.posttype  = 2;
    pd.boundary  = "X";
    pd.content   = std::string_view(b1);
    parser.post_data_process(pd, 0);

    BodyResult r;
    r.error           = parser.error;
    r.error_is_conn   = parser.error_is_conn;
    r.h2_code         = parser.h2_error_code;
    r.post_json       = peer->post.to_json();
    r.closed          = pd.multipart_closed;
    r.waiting_closing = pd.boundary_hit_waiting_closing;
    r.dashes_seen     = pd.closing_dashes_seen;
    pd.fp.reset();
    return r;
}

// keep-alive：同一 parser 与同一 temp_post_data 跨两条请求，中间只走 clear()。
// 产品里 server.cpp 的 keep-alive 分支就是这个形状（http1pre->clear(); peer->clear();），
// unique_ptr 本身不重建，所以闭合状态必须由 clear() 归零。
BodyResult run_body_h1_reuse(const std::string &b1, const std::string &b2)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse parser;
    parser.setpeer(peer);
    parser.posttype       = 2;
    parser.temp_post_data = std::make_unique<http::HTTP_POST_DATA_T>();
    parser.temp_post_data->boundary = "X";
    parser.append_post_body(reinterpret_cast<const unsigned char *>(b1.data()),
                            static_cast<unsigned int>(b1.size()), true);

    parser.clear();
    peer->clear();
    // 下一条请求的头阶段会重新填回这两项
    parser.posttype               = 2;
    parser.temp_post_data->boundary = "X";
    parser.append_post_body(reinterpret_cast<const unsigned char *>(b2.data()),
                            static_cast<unsigned int>(b2.size()), true);

    BodyResult r;
    r.error           = parser.error;
    r.post_json       = peer->post.to_json();
    r.isfinish        = peer->isfinish;
    r.closed          = parser.temp_post_data->multipart_closed;
    r.waiting_closing = parser.temp_post_data->boundary_hit_waiting_closing;
    r.dashes_seen     = parser.temp_post_data->closing_dashes_seen;
    parser.temp_post_data.reset();
    return r;
}

// 三喂：上一包停在分隔符中间（pre_content 留有 1 / 2 / >2 字节），第二包恰好在 boundary
// 文本后只剩一个 '-' 就用完。pre_content 孪生分支的"缓冲剩 1 字节"只有这种形状能到达；
// mid 读第二包后的挂起态，返回值读第三包后的终态。
BodyResult run_body_h1_three(const std::string &bd, const std::string &b1, const std::string &b2,
                             const std::string &b3, BodyResult &mid)
{
    auto peer = std::make_shared<http::httppeer>();
    http::httpparse parser;
    parser.setpeer(peer);
    parser.posttype       = 2;
    parser.temp_post_data = std::make_unique<http::HTTP_POST_DATA_T>();
    parser.temp_post_data->boundary = bd;
    parser.append_post_body(reinterpret_cast<const unsigned char *>(b1.data()),
                            static_cast<unsigned int>(b1.size()), false);
    parser.append_post_body(reinterpret_cast<const unsigned char *>(b2.data()),
                            static_cast<unsigned int>(b2.size()), false);
    mid.error           = parser.error;
    mid.post_json       = peer->post.to_json();
    mid.isfinish        = peer->isfinish;
    mid.closed          = parser.temp_post_data->multipart_closed;
    mid.waiting_closing = parser.temp_post_data->boundary_hit_waiting_closing;
    mid.dashes_seen     = parser.temp_post_data->closing_dashes_seen;
    parser.append_post_body(reinterpret_cast<const unsigned char *>(b3.data()),
                            static_cast<unsigned int>(b3.size()), true);

    BodyResult r;
    r.error           = parser.error;
    r.post_json       = peer->post.to_json();
    r.isfinish        = peer->isfinish;
    r.closed          = parser.temp_post_data->multipart_closed;
    r.waiting_closing = parser.temp_post_data->boundary_hit_waiting_closing;
    r.dashes_seen     = parser.temp_post_data->closing_dashes_seen;
    parser.temp_post_data.reset();
    return r;
}

BodyResult run_body_h2_three(const std::string &bd, const std::string &b1, const std::string &b2,
                             const std::string &b3, BodyResult &mid)
{
    auto peer = std::make_shared<http::httppeer>();
    http::http2parse parser;
    http::HTTP2_POST_DATA_T pd;
    pd.peer      = peer;
    pd.stream_id = 1;
    pd.posttype  = 2;
    pd.boundary  = bd;
    pd.content   = std::string_view(b1);
    parser.post_data_process(pd, 0);
    pd.content   = std::string_view(b2);
    parser.post_data_process(pd, 0);
    mid.error           = parser.error;
    mid.error_is_conn   = parser.error_is_conn;
    mid.h2_code         = parser.h2_error_code;
    mid.post_json       = peer->post.to_json();
    mid.closed          = pd.multipart_closed;
    mid.waiting_closing = pd.boundary_hit_waiting_closing;
    mid.dashes_seen     = pd.closing_dashes_seen;
    pd.content   = std::string_view(b3);
    parser.post_data_process(pd, 1);

    BodyResult r;
    r.error           = parser.error;
    r.error_is_conn   = parser.error_is_conn;
    r.h2_code         = parser.h2_error_code;
    r.post_json       = peer->post.to_json();
    r.closed          = pd.multipart_closed;
    r.waiting_closing = pd.boundary_hit_waiting_closing;
    r.dashes_seen     = pd.closing_dashes_seen;
    pd.fp.reset();
    return r;
}

std::string body_desc(const BodyResult &r, bool show_conn)
{
    std::string out = error_desc(r.error, r.error_is_conn, show_conn);
    if (show_conn)
    {
        out += " h2_code=0x" + std::to_string(r.h2_code);
    }
    else
    {
        out += " isfinish=" + std::to_string(r.isfinish ? 1 : 0);
    }
    out += " closed=" + std::to_string(r.closed ? 1 : 0);
    out += " wait=" + std::to_string(r.waiting_closing ? 1 : 0);
    out += " seen=" + std::to_string(r.dashes_seen);
    out += " post=" + r.post_json;
    return out;
}

void run_body_scan_side(const std::string &side)
{
    const bool show_conn = (side == "h2");
    auto run             = [&](const std::string &body) {
        return (side == "h1") ? run_body_h1(body) : run_body_h2(body);
    };

    // E1：终止 boundary 之后的 epilogue 里再放一个伪 boundary + 伪造 part。
    // RFC 2046 §5.1.1 要求忽略 epilogue；正确行为是 ghost 不出现、REAL 仍在。
    {
        std::string body = "--X\r\n"
                           "Content-Disposition: form-data; name=\"textfield\"\r\n"
                           "\r\n"
                           "REAL\r\n"
                           "--X--\r\n"
                           "--X\r\n"
                           "Content-Disposition: form-data; name=\"ghost\"\r\n"
                           "\r\n"
                           "GHOST\r\n"
                           "--X--\r\n";
        BodyResult r = run(body);
        bool ok      = (r.error == 0 && r.post_json.find("REAL") != std::string::npos &&
                        r.post_json.find("GHOST") == std::string::npos);
        report2(side, "E1_epilogue_fake_part_not_injected", ok,
                "error=0 post 含 REAL 且不含 GHOST", body_desc(r, show_conn));
    }

    // E2：epilogue 只是普通文本（没有伪 boundary）→ 不得变成字段，也不得报错
    {
        std::string body = "--X\r\n"
                           "Content-Disposition: form-data; name=\"textfield\"\r\n"
                           "\r\n"
                           "REAL\r\n"
                           "--X--\r\n"
                           "just some epilogue text\r\n";
        BodyResult r = run(body);
        bool ok      = (r.error == 0 && r.post_json.find("REAL") != std::string::npos &&
                        r.post_json.find("just some") == std::string::npos);
        report2(side, "E2_epilogue_plain_text_ignored", ok,
                "error=0 post 含 REAL 且不含 epilogue 文本", body_desc(r, show_conn));
    }

    // P1：首个 boundary 之前的 preamble。当前实现的既定行为是 40026（严格拒绝），
    // 本用例锁住该现状；若将来改为按 RFC 忽略 preamble，这条需要同步改成 error=0。
    {
        std::string body = "This is a preamble\r\n"
                           "--X\r\n"
                           "Content-Disposition: form-data; name=\"textfield\"\r\n"
                           "\r\n"
                           "HELLO\r\n"
                           "--X--\r\n";
        BodyResult r   = run(body);
        bool code_ok   = (!show_conn || (r.h2_code == 0x1 && !r.error_is_conn));
        bool ok        = (r.error == 40026 && code_ok);
        report2(side, "P1_preamble_currently_40026", ok,
                std::string("error=40026") + (show_conn ? " error_is_conn=0 h2_code=0x1" : ""),
                body_desc(r, show_conn));
    }

    // M1：两个 part 同包（检验 boundary 后的 CRLF 处理是否会把下一个 part 的头吃掉）
    {
        std::string body = "--X\r\n"
                           "Content-Disposition: form-data; name=\"a\"\r\n"
                           "\r\n"
                           "AAA\r\n"
                           "--X\r\n"
                           "Content-Disposition: form-data; name=\"b\"\r\n"
                           "\r\n"
                           "BBB\r\n"
                           "--X--\r\n";
        BodyResult r = run(body);
        bool ok      = (r.error == 0 && r.post_json.find("AAA") != std::string::npos &&
                        r.post_json.find("BBB") != std::string::npos);
        report2(side, "M1_two_parts_same_buffer", ok,
                "error=0 post 同时含 AAA 与 BBB", body_desc(r, show_conn));
    }

    // M2：第一个 part 正文为空，紧跟第二个 part（field_offset 跳 CRLF 的边界情形）
    {
        std::string body = "--X\r\n"
                           "Content-Disposition: form-data; name=\"a\"\r\n"
                           "\r\n"
                           "\r\n"
                           "--X\r\n"
                           "Content-Disposition: form-data; name=\"b\"\r\n"
                           "\r\n"
                           "BBB\r\n"
                           "--X--\r\n";
        BodyResult r = run(body);
        bool ok      = (r.error == 0 && r.post_json.find("BBB") != std::string::npos);
        report2(side, "M2_empty_part_then_next", ok,
                "error=0 post 含 BBB", body_desc(r, show_conn));
    }

    // R1：boundary 文本收全后缓冲只剩一个 '-'。这一字节已经被计入 closing_dashes_seen，
    // field_offset 必须越过它，否则外层循环会在同一缓冲里重入、把同一个 '-' 再数一次，
    // 只见到一个横线就判闭合，后面真正的 part 被当成 epilogue 丢掉。
    {
        std::string seg1 = "--X\r\n"
                           "Content-Disposition: form-data; name=\"a\"\r\n"
                           "\r\n"
                           "REAL\r\n"
                           "--X-";          // 闭合的两个横线只到了第一个
        BodyResult r = (side == "h1") ? run_body_h1_first(seg1) : run_body_h2_first(seg1);
        bool ok      = (r.error == 0 && !r.closed && r.waiting_closing && r.dashes_seen == 1);
        report2(side, "R1_single_dash_at_tail_stays_waiting", ok,
                "error=0 closed=0 wait=1 seen=1", body_desc(r, show_conn));

        // 正极对照：下一包补上第二个 '-'，挂起状态必须能收齐并闭合，其后 epilogue 仍忽略。
        // 这条在缺陷前后都绿，用来证明 R1 的修复没有把真的 1+1 拆分一起判死。
        std::string seg2 = "-\r\n"
                           "--X\r\n"
                           "Content-Disposition: form-data; name=\"ghost\"\r\n"
                           "\r\n"
                           "GHOST\r\n"
                           "--X--\r\n";
        BodyResult r2 = (side == "h1") ? run_body_h1_two(seg1, seg2) : run_body_h2_frames(seg1, seg2);
        bool ok2      = (r2.error == 0 && r2.closed &&
                         r2.post_json.find("REAL") != std::string::npos &&
                         r2.post_json.find("GHOST") == std::string::npos);
        report2(side, "R1b_single_dash_then_second_dash_closes", ok2,
                "error=0 closed=1 post 含 REAL 且不含 GHOST", body_desc(r2, show_conn));
    }

    // R4/R5/R6：pre_content 路径的"缓冲剩 1 字节"孪生分支。第一包停在分隔符中间
    // （pre_content 留 1 / 2 / >2 字节），第二包在 boundary 文本后恰剩一个 '-' 就用完。
    // 这个 '-' 已计入 seen，field_offset 必须越过它，否则同一缓冲重入会再数一次、
    // 只见到一个横线就判闭合。第三包喂 'Z'（不是第二个横线）：正确行为是挂起失效、
    // 始终不闭合；缺陷会提前置 closed=1，把畸形收尾当成合法终止 boundary 放行。
    {
        const std::string partx  = "--X\r\n"
                                   "Content-Disposition: form-data; name=\"a\"\r\n"
                                   "\r\n"
                                   "REAL\r\n";
        const std::string partxy = "--XY\r\n"
                                   "Content-Disposition: form-data; name=\"a\"\r\n"
                                   "\r\n"
                                   "REAL\r\n";
        const std::string tail   = "Z\r\n";
        struct Shape
        {
            const char *name;
            const char *bd;
            std::string b1;
            std::string b2;
        };
        const Shape shapes[] = {
            {"R4_precontent_size1_dash_at_tail", "X", partx + "-", "-X-"},
            {"R5_precontent_size2_dash_at_tail", "X", partx + "--", "X-"},
            {"R6_precontent_partial_dash_at_tail", "XY", partxy + "--X", "Y-"},
        };
        for (const auto &sh : shapes)
        {
            BodyResult mid;
            BodyResult res = (side == "h1")
                                 ? run_body_h1_three(sh.bd, sh.b1, sh.b2, tail, mid)
                                 : run_body_h2_three(sh.bd, sh.b1, sh.b2, tail, mid);
            bool pass      = (mid.error == 0 && !mid.closed && mid.waiting_closing && mid.dashes_seen == 1 &&
                              res.error == 0 && !res.closed &&
                              res.post_json.find("REAL") != std::string::npos);
            report2(side, sh.name, pass,
                    "mid: error=0 closed=0 wait=1 seen=1; final: error=0 closed=0 post 含 REAL",
                    "mid: " + body_desc(mid, show_conn) + " | final: " + body_desc(res, show_conn));
        }
    }

    // 两侧不同：h1 chunked 在终止 boundary 处就 break，h2 每一帧都从 field_offset=0 重扫
    const std::string epi_body = "--X\r\n"
                                 "Content-Disposition: form-data; name=\"textfield\"\r\n"
                                 "\r\n"
                                 "REAL\r\n"
                                 "--X--\r\n"
                                 "--X\r\n"
                                 "Content-Disposition: form-data; name=\"ghost\"\r\n"
                                 "\r\n"
                                 "GHOST\r\n"
                                 "--X--\r\n";
    if (side == "h1")
    {
        BodyResult r = run_body_h1_chunked(epi_body);
        bool ok      = (r.error == 0 && r.post_json.find("GHOST") == std::string::npos);
        report2(side, "E3_chunked_epilogue_not_injected", ok,
                "error=0 post 不含 GHOST", body_desc(r, show_conn));

        // E5：终止 boundary 的闭合 "--" 被跨缓冲切开 + epilogue 伪 CD 注入
        // seg1 完整命中 boundary 文本 "--X" 但闭合判定因缓冲不够挂起；
        // seg2 到来时挂起续接机制把两个 "-" 收齐，终止 boundary 闭合，
        // 后续的 "--X\r\nContent-Disposition...GHOST" 全属 epilogue，必须忽略。
        {
            // seg1：正文 + boundary 文本，但闭合的 "--" 还没到
            std::string seg1 = "--X\r\n"
                               "Content-Disposition: form-data; name=\"textfield\"\r\n"
                               "\r\n"
                               "REAL\r\n"
                               "--X";          // boundary 完整，但闭合 "--" 在这里断开
            // seg2：闭合 "--" + CRLF + epilogue 里的伪造 part
            std::string seg2 = "--\r\n"       // 终止 boundary 完整闭合
                               "--X\r\n"
                               "Content-Disposition: form-data; name=\"ghost\"\r\n"
                               "\r\n"
                               "GHOST\r\n"
                               "--X--\r\n";
            BodyResult r = run_body_h1_two(seg1, seg2);
            bool ok      = (r.error == 0 && r.post_json.find("REAL") != std::string::npos &&
                            r.post_json.find("GHOST") == std::string::npos);
            report2(side, "E5_closed_boundary_split_across_buffers", ok,
                    "error=0 post 含 REAL 且不含 GHOST", body_desc(r, show_conn));
        }

        // R2：非 chunked，闭合分隔符在上一包就认出来了，本包只剩 CRLF 与 epilogue。
        // 本包不进扫描器，但声明长度已经读完，收尾仍须置 isfinish；
        // 否则外层等不到完成信号，连接既不响应也不报错。
        {
            std::string seg1 = "--X\r\n"
                               "Content-Disposition: form-data; name=\"textfield\"\r\n"
                               "\r\n"
                               "REAL\r\n"
                               "--X--";        // 闭合分隔符完整，CRLF 留到下一包
            std::string seg2 = "\r\n"
                               "--X\r\n"
                               "Content-Disposition: form-data; name=\"ghost\"\r\n"
                               "\r\n"
                               "GHOST\r\n"
                               "--X--\r\n";
            BodyResult res = run_body_h1_two(seg1, seg2);
            bool pass      = (res.error == 0 && res.isfinish && res.closed &&
                              res.post_json.find("REAL") != std::string::npos &&
                              res.post_json.find("GHOST") == std::string::npos);
            report2(side, "R2_epilogue_next_buffer_still_finishes", pass,
                    "error=0 isfinish=1 closed=1 post 含 REAL 且不含 GHOST", body_desc(res, show_conn));
        }

        // R3：keep-alive 复用同一 parser。temp_post_data 是 unique_ptr，clear() 不重建它，
        // 上一条请求的 multipart_closed 若留着，下一条的正文会整段被当 epilogue 丢掉。
        {
            std::string b1 = "--X\r\n"
                             "Content-Disposition: form-data; name=\"first\"\r\n"
                             "\r\n"
                             "ONE\r\n"
                             "--X--\r\n";
            std::string b2 = "--X\r\n"
                             "Content-Disposition: form-data; name=\"second\"\r\n"
                             "\r\n"
                             "TWO\r\n"
                             "--X--\r\n";
            BodyResult res = run_body_h1_reuse(b1, b2);
            bool pass      = (res.error == 0 && res.isfinish &&
                              res.post_json.find("TWO") != std::string::npos);
            report2(side, "R3_clear_then_next_request_parsed", pass,
                    "error=0 isfinish=1 post 含 TWO", body_desc(res, show_conn));
        }
    }
    else
    {
        BodyResult r = run_body_h2_frames("--X\r\n"
                                          "Content-Disposition: form-data; name=\"textfield\"\r\n"
                                          "\r\n"
                                          "REAL\r\n"
                                          "--X--\r\n",
                                          "--X\r\n"
                                          "Content-Disposition: form-data; name=\"ghost\"\r\n"
                                          "\r\n"
                                          "GHOST\r\n"
                                          "--X--\r\n");
        bool ok      = (r.error == 0 && r.post_json.find("GHOST") == std::string::npos);
        report2(side, "E4_epilogue_in_next_frame_not_injected", ok,
                "error=0 post 不含 GHOST", body_desc(r, show_conn));

        // E6：终止 boundary 的闭合 "--" 被跨帧切开 + epilogue 伪 CD 注入
        // seg1 完整命中 boundary 文本 "--X"，闭合判定因帧不够挂起；
        // seg2 挂起续接收齐两个 "-" 后终止 boundary 闭合，后续伪造 part 必须忽略。
        {
            std::string seg1 = "--X\r\n"
                               "Content-Disposition: form-data; name=\"textfield\"\r\n"
                               "\r\n"
                               "REAL\r\n"
                               "--X";          // boundary 完整，闭合 "--" 在帧尾断开
            std::string seg2 = "--\r\n"       // 终止 boundary 完整闭合
                               "--X\r\n"
                               "Content-Disposition: form-data; name=\"ghost\"\r\n"
                               "\r\n"
                               "GHOST\r\n"
                               "--X--\r\n";
            BodyResult r = run_body_h2_frames(seg1, seg2);
            bool ok      = (r.error == 0 && r.post_json.find("REAL") != std::string::npos &&
                            r.post_json.find("GHOST") == std::string::npos);
            report2(side, "E6_closed_boundary_split_across_frames", ok,
                    "error=0 post 含 REAL 且不含 GHOST", body_desc(r, show_conn));
        }
    }
}

// ---- 报文构造 -------------------------------------------------------------

std::string make_part(const std::vector<std::string> &headers, const std::string &body)
{
    std::string out;
    for (const auto &h : headers)
    {
        out += h;
        out += "\r\n";
    }
    out += "\r\n";
    out += body;
    return out;
}

const char *CD_FILE = "Content-Disposition: form-data; name=\"fileField\"; filename=\"a.txt\"";
const char *CD_TEXT = "Content-Disposition: form-data; name=\"textfield\"";
const char *CT      = "Content-Type: text/plain";
const char *CTE     = "Content-Transfer-Encoding: binary";

// ---- 单侧 10 个用例 -------------------------------------------------------

void run_side(const std::string &side, const std::string &outdir)
{
    const bool show_conn = (side == "h2");
    auto run             = [&](const std::string &part, bool is_file) {
        return (side == "h1") ? run_h1(part, is_file) : run_h2(part, is_file);
    };

    // S1：文件 part 只有 Content-Disposition，没有 Content-Type → 正文应完整
    {
        std::string part = make_part({CD_FILE}, "HELLO");
        Result r         = run(part, true);
        bool ok          = (r.error == 0 && r.body == "HELLO");
        report(side, "S1_file_no_ct", ok,
               "error=0 body=\"HELLO\" (5 bytes)", actual_desc(r, show_conn));
    }

    // S2a：文件 part CD + CT + CTE → 正文应恰好 HELLO
    {
        std::string part = make_part({CD_FILE, CT, CTE}, "HELLO");
        Result r         = run(part, true);
        bool ok          = (r.error == 0 && r.body == "HELLO");
        report(side, "S2a_file_ct_cte", ok,
               "error=0 body=\"HELLO\" (5 bytes)", actual_desc(r, show_conn));
    }

    // S2b：文本 part CD + CT + CTE → 字段值应恰好 HELLO
    {
        std::string part = make_part({CD_TEXT, CT, CTE}, "HELLO");
        Result r         = run(part, false);
        bool ok          = (r.error == 0 && r.body == "HELLO");
        report(side, "S2b_text_ct_cte", ok,
               "error=0 textfield=\"HELLO\" (5 bytes)", actual_desc(r, show_conn));
    }

    // S2c：文件 part CD + CT + CTE + X-Custom → 正文应恰好 HELLO
    {
        std::string part = make_part({CD_FILE, CT, CTE, "X-Custom: v"}, "HELLO");
        Result r         = run(part, true);
        bool ok          = (r.error == 0 && r.body == "HELLO");
        report(side, "S2c_file_3_extra_headers", ok,
               "error=0 body=\"HELLO\" (5 bytes)", actual_desc(r, show_conn));
    }

    // S3：头顺序颠倒（Content-Type 在 Content-Disposition 之前）→ 应能解析出 name/filename/正文
    {
        std::string part = make_part({CT, CD_FILE}, "HELLO");
        Result r         = run(part, true);
        bool ok = (r.error == 0 && r.field_name == "fileField" && r.filename == "a.txt" && r.body == "HELLO");
        report(side, "S3_reversed_header_order", ok,
               "error=0 name=\"fileField\" filename=\"a.txt\" body=\"HELLO\"",
               actual_desc(r, show_conn));
    }

    // S4：mimetype 只应来自 Content-Type；part 只有 CTE 时 mimetype 应为空
    {
        std::string part = make_part({CD_FILE, CTE}, "HELLO");
        Result r         = run(part, true);
        bool ok          = (r.error == 0 && r.body == "HELLO" && r.mimetype.empty());
        report(side, "S4_mimetype_only_from_ct", ok,
               "error=0 mimetype=\"\" body=\"HELLO\"", actual_desc(r, show_conn));
    }

    // 决策 1：头区超限（17 个额外头，含 CD 共 18 行）→ 40059；h2 流级且码为 0x1
    {
        std::vector<std::string> headers;
        headers.push_back(CD_FILE);
        for (int i = 0; i < 17; ++i)
        {
            headers.push_back("X-H" + std::to_string(i) + ": v");
        }
        std::string part = make_part(headers, "HELLO");
        Result r         = run(part, true);
        bool code_ok     = (!show_conn || (r.h2_code == 0x1 && !r.error_is_conn));
        bool ok          = (r.error == 40059 && code_ok);
        report(side, "LIMIT_17_headers_40059", ok,
               std::string("error=40059") + (show_conn ? " error_is_conn=0 h2_code=0x1" : ""),
               actual_desc(r, show_conn));
    }

    // 决策 1：超大单头（>128 字节）→ 同样 40059、h2 流级且码为 0x1
    {
        std::string long_header = "X-Long: " + std::string(200, 'a');
        std::string part        = make_part({CD_FILE, long_header}, "HELLO");
        Result r                = run(part, true);
        bool code_ok            = (!show_conn || (r.h2_code == 0x1 && !r.error_is_conn));
        bool ok                 = (r.error == 40059 && code_ok);
        report(side, "LIMIT_oversized_header_40059", ok,
               std::string("error=40059") + (show_conn ? " error_is_conn=0 h2_code=0x1" : ""),
               actual_desc(r, show_conn));
    }

    // 决策 2：CD 并入头预算后容量仍是「CD + 16 个额外头」→ 必须 200（error=0）
    {
        std::vector<std::string> headers;
        headers.push_back(CD_FILE);
        for (int i = 0; i < 16; ++i)
        {
            headers.push_back("X-H" + std::to_string(i) + ": v");
        }
        std::string part = make_part(headers, "HELLO");
        Result r         = run(part, true);
        bool ok          = (r.error == 0 && r.body == "HELLO");
        report(side, "CAP_CD_plus_16_headers_ok", ok,
               "error=0 body=\"HELLO\" (5 bytes)", actual_desc(r, show_conn));
    }

    // 决策 3：40226 是服务端落盘失败（内部问题）→ h2 保持 0x2，不得改成 0x1
    {
        http::server_loaclvar &lv = http::get_server_global_var();
        std::string saved         = lv.temp_path;
        std::string blocker       = outdir + "/not_a_dir";
        std::ofstream(blocker) << "x";
        lv.temp_path = blocker + "/";
        Result r     = run(make_part({CD_FILE, CT}, "HELLO"), true);
        lv.temp_path = saved;
        std::remove(blocker.c_str());
        bool ok = (r.error == 40226 && (!show_conn || (!r.error_is_conn && r.h2_code == 0x2)));
        report(side, "KEEP_40226_internal_0x2", ok,
               std::string("error=40226") + (show_conn ? " error_is_conn=0 h2_code=0x2" : ""),
               actual_desc(r, show_conn));
    }

    // 反例 1：正常文件 part CD + CT → 必须仍然正确（绿）
    {
        std::string part = make_part({CD_FILE, CT}, "HELLO");
        Result r         = run(part, true);
        bool ok = (r.error == 0 && r.body == "HELLO" && r.mimetype == "text/plain" &&
                   r.field_name == "fileField" && r.filename == "a.txt");
        report(side, "NEG_normal_file_cd_ct", ok,
               "error=0 name=\"fileField\" filename=\"a.txt\" mimetype=\"text/plain\" body=\"HELLO\"",
               actual_desc(r, show_conn));
    }

    // 反例 2：正常文本 part（只有 CD）→ 必须仍然正确（绿）
    {
        std::string part = make_part({CD_TEXT}, "HELLO");
        Result r         = run(part, false);
        bool ok          = (r.error == 0 && r.body == "HELLO");
        report(side, "NEG_normal_text_cd_only", ok,
               "error=0 textfield=\"HELLO\"", actual_desc(r, show_conn));
    }

    // 反例 3：正文首个字节就是“伪头” → 不得被头解析误吃（绿）
    {
        std::string pseudo = "Content-Type: text/plain\r\n\r\nHELLO";
        std::string part   = make_part({CD_FILE, CT}, pseudo);
        Result r           = run(part, true);
        bool ok            = (r.error == 0 && r.body == pseudo);
        report(side, "NEG_body_starts_with_pseudo_header", ok,
               "error=0 body=" + value_desc(pseudo), actual_desc(r, show_conn));
    }

    // 反例 4：正文含大量 CRLF CRLF → 必须逐字节保留（绿）
    {
        std::string body = "A\r\n\r\nB\r\n\r\nC\r\n\r\nD";
        std::string part = make_part({CD_FILE, CT}, body);
        Result r         = run(part, true);
        bool ok          = (r.error == 0 && r.body == body);
        report(side, "NEG_body_with_many_crlf", ok,
               "error=0 body=" + value_desc(body), actual_desc(r, show_conn));
    }
}
} // namespace

int main(int argc, char **argv)
{
    std::string outdir = (argc > 1) ? argv[1] : "/tmp/pz_mp_part_headers";
    fs::create_directories(outdir);
    http::server_loaclvar &lv = http::get_server_global_var();
    lv.temp_path              = outdir;
    if (!lv.temp_path.empty() && lv.temp_path.back() != '/')
    {
        lv.temp_path.push_back('/');
    }

    std::printf("== multipart part header probe (h1 + h2) ==\n");
    std::printf("temp dir: %s\n", lv.temp_path.c_str());

    run_side("h1", outdir);
    run_side("h2", outdir);

    std::printf("SUMMARY total=%d pass=%d fail=%d\n", g_total, g_pass, g_fail);

    std::printf("== multipart body scan (epilogue / preamble / multi-part) ==\n");
    run_body_scan_side("h1");
    run_body_scan_side("h2");
    std::printf("BODY_SCAN_SUMMARY total=%d pass=%d fail=%d\n", g2_total, g2_pass, g2_fail);

    return ((g_fail + g2_fail) > 0) ? 1 : 0;
}
