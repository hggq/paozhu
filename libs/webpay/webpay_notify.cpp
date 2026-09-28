#include "webpay_notify.h"

namespace http
{
namespace webpay
{

bool notify_match_merchant(const notify_merchant &got, const notify_merchant &want,
                           std::string &why)
{
    why.clear();
    auto check = [&](const std::string &got_val, const std::string &want_val, const char *name) -> bool
    {
        if (want_val.empty()) return true; // want 空即此位跳过
        if (got_val != want_val)
        {
            // 这里可以带两端的值：本函数只在验签之后被调用，拿不到合法签名就看不到这句。
            // webpay_cert_gate 那句定值文案不能照此改 —— 它拼在未验签的出口上。
            why = std::string("回调 ") + name + " 与本地商户配置不匹配（got=" + got_val +
                  "，want=" + want_val + "）";
            return false;
        }
        return true;
    };
    return check(got.appid, want.appid, "appid") &&
           check(got.mchid, want.mchid, "mchid");
}

} // namespace webpay
} // namespace http
