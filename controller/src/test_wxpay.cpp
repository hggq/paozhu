// 微信支付 V2（XML 报文 + MD5 签名）测试页，走 pay::weixinpay。
// 凭据来自 conf/webpay.conf 的 [wxpayv2]，同名 URL 参数可临时覆盖。
// V3（JSON + RSA Authorization 头）在 controller/src/test_wxpayv3.cpp。
#include <chrono>
#include <thread>
#include <map>
#include <string>
#include <vector>
#include "httppeer.h"
#include "test_wxpay.h"
#include "func.h"
#include "request.h"
#include "httpclient.h"
#include "orm.h"
#ifdef ENABLE_WEBPAY
#include "webpay_config.h"
#include "weixinxcx.h"
#endif
// 订单层不依赖支付 SDK（见 webpay_order.h 顶部注释），关闭 ENABLE_WEBPAY 也要能落库。
#include "webpay/webpay_order.h"
#include "webpay/webpay_qr.h"
#include "webpay/webpay_cert_gate.h"
#include "webpay/webpay_wxpayv2.h"

namespace http
{

static void twv2_head(httppeer &client, const std::string &title)
{
    client << "<!doctype html><html><head><meta charset=\"utf-8\"><title>";
    client << html_encode(title);
    client << "</title><style>"
              "body{font-family:-apple-system,BlinkMacSystemFont,\"PingFang SC\",sans-serif;margin:24px;color:#222;background:#fafafa}"
              "h2{font-size:18px;margin:20px 0 10px}"
              "table{border-collapse:collapse;background:#fff}td,th{border:1px solid #e3e3e3;padding:6px 10px;font-size:13px;text-align:left}"
              "th{background:#f2f4f7;color:#555;font-weight:600}"
              "pre{background:#fff;border:1px solid #e3e3e3;padding:10px;font-size:12px;white-space:pre-wrap;word-break:break-all;max-width:900px}"
              "a{color:#1677ff}.ok{color:#1a7f37;font-weight:600}.err{color:#cf222e;font-weight:600}.warn{color:#9a6700;font-weight:600}"
              "</style></head><body>";
}

static void twv2_foot(httppeer &client)
{
    client << "<p style=\"margin-top:24px\"><a href=\"/testwxpaynative\">v2 native 下单</a> | "
              "<a href=\"/testwxpayjsapi\">v2 jsapi 统一下单</a> | "
              "<a href=\"/testwxpayrefund\">v2 退款 / 退款查询</a> | "
              "<a href=\"/testwxpay\">v3 native 下单</a> | "
              "<a href=\"/testwxpaydownloadcert\">v3 下载平台证书</a></p></body></html>";
}

#ifdef ENABLE_WEBPAY

// 商户凭据直接用 vendor 的 webpay_merchant_t；外面再包一层只放两个调试上下文，
// 供 twv2_gate() 渲染"配置来源"—— webpay_merchant_t 里没有对应字段。
struct twv2_mch
{
    webpay_merchant_t mch;
    bool cfg_loaded = false;
    std::string cfg_file;
};

// 凭据只来自配置文件，不接受 URL 临时覆盖；商户由 kPayTag 钉死。
// kPayTag 留空 = 公共裸段 [wxpayv2]。要按业务隔离收款方：改成 "sitea" 这类 tag 并重新编译。
// 实际用了哪一段看 twv2_gate 的"配置来源"那一行（打的就是 mch.section）。
constexpr const char *kPayTag = "";

static twv2_mch twv2_merchant()
{
    webpay_config_t &cfg = get_webpay_config();

    twv2_mch m;
    m.mch        = cfg.wxpayv2(kPayTag);
    m.cfg_loaded = cfg.is_load();
    m.cfg_file   = cfg.file;
    return m;
}

// 打参数表；凭据没填好时返回 false，调用方必须就此打住，一条请求都不发
static bool twv2_gate(httppeer &client, const twv2_mch &m, bool need_cert)
{
    // 缺项判定在 libs（webpay_wxpayv2.h），本函数只管渲染与打住
    std::vector<std::string> lack = webpay::wxv2_config_gaps(m.mch, need_cert);

    client << "<h2>请求参数</h2><table>";
    client << "<tr><th>商户号 mch_id</th><td>"
           << (webpay_is_placeholder(m.mch.mch_id) ? "<span class=\"err\">未填</span>" : html_encode(m.mch.mch_id)) << "</td></tr>";
    client << "<tr><th>应用 appid</th><td>"
           << (webpay_is_placeholder(m.mch.appid) ? "<span class=\"err\">未填</span>" : html_encode(m.mch.appid)) << "</td></tr>";
    client << "<tr><th>API 密钥 apikey</th><td>";
    if (webpay_is_placeholder(m.mch.apikey))
    {
        client << "<span class=\"err\">未填</span>";
    }
    else
    {
        client << "<span class=\"ok\">已填，长度 " << m.mch.apikey.size() << " 位</span>（不回显内容）";
    }
    client << "</td></tr>";
    client << "<tr><th>回调 notifyurl</th><td>" << html_encode(m.mch.notifyurl) << "</td></tr>";
    if (need_cert)
    {
        client << "<tr><th>商户证书</th><td>" << html_encode(m.mch.cert_file)
               << (webpay::file_is_readable(m.mch.cert_file) ? " <span class=\"ok\">可读取</span>" : " <span class=\"err\">读取失败</span>") << "</td></tr>";
        client << "<tr><th>商户证书私钥</th><td>" << html_encode(m.mch.key_file)
               << (webpay::file_is_readable(m.mch.key_file) ? " <span class=\"ok\">可读取</span>" : " <span class=\"err\">读取失败</span>") << "</td></tr>";
    }
    client << "<tr><th>配置来源</th><td>";
    if (m.cfg_loaded)
    {
        client << html_encode(m.cfg_file) << " <span class=\"ok\">已加载</span>";
    }
    else
    {
        client << "<span class=\"err\">webpay.conf 未加载</span>";
    }
    // 本次实际取用的商户段（kPayTag 命中是 <tag>.wxpayv2，软回落时是裸段）
    client << "，商户段 ";
    if (m.mch.section.empty())
    {
        client << "<span class=\"err\">未取得（这一段在 conf 里不存在）</span>";
    }
    else
    {
        client << "<b>[" << html_encode(m.mch.section) << "]</b>";
    }
    client << "</td></tr>";
    client << "</table>";

    if (!lack.empty())
    {
        client << "<p class=\"err\">闸门：凭据未就绪，本次没有向微信发出任何请求。缺下列项：</p><ul>";
        for (const auto &k : lack)
        {
            client << "<li>" << html_encode(k) << "</li>";
        }
        client << "</ul>";
        client << "<p>请写进 " << html_encode(m.cfg_file.empty() ? std::string("conf/webpay.conf") : m.cfg_file)
               << " 的 [" << html_encode(m.mch.section.empty() ? std::string("wxpayv2") : m.mch.section)
               << "] 段（appid / mch_id / apikey / notifyurl），改完重启 http 服务器。"
                  "凭据只从配置文件来，URL 参数不再覆盖。换收款方的两条路：改这一段，"
                  "或改 controller/src/test_wxpay.cpp 里的 kPayTag 再重新编译。</p>";
        return false;
    }
    if (webpay_is_placeholder(m.mch.notifyurl))
    {
        client << "<p class=\"warn\">notifyurl 看着还是占位域名：下单不受影响，但微信回调投不到，通知判据本轮不判。</p>";
    }
    return true;
}

// v2 客户端没有对应接口时的统一说明：写清缺什么，不猜参数
static void twv2_not_implemented(httppeer &client, const std::string &what, const std::string &detail)
{
    client << "<h2>" << html_encode(what) << "</h2>";
    client << "<p class=\"err\">V2 客户端 <code>pay::weixinpay</code> 未实现该接口。</p>";
    client << "<p>" << detail << "</p>";
    client << "<p>V2 客户端未实现该接口。要测这个能力，"
              "得先给 <code>vendor/webpay/src/weixinxcx.cpp</code> 加出口，或改走 V3。</p>";
}

#endif// ENABLE_WEBPAY

// ===== 订单管理 / 退款管理（基于 cms.orderlist 模拟，V2，不要求登录，全量展示）=====
//
// 打标与查单口径不同，别当成漂移：
// - 写侧一律打 paytype=order_paytype::wxpayv2，列表页按它过滤（哪个渠道的页面列哪个渠道的单）；
// - 回调/退款按 wxorder 查单时**不带**这个过滤 —— 未打标老单 paytype=0，带上过滤就判成
//   "订单不存在"，回调永远落不了账。wxorder 有唯一索引 uk_wxorder，按单号只命中一行。
//
// 落库、查单、退款幂等键、退款闸门这些订单操作都在 libs/webpay/webpay_order.h
// （http::webpay::order_insert / order_find / order_ref_of / order_set_paid /
//   order_set_refunded / can_refund / refund_no_gen），本页只负责渲染。

static void twv2_order_rows(httppeer &client, const orm::cms::Orderlist &m)
{
    client << "<h2>订单列表（cms.orderlist，渠道 wxpayv2）</h2>";
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
               << "<td><a href=\"/testwxpayv2_order_detail?orderid=" << o.orderid << "\">详情</a> "
               << "<a href=\"/testwxpayv2_refund?orderid=" << o.orderid << "\">退款</a> "
               << "<a href=\"/testwxpayv2_refund_query?orderid=" << o.orderid << "\">退款查询</a></td></tr>";
    }
    client << "</table>";
}

