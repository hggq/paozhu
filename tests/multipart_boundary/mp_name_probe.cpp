// 两件事：临时件落盘名必须每次不同；上传文件名里的路径不能带出去。
// 名字撞车的后果是同一个请求里两个等长文件 part 落到同一个文件，后一个把前一个覆盖掉，
// 而回显的 size 各归各，只有读文件才发现少了一个。
// 改名等于动 httpwatch 清理临时件的输入，所以每条名字都当场过一遍白名单 is_http_temp_filename()。
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <filesystem>
#include <set>
#include <string>
#include <vector>
#include "func.h"
#include "http_parse.h"
#include "httppeer.h"
#include "server_localvar.h"

static const std::string BD = "MPBOUNDARY9";
static std::string          g_outdir = "./temp/multipart_boundary_run/out/";

static unsigned int g_checked = 0;
static unsigned int g_fail    = 0;

static void check(bool ok, const std::string &tag)
{
    g_checked++;
    if (!ok)
    {
        std::printf("FAIL %s\n", tag.c_str());
        g_fail++;
    }
}

static std::string readfile(const std::string &p)
{
    if (p.empty())
        return "";
    std::FILE *f = std::fopen(p.c_str(), "rb");
    if (f == nullptr)
        return "<missing>";
    std::string out;
    char         buf[65536];
    size_t       n = 0;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0)
        out.append(buf, n);
    std::fclose(f);
    return out;
}

static std::string stem_of(const std::string &path)
{
    return path.substr(path.find_last_of('/') + 1);
}

// —— 名字段 ——
static void uniq_upload(unsigned long long len, unsigned int times)
{
    std::set<std::string> seen;
    unsigned int          dup = 0, notwhite = 0, badsuffix = 0;
    std::string           first, last;
    for (unsigned int i = 0; i < times; i++)
    {
        std::string nm = http::make_http_temp_upload_name(len);
        if (i == 0)
            first = nm;
        last = nm;
        if (!seen.insert(nm).second)
            dup++;
        if (!http::is_http_temp_filename(nm))
            notwhite++;
        std::string want_suffix = "_" + std::to_string(len);
        if (nm.size() < want_suffix.size() || nm.compare(nm.size() - want_suffix.size(), want_suffix.size(), want_suffix) != 0)
            badsuffix++;
    }
    std::printf("UNIQ upload times=%u distinct=%zu dup=%u notwhite=%u badsuffix=%u first=%s last=%s\n", times,
                seen.size(), dup, notwhite, badsuffix, first.c_str(), last.c_str());
    check(dup == 0, "upload/dup");
    check(notwhite == 0, "upload/whitelist");
    check(badsuffix == 0, "upload/length-suffix");
}

static void uniq_raw(unsigned int times)
{
    std::set<std::string> seen;
    unsigned int          dup = 0, notwhite = 0;
    std::string           first, last;
    for (unsigned int i = 0; i < times; i++)
    {
        std::string nm = http::make_http_temp_raw_name();
        if (i == 0)
            first = nm;
        last = nm;
        if (!seen.insert(nm).second)
            dup++;
        if (!http::is_http_temp_filename(nm))
            notwhite++;
    }
    std::printf("UNIQ raw times=%u distinct=%zu dup=%u notwhite=%u first=%s last=%s\n", times, seen.size(), dup,
                notwhite, first.c_str(), last.c_str());
    check(dup == 0, "raw/dup");
    check(notwhite == 0, "raw/whitelist");
}

// —— 消毒段（纯函数真值表）——
static void safe_name_table()
{
    struct Row
    {
        std::string in;
        std::string want;
        std::string tag;
    };
    std::vector<Row> rows;
    rows.push_back({"plain.txt", "plain.txt", "plain"});
    rows.push_back({"/var/tmp/x.txt", "x.txt", "posix-abs"});
    rows.push_back({"../../etc/passwd", "passwd", "posix-up"});
    rows.push_back({"a\\b\\c.txt", "c.txt", "win-rel"});
    rows.push_back({"C:\\evil\\dir\\rep.pdf", "rep.pdf", "win-abs"});
    rows.push_back({"mix\\slash/last.txt", "last.txt", "mixed"});
    rows.push_back({"../../", "", "only-up"});
    rows.push_back({"/", "", "root-only"});
    rows.push_back({"", "", "empty"});
    // 含真 NUL 的夹具必须显式给长度：std::string(const char*) 会在 0 处截断，那条臂就恒真了
    rows.push_back({std::string("et/\0passwd", 10), "passwd", "real-nul"});
    // 写成 \ 再跟一个 0 是字面文本，反斜杠先变成 '/' 再取 basename
    rows.push_back({"pa\\0sswd.txt", "0sswd.txt", "backslash-zero"});
    rows.push_back({std::string("\xff\xfe bad.bin", 10), "\xff\xfe bad.bin", "highbytes"});

    unsigned int matched = 0;
    for (auto &row : rows)
    {
        std::string got = http::safe_filename(row.in);
        bool        ok  = (got == row.want);
        if (!ok)
        {
            std::printf("FAIL safe[%s] got=%s want=%s\n", row.tag.c_str(), got.c_str(), row.want.c_str());
        }
        check(ok, "safe/" + row.tag);
        // 任何输入的结果都不许再带路径分隔符
        check(got.find('/') == std::string::npos && got.find('\\') == std::string::npos, "safe/nosep/" + row.tag);
        if (ok)
            matched++;
    }
    std::printf("TABLE safe_filename rows=%zu matched=%u\n", rows.size(), matched);
}

