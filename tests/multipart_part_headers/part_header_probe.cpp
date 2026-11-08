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
    return (g_fail > 0) ? 1 : 0;
}