static void twv2_order_list_render(httppeer &client)
{
    orm::cms::Orderlist m;
    m.where("paytype", order_paytype::wxpayv2).order("orderid", "desc").limit(100).fetch();
    twv2_order_rows(client, m);
}

static asio::awaitable<void> twv2_order_list_render_co(httppeer &client)
{
    orm::cms::Orderlist m;
    m.where("paytype", order_paytype::wxpayv2).order("orderid", "desc").limit(100);
    co_await m.async_fetch();
    twv2_order_rows(client, m);
}

// 落库这一趟没写进去 ⇒ 后面一步都不许做：库里没这一行，微信那一侧收到的钱就没有可对账的
// 单据（回调 order not found ⇒ 一直重投）。放在 #ifdef 外面：落库不依赖支付 SDK，
// 两条 order_new 路由关不开关 ENABLE_WEBPAY 都要能落。
static void twv2_store_failed(httppeer &client)
{
    client << "<p class=\"err\">闸门：订单没写进 cms.orderlist（save 没回自增 ID），"
              "本次没有向微信发出任何请求。请查 [cms] 段的库连接与 orderlist 表。</p>";
}

#ifdef ENABLE_WEBPAY
// 网关那一趟没成 ⇒ 把刚插进去的待付行标成已撤销，别留在列表页当孤儿单。
// 撤不动只说明这一行的 status 已经不是 0，文案里不去宣称"已撤"。
static void twv2_store_cancelled(httppeer &client, const std::string &wxorder)
{
    if (!webpay::order_cancel_by_wxorder(wxorder))
    {
        client << "<p class=\"warn\">撤单未生效（cms.orderlist 里这一行 status 已不是 0），需要手工清理。</p>";
    }
}

static asio::awaitable<void> twv2_store_cancelled_co(httppeer &client, const std::string &wxorder)
{
    if (!co_await webpay::async_order_cancel_by_wxorder(wxorder))
    {
        client << "<p class=\"warn\">撤单未生效（cms.orderlist 里这一行 status 已不是 0），需要手工清理。</p>";
    }
    co_return;
}
#endif// ENABLE_WEBPAY

