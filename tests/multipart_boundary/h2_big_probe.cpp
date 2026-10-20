// HTTP/2 侧大文件（>2MB 刷盘阈值）multipart 分包判据台。
// h2 一帧 = 一次 post_data_process，刷盘阈值是 2097152（比 h1 的 3145728 小）。
// 判据与 h1 臂一致：任意帧切分的结果必须与整帧（单帧）逐字段同解。
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

static const std::string BD = "MPBOUNDARY9";
static std::string g_outdir = "./temp/multipart_boundary_run/h2out/";

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
    int bdin         = 0;
    std::string tail16;
    std::string brief() const
    {
        return "err=" + std::to_string(error) + " fin=" + (finish ? "1" : "0") +
               " keys=" + std::to_string(keys) +
               " alpha[" + esc(alpha) + "] beta[" + esc(beta) + "]" +
               " upsize=" + std::to_string(upsize) + " bdin=" + std::to_string(bdin) +
               " tail[" + esc(tail16) + "]";
    }
};

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
    http::http2parse P;
    http::HTTP2_POST_DATA_T pd;
    pd.peer      = peer;
    pd.stream_id = 1;
    pd.boundary  = BD;
    pd.posttype  = 2;
    peer->posttype = 2;

    bool delivered_all = true;
    for (size_t k = 0; k < chunks.size(); k++)
    {
        if (peer->isfinish || P.error > 0)
        {
            delivered_all = false;
            break;
        }
        pd.content = std::string_view(chunks[k]);
        P.post_data_process(pd, (k + 1 == chunks.size()) ? 1 : 0);
    }
    r.error  = P.error;
    r.finish = peer->isfinish;
    if (delivered_all && P.error == 0)
    {
        peer->isfinish = true;
        r.finish       = true;
    }
    r.keys  = peer->post.size();
    r.alpha = peer->post["alpha"].to_string();
    r.beta  = peer->post["beta"].to_string();
    std::string upfile = peer->files["up"]["tempfile"].to_string();
    if (!upfile.empty())
    {
        std::string up = readfile(upfile);
        r.upsize       = (long long)up.size();
        r.bdin         = (int)(up.find("--" + BD) != std::string::npos);
        r.tail16       = up.size() > 16 ? up.substr(up.size() - 16) : up;
    }
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

struct Arm
{
    std::string name;
    std::string body;
    std::vector<unsigned int> cuts;
};

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code gpec;
    std::filesystem::create_directories(g_outdir, gpec);
    gv.temp_path = g_outdir;

    const unsigned int THRESH = 2097152;  // http2_parse.cpp 的刷盘阈值
    std::vector<Arm> arms;

    {
        std::string fill(THRESH + 5000, 'X');
        std::string body = tp("alpha", "AAA") + fp("up", "up.bin", fill) + tp("beta", "BBB") + "--" + BD + "--\r\n";
        size_t crlf      = nth(body, "\r\n--" + BD, 1);
        std::vector<unsigned int> cuts;
        for (unsigned int d = 0; d <= 12; d++)
            cuts.push_back((unsigned int)(crlf + d));
        size_t fin = body.find("--" + BD + "--");
        for (unsigned int d = 0; d <= 4; d++)
            cuts.push_back((unsigned int)(fin - 2 + d));
        cuts.push_back((unsigned int)(crlf - 100000));
        arms.push_back({"A-legit-bigfile", body, cuts});
        std::printf("ARM A body=%zu file_crlf=%zu close=%zu\n", body.size(), crlf, fin);
    }
    {
        std::string fill(2048, 'X');
        std::string body = tp("alpha", "AAA") + fp("up", "up.bin", fill) + tp("beta", "BBB") + "--" + BD + "--\r\n";
        size_t crlf      = nth(body, "\r\n--" + BD, 1);
        std::vector<unsigned int> cuts;
        for (unsigned int d = 0; d <= 12; d++)
            cuts.push_back((unsigned int)(crlf + d));
        arms.push_back({"B-legit-smallfile", body, cuts});
        std::printf("ARM B body=%zu file_crlf=%zu\n", body.size(), crlf);
    }
    {
        std::string fill(THRESH + 5000, 'X');
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

    unsigned int total_fail = 0;
    for (auto &arm : arms)
    {
        Reading golden = run(arm.body, {});
        std::printf("\n=== %s ===\nGOLDEN %s\n", arm.name.c_str(), golden.brief().c_str());
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
        total_fail += fail;
        std::printf("SUMMARY %s checked=%zu fail=%u\n", arm.name.c_str(), arm.cuts.size(), fail);
    }
    std::printf("\nH2BIGPROBE total_fail=%u\n", total_fail);
    clean_outdir();
    return total_fail > 0 ? 1 : 0;
}
