#include <string>
#include "webpay_wxpayv3.h" // 内部已先引 asio/awaitable.hpp（wxpay.h 需要）

namespace http
{
namespace webpay
{

wx_reply::wx_reply(const std::string &json)
{
    try
    {
        v.from_json(json);
        parsed = true;
    }
    catch (const std::exception &)
    {
        parsed = false;
    }
}

std::string wx_reply::operator[](const std::string &key)
{
    if (!parsed) return "";
    try
    {
        return v[key].to_string();
    }
    catch (const std::exception &)
    {
        return "";
    }
}

int wx_trade_settle(const std::string &resp, unsigned int payprice)
{
    try
    {
        obj_val v;
        v.from_json(resp);
        if (v["trade_state"].to_string() != "SUCCESS") return 0;
        long long total = v["amount"]["total"].to_int();
        if (total < 0 || (unsigned long long)total != payprice) return -1;
        return 1;
    }
    catch (const std::exception &)
    {
        return 0;
    }
}

#ifdef ENABLE_WEBPAY
std::string pick_primary_serial(const std::map<std::string, pay::wx_cert_t> &certs)
{
    std::string best, best_exp;
    for (const auto &kv : certs)
    {
        if (best.empty() || kv.second.expire_time > best_exp) { best = kv.first; best_exp = kv.second.expire_time; }
    }
    return best;
}

std::string select_wxpay_section(webpay_config_t &cfg, const std::string &want)
{
    if (!want.empty() && want.rfind("wxpay", 0) == 0 && cfg.has_section(want))
    {
        return want;
    }
    if (cfg.has_section("wxpayv3_web")) return "wxpayv3_web";
    if (cfg.has_section("wxpayv3")) return "wxpayv3";
    for (const auto &s : cfg.sections())
    {
        if (s.rfind("wxpay", 0) == 0) return s;
    }
    return "";
}
#endif // ENABLE_WEBPAY

} // namespace webpay
} // namespace http
