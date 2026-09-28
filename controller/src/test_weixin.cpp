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
#include "httppeer.h"
#include "test_weixin.h"
#include "func.h"
#include "request.h"
#include "httpclient.h"
#include "orm.h"
#ifdef ENABLE_WEBPAY
#include "webpay_config.h"
#include "weixinxcx.h"
#include "urlcode.h"// http::url_encode：请求值进查询串前要编码（与 wxpay.cpp / alipay.cpp 同源）
#endif
// 订单层不依赖支付 SDK（见 webpay_order.h 顶部注释），关闭 ENABLE_WEBPAY 也要能落库。
#include "webpay/webpay_order.h"
#include "webpay/webpay_qr.h"
// steady_seconds() 与 token 缓存共用一把单调时钟（声明在 webpay_cert_gate.h）。
#include "webpay/webpay_cert_gate.h"
#include "webpay/webpay_token.h"

namespace http
{
#ifdef ENABLE_WEBPAY
// 占位串检查与配置层同源：webpay_is_placeholder（vendor/httpserver/include/webpay_config.h）
static bool twx_has_placeholder(const std::string &v) { return http::webpay_is_placeholder(v); }

// 商户由代码钉死（kPayTag），请求参数不参与选商户。
// 留空 = 公共裸段 [wxpayv2]；换成 "sitea" 这类 tag 就取 [<tag>.wxpayv2]，
// 缺段或关键凭据缺项时配置层软回落回裸段，实际取用的段名在 merchant.section 里。
constexpr const char *kPayTag = "";

// 报错文案里点名"实际取用的那一段"：配了 tag 的部署如果还写死 [wxpayv2]，运维就去改了不生效的那一段
static std::string twx_mch_section(const webpay_merchant_t &m)
{
    return m.section.empty() ? std::string("wxpayv2") : m.section;
}

// 缺哪一项就把它写进 msg 并回 false：调用方必须一条请求都不发给微信。
// notifyurl 只查是否为空——统一下单少了它必报错，但占位域名不影响下单，只是回调投不到。
// need_cert 只有退款要：secapi 走双向 TLS，证书没配时不查就只剩一个连接层的
// SSL 报错，看不出是配置缺项。
static bool twx_need_mch(std::shared_ptr<httppeer> peer, webpay_merchant_t &m, bool need_notify, bool need_cert = false)
{
    httppeer &client = peer->get_peer();
    std::string lack;
    if (twx_has_placeholder(m.appid)) lack += "appid ";
    if (twx_has_placeholder(m.mch_id)) lack += "mch_id ";
    if (twx_has_placeholder(m.apikey) || m.apikey.size() != 32) lack += "apikey ";
    if (need_notify && m.notifyurl.empty()) lack += "notifyurl ";
    if (need_cert)
    {
        // 读得到才算有：目录、无权限的路径都判成缺项（与各支付页同一口径）。
        if (!webpay::file_is_readable(m.cert_file)) lack += "cert_file ";
        if (!webpay::file_is_readable(m.key_file)) lack += "key_file ";
    }
    if (!lack.empty())
    {
        client.val["code"] = -1;
        client.val["msg"]  = "conf/webpay.conf [" + twx_mch_section(m) + "] missing: " + lack;
        client.out_json();
        return false;
    }
    return true;
}

// 落库这一趟没写进去 ⇒ 后面一步都不许做：库里没这一行，微信那一侧收到的钱没有可对账的
// 单据（回调 order not found ⇒ 一直重投）。这几条路由对小程序都是 JSON 口径，闸门也回 JSON。
static void twxi_store_failed(httppeer &client)
{
    client.val["code"] = -1;
    client.val["msg"]  = "order not stored: cms.orderlist save returned no id";
    client.out_json();
}
#endif

//@urlpath(null,testgetopenid)
std::string test_getopenid(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    webpay_merchant_t mchcfg = get_webpay_config().wxpayv2(kPayTag);
    if (twx_has_placeholder(mchcfg.appid))
    {
        client.output = "{\"code\":\"-1\",\"msg\":\"conf/webpay.conf [" + twx_mch_section(mchcfg) + "] appid not configured\"}";
        return "";
    }
    if (twx_has_placeholder(mchcfg.secret))
    {
        client.output = "{\"code\":\"-1\",\"msg\":\"conf/webpay.conf [" + twx_mch_section(mchcfg) + "] secret not configured\"}";
        return "";
    }

    std::string response = "https://api.weixin.qq.com/sns/jscode2session?appid=";
    response.append(mchcfg.appid);
    response.append("&secret=");
    response.append(mchcfg.secret);
    response.append("&js_code=");
    // code 来自请求，编码之后再进查询串：裸拼的话 ?code=x%26appid%3Dyyy 解码出 `&appid=`，
    // 就能把这一趟导向别的 appid（而本机 secret 还在同一串里）。合法 js_code 只含字母数字
    // 与 -/_ ⇒ 编码对真流量是 no-op。
    const std::string jscode = client.get["code"].to_string();
    response.append(http::url_encode(jscode.data(), jscode.size()));
    response.append("&grant_type=authorization_code");

    std::shared_ptr<http::client> a = std::make_shared<http::client>();

    a->get(response);
    a->timeout(8);
    a->send();

    if (a->get_status() == 200)
    {
        if (a->page.isjson == 1)
        {
            std::string openid = a->page.json["openid"].to_string();
            if (!openid.empty())
            {
                // 成功：微信返回 openid / session_key，没有 code 字段
                peer->output = "{\"openid\":\"" + openid + "\"}";
            }
            else
            {
                // 失败：微信返回 errcode / errmsg，原样回给前端
                peer->output = "{\"code\":";
                peer->output.append(a->page.json["errcode"].to_string());
                peer->output.append(",\"msg\":\"");
                peer->output.append(a->page.json["errmsg"].to_string());
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
        peer->output = "{\"code\":-1,\"msg\":\"weixin http status ";
        peer->output.append(std::to_string(a->get_status()));
        peer->output.append("\"}");
    }
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,testweixinpay)
std::string test_weixinpay(std::shared_ptr<httppeer> peer)
{
    httppeer &client         = peer->get_peer();
#ifdef ENABLE_WEBPAY
    webpay_merchant_t mchcfg = get_webpay_config().wxpayv2(kPayTag);
    if (!twx_need_mch(peer, mchcfg, true)) return "";

    std::string openid       = client.get["openid"].to_string();
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 1);
    // str2uint 滤非数字、不限长度、不抛；0（没传/全非数字）回落 1 分
    unsigned long long want_fee = str2uint(client.get["total"].to_string());
    if (want_fee == 0) want_fee = 1;
    std::string total_fee = std::to_string(want_fee);

    pay::weixinpay wxpay;
    wxpay.setAppid(mchcfg.appid);
    wxpay.setMchId(mchcfg.mch_id);
    wxpay.setApiKey(mchcfg.apikey);
    wxpay.setOpenid(openid);
    wxpay.setOutTradeNo(out_trade_no);
    wxpay.setBody("测试购买");
    wxpay.setTotalFee(total_fee);
    wxpay.setClientIp(client.client_ip);
    wxpay.setNotifyUrl(mchcfg.notifyurl);

    // 落库排在下单之前：这一行没写进去就一步都不许往下走
    std::string storeorder_j = "SO" + std::to_string(timeid()) + rand_string(4, 1);
    if (webpay::order_insert(order_paytype::weixinxcx, out_trade_no, storeorder_j, openid,
                             "测试购买", (unsigned int)want_fee) <= 0)
    {
        twxi_store_failed(client);
        return "";
    }

    std::string payresp = wxpay.getpay();
    if (payresp.empty() || payresp.rfind("ERROR:", 0) == 0)
    {
        // 网关没吐出调起参数 ⇒ 把刚插进去的待付行撤掉，别留 status=0 的孤儿单。
        // order_cancelled 一起回：撤没撤动只有调用方看得见。
        client.val["code"]     = -1;
        client.val["msg"]      = payresp;
        client.val["order_cancelled"] = webpay::order_cancel_by_wxorder(out_trade_no);
        client.out_json();
        return "";
    }
    client.output = payresp;
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
    webpay_merchant_t mchcfg = get_webpay_config().wxpayv2(kPayTag);
    if (!twx_need_mch(peer, mchcfg, true)) return "";

    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 1);
    // str2uint 滤非数字、不限长度、不抛；0（没传/全非数字）回落 1 分
    unsigned long long want_fee = str2uint(client.get["total"].to_string());
    if (want_fee == 0) want_fee = 1;
    std::string total_fee = std::to_string(want_fee);

    pay::weixinpay wxpay;
    wxpay.setAppid(mchcfg.appid);
    wxpay.setMchId(mchcfg.mch_id);
    wxpay.setApiKey(mchcfg.apikey);
    wxpay.setOutTradeNo(out_trade_no);
    wxpay.setBody("测试购买");
    wxpay.setTotalFee(total_fee);// 单位：分
    wxpay.setClientIp(client.client_ip);
    wxpay.setNotifyUrl(mchcfg.notifyurl);

    // 落库排在下单之前：这一行没写进去就一步都不许往下走
    std::string storeorder_n = "SO" + std::to_string(timeid()) + rand_string(4, 1);
    if (webpay::order_insert(order_paytype::weixinxcx, out_trade_no, storeorder_n, "",
                             "测试购买", (unsigned int)want_fee) <= 0)
    {
        twxi_store_failed(client);
        return "";
    }

    std::string qrcontent = wxpay.createNative();

    if (qrcontent.find("ERROR:") == 0 || qrcontent.empty())
    {
        client.val["code"]        = -1;
        client.val["msg"]         = qrcontent;
        client.val["out_trade_no"] = out_trade_no;
        // 网关没吐出 code_url ⇒ 撤掉刚插进去的待付行；撤没撤动一起回，别只说"失败了"
        client.val["order_cancelled"] = webpay::order_cancel_by_wxorder(out_trade_no);
        client.out_json();
        return "";
    }

    // 二维码走内联 SVG，不落盘。输出口径统一："有图出图，没图一句话 + 可复制的原文"。
    std::string qrhtml = webpay::qr_svg(qrcontent);
    if (!qrhtml.empty())
    {
        client << "<div>" << qrhtml << "</div>";
    }
    else
    {
        client << "<p>本二进制没编 ENABLE_IMAGE，画不出二维码。扫码内容：<code>" << html_encode(qrcontent) << "</code></p>";
    }

    // ===== PNG 落盘通路：能力保留，默认关闭 =====
    // 启用 = 把 libs/webpay/webpay_qr.h 里的 k_qr_png_enabled 改成 true 重新编译。
    // 固定文件名会并发覆盖：后一人扫到前一人的码 ⇒ 钱付到别人的单上，文件名必须按单号走。
    if constexpr (webpay::k_qr_png_enabled)
    {
        if (webpay::qr_png_save(client.get_sitepath(), out_trade_no, qrcontent))
        {
            client << "<img src=\"/upload/qr_" << html_encode(out_trade_no) << ".png\">";
        }
        else
        {
            client << "<p>PNG 没能写进 " << html_encode(webpay::qr_png_path(client.get_sitepath(), out_trade_no))
                   << " ⇒ 落盘失败（没编 ENABLE_IMAGE 或磁盘/权限问题），不是网关没给码</p>";
        }
    }
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    return "";
}

//@urlpath(null,xcxnotify)
asio::awaitable<std::string> test_xcxnotify(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // V2 的应答必须是 XML，回 JSON 微信会一直重试
    client.type("text/xml; charset=utf-8");
#ifdef ENABLE_WEBPAY
    webpay_merchant_t mchcfg = get_webpay_config().wxpayv2(kPayTag);
    if (twx_has_placeholder(mchcfg.apikey))
    {
        client.output = "<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[apikey not configured]]></return_msg></xml>";
        co_return "";
    }

    pay::weixinpay wxpay;
    wxpay.setAppid(mchcfg.appid);
    wxpay.setMchId(mchcfg.mch_id);
    wxpay.setApiKey(mchcfg.apikey);
    // handle_notify 把该回的 XML 应答放在 error_msg 里，不论成功失败
    auto resp_kv = wxpay.handle_notify(client.rawcontent);

    // 只认验签通过的那一条通路：handle_notify 验签失败时只把 status_code 置 "3"，解析出来的业务
    // 字段照样原样返回，所以单判 result_code 的话，一段不带签名的 XML 就能把任意单号标成已付
    // —— 而那时 error_msg 回给微信的是 FAIL，对外说失败、对内记成交。
    if (resp_kv["status_code"] != "0" || resp_kv["result_code"] != "SUCCESS")
    {
        client.output = resp_kv["error_msg"];
        co_return "";
    }

    // 这里不判回调归属：V2 的 sign 用商户自己的 apikey 算，取错段与跨商户重投都过不了验签。
    // 各渠道通知侧的校验要求见 webpay_notify.h。

    // 落账三步（查单 → 金额逐分比对 → 0→1 CAS）与 /wxpaynotify、/wxpayv3notify、/alipaynotify
    // 共用 webpay::settle_notify()。按商户单号定位、不按渠道过滤：老单 paytype=0 会被过滤成
    // "找不到"，微信据此无限重投。
    // settled 与 duplicate 都答 SUCCESS（那份是 SDK 备好的 error_msg），FAIL 两支手写 XML。
    auto st = co_await webpay::settle_notify(resp_kv["out_trade_no"], str2uint(resp_kv["total_fee"]));
    if (!st.ok())
    {
        client.output = std::string("<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[")
                        + (st.code == webpay::settle_code::amount_mismatch ? "amount mismatch"
                                                                          : "order not found")
                        + "]]></return_msg></xml>";
        co_return "";
    }
    client.output = resp_kv["error_msg"];
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    co_return "";
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

#ifdef ENABLE_WEBPAY
    webpay_merchant_t mchcfg = get_webpay_config().wxpayv2(kPayTag);
    if (twx_has_placeholder(mchcfg.appid))
    {
        client.val["error_msg"] = "conf/webpay.conf [" + twx_mch_section(mchcfg) + "] appid not configured";
        client.out_json();
        return "";
    }
    if (twx_has_placeholder(mchcfg.secret))
    {
        client.val["error_msg"] = "conf/webpay.conf [" + twx_mch_section(mchcfg) + "] secret not configured";
        client.out_json();
        return "";
    }

    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    unsigned long long tok_now = webpay::steady_seconds();
    std::string access_token;
    if (webpay::token_cache_get(mchcfg.appid, access_token, tok_now))
    {
        // 命中就不问微信要 token 了。这条路由的对外通道只有应答体（框架没有日志 API），
        // 所以判配额时看这个字段：cache 越多说明缓存真的在挡回源。
        client.val["token_from"] = "cache";
    }
    else
    {
        client.val["token_from"] = "fetch";
        std::string tokenurl = "https://api.weixin.qq.com/cgi-bin/token?grant_type=client_credential&appid=";
        tokenurl.append(mchcfg.appid);
        tokenurl.append("&secret=");
        tokenurl.append(mchcfg.secret);

        a->get(tokenurl);
        a->timeout(8);
        a->send();

        if (a->get_status() != 200)
        {
            client.val["error_msg"] = "access_token 请求失败！";
            client.out_json();
            return "";
        }
        // 换取失败微信也回 HTTP 200，errcode 在正文里、access_token 是空。空串既不能拿去
        // 要手机号，也不能进缓存（token_cache_put 自己也挡一道，两处都得挡）。
        access_token = a->page.json["access_token"].to_string();
        if (access_token.empty())
        {
            client.val["error_msg"] = "access_token 换取失败！errcode=" + a->page.json["errcode"].to_string() +
                                      " errmsg=" + a->page.json["errmsg"].to_string();
            client.out_json();
            return "";
        }
        webpay::token_cache_put(mchcfg.appid, access_token, tok_now);
    }

    std::string response = "https://api.weixin.qq.com/wxa/business/getuserphonenumber?access_token=";
    response.append(access_token);
    http::obj_val postitem;
    postitem["code"] = code;
    a->clear();
    a->post_json(response, postitem);
    a->timeout(8);
    a->send();

    if (a->get_status() != 200)
    {
        client.val["error_msg"] = "code 获取手机号请求失败！";
        client.out_json();
        return "";
    }

    // 缓存这一格可能被微信单方面作废（改过 appsecret、或微信提前收回）。认出来就把这一格删掉，
    // 下一笔自己回源 —— 这一笔仍然失败，但不删的话后面 7000 秒每一笔都失败。
    // 不在这一笔里重试第二趟：40001 最常见的成因是 appid/appsecret 配错，那一趟也是白趟。
    std::string wxerr = a->page.json["errcode"].to_string();
    if (webpay::token_rejected_errcode(wxerr))
    {
        webpay::token_cache_invalidate(mchcfg.appid);
        client.val["error_msg"] = "access_token 已失效，请重试！errcode=" + wxerr;
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
#else
    client.val["error_msg"] = "ENABLE_WEBPAY not enabled";
    client.out_json();
#endif

    return "";
}

// ===== 订单管理 / 退款管理（基于 cms.orderlist 模拟，小程序 V2，不要求登录，全量展示）=====
//
// 落库/落账/退款都走 libs/webpay/webpay_order.h（http::webpay::*），这里不另写一份。

static void twxi_order_list_render(httppeer &client)
{
    orm::cms::Orderlist m;
    // 按渠道过滤：orderlist 是四渠道共表，不过滤的话小程序这页会列出支付宝/V2/V3 的单。
    // 回调按 wxorder 查单时**不**带这个过滤——打标之前的老单 paytype=0（未标记），
    // 带上就判成"订单不存在"，回调永远落不了账、微信一直重发。
    m.where("paytype", order_paytype::weixinxcx).order("orderid", "desc").limit(100).fetch();
    client << "<h2>订单列表（cms.orderlist，渠道 weixinxcx）</h2>";
    if (m.record.empty()) { client << "<p>暂无订单。</p>"; return; }
    client << "<table><tr><th>orderid</th><th>内部单号</th><th>微信单号</th><th>openid</th>"
              "<th>金额(分)</th><th>退款</th><th>状态</th><th>下单时间</th><th>操作</th></tr>";
    for (auto &o : m.record)
    {
        client << "<tr><td>" << o.orderid << "</td>"
               << "<td>" << html_encode(o.storeorder) << "</td>"
               << "<td>" << html_encode(o.wxorder) << "</td>"
               << "<td>" << html_encode(o.openid) << "</td>"
               << "<td>" << o.payprice << "</td>"
               << "<td>" << (o.isrefund ? (std::string("是(") + std::to_string((int)o.refundnum) + ")") : std::string("否")) << "</td>"
               << "<td>" << (int)o.status << "</td>"
               << "<td>" << o.addtime << "</td>"
               << "<td><a href=\"/testweixin_order_detail?orderid=" << o.orderid << "\">详情</a> "
               << "<a href=\"/testweixin_refund?orderid=" << o.orderid << "\">退款</a> "
               << "<a href=\"/testweixin_refund_query?orderid=" << o.orderid << "\">退款查询</a></td></tr>";
    }
    client << "</table>";
}

//@urlpath(null,testweixin_order_new)
std::string test_weixin_order_new(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    webpay_merchant_t mchcfg = get_webpay_config().wxpayv2(kPayTag);
    if (!twx_need_mch(peer, mchcfg, true)) return "";

    std::string openid = client.get["openid"].to_string();
    // str2uint 滤非数字、不限长度、不抛；上限自己判（std::stoul 遇超长串抛 out_of_range）
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    unsigned int price = (unsigned int)want;
    std::string total  = std::to_string(price);
    std::string desc = client.get["desc"].to_string();
    if (desc.empty()) desc = "小程序测试商品";
    // native / jsapi：微信的 trade_type（下单方式），不是 orderlist.paytype 那个渠道列
    std::string trade_type = client.get["type"].to_string();
    if (trade_type.empty()) trade_type = "jsapi";

    std::string out_trade_no = "xcxo" + std::to_string(timeid()) + rand_string(4, 1);
    std::string storeorder = "SO" + std::to_string(timeid()) + rand_string(6, 1);
    long long oid = webpay::order_insert(order_paytype::weixinxcx, out_trade_no, storeorder,
                                         openid, desc, price);

    if (oid <= 0)
    {
        // "已落库"只能是"库里真有这一行"的同义词：save 没回自增 ID 就不许这么印，
        // 更不能接着去下单 —— 付成功也找不到账，回调会一直 FAIL。
        client << "<p class=\"err\">闸门：订单没写进 cms.orderlist（save 没回自增 ID），"
                  "本次没有向微信发出任何请求。请查 [cms] 段的库连接与 orderlist 表。</p>";
        return "";
    }
    client << "<p>已生成订单并落库（测试用，不要求登录）。商户单号 <code>" << html_encode(out_trade_no)
           << "</code>，内部单号 <code>" << html_encode(storeorder) << "</code>，订单ID " << oid << "。</p>";

    if (trade_type == "jsapi" && openid.empty())
    {
        client << "<p class=\"err\">小程序支付必须传 openid（?openid=xxx）。订单已落库。</p>";
        twxi_order_list_render(client); return "";
    }

    pay::weixinpay wxpay;
    wxpay.setAppid(mchcfg.appid);
    wxpay.setMchId(mchcfg.mch_id);
    wxpay.setApiKey(mchcfg.apikey);
    wxpay.setOutTradeNo(out_trade_no);
    wxpay.setBody(desc);
    wxpay.setTotalFee(total);
    wxpay.setClientIp(client.client_ip);
    wxpay.setNotifyUrl(mchcfg.notifyurl);
    if (trade_type == "jsapi") wxpay.setOpenid(openid);

    if (trade_type == "jsapi")
    {
        std::string payresp = wxpay.getpay();
        if (payresp.empty() || payresp.rfind("ERROR:", 0) == 0)
        {
            // 网关没吐出调起参数 ⇒ 撤掉刚插进去的待付行，别留 status=0 的孤儿单
            client.val["code"]     = -1;
            client.val["msg"]      = payresp;
            client.val["order_cancelled"] = webpay::order_cancel_by_wxorder(out_trade_no);
            client.out_json();
            return "";
        }
        client.output = payresp;
    }
    else
    {
        std::string qrcontent = wxpay.createNative();
        if (qrcontent.empty() || qrcontent.rfind("ERROR:", 0) == 0)
        {
            client << "<p class=\"err\">未取得 code_url：" << html_encode(qrcontent) << "</p>";
            if (!webpay::order_cancel_by_wxorder(out_trade_no))
                client << "<p class=\"warn\">撤单未生效（cms.orderlist 里这一行 status 已不是 0），需要手工清理。</p>";
        }
        else
            client << "<p class=\"ok\">已下单，扫码链接：<code>" << html_encode(qrcontent) << "</code></p>";
    }
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,testweixin_order_list)
std::string test_weixin_order_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    twxi_order_list_render(client);
    return "";
}

//@urlpath(null,testweixin_order_detail)
std::string test_weixin_order_detail(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    if (m.fetch_one() == 0) { client << "<p class=\"err\">订单不存在。</p>"; return ""; }
    auto &o = m.data;
    client << "<h2>订单 #" << o.orderid << "</h2><table>";
    client << "<tr><th>内部单号</th><td>" << html_encode(o.storeorder) << "</td></tr>";
    client << "<tr><th>微信单号</th><td>" << html_encode(o.wxorder) << "</td></tr>";
    client << "<tr><th>openid</th><td>" << html_encode(o.openid) << "</td></tr>";
    client << "<tr><th>商品</th><td>" << html_encode(o.paytitle) << "</td></tr>";
    client << "<tr><th>金额(分)</th><td>" << o.payprice << "</td></tr>";
    client << "<tr><th>退款</th><td>" << (o.isrefund ? (std::string("是(") + std::to_string((int)o.refundnum) + ")") : std::string("否")) << "</td></tr>";
    client << "<tr><th>状态</th><td>" << (int)o.status << "</td></tr>";
    client << "<tr><th>下单时间</th><td>" << o.addtime << "</td></tr>";
    client << "</table>";
    client << "<p><a href=\"/testweixin_refund?orderid=" << o.orderid << "\">申请退款</a> | "
              "<a href=\"/testweixin_refund_query?orderid=" << o.orderid << "\">退款查询</a></p>";
    return "";
}

//@urlpath(null,testweixin_refund)
std::string test_weixin_refund(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    if (m.fetch_one() == 0) { client << "<p class=\"err\">订单不存在。</p>"; return ""; }
    webpay::order_ref o = webpay::order_ref_of(m);
    // 0 ＝ 没传 refund，按订单金额全额退（口径在 resolve_refund_fen）
    unsigned long long fen =
        webpay::resolve_refund_fen(o, str2uint(client.get["refund"].to_string()));
    std::string reason = client.get["reason"].to_string();
    if (reason.empty()) reason = "测试退款";

    // 退款闸门：回调把 status 推进到 1（已付）之后才允许退；金额必须落在 (0, payprice]。
    std::string why;
    if (!webpay::can_refund(o, fen, why))
    {
        client << "<p class=\"err\">" << why << "未向微信发出任何请求。</p>";
        return "";
    }
    std::string refund = std::to_string(fen);
    // 幂等键由订单侧生成：SDK 不再兜底造号，out_refund_no 为空时 refund() 直接失败、不外发
    std::string refund_no = webpay::refund_no_gen(o);

    client << "<p>订单 #" << o.oid << " 微信单号 <code>" << html_encode(o.wxorder)
           << "</code>，退款金额 " << refund << " 分（默认全额，需商户双向证书），退款单号 <code>"
           << html_encode(refund_no) << "</code>。</p>";
    client << "<p>" << html_encode(webpay::refund_once_notice()) << "</p>";

#ifdef ENABLE_WEBPAY
    webpay_merchant_t mchcfg = get_webpay_config().wxpayv2(kPayTag);
    if (!twx_need_mch(peer, mchcfg, true, true)) return "";
    pay::weixinpay wxpay;
    wxpay.setAppid(mchcfg.appid);
    wxpay.setMchId(mchcfg.mch_id);
    wxpay.setApiKey(mchcfg.apikey);
    wxpay.setCertFile(mchcfg.cert_file);
    wxpay.setKeyFile(mchcfg.key_file);
    wxpay.setOutTradeNo(o.wxorder);
    wxpay.setOutRefundNo(refund_no);
    wxpay.setTotalFee(std::to_string(o.payprice));
    wxpay.setRefundFee(refund);
    wxpay.setRefundDesc(reason);
    std::map<std::string, std::string> resp = wxpay.refund();
    client << "<h2>微信应答</h2><table><tr><th>字段</th><th>值</th></tr>";
    for (const auto &kv : resp) client << "<tr><td>" << html_encode(kv.first) << "</td><td>" << html_encode(kv.second) << "</td></tr>";
    client << "</table>";
    if (resp["status_code"] == "0")
    {
        // 退款落账是一条 CAS（WHERE wxorder AND status=1）：isrefund=1 / status=3 / refundnum 自增。
        if (webpay::order_set_refunded(o))
            client << "<p class=\"ok\">退款受理成功，已更新 isrefund=1 / status=3，refundnum 自增 1。</p>";
        else
            client << "<p class=\"warn\">退款受理成功，但订单状态已不是 1（已付），未改写落库 —— 请核对是否已被另一笔回调推进。</p>";
    }
    else
    {
        client << "<p class=\"err\">退款失败。</p>";
    }
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，未向微信发起退款。</p>";
#endif
    return "";
}

//@urlpath(null,testweixin_refund_query)
std::string test_weixin_refund_query(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    if (m.fetch_one() == 0) { client << "<p class=\"err\">订单不存在。</p>"; return ""; }
    auto &o = m.data;
    client << "<p>微信单号 <code>" << html_encode(o.wxorder) << "</code></p>";
#ifdef ENABLE_WEBPAY
    webpay_merchant_t mchcfg = get_webpay_config().wxpayv2(kPayTag);
    if (!twx_need_mch(peer, mchcfg, false)) return "";
    pay::weixinpay wxpay;
    wxpay.setAppid(mchcfg.appid);
    wxpay.setMchId(mchcfg.mch_id);
    wxpay.setApiKey(mchcfg.apikey);
    wxpay.setOutTradeNo(o.wxorder);
    std::map<std::string, std::string> resp = wxpay.refundquery();
    client << "<h2>微信退款单</h2><table><tr><th>字段</th><th>值</th></tr>";
    for (const auto &kv : resp) client << "<tr><td>" << html_encode(kv.first) << "</td><td>" << html_encode(kv.second) << "</td></tr>";
    client << "</table>";
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启。</p>";
#endif
    return "";
}

}// namespace http
