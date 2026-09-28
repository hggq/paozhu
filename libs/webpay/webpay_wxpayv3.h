#ifndef __WEB_PAY_WXPAYV3_H__
#define __WEB_PAY_WXPAYV3_H__
// wx_reply / wx_trade_settle 有意不加 ENABLE_WEBPAY：只依赖 obj_val（request.h），
// 不引支付 SDK。
// pick_primary_serial / select_wxpay_section 用到 pay::wx_cert_t（wxpay.h）和
// webpay_config_t（webpay_config.h），两者都归 ENABLE_WEBPAY 管（vendor/webpay/include
// 只在开关打开时才进 include 路径），include 与声明一起用 #ifdef 包住。

#include <map>
#include <string>
#include "request.h"

#ifdef ENABLE_WEBPAY
#include "wxpay.h"
#include "webpay_config.h"
#endif

namespace http
{
namespace webpay
{

// 微信 V3 的应答是 JSON。构造时解析一次，之后按 key 读取：`v[key]`；
// 非 JSON（网关 5xx 回的 HTML 错误页）或读字段抛异常一律回空串——与"应答里没这个
// 字段"落到调用方判空的同一条分支。
class wx_reply
{
public:
    explicit wx_reply(const std::string &json);
    std::string operator[](const std::string &key);

private:
    obj_val v;
    bool parsed = false;
};

// 查单应答的落账条件。V3 这条线没有回调路由，成交结论只能靠查单，所以金额必须在这里对：
// 应答里的 amount.total（分）要等于下单时落库的 payprice。
// 返回：0 = 还没成交（不回填）；1 = 成交且金额相符（可回填）；
//      -1 = trade_state 是 SUCCESS 但金额对不上（拒绝回填，页面上要说清楚）。
int wx_trade_settle(const std::string &resp, unsigned int payprice);

#ifdef ENABLE_WEBPAY
// 选主证书：expire_time 最大者，证书轮换期取最新那张。
// 依赖 SDK 在 parsePlatformCerts() 里一并取出 expire_time（按 JSON 对象内字段取，
// 与顺序无关），不自己啃原始 JSON，也免为它单独发一趟下载。
std::string pick_primary_serial(const std::map<std::string, pay::wx_cert_t> &certs);

// 证书下载页的段选择器。白名单 = has_section() 实查段是否存在，且段名必须是 wxpay*：
// 下载页要往段里写 platform_cert_file，落到 [alipay] 或 [domains] 上就是把微信证书
// 写进别人的配置。
// 默认回落顺序：wxpayv3_web → wxpayv3 → 第一个 wxpay* 段 → 空串。
std::string select_wxpay_section(webpay_config_t &cfg, const std::string &want);
#endif // ENABLE_WEBPAY

} // namespace webpay
} // namespace http
#endif
