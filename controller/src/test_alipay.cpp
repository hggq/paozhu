#include <chrono>
#include <thread>
#include <filesystem>
#include "httppeer.h"
#include "test_alipay.h"
#include "func.h"
#include "request.h"
#include "httpclient.h"
#ifdef ENABLE_WEBPAY
#include "alipay.h"
#endif
#ifdef ENABLE_IMAGE
#include "qrcode.h"
#include "pzpng.h"
#endif

namespace http
{

//@urlpath(null,testalipayapp)
std::string test_alipay_app(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::alipay apay;
    apay.setAppId("你的支付宝APPID");
    apay.setPrivateKey("你的支付宝私钥");
    apay.setPublicKey("你的支付宝公钥");
    
    apay.setOutTradeNo(out_trade_no);
    apay.setSubject("测试商品");
    apay.setBody("测试商品描述");
    apay.setTotalAmount("0.01");
    apay.setNotifyUrl("https://yourdomain.com/alipaynotify");
    apay.setProductCode("QUICK_MSECURITY_PAY");

    std::string pay_params = apay.createTrade();
    client.output = pay_params;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,testalipayqrcode)
std::string test_alipay_qrcode(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::alipay apay;
    apay.setAppId("你的支付宝APPID");
    apay.setPrivateKey("你的支付宝私钥");
    apay.setPublicKey("你的支付宝公钥");

    apay.setOutTradeNo(out_trade_no);
    apay.setSubject("测试商品");
    apay.setTotalAmount("0.01");
    apay.setNotifyUrl("https://yourdomain.com/alipaynotify");
    apay.setSandbox(true);

    std::string response = apay.createTradeQRCode();

#ifdef ENABLE_IMAGE
    http::obj_val resp_json;
    resp_json.from_json(response);
    std::string qrcontent = resp_json["qr_code"].to_string();

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

        wwwpath.append("/alipay_qr.png");
        img.save(wwwpath);

        client << "<img src=\"/upload/alipay_qr.png?token=" << rand_string(6, 6) << "\">";
    } else {
        client.output = response;
    }
#else
    client.output = response;
#endif
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,testalipaypage)
std::string test_alipay_page(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::alipay apay;
    apay.setAppId("你的支付宝APPID");
    apay.setPrivateKey("你的支付宝私钥");
    apay.setPublicKey("你的支付宝公钥");
    apay.setOutTradeNo(out_trade_no);
    apay.setSubject("测试商品");
    apay.setBody("测试商品描述");
    apay.setTotalAmount("0.01");
    apay.setNotifyUrl("https://yourdomain.com/alipaynotify");
    apay.setReturnUrl("https://yourdomain.com/alipayreturn");

    std::string pay_url = apay.createTradePage();
    client.output = pay_url;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,testalipayquery)
std::string test_alipay_query(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = client.get["out_trade_no"].to_string();

    if (out_trade_no.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少out_trade_no\"}";
        return "";
    }

    pay::alipay apay;
    apay.setAppId("你的支付宝APPID");
    apay.setPrivateKey("你的支付宝私钥");
    apay.setPublicKey("你的支付宝公钥");
    apay.setSandbox(true);

    std::string response = apay.queryTrade(out_trade_no);
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,testalipaycancel)
std::string test_alipay_cancel(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = client.get["out_trade_no"].to_string();

    if (out_trade_no.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少out_trade_no\"}";
        return "";
    }

    pay::alipay apay;
    apay.setAppId("你的支付宝APPID");
    apay.setPrivateKey("你的支付宝私钥");
    apay.setPublicKey("你的支付宝公钥");
    apay.setSandbox(true);

    std::string response = apay.cancelTrade(out_trade_no);
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,testalipayrefund)
std::string test_alipay_refund(std::shared_ptr<httppeer> peer)
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

    pay::alipay apay;
    apay.setAppId("你的支付宝APPID");
    apay.setPrivateKey("你的支付宝私钥");
    apay.setPublicKey("你的支付宝公钥");
    apay.setSandbox(true);

    std::string response = apay.refundTrade(refund_amount, "测试退款", out_trade_no);
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,alipaynotify)
std::string test_alipay_notify(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("Content-Type: text/plain; charset=utf-8");
#ifdef ENABLE_WEBPAY
    pay::alipay apay;
    apay.setAppId("你的支付宝APPID");
    apay.setPrivateKey("你的支付宝私钥");
    apay.setPublicKey("你的支付宝公钥");

    auto resp = apay.handleNotify(client.rawcontent);

    if (resp["status_code"] == "0") {
        std::string out_trade_no = resp["out_trade_no"];
        std::string trade_no = resp["trade_no"];
        std::string total_amount = resp["total_amount"];
        std::string trade_status = resp["trade_status"];
        client.output = "success";
    } else {
        client.output = "fail";
    }
#else
    client.output = "fail";
#endif
    return "";
}

//@urlpath(null,alipayreturn)
std::string test_alipay_return(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    pay::alipay apay;
    apay.setAppId("你的支付宝APPID");
    apay.setPrivateKey("你的支付宝私钥");
    apay.setPublicKey("你的支付宝公钥");

    std::string query_str = client.querystring;
    auto resp = apay.handleNotify(query_str);

    if (resp["status_code"] == "0") {
        client << "<h1>支付成功</h1>";
        client << "<p>订单号: " << resp["out_trade_no"] << "</p>";
        client << "<p>支付宝交易号: " << resp["trade_no"] << "</p>";
        client << "<p>金额: " << resp["total_amount"] << "</p>";
    } else {
        client << "<h1>支付失败</h1>";
        client << "<p>" << resp["error_msg"] << "</p>";
    }
#else
    client << "<h1>ENABLE_WEBPAY not enabled</h1>";
#endif
    return "";
}

}
