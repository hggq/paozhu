// raw POST 内存快路径判据台（HTTP/1 侧）。
//
// 被测行为：Content-Type 为 application/octet-stream / binary / 未识别，或无 Content-Type
// 且首字节不落入 multipart/json/xml 嗅探（即 posttype==5）的请求正文——
//   * 累计 <= CONST_HTTP_RAW_POST_SIZE：留在 peer->rawcontent，不建临时文件，
//     files["rawcontent"]["size"] == rawcontent 字节数，tempfile 为空。
//   * 超过 CONST_HTTP_RAW_POST_SIZE：spill 到磁盘，建且仅建一个 pzraw_ 文件，
//     文件正文与整段 body 逐字节相同（含把内存前缀先刷盘那一步），spill 后 rawcontent 清空，
//     files["rawcontent"]["size"] == 声明/累计的正文总长。
//
// 判式都是"行为红"：撤掉内存分支（一律 spill）则 filecount 臂红；撤掉 spill 前缀刷盘则
// spill 文件正文少一段前缀红；把 <= 写成 < 则恰好 == 阈值那条从内存变落盘红。
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <filesystem>
#include "http_parse.h"
#include "httppeer.h"
#include "server_localvar.h"
#include "func.h"
#include "cost_define.h"

static_assert(CONST_HTTP_RAW_POST_SIZE == 2097152, "raw 快路径阈值漂移：与 cost_define.h 不同步");

// 末尾的 / 不能省：落盘名直接拼在 temp_path 后面，框架读 conf 时也会补这个 /
static std::string g_outdir = "./temp/raw_post_fastpath_run/h1out/";

static std::string readfile(const std::string &p)
{
    if (p.empty())
        return "";
    std::FILE *f = std::fopen(p.c_str(), "rb");
    if (f == nullptr)
        return "<missing:" + p + ">";
    std::string out;
    char buf[65536];
    size_t n = 0;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0)
        out.append(buf, n);
    std::fclose(f);
    return out;
}

static void clean_outdir()
{
    std::error_code ec;
    if (!std::filesystem::exists(g_outdir, ec))
        return;
    for (auto &e : std::filesystem::directory_iterator(g_outdir, ec))
        std::filesystem::remove(e.path(), ec);
}

// outdir 里 pzraw_ 前缀的文件数（框架 raw 落盘名以此前缀开头，见 func.h）
static unsigned int count_rawfiles()
{
    unsigned int n = 0;
    std::error_code ec;
    if (!std::filesystem::exists(g_outdir, ec))
        return 0;
    const std::string_view pfx = http::HTTP_TEMP_RAW_PREFIX;
    for (auto &e : std::filesystem::directory_iterator(g_outdir, ec))
    {
        if (!e.is_regular_file(ec))
            continue;
        std::string name = e.path().filename().string();
        if (name.size() > pfx.size() && std::string_view(name).substr(0, pfx.size()) == pfx)
            n++;
    }
    return n;
}

struct Reading
{
    unsigned int error   = 0;
    bool finish          = false;
    std::string raw;          // peer->rawcontent（内存快路径正文）
    std::string filebytes;    // tempfile 读回的正文（spill 才有）
    std::string tempfile;     // files["rawcontent"]["tempfile"]
    long long size            = -1; // files["rawcontent"]["size"]
    unsigned int filecount    = 0;  // outdir 里 pzraw_ 文件数
};

static std::string make_hdr(bool with_ctype, unsigned long long clen, bool chunked)
{
    std::string h = "POST /raw HTTP/1.1\r\nHost: 127.0.0.1\r\n";
    if (with_ctype)
        h += "Content-Type: application/octet-stream\r\n";
    if (chunked)
        h += "Transfer-Encoding: chunked\r\n";
    else
        h += "Content-Length: " + std::to_string(clen) + "\r\n";
    h += "\r\n";
    return h;
}

