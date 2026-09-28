#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <ctime>
#include <functional>
#include <iostream>
#include <cctype>// ::toupper 显式包含，避免依赖传递性 include（Windows/MSVC 下更稳）
#include "httpclient.h"
#include "weixinxcx.h"
#include "md5.h"
#include "func.h"// http::json_escape：与 wxpay.cpp / alipay.cpp 同一份转义实现

namespace pay
{
weixinpay::weixinpay() {}

weixinpay::weixinpay(std::map<std::string, std::string> &&params)
{
    // 从 map 中提取已知字段，支持字段名大小写（假设使用小写）
    auto it = params.find("appid");
    if (it != params.end())
        appid_ = it->second;
    it = params.find("mch_id");
    if (it != params.end())
        mch_id_ = it->second;
    it = params.find("apikey");
    if (it != params.end())
        apikey_ = it->second;
    it = params.find("openid");
    if (it != params.end())
        openid_ = it->second;
    it = params.find("out_trade_no");
    if (it != params.end())
        out_trade_no_ = it->second;
    it = params.find("body");
    if (it != params.end())
        body_ = it->second;
    it = params.find("total_fee");
    if (it != params.end())
        total_fee_ = it->second;
    // 如果传入的是 total_free，也兼容
    it = params.find("total_free");
    if (it != params.end())
        total_fee_ = it->second;
    it = params.find("notify_url");
    if (it != params.end())
        notify_url_ = it->second;
}

std::string weixinpay::generate_nonce_str()
{
    static const char *chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    // 单次 rd() 只有 32 位，喂不饱下面 32 个字符（≈190 位）的 nonce，且同种子完全可复现。
    // seed_seq 把 128 位喂进既有引擎 ⇒ 输出分布不变。不用 thread_local，也不引共享状态。
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(0, 61);
    std::string nonce;
    for (int i = 0; i < 32; ++i)
    {
        nonce += chars[dis(gen)];
    }
    return nonce;
}

std::string weixinpay::kvmap_to_string(const std::map<std::string, std::string> &params, bool for_sign)
{
    std::string result;
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        if (for_sign && it->first == "sign")
            continue;// 签名时忽略 sign 字段
        if (!result.empty())
            result += "&";
        result += it->first + "=" + it->second;
    }
    return result;
}

std::string weixinpay::get_sign(const std::map<std::string, std::string> &params)
{
    // 1. 过滤空值并排序（map 本身按键排序）
    std::map<std::string, std::string> filtered;
    for (const auto &p : params)
    {
        if (!p.second.empty() && p.first != "sign")
        {
            filtered.insert(p);
        }
    }

    // 2. 生成待签名字符串
    std::string str = kvmap_to_string(filtered, true);
    str += "&key=" + apikey_;
    std::string sign = http::md5(str);                                // ss.str();
    std::transform(sign.begin(), sign.end(), sign.begin(), ::toupper);// 转大写

    return sign;
}

// 值一律走 CDATA：body/attach 这些字段取自调用方文本（演示页的 desc 直接来自请求），
// 裸拼进 XML 时值里一个 `<` 或 `&` 就能改写报文结构。
// 值里自己出现 `]]>` 时按 XML 规范拆成相邻两段 CDATA，不能原样留在一段里。
static std::string cdata_value(const std::string &v)
{
    std::string out;
    std::size_t pos = 0;
    for (;;)
    {
        std::size_t hit = v.find("]]>", pos);
        if (hit == std::string::npos)
        {
            out += v.substr(pos);
            break;
        }
        out += v.substr(pos, hit - pos) + "]]]]><![CDATA[>";
        pos = hit + 3;
    }
    return out;
}

std::string weixinpay::array_to_xml(const std::map<std::string, std::string> &params)
{
    std::string xml = "<xml>";
    for (const auto &p : params)
    {
        xml += "<" + p.first + "><![CDATA[" + cdata_value(p.second) + "]]></" + p.first + ">";
    }
    xml += "</xml>";
    return xml;
}