//@urlpath(null,testwxpaynative)
std::string test_wxpay_native(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    twv2_mch m = twv2_merchant();

    // str2uint 滤非数字、不限长度、不抛；金额上限自己判，0/超上限回落 1 分
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    std::string total = std::to_string(want);
    std::string out_trade_no = "v2n" + std::to_string(timeid()) + rand_string(6, 0);

    twv2_head(client, "微信支付 V2 native 扫码下单");
    client << "<p>协议口径：XML 报文 + MD5 签名（<code>pay::weixinpay::createNative()</code>，trade_type 库里写死 NATIVE）。"
              "V3 的 JSON 接口在 <a href=\"/testwxpay\">/testwxpay</a>。</p>";
    if (!twv2_gate(client, m, false))
    {
        twv2_foot(client);
        return "";
    }
    client << "<table><tr><th>商户订单号</th><td>" << html_encode(out_trade_no)
           << "</td></tr><tr><th>金额(分)</th><td>" << html_encode(total) << "</td></tr></table>";

    // 落库排在下单之前：这一行没写进去就一步都不许往下走
    std::string storeorder_n = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    if (webpay::order_insert(order_paytype::wxpayv2, out_trade_no, storeorder_n, "", "V2测试商品",
                             (unsigned int)want) <= 0)
    {
        twv2_store_failed(client);
        twv2_foot(client);
        return "";
    }

    pay::weixinpay p;
    p.setAppid(m.mch.appid);
    p.setMchId(m.mch.mch_id);
    p.setApiKey(m.mch.apikey);
    p.setOutTradeNo(out_trade_no);
    p.setBody("V2测试商品");
    p.setTotalFee(total);
    p.setClientIp(client.client_ip);
    p.setNotifyUrl(m.mch.notifyurl);

    std::string code_url = p.createNative();

    client << "<h2>结果</h2>";
    if (code_url.empty() || code_url.rfind("ERROR:", 0) == 0)
    {
        client << "<p class=\"err\">未取得 code_url：" << html_encode(code_url) << "</p>";
        twv2_store_cancelled(client, out_trade_no);
        client << "<p>报文与签名都由 <code>weixinxcx.cpp</code> 生成：签名对不上会回 <code>SIGN_ERROR</code>，"
                  "appid 与 mch_id 未绑定会回 <code>RETURN_ERROR</code>。</p>";
        twv2_foot(client);
        return "";
    }

    client << "<p class=\"ok\">已下单（" << html_encode(total) << " 分），扫码链接：<code>"
           << html_encode(code_url) << "</code></p>";

    // 二维码走内联 SVG，不落盘（固定名的 PNG 会被并发下单互相覆盖 ⇒ 付到别人的单上）。
    // 三处二维码路由的写法统一：qr_svg 在没编 ENABLE_IMAGE 时回空串，走下面那条提示。
    std::string qrhtml = webpay::qr_svg(code_url);
    if (!qrhtml.empty())
    {
        client << "<div>" << qrhtml << "</div>";
    }
    else
    {
        client << "<p class=\"warn\">本二进制没编 ENABLE_IMAGE，画不出二维码。请复制上面的链接自行生成。</p>";
    }

    // ===== PNG 落盘通路：能力保留，默认关闭 =====
    // 启用 = 把 libs/webpay/webpay_qr.h 里的 k_qr_png_enabled 改成 true 重新编译；
    // 那头的注释写了启用后必须一起做的两件事（落账后删码、未支付单要自己清理）。
    if constexpr (webpay::k_qr_png_enabled)
    {
        if (webpay::qr_png_save(client.get_sitepath(), out_trade_no, code_url))
        {
            client << "<img src=\"/upload/qr_" << html_encode(out_trade_no) << ".png\">";
        }
        else
        {
            client << "<p class=\"err\">PNG 没能写进 "
                   << html_encode(webpay::qr_png_path(client.get_sitepath(), out_trade_no))
                   << " ⇒ 落盘失败（没编 ENABLE_IMAGE 或磁盘/权限问题），不是网关没给码</p>";
        }
    }
    client << "<p>out_trade_no：" << html_encode(out_trade_no) << "（不支付会在微信侧超时自动关闭）</p>";
    twv2_foot(client);
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
    twv2_mch m = twv2_merchant();

    // str2uint 滤非数字、不限长度、不抛；金额上限自己判，0/超上限回落 1 分
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    std::string total = std::to_string(want);
    std::string openid       = webpay::id_safe(client.get["openid"].to_string());
    std::string out_trade_no = "v2j" + std::to_string(timeid()) + rand_string(6, 0);

    twv2_head(client, "微信支付 V2 JSAPI 统一下单");
    client << "<p><code>pay::weixinpay::unifiedorder()</code>，trade_type 在库里写死 JSAPI，openid 必传："
              "<code>/testwxpayjsapi?openid=xxx</code>。openid 可用 <a href=\"/testgetopenid\">/testgetopenid</a> "
              "拿小程序 code 换。</p>";
    if (openid.empty())
    {
        client << "<p class=\"err\">缺少 openid，未向微信发出任何请求。</p>";
        twv2_foot(client);
        return "";
    }
    if (!twv2_gate(client, m, false))
    {
        twv2_foot(client);
        return "";
    }
    client << "<table><tr><th>商户订单号</th><td>" << html_encode(out_trade_no)
           << "</td></tr><tr><th>金额(分)</th><td>" << html_encode(total)
           << "</td></tr><tr><th>openid</th><td>" << html_encode(openid) << "</td></tr></table>";

    // 落库排在下单之前：这一行没写进去就一步都不许往下走
    std::string storeorder_j = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    if (webpay::order_insert(order_paytype::wxpayv2, out_trade_no, storeorder_j, openid, "V2测试商品",
                             (unsigned int)want) <= 0)
    {
        twv2_store_failed(client);
        twv2_foot(client);
        return "";
    }

    pay::weixinpay p;
    p.setAppid(m.mch.appid);
    p.setMchId(m.mch.mch_id);
    p.setApiKey(m.mch.apikey);
    p.setOpenid(openid);
    p.setOutTradeNo(out_trade_no);
    p.setBody("V2测试商品");
    p.setTotalFee(total);
    p.setClientIp(client.client_ip);
    p.setNotifyUrl(m.mch.notifyurl);

    std::string prepay_id = p.unifiedorder();

    client << "<h2>结果</h2>";
    if (prepay_id.empty() || prepay_id.rfind("ERROR:", 0) == 0)
    {
        client << "<p class=\"err\">未取得 prepay_id：" << html_encode(prepay_id) << "</p>";
        twv2_store_cancelled(client, out_trade_no);
        client << "<p>openid 与该 appid 不属于同一主体时会回 <code>OPENID_MISMATCH</code>。</p>";
    }
    else
    {
        client << "<p class=\"ok\">prepay_id：<code>" << html_encode(prepay_id) << "</code></p>";
        client << "<p>小程序调起参数（prepay_id + paySign）由 <code>getpay()</code> 生成，同一份 [wxpayv2] 配置的测试路由在 "
                  "<a href=\"/testweixinpay\">/testweixinpay</a>。</p>";
        client << "<p>out_trade_no：" << html_encode(out_trade_no) << "</p>";
    }
    twv2_foot(client);
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
    twv2_head(client, "微信支付 V2 APP 下单");
    twv2_not_implemented(client, "trade_type=APP 未实现",
                         "统一下单的 trade_type 在 <code>weixinxcx.cpp</code> 的 "
                         "<code>build_unifiedorder_params()</code> 里写死 JSAPI、"
                         "<code>build_createnative_params()</code> 里写死 NATIVE，没有 APP 分支。");
    twv2_foot(client);
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
    twv2_head(client, "微信支付 V2 H5 下单");
    twv2_not_implemented(client, "trade_type=MWEB 未实现",
                         "同上：V2 客户端没有 MWEB 分支。V3 侧 <code>pay::wxpay::createH5()</code> 有实现，"
                         "但本仓库还没有对应的测试路由。");
    twv2_foot(client);
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
    twv2_head(client, "微信支付 V2 查单");
    twv2_not_implemented(client, "查单（pay/orderquery）未实现",
                         "<code>weixinxcx.cpp</code> 只有 unifiedorder / createNative / getpay / refund / "
                         "refundquery / handle_notify 六个出口，没有 orderquery。"
                         "V3 侧 <code>pay::wxpay::queryTrade()</code> / <code>async_query_trade()</code> 有实现，"
                         "测试路由也有（订单详情页与协程查单页各一处）。");
    twv2_foot(client);
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
    twv2_head(client, "微信支付 V2 关单");
    twv2_not_implemented(client, "关单（pay/closeorder）未实现",
                         "V2 客户端没有 closeorder 出口。测试期间下的单不能从这里关，"
                         "要么走 <a href=\"/testwxpayrefund\">/testwxpayrefund</a> 退款冲掉，要么等它自然超时。");
    twv2_foot(client);
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,wxpaynotify)
asio::awaitable<std::string> test_wxpay_notify(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    // V2 要求应答也是 XML；回 JSON 微信会一直重试
    client.type("text/xml; charset=utf-8");
    twv2_mch m = twv2_merchant();
    if (webpay_is_placeholder(m.mch.apikey))
    {
        client.output = "<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[apikey not configured]]></return_msg></xml>";
        co_return "";
    }

    pay::weixinpay p;
    p.setAppid(m.mch.appid);
    p.setMchId(m.mch.mch_id);
    p.setApiKey(m.mch.apikey);
    // handle_notify 把该回的 XML 应答放在 error_msg 里，不论成功失败
    std::map<std::string, std::string> resp = p.handle_notify(client.rawcontent);

    // 只认"验签过 + 支付成功"这一条通路。status_code 非 0 时（缺参/非 SUCCESS/签名不符），
    // 报文里的业务字段是谁递的都说不清，一个都不采信，直接把 handle_notify 备好的 FAIL 回去。
    if (resp["status_code"] != "0" || resp["result_code"] != "SUCCESS")
    {
        client.output = resp["error_msg"];
        co_return "";
    }

    // 这里不判回调归属：V2 的 sign 用商户自己的 apikey 算，取错段与跨商户重投都过不了
    // 上面那道验签。哪些渠道承重写在 webpay_notify.h。

    // 查单 → 金额逐分比对 → 0→1 CAS 三步在 webpay::settle_notify()，四条通知路由共用同一份
    // 结算判定；这里只留 V2 的应答形状（FAIL 两支手写 XML，SUCCESS 用 SDK 备好的那份）。
    // settled 与 duplicate 都答 SUCCESS：重投那条不再写行，但必须让微信停手。
    auto st = co_await webpay::settle_notify(resp["out_trade_no"], str2uint(resp["total_fee"]));
    if (!st.ok())
    {
        client.output = std::string("<xml><return_code><![CDATA[FAIL]]></return_code><return_msg><![CDATA[")
                        + (st.code == webpay::settle_code::amount_mismatch ? "amount mismatch"
                                                                          : "order not found")
                        + "]]></return_msg></xml>";
        co_return "";
    }
    client.output = resp["error_msg"];
#else
    client.type("application/json; charset=utf-8");
    client.output = "{\"code\":\"FAIL\",\"message\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    co_return "";
}

