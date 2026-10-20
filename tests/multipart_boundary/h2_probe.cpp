// 离线分帧枚举台（HTTP/2 侧）：把同一个 multipart body 按每个字节位置切成两帧/三帧，
// 逐帧喂进 http2parse::post_data_process()，比对字段值与上传文件内容。
// 判据一 = 任意帧切分的结果必须与整帧一致；判据二 = 整帧读数必须等于按 body 手算的期望值。
// HTTP/2 的 content 就是单个 DATA 帧正文、field_offset 每帧归零，所以这里的"帧"与
// HTTP/1 探针里的"TCP 包"是同一类分段入口（DATA payload 上限 16384，上传必被切）。
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
// 末尾的 / 不能省：落盘名是直接拼在 temp_path 后面的，服务器读 conf 时也会补这个 /
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
    char buf[4096];
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
    bool finish        = false;
    int pfin           = -1;  // 解析器自己有没有认出收尾边界（h2 的收尾由 END_STREAM 落，见 run()）
    unsigned int keys  = 0;
    std::string alpha, gamma, beta;
    std::string up;
    long long upsize = 0;
    std::string raw;
    int pt  = -1;   // 解析用的 posttype
    int ppt = -1;   // peer->posttype（头侧声明）
    // 参与比对：只含解析器职责内的量（字段、文件、posttype、错误）
    std::string brief() const
    {
        return "err=" + std::to_string(error) +
               " keys=" + std::to_string(keys) + " pt=" + std::to_string(pt) +
               " ppt=" + std::to_string(ppt) + " alpha[" + esc(alpha) + "] gamma[" + esc(gamma) +
               "] beta[" + esc(beta) + "] up[" + esc(up) + "] upsize=" + std::to_string(upsize) +
               " raw[" + esc(raw) + "]";
    }
    // 只进日志读数，不进判据
    std::string info() const
    {
        return brief() + " fin=" + (finish ? "1" : "0") + " pfin=" + std::to_string(pfin);
    }
};

struct Want
{
    unsigned int keys   = 0;
    unsigned int error  = 0;
    int pt              = 2;
    std::string alpha, gamma, beta, up, raw;
    long long upsize = 0;
};

static std::string want_text(const Want &w)
{
    return "err=" + std::to_string(w.error) +
           " keys=" + std::to_string(w.keys) + " pt=" + std::to_string(w.pt) + " alpha[" + esc(w.alpha) +
           "] gamma[" + esc(w.gamma) + "] beta[" + esc(w.beta) + "] up[" + esc(w.up) +
           "] upsize=" + std::to_string(w.upsize) + " raw[" + esc(w.raw) + "]";
}
static std::string got_text(const Reading &r)
{
    Want w;
    w.keys   = r.keys;
    w.error  = r.error;
    w.pt     = r.pt;
    w.alpha  = r.alpha;
    w.gamma  = r.gamma;
    w.beta   = r.beta;
    w.up     = r.up;
    w.upsize = r.upsize;
    w.raw    = r.raw;
    return want_text(w);
}

struct Case
{
    std::string name;
    bool has_boundary = true;  // 头侧是否解析出 boundary（Content-Type 带不带参数）
    bool declared     = false; // 头侧声明 multipart（callposttype 只写 peer->posttype）
    bool force_pt2    = false; // 直接把体侧 posttype 置 2，绕过嗅探
    bool judge        = true;  // 参与"分帧不变"判据
    bool pairs        = true;  // 枚举两处分帧
    bool has_want     = false;
    Want want;
    std::string body;
};

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

