/*
使用方式
小程序js代码  app.js文件
wx.login({
        success(res) {
          if (res.code) {
            //发起网络请求
            wx.request({
              url: 'https://myweixin.xxx.com/testgetopenid',
              data: {
                code: res.code
              },
              success: (res) => {
                console.log(res)
                if (res.data.openid) {
                  wx.setStorageSync("openid", res.data.openid);
                }
            }});
        }
        }
});
*/
/*
    小程序下单
    var geturl="https://myweixin.xxx.com/testweixinpay";
    wx.request({
        //请求地址
        url: geturl,
        method: 'POST',
        data: {
            openid: openid
        },
        //请求头
        header:{
            'content-type': 'application/json' //默认值
        },
        //请求成功回调
        success(res){
            wx.requestPayment({
            'timeStamp': res.data.timeStamp,
            'nonceStr': res.data.nonceStr,
            'package': res.data.package,
            'signType': 'MD5',
            'paySign': res.data.paySign,
            'success':function(res){
                wx.navigateTo({
                    url: '../order/list'
                });
            }
            });
        },
        //请求失败回调
        fail(res){

        }
        });
*/
//小程序获取手机号
/*
wxml文件
<button type="warn" open-type="getPhoneNumber" bindgetphonenumber="getPhoneNumber">授权手机号</button>
js 文件
getPhoneNumber(e) {
    console.log('回调信息：', e.detail)
    var self = this;
    // 用户拒绝授权
    if (e.detail.errMsg && e.detail.errMsg.includes('fail')) {
        wx.showToast({ title: '已拒绝授权', icon: 'none' })
        return
    }
    // 获取加密数据及 code
    const { encryptedData, iv, code } = e.detail;
    wx.request({
        url: 'https://myweixin.xxx.com/xcxgetphone', // 替换为你的后端地址
        method: 'POST',
        data: {
        code: code,
        encryptedData: encryptedData,
        iv: iv,
        openid:openid
        },
        success(res) {}
    });
}
*/
#include <chrono>
#include <thread>
#include <vector>
#include <list>
#include <filesystem>
#include "httppeer.h"
#include "test_weixin.h"
#include "func.h"
#include "request.h"
#include "httpclient.h"
#ifdef ENABLE_WEBPAY
#include "weixinxcx.h"
#endif
#ifdef ENABLE_IMAGE
#include "qrcode.h"
#include "pzpng.h"
#endif

