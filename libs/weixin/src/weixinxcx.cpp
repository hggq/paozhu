#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <ctime>
#include <functional>
#include <iostream>
#include "httpclient.h"
#include "weixinxcx.h"
#include "md5.h"

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
    std::mt19937 gen(rd());
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

std::string weixinpay::post_to_xml(const std::map<std::string, std::string> &params)
{
    std::string xml = "<xml>";
    for (const auto &p : params)
    {
        xml += "<" + p.first + ">" + p.second + "</" + p.first + ">";
    }
    xml += "</xml>";
    return xml;
}

std::string weixinpay::array_to_xml(const std::map<std::string, std::string> &params)
{
    std::string xml = "<xml>";
    for (const auto &p : params)
    {
        xml += "<" + p.first + "><![CDATA[" + p.second + "]]></" + p.first + ">";
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
        if (content.find("<![CDATA[") == 0)
        {
            content = content.substr(9, content.size() - 12);// 去掉 <![CDATA[ 和 ]]>
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
    a->timeout(20);
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

std::string weixinpay::unifiedorder()
{
    // 构造请求参数
    std::map<std::string, std::string> params;
    params["appid"]            = appid_;
    params["mch_id"]           = mch_id_;
    params["nonce_str"]        = generate_nonce_str();
    params["body"]             = body_;
    params["out_trade_no"]     = out_trade_no_;
    params["total_fee"]        = total_fee_; // 注意字段名是 total_fee
    params["spbill_create_ip"] = client_ip_; // 实际项目中需从请求上下文中获取
    params["notify_url"]       = notify_url_;// "https://myweixin.xxx.com/xcxnotify";  // 配置你的通知地址

    params["openid"]     = openid_;// JSAPI 支付必需
    params["trade_type"] = "JSAPI";// 根据业务调整

    // 生成签名
    std::string sign = get_sign(params);
    params["sign"]   = sign;

    // 转为 XML
    std::string xml = post_to_xml(params);
    // 发送请求
    std::string url = "https://api.mch.weixin.qq.com/pay/unifiedorder";
    xml             = postxmlto(url, xml);
    // 解析返回的 XML
    std::map<std::string, std::string> resp = parse_xml(xml);

    // 根据返回结果处理
    if (resp["return_code"] == "SUCCESS" && resp["result_code"] == "SUCCESS")
    {
        return resp["prepay_id"];// 返回 prepay_id
    }
    else
    {
        // 返回错误信息（可根据需要格式化）
        return "ERROR:" + resp["return_msg"] + ";" + resp["err_code_des"];
    }
}

std::string weixinpay::getpay()
{
    // 1. 调用统一下单获取 prepay_id
    std::string prepay_id = unifiedorder();
    if (prepay_id.find("ERROR:") == 0)
    {
        return prepay_id;// 返回错误
    }

    // 2. 构造调起支付所需的参数（以小程序/JSAPI 为例）
    std::map<std::string, std::string> pay_params;
    pay_params["appId"]     = appid_;
    pay_params["timeStamp"] = std::to_string(std::time(nullptr));
    pay_params["nonceStr"]  = generate_nonce_str();
    pay_params["package"]   = "prepay_id=" + prepay_id;
    pay_params["signType"]  = "MD5";

    // 再次签名（注意字段名大小写，微信要求使用小写键名计算签名）
    std::map<std::string, std::string> sign_params;
    sign_params["appid"]  = appid_;
    sign_params["mch_id"] = mch_id_;// 实际支付签名不需要 mch_id，此处仅作示例，具体参考微信文档
    // 正确的支付签名所需字段：appId, timeStamp, nonceStr, package, signType
    // 但我们使用 pay_params 中的值重新组织
    sign_params.clear();
    sign_params["appId"]     = appid_;
    sign_params["timeStamp"] = pay_params["timeStamp"];
    sign_params["nonceStr"]  = pay_params["nonceStr"];
    sign_params["package"]   = pay_params["package"];
    sign_params["signType"]  = pay_params["signType"];
    std::string pay_sign     = get_sign(sign_params);// 注意：这里使用类的 get_sign，内部会拼接 &key=apikey_

    pay_params["paySign"]      = pay_sign;
    pay_params["out_trade_no"] = out_trade_no_;

    // 将 pay_params 转换为 JSON 格式返回（简易拼接，生产建议使用 json 库）
    std::string json = "{";
    for (auto it = pay_params.begin(); it != pay_params.end(); ++it)
    {
        if (it != pay_params.begin())
            json += ",";
        json += "\"" + it->first + "\":\"" + it->second + "\"";
    }
    json += "}";
    return json;
}

std::map<std::string, std::string> weixinpay::handle_notify(const std::string &postData)
{
    // 1. 解析 XML 为 map
    std::map<std::string, std::string> params = parse_xml(postData);

    // 2. 提取关键字段
    std::string return_code    = params["return_code"];
    std::string result_code    = params["result_code"];
    std::string out_trade_no   = params["out_trade_no"];
    std::string transaction_id = params["transaction_id"];
    std::string total_fee      = params["total_fee"];
    std::string openid         = params["openid"];
    std::string time_end       = params["time_end"];
    std::string sign           = params["sign"];

    // 3. 验证必要字段是否存在
    if (return_code.empty() || result_code.empty() || out_trade_no.empty())
    {
        // 参数不全，返回失败
        params["status_code"] = "1";
        params["error_msg"]   = "<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[参数错误]]></return_msg></xml>";
        return params;
    }

    // 4. 检查通信和业务结果
    if (return_code != "SUCCESS" || result_code != "SUCCESS")
    {
        // 支付失败，记录日志后返回成功（避免微信重复通知）
        // 实际项目中可能需要记录失败原因
        params["status_code"] = "2";
        params["error_msg"]   = "<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[FAIL]]></return_msg></xml>";
        return params;
    }

    // 5. 验证签名（使用商户 API 密钥）
    //    将收到的参数（除去 sign 字段）按照 key 排序后拼接，最后加上 &key=商户密钥
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
        // 签名验证失败，返回失败
        params["status_code"] = "3";
        params["error_msg"]   = "<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[签名错误]]></return_msg></xml>";
        return params;
    }

    params["status_code"] = "0";
    params["error_msg"]   = "<xml><return_code><![CDATA[SUCCESS]]></return_code><return_msg><![CDATA[OK]]></return_msg></xml>";
    return params;
}

}// namespace pay