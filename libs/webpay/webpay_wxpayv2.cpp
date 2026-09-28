#include <string>
#include <vector>
#include "webpay_wxpayv2.h"
#ifdef ENABLE_WEBPAY
#include "webpay_cert_gate.h"
#endif

namespace http
{
namespace webpay
{

#ifdef ENABLE_WEBPAY

std::vector<std::string> wxv2_config_gaps(const webpay_merchant_t &mch, bool need_cert)
{
    std::vector<std::string> lack;
    if (webpay_is_placeholder(mch.appid)) lack.push_back("appid");
    if (webpay_is_placeholder(mch.mch_id)) lack.push_back("mch_id");
    if (webpay_is_placeholder(mch.apikey))
    {
        lack.push_back("apikey");
    }
    else if (mch.apikey.size() != 32)
    {
        lack.push_back("apikey（V2 密钥应正好 32 位，当前 " + std::to_string(mch.apikey.size()) + " 位）");
    }
    if (mch.notifyurl.empty()) lack.push_back("notifyurl（统一下单必填）");
    if (need_cert && !file_is_readable(mch.cert_file)) lack.push_back("cert_file（退款要 apiclient_cert.pem，当前不可读）");
    if (need_cert && !file_is_readable(mch.key_file)) lack.push_back("key_file（退款要 apiclient_key.pem，当前不可读）");
    return lack;
}

#endif// ENABLE_WEBPAY

}  // namespace webpay
}  // namespace http
