// conf/oauth.conf 的读取。形状照 vendor/httpserver/src/webpay_config.cpp：
// 先断"存在且是常规文件"，parse_file 打不开会抛，拦住它别让启动/请求流程崩，
// 取值只走 try_section / try_find，不用 operator[]（后者查不到会把空段塞进解析结果）。
#include "oauth_config.h"

#include <filesystem>
#include <iostream>
#include <mutex>

#include "cost_define.h"
#include "func.h"
#include "parse_ini.h"
#include "serverconfig.h"

namespace http
{
namespace oauth
{

namespace fs = std::filesystem;

namespace
{
// 这一段 ini 的段名。
constexpr const char *OAUTH_SECTION = "oauth";

// conf 目录：优先 serverconfig::configpath（带结尾 '/'），独立进程没加载过 server.conf 时用 "conf/"。
std::string conf_dir()
{
    std::string configpath = getserversysconfig().configpath;
    if (!configpath.empty())
    {
        return configpath;
    }
    return "conf/";
}

// 相对路径按 conf 目录拼，绝对路径原样用；两条都过一遍 lexically_normal，
// 免得 "./conf//x" 这种写法在错误信息里长得跟配置里不一样。
std::string resolve_key_path(const std::string &confdir, const std::string &value)
{
    std::error_code ec;
    fs::path p(value);
    if (!p.is_absolute())
    {
        p = fs::path(confdir) / p;
    }
    return p.lexically_normal().string();
}
}// namespace

oauth_settings read_oauth_settings(const std::string &conffile)
{
    oauth_settings out;

    std::string path    = conffile.empty() ? conf_dir() + "oauth.conf" : conffile;
    std::string confdir = fs::path(path).parent_path().string();
    if (!confdir.empty() && confdir.back() != '/')
    {
        confdir.push_back('/');
    }

    // 先把默认值填上：文件缺失时这几个就是最终读数，调用方不用各自再兜一次。
    out.kid          = CONST_OAUTH_KID;
    out.privkey_file = resolve_key_path(confdir, CONST_OAUTH_PRIVKEY_FILE);
    out.pubkey_file  = resolve_key_path(confdir, CONST_OAUTH_PUBKEY_FILE);
    out.refresh_ttl  = CONST_OAUTH_REFRESH_TTL;

    std::error_code ec;
    if (!fs::exists(path, ec) || !fs::is_regular_file(path, ec))
    {
        return out;
    }
    parse_ini data;
    try
    {
        // parse_file 打不开文件会抛 runtime_error，这里拦住，不能让它把请求流程搞崩
        data.parse_file(path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[oauth] load config fail: " << path << " " << e.what() << std::endl;
        return out;
    }
    out.file = path;

    auto [p_sec, sec_found] = data.config.try_section(OAUTH_SECTION);
    if (!sec_found || p_sec == nullptr)
    {
        return out;
    }
    out.loaded = true;

    auto value = [&](const char *name) -> std::string
    {
        auto [p_val, key_found] = p_sec->try_find(name);
        if (!key_found || p_val == nullptr)
        {
            return "";
        }
        return *p_val;
    };

    out.issuer = value("issuer");
    // iss 要跟对外地址逐字符一致，多写的结尾 '/' 一律截掉（"https://x/" 这种配法很常见）。
    while (!out.issuer.empty() && out.issuer.back() == '/')
    {
        out.issuer.pop_back();
    }
    std::string kid = value("kid");
    if (!kid.empty())
    {
        out.kid = kid;
    }
    std::string priv = value("private_key_file");
    if (!priv.empty())
    {
        out.privkey_file = resolve_key_path(confdir, priv);
    }
    std::string pub = value("public_key_file");
    if (!pub.empty())
    {
        out.pubkey_file = resolve_key_path(confdir, pub);
    }
    std::string ttl = value("refresh_ttl");
    if (!ttl.empty())
    {
        unsigned long long parsed = 0;
        // 上限就是清理保留期：revoked 的 refresh 行是重放检测的依据，refresh 活得比保留期久
        // 就等于"到期前那几行已经被删了"，轮换过的旧 token 再拿回来时查无此据。
        // 越界/非数字不静默回落——打一行日志，否则运维改了配置却看不出没生效。
        if (str2uint64_strict(ttl, parsed) && parsed > 0 && parsed < (unsigned long long)CONST_OAUTH_PURGE_KEEP)
        {
            out.refresh_ttl = (long long)parsed;
        }
        else
        {
            std::cerr << "[oauth] refresh_ttl ignored: " << ttl << " (must be 1.."
                      << (CONST_OAUTH_PURGE_KEEP - 1) << ")" << std::endl;
        }
    }
    return out;
}

oauth_settings oauth_settings_cached()
{
    static std::mutex     guard;
    static oauth_settings cached;
    static bool           ready = false;

    std::lock_guard<std::mutex> lk(guard);
    if (ready)
    {
        return cached;
    }
    oauth_settings fresh = read_oauth_settings("");
    if (fresh.loaded && !fresh.issuer.empty())
    {
        cached = fresh;
        ready  = true;
    }
    // 没读成（文件缺失、段不在、issuer 还空着）就把这一次的回落读数交出去，但不落缓存：
    // 配置文件晚到一步时要能在下一次调用里自动跟上。
    return fresh;
}

}// namespace oauth
}// namespace http
