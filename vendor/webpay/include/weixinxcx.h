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
    void setTotalFee(const std::string& fee) { total_fee_ = fee; }  // 单位：分
    void setNotifyUrl(const std::string& url) { notify_url_ = url; }
    void setTransactionId(const std::string& id) { transaction_id_ = id; }

    // 退款设置。out_refund_no 是幂等键，必须显式给：
    // 空值时 refund()/async_refund() 直接返回失败、不外发，SDK 不兜底造号。
    void setOutRefundNo(const std::string& no) { out_refund_no_ = no; }
    void setRefundFee(const std::string& fee) { refund_fee_ = fee; }
    void setRefundDesc(const std::string& d) { refund_desc_ = d; }
    void setCertFile(const std::string& file) { cert_file_ = file; }
    void setKeyFile(const std::string& file) { key_file_ = file; }

    std::string getOutRefundNo() const { return out_refund_no_; }

    // ===== 核心接口 =====
    std::string getpay();          // 统一下单 + 生成调起支付参数
    std::string unifiedorder();    // 仅统一下单，返回 prepay_id 或原始 XML
    std::string createNative();    // NATIVE 扫码下单，返回 code_url
    std::map<std::string, std::string> handle_notify(const std::string& postData);
    std::map<std::string, std::string> refund();         // 申请退款（需商户双向证书）
    std::map<std::string, std::string> refundquery();    // 退款查询（不需要证书）

    // ===== 请求参数与应答解析（同步版与协程版唯一共用的构造口）=====
    // nonce 必须由调用方传进来（构造口内不取随机），sign 由调用方写好——构造口不含 sign。
    std::map<std::string, std::string> build_unifiedorder_params(const std::string& nonce) const;
    std::map<std::string, std::string> build_createnative_params(const std::string& nonce) const;
    std::map<std::string, std::string> build_refund_params(const std::string& nonce) const;
    std::map<std::string, std::string> build_refundquery_params(const std::string& nonce) const;
    std::map<std::string, std::string> build_pay_params(const std::string& prepay_id,
                                                        const std::string& nonce,
                                                        const std::string& timestamp) const;
    std::map<std::string, std::string> build_refund_gate_result() const;    // 缺幂等键时的固定失败应答
    static std::string pay_params_to_json(const std::map<std::string, std::string>& pay_params);
    // 统一下单应答 -> want_field（prepay_id / code_url），失败返回 "ERROR:..." 串
    std::string unifiedorder_result(const std::string& xml, const std::string& want_field);
    void refund_result(std::map<std::string, std::string>& result);      // 校验应答签名 + status_code
    void refundquery_result(std::map<std::string, std::string>& result);

    // 协程版（awaitable，与同步方法一一对应）
    asio::awaitable<std::string> async_unifiedorder();
    asio::awaitable<std::string> async_create_native();
    asio::awaitable<std::string> async_getpay();
    asio::awaitable<std::map<std::string, std::string>> async_refund();
    asio::awaitable<std::map<std::string, std::string>> async_refundquery();
private:
    asio::awaitable<std::string> async_post_xml_to(const std::string& url, const std::string& xml_data);
    asio::awaitable<std::string> async_post_xml_to_cert(const std::string& url, const std::string& xml_data);
    std::string appid_;
    std::string mch_id_;
    std::string apikey_;
    std::string openid_;
    std::string out_trade_no_;
    std::string body_;
    std::string client_ip_;
    std::string total_fee_;        // 分
    std::string notify_url_;
    std::string transaction_id_;
    std::string out_refund_no_;
    std::string refund_fee_;
    std::string refund_desc_;
    std::string cert_file_;        // apiclient_cert.pem
    std::string key_file_;         // apiclient_key.pem

    std::string postxmlto(const std::string& url, const std::string& xml_data);
    std::string postxmlto_cert(const std::string& url, const std::string& xml_data); // 退款 secapi 必须
    // 请求体唯一构造口：所有值走 CDATA（值里出现 `<`/`&` 也不破坏报文）。
    std::string array_to_xml(const std::map<std::string, std::string>& params);
    
    std::string kvmap_to_string(const std::map<std::string, std::string>& params, bool for_sign = true);
    std::string get_sign(const std::map<std::string, std::string>& params);
    std::string generate_nonce_str();
    std::map<std::string, std::string> parse_xml(const std::string& xml);
    std::string urlencode(const std::string& str);  // 备用
};
}
#endif // WEIXINPAY_H