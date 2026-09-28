#ifndef __WEB_PAY_NOTIFY_H__
#define __WEB_PAY_NOTIFY_H__
// 有意不加 ENABLE_WEBPAY，也不引支付 SDK：只比对两个已在手边的字符串。

#include <string>

namespace http
{
namespace webpay
{

// 回调归属校验：验签之后的 appid / mchid 必须与本地商户配置匹配。**只有支付宝需要**。
//   微信 V2 —— 不需要。sign = MD5(参数 + "&key=" + 本商户 apikey)，取错段和跨商户重投都算不出
//              同一个 sign，验签先挡。（vendor/webpay/src/weixinxcx.cpp 的 get_sign）
//   微信 V3 —— 不需要。外层签名用的是全商户共用的平台证书，但落账字段（appid / mchid /
//              out_trade_no…）来自本商户 api_v3_key 的 AES-GCM 明文，别商户的密文解不开。
//              （vendor/webpay/src/wxpay.cpp 的 handleNotify）
//   支付宝   —— **需要**。rsaVerify 用的支付宝公钥对所有商户共用，app_id 只是报文里的一个字段：
//              抓到一份合法正文原样投到本路由，验签照样过 ⇒ 归属只能在这里比对。
// 所以本函数只有 /alipaynotify 与 /alipayreturn 两个调用点；三条微信路由不加这道校验。
struct notify_merchant
{
    std::string appid;
    std::string mchid; // 支付宝通知无此项，留空即不判
};

// 验签后的通知身份必须与本地商户配置匹配。want 里某项为空则跳过这一项：兼容只填 apikey
// 不填 appid 的老配置。
bool notify_match_merchant(const notify_merchant &got, const notify_merchant &want,
                           std::string &why);

} // namespace webpay
}//namespace http
#endif
