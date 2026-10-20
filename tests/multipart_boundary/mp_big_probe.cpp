// 大文件（>3MB 刷盘阈值）multipart 上传的分包判据台。
// 刷盘把 field_item 清空，boundary 前面的 CRLF 可能因此"看不见"（prev_is 返回 -1）。
// 本探针把切点铺在真正的 \r\n--boundary 前后，比较每一种分段与整包读数是否一致，
// 用来分别回答两个问题：
//   1) 数据里混进 "\n--boundary"（LF 没有 CR）时会不会被误当边界（P1 想修的）
//   2) 真边界的 CRLF 恰好断在包缝上时，会不会反过来把真边界吞掉（P1 可能新引入的）
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

static const std::string BD = "MPBOUNDARY9";
static std::string g_outdir = "./temp/multipart_boundary_run/out/";

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

struct Reading
{
    unsigned int error = 0;
    bool finish       = false;
    unsigned int keys = 0;
    std::string alpha, beta;
    long long upsize = -1;
    // 文件正文里是否混进了分隔符文本（真边界被当数据吞掉的直接证据）
    int bdin = 0;
    std::string tail16;  // 文件末尾 16 字节，看多出来的 CRLF/LF
    std::string files;   // 每个文件 part 各自的字节数，D 臂用得上
    // 只打进 GOLDEN 行，不参与比对：临时文件名带随机串，比了对不对都吵
    std::string paths;
    std::string brief() const
    {
        return "err=" + std::to_string(error) + " fin=" + (finish ? "1" : "0") +
               " keys=" + std::to_string(keys) +
               " alpha[" + esc(alpha) + "] beta[" + esc(beta) + "]" +
               " upsize=" + std::to_string(upsize) + " bdin=" + std::to_string(bdin) +
               " tail[" + esc(tail16) + "] files[" + files + "]";
    }
};

static std::string make_hdr(unsigned long long clen)
{
    return "POST /mp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: multipart/form-data; boundary=" + BD +
           "\r\nContent-Length: " + std::to_string(clen) + "\r\n\r\n";
}

static Reading run(const std::string &body, const std::vector<unsigned int> &cuts)
{
    Reading r;
    clean_outdir();

    std::vector<std::string> chunks;
    unsigned int prev = 0;
    for (unsigned int c : cuts)
    {
        if (c > prev && c <= body.size())
        {
            chunks.push_back(body.substr(prev, c - prev));
            prev = c;
        }
    }
    chunks.push_back(body.substr(prev));

    auto peer = std::make_shared<http::httppeer>();
    http::httpparse P;
    P.setpeer(peer);

    std::string hdr = make_hdr(body.size());
    unsigned int guard = 0;
    while (!P.isfinish_header && P.error == 0 && P.readoffset < hdr.size() && ++guard < 128)
        P.read_http_header_block((const unsigned char *)hdr.data(), (unsigned int)hdr.size());
    if (P.error > 0 || !P.isfinish_header)
    {
        r.error = P.error > 0 ? P.error : 9001;
        return r;
    }
    P.http_content_length = 0;

    for (auto &chunk : chunks)
    {
        if (peer->isfinish || P.error > 0)
            break;
        P.readoffset = 0;
        P.read_http_post_block((const unsigned char *)chunk.data(), (unsigned int)chunk.size());
    }

    r.error  = P.error;
    r.finish = peer->isfinish;
    r.keys   = peer->post.size();
    r.alpha  = peer->post["alpha"].to_string();
    r.beta   = peer->post["beta"].to_string();
    std::string upfile = peer->files["up"]["tempfile"].to_string();
    if (!upfile.empty())
    {
        std::string up = readfile(upfile);
        r.upsize       = (long long)up.size();
        r.bdin         = (int)(up.find("--" + BD) != std::string::npos);
        r.tail16       = up.size() > 16 ? up.substr(up.size() - 16) : up;
    }
    for (const char *nm : {"up", "up2"})
    {
        std::string f = peer->files[nm]["tempfile"].to_string();
        if (f.empty())
            continue;
        std::string content = readfile(f);
        char last           = content.empty() ? '?' : content.back();
        r.files += std::string(nm) + "=" + std::to_string(content.size()) + ":" + std::string(1, last) + " ";
        std::string stem = f.substr(f.find_last_of('/') + 1);
        // 名字必须仍被 httpwatch 的临时件白名单认出来，否则改名等于把上传件变成永久残留
        r.paths += std::string(nm) + "=" + stem +
                   (http::is_http_temp_filename(stem) ? ":ok " : ":NOTWHITELIST ");
    }
    std::error_code ec;
    unsigned int nfiles = 0;
    if (std::filesystem::exists(g_outdir, ec))
    {
        for (auto &e : std::filesystem::directory_iterator(g_outdir, ec))
        {
            (void)e;
            nfiles++;
        }
    }
    r.paths += "NF=" + std::to_string(nfiles);
    return r;
}