std::map<std::string, std::string> weixinpay::parse_xml(const std::string &xml)
{
    std::map<std::string, std::string> result;
    // 简易解析：只提取 <tag>content</tag> 或 <tag><![CDATA[content]]></tag>
    size_t pos = 0;
    for (; pos < xml.size(); pos++)
    {
        if (xml[pos] == 0x20 || xml[pos] == 0x0A || xml[pos] == 0x0D)
        {
            continue;
        }
        break;
    }
    if (xml[pos] == '<' && xml[pos + 1] == 'x' && xml[pos + 2] == 'm' && xml[pos + 3] == 'l' && xml[pos + 4] == '>')
    {
        pos += 5;
    }
    else
    {
        result["return_msg"] = "return xml error!";
        return result;
    }

    while (true)
    {
        size_t start = xml.find('<', pos);
        if (start == std::string::npos)
            break;
        size_t end = xml.find('>', start);
        if (end == std::string::npos)
            break;
        std::string tag = xml.substr(start + 1, end - start - 1);
        if (tag.empty() || tag[0] == '/')
        {// 忽略结束标签
            pos = end + 1;
            continue;
        }
        // 查找对应的结束标签
        std::string end_tag  = "</" + tag + ">";
        size_t content_start = end + 1;
        size_t content_end   = xml.find(end_tag, content_start);
        if (content_end == std::string::npos)
            break;

        std::string content = xml.substr(content_start, content_end - content_start);
        // 移除 CDATA 标记
        if (content.compare(0, 9, "<![CDATA[") == 0)
        {
            // 必须先判 ]]> 后缀再剥：直接 substr(9, size()-12) 在结尾不是 ]]> 时会静默砍掉
            // 值尾（金额字段被改写），长度不足时下溢成巨值、把半截标记当值留下。
            if (content.size() >= 12 && content.compare(content.size() - 3, 3, "]]>") == 0)
            {
                content = content.substr(9, content.size() - 12);
            }
            else
            {
                // 畸形 CDATA ⇒ 当作没有这个字段，交给 handle_notify 的"必要字段为空"那道闸。
                // pos 必须照常推进：不推进的话下一轮 find('<', pos) 停在同一位置，死循环。
                pos = content_end + end_tag.length();
                continue;
            }
        }
        result[tag] = content;
        pos         = content_end + end_tag.length();
    }
    return result;
}

std::string weixinpay::postxmlto(const std::string &url, const std::string &xml_data)
{
    std::string response;
    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->post(url);
    a->set_header("Content-Type", "application/xml");
    a->set_body(xml_data);
    a->timeout(8);// 8s 是全渠道统一档；V2 退款走 secapi 双向 TLS，是最贴近这一秒数的一条
    a->send();

    if (a->get_status() == 200)
    {
        response = a->get_body();
    }
    else
    {
        response = "<xml><return_code><![CDATA[ERROR]]></return_code><return_msg><![CDATA[URL Error]]></return_msg><result_code><![CDATA[ERROR]]></result_code></xml>";
    }

    return response;
}

std::string weixinpay::postxmlto_cert(const std::string &url, const std::string &xml_data)
{
    // 退款等 secapi 接口要求双向证书：携带商户证书与私钥发起 HTTPS 请求
    std::string response;
    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->post(url);
    a->set_header("Content-Type", "application/xml");
    a->set_body(xml_data);
    a->timeout(8);
    if (!cert_file_.empty())
    {
        a->set_ssl_certificate_file(cert_file_);
    }
    if (!key_file_.empty())
    {
        a->set_ssl_private_key_file(key_file_);
    }
    a->send();

    if (a->get_status() == 200)
    {
        response = a->get_body();
    }
    else
    {
        response = "<xml><return_code><![CDATA[ERROR]]></return_code><return_msg><![CDATA[URL Error]]></return_msg><result_code><![CDATA[ERROR]]></result_code></xml>";
    }

    return response;
}

// ===== 请求参数与应答解析的构造口：同步版与协程版共用 =====
// nonce 由调用方传入、sign 由调用方拿到 params 后自己写进 "sign"：这两样都不进构造口。
// std::map 按 key 有序，写入顺序不影响报文字节，搬动赋值行序是安全的。

