// conf/github.conf 的读取。形状照 libs/oauth/oauth_config.cpp：
// 先断"存在且是常规文件"，parse_file 打不开会抛，拦住它别让请求流程崩，
// 取值只走 try_section / try_find，不用 operator[]。
#include "github_config.h"

#include <filesystem>
#include <iostream>

#include "cost_define.h"
#include "func.h"
#include "github_crypto.h"
#include "parse_ini.h"
#include "serverconfig.h"

namespace http
{
namespace github
{

namespace fs = std::filesystem;

namespace
{
constexpr const char *GITHUB_SECTION = "github";

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

// 端点根：必须以 https:// 开头，多写的结尾 '/' 一律截掉（调用方拼的是 "/login/oauth/..."）。
// 不合格的读数丢掉并打一行日志，保留调用方填好的默认值——把凭据从明文口发出去
// 不是"配置没配好"，是泄露。
std::string take_https_base(const std::string &name, const std::string &value, const std::string &fallback)
{
    if (!outbound_base_ok(value))
    {
        std::cerr << "[github] " << name << " ignored: " << (value.empty() ? "(empty)" : value)
                  << " (must start with https://)" << std::endl;
        return fallback;
    }
    std::string out = value;
    while (!out.empty() && out.back() == '/')
    {
        out.pop_back();
    }
    return out;
}
}// namespace

bool outbound_base_ok(const std::string &base)
{
    if (base.size() < 8)
    {
        return false;
    }
    // 只认 https://，前缀比较不分大小写；http:// 与空串都过不了这一关。
    return str_casecmp_pre(base, "https://", 8);
}

github_settings read_github_settings(const std::string &conffile)
{
    github_settings out;

    std::string path    = conffile.empty() ? conf_dir() + "github.conf" : conffile;
    std::string confdir = fs::path(path).parent_path().string();
    if (!confdir.empty() && confdir.back() != '/')
    {
        confdir.push_back('/');
    }

    // 先把默认值填上：文件缺失时这几个就是最终读数，调用方不用各自再兜一次。
    out.api_base         = CONST_GH_API_BASE;
    out.auth_base        = CONST_GH_AUTH_BASE;
    out.sync_interval    = CONST_GH_SYNC_INTERVAL;
    out.sync_batch       = CONST_GH_SYNC_BATCH;
    out.sync_max_pages   = CONST_GH_SYNC_MAX_PAGES;
    out.sync_backoff_max = CONST_GH_SYNC_BACKOFF_MAX;
    out.http_timeout     = CONST_GH_HTTP_TIMEOUT;
    out.secret_max_len   = CONST_GH_SECRET_MAX_LEN;
    out.default_scopes   = CONST_GH_DEFAULT_SCOPES;
    out.state_ttl        = CONST_GH_STATE_TTL;

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
        std::cerr << "[github] load config fail: " << path << " " << e.what() << std::endl;
        return out;
    }
    out.file = path;

    auto [p_sec, sec_found] = data.config.try_section(GITHUB_SECTION);
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
        return str_trim(*p_val);
    };
    // 数值键：越界或非数字不静默回落，打一行日志，否则运维改了配置却看不出没生效。
    auto number = [&](const char *name, unsigned int fallback, unsigned int low, unsigned int high) -> unsigned int
    {
        std::string text = value(name);
        if (text.empty())
        {
            return fallback;
        }
        unsigned long long parsed = 0;
        if (!str2uint64_strict(text, parsed) || parsed < (unsigned long long)low ||
            parsed > (unsigned long long)high)
        {
            std::cerr << "[github] " << name << " ignored: " << text << " (must be " << low << ".." << high
                      << ")" << std::endl;
            return fallback;
        }
        return (unsigned int)parsed;
    };

    // master key 的失败不退出进程，只让 key_ok 立不起来：绑定/回调拒绝服务，定时那一拍整轮跳过。
    std::string keyhex = value("crypto_key");
    if (!keyhex.empty())
    {
        out.key_ok = crypto_key_from_hex(keyhex, out.crypto_key);
        if (!out.key_ok)
        {
            std::cerr << "[github] crypto_key ignored: must be exactly " << CONST_GH_CRYPTO_KEY_LEN * 2
                      << " hex characters" << std::endl;
        }
    }

    out.api_base  = take_https_base("api_base", value("api_base"), out.api_base);
    out.auth_base = take_https_base("auth_base", value("auth_base"), out.auth_base);

    out.sync_interval    = number("sync_interval", out.sync_interval, 1, 86400);
    out.sync_batch       = number("sync_batch", out.sync_batch, 1, 100);
    out.sync_max_pages   = number("sync_max_pages", out.sync_max_pages, 1, 30);
    out.sync_backoff_max = number("sync_backoff_max", out.sync_backoff_max, 60, 86400);
    out.http_timeout     = number("http_timeout", out.http_timeout, 1, 90);
    // 明文闸的上界由列宽倒推：密文 = 明文 + 12(IV) + 16(tag)，必须装进 varbinary(255)。
    out.secret_max_len = number("secret_max_len", out.secret_max_len, 1, CONST_GH_CIPHER_MAX_LEN - 28);
    out.state_ttl      = number("state_ttl", out.state_ttl, 601, 86400);

    std::string scopes = value("default_scopes");
    if (!scopes.empty())
    {
        out.default_scopes = scopes;
    }

    // 默认 OAuth App 三键：空串就保持空，由 default_app_ready() 统一判。
    out.default_client_id     = value("default_client_id");
    out.default_client_secret = value("default_client_secret");
    out.default_redirect_uri  = value("default_redirect_uri");

    return out;
}

bool default_app_ready(const github_settings &cfg)
{
    return cfg.key_ok && !cfg.default_client_id.empty() && !cfg.default_client_secret.empty() && !cfg.default_redirect_uri.empty();
}

}// namespace github
}// namespace http