// 一帧 = 一次 post_data_process 调用，最后一帧带 END_STREAM 语义（islast_pack=1）
static Reading run(const Case &cs, const std::vector<unsigned int> &cuts)
{
    Reading r;
    clean_outdir();

    std::vector<std::string> chunks;
    unsigned int prev = 0;
    for (unsigned int c : cuts)
    {
        if (c > prev && c <= cs.body.size())
        {
            chunks.push_back(cs.body.substr(prev, c - prev));
            prev = c;
        }
    }
    chunks.push_back(cs.body.substr(prev));

    auto peer      = std::make_shared<http::httppeer>();
    http::http2parse P;
    http::HTTP2_POST_DATA_T pd;
    pd.peer      = peer;
    pd.stream_id = 1;
    if (cs.has_boundary)
        pd.boundary = BD;
    if (cs.declared)
        peer->posttype = 2;   // h2 的 callposttype 只做这一件事
    if (cs.force_pt2)
        pd.posttype = 2;

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

    r.error = P.error;
    r.pfin  = peer->isfinish ? 1 : 0;
    // readpostdata() 在 END_STREAM 那一帧处理完之后无条件把这条流的 isfinish 置 true，
    // 解析器自己认不认出收尾边界都不影响这件事，所以这里照做，isfinish 不进判据。
    if (delivered_all && P.error == 0)
    {
        peer->isfinish = true;
    }
    r.finish = peer->isfinish;
    r.pt     = pd.posttype;
    r.ppt    = peer->posttype;
    r.keys   = peer->post.size();
    r.alpha  = peer->post["alpha"].to_string();
    r.gamma  = peer->post["gamma"].to_string();
    r.beta   = peer->post["beta"].to_string();
    std::string upfile = peer->files["up"]["tempfile"].to_string();
    if (!upfile.empty())
    {
        r.up     = readfile(upfile);
        r.upsize = peer->files["up"]["size"].to_int();
    }
    std::string rawfile = peer->files["rawcontent"]["tempfile"].to_string();
    if (!rawfile.empty())
        r.raw = readfile(rawfile);
    return r;
}

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code gec;
    std::filesystem::create_directories(g_outdir, gec);
    gv.temp_path = g_outdir;

    const std::string close_ = "--" + BD + "--\r\n";
    std::vector<Case> cases;

    auto push = [&](const std::string &name, const std::string &body, Want w)
    {
        Case cs;
        cs.name      = name;
        cs.body      = body;
        cs.has_want  = true;
        cs.want      = w;
        cases.push_back(cs);
    };

    {
        Want w;
        w.keys  = 2;
        w.alpha = "AAA";
        w.beta  = "BBB";
        push("A-plain", tp("alpha", "AAA") + tp("beta", "BBB") + close_, w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "x--y";
        w.beta  = "BBB";
        push("B-lone-dashes", tp("alpha", "x--y") + tp("beta", "BBB") + close_, w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "AA-";
        w.beta  = "BBB";
        push("C-trail-dash", tp("alpha", "AA-") + tp("beta", "BBB") + close_, w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "AA--";
        w.beta  = "BBB";
        push("D-trail-2dash", tp("alpha", "AA--") + tp("beta", "BBB") + close_, w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "A\r\nB";
        w.beta  = "CCC";
        push("E-value-crlf", tp("alpha", "A\r\nB") + tp("beta", "CCC") + close_, w);
    }
    {
        Want w;
        w.keys  = 3;
        w.alpha = "AAA";
        w.gamma = "GGG";
        w.beta  = "BBB";
        push("F-three-fields", tp("alpha", "AAA") + tp("gamma", "GGG") + tp("beta", "BBB") + close_, w);
    }
    {
        Want w;
        w.keys   = 1;
        w.alpha  = "AAA";
        w.up     = "FILEDATA-123";
        w.upsize = 12;
        push("G-filepart", tp("alpha", "AAA") + fp("up", "up.bin", "FILEDATA-123") + close_, w);
    }
    {
        Want w;
        w.keys   = 1;
        w.beta   = "BBB";
        w.up     = "AAAA";
        w.upsize = 4;
        push("H-file-and-text", fp("up", "up.bin", "AAAA") + tp("beta", "BBB") + close_, w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "PFX--" + BD + "\r\nTAIL";
        w.beta  = "BBB";
        push("I-embed-delim", tp("alpha", w.alpha) + tp("beta", "BBB") + close_, w);
    }
    {
        // 值与分隔符之间只有一个 \n：缺 \r 就不算分隔符，整段粘成 alpha 一个值，
        // 直到下一个真正带 CRLF 的分隔符（这里是闭合那条）才切。
        Want w;
        w.keys  = 1;
        w.alpha = "AAA\n--" + BD + "\r\nContent-Disposition: form-data; name=\"beta\"\r\n\r\nBBB";
        push("J-lf-only-delim",
             "--" + BD + "\r\nContent-Disposition: form-data; name=\"alpha\"\r\n\r\nAAA\n" + tp("beta", "BBB") + close_, w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "PFX--" + BD + "SFX";
        w.beta  = "BBB";
        push("K-embed-bare", tp("alpha", w.alpha) + tp("beta", "BBB") + close_, w);
    }
    {
        // 头侧完全没声明（无 Content-Type 语义）：body 原样落盘，不猜 multipart
        Want w;
        w.keys   = 0;
        w.pt     = 5;
        w.raw    = tp("alpha", "AAA") + tp("beta", "BBB") + close_;
        Case cs;
        cs.name         = "L-sniff-multipart";
        cs.has_boundary = false;
        cs.body         = w.raw;
        cs.has_want     = true;
        cs.want         = w;
        cases.push_back(cs);
    }
    {
        Want w;
        w.keys   = 0;
        w.pt     = 5;
        w.raw    = "-----BEGIN PRIVATE KEY-----\nabc\n-----END PRIVATE KEY-----\n";
        Case cs;
        cs.name         = "N-sniff-pem";
        cs.has_boundary = false;
        cs.body         = w.raw;
        cs.has_want     = true;
        cs.want         = w;
        cases.push_back(cs);
    }
    {
        // 头里声明 multipart 却没带 boundary：h2 的体侧 posttype 只由嗅探决定，
        // 所以这里落到 raw（HTTP/1 同条件落到体侧 40099）。手算期望 = raw。
        Want w;
        w.keys   = 0;
        w.pt     = 5;
        w.raw    = tp("alpha", "AAA") + tp("beta", "BBB") + close_;
        Case cs;
        cs.name         = "O-declared-noboundary";
        cs.has_boundary = false;
        cs.declared     = true;
        cs.body         = w.raw;
        cs.has_want     = true;
        cs.want         = w;
        cases.push_back(cs);
    }
    {
        // 绕过嗅探、体侧直接按 multipart 走（对应 callposttype 放开 pd.posttype=2 的形状）：
        // 没有 boundary 必须硬失败，否则正文里每个 "--" 都会当分隔符切。
        Want w;
        w.keys   = 0;
        w.pt     = 2;
        w.error  = 40099;
        Case cs;
        cs.name         = "P-forced-noboundary";
        cs.has_boundary = false;
        cs.declared     = true;
        cs.force_pt2    = true;
        cs.body         = tp("alpha", "AAA") + tp("beta", "BBB") + close_;
        cs.has_want     = true;
        cs.want         = w;
        cases.push_back(cs);
    }
    // 读数臂：只打印整帧读数，不参与判据
    auto push_ro = [&](const std::string &name, const std::string &body)
    {
        Case cs;
        cs.name  = name;
        cs.body  = body;
        cs.judge = false;
        cs.pairs = false;
        cases.push_back(cs);
    };
    push_ro("R-preamble-declared", "PREAMBLE\r\n" + tp("alpha", "AAA") + tp("beta", "BBB") + close_);
    push_ro("R-closedash-split", tp("alpha", "AAA") + close_);
    push_ro("R-cr-only-delim", "--" + BD + "\r\nContent-Disposition: form-data; name=\"alpha\"\r\n\r\nAAA\r"
                                  "--" + BD + "\r\nContent-Disposition: form-data; name=\"beta\"\r\n\r\nBBB\r\n"
                                  + close_);

    std::printf("H2PROBE begin cases=%zu\n", cases.size());
    unsigned int total_fail = 0;
    unsigned int abs_fail   = 0;
    for (auto &cs : cases)
    {
        Reading golden = run(cs, {});
        std::printf("GOLDEN %s size=%zu %s\n", cs.name.c_str(), cs.body.size(), golden.info().c_str());
        if (cs.has_want)
        {
            if (got_text(golden) != want_text(cs.want))
            {
                abs_fail++;
                std::printf("ABSFAIL %s want=%s got=%s\n", cs.name.c_str(), want_text(cs.want).c_str(),
                            got_text(golden).c_str());
            }
            else
            {
                std::printf("ABSOK %s %s\n", cs.name.c_str(), got_text(golden).c_str());
            }
        }
        if (!cs.judge)
            continue;

        unsigned int n    = (unsigned int)cs.body.size();
        unsigned int pass = 0, fail = 0;
        for (unsigned int i = 1; i < n; i++)
        {
            Reading r = run(cs, {i});
            if (r.brief() != golden.brief())
            {
                fail++;
                std::printf("FAIL %s cut=%u got=%s\n", cs.name.c_str(), i, r.info().c_str());
            }
            else
                pass++;
        }
        if (cs.pairs)
        {
            unsigned int step = (n > 120) ? 3 : 1;
            for (unsigned int i = 1; i < n; i += step)
            {
                for (unsigned int j = i + 1; j < n; j += step)
                {
                    Reading r = run(cs, {i, j});
                    if (r.brief() != golden.brief())
                    {
                        fail++;
                        std::printf("FAIL %s cut=%u,%u got=%s\n", cs.name.c_str(), i, j, r.info().c_str());
                    }
                    else
                        pass++;
                }
            }
        }
        total_fail += fail;
        std::printf("SUMMARY %s checked=%u pass=%u fail=%u\n", cs.name.c_str(), pass + fail, pass, fail);
    }
    total_fail += abs_fail;
    std::printf("H2PROBE abs_fail=%u total_fail=%u\n", abs_fail, total_fail);
    clean_outdir();
    return total_fail > 0 ? 1 : 0;
}