std::map<std::string, std::string> weixinpay::build_unifiedorder_params(const std::string& nonce) const
{
    std::map<std::string, std::string> params;
    params["appid"]            = appid_;
    params["mch_id"]           = mch_id_;
    params["nonce_str"]        = nonce;
    params["body"]             = body_;
    params["out_trade_no"]     = out_trade_no_;
    params["total_fee"]        = total_fee_; // 注意字段名是 total_fee
    params["spbill_create_ip"] = client_ip_; // 实际项目中需从请求上下文中获取
    params["notify_url"]       = notify_url_;// "https://myweixin.xxx.com/xcxnotify";  // 配置你的通知地址
    params["openid"]           = openid_;// JSAPI 支付必需
    params["trade_type"]       = "JSAPI";// 根据业务调整
    return params;
}

std::map<std::string, std::string> weixinpay::build_createnative_params(const std::string& nonce) const
{
    // NATIVE 扫码支付下单，不需要 openid，成功返回 code_url 供生成二维码
    std::map<std::string, std::string> params;
    params["appid"]            = appid_;
    params["mch_id"]           = mch_id_;
    params["nonce_str"]        = nonce;
    params["body"]             = body_;
    params["out_trade_no"]     = out_trade_no_;
    params["total_fee"]        = total_fee_;
    params["spbill_create_ip"] = client_ip_;
    params["notify_url"]       = notify_url_;
    params["trade_type"]       = "NATIVE";
    params["product_id"]       = out_trade_no_;// NATIVE 模式一必传，模式二用商户单号即可
    return params;
}

std::map<std::string, std::string> weixinpay::build_refund_params(const std::string& nonce) const
{
    std::map<std::string, std::string> params;
    params["appid"]     = appid_;
    params["mch_id"]    = mch_id_;
    params["nonce_str"] = nonce;
    params["sign_type"] = "MD5";
    if (!transaction_id_.empty())
    {
        params["transaction_id"] = transaction_id_;// 与 out_trade_no 二选一
    }
    params["out_trade_no"]  = out_trade_no_;
    params["out_refund_no"] = out_refund_no_;
    params["total_fee"]     = total_fee_;
    params["refund_fee"]    = refund_fee_;
    if (!refund_desc_.empty())
    {
        params["refund_desc"] = refund_desc_;
    }
    return params;
}

std::map<std::string, std::string> weixinpay::build_refundquery_params(const std::string& nonce) const
{
    std::map<std::string, std::string> params;
    params["appid"]     = appid_;
    params["mch_id"]    = mch_id_;
    params["nonce_str"] = nonce;
    if (!transaction_id_.empty())
    {
        params["transaction_id"] = transaction_id_;
    }
    if (!out_trade_no_.empty())
    {
        params["out_trade_no"] = out_trade_no_;
    }
    if (!out_refund_no_.empty())
    {
        params["out_refund_no"] = out_refund_no_;
    }
    return params;
}

std::map<std::string, std::string> weixinpay::build_pay_params(const std::string& prepay_id,
                                                               const std::string& nonce,
                                                               const std::string& timestamp) const
{
    // 调起支付所需的五个键（appId/timeStamp/nonceStr/package/signType，大小写按微信要求）。
    // 签名口径就是这五个键本身，所以 paySign 与 out_trade_no 由调用方在签完之后补进去。
    std::map<std::string, std::string> pay_params;
    pay_params["appId"]     = appid_;
    pay_params["timeStamp"] = timestamp;
    pay_params["nonceStr"]  = nonce;
    pay_params["package"]   = "prepay_id=" + prepay_id;
    pay_params["signType"]  = "MD5";
    return pay_params;
}