//@urlpath(null,wxpaydownloadcert)
std::string test_wxpay_download_cert(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    twv2_head(client, "微信支付 V2 下载平台证书");
    twv2_not_implemented(client, "下载平台证书是 V3 接口",
                         "V2 的 MD5 签名只依赖商户 API 密钥，没有平台证书这一环。"
                         "V3 的证书下载在 <a href=\"/testwxpaydownloadcert\">/testwxpaydownloadcert</a>。");
    twv2_foot(client);
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

// ===== 协程镜像控制函数（V2，异步非阻塞，内部 co_await pay::weixinpay 的 async_ 方法）=====

//@urlpath(null,testwxpaynative_co)
asio::awaitable<std::string> test_wxpay_native_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    twv2_mch m = twv2_merchant();
    // str2uint 滤非数字、不限长度、不抛；金额上限自己判，0/超上限回落 1 分
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    std::string total = std::to_string(want);
    std::string out_trade_no = "v2n" + std::to_string(timeid()) + rand_string(6, 0);

    twv2_head(client, "微信支付 V2 native 扫码下单（协程）");
    client << "<p>协议口径：XML 报文 + MD5 签名（<code>pay::weixinpay::async_create_native()</code>，协程非阻塞）。"
              "同步版在 <a href=\"/testwxpaynative\">/testwxpaynative</a>。</p>";
    if (!twv2_gate(client, m, false))
    {
        twv2_foot(client);
        co_return "";
    }
    client << "<table><tr><th>商户订单号</th><td>" << html_encode(out_trade_no)
           << "</td></tr><tr><th>金额(分)</th><td>" << html_encode(total) << "</td></tr></table>";

    // 落库排在下单之前：这一行没写进去就一步都不许往下走（与同步孪生同形）
    std::string storeorder_n = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    if (co_await webpay::async_order_insert(order_paytype::wxpayv2, out_trade_no, storeorder_n, "", "V2测试商品",
                                            (unsigned int)want) <= 0)
    {
        twv2_store_failed(client);
        twv2_foot(client);
        co_return "";
    }

    pay::weixinpay p;
    p.setAppid(m.mch.appid);
    p.setMchId(m.mch.mch_id);
    p.setApiKey(m.mch.apikey);
    p.setOutTradeNo(out_trade_no);
    p.setBody("V2测试商品");
    p.setTotalFee(total);
    p.setClientIp(client.client_ip);
    p.setNotifyUrl(m.mch.notifyurl);

    std::string code_url = co_await p.async_create_native();

    client << "<h2>结果</h2>";
    if (code_url.empty() || code_url.rfind("ERROR:", 0) == 0)
    {
        client << "<p class=\"err\">未取得 code_url：" << html_encode(code_url) << "</p>";
        co_await twv2_store_cancelled_co(client, out_trade_no);
        client << "<p>报文与签名都由 <code>weixinxcx.cpp</code> 生成：签名对不上会回 <code>SIGN_ERROR</code>，"
                  "appid 与 mchid 未绑定会回 <code>RETURN_ERROR</code>。</p>";
        twv2_foot(client);
        co_return "";
    }

    client << "<p class=\"ok\">已下单（" << html_encode(total) << " 分），扫码链接：<code>"
           << html_encode(code_url) << "</code></p>";
    client << "<p>out_trade_no：" << html_encode(out_trade_no) << "（不支付会在微信侧超时自动关闭）</p>";
    twv2_foot(client);
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    co_return "";
}