// cuts：body（或 chunked 编码流）的分段字节位置；空 = 整段一次投递。
static Reading run(bool with_ctype, const std::string &body, const std::vector<unsigned int> &cuts, bool chunked)
{
    Reading r;
    clean_outdir();

    // 需要分段的输入流：CL 用 body 本身，chunked 用编码后的整条流
    std::string stream;
    if (chunked)
    {
        // 单个 chunk 承载全部 body，再接终止块
        char hex[24];
        std::snprintf(hex, sizeof hex, "%lx\r\n", (unsigned long)body.size());
        stream.assign(hex);
        stream += body;
        stream += "\r\n0\r\n\r\n";
    }
    else
    {
        stream = body;
    }

    std::vector<std::string> chunks;
    {
        unsigned int prev = 0;
        for (unsigned int c : cuts)
        {
            if (c > prev && c <= stream.size())
            {
                chunks.push_back(stream.substr(prev, c - prev));
                prev = c;
            }
        }
        chunks.push_back(stream.substr(prev));
    }

    auto peer = std::make_shared<http::httppeer>();
    http::httpparse P;
    P.setpeer(peer);

    std::string hdr = make_hdr(with_ctype, body.size(), chunked);
    unsigned int guard = 0;
    while (!P.isfinish_header && P.error == 0 && P.readoffset < hdr.size() && ++guard < 256)
        P.read_http_header_block((const unsigned char *)hdr.data(), (unsigned int)hdr.size());
    if (P.error > 0 || !P.isfinish_header)
    {
        r.error = P.error > 0 ? P.error : 9001;
        return r;
    }
    // 头完成后把已消费计数归零（直接驱动 read_http_post_block 必须照做，否则 body 被截断）
    P.http_content_length = 0;

    for (auto &chunk : chunks)
    {
        if (peer->isfinish || P.error > 0)
            break;
        P.readoffset = 0;
        P.read_http_post_block((const unsigned char *)chunk.data(), (unsigned int)chunk.size());
    }

    r.error = P.error;
    // 收尾只由 is_final 落：CL 路径读完即 is_final；chunked 路径由终止块触发。
    // 若因分段提前 break，isfinish 仍按解析器实报。
    r.finish = peer->isfinish;
    r.raw    = peer->rawcontent;
    r.tempfile = peer->files["rawcontent"]["tempfile"].to_string();
    r.size     = peer->files["rawcontent"]["size"].to_int();
    if (!r.tempfile.empty())
        r.filebytes = readfile(r.tempfile);
    // 解析器对象出作用域前，spill 文件已在 is_final 处 fclose；这里再取一次计数
    r.filecount = count_rawfiles();
    return r;
}

static unsigned int g_total = 0, g_fail = 0;
static void check(const char *name, bool ok, const std::string &detail)
{
    g_total++;
    if (!ok)
    {
        g_fail++;
        std::printf("FAIL %s %s\n", name, detail.c_str());
    }
    else
    {
        std::printf("PASS %s\n", name);
    }
}

// 内存臂判式：无文件、tempfile 空、rawcontent==body、size==body 长度
static void expect_mem(const char *name, const Reading &r, const std::string &body)
{
    bool ok = r.error == 0 && r.finish && r.filecount == 0 && r.tempfile.empty() && r.raw == body &&
              r.size == (long long)body.size();
    std::string d = "err=" + std::to_string(r.error) + " fin=" + (r.finish ? "1" : "0") +
                    " fc=" + std::to_string(r.filecount) + " size=" + std::to_string(r.size) +
                    " memlen=" + std::to_string(r.raw.size()) + " tempfile[" + r.tempfile + "]";
    check(name, ok, d);
}

// spill 臂判式：恰一个文件、tempfile 非空、文件正文==body、rawcontent 已清空、size==body 长度
static void expect_spill(const char *name, const Reading &r, const std::string &body)
{
    bool ok = r.error == 0 && r.finish && r.filecount == 1 && !r.tempfile.empty() && r.filebytes == body &&
              r.raw.empty() && r.size == (long long)body.size();
    std::string d = "err=" + std::to_string(r.error) + " fin=" + (r.finish ? "1" : "0") +
                    " fc=" + std::to_string(r.filecount) + " size=" + std::to_string(r.size) +
                    " filedots=" + std::to_string(r.filebytes.size()) + " memlen=" + std::to_string(r.raw.size()) +
                    " tempfile[" + r.tempfile + "]";
    check(name, ok, d);
}