std::map<std::string, std::string> weixinpay::build_refund_gate_result() const
{
    // 缺幂等键时的失败应答：形状与退款应答解析后一致（status_code=1 + error_msg），
    // 两个入口共用，避免同步版改了文案协程版没改。
    std::map<std::string, std::string> result;
    result["return_code"]  = "FAIL";
    result["result_code"]  = "FAIL";
    result["error_msg"]    = "缺少退款单号 out_refund_no（幂等键必须由调用方显式设置）";
    result["status_code"]  = "1";
    result["out_trade_no"] = out_trade_no_;
    return result;
}

std::string weixinpay::pay_params_to_json(const std::map<std::string, std::string>& pay_params)
{
    // 键与值都过 http::json_escape：这一串是给 wx.requestPayment 的报文，值里出现一个引号就能
    // 把它改写掉。与 wxpay.cpp（V3）、alipay.cpp 用同一份转义实现。
    std::string json = "{";
    for (auto it = pay_params.begin(); it != pay_params.end(); ++it)
    {
        if (it != pay_params.begin())
            json += ",";
        json += "\"" + http::json_escape(it->first) + "\":\"" + http::json_escape(it->second) + "\"";
    }
    json += "}";
    return json;
}

std::string weixinpay::unifiedorder_result(const std::string& xml, const std::string& want_field)
{
    std::map<std::string, std::string> resp = parse_xml(xml);
    if (resp["return_code"] == "SUCCESS" && resp["result_code"] == "SUCCESS")
    {
        return resp[want_field];// prepay_id / code_url
    }
    // 返回错误信息（可根据需要格式化）
    return "ERROR:" + resp["return_msg"] + ";" + resp["err_code_des"];
}

void weixinpay::refund_result(std::map<std::string, std::string>& result)
{
    bool isok = false;
    if (result["return_code"] == "SUCCESS" && result["result_code"] == "SUCCESS")
    {
        // 校验应答签名，防止伪造的退款结果
        std::string sign = result["sign"];
        // 无签名不认：V2 退款应答必带 sign，缺了就是伪造或半截报文。
        if (!sign.empty() && get_sign(result) == sign)
        {
            isok = true;
        }
        else
        {
            result["error_msg"] = sign.empty() ? "退款应答缺少签名" : "退款应答签名校验失败";
        }
    }
    else
    {
        result["error_msg"] = result["err_code_des"].empty() ? result["return_msg"] : result["err_code_des"];
    }
    result["status_code"]   = isok ? "0" : "1";
    result["refund_fee"]    = refund_fee_;
    result["out_refund_no"] = out_refund_no_;
}

void weixinpay::refundquery_result(std::map<std::string, std::string>& result)
{
    bool isok = (result["return_code"] == "SUCCESS" && result["result_code"] == "SUCCESS");
    if (!isok)
    {
        result["error_msg"] = result["err_code_des"].empty() ? result["return_msg"] : result["err_code_des"];
    }
    result["status_code"] = isok ? "0" : "1";
}

std::map<std::string, std::string> weixinpay::refund()
{
    // 申请退款 https://api.mch.weixin.qq.com/secapi/pay/refund
    // 幂等键必须显式给：空则不外发。兜底造号会把"调用点漏 set"掩成一句"退款受理成功"。
    if (out_refund_no_.empty())
    {
        return build_refund_gate_result();
    }
    if (refund_fee_.empty())
    {
        refund_fee_ = total_fee_;// 不指定退款金额时默认全额退款
    }

    std::map<std::string, std::string> params = build_refund_params(generate_nonce_str());
    params["sign"] = get_sign(params);

    std::string resp = postxmlto_cert("https://api.mch.weixin.qq.com/secapi/pay/refund", array_to_xml(params));

    std::map<std::string, std::string> result = parse_xml(resp);
    refund_result(result);
    return result;
}

std::map<std::string, std::string> weixinpay::refundquery()
{
    // 退款查询 https://api.mch.weixin.qq.com/pay/refundquery (不需要证书)
    std::map<std::string, std::string> params = build_refundquery_params(generate_nonce_str());
    params["sign"] = get_sign(params);

    std::string resp = postxmlto("https://api.mch.weixin.qq.com/pay/refundquery", array_to_xml(params));

    std::map<std::string, std::string> result = parse_xml(resp);
    refundquery_result(result);
    return result;
}