//@urlpath(null,testwxpayjsapi_co)
asio::awaitable<std::string> test_wxpay_jsapi_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    twv2_mch m = twv2_merchant();
    // str2uint 滤非数字、不限长度、不抛；金额上限自己判，0/超上限回落 1 分
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    std::string total = std::to_string(want);
    std::string openid       = webpay::id_safe(client.get["openid"].to_string());
    std::string out_trade_no = "v2j" + std::to_string(timeid()) + rand_string(6, 0);

    twv2_head(client, "微信支付 V2 JSAPI 统一下单（协程）");
    client << "<p><code>pay::weixinpay::async_unifiedorder()</code>，trade_type 写死 JSAPI，openid 必传："
              "<code>/testwxpayjsapi_co?openid=xxx</code>。同步版在 <a href=\"/testwxpayjsapi\">/testwxpayjsapi</a>。</p>";
    if (openid.empty())
    {
        client << "<p class=\"err\">缺少 openid，未向微信发出任何请求。</p>";
        twv2_foot(client);
        co_return "";
    }
    if (!twv2_gate(client, m, false))
    {
        twv2_foot(client);
        co_return "";
    }
    client << "<table><tr><th>商户订单号</th><td>" << html_encode(out_trade_no)
           << "</td></tr><tr><th>金额(分)</th><td>" << html_encode(total)
           << "</td></tr><tr><th>openid</th><td>" << html_encode(openid) << "</td></tr></table>";

    // 落库排在下单之前：这一行没写进去就一步都不许往下走（与同步孪生同形）
    std::string storeorder_j = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    if (co_await webpay::async_order_insert(order_paytype::wxpayv2, out_trade_no, storeorder_j, openid, "V2测试商品",
                                            (unsigned int)want) <= 0)
    {
        twv2_store_failed(client);
        twv2_foot(client);
        co_return "";
    }

    pay::weixinpay p;
    p.setAppid(m.mch.appid);
    p.setMchId(m.mch.mch_id);
    p.setApiKey(m.mch.apikey);
    p.setOpenid(openid);
    p.setOutTradeNo(out_trade_no);
    p.setBody("V2测试商品");
    p.setTotalFee(total);
    p.setClientIp(client.client_ip);
    p.setNotifyUrl(m.mch.notifyurl);

    std::string prepay_id = co_await p.async_unifiedorder();

    client << "<h2>结果</h2>";
    if (prepay_id.empty() || prepay_id.rfind("ERROR:", 0) == 0)
    {
        client << "<p class=\"err\">未取得 prepay_id：" << html_encode(prepay_id) << "</p>";
        co_await twv2_store_cancelled_co(client, out_trade_no);
        client << "<p>openid 与该 appid 不属于同一主体时会回 <code>OPENID_MISMATCH</code>。</p>";
        twv2_foot(client);
        co_return "";
    }
    else
    {
        client << "<p class=\"ok\">prepay_id：<code>" << html_encode(prepay_id) << "</code></p>";
        client << "<p>小程序调起参数（prepay_id + paySign）由 <code>getpay()</code> 生成，同一份 [wxpayv2] 配置的测试路由在 "
                  "<a href=\"/testweixinpay\">/testweixinpay</a>。</p>";
        client << "<p>out_trade_no：" << html_encode(out_trade_no) << "</p>";
    }
    twv2_foot(client);
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif

    co_return "";
}

