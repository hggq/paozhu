// 离线分包枚举台：把同一个 multipart body 按每个字节位置切成两包/三包喂进解析器，
// 逐字节比对字段值与上传文件内容。判据 = 任意 TCP 分段的结果必须与整包一致。
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
// 末尾的 / 不能省：落盘名是直接拼在 temp_path 后面的，服务器读 conf 时也会补这个 /
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
    bool finish = false;
    unsigned int keys = 0;
    std::string alpha, gamma, beta;
    std::string up;      // 上传文件正文
    long long upsize = 0;
    std::string raw;     // raw 回落落盘正文
    long long cl = -1;   // peer->content_length
    int pt = -1;         // 解析器 posttype
    int ihead = -1;      // isfinish_header
    int ppt = -1;        // peer->posttype
    std::string brief() const
    {
        std::string s = "err=" + std::to_string(error) + " fin=" + (finish ? "1" : "0") +
                        " keys=" + std::to_string(keys) +
                        " cl=" + std::to_string(cl) + " pt=" + std::to_string(pt) +
                        " ppt=" + std::to_string(ppt) + " ih=" + std::to_string(ihead) +
                        " alpha[" + esc(alpha) + "] gamma[" + esc(gamma) + "] beta[" + esc(beta) + "]" +
                        " up[" + esc(up) + "] upsize=" + std::to_string(upsize) +
                        " raw[" + esc(raw) + "]";
        return s;
    }
};

// 三段式头：Content-Type 可为空（不发该头）
static std::string make_hdr(const std::string &ctype, unsigned long long clen)
{
    std::string h = "POST /mp HTTP/1.1\r\nHost: 127.0.0.1\r\n";
    if (!ctype.empty())
        h += "Content-Type: " + ctype + "\r\n";
    h += "Content-Length: " + std::to_string(clen) + "\r\n\r\n";
    return h;
}

static Reading run(const std::string &ctype, const std::string &body, const std::vector<unsigned int> &cuts)
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

    std::string hdr = make_hdr(ctype, body.size());
    unsigned int guard = 0;
    while (!P.isfinish_header && P.error == 0 && P.readoffset < hdr.size() && ++guard < 128)
        P.read_http_header_block((const unsigned char *)hdr.data(), (unsigned int)hdr.size());
    if (P.error > 0 || !P.isfinish_header)
    {
        r.error = P.error > 0 ? P.error : 9001;
        r.cl = (long long)peer->content_length;
        r.pt = P.posttype;
        r.ppt = peer->posttype;
        r.ihead = P.isfinish_header ? 1 : 0;
        return r;
    }
    // process() 在头完成处把 http_content_length 归零（它是 body 已消费计数的起点），
    // 直接驱动 read_http_post_block 必须照做，否则 body 会被按"头字节数"截断。
    P.http_content_length = 0;

    for (auto &chunk : chunks)
    {
        if (peer->isfinish || P.error > 0)
            break;
        P.readoffset = 0;
        P.read_http_post_block((const unsigned char *)chunk.data(), (unsigned int)chunk.size());
    }

    r.error = P.error;
    r.finish = peer->isfinish;
    r.cl = (long long)peer->content_length;
    r.pt = P.posttype;
    r.ppt = peer->posttype;
    r.ihead = P.isfinish_header ? 1 : 0;
    r.keys = peer->post.size();
    r.alpha = peer->post["alpha"].to_string();
    r.gamma = peer->post["gamma"].to_string();
    r.beta = peer->post["beta"].to_string();
    std::string upfile = peer->files["up"]["tempfile"].to_string();
    if (!upfile.empty())
    {
        r.up = readfile(upfile);
        r.upsize = peer->files["up"]["size"].to_int();
    }
    std::string rawfile = peer->files["rawcontent"]["tempfile"].to_string();
    if (!rawfile.empty())
        r.raw = readfile(rawfile);
    return r;
}

struct Want
{
    unsigned int keys = 0;
    bool finish = true;
    unsigned int error = 0;
    std::string alpha, gamma, beta, up, raw;
    long long upsize = 0;
};