static std::string blob(size_t n, char fill)
{
    return std::string(n, fill);
}

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code gec;
    std::filesystem::create_directories(g_outdir, gec);
    gv.temp_path = g_outdir;

    const size_t CAP = CONST_HTTP_RAW_POST_SIZE;
    const std::string small = "RAWBODY-" + std::string(24, 'x') + "-END";

    std::printf("RP1 begin cap=%zu small=%zu\n", CAP, small.size());

    // C1a：小 body + Content-Type octet-stream，整段一次投递 → 内存
    {
        Reading r = run(true, small, {}, false);
        expect_mem("C1a-tiny-ctype-mem", r, small);
    }
    // C1b：无 Content-Type，首字节 'R' 落入 raw 回落 → 仍内存
    {
        Reading r = run(false, small, {}, false);
        expect_mem("C1b-tiny-nosniff-mem", r, small);
    }
    // C1c：小 body 分段投递（TCP 分包不变性）——每一刀的结果必须与整段一致（都落内存、同读数）
    {
        Reading golden = run(true, small, {}, false);
        bool gold_ok = golden.error == 0 && golden.finish && golden.filecount == 0 && golden.raw == small &&
                       golden.size == (long long)small.size();
        std::string bad;
        for (unsigned int i = 1; i < small.size(); i++)
        {
            Reading r = run(true, small, {i}, false);
            bool same = r.error == golden.error && r.finish == golden.finish && r.filecount == golden.filecount &&
                        r.raw == golden.raw && r.size == golden.size && r.tempfile == golden.tempfile;
            if (!same)
                bad += " cut=" + std::to_string(i);
        }
        check("C1c-tiny-cutinvariance-mem", gold_ok && bad.empty(),
              "golderr=" + std::to_string((int)gold_ok) + " badcuts=" + std::to_string(bad.empty() ? 0 : 1) + bad);
    }
    // C2：恰好 == 阈值 → 仍在内存（<= 边界）
    {
        std::string body = blob(CAP, 'A');
        Reading r        = run(true, body, {}, false);
        expect_mem("C2-at-cap-mem", r, body);
    }
    // C3：阈值 + 1，一次投递 → spill（无内存前缀，直接落盘），文件逐字节完整
    {
        std::string body = blob(CAP + 1, 'A');
        Reading r        = run(true, body, {}, false);
        expect_spill("C3-over-cap-spill", r, body);
    }
    // C4：前缀刷盘——先投满阈值（留在内存），再投 1 字节触发 spill；
    //     文件必须含内存前缀 + 这 1 字节（逐字节完整），且 spill 后 rawcontent 清空。
    {
        std::string head = blob(CAP, 'A');
        std::string body = head + "Z";
        // cut 落在阈值末尾：第一段 0..CAP（含头行后的 body 前 CAP 字节），第二段剩 1 字节
        Reading r = run(true, body, {(unsigned int)CAP}, false);
        expect_spill("C4-prefix-flush-spill", r, body);
    }
    // C5：chunked 小 body → 内存
    {
        Reading r = run(true, small, {}, true);
        expect_mem("C5-chunked-tiny-mem", r, small);
    }
    // C6：chunked 超阈值（两 chunk：阈值 + 1）→ spill，前缀刷盘，字节完整
    {
        std::string body = blob(CAP, 'A') + "Z";
        // 单个 chunk 承载整段 body，spill 前缀 = 阈值字节
        Reading r = run(true, body, {}, true);
        expect_spill("C6-chunked-over-spill", r, body);
    }

    std::printf("RP1 SUMMARY total=%u fail=%u\n", g_total, g_fail);
    clean_outdir();
    return g_fail > 0 ? 1 : 0;
}