// —— 解析路径段 ——
static std::string fp(const std::string &name, const std::string &file, const std::string &value)
{
    return "--" + BD + "\r\nContent-Disposition: form-data; name=\"" + name + "\"; filename=\"" + file + "\"\r\n"
           "Content-Type: application/octet-stream\r\n\r\n" +
           value + "\r\n";
}

struct Parsed
{
    unsigned int error = 0;
    bool         finish = false;
    std::string  f1, f2, t1, t2, c1, c2;
};

static Parsed parse_h1(const std::string &body)
{
    Parsed p;
    std::error_code ec;
    if (!std::filesystem::exists(g_outdir, ec))
        std::filesystem::create_directories(g_outdir, ec);
    for (auto &e : std::filesystem::directory_iterator(g_outdir, ec))
        std::filesystem::remove(e.path(), ec);

    auto peer = std::make_shared<http::httppeer>();
    http::httpparse P;
    P.setpeer(peer);

    std::string hdr = "POST /mp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: multipart/form-data; boundary=" + BD +
                      "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    unsigned int guard = 0;
    while (!P.isfinish_header && P.error == 0 && P.readoffset < hdr.size() && ++guard < 128)
        P.read_http_header_block((const unsigned char *)hdr.data(), (unsigned int)hdr.size());
    if (P.error > 0 || !P.isfinish_header)
    {
        p.error  = P.error > 0 ? P.error : 9001;
        p.finish = peer->isfinish;
        return p;
    }
    P.http_content_length = 0;
    P.readoffset          = 0;
    P.read_http_post_block((const unsigned char *)body.data(), (unsigned int)body.size());

    p.error  = P.error;
    p.finish = peer->isfinish;
    p.f1     = peer->files["up1"]["filename"].to_string();
    p.f2     = peer->files["up2"]["filename"].to_string();
    p.t1     = peer->files["up1"]["tempfile"].to_string();
    p.t2     = peer->files["up2"]["tempfile"].to_string();
    p.c1     = readfile(p.t1);
    p.c2     = readfile(p.t2);
    std::printf("PARSE err=%u fin=%d f1=%s f2=%s t1=%s t2=%s c1=%zu c2=%zu\n", p.error, p.finish ? 1 : 0, p.f1.c_str(),
                p.f2.c_str(), stem_of(p.t1).c_str(), stem_of(p.t2).c_str(), p.c1.size(), p.c2.size());
    return p;
}

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code        ec;
    if (!std::filesystem::exists(g_outdir, ec))
        std::filesystem::create_directories(g_outdir, ec);
    gv.temp_path = g_outdir;

    std::printf("=== part1 名字唯一性（同一秒内连续调用）===\n");
    uniq_upload(558, 2000);
    uniq_upload(3151001, 2000);
    uniq_raw(2000);

    std::printf("\n=== part2 safe_filename 真值表 ===\n");
    safe_name_table();

    std::printf("\n=== part3 h1 解析路径：穿越文件名 + 两个等长文件 part ===\n");
    {
        std::string body = fp("up1", "../../etc/passwd", std::string(600, 'A')) +
                           fp("up2", "C:\\evil\\name.txt", std::string(600, 'B')) + "--" + BD + "--\r\n";
        Parsed p = parse_h1(body);
        check(p.error == 0 && p.finish, "parse/finish");
        check(p.f1 == "passwd", "parse/filename-1");
        check(p.f2 == "name.txt", "parse/filename-2");
        check(!p.t1.empty() && p.t1 != p.t2, "parse/tempfile-distinct");
        check(p.t1.compare(0, g_outdir.size(), g_outdir) == 0 && p.t2.compare(0, g_outdir.size(), g_outdir) == 0,
              "parse/tempfile-under-tmppath");
        check(http::is_http_temp_filename(stem_of(p.t1)) && http::is_http_temp_filename(stem_of(p.t2)),
              "parse/tempfile-whitelist");
        check(p.c1 == std::string(600, 'A'), "parse/content-1-not-overwritten");
        check(p.c2 == std::string(600, 'B'), "parse/content-2");
    }

    std::printf("\nSUMMARY name checked=%u fail=%u\n", g_checked, g_fail);
    std::printf("NAMEPROBE total_fail=%u\n", g_fail);
    return g_fail > 0 ? 1 : 0;
}
