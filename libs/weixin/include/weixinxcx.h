#ifndef WEIXINPAY_XCX_H
#define WEIXINPAY_XCX_H

#include <string>
#include <map>

namespace pay
{
class weixinpay {
public:
    weixinpay();
    // 使用键值对初始化对象（移动语义）
    weixinpay(std::map<std::string, std::string>&& params);

    // 设置/获取成员变量
    void setAppid(const std::string& id) { appid_ = id; }
    void setMchId(const std::string& id) { mch_id_ = id; }
    void setApiKey(const std::string& key) { apikey_ = key; }
    void setOpenid(const std::string& id) { openid_ = id; }
    void setOutTradeNo(const std::string& no) { out_trade_no_ = no; }
    void setBody(const std::string& b) { body_ = b; }
    void setClientIp(const std::string& b) { client_ip_ = b; }
    void setTotalFee(const std::string& fee) { total_fee_ = fee; }  // 注意字段名修正为 total_fee
    void settNotifyUrl(const std::string& url) { notify_url_ = url; }  // 注意字段名修正为 total_fee

    // 核心接口
    std::string getpay();          // 统一下单 + 生成调起支付参数
    std::string unifiedorder();    // 仅统一下单，返回 prepay_id 或原始 XML
    std::map<std::string, std::string> handle_notify(const std::string& postData);
private:
    // 成员变量（私有）
    std::string appid_;
    std::string mch_id_;
    std::string apikey_;
    std::string openid_;
    std::string out_trade_no_;
    std::string body_;
    std::string client_ip_;
    std::string total_fee_;        // 单位：分，对应微信的 total_fee
    std::string notify_url_;

    // 辅助函数
    std::string postxmlto(const std::string& url, const std::string& xml_data);
    std::string array_to_xml(const std::map<std::string, std::string>& params);
    std::string post_to_xml(const std::map<std::string, std::string>& params);
    
    std::string kvmap_to_string(const std::map<std::string, std::string>& params, bool for_sign = true);
    std::string get_sign(const std::map<std::string, std::string>& params);
    std::string generate_nonce_str();
    std::map<std::string, std::string> parse_xml(const std::string& xml);
    std::string urlencode(const std::string& str);  // 备用
};
}
#endif // WEIXINPAY_H