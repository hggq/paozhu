#ifndef __GITHUB_API_H__
#define __GITHUB_API_H__

#include <string>
#include <vector>

#include "github_config.h"

namespace http
{
namespace github
{

// ---------------------------------------------------------------------------
// 纯函数部分：正文解析与状态码分路。判据台测的就是这几块——它们不发 HTTP。
// ---------------------------------------------------------------------------

struct gh_user_info
{
    // login 会被用户改名，id 不会；展示用 login，认人用 id。
    unsigned long long user_id = 0;
    std::string        login;
    std::string        avatar;
};

struct gh_repo_item
{
    unsigned long long gh_repo_id = 0;
    std::string        full_name;
    bool               is_private = false;
    std::string        description;
    std::string        primary_language;
    unsigned long long stars = 0;
    // GitHub 报的是 ISO8601（"2026-10-29T08:15:30Z"），这里换算成 unix 秒；解不出来记 0。
    unsigned int pushed_at = 0;
};

// ISO8601 的 UTC 那一格换算成 unix 秒；形状不对或月份越界一律回 0，不做本地时区解释。
// 不借 timegm/_mkgmtime：那是两套平台各一个名字的可移植调用，且它们会接受越界日期回绕。
unsigned int iso8601_to_unix(const std::string &iso);

// GET /user 的响应。is_json 是框架报的"响应 Content-Type 是不是 json"，为 false 就是失败。
// 身份三列拿不到时调用方要能继续绑定，所以这里返回 false 不代表绑定失败，只代表"填不上"。
bool parse_user_body(const std::string &body, bool is_json, gh_user_info &out, std::string &error);

// GET /user/repos 的一页。数组里缺 id 的条目直接跳过——那一行连唯一键都拼不出来，
// 存进去只会变成另一条无名行盖住以后真能对上 id 的那条。
bool parse_repos_body(const std::string &body, bool is_json, std::vector<gh_repo_item> &out, std::string &error);

// 一页拉回来之后该怎么走。401 与"403 且额度见底"是两个完全不同的处置：前者这枚票永久死了，
// 后者只是排队；把两者并成一支会**能永久打死一个账号**（把限流当死票）。
enum class gh_api_state
{
    ok,           // 200 且正文解析成功
    token_dead,   // 401：这枚票在 GitHub 侧已经没了，只有用户重新授权能救回来
    rate_limited, // 403 或 429 且 X-RateLimit-Remaining == 0
    failed        // 5xx、传输错、正文不是 json
};
// ratelimit_remaining < 0 表示头没带（网关吃掉、或限流响应根本没有额度头），此时按 failed 走。
gh_api_state classify_page(unsigned int status, long long ratelimit_remaining, bool body_parsed_ok);

// 一页的请求地址。api_base 过不了 https 闸时返回空串，调用方据此不发。
std::string repos_page_url(const github_settings &cfg, unsigned int page_no);

// ---------------------------------------------------------------------------
// 出站部分
// ---------------------------------------------------------------------------

bool fetch_user(const github_settings &cfg,
                const std::string     &access_token,
                gh_user_info          &out,
                std::string           &error);

struct gh_page_result
{
    // 200 且正文解析成功。假的时候 items 为空，调用方按 classify_page 分路。
    bool ok = false;
    // 传输失败/超时是 0；正常拿到状态行就是 GitHub 报的那个数。
    unsigned int status = 0;
    std::vector<gh_repo_item> items;
    // 两个限流头，缺失记 -1。**不能用 0 表示缺失**——0 在限流语义里是"额度用尽"。
    long long ratelimit_remaining = -1;
    long long ratelimit_reset     = -1;
    // 失败原因。只放状态码与固定文案；这条串会进 last_error，所以绝不含令牌。
    std::string error;

    gh_api_state state() const
    {
        return classify_page(status, ratelimit_remaining, ok);
    }
};

// 拉一页仓库。page_no 从 1 开始（GitHub 的 page=0 与 page=1 同义，这里不放开 0）。
// 这一枚函数**不写库**，也不决定退避——它是§6.3 那一轮里唯一的一条 GET。
gh_page_result fetch_repos_page(const github_settings &cfg,
                                const std::string     &access_token,
                                unsigned int           page_no);

} // namespace github
} // namespace http
#endif
