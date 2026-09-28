#ifndef ALIPAY_PAY_H
#define ALIPAY_PAY_H

#include <string>
#include <map>
#include <asio/awaitable.hpp>

namespace pay
{
class alipay {
public:
    alipay();

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
    // 退款幂等键（out_request_no）。同一笔订单分次退款每次必须不同，
    // 空则 refundTrade 直接返回错误应答、不外发 —— vendor 不替调用点造兜底号。
    void setOutRequestNo(const std::string& no) { out_request_no_ = no; }
    // 电脑网站支付前置模式（qr_pay_mode/qrcode_width）。空则不发这两个键。
    void setQrPayMode(const std::string& mode) { qr_pay_mode_ = mode; }
    void setQrcodeWidth(const std::string& width) { qrcode_width_ = width; }

    std::string getAppId() const { return app_id_; }
    std::string getOutTradeNo() const { return out_trade_no_; }

    // 密钥诊断：读不出密钥时 createTrade* 只给空 sign，排障时用这两个独立接口定位密钥本身。
    bool privateKeyReadable() const;
    bool publicKeyReadable() const;

    // 诊断回显：最近一次 buildRequest() 交给网关的「待签名串」和「实际发出的 body」。
    // 没有私钥，可以安全回显。用于排查 isv.invalid-signature。
    const std::string &lastSignContent() const { return last_sign_content_; }
    const std::string &lastRequestBody() const { return last_request_body_; }

    std::string createTrade();
    std::string createTradeQRCode();
    std::string createTradePage();
    std::string queryTrade(const std::string& out_trade_no = "");
    std::string cancelTrade(const std::string& out_trade_no = "");
    std::string refundTrade(const std::string& refund_amount, const std::string& refund_reason = "", const std::string& out_trade_no = "");
    // 协程版孪生（报文组装共用 buildRequest()，只有网关那一跳换成 co_await）。
    // 只有 query 有协程调用点；createTrade/createTradePage 无网络往返，其余无协程调用。
    asio::awaitable<std::string> async_query_trade(const std::string& out_trade_no = "");
    // 异步通知：验签 + trade_status 两道都过才算 0（落账依据只认这一条）
    std::map<std::string, std::string> handleNotify(const std::string& postData);
    // 同步回跳（alipay.trade.page.pay.return）：参数表无 trade_status，只验签。不是落账依据。
    std::map<std::string, std::string> handleReturn(const std::string& queryString);

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
    std::string out_request_no_;
    std::string qr_pay_mode_;
    std::string qrcode_width_;

    std::string last_sign_content_;
    std::string last_request_body_;

    std::string getApiUrl();
    std::string buildRequest(const std::string& method, const std::map<std::string, std::string>& biz_content);
    std::string rsaSign(const std::string& content);
    bool rsaVerify(const std::string& content, const std::string& sign);
    // exclude_sign_type: 请求签名侧传 false（sign_type 参与签名），异步通知验签侧传 true
    std::string buildSignContent(const std::map<std::string, std::string>& params, bool exclude_sign_type = false);
    std::string getTimestamp();
    std::string postToUrl(const std::string& url, const std::string& data);
    asio::awaitable<std::string> async_post_to_url(const std::string& url, const std::string& data);
    // 回调报文公共前半段：解析 + 按通知口径验签，status_code 只会是 0/1/2。
    // 错误分支也带着已解析的参数回来，否则日志里的状态码配不上"是哪一单"。
    std::map<std::string, std::string> verifySignedParams(std::map<std::string, std::string> params);
};
}

#endif