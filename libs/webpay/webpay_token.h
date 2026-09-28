#ifndef __WEB_PAY_TOKEN_H__
#define __WEB_PAY_TOKEN_H__
// 有意不加 ENABLE_WEBPAY，也不引支付 SDK：这里只有字符串、时钟和一张进程内小表。

#include <string>

namespace http
{
namespace webpay
{

// 微信 access_token 有效期 7200 秒、每日有配额。这里只缓 7000 秒，留 200 秒余量：
// 撞边界那几秒宁可回源多换一次。
constexpr unsigned int wx_token_ttl_sec = 7000;

// 命中返回 true 并把 token **拷**进 tok（不给引用：出锁之后这条缓存随时可能被别的请求
// 改掉或删掉，而 tok 是凭据）。now_sec 由调用方传 webpay::steady_seconds()
// （声明在 webpay_cert_gate.h）：墙上时钟被 NTP 往回拨会让这条缓存多活一段。
bool token_cache_get(const std::string &appid, std::string &tok, unsigned long long now_sec);

// 空 token 不入库：微信对"换取失败"也回 HTTP 200，正文里 errcode 非 0、access_token 缺项，
// 存进去就是拿着空串要 7000 秒的手机号。
void token_cache_put(const std::string &appid, const std::string &tok, unsigned long long now_sec);

// 丢掉这条缓存：微信说手里的 token 不作数时调用，下一笔自己回源。
void token_cache_invalidate(const std::string &appid);

// 只有这三个 errcode 说明"问题在 token 本身"（不合法 / 已过期 / 被作废）。
// 40125（appsecret 错）、40029（code 无效）这些清了没用：配置就是错的，
// 只会把每一笔都变成两趟外发。
bool token_rejected_errcode(const std::string &errcode);

} // namespace webpay
} // namespace http

#endif
