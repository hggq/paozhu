#ifndef __WEB_PAY_ALIPAY_H__
#define __WEB_PAY_ALIPAY_H__
// alipay_fill 要用 pay::alipay 与 webpay_merchant_t，两者都归 ENABLE_WEBPAY 管
// （vendor/webpay/include 只在开关打开时进 include 路径），include 与声明一起用 #ifdef 包住。

#include <string>
#include "request.h"   // obj_val

#ifdef ENABLE_WEBPAY
#include "alipay.h"
#include "webpay_config.h"
#endif

namespace http
{
namespace webpay
{

// 支付宝网关应答包一层 wrapper（如 alipay_trade_precreate_response），从顶层取永远取不到。
// 解析失败 / wrapper 不是对象 / 字段缺失都回空串，不抛。
std::string alipay_resp_field(const std::string &response, const std::string &wrapper,
                              const std::string &field);

#ifdef ENABLE_WEBPAY
// 从 conf/webpay.conf 的 [<tag>.alipay]（tag 空 = 裸段 [alipay]）装配 pay::alipay。
// 凭据只来自配置文件，不接受请求参数覆盖，商户由调用方传入的 tag 钉死。
// 返回 false 表示关键项缺失或私钥加载不出，msg 里带上配置来源与实际取用的段名，
// 调用方直接回给客户端。
// need_private_key：这一路要不要用应用私钥去签报文。下单/查单/撤单/退款都要（签不出报文就没有
// 必要往外发一个字节）；回调/同步返回页只用支付宝公钥验签，不签任何东西，所以它们传 false ——
// 不然"私钥文件坏掉"会把回调也一起闸死，付了钱的单子再也落不了账。
bool alipay_fill(pay::alipay &apay, const std::string &tag, std::string &msg,
                 bool need_private_key = true);
#endif

}  // namespace webpay
}  // namespace http

#endif
