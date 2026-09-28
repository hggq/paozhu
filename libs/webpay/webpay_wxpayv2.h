#ifndef __WEB_PAY_WXPAYV2_H__
#define __WEB_PAY_WXPAYV2_H__
// wxv2_config_gaps 要用 webpay_merchant_t（webpay_config.h），归 ENABLE_WEBPAY 管
// （vendor/webpay/include 只在开关打开时进 include 路径），include 与声明一起用 #ifdef 包住。

#include <string>
#include <vector>

#ifdef ENABLE_WEBPAY
#include "webpay_config.h"   // webpay_merchant_t / webpay_is_placeholder
#endif

namespace http
{
namespace webpay
{

#ifdef ENABLE_WEBPAY
// V2 凭据缺项清单：空 vector = 就绪，可发请求。条目是给页面回显的中文文案，逐字保留。
// 证书/私钥用 file_is_readable（mTLS 要盘上真实文件），不用 credential_readable——
// V2 的 cert_file/key_file 填一段 PEM 文本到不了 curl，判"可读"只会把失败推迟到退款那一刻。
std::vector<std::string> wxv2_config_gaps(const webpay_merchant_t &mch, bool need_cert);
#endif

}  // namespace webpay
}  // namespace http

#endif
