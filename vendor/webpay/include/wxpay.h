#ifndef WXPAY_PAY_H
#define WXPAY_PAY_H

#include <string>
#include <map>

namespace pay
{
class wxpay {
public:
    wxpay();
    wxpay(std::map<std::string, std::string>&& params);

    void setAppId(const std::string& id) { app_id_ = id; }
    void setMchId(const std::string& id) { mch_id_ = id; }
    void setPrivateKey(const std::string& key) { private_key_ = key; }
    void setPublicKey(const std::string& key) { public_key_ = key; }
    void setApiKey(const std::string& key) { api_key_ = key; }
    void setSerialNo(const std::string& no) { serial_no_ = no; }
    void setOutTradeNo(const std::string& no) { out_trade_no_ = no; }
    void setDescription(const std::string& desc) { description_ = desc; }
    void setBody(const std::string& b) { body_ = b; }
    void setTotalAmount(const std::string& amount) { total_amount_ = amount; }
    void setNotifyUrl(const std::string& url) { notify_url_ = url; }
    void setReturnUrl(const std::string& url) { return_url_ = url; }
    void setOpenId(const std::string& id) { openid_ = id; }
    void setSandbox(bool sandbox) { is_sandbox_ = sandbox; }

    std::string getAppId() const { return app_id_; }
    std::string getMchId() const { return mch_id_; }
    std::string getOutTradeNo() const { return out_trade_no_; }

    std::string createNative();
    std::string createJSAPI();
    std::string createAPP();
    std::string createH5();
    std::string createMiniProgram();
    std::string queryTrade(const std::string& out_trade_no = "");
    std::string closeTrade(const std::string& out_trade_no = "");
    std::string refundTrade(const std::string& refund_amount, const std::string& refund_reason = "", const std::string& out_trade_no = "");
    std::string queryRefund(const std::string& out_trade_no = "");
    std::string downloadCertificates();
    std::map<std::string, std::string> handleNotify(const std::string& postData, const std::map<std::string, std::string>& headers);

private:
    std::string app_id_;
    std::string mch_id_;
    std::string private_key_;
    std::string public_key_;
    std::string api_key_;
    std::string serial_no_;
    std::string out_trade_no_;
    std::string description_;
    std::string body_;
    std::string total_amount_;
    std::string notify_url_;
    std::string return_url_;
    std::string openid_;
    bool is_sandbox_;

    std::string getApiUrl();
    std::string buildAuthorization(const std::string& method, const std::string& path, const std::string& body);
    std::string rsaSign(const std::string& content);
    bool rsaVerify(const std::string& content, const std::string& sign);
    std::string generateNonceStr();
    std::string getTimestamp();
    std::map<std::string, std::string> parseResponse(const std::string& response);
    std::string httpRequest(const std::string& method, const std::string& path, const std::string& body = "");
    std::string aesGcmDecrypt(const std::string& ciphertext, const std::string& nonce, const std::string& associated_data);
};
}

#endif