//@urlpath(null,testwxpayrefund)
std::string test_wxpay_refund(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    twv2_mch m = twv2_merchant();

    std::string out_trade_no  = webpay::id_safe(client.get["out_trade_no"].to_string());
    std::string out_refund_no = webpay::id_safe(client.get["out_refund_no"].to_string());
    // str2uint：滤非数字、不抛；0 ＝ 没传 refund_fee，走库里的 payprice 全额
    unsigned long long refund_fee = str2uint(client.get["refund_fee"].to_string());
    std::string reason        = client.get["reason"].to_string();
    bool isquery              = (client.get["act"].to_string() == "query");

    for (char &c : reason)
    {
        if (c == '<' || c == '>' || c == '&') c = ' ';
    }

    twv2_head(client, isquery ? "微信支付 V2 退款查询" : "微信支付 V2 申请退款");
    client << "<p>退款：<code>/testwxpayrefund?out_trade_no=xxx&amp;refund_fee=1</code>"
              "（需商户双向证书；原单金额取库里的 payprice，不收 URL 的 total_fee）<br />"
              "退款查询：<code>/testwxpayrefund?act=query&amp;out_trade_no=xxx</code>（不需证书，也可只给 out_refund_no）</p>";
    if (!isquery) client << "<p>" << html_encode(webpay::refund_once_notice()) << "</p>";

    if (out_trade_no.empty() && !(isquery && !out_refund_no.empty()))
    {
        client << "<p class=\"err\">缺少 out_trade_no（退款查询可以只给 out_refund_no），未向微信发出任何请求。</p>";
        twv2_foot(client);
        return "";
    }

    // 退款闸门（与 /testwxpayv2_refund 同口径）：回调把 status 推进到 1 之后才允许退，
    // 金额封顶按库里的 payprice。查询不退款，不判状态。
    webpay::order_ref o;
    if (!isquery)
    {
        if (!webpay::order_find(out_trade_no, o))
        {
            client << "<p class=\"err\">订单不存在（本站没有这个商户单号），未向微信发出任何请求。</p>";
            twv2_foot(client);
            return "";
        }
        unsigned long long fen = webpay::resolve_refund_fen(o, refund_fee);
        std::string why;
        if (!webpay::can_refund(o, fen, why))
        {
            client << "<p class=\"err\">" << html_encode(why) << "未向微信发出任何请求。</p>";
            twv2_foot(client);
            return "";
        }
        refund_fee = fen;
        // 幂等键缺省时由订单侧生成（R+商户单号-序号）；显式传了就用传的，重放同一笔才需要这么干
        if (out_refund_no.empty()) out_refund_no = webpay::refund_no_gen(o);
    }

    // 证书只在退款时必须；退款查询走普通 postxmlto
    if (!twv2_gate(client, m, !isquery))
    {
        twv2_foot(client);
        return "";
    }

    pay::weixinpay p;
    p.setAppid(m.mch.appid);
    p.setMchId(m.mch.mch_id);
    p.setApiKey(m.mch.apikey);
    p.setCertFile(m.mch.cert_file);
    p.setKeyFile(m.mch.key_file);
    p.setOutTradeNo(out_trade_no);
    p.setOutRefundNo(out_refund_no);
    if (!isquery) p.setTotalFee(std::to_string(o.payprice));
    if (refund_fee > 0) p.setRefundFee(std::to_string(refund_fee));
    if (!reason.empty()) p.setRefundDesc(reason);

    std::map<std::string, std::string> resp = isquery ? p.refundquery() : p.refund();

    client << "<h2>微信应答</h2><table><tr><th>字段</th><th>值</th></tr>";
    for (const auto &kv : resp)
    {
        client << "<tr><td>" << html_encode(kv.first) << "</td><td>" << html_encode(kv.second) << "</td></tr>";
    }
    client << "</table>";
    client << "<p>";
    if (resp["status_code"] == "0")
    {
        client << "<span class=\"ok\">" << (isquery ? "查询成功" : "退款受理成功") << "</span>";
        if (!isquery)
        {
            client << "，退款单号 <code>" << html_encode(out_refund_no) << "</code>";
            if (webpay::order_set_refunded(o))
            {
                client << "，已置 isrefund=1 / status=3，refundnum 自增 1。";
            }
            else
            {
                client << "，但本地落账没命中（status 已不是 1，或这一行已被别的路径改过），"
                          "<span class=\"warn\">请到订单列表核对 status/refundnum</span>。";
            }
        }
    }
    else
    {
        client << "<span class=\"err\">" << (isquery ? "查询失败" : "退款失败") << "</span>";
        if (!isquery)
        {
            client << "<p>证书不对时最常见是 <code>SG_ERROR_CERT_LOAD_FAILED</code> 一类连接层错误："
                      "[wxpayv2] 的 cert_file / key_file 要指向商户 API 证书那对 pem "
                      "（V2 退款走双向 TLS，与 V3 的商户 API 私钥不是同一个用途）。</p>";
        }
    }
    client << "</p>";
    twv2_foot(client);
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,testwxpayv2_order_new)
std::string testwxpayv2_order_new(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // str2uint 滤非数字、不限长度、不抛；金额上限自己判（std::stoul 遇超长串会抛 out_of_range）
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    unsigned int price = (unsigned int)want;
    std::string total = std::to_string(price);
    std::string desc = client.get["desc"].to_string();
    if (desc.empty()) desc = "V2测试商品";
    std::string openid = webpay::id_safe(client.get["openid"].to_string());
    // native / jsapi：微信的 trade_type（下单方式），不是 orderlist.paytype 那个渠道列
    std::string trade_type = client.get["type"].to_string();
    if (trade_type.empty()) trade_type = "native";

    std::string out_trade_no = "v2o" + std::to_string(timeid()) + rand_string(6, 0);
    std::string storeorder = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    long long oid = webpay::order_insert(order_paytype::wxpayv2, out_trade_no, storeorder, openid, desc, price);

    twv2_head(client, "微信支付 V2 新建订单（落库 cms.orderlist）");
    if (oid <= 0)
    {
        // "已落库"只能是"库里真有这一行"的同义词：save 没回自增 ID 就不许这么印，
        // 更不能接着去下单 —— 付成功也找不到账，回调会一直 FAIL。
        twv2_store_failed(client);
        twv2_order_list_render(client); twv2_foot(client); return "";
    }
    client << "<p>已生成订单并落库（测试用，不要求登录）。商户单号 <code>" << html_encode(out_trade_no)
           << "</code>，内部单号 <code>" << html_encode(storeorder) << "</code>，订单ID " << oid << "。</p>";

#ifdef ENABLE_WEBPAY
    twv2_mch m = twv2_merchant();
    if (trade_type == "jsapi" && openid.empty())
    {
        client << "<p class=\"err\">JSAPI 支付必须传 openid（?openid=xxx）。订单已落库。</p>";
        twv2_order_list_render(client); twv2_foot(client); return "";
    }
    if (!twv2_gate(client, m, false))
    {
        client << "<p class=\"warn\">订单已落库，但凭据未就绪，未向微信发起下单。</p>";
        twv2_order_list_render(client); twv2_foot(client); return "";
    }
    pay::weixinpay p;
    p.setAppid(m.mch.appid); p.setMchId(m.mch.mch_id); p.setApiKey(m.mch.apikey);
    p.setOutTradeNo(out_trade_no); p.setBody(desc); p.setTotalFee(total);
    p.setClientIp(client.client_ip); p.setNotifyUrl(m.mch.notifyurl);
    if (trade_type == "jsapi") p.setOpenid(openid);
    bool got_pay = false;
    if (trade_type == "jsapi")
    {
        std::string resp = p.getpay();
        client << "<h2>微信应答（getpay，JSAPI）</h2><pre>" << html_encode(resp) << "</pre>";
        got_pay = !resp.empty() && resp.rfind("ERROR:", 0) != 0;
    }
    else
    {
        std::string code_url = p.createNative();
        client << "<h2>结果（createNative）</h2>";
        if (code_url.empty() || code_url.rfind("ERROR:", 0) == 0)
            client << "<p class=\"err\">未取得 code_url：" << html_encode(code_url) << "</p>";
        else
        {
            client << "<p class=\"ok\">已下单，扫码链接：<code>" << html_encode(code_url) << "</code></p>";
            got_pay = true;
        }
    }
    // 网关没吐出付款参数 ⇒ 刚插进去的待付行没有下文，撤成已撤销，别留孤儿单
    if (!got_pay) twv2_store_cancelled(client, out_trade_no);
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，仅落库，未向微信下单。</p>";
#endif
    twv2_order_list_render(client);
    twv2_foot(client);
    return "";
}

//@urlpath(null,testwxpayv2_order_list)
std::string testwxpayv2_order_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    twv2_head(client, "微信支付 V2 订单列表");
    twv2_order_list_render(client);
    twv2_foot(client);
    return "";
}

