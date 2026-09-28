#ifndef WXPAY_PAY_H
#define WXPAY_PAY_H

#include <string>
#include <map>
#include <asio/awaitable.hpp>// async_* 成员声明用了 asio::awaitable，与 alipay.h 同法自含

namespace pay
{
// 一张平台证书：AES-GCM 解密后的 PEM 正文 + 应答里同对象的 expire_time。
// 两者成对返回：证书列表页一次下载要同时拿到 PEM 和过期时间。
struct wx_cert_t
{
    std::string pem;
    std::string expire_time;
};
class wxpay {
public:
    wxpay();
    wxpay(std::map<std::string, std::string>&& params);

    void setAppId(const std::string& id) { app_id_ = id; }
    void setMchId(const std::string& id) { mch_id_ = id; }
    void setPrivateKey(const std::string& key) { private_key_ = key; }
    void setPublicKey(const std::string& key) { public_key_ = key; }
    // 商户 API 证书文件（PEM 路径或 PEM 内容），用于 V3 接口的 mTLS 双向证书握手
    void setCertFile(const std::string& f) { cert_file_ = f; }
    void setApiKey(const std::string& key) { api_key_ = key; }
    // APIv3 密钥（商户平台 → API安全 → APIv3 密钥），用于解密回调/平台证书。
    // 不设置时回退使用 api_key_（老配置方式）
    void setApiV3Key(const std::string& key) { api_v3_key_ = key; }
    // 微信支付平台证书（PEM 内容或文件路径），用于验证回调签名
    void setPlatformCert(const std::string& pem) { public_key_ = pem; }
    void setSerialNo(const std::string& no) { serial_no_ = no; }
    void setOutTradeNo(const std::string& no) { out_trade_no_ = no; }
    void setDescription(const std::string& desc) { description_ = desc; }
    void setBody(const std::string& b) { body_ = b; }
    void setTotalAmount(const std::string& amount) { total_amount_ = amount; }
    // 退款幂等键必须显式给：空值时 refundTrade / async_refund_trade 直接返回错误应答、不外发。
    // SDK 不替调用点造号（造号 ⇒ 同一笔每点一次退款都是新单号，重复退款拦不住）；
    // 幂等键生成属于订单层，按 webpay::refund_no_gen() 传进来。
    void setOutRefundNo(const std::string& no) { out_refund_no_ = no; }
    void setNotifyUrl(const std::string& url) { notify_url_ = url; }
    void setReturnUrl(const std::string& url) { return_url_ = url; }
    void setOpenId(const std::string& id) { openid_ = id; }
    void setSandbox(bool sandbox) { is_sandbox_ = sandbox; }

    std::string getAppId() const { return app_id_; }
    std::string getMchId() const { return mch_id_; }
    std::string getOutTradeNo() const { return out_trade_no_; }

    // 下单接口返回 JSON。
    // 成功时 createJSAPI/createMiniProgram 返回可直接调起支付的完整参数
    // {appId,timeStamp,nonceStr,package,signType,paySign,out_trade_no}；
    // 失败时原样返回微信的错误响应便于排查。
    std::string createNative();
    std::string createJSAPI();
    std::string createAPP();
    std::string createH5();
    std::string createMiniProgram();
    std::string queryTrade(const std::string& out_trade_no = "");
    std::string closeTrade(const std::string& out_trade_no = "");
    std::string refundTrade(const std::string& refund_amount, const std::string& refund_reason = "", const std::string& out_trade_no = "");
    std::string queryRefund(const std::string& out_trade_no = "");
    std::string downloadCertificates();               // 下载平台证书原始响应
    // 下载并解密，返回 证书序列号 -> {PEM 正文, expire_time}（失败为空）
    std::map<std::string, wx_cert_t> downloadPlatformCerts();
    // 把 /v3/certificates 应答解析成上面的表：纯函数不外发，sync/async 共用。
    std::map<std::string, wx_cert_t> parsePlatformCerts(const std::string& response);
    bool downloadAndSaveCert();                       // 下载并保存到内存（供 handleNotify 验签）
    std::string notifySuccessReply();                 // V3 回调成功应答 "SUCCESS"
    std::string notifyFailReply(const std::string& message = ""); // V3 回调失败应答 "FAIL"

    std::map<std::string, std::string> handleNotify(const std::string& postData, const std::map<std::string, std::string>& headers);
    // 从 jsapi 下单应答里读 prepay_id：非 JSON 或缺字段一律空串（调用方据此原样回吐应答）。
    // 纯函数不外发，提 public static 便于直接喂应答报文核对读法。
    static std::string extract_prepay_id(const std::string& response);

    // ===== 报文拼装：同步版与协程版唯一共用的构造口，const 不发网络 =====
    // 提 public 便于直接喂边界输入核对字节。
    std::string build_native_body() const;
    std::string build_jsapi_body() const;
    std::string build_app_body() const;
    std::string build_h5_body() const;
    std::string build_refund_body(const std::string& refund_amount, const std::string& refund_reason, const std::string& out_trade_no) const;
    std::string build_query_trade_path(const std::string& out_trade_no) const;
    std::string build_close_trade_path(const std::string& out_trade_no) const;
    std::string build_query_refund_path(const std::string& out_trade_no) const;

    // 协程版（awaitable，内部 http::client::async_send()，与上面同步方法一一对应）
    asio::awaitable<std::string> async_create_native();
    asio::awaitable<std::string> async_create_jsapi();
    asio::awaitable<std::string> async_create_app();
    asio::awaitable<std::string> async_create_h5();
    asio::awaitable<std::string> async_create_mini_program();
    asio::awaitable<std::string> async_query_trade(const std::string& out_trade_no = "");
    asio::awaitable<std::string> async_close_trade(const std::string& out_trade_no = "");
    asio::awaitable<std::string> async_refund_trade(const std::string& refund_amount, const std::string& refund_reason = "", const std::string& out_trade_no = "");
    asio::awaitable<std::string> async_query_refund(const std::string& out_trade_no = "");
    asio::awaitable<std::string> async_download_certificates();
    asio::awaitable<std::map<std::string, wx_cert_t>> async_download_platform_certs();
    asio::awaitable<bool> async_download_and_save_cert();

private:
    asio::awaitable<std::string> async_http_request(const std::string& method, const std::string& path, const std::string& body = "");
    std::string app_id_;
    std::string mch_id_;
    std::string private_key_;
    std::string cert_file_;   // 商户 API 证书（mTLS 握手用）
    std::string public_key_;
    std::string api_key_;
    std::string api_v3_key_;
    std::string serial_no_;
    std::string out_trade_no_;
    std::string description_;
    std::string body_;
    std::string total_amount_;
    std::string out_refund_no_;
    std::string notify_url_;
    std::string return_url_;
    std::string openid_;
    bool is_sandbox_;
    std::map<std::string, std::string> platform_certs_; // 证书序列号 -> PEM

    std::string getApiUrl();
    std::string getApiV3Key() const { return api_v3_key_.empty() ? api_key_ : api_v3_key_; }
    std::string buildAuthorization(const std::string& method, const std::string& path, const std::string& body);
    std::string buildPayParams(const std::string& prepay_id);
    std::string rsaSign(const std::string& content);
    bool rsaVerify(const std::string& content, const std::string& sign, const std::string& cert_pem = "");
    std::string generateNonceStr();
    std::string getTimestamp();
    std::string httpRequest(const std::string& method, const std::string& path, const std::string& body = "");
    std::string aesGcmDecrypt(const std::string& ciphertext, const std::string& nonce, const std::string& associated_data);
};
}

#endif