// 只比"应该是什么"的那几个字段：整包读数必须逐字段等于按 body 手算出来的期望值
static std::string want_text(const Want &w)
{
    return "err=" + std::to_string(w.error) + " fin=" + (w.finish ? "1" : "0") +
           " keys=" + std::to_string(w.keys) +
           " alpha[" + esc(w.alpha) + "] gamma[" + esc(w.gamma) + "] beta[" + esc(w.beta) + "]" +
           " up[" + esc(w.up) + "] upsize=" + std::to_string(w.upsize) + " raw[" + esc(w.raw) + "]";
}
static std::string got_text(const Reading &r)
{
    Want w;
    w.keys   = r.keys;
    w.finish = r.finish;
    w.error  = r.error;
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
    std::string ctype;
    std::string body;
    bool judge = true;  // true = 参与"分段不变"判据
    bool pairs = true;  // 是否枚举两处分段
    bool has_want = false;
    Want want;
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

int main()
{
    http::server_loaclvar &gv = http::get_server_global_var();
    std::error_code gpec;
    std::filesystem::create_directories(g_outdir, gpec);
    gv.temp_path = g_outdir;

    std::vector<Case> cases;
    // 判据臂：分段必须与整包逐字节同解，且整包解必须等于下面手算的期望值
    auto push = [&](const std::string &name, const std::string &body, Want w)
    {
        Case cs;
        cs.name     = name;
        cs.ctype    = "multipart/form-data; boundary=" + BD;
        cs.body     = body;
        cs.has_want = true;
        cs.want     = w;
        cases.push_back(cs);
    };
    {
        Want w;
        w.keys = 2;
        w.alpha = "AAA";
        w.beta  = "BBB";
        push("A-plain", tp("alpha", "AAA") + tp("beta", "BBB") + "--" + BD + "--\r\n", w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "x--y";
        w.beta  = "BBB";
        push("B-lone-dashes", tp("alpha", "x--y") + tp("beta", "BBB") + "--" + BD + "--\r\n", w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "AA-";
        w.beta  = "BBB";
        push("C-trail-dash", tp("alpha", "AA-") + tp("beta", "BBB") + "--" + BD + "--\r\n", w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "AA--";
        w.beta  = "BBB";
        push("D-trail-2dash", tp("alpha", "AA--") + tp("beta", "BBB") + "--" + BD + "--\r\n", w);
    }
    {
        Want w;
        w.keys  = 2;
        w.alpha = "A\r\nB";
        w.beta  = "CCC";
        push("E-value-crlf", tp("alpha", "A\r\nB") + tp("beta", "CCC") + "--" + BD + "--\r\n", w);
    }
    {
        Want w;
        w.keys  = 3;
        w.alpha = "AAA";
        w.gamma = "GGG";
        w.beta  = "BBB";
        push("F-three-fields",
             tp("alpha", "AAA") + tp("gamma", "GGG") + tp("beta", "BBB") + "--" + BD + "--\r\n", w);
    }
    {
        Want w;
        w.keys   = 1;
        w.alpha  = "AAA";
        w.up     = "FILEDATA-123";
        w.upsize = 12;
        push("G-filepart", tp("alpha", "AAA") + fp("up", "up.bin", "FILEDATA-123") + "--" + BD + "--\r\n", w);
    }
    {
        Want w;
        w.keys   = 1;
        w.beta   = "BBB";
        w.up     = "AAAA";
        w.upsize = 4;
        push("H-file-and-text", fp("up", "up.bin", "AAAA") + tp("beta", "BBB") + "--" + BD + "--\r\n", w);
    }
    {
        // 数据里正好含一条"看起来像分隔符"的行：它前面不是 CRLF，整段都是 alpha 的值
        Want w;
        w.keys  = 2;
        w.alpha = "PFX--" + BD + "\r\nTAIL";
        w.beta  = "BBB";
        push("I-embed-delim", tp("alpha", w.alpha) + tp("beta", "BBB") + "--" + BD + "--\r\n", w);
    }
    {
        // 值与分隔符之间只有一个 \n（没有 \r）：畸形框架。
        // 分隔符前面必须是完整 CRLF，缺 \r 就退回当普通数据，整段粘成 alpha 一个值，
        // 直到下一个真正带 CRLF 的分隔符（这里是闭合那条）才切。
        Want w;
        w.keys  = 1;
        w.alpha = "AAA\n--" + BD + "\r\nContent-Disposition: form-data; name=\"beta\"\r\n\r\nBBB";
        push("J-lf-only-delim",
             "--" + BD + "\r\nContent-Disposition: form-data; name=\"alpha\"\r\n\r\nAAA\n" + tp("beta", "BBB") +
                 "--" + BD + "--\r\n",
             w);
    }
    {
        // 数据里出现"紧挨边界串的一串横线"，且前面不是 CRLF：整段仍是 alpha 的值
        Want w;
        w.keys  = 2;
        w.alpha = "PFX--" + BD + "SFX";
        w.beta  = "BBB";
        Case cs;
        cs.name     = "K-embed-bare";
        cs.ctype    = "multipart/form-data; boundary=" + BD;
        cs.body     = tp("alpha", w.alpha) + tp("beta", "BBB") + "--" + BD + "--\r\n";
        cs.judge    = true;
        cs.pairs    = true;
        cs.has_want = true;
        cs.want     = w;
        cases.push_back(cs);
    }
    // F-4 三条：请求头有没有声明 multipart，决定"没有 boundary"是协议错误还是原始正文
    {
        // 完全没有 Content-Type：body 原样落盘，不报错、不猜 multipart
        Want w;
        w.keys  = 0;
        w.alpha = "";
        w.beta  = "";
        w.raw   = tp("alpha", "AAA") + tp("beta", "BBB") + "--" + BD + "--\r\n";
        Case cs;
        cs.name     = "L-sniff-multipart";
        cs.ctype    = "";
        cs.body     = w.raw;
        cs.has_want = true;
        cs.want     = w;
        cases.push_back(cs);
    }
    {
        // 同上，body 是一段 PEM：改前被 multipart 嗅探吞掉（零字段），现在给回原始正文
        Want w;
        w.keys = 0;
        w.raw  = "-----BEGIN PRIVATE KEY-----\nabc\n-----END PRIVATE KEY-----\n";
        Case cs;
        cs.name     = "N-sniff-pem";
        cs.ctype    = "";
        cs.body     = w.raw;
        cs.has_want = true;
        cs.want     = w;
        cases.push_back(cs);
    }
    {
        // 头里写了 multipart 却没带 boundary：分隔符无从判断，必须拒（RFC 2046 必需参数）
        Want w;
        w.keys  = 0;
        w.finish = false;
        w.error  = 40099;
        Case cs;
        cs.name     = "O-declared-noboundary";
        cs.ctype    = "multipart/form-data";
        cs.body     = tp("alpha", "AAA") + tp("beta", "BBB") + "--" + BD + "--\r\n";
        cs.has_want = true;
        cs.want     = w;
        cases.push_back(cs);
    }
    // 截断 body 的三条：请求声明的字节读完了，但一直没认到闭合分隔符。
    // 这三种情形都把最后一段整段丢掉，请求照常结束、不判坏请求；只比整包读数，不做分段枚举。
    auto pushnoflag = [&](const std::string &name, const std::string &body, Want w)
    {
        Case cs;
        cs.name     = name;
        cs.ctype    = "multipart/form-data; boundary=" + BD;
        cs.body     = body;
        cs.judge    = false;
        cs.pairs    = false;
        cs.has_want = true;
        cs.want     = w;
        cases.push_back(cs);
    };
    {
        // 尾字段头部完整、值也在，只缺最后的 "--边界--"
        Want w;
        w.keys  = 1;
        w.alpha = "AAA";
        pushnoflag("T2-unclosed-tail",
                   tp("alpha", "AAA") + "--" + BD + "\r\nContent-Disposition: form-data; name=\"beta\"\r\n\r\nTAILVAL\r\n",
                   w);
    }
    {
        // 尾字段连头部都被截断（引号没闭合、没有值）
        Want w;
        w.keys  = 1;
        w.alpha = "AAA";
        pushnoflag("T3-unclosed-broken-header",
                   tp("alpha", "AAA") + "--" + BD + "\r\nContent-Disposition: form-data; name=\"bt", w);
    }
    {
        // 尾段是文件 part：整段的头部和内容都在，只缺闭合分隔符
        // 实测：这段连临时文件都没建（头部没解析过，isfile 还是 false，落盘名是空的），
        // 所以文件正文跟尾字段一样整段丢掉。
        Want w;
        w.keys   = 1;
        w.alpha  = "AAA";
        w.up     = "";
        w.upsize = 0;
        pushnoflag("T4-unclosed-filetail", tp("alpha", "AAA") + fp("up", "up.bin", "FILEDATA-123"), w);
    }
    // 读数臂：只打印整包读数，不参与判据
    // 尾段是文件 part，但正文里没有单个 "-"（上一个臂的正文有），看两种写法的收尾是否一致
    cases.push_back({"R-filetail-nodash", "multipart/form-data; boundary=" + BD,
                     tp("alpha", "AAA") + fp("up", "up.bin", "FILEBYTES"), false, false});
    cases.push_back({"R-empty-boundary", "multipart/form-data",
                     tp("alpha", "AAA") + "--" + BD + "--\r\n", false, false});
    cases.push_back({"R-no-ctype-dashbody", "",
                     "--" + BD + "\r\nContent-Disposition: form-data; name=\"alpha\"\r\n\r\nAAA\r\n--" + BD + "--\r\n",
                     false, false});
    cases.push_back({"R-pem-plain", "", "-----BEGIN PRIVATE KEY-----\nabc\n-----END PRIVATE KEY-----\n", false, false});
    // 值后只有 \r 就接分隔符（没有 \n）：改前接受并剥 2 字节，本实现判为普通数据。只读不判。
    cases.push_back({"R-cr-only-delim", "multipart/form-data; boundary=" + BD,
                     "--" + BD + "\r\nContent-Disposition: form-data; name=\"alpha\"\r\n\r\nAAA\r"
                     "--" + BD + "\r\nContent-Disposition: form-data; name=\"beta\"\r\n\r\nBBB\r\n--" + BD + "--\r\n",
                     false, false});

    std::printf("PROBE begin cases=%zu\n", cases.size());
    unsigned int total_fail = 0;
    unsigned int abs_fail   = 0;
    for (auto &cs : cases)
    {
        Reading golden = run(cs.ctype, cs.body, {});
        std::printf("GOLDEN %s size=%zu %s\n", cs.name.c_str(), cs.body.size(), golden.brief().c_str());
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

        unsigned int n = (unsigned int)cs.body.size();
        unsigned int pass = 0, fail = 0;
        // 单切点：逐字节枚举
        for (unsigned int i = 1; i < n; i++)
        {
            Reading r = run(cs.ctype, cs.body, {i});
            if (r.brief() != golden.brief())
            {
                fail++;
                std::printf("FAIL %s cut=%u got=%s\n", cs.name.c_str(), i, r.brief().c_str());
            }
            else
                pass++;
        }
        // 双切点：跨步枚举（含三段）
        if (cs.pairs)
        {
            unsigned int step = (n > 120) ? 3 : 1;
            for (unsigned int i = 1; i < n; i += step)
            {
                for (unsigned int j = i + 1; j < n; j += step)
                {
                    Reading r = run(cs.ctype, cs.body, {i, j});
                    if (r.brief() != golden.brief())
                    {
                        fail++;
                        std::printf("FAIL %s cut=%u,%u got=%s\n", cs.name.c_str(), i, j, r.brief().c_str());
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
    std::printf("PROBE abs_fail=%u total_fail=%u\n", abs_fail, total_fail);
    clean_outdir();
    return total_fail > 0 ? 1 : 0;
}