//@urlpath(null,testwxpayv2_order_detail)
std::string testwxpayv2_order_detail(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    unsigned int rows = m.fetch_one();
    twv2_head(client, "微信支付 V2 订单详情");
    if (rows == 0) { client << "<p class=\"err\">订单不存在。</p>"; twv2_foot(client); return ""; }
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
    client << "<p><a href=\"/testwxpayv2_refund?orderid=" << o.orderid << "\">申请退款</a> | "
              "<a href=\"/testwxpayv2_refund_query?orderid=" << o.orderid << "\">退款查询</a></p>";
    twv2_foot(client);
    return "";
}

//@urlpath(null,testwxpayv2_refund)
std::string testwxpayv2_refund(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    m.fetch_one();
    webpay::order_ref o = webpay::order_ref_of(m);
    twv2_head(client, "微信支付 V2 申请退款");
    if (!o.found) { client << "<p class=\"err\">订单不存在。</p>"; twv2_foot(client); return ""; }
    // str2uint 滤非数字、不抛；0 ＝ 没传 refund，按订单金额全额退（口径在 resolve_refund_fen）
    unsigned long long fen =
        webpay::resolve_refund_fen(o, str2uint(client.get["refund"].to_string()));
    std::string reason = client.get["reason"].to_string();
    if (reason.empty()) reason = "测试退款";

    // 闸门：回调把 status 推进到 1 之后才允许退，金额落在 (0, payprice]。
    std::string why;
    if (!webpay::can_refund(o, fen, why))
    {
        client << "<p class=\"err\">" << html_encode(why) << "未向微信发出任何请求。</p>";
        twv2_foot(client);
        return "";
    }
    std::string refund = std::to_string(fen);
    std::string refund_no = webpay::refund_no_gen(o);

    client << "<p>订单 #" << o.oid << " 微信单号 <code>" << html_encode(o.wxorder)
           << "</code>，退款金额 " << refund << " 分（默认全额，需商户双向证书），退款单号 <code>"
           << html_encode(refund_no) << "</code>。</p>";
    client << "<p>" << html_encode(webpay::refund_once_notice()) << "</p>";

#ifdef ENABLE_WEBPAY
    twv2_mch mc = twv2_merchant();
    if (!twv2_gate(client, mc, true))
    {
        twv2_foot(client); return "";
    }
    pay::weixinpay p;
    p.setAppid(mc.mch.appid); p.setMchId(mc.mch.mch_id); p.setApiKey(mc.mch.apikey);
    p.setCertFile(mc.mch.cert_file); p.setKeyFile(mc.mch.key_file);
    p.setOutTradeNo(o.wxorder);
    p.setOutRefundNo(refund_no);
    p.setTotalFee(std::to_string(o.payprice));
    p.setRefundFee(refund);
    p.setRefundDesc(reason);
    std::map<std::string, std::string> resp = p.refund();
    client << "<h2>微信应答</h2><table><tr><th>字段</th><th>值</th></tr>";
    for (const auto &kv : resp) client << "<tr><td>" << html_encode(kv.first) << "</td><td>" << html_encode(kv.second) << "</td></tr>";
    client << "</table>";
    if (resp["status_code"] == "0")
    {
        if (webpay::order_set_refunded(o))
        {
            client << "<p class=\"ok\">退款受理成功，已更新 isrefund=1 / status=3，refundnum 自增 1。</p>";
        }
        else
        {
            client << "<p class=\"warn\">退款受理成功，但本地落账没命中（status 已不是 1），请核对这一行的 status/refundnum。</p>";
        }
    }
    else
    {
        client << "<p class=\"err\">退款失败。</p>";
    }
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，未向微信发起退款。</p>";
#endif
    twv2_foot(client);
    return "";
}

//@urlpath(null,testwxpayv2_refund_query)
std::string testwxpayv2_refund_query(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    unsigned int rows = m.fetch_one();
    twv2_head(client, "微信支付 V2 退款查询");
    if (rows == 0) { client << "<p class=\"err\">订单不存在。</p>"; twv2_foot(client); return ""; }
    auto &o = m.data;
    client << "<p>微信单号 <code>" << html_encode(o.wxorder) << "</code></p>";
#ifdef ENABLE_WEBPAY
    twv2_mch mc = twv2_merchant();
    if (!twv2_gate(client, mc, false)) { twv2_foot(client); return ""; }
    pay::weixinpay p;
    p.setAppid(mc.mch.appid); p.setMchId(mc.mch.mch_id); p.setApiKey(mc.mch.apikey);
    p.setOutTradeNo(o.wxorder);
    std::map<std::string, std::string> resp = p.refundquery();
    client << "<h2>微信退款单</h2><table><tr><th>字段</th><th>值</th></tr>";
    for (const auto &kv : resp) client << "<tr><td>" << html_encode(kv.first) << "</td><td>" << html_encode(kv.second) << "</td></tr>";
    client << "</table>";
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启。</p>";
#endif
    twv2_foot(client);
    return "";
}

//@urlpath(null,testwxpayv2_order_new_co)
asio::awaitable<std::string> testwxpayv2_order_new_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // str2uint 滤非数字、不限长度、不抛；金额上限自己判（std::stoul 遇超长串会抛 out_of_range）
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    std::string total = std::to_string(want);
    std::string desc = client.get["desc"].to_string();
    if (desc.empty()) desc = "V2测试商品";
    std::string openid = webpay::id_safe(client.get["openid"].to_string());
    // native / jsapi：微信的 trade_type（下单方式），不是 orderlist.paytype 那个渠道列
    std::string trade_type = client.get["type"].to_string();
    if (trade_type.empty()) trade_type = "native";

    std::string out_trade_no = "v2o" + std::to_string(timeid()) + rand_string(6, 0);
    std::string storeorder = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    long long oid = co_await webpay::async_order_insert(order_paytype::wxpayv2, out_trade_no, storeorder, openid, desc,
                                                          (unsigned int)want);

    twv2_head(client, "微信支付 V2 新建订单（协程，落库 cms.orderlist）");
    if (oid <= 0)
    {
        // "已落库"只能是"库里真有这一行"的同义词：save 没回自增 ID 就不许这么印。
        twv2_store_failed(client);
        co_await twv2_order_list_render_co(client); twv2_foot(client); co_return "";
    }
    client << "<p>已落库（协程非阻塞）。商户单号 <code>" << html_encode(out_trade_no)
           << "</code>，内部单号 <code>" << html_encode(storeorder) << "</code>，订单ID " << oid << "。</p>";