static std::string tp(const std::string &name, const std::string &value)
{
    return "--" + BD + "\r\nContent-Disposition: form-data; name=\"" + name + "\"\r\n\r\n" + value + "\r\n";
}
static std::string fp(const std::string &name, const std::string &file, const std::string &value)
{
    return "--" + BD + "\r\nContent-Disposition: form-data; name=\"" + name + "\"; filename=\"" + file + "\"\r\n"
           "Content-Type: application/octet-stream\r\n\r\n" +
           value + "\r\n";
}

struct Arm
{
    std::string name;
    std::string body;
    // 在这些下标处各切一刀（单切点），逐条与整包读数比对
    std::vector<unsigned int> cuts;
    // false = 只出 READONLY 行不进总数：临时上传名撞车时整包参照本身被污染，没有可比性。
    // 撞车已在 make_http_temp_upload_name 修掉（微秒时间戳 + 进程内自增号），这个开关留着当旋钮。
    bool gate = true;
};

// 在 body 里找第 occ 次出现的 needle，返回其起始下标；找不到返回 SIZE_MAX
static size_t nth(const std::string &body, const std::string &needle, unsigned int occ)
{
    size_t pos = std::string::npos;
    for (unsigned int i = 0; i <= occ; i++)
    {
        pos = body.find(needle, pos + 1);
        if (pos == std::string::npos)
            return SIZE_MAX;
    }
    return pos;
}

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code gpec;
    std::filesystem::create_directories(g_outdir, gpec);
    gv.temp_path = g_outdir;

    const unsigned int THRESH = 3145728;  // http_parse.cpp 的刷盘阈值
    std::vector<Arm> arms;

    // ---- A: 合法大文件上传，真边界 CRLF。期望所有分段与整包同解 ----
    {
        std::string fill(THRESH + 5000, 'X');
        std::string body = tp("alpha", "AAA") + fp("up", "up.bin", fill) + tp("beta", "BBB") + "--" + BD + "--\r\n";
        size_t crlf = nth(body, "\r\n--" + BD, 1);  // 文件 part 收尾的那个 CRLF
        std::vector<unsigned int> cuts;
        if (crlf != SIZE_MAX)
        {
            for (unsigned int d = 0; d <= 12; d++)
                cuts.push_back((unsigned int)(crlf + d));  // crlf, crlf+1 = 切在 \r 与 \n 之间
        }
        // 顺带扫闭合标记前后
        size_t fin = body.find("--" + BD + "--");
        for (unsigned int d = 0; d <= 4; d++)
            cuts.push_back((unsigned int)(fin - 2 + d));
        // 深埋在文件中间的切点：远端 CRLF 落在下一包里，理应无恙（对照）
        cuts.push_back((unsigned int)(crlf - 100000));
        cuts.push_back((unsigned int)(crlf - THRESH));
        arms.push_back({"A-legit-bigfile", body, cuts});
        std::printf("ARM A body=%zu file_crlf=%zu close=%zu\n", body.size(), crlf, fin);
    }

    // ---- B: 小文件对照（不触发刷盘），同样的切点 ----
    {
        std::string fill(2048, 'X');
        std::string body = tp("alpha", "AAA") + fp("up", "up.bin", fill) + tp("beta", "BBB") + "--" + BD + "--\r\n";
        size_t crlf = nth(body, "\r\n--" + BD, 1);
        std::vector<unsigned int> cuts;
        for (unsigned int d = 0; d <= 12; d++)
            cuts.push_back((unsigned int)(crlf + d));
        arms.push_back({"B-legit-smallfile", body, cuts});
        std::printf("ARM B body=%zu file_crlf=%zu\n", body.size(), crlf);
    }

    // ---- C: P1 靶心，刷盘后文件里混进 LF-only 的真 boundary 文本 ----
    {
        std::string fill(THRESH + 5000, 'X');
        // 先放 3MB+，再放 "\n--BD\r\n...name=evil...\r\n" —— 刷盘后这段的 CRLF 前缀看不见
        std::string evil = "\n--" + BD + "\r\nContent-Disposition: form-data; name=\"alpha\"\r\n\r\nEVIL\r\n";
        std::string body = fp("up", "up.bin", fill) + evil + tp("beta", "BBB") + "--" + BD + "--\r\n";
        size_t evpos     = body.find(evil);
        std::vector<unsigned int> cuts;
        for (unsigned int d = 0; d <= 12; d++)
            cuts.push_back((unsigned int)(evpos + d));
        cuts.push_back((unsigned int)(evpos - 1));
        arms.push_back({"C-lf-only-in-bigfile", body, cuts});
        std::printf("ARM C body=%zu evil=%zu\n", body.size(), evpos);
    }

    // ---- D: 两个大文件 part，第二个 part 的收尾 CRLF 也铺切点 ----
    {
        std::string f1(THRESH + 300, 'Y');
        std::string f2(THRESH + 900, 'Z');
        std::string body = "--" + BD +
                           "\r\nContent-Disposition: form-data; name=\"up\"; filename=\"a.bin\"\r\n"
                           "Content-Type: application/octet-stream\r\n\r\n" +
                           f1 + "\r\n" +
                           "--" + BD +
                           "\r\nContent-Disposition: form-data; name=\"up2\"; filename=\"b.bin\"\r\n"
                           "Content-Type: application/octet-stream\r\n\r\n" +
                           f2 + "\r\n--" + BD + "--\r\n";
        size_t crlf = nth(body, "\r\n--" + BD, 1);
        std::vector<unsigned int> cuts;
        for (unsigned int d = 0; d <= 12; d++)
            cuts.push_back((unsigned int)(crlf + d));
        arms.push_back({"D-two-bigfiles", body, cuts});
        std::printf("ARM D body=%zu second_crlf=%zu\n", body.size(), crlf);
    }

    // ---- E: 两个小文件 part（远不跨刷盘阈值），只看同一请求里两个临时文件会不会撞名 ----
    {
        std::string f1(100, 'Y');
        std::string f2(200, 'Z');
        std::string body = "--" + BD +
                           "\r\nContent-Disposition: form-data; name=\"up\"; filename=\"a.bin\"\r\n"
                           "Content-Type: application/octet-stream\r\n\r\n" +
                           f1 + "\r\n" +
                           "--" + BD +
                           "\r\nContent-Disposition: form-data; name=\"up2\"; filename=\"b.bin\"\r\n"
                           "Content-Type: application/octet-stream\r\n\r\n" +
                           f2 + "\r\n--" + BD + "--\r\n";
        std::printf("\n=== E-small-two-files x3 ===\n");
        for (int i = 0; i < 3; i++)
        {
            Reading g = run(body, {});
            std::printf("E%d %s | %s\n", i, g.brief().c_str(), g.paths.c_str());
        }
        arms.push_back({"E-small-two-files", body, {}});
    }

    unsigned int total_fail = 0;
    for (auto &arm : arms)
    {
        Reading golden = run(arm.body, {});
        std::printf("\n=== %s ===\nGOLDEN %s | %s\n", arm.name.c_str(), golden.brief().c_str(),
                    golden.paths.c_str());
        unsigned int fail = 0;
        for (unsigned int c : arm.cuts)
        {
            if (c == 0 || c >= arm.body.size())
                continue;
            Reading r = run(arm.body, {c});
            if (r.brief() != golden.brief())
            {
                fail++;
                std::printf("DIFF cut=%u (byte_before=[%s]) got=%s\n", c,
                            esc(std::string_view(arm.body).substr(c - 1, 2)).c_str(), r.brief().c_str());
            }
            else
            {
                std::printf("OK   cut=%u\n", c);
            }
        }
        total_fail += arm.gate ? fail : 0;
        std::printf("%s %s checked=%zu fail=%u\n", arm.gate ? "SUMMARY" : "READONLY", arm.name.c_str(),
                    arm.cuts.size(), fail);
    }
    std::printf("\nBIGPROBE total_fail=%u\n", total_fail);
    clean_outdir();
    return total_fail > 0 ? 1 : 0;
}
