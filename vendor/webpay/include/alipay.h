#ifndef ALIPAY_PAY_H
#define ALIPAY_PAY_H

#include <string>
#include <map>

namespace pay
{
class alipay {
public:
    alipay();
    alipay(std::map<std::string, std::string>&& params);

    void setAppId(const std::string& id) { app_id_ = id; }
    void setPrivateKey(const std::string& key) { private_key_ = key; }
    void setPublicKey(const std::string& key) { public_key_ = key; }
    void setOutTradeNo(const std::string& no) { out_trade_no_ = no; }
    void setSubject(const std::string& sub) { subject_ = sub; }
    void setBody(const std::string& b) { body_ = b; }
    void setTotalAmount(const std::string& amount) { total_amount_ = amount; }
    void setNotifyUrl(const std::string& url) { notify_url_ = url; }
    void setReturnUrl(const std::string& url) { return_url_ = url; }
    void setProductCode(const std::string& code) { product_code_ = code; }
    void setAuthToken(const std::string& token) { auth_token_ = token; }
    void setAppAuthToken(const std::string& token) { app_auth_token_ = token; }
    void setSandbox(bool sandbox) { is_sandbox_ = sandbox; }

    std::string getAppId() const { return app_id_; }
    std::string getOutTradeNo() const { return out_trade_no_; }

    std::string createTrade();
    std::string createTradeQRCode();
    std::string createTradePage();
    std::string queryTrade(const std::string& out_trade_no = "");
    std::string cancelTrade(const std::string& out_trade_no = "");
    std::string refundTrade(const std::string& refund_amount, const std::string& refund_reason = "", const std::string& out_trade_no = "");
    std::string queryRefund(const std::string& out_trade_no = "");
    std::map<std::string, std::string> handleNotify(const std::string& postData);

private:
    std::string app_id_;
    std::string private_key_;
    std::string public_key_;
    std::string out_trade_no_;
    std::string subject_;
    std::string body_;
    std::string total_amount_;
    std::string notify_url_;
    std::string return_url_;
    std::string product_code_;
    std::string auth_token_;
    std::string app_auth_token_;
    bool is_sandbox_;

    std::string getApiUrl(const std::string& method);
    std::string buildRequest(const std::string& method, const std::map<std::string, std::string>& biz_content);
    std::string rsaSign(const std::string& content);
    bool rsaVerify(const std::string& content, const std::string& sign);
    std::string buildSignContent(const std::map<std::string, std::string>& params);
    std::string generateNonceStr();
    std::string getTimestamp();
    std::map<std::string, std::string> parseResponse(const std::string& response);
    std::string postToUrl(const std::string& url, const std::string& data);
};
}

#endif