std::string weixinpay::unifiedorder()
{
    std::map<std::string, std::string> params = build_unifiedorder_params(generate_nonce_str());
    params["sign"] = get_sign(params);

    std::string xml = array_to_xml(params);
    std::string url = "https://api.mch.weixin.qq.com/pay/unifiedorder";
    xml             = postxmlto(url, xml);
    return unifiedorder_result(xml, "prepay_id");
}

std::string weixinpay::createNative()
{
    // 成功返回扫码链接，如 weixin://wxpay/bizpayurl?pr=xxxx
    std::map<std::string, std::string> params = build_createnative_params(generate_nonce_str());
    params["sign"] = get_sign(params);

    std::string xml = array_to_xml(params);
    std::string url = "https://api.mch.weixin.qq.com/pay/unifiedorder";
    xml             = postxmlto(url, xml);
    return unifiedorder_result(xml, "code_url");
}

std::string weixinpay::getpay()
{
    std::string prepay_id = unifiedorder();
    if (prepay_id.find("ERROR:") == 0)
    {
        return prepay_id;// 下单失败原样返回 "ERROR:..."
    }

    std::map<std::string, std::string> pay_params =
        build_pay_params(prepay_id, generate_nonce_str(), std::to_string(std::time(nullptr)));

    // 再次签名：调起支付的签名口径就是那五个键本身，此时 paySign 与 out_trade_no
    // 还没写进去，所以直接拿 pay_params 签，不必再抄一份同内容的 map。
    pay_params["paySign"]      = get_sign(pay_params);
    pay_params["out_trade_no"] = out_trade_no_;

    return pay_params_to_json(pay_params);
}

std::map<std::string, std::string> weixinpay::handle_notify(const std::string &postData)
{
    std::map<std::string, std::string> params = parse_xml(postData);

    std::string return_code    = params["return_code"];
    std::string result_code    = params["result_code"];
    std::string out_trade_no   = params["out_trade_no"];
    std::string transaction_id = params["transaction_id"];
    std::string total_fee      = params["total_fee"];
    std::string openid         = params["openid"];
    std::string time_end       = params["time_end"];
    std::string sign           = params["sign"];

    if (return_code.empty() || result_code.empty() || out_trade_no.empty())
    {
        params["status_code"] = "1";
        params["error_msg"]   = "<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[参数错误]]></return_msg></xml>";
        return params;
    }

    if (return_code != "SUCCESS" || result_code != "SUCCESS")
    {
        // 支付没成，回 FAIL（微信会重发这条通知）。status_code=2 让调用方知道
        // "报文合法但不是成功支付"，业务字段仍然带着，方便记日志时对上是哪一单。
        params["status_code"] = "2";
        params["error_msg"]   = "<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[FAIL]]></return_msg></xml>";
        return params;
    }

    // 验签用商户 API 密钥：收到的参数（除去 sign 与空值）按 key 排序拼接，末尾加 &key=密钥
    std::map<std::string, std::string> sign_params;
    for (const auto &p : params)
    {
        if (p.first != "sign" && !p.second.empty())
        {
            sign_params.insert(p);
        }
    }
    std::string calc_sign = get_sign(sign_params);// get_sign 内部已经使用了成员变量 apikey_

    if (calc_sign != sign)
    {
        params["status_code"] = "3";
        params["error_msg"]   = "<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[签名错误]]></return_msg></xml>";
        return params;
    }

    params["status_code"] = "0";
    params["error_msg"]   = "<xml><return_code><![CDATA[SUCCESS]]></return_code><return_msg><![CDATA[OK]]></return_msg></xml>";
    return params;
}

// ===== 协程镜像：以上同步方法的 awaitable 版本，内部改用 http::client::async_send() =====

