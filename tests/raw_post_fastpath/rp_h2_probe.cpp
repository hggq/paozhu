// raw POST 内存快路径判据台（HTTP/2 / h2c 侧）。
//
// 与 H1 探针同一组判式，只是入口换成直接驱动 http2parse::post_data_process()：
// 每帧 = 一次调用，最后一帧 islast_pack=1（对应 END_STREAM）。DATA 帧正文上限 16384，
// 所以 >16384 的 body 天然被切成多帧——这正好覆盖"内存前缀刷盘"与 cur_length 不重不漏。
//
// posttype==5 的触发：首帧首字节既不是 '-'（且无 boundary）也不是 '{' '[' '<'，嗅探落到 raw。
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <filesystem>
#include "http2_parse.h"
#include "httppeer.h"
#include "server_localvar.h"
#include "func.h"
#include "cost_define.h"

static_assert(CONST_HTTP_RAW_POST_SIZE == 2097152, "raw 快路径阈值漂移：与 cost_define.h 不同步");

static std::string g_outdir = "./temp/raw_post_fastpath_run/h2out/";

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
    unsigned int error = 0;
    std::string raw;
    std::string filebytes;
    std::string tempfile;
    long long size      = -1;
    unsigned int filecount = 0;
};

// 一帧 = 一次 post_data_process；frame 0 时整段当一帧投递（仅用于阈值数学），
// 否则按 frame 大小切片，最后一帧 islast=1。
static Reading run(const std::string &body, unsigned int frame)
{
    Reading r;
    clean_outdir();

    std::vector<std::string> chunks;
    if (frame == 0 || body.size() <= frame)
    {
        chunks.push_back(body);
    }
    else
    {
        for (size_t off = 0; off < body.size(); off += frame)
            chunks.push_back(body.substr(off, frame));
    }

    auto peer = std::make_shared<http::httppeer>();
    http::http2parse P;
    http::HTTP2_POST_DATA_T pd;
    pd.peer      = peer;
    pd.stream_id = 1;
    // boundary 留空 + 首字节 'R' ⇒ 嗅探落到 posttype 5

    for (size_t k = 0; k < chunks.size(); k++)
    {
        if (P.error > 0)
            break;
        pd.content = std::string_view(chunks[k]);
        P.post_data_process(pd, (k + 1 == chunks.size()) ? 1 : 0);
    }

    r.error    = P.error;
    r.raw      = peer->rawcontent;
    r.tempfile = peer->files["rawcontent"]["tempfile"].to_string();
    r.size     = peer->files["rawcontent"]["size"].to_int();
    if (!r.tempfile.empty())
        r.filebytes = readfile(r.tempfile);
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

static void expect_mem(const char *name, const Reading &r, const std::string &body)
{
    bool ok = r.error == 0 && r.filecount == 0 && r.tempfile.empty() && r.raw == body &&
              r.size == (long long)body.size();
    std::string d = "err=" + std::to_string(r.error) + " fc=" + std::to_string(r.filecount) +
                    " size=" + std::to_string(r.size) + " memlen=" + std::to_string(r.raw.size()) +
                    " tempfile[" + r.tempfile + "]";
    check(name, ok, d);
}

static void expect_spill(const char *name, const Reading &r, const std::string &body)
{
    bool ok = r.error == 0 && r.filecount == 1 && !r.tempfile.empty() && r.filebytes == body &&
              r.raw.empty() && r.size == (long long)body.size();
    std::string d = "err=" + std::to_string(r.error) + " fc=" + std::to_string(r.filecount) +
                    " size=" + std::to_string(r.size) + " filedots=" + std::to_string(r.filebytes.size()) +
                    " memlen=" + std::to_string(r.raw.size()) + " tempfile[" + r.tempfile + "]";
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

    const size_t CAP   = CONST_HTTP_RAW_POST_SIZE;
    const unsigned int FRAME = 16384;
    const std::string small = "RAWBODY-" + std::string(24, 'x') + "-END";

    std::printf("RP2 begin cap=%zu frame=%u small=%zu\n", CAP, FRAME, small.size());

    // H-C1：小 body 单帧 → 内存
    {
        Reading r = run(small, FRAME);
        expect_mem("H1-tiny-mem", r, small);
    }
    // H-C2：恰好 == 阈值（128 × 16384）多帧投满 → 仍在内存（<= 边界）
    {
        std::string body = blob(CAP, 'A');
        Reading r        = run(body, FRAME);
        expect_mem("H2-at-cap-mem", r, body);
    }
    // H-C3：阈值 + 1（末帧 1 字节触发 spill）→ 恰一个文件、逐字节完整、rawcontent 清空
    {
        std::string body = blob(CAP, 'A') + "Z";
        Reading r        = run(body, FRAME);
        expect_spill("H3-over-cap-spill", r, body);
    }
    // H-C4：分帧不变性——40000 字节整段一帧 vs 按 16384 切多帧，两者都落内存且读数逐字段相同
    {
        std::string body = blob(40000, 'B');
        Reading one      = run(body, 0);       // 单帧
        Reading split    = run(body, FRAME);   // 多帧
        bool same = one.error == split.error && one.filecount == split.filecount &&
                    one.tempfile == split.tempfile && one.raw == split.raw && one.size == split.size;
        bool mem  = one.error == 0 && one.filecount == 0 && one.tempfile.empty() && one.raw == body &&
                    one.size == (long long)body.size();
        check("H4-framing-invariance-mem", same && mem,
              "same=" + std::to_string((int)same) + " mem=" + std::to_string((int)mem) +
                  " one.size=" + std::to_string(one.size) + " split.size=" + std::to_string(split.size) +
                  " one.fc=" + std::to_string(one.filecount) + " split.fc=" + std::to_string(split.filecount));
    }
    // H-C5：分帧 spill 前缀刷盘——阈值整段先留内存，末帧 +1 落盘，文件必须含前缀且不重不漏
    //        （cur_length 在内存帧已累加，spill 时既要把内存前缀写盘，又不能把它再计一次）
    {
        std::string body = blob(CAP, 'A') + "Z";
        Reading r        = run(body, FRAME);
        bool sized_ok    = r.size == (long long)body.size();     // 等于总长，非 2×阈值
        bool file_ok     = r.filebytes == body;                  // 逐字节，无重复前缀
        expect_spill("H5-prefix-flush-spill", r, body);
        check("H5b-cur-length-not-doubled", sized_ok && file_ok,
              "size=" + std::to_string(r.size) + " want=" + std::to_string(body.size()) +
                  " filedots=" + std::to_string(r.filebytes.size()));
    }

    std::printf("RP2 SUMMARY total=%u fail=%u\n", g_total, g_fail);
    clean_outdir();
    return g_fail > 0 ? 1 : 0;
}
