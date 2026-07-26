#include <chrono>
#include <thread>
#include <filesystem>
#include "httppeer.h"
#include "test_wxpay.h"
#include "func.h"
#include "request.h"
#include "httpclient.h"
#ifdef ENABLE_WEBPAY
#include "wxpay.h"
#endif
#ifdef ENABLE_IMAGE
#include "qrcode.h"
#include "pzpng.h"
#endif

namespace http
{

//@urlpath(null,testwxpaynative)
std::string test_wxpay_native(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setPublicKey("你的微信平台证书路径或内容");
    wp.setApiKey("你的微信API密钥");
    wp.setSerialNo("你的证书序列号");
    wp.setOutTradeNo(out_trade_no);
    wp.setDescription("测试商品");
    wp.setTotalAmount("1");
    wp.setNotifyUrl("https://yourdomain.com/wxpaynotify");

    std::string response = wp.createNative();
    client.output = response;

#ifdef ENABLE_IMAGE
    http::obj_val resp_json;
    resp_json.from_json(response);
    std::string qrcontent = resp_json["code_url"].to_string();

    if (!qrcontent.empty()) {
        namespace fs = std::filesystem;
        qr::qrcode q;
        q.text(qrcontent, qr::Ecc::M, 1);
        image::png img;
        unsigned char scale = 10;
        img.create(q.width()*scale + 40, q.height()*scale + 40, 6, 8);
        img.fillColor({255, 255, 255, 255});
        img.qrdata(q.data,q.width(),q.height(),scale, 20 ,20);
        
        std::string wwwpath;
        wwwpath.append(client.get_sitepath());
        wwwpath.append("/upload");

        fs::path paths = wwwpath;
        bool is_success = fs::create_directories(paths);
        if (is_success) {
            fs::permissions(paths,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add);
        }

        wwwpath.append("/wxpay_qr.png");
        img.save(wwwpath);

        client << "<img src=\"/upload/wxpay_qr.png?token=" << rand_string(6, 6) << "\">";
    }
#endif
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,testwxpayjsapi)
std::string test_wxpay_jsapi(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setPublicKey("你的微信平台证书路径或内容");
    wp.setApiKey("你的微信API密钥");
    wp.setSerialNo("你的证书序列号");
    wp.setOutTradeNo(out_trade_no);
    wp.setDescription("测试商品");
    wp.setTotalAmount("1");
    wp.setNotifyUrl("https://yourdomain.com/wxpaynotify");
    wp.setOpenId("用户的OpenID");

    std::string response = wp.createJSAPI();
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,testwxpayapp)
std::string test_wxpay_app(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setPublicKey("你的微信平台证书路径或内容");
    wp.setApiKey("你的微信API密钥");
    wp.setSerialNo("你的证书序列号");
    wp.setOutTradeNo(out_trade_no);
    wp.setDescription("测试商品");
    wp.setTotalAmount("1");
    wp.setNotifyUrl("https://yourdomain.com/wxpaynotify");

    std::string response = wp.createAPP();
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,testwxpayh5)
std::string test_wxpay_h5(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setPublicKey("你的微信平台证书路径或内容");
    wp.setApiKey("你的微信API密钥");
    wp.setSerialNo("你的证书序列号");
    wp.setOutTradeNo(out_trade_no);
    wp.setDescription("测试商品");
    wp.setTotalAmount("1");
    wp.setNotifyUrl("https://yourdomain.com/wxpaynotify");

    std::string response = wp.createH5();
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,testwxpayquery)
std::string test_wxpay_query(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = client.get["out_trade_no"].to_string();

    if (out_trade_no.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少out_trade_no\"}";
        return "";
    }

    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setPublicKey("你的微信平台证书路径或内容");
    wp.setApiKey("你的微信API密钥");
    wp.setSerialNo("你的证书序列号");

    std::string response = wp.queryTrade(out_trade_no);
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,testwxpayclose)
std::string test_wxpay_close(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = client.get["out_trade_no"].to_string();

    if (out_trade_no.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少out_trade_no\"}";
        return "";
    }

    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setPublicKey("你的微信平台证书路径或内容");
    wp.setApiKey("你的微信API密钥");
    wp.setSerialNo("你的证书序列号");

    std::string response = wp.closeTrade(out_trade_no);
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,testwxpayrefund)
std::string test_wxpay_refund(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = client.get["out_trade_no"].to_string();
    std::string refund_amount = client.get["refund_amount"].to_string();

    if (out_trade_no.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少out_trade_no\"}";
        return "";
    }
    if (refund_amount.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少refund_amount\"}";
        return "";
    }

    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setPublicKey("你的微信平台证书路径或内容");
    wp.setApiKey("你的微信API密钥");
    wp.setSerialNo("你的证书序列号");
    wp.setTotalAmount("1");

    std::string response = wp.refundTrade(refund_amount, "测试退款", out_trade_no);
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,wxpaynotify)
std::string test_wxpay_notify(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("Content-Type: application/json; charset=utf-8");
#ifdef ENABLE_WEBPAY
    std::map<std::string, std::string> headers;
    headers["Wechatpay-Timestamp"] = client.get_header("Wechatpay-Timestamp");
    headers["Wechatpay-Nonce"] = client.get_header("Wechatpay-Nonce");
    headers["Wechatpay-Signature"] = client.get_header("Wechatpay-Signature");
    headers["Wechatpay-Serial"] = client.get_header("Wechatpay-Serial");

    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setPublicKey("你的微信平台证书路径或内容");
    wp.setApiKey("你的微信API密钥");

    auto resp = wp.handleNotify(client.rawcontent, headers);

    if (resp["status_code"] == "0") {
        std::string out_trade_no = resp["out_trade_no"];
        std::string transaction_id = resp["transaction_id"];
        std::string amount = resp["amount"];
        client.output = "{\"code\":\"SUCCESS\",\"message\":\"成功\"}";
    } else {
        client.output = "{\"code\":\"FAIL\",\"message\":\"" + resp["error_msg"] + "\"}";
    }
#else
    client.output = "{\"code\":\"FAIL\",\"message\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,wxpaydownloadcert)
std::string test_wxpay_download_cert(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    pay::wxpay wp;
    wp.setAppId("你的微信APPID");
    wp.setMchId("你的微信商户号");
    wp.setPrivateKey("你的微信商户私钥路径或内容");
    wp.setApiKey("你的微信API密钥");

    std::string response = wp.downloadCertificates();
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

}