asio::awaitable<std::string> weixinpay::async_post_xml_to(const std::string &url, const std::string &xml_data)
{
    std::string response;
    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    a->post(url);
    a->set_header("Content-Type", "application/xml");
    a->set_body(xml_data);
    a->timeout(8);
    co_await a->async_send();
    if (a->get_status() == 200)
    {
        response = a->get_body();
    }
    else
    {
        response = "<xml><return_code><![CDATA[ERROR]]></return_code><return_msg><![CDATA[URL Error]]></return_msg><result_code><![CDATA[ERROR]]></result_code></xml>";
    }
    co_return response;
}

asio::awaitable<std::string> weixinpay::async_post_xml_to_cert(const std::string &url, const std::string &xml_data)
{
    std::string response;
    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    a->post(url);
    a->set_header("Content-Type", "application/xml");
    a->set_body(xml_data);
    a->timeout(8);
    // 退款等 secapi 接口要求双向证书：携带商户证书与私钥发起 HTTPS 请求
    if (!cert_file_.empty()) a->set_ssl_certificate_file(cert_file_);
    if (!key_file_.empty())  a->set_ssl_private_key_file(key_file_);
    co_await a->async_send();
    if (a->get_status() == 200)
    {
        response = a->get_body();
    }
    else
    {
        response = "<xml><return_code><![CDATA[ERROR]]></return_code><return_msg><![CDATA[URL Error]]></return_msg><result_code><![CDATA[ERROR]]></result_code></xml>";
    }
    co_return response;
}

asio::awaitable<std::string> weixinpay::async_unifiedorder()
{
    std::map<std::string, std::string> params = build_unifiedorder_params(generate_nonce_str());
    params["sign"] = get_sign(params);
    std::string xml = array_to_xml(params);
    std::string url = "https://api.mch.weixin.qq.com/pay/unifiedorder";
    xml             = co_await async_post_xml_to(url, xml);
    co_return unifiedorder_result(xml, "prepay_id");
}

asio::awaitable<std::string> weixinpay::async_create_native()
{
    std::map<std::string, std::string> params = build_createnative_params(generate_nonce_str());
    params["sign"] = get_sign(params);
    std::string xml = array_to_xml(params);
    std::string url = "https://api.mch.weixin.qq.com/pay/unifiedorder";
    xml             = co_await async_post_xml_to(url, xml);
    co_return unifiedorder_result(xml, "code_url");
}

asio::awaitable<std::string> weixinpay::async_getpay()
{
    std::string prepay_id = co_await async_unifiedorder();
    if (prepay_id.find("ERROR:") == 0)
    {
        co_return prepay_id;
    }
    std::map<std::string, std::string> pay_params =
        build_pay_params(prepay_id, generate_nonce_str(), std::to_string(std::time(nullptr)));
    // 签名口径同同步版：此刻 pay_params 就是那五个键，直接签，不再抄一份 map。
    pay_params["paySign"]      = get_sign(pay_params);
    pay_params["out_trade_no"] = out_trade_no_;
    co_return pay_params_to_json(pay_params);
}

asio::awaitable<std::map<std::string, std::string>> weixinpay::async_refund()
{
    // 与同步版同一条闸门：幂等键漏 set 就报错，不外发、不兜底。
    if (out_refund_no_.empty())
    {
        co_return build_refund_gate_result();
    }
    if (refund_fee_.empty()) refund_fee_ = total_fee_;
    std::map<std::string, std::string> params = build_refund_params(generate_nonce_str());
    params["sign"] = get_sign(params);
    std::string resp = co_await async_post_xml_to_cert("https://api.mch.weixin.qq.com/secapi/pay/refund", array_to_xml(params));
    std::map<std::string, std::string> result = parse_xml(resp);
    refund_result(result);
    co_return result;
}

asio::awaitable<std::map<std::string, std::string>> weixinpay::async_refundquery()
{
    std::map<std::string, std::string> params = build_refundquery_params(generate_nonce_str());
    params["sign"] = get_sign(params);
    std::string resp = co_await async_post_xml_to("https://api.mch.weixin.qq.com/pay/refundquery", array_to_xml(params));
    std::map<std::string, std::string> result = parse_xml(resp);
    refundquery_result(result);
    co_return result;
}

}// namespace pay