namespace http
{

//@urlpath(null,testgetopenid)
std::string test_getopenid(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    std::string response = "https://api.weixin.qq.com/sns/jscode2session?appid=";
    response.append("小程序appid");
    response.append("&secret=");
    response.append("小程序密钥");
    response.append("&js_code=");
    response.append(client.get["code"].to_string());
    response.append("&grant_type=authorization_code");

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get(response);
    a->timeout(10);
    a->send();

    if (a->get_status() == 200)
    {
        if (a->page.isjson == 1)
        {
            if (a->page.json["code"].to_int() == 0)
            {
                peer->output = "{\"openid:\"\"";
                peer->output.append(a->page.json["openid"].to_string());
                peer->output.append("\"}");
            }
        }
        else
        {
            peer->output = a->get_body();
        }
    }
    else
    {
        peer->output = "{\"code:\"\"";
        peer->output.append(client.get["code"].to_string());
        peer->output.append("\"}");
    }

    return "";
}

//@urlpath(null,testweixinpay)
std::string test_weixinpay(std::shared_ptr<httppeer> peer)
{
    httppeer &client         = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string openid       = client.get["openid"].to_string();
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::weixinpay wxpay;
    wxpay.setAppid("小程序id");
    wxpay.setMchId("商户id");
    wxpay.setApiKey("商户apiv2_key");
    wxpay.setOpenid(openid);
    wxpay.setOutTradeNo(out_trade_no);
    wxpay.setBody("测试购买");
    wxpay.setTotalFee("1");
    wxpay.setClientIp(client.client_ip);
    wxpay.setNotifyUrl("https://myweixin.xxx.com/xcxnotify");

    client.output = wxpay.getpay();
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

// 微信支付 V2 NATIVE 扫码支付（旧版财付通接口），下单后生成二维码图片，用户扫码支付
//@urlpath(null,testweixinnative)
std::string test_weixin_native(std::shared_ptr<httppeer> peer)
{
    httppeer &client         = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::weixinpay wxpay;
    wxpay.setAppid("小程序id");
    wxpay.setMchId("商户id");
    wxpay.setApiKey("商户apiv2_key");
    wxpay.setOutTradeNo(out_trade_no);
    wxpay.setBody("测试购买");
    wxpay.setTotalFee("1");// 单位：分
    wxpay.setClientIp(client.client_ip);
    wxpay.setNotifyUrl("https://myweixin.xxx.com/xcxnotify");

    std::string qrcontent = wxpay.createNative();

    if (qrcontent.find("ERROR:") == 0 || qrcontent.empty())
    {
        client.output = "{\"code\":\"-1\",\"msg\":\"" + qrcontent + "\"}";
        return "";
    }

#ifdef ENABLE_IMAGE
    namespace fs = std::filesystem;
    qr::qrcode q;
    q.text(qrcontent, qr::Ecc::M, 1);
    image::png img;
    unsigned char scale = 10;
    img.create(q.width() * scale + 40, q.height() * scale + 40, 6, 8);
    img.fillColor({255, 255, 255, 255});
    img.qrdata(q.data, q.width(), q.height(), scale, 20, 20);

    std::string wwwpath;
    wwwpath.append(client.get_sitepath());
    wwwpath.append("/upload");

    fs::path paths  = wwwpath;
    bool is_success = fs::create_directories(paths);
    if (is_success)
    {
        fs::permissions(paths,
                        fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                        fs::perm_options::add);
    }

    wwwpath.append("/weixin_native_qr.png");
    img.save(wwwpath);

    client << "<img src=\"/upload/weixin_native_qr.png?token=" << rand_string(6, 6) << "\">";
#else
    // 未开启图片模块时直接返回 code_url，前端自行生成二维码
    client.output = "{\"code_url\":\"" + qrcontent + "\",\"out_trade_no\":\"" + out_trade_no + "\"}";
#endif
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,xcxnotify)
std::string test_xcxnotify(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("Content-Type: application/json; charset=utf-8");
#ifdef ENABLE_WEBPAY
    pay::weixinpay wxpay;
    wxpay.setAppid("小程序id");
    wxpay.setMchId("商户id");
    wxpay.setApiKey("商户apiv2_key");
    auto resp_kv = wxpay.handle_notify(client.rawcontent);

    if (resp_kv["status_code"] == "0")
    {
    }
    client.output = resp_kv["error_msg"];
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//小程序获取手机号
/*
wxml文件
<button type="warn" open-type="getPhoneNumber" bindgetphonenumber="getPhoneNumber">授权手机号</button>
js 文件
getPhoneNumber(e) {
    console.log('回调信息：', e.detail)
    var self = this;
    // 用户拒绝授权
    if (e.detail.errMsg && e.detail.errMsg.includes('fail')) {
        wx.showToast({ title: '已拒绝授权', icon: 'none' })
        return
    }
    // 获取加密数据及 code
    const { encryptedData, iv, code } = e.detail;
    wx.request({
        url: 'https://myweixin.xxx.com/xcxgetphone', // 替换为你的后端地址
        method: 'POST',
        data: {
        code: code,
        encryptedData: encryptedData,
        iv: iv,
        openid:openid
        },
        success(res) {}
    });
}
*/

//@urlpath(null,xcxgetphone)
std::string test_xcxgetphone(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    std::string code   = client.json["code"].to_string();
    std::string openid = client.json["openid"].to_string();
    if (code.size() == 0)
    {
        code   = client.post["code"].to_string();
        openid = client.post["openid"].to_string();
    }

    if (openid.size() < 5)
    {
        client.val["error_msg"] = "微信openid获取失败";
        client.out_json();
        return "";
    }

    if (code.size() < 5)
    {
        client.val["error_msg"] = "微信code获取失败";
        client.out_json();
        return "";
    }

    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    /*
    从缓存中取 access_token 这样不用再次获取 参考

    pzcache<std::string> &temp_cache = pzcache<std::string>::conn();
    std::string namestring = "access_token";
    std::string access_token = temp_cache.get(namestring);

    */
    std::string response = "https://api.weixin.qq.com/cgi-bin/token?grant_type=client_credential&appid=";
    response.append("你的小程序appid");
    response.append("&secret=");
    response.append("你的小程序密钥");

    a->get(response);
    a->timeout(20);
    a->send();

    if (a->get_status() != 200)
    {
        client.val["error_msg"] = "access_token 请求失败！";
        client.out_json();
        return "";
    }

    /*
    缓存 access_token

    td::string cache_data = a->page.json["access_token"].to_string();
    temp_cache.save(access_token, cache_data, 30);

    */
    response = "https://api.weixin.qq.com/wxa/business/getuserphonenumber?access_token=";
    response.append(a->page.json["access_token"].to_string());
    http::obj_val postitem;
    postitem["code"] = code;
    a->clear();
    a->post_json(response, postitem);
    a->timeout(20);
    a->send();

    if (a->get_status() != 200)
    {
        client.val["error_msg"] = "code 获取手机号请求失败！";
        client.out_json();
        return "";
    }
    response.clear();
    response = a->page.json["phone_info"]["phoneNumber"].to_string();

    if (response.size() < 6)
    {
        client.val["error_msg"] = "获取手机号失败";
        client.out_json();
        return "";
    }

    client.val["phoneNumber"] = response;
    client.out_json();

    return "";
}

}// namespace http
