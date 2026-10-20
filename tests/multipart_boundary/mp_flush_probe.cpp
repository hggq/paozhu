// 大文件刷盘之后，boundary 前缀跨包的边界测试。
// HTTP/1.1 的文件 part 攒过 3145728 字节就落盘并清空 field_item，此后分隔符前面的
// CRLF 可能只剩下一个包开头的 \n，也可能整条 \r\n 都落在上一包。
// 判据是相对式的：每一种切包读数必须和整包读数逐字节相同，
// 因为"该不该切成边界"由 http_parse.cpp 的 CRLF 校验决定，本文件不重复那条结论。
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
    std::string tail16;  // 文件末尾若干字节，多出来的 CRLF/LF 在这里现形
    std::string brief() const
    {
        return "err=" + std::to_string(error) + " fin=" + (finish ? "1" : "0") +
               " keys=" + std::to_string(keys) + " alpha[" + esc(alpha) + "] beta[" + esc(beta) + "]" +
               " upsize=" + std::to_string(upsize) + " tail[" + esc(tail16) + "]";
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
    http::httpparse P;
    P.setpeer(peer);

    std::string hdr = "POST /mp HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: multipart/form-data; boundary=" + BD +
                      "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n";
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
        r.tail16       = up.size() > 24 ? up.substr(up.size() - 24) : up;
    }
    return r;
}

static std::string fp(const std::string &name, const std::string &file, const std::string &value)
{
    return "--" + BD + "\r\nContent-Disposition: form-data; name=\"" + name + "\"; filename=\"" + file + "\"\r\n"
           "Content-Type: application/octet-stream\r\n\r\n" +
           value + "\r\n";
}

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code gpec;
    std::filesystem::create_directories(g_outdir, gpec);
    gv.temp_path = g_outdir;

    unsigned int fail = 0;

    // --- 用例1：刷盘后文件里混进 LF 开头的分隔符文本 ---
    // 假分隔符写成 \n--BD\r\n 加一段合法头，语法上只差前面那个 \r。
    {
        const unsigned int THRESH = 3145728;
        std::string fake = "\n--" + BD + "\r\nContent-Disposition: form-data; name=\"alpha\"\r\n\r\nEVIL";
        std::string body = fp("up", "up.bin", std::string(THRESH + 1000, 'X') + fake) +
                           "--" + BD + "\r\nContent-Disposition: form-data; name=\"beta\"\r\n\r\nBBB\r\n--" + BD +
                           "--\r\n";
        size_t evpos = body.find(fake);
        size_t hdrsz = body.find(std::string(THRESH + 1000, 'X'));

        std::printf("=== case1 刷盘后 LF-only 假分隔符 body=%zu fake=%zu ===\n", body.size(), evpos);
        Reading golden = run(body, {});
        std::printf("GOLDEN %s\n", golden.brief().c_str());

        std::vector<unsigned int> cuts;
        cuts.push_back((unsigned int)(hdrsz + THRESH + 10));  // 文件值中间，刷盘在切点之前发生
        for (unsigned int d = 0; d <= 12; d++)
            cuts.push_back((unsigned int)(evpos + d));  // 覆盖 \n 收尾、\n| 之间、--BD 之内
        cuts.push_back((unsigned int)(evpos - 1));

        for (unsigned int c : cuts)
        {
            if (c == 0 || c >= body.size())
                continue;
            Reading r = run(body, {c});
            if (r.brief() == golden.brief())
            {
                std::printf("PASS cut=%u\n", c);
            }
            else
            {
                std::printf("FAIL cut=%u got=%s\n", c, r.brief().c_str());
                fail++;
            }
        }
        std::printf("RESULT case1 fail=%u\n", fail);
    }

    // --- 用例2：真分隔符的 CRLF 恰好断在包缝上 ---
    {
        const unsigned int THRESH = 3145728;
        std::string body = fp("up", "up.bin", std::string(THRESH + 1000, 'X')) +
                           "--" + BD + "\r\nContent-Disposition: form-data; name=\"beta\"\r\n\r\nBBB\r\n--" + BD +
                           "--\r\n";
        size_t crlf = body.rfind("\r\n--" + BD);
        std::printf("\n=== case2 真分隔符 CRLF 断包 crlf=%zu ===\n", crlf);
        Reading golden = run(body, {});
        std::printf("GOLDEN %s\n", golden.brief().c_str());
        unsigned int casefail = 0;
        for (unsigned int d = 0; d <= 12; d++)
        {
            unsigned int c = (unsigned int)(crlf + d);
            if (c == 0 || c >= body.size())
                continue;
            Reading r = run(body, {c});
            if (r.brief() == golden.brief())
            {
                std::printf("PASS cut=%u\n", c);
            }
            else
            {
                std::printf("FAIL cut=%u got=%s\n", c, r.brief().c_str());
                casefail++;
                fail++;
            }
        }
        std::printf("RESULT case2 fail=%u\n", casefail);
    }

    // --- 用例3：小文件正例，两个字段都要出 ---
    {
        std::string body = "--" + BD + "\r\nContent-Disposition: form-data; name=\"alpha\"\r\n\r\nAAA\r\n"
                           "--" + BD + "\r\nContent-Disposition: form-data; name=\"beta\"\r\n\r\nBBB\r\n"
                           "--" + BD + "--\r\n";
        std::printf("\n=== case3 小文件正例 ===\n");
        Reading r = run(body, {});
        std::printf("READ %s\n", r.brief().c_str());
        unsigned int casefail = 0;
        if (r.error != 0)
        {
            std::printf("FAIL case3: err=%u want=0\n", r.error);
            casefail++;
        }
        if (r.keys != 2)
        {
            std::printf("FAIL case3: keys=%u want=2\n", r.keys);
            casefail++;
        }
        if (r.alpha != "AAA" || r.beta != "BBB")
        {
            std::printf("FAIL case3: alpha[%s] beta[%s]\n", esc(r.alpha).c_str(), esc(r.beta).c_str());
            casefail++;
        }
        fail += casefail;
        std::printf("RESULT case3 fail=%u\n", casefail);
    }

    clean_outdir();
    std::printf("\nRESULT fail=%u\n", fail);
    return fail > 0 ? 1 : 0;
}
