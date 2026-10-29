// GitHub REST 侧：读身份、读仓库清单的一页。
// 解析与分路都是纯函数（判据台测这两块），只有 fetch_* 两个函数真发 HTTP。
#include "github_api.h"

#include <cctype>

#include "cost_define.h"
#include "func.h"
#include "github_crypto.h"
#include "httpclient.h"
#include "request.h"
#include "urlcode.h"

namespace http
{
namespace github
{

namespace
{
// GitHub 的响应头大小写由服务端决定（实测是 X-RateLimit-Remaining 这种首字母大写），
// page.header 是按原样入表的 map，所以查它必须忽略大小写——写死一种拼法会读成"头没带"。
bool header_number(const std::map<std::string, std::string> &headers, const char *name, long long &out)
{
    for (auto &[key, value] : headers)
    {
        if (!str_casecmp(key, name))
        {
            continue;
        }
        unsigned long long digits = 0;
        // 两个额度头都是非负数；带符号的读法会把"-1"之类的畸形值当成额度。
        if (!str2uint64_strict(str_trim(value), digits))
        {
            return false;
        }
        // str2uint64_strict 按 20 位收，能收到超过 long long 的数；先卡住再变号。
        if (digits > (unsigned long long)9223372036854775807LL)
        {
            return false;
        }
        out = (long long)digits;
        return true;
    }
    return false;
}

bool digit_only(const std::string &text, unsigned int offset, unsigned int count)
{
    if (offset + count > text.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < count; i++)
    {
        if (text[offset + i] < '0' || text[offset + i] > '9')
        {
            return false;
        }
    }
    return true;
}

unsigned int four(const std::string &text, unsigned int offset)
{
    return (unsigned int)((text[offset] - '0') * 1000 + (text[offset + 1] - '0') * 100 +
                          (text[offset + 2] - '0') * 10 + (text[offset + 3] - '0'));
}

unsigned int two(const std::string &text, unsigned int offset)
{
    return (unsigned int)((text[offset] - '0') * 10 + (text[offset + 1] - '0'));
}

// 从 1970-01-01 起的天数（Howard Hinnant 的 days_from_civil，无库依赖、无平台差异）。
long long days_from_civil(long long y, long long m, long long d)
{
    y -= m <= 2;
    long long era = (y >= 0 ? y : y - 399) / 400;
    long long yoe = y - era * 400;            // [0, 399]
    long long m0  = m > 2 ? m - 3 : m + 9;    // 3 月是这一年第 0 个月
    long long doy = (153 * m0 + 2) / 5 + d - 1;
    long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}
} // namespace

unsigned int iso8601_to_unix(const std::string &iso)
{
    // GitHub 的 pushed_at 恒为 "YYYY-MM-DDTHH:MM:SSZ"。这里只认这一种形状：
    // 带本地时区的偏移（+08:00）不在它的可能取值里，硬接受就要引入时区规则。
    if (iso.size() != 20 || iso[4] != '-' || iso[7] != '-' || iso[10] != 'T' || iso[13] != ':' ||
        iso[16] != ':' || iso[19] != 'Z')
    {
        return 0;
    }
    if (!digit_only(iso, 0, 4) || !digit_only(iso, 5, 2) || !digit_only(iso, 8, 2) ||
        !digit_only(iso, 11, 2) || !digit_only(iso, 14, 2) || !digit_only(iso, 17, 2))
    {
        return 0;
    }
    unsigned int year   = four(iso, 0);
    unsigned int month  = two(iso, 5);
    unsigned int day    = two(iso, 8);
    unsigned int hour   = two(iso, 11);
    unsigned int minute = two(iso, 14);
    unsigned int second = two(iso, 17);
    if (year < 1970 || year > 2999 || month < 1 || month > 12 || day < 1 || hour > 23 ||
        minute > 59 || second > 59)
    {
        return 0;
    }
    // 日必须有这一月那么长："2026-02-30" 在日历换算里会被搬成 3 月 2 日，
    // 落库的是一个看着合法的错时间，没有任何一处会指向真正的原因。
    unsigned int last_day = 31;
    if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        last_day = 30;
    }
    else if (month == 2)
    {
        bool leap     = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        last_day      = leap ? 29 : 28;
    }
    if (day > last_day)
    {
        return 0;
    }
    long long seconds = days_from_civil((long long)year, month, day) * 86400LL + (long long)hour * 3600LL +
                        (long long)minute * 60LL + (long long)second;
    if (seconds < 0)
    {
        return 0;
    }
    return (unsigned int)seconds;
}

bool parse_user_body(const std::string &body, bool is_json, gh_user_info &out, std::string &error)
{
    out = gh_user_info();
    if (!is_json)
    {
        error = "user response is not json";
        return false;
    }
    obj_val root;
    try
    {
        root.from_json(body);
    }
    catch (const std::exception &)
    {
        error = "user body bad json";
        return false;
    }
    if (!root.is_object())
    {
        error = "user body not object";
        return false;
    }
    if (root.isset("message") && !root.isset("login"))
    {
        // GitHub 的错误体形状：{"message":"Bad credentials","documentation_url":"..."}。
        // 只取 message 文本，且它不含凭据，可以直接作为原因上抛。
        error = scrub_error_text(root["message"].to_string());
        return false;
    }
    long long id = root["id"].to_int();
    if (id <= 0)
    {
        error = "user id missing";
        return false;
    }
    out.user_id = (unsigned long long)id;
    out.login   = root["login"].to_string();
    out.avatar  = root["avatar_url"].to_string();
    if (out.login.empty())
    {
        error = "user login missing";
        return false;
    }
    return true;
}

bool parse_repos_body(const std::string &body, bool is_json, std::vector<gh_repo_item> &out, std::string &error)
{
    out.clear();
    if (!is_json)
    {
        error = "repos response is not json";
        return false;
    }
    obj_val root;
    try
    {
        root.from_json(body);
    }
    catch (const std::exception &)
    {
        error = "repos body bad json";
        return false;
    }
    if (!root.is_array())
    {
        // 出错时 GitHub 在这一页返回的是对象（{"message":...}），不是数组。
        error = "repos body not array";
        if (root.is_object() && root.isset("message"))
        {
            error += ": ";
            error += scrub_error_text(root["message"].to_string());
        }
        return false;
    }
    for (obj_val &item : root.as_array())
    {
        if (!item.is_object())
        {
            continue;
        }
        long long id = item["id"].to_int();
        if (id <= 0)
        {
            // 连唯一键 (account_id, gh_repo_id) 都拼不出来；收下只会变成一条无名行。
            continue;
        }
        gh_repo_item one;
        one.gh_repo_id = (unsigned long long)id;
        one.full_name  = item["full_name"].to_string();
        if (one.full_name.empty())
        {
            // owner/repo 是页面唯一可读的东西；空名的行展示不出、也没法在 GitHub 上核对。
            continue;
        }
        one.is_private       = item["private"].to_bool();
        one.description      = item["description"].to_string();
        one.primary_language = item["language"].to_string();
        long long stars      = item["stargazers_count"].to_int();
        one.stars            = stars > 0 ? (unsigned long long)stars : 0;
        one.pushed_at        = iso8601_to_unix(item["pushed_at"].to_string());
        out.push_back(std::move(one));
    }
    return true;
}

gh_api_state classify_page(unsigned int status, long long ratelimit_remaining, bool body_parsed_ok)
{
    if (status == 200)
    {
        return body_parsed_ok ? gh_api_state::ok : gh_api_state::failed;
    }
    if (status == 401)
    {
        return gh_api_state::token_dead;
    }
    if ((status == 403 || status == 429) && ratelimit_remaining == 0)
    {
        // 403 有两种：权限不够，和额度用尽。两者状态码相同，只有额度头能分开。
        // 分不开时宁可当失败退避，也不能当死票——把限流当死票会永久打死一个账号。
        return gh_api_state::rate_limited;
    }
    if (status >= 500)
    {
        return gh_api_state::failed;
    }
    return gh_api_state::failed;
}

std::string repos_page_url(const github_settings &cfg, unsigned int page_no)
{
    if (!outbound_base_ok(cfg.api_base) || page_no == 0)
    {
        return "";
    }
    return cfg.api_base + "/user/repos?per_page=" + std::to_string(CONST_GH_REPO_PER_PAGE) + "&page=" +
           std::to_string(page_no) + "&type=all";
}

bool fetch_user(const github_settings &cfg, const std::string &access_token, gh_user_info &out, std::string &error)
{
    out = gh_user_info();
    if (!outbound_base_ok(cfg.api_base))
    {
        error = "api_base is not https";
        return false;
    }
    if (access_token.empty())
    {
        error = "access token missing";
        return false;
    }
    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    a->get(cfg.api_base + "/user");
    // Accept 决定响应的 Content-Type，而框架只在它正是 application/json 时才置 page.isjson。
    a->set_header("Accept", "application/vnd.github+json");
    a->set_header("Authorization", "Bearer " + access_token);
    a->timeout(cfg.http_timeout);
    a->send();
    unsigned int status = a->get_status();
    if (a->iserror > 0 || status == 0)
    {
        a->close_connect();
        // error_msg 是框架的原文，可能带上请求行里的东西；这里只留固定文案。
        error = "transport error";
        return false;
    }
    a->close_connect();
    if (parse_user_body(a->page.content, a->page.isjson, out, error))
    {
        return true;
    }
    if (status != 200)
    {
        error = "http " + std::to_string(status);
    }
    return false;
}

gh_page_result fetch_repos_page(const github_settings &cfg, const std::string &access_token, unsigned int page_no)
{
    gh_page_result out;
    std::string    url = repos_page_url(cfg, page_no);
    if (url.empty())
    {
        out.error = cfg.api_base.empty() ? "api_base missing" : "api_base is not https";
        return out;
    }
    if (access_token.empty())
    {
        out.error = "access token missing";
        return out;
    }
    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    a->get(url);
    a->set_header("Accept", "application/vnd.github+json");
    a->set_header("Authorization", "Bearer " + access_token);
    a->timeout(cfg.http_timeout);
    a->send();
    out.status = a->get_status();
    if (a->iserror > 0 || out.status == 0)
    {
        a->close_connect();
        out.error = "transport error";
        return out;
    }
    a->close_connect();
    // 两个额度头缺一个都不致命：缺 Remaining 就分不出 403 是哪一种，调用方按失败退避。
    header_number(a->page.header, "X-RateLimit-Remaining", out.ratelimit_remaining);
    header_number(a->page.header, "X-RateLimit-Reset", out.ratelimit_reset);
    std::string parse_error;
    out.ok = parse_repos_body(a->page.content, a->page.isjson, out.items, parse_error);
    if (!out.ok)
    {
        out.error = parse_error;
    }
    if (!out.ok && out.status != 200)
    {
        out.error = "http " + std::to_string(out.status);
    }
    return out;
}

} // namespace github
} // namespace http
