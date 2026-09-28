#include <map>
#include <mutex>
#include <string>
#include "webpay_token.h"

namespace http
{
namespace webpay
{

namespace
{
struct token_slot
{
    std::string token;
    unsigned long long not_after = 0;
};

// 键是配置里的 appid ⇒ 条目数由商户段数决定，不随请求条数增长。
std::mutex g_token_mtx;
std::map<std::string, token_slot> g_token_cache;
} // namespace

bool token_cache_get(const std::string &appid, std::string &tok, unsigned long long now_sec)
{
    std::lock_guard<std::mutex> lock(g_token_mtx);
    auto it = g_token_cache.find(appid);
    if (it == g_token_cache.end() || now_sec >= it->second.not_after) return false;
    tok = it->second.token;
    return true;
}

void token_cache_put(const std::string &appid, const std::string &tok, unsigned long long now_sec)
{
    if (tok.empty()) return;
    std::lock_guard<std::mutex> lock(g_token_mtx);
    g_token_cache[appid] = token_slot{tok, now_sec + wx_token_ttl_sec};
}

void token_cache_invalidate(const std::string &appid)
{
    std::lock_guard<std::mutex> lock(g_token_mtx);
    g_token_cache.erase(appid);
}

bool token_rejected_errcode(const std::string &errcode)
{
    return errcode == "40001" || errcode == "40014" || errcode == "42001";
}

} // namespace webpay
} // namespace http