#ifdef ENABLE_WEBPAY
    twv2_mch m = twv2_merchant();
    if (trade_type == "jsapi" && openid.empty())
    {
        client << "<p class=\"err\">JSAPI 支付必须传 openid。订单已落库。</p>";
        co_await twv2_order_list_render_co(client); twv2_foot(client); co_return "";
    }
    if (!twv2_gate(client, m, false))
    {
        client << "<p class=\"warn\">订单已落库，但凭据未就绪。</p>";
        co_await twv2_order_list_render_co(client); twv2_foot(client); co_return "";
    }
    pay::weixinpay p;
    p.setAppid(m.mch.appid); p.setMchId(m.mch.mch_id); p.setApiKey(m.mch.apikey);
    p.setOutTradeNo(out_trade_no); p.setBody(desc); p.setTotalFee(total);
    p.setClientIp(client.client_ip); p.setNotifyUrl(m.mch.notifyurl);
    if (trade_type == "jsapi") p.setOpenid(openid);
    bool got_pay = false;
    if (trade_type == "jsapi")
    {
        std::string resp = co_await p.async_getpay();
        client << "<h2>微信应答（async_getpay，JSAPI）</h2><pre>" << html_encode(resp) << "</pre>";
        got_pay = !resp.empty() && resp.rfind("ERROR:", 0) != 0;
    }
    else
    {
        std::string code_url = co_await p.async_create_native();
        client << "<h2>结果（async_create_native）</h2>";
        if (code_url.empty() || code_url.rfind("ERROR:", 0) == 0)
            client << "<p class=\"err\">未取得 code_url：" << html_encode(code_url) << "</p>";
        else
        {
            client << "<p class=\"ok\">已下单，扫码链接：<code>" << html_encode(code_url) << "</code></p>";
            got_pay = true;
        }
    }
    // 网关没吐出付款参数 ⇒ 刚插进去的待付行没有下文，撤成已撤销，别留孤儿单
    if (!got_pay) co_await twv2_store_cancelled_co(client, out_trade_no);
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，仅落库。</p>";
#endif
    co_await twv2_order_list_render_co(client);
    twv2_foot(client);
    co_return "";
}

//@urlpath(null,testwxpayv2_refund_co)
asio::awaitable<std::string> testwxpayv2_refund_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    co_await m.async_fetch_one();
    webpay::order_ref o = webpay::order_ref_of(m);
    twv2_head(client, "微信支付 V2 申请退款（协程）");
    if (!o.found) { client << "<p class=\"err\">订单不存在。</p>"; twv2_foot(client); co_return ""; }
    // str2uint 滤非数字、不抛；0 ＝ 没传 refund，按订单金额全额退（口径在 resolve_refund_fen）
    unsigned long long fen =
        webpay::resolve_refund_fen(o, str2uint(client.get["refund"].to_string()));
    std::string reason = client.get["reason"].to_string();
    if (reason.empty()) reason = "测试退款";
    std::string why;
    if (!webpay::can_refund(o, fen, why))
    {
        client << "<p class=\"err\">" << html_encode(why) << "未向微信发出任何请求。</p>";
        twv2_foot(client); co_return "";
    }
    std::string refund = std::to_string(fen);
    std::string refund_no = webpay::refund_no_gen(o);
    client << "<p>订单 #" << o.oid << " 微信单号 <code>" << html_encode(o.wxorder)
           << "</code>，退款金额 " << refund << " 分（默认全额），退款单号 <code>"
           << html_encode(refund_no) << "</code>。</p>";
    client << "<p>" << html_encode(webpay::refund_once_notice()) << "</p>";
#ifdef ENABLE_WEBPAY
    twv2_mch mc = twv2_merchant();
    if (!twv2_gate(client, mc, true)) { twv2_foot(client); co_return ""; }
    pay::weixinpay p;
    p.setAppid(mc.mch.appid); p.setMchId(mc.mch.mch_id); p.setApiKey(mc.mch.apikey);
    p.setCertFile(mc.mch.cert_file); p.setKeyFile(mc.mch.key_file);
    p.setOutTradeNo(o.wxorder);
    p.setOutRefundNo(refund_no);
    p.setTotalFee(std::to_string(o.payprice));
    p.setRefundFee(refund);
    p.setRefundDesc(reason);
    std::map<std::string, std::string> resp = co_await p.async_refund();
    client << "<h2>微信应答（协程）</h2><table><tr><th>字段</th><th>值</th></tr>";
    for (const auto &kv : resp) client << "<tr><td>" << html_encode(kv.first) << "</td><td>" << html_encode(kv.second) << "</td></tr>";
    client << "</table>";
    if (resp["status_code"] == "0")
    {
        if (co_await webpay::async_order_set_refunded(o))
        {
            client << "<p class=\"ok\">退款受理成功，已更新 isrefund=1 / status=3，refundnum 自增 1。</p>";
        }
        else
        {
            client << "<p class=\"warn\">退款受理成功，但本地落账没命中（status 已不是 1），请核对这一行的 status/refundnum。</p>";
        }
    }
    else
    {
        client << "<p class=\"err\">退款失败。</p>";
    }
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，未向微信发起退款。</p>";
#endif
    twv2_foot(client);
    co_return "";
}

//@urlpath(null,testwxpayv2_refund_query_co)
asio::awaitable<std::string> testwxpayv2_refund_query_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    unsigned int rows = co_await m.async_fetch_one();
    twv2_head(client, "微信支付 V2 退款查询（协程）");
    if (rows == 0) { client << "<p class=\"err\">订单不存在。</p>"; twv2_foot(client); co_return ""; }
    auto &o = m.data;
    client << "<p>微信单号 <code>" << html_encode(o.wxorder) << "</code></p>";
#ifdef ENABLE_WEBPAY
    twv2_mch mc = twv2_merchant();
    if (!twv2_gate(client, mc, false)) { twv2_foot(client); co_return ""; }
    pay::weixinpay p;
    p.setAppid(mc.mch.appid); p.setMchId(mc.mch.mch_id); p.setApiKey(mc.mch.apikey);
    p.setOutTradeNo(o.wxorder);
    std::map<std::string, std::string> resp = co_await p.async_refundquery();
    client << "<h2>微信退款单（协程）</h2><table><tr><th>字段</th><th>值</th></tr>";
    for (const auto &kv : resp) client << "<tr><td>" << html_encode(kv.first) << "</td><td>" << html_encode(kv.second) << "</td></tr>";
    client << "</table>";
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启。</p>";
#endif
    twv2_foot(client);
    co_return "";
}

}// namespace http
