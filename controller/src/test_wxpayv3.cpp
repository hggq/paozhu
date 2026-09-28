#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "httppeer.h"
#include "func.h"
#include "datetime.h"
#include "httpclient.h"
#include "orm.h"
#ifdef ENABLE_WEBPAY
#include "wxpay.h"
#endif
#ifdef ENABLE_WEBPAY
#include "webpay_config.h"
#endif
#include "func.h"
#include "webpay/webpay_order.h"
#include "webpay/webpay_cert_gate.h"
#include "webpay/webpay_qr.h"
#include "webpay/webpay_wxpayv3.h"
#include "test_wxpayv3.h"

namespace http
{
static void twx_page_head(httppeer &client, const std::string &title)
{
    client << "<!doctype html><html><head><meta charset=\"utf-8\"><title>";
    client << title;
    client << "</title><style>"
              "body{font-family:-apple-system,BlinkMacSystemFont,\"PingFang SC\",sans-serif;margin:24px;color:#222;background:#fafafa}"
              "h2{font-size:18px;margin:20px 0 10px}"
              "table{border-collapse:collapse;background:#fff}td,th{border:1px solid #e3e3e3;padding:6px 10px;font-size:13px;text-align:left}"
              "th{background:#f2f4f7;color:#555;font-weight:600}"
              "pre{background:#fff;border:1px solid #e3e3e3;padding:10px;font-size:12px;white-space:pre-wrap;word-break:break-all;max-width:900px}"
              "textarea{width:900px;height:300px;font-size:12px}"
              "a{color:#1677ff}.ok{color:#1a7f37;font-weight:600}.err{color:#cf222e;font-weight:600}"
              "</style></head><body>";
}

static void twx_page_foot(httppeer &client)
{
    client << "<p style=\"margin-top:24px\"><a href=\"/testwxpay\">native 下单</a> | "
              "<a href=\"/testwxpayv3jsapi\">小程序V3支付</a> | "
              "<a href=\"/testwxpaydownloadcert\">下载平台证书</a></p></body></html>";
}

// 证书下载页访问闸门：每次访问都会真的向微信发请求并把 platform_cert_file 写回 conf。
// 放行条件是"或"，两条都不满足才拒：
//   1) 同一域名下已经登录后台（admin userid / superadmin superid 任一非 0）；
//   2) 来源 IP 是本机或内网单播（ip_is_local()）。
// 写成"或"而不是只留一条：只看登录会把服务器本机 curl 挡掉，只看 IP 会把公网浏览器进来的
// 运维挡掉（那种情况先登录后台再回来）。
// client_ip 取套接字对端地址，不是 X-Forwarded-For，伪造头换不来本机身份。
static bool twx_cert_guard_ok(httppeer &client)
{
    if (client.session["userid"].to_int() != 0) return true;
    if (client.session["superid"].to_int() != 0) return true;
    return ip_is_local(client.client_ip);
}

static void twx_cert_guard_deny(httppeer &client)
{
    client.status(403);
    twx_page_head(client, "403 平台证书下载页需要本机或后台登录");
    client << "<h2 class=\"err\">403：本页只允许本机/内网来源，或已登录后台的会话</h2>";
    client << "<p>这一页会向微信发出真实请求，并把平台证书路径写回 <code>conf/webpay.conf</code>，不对外开放。</p>";
    client << "<p>可以走的两条路：① 在同一域名下先登录后台（<a href=\"/admin/login\">/admin/login</a> 或 "
              "<a href=\"/superadmin/login\">/superadmin/login</a>），再回到本页；② 在服务器本机访问 "
              "<code>http://127.0.0.1/testwxpaydownloadcert</code>（公网机器上先起 "
              "<code>ssh -L 8888:127.0.0.1:80 user@server</code>，浏览器打开 <code>http://127.0.0.1:8888/</code>）。</p>";
    client << "<p>本次来源 IP：" << html_encode(client.client_ip) << "</p>";
    twx_page_foot(client);
}

// 前置过滤器要和路由函数一样带 @urlpath：controller/include 里的声明只给带注解的函数生成，
// 少了注解 common/autocontrolmethod.hpp 就引用到一个未声明的名字（admin_islogin /
// superadmin_islogin 同样是"既是过滤器又占一条路由"）。
// 协程路由的 pre 类型是 asio::awaitable<std::string>（见 vendor/httpserver/include/httppeer.h），
// 所以下面这条不能复用上面那条，得单独写一版 —— 两条的放行条件完全一致。
//@urlpath(null,testwxpaycertislogin)
std::string testwxpaycert_islogin(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (twx_cert_guard_ok(client)) return "ok";
    twx_cert_guard_deny(client);
    return "";
}

//@urlpath(null,testwxpaycertislogin_co)
asio::awaitable<std::string> testwxpaycert_islogin_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (twx_cert_guard_ok(client)) co_return "ok";
    twx_cert_guard_deny(client);
    co_return "";
}

#ifdef ENABLE_WEBPAY
// 脱敏显示密钥：未配置显示提示，已配置显示 前4****后4
static std::string twx_mask_key(const std::string &v)
{
    if (v.empty()) return "<span class=\"err\">未配置</span>";
    if (v.size() <= 8) return html_encode(v);
    return html_encode(v.substr(0, 4)) + "****" + html_encode(v.substr(v.size() - 4));
}

// 商户由代码钉死（kPayTag），请求参数不参与选商户。要按业务隔离收款方，把 kPayTag 改成
// "sitea" 这类 tag 并重新编译，走 [<tag>.wxpayv3_web]；缺段或关键凭据缺项时配置层软回落回裸段，
// 实际用了哪一段由返回值的 section 字段标明（页面/报错都打它）。
// 存量部署若只配了 [wxpayv3] 没配 [wxpayv3_web]，这里会取不到商户 ⇒ 闸门拒绝下单（不会误发到别的段）。
constexpr const char *kPayTag = "";

// 证书下载页的三段渲染：同步版和协程版只差"那一趟外发"和 return / co_return，页面渲染
// 两个版本共用。哪些项算就绪的判断仍在各调用方，渲染函数只画不改判。
// self_path 是本路由自己的 URL：导航链接指回自己，两版只差一个 _co 后缀。
static void twx_cert_render_nav(httppeer &client, const std::vector<std::string> &all_secs,
                                const std::string &wp_section, const char *self_path)
{
    client << "<h2>商户段（conf/webpay.conf）</h2><p>";
    for (const auto &s : all_secs)
    {
        if (s.rfind("wxpay", 0) != 0) continue;// 本页只往微信段写证书，[alipay] 这类段列出来只会误点
        if (s == wp_section)
            client << "<b>" << html_encode(s) << "</b> ";
        else
            client << "<a href=\"" << self_path << "?section=" << html_encode(s) << "\">" << html_encode(s) << "</a> ";
    }
    client << "</p>";
}

// 当前段配置状态面板（开发者一目了然哪里没配）
static void twx_cert_render_panel(httppeer &client, webpay_config_t &cfg, const std::string &wp_section,
                                  const webpay_merchant_t &mchcfg)
{
    client << "<h2>当前段 [" << html_encode(wp_section.empty() ? std::string("conf 里没有 wxpay* 段") : wp_section) << "] 配置状态</h2><table>";
    client << "<tr><th>商户号 mchid</th><td>"
           << (mchcfg.mch_id.empty() ? std::string("<span class=\"err\">未配置</span>") : html_encode(mchcfg.mch_id)) << "</td></tr>";
    client << "<tr><th>应用 appid</th><td>"
           << (mchcfg.appid.empty() ? std::string("<span class=\"err\">未配置</span>") : html_encode(mchcfg.appid)) << "</td></tr>";
    client << "<tr><th>APIv3 密钥</th><td>" << twx_mask_key(mchcfg.api_v3_key) << "</td></tr>";
    client << "<tr><th>商户证书序列号</th><td>"
           << (mchcfg.v3_serial_no.empty() ? std::string("<span class=\"err\">未配置</span>") : html_encode(mchcfg.v3_serial_no)) << "</td></tr>";
    client << "<tr><th>商户证书 cert_file</th><td>" << html_encode(mchcfg.cert_file)
           << (webpay::credential_readable(mchcfg.cert_file) ? " <span class=\"ok\">可读</span>" : " <span class=\"err\">读不到</span>") << "</td></tr>";
    client << "<tr><th>商户私钥 key_file</th><td>" << html_encode(mchcfg.key_file)
           << (webpay::credential_readable(mchcfg.key_file) ? " <span class=\"ok\">可读</span>" : " <span class=\"err\">读不到</span>") << "</td></tr>";
    std::string pcf = mchcfg.platform_cert_file;
    client << "<tr><th>平台证书 platform_cert_file</th><td>";
    if (pcf.empty())
        client << "<span class=\"err\">未配置（下载后会自动写回）</span>";
    else
        client << html_encode(pcf) << (webpay::credential_readable(pcf) ? " <span class=\"ok\">文件存在</span>" : " <span class=\"err\">文件缺失</span>");
    client << "</td></tr>";
    client << "<tr><th>回调地址 notifyurl</th><td>"
           << (mchcfg.notifyurl.empty() ? std::string("<span class=\"err\">未配置</span>") : html_encode(mchcfg.notifyurl)) << "</td></tr>";
    client << "<tr><th>配置来源</th><td>";
    if (cfg.is_load())
        client << html_encode(cfg.file) << " <span class=\"ok\">已加载</span>";
    else
        client << "<span class=\"err\">未加载，请检查 conf/webpay.conf</span>";
    client << "</td></tr>";
    client << "</table>";
}

// 两道闸门（密钥长度 / 凭据就绪）。返回 true = 页面已经收尾文案备好，调用方只需
// twx_page_foot + return，本次不会向微信发出任何请求。
static bool twx_cert_render_gate(httppeer &client, webpay_config_t &cfg, const std::string &wp_section,
                                 const std::string &v3key, const std::string &serial,
                                 const std::string &keyfile, const std::string &certfile)
{
    if (v3key.size() != 32)
    {
        client << "<p class=\"err\">当前段还没有 32 位 APIv3 密钥（商户平台 → 账户中心 → API安全 → APIv3 密钥）。</p>";
        // api_v3_key 只从 conf 来：既不从 URL 读，也不由本页写回 conf（见下方 need_save 分支）。
        client << "<p>请把密钥写进 " << html_encode(cfg.file.empty() ? std::string("conf/webpay.conf") : cfg.file)
               << " 的 [" << html_encode(wp_section) << "] 段（<code>api_v3_key = 32位密钥</code>），"
               << "然后重启 http 服务器再访问本页（配置只在启动时加载一次）。</p>";
        return true;
    }

    if (serial.empty() || !webpay::credential_readable(keyfile) || !webpay::credential_readable(certfile))
    {
        client << "<p class=\"err\">闸门：凭据未就绪，本次没有向微信发出任何请求。缺下列项：</p><ul>";
        if (serial.empty()) client << "<li>serial_no（商户 API 证书序列号，v3_serial_no）</li>";
        if (!webpay::credential_readable(keyfile)) client << "<li>key_file（商户 API 证书私钥，当前读不到）</li>";
        if (!webpay::credential_readable(certfile)) client << "<li>cert_file（商户 API 证书，当前读不到）</li>";
        client << "</ul>";
        client << "<p>写进 " << html_encode(cfg.file.empty() ? std::string("conf/webpay.conf") : cfg.file)
               << " 的 [" << html_encode(wp_section) << "] 段，改完重启 http 服务器（凭据只从配置文件来，URL 不再覆盖）。</p>";
        return true;
    }
    return false;
}

// 下载结果渲染 + 保存。返回 false = 一个证书都没解出来（原始应答已按原样打出来做诊断），
// 调用方只需 twx_page_foot + return。本函数只执行不改判：主证书选取仍走 webpay::pick_primary_serial。
static bool twx_cert_render_result(httppeer &client, webpay_config_t &cfg, const std::string &wp_section,
                                   const std::string &mch_id, const std::string &raw,
                                   const std::map<std::string, pay::wx_cert_t> &certs, bool need_save)
{
    client << "<h2>下载结果（商户号 " << html_encode(mch_id) << "）</h2>";
    client << "<table><tr><th>平台证书序列号</th><th>过期时间</th><th>保存</th></tr>";
    if (certs.empty())
    {
        client << "</table>";
        client << "<p class=\"err\">没有解出证书，多半是 APIv3 密钥不对（也可能 serial_no/私钥不对）。原始应答：</p><pre>";
        client << html_encode(raw);
        client << "</pre>";
        return false;
    }

    // 选主证书：expire_time 最大者（证书轮换期取最新那张）
    std::string primary = webpay::pick_primary_serial(certs);

    for (const auto &c : certs)
    {
        // 落盘锚点与回调读证书、配置里 platform_cert_file 同一套规则（webpay::platform_cert_path），
        // 不再是依赖 cwd 的 "conf/..." 字面量。空串 = 序列号不可用作文件名，宁可不写也不能写到别处。
        std::string path = webpay::platform_cert_path(cfg.file, c.first);
        client << "<tr><td>" << html_encode(c.first) << (c.first == primary ? " <span class=\"ok\">(主)</span>" : "") << "</td>";
        client << "<td>" << html_encode(c.second.expire_time) << "</td>";
        if (!need_save)
        {
            client << "<td>预览（?save=0）</td>";
        }
        else if (path.empty())
        {
            client << "<td><span class=\"err\">序列号不能作文件名，未写入</span></td>";
        }
        else if (webpay::write_text_file(path, c.second.pem))
        {
            client << "<td><span class=\"ok\">" << html_encode(path) << "</span></td>";
        }
        else
        {
            client << "<td><span class=\"err\">保存失败（目录不可写？）</span></td>";
        }
        client << "</tr>";
    }
    client << "</table>";

    if (!need_save)
    {
        client << "<p>当前为预览模式（?save=0）。去掉该参数即会自动保存并写回 conf。</p>";
        return true;
    }

    // 只写回 platform_cert_file：它的值是网关返回的证书路径，不是用户输入的凭据。
    // api_v3_key 一律不由本页写回——URL 进来的东西不许落进 conf。
    std::string ppath = webpay::platform_cert_path(cfg.file, primary);
    bool okp = (!ppath.empty() && cfg.save_value(wp_section, "platform_cert_file", ppath));
    if (okp)
    {
        client << "<p class=\"ok\">已写回 conf/webpay.conf [" << html_encode(wp_section)
               << "]：platform_cert_file（主证书 " << html_encode(primary) << "）</p>";
        // 写回只动盘上的文件：get_webpay_config() 里那份内存配置是启动时加载的，不重启读不到新值
        client << "<p class=\"warn\">改完建议重启 http 服务器 —— 本进程用的还是启动时加载的那份配置。</p>";
    }
    else
        client << "<p class=\"err\">写回 conf 失败（webpay.conf 未加载？）</p>";
    client << "<p class=\"ok\">平台证书已自动保存到 conf/，无需再手动 ?save=1。</p>";
    return true;
}

struct twxv3_arg
{
    std::string appid;
    std::string mch_id;
    std::string key_file;
    std::string cert_file;
    std::string serial;
    std::string notifyurl;
    std::string total;// 已过滤成纯数字
    std::string out_trade_no;
    std::string openid;     // JSAPI/小程序支付必填（payer.openid）
    std::string cfg_file;
    std::string cfg_section;// 实际取用的段名（kPayTag 命中时是 <tag>.<kind>，回落/裸段时是 kind）—— 页面上标明收款方
    bool cfg_loaded = false;
    std::string url_over;// 哪些业务项是 URL 临时传的（total / openid），只记字段名；凭据项已不许从 URL 来
};

// 商户凭据只来自配置文件，任何人都不能用 URL 查询串换掉收款方。
static void twxv3_fill_cfg_only(twxv3_arg &a, const webpay_merchant_t &mchcfg)
{
    a.key_file  = mchcfg.key_file;
    a.cert_file = mchcfg.cert_file;
    a.serial    = mchcfg.v3_serial_no;
    a.mch_id    = mchcfg.mch_id;
    a.appid     = mchcfg.appid;
    a.notifyurl = mchcfg.notifyurl;
    a.cfg_section = mchcfg.section;
}

// 外发闸门：缺任何一项就不发请求。检查项只取 v3 真正用到的东西——
// body 要 appid/mch_id/out_trade_no/notify_url/amount.total，
// Authorization 头要 mch_id/serial_no，签名要能拿到商户私钥。
static bool twx_v3_gate(httppeer &client, const twxv3_arg &a)
{
    std::vector<std::string> lack;
    if (webpay_is_placeholder(a.appid)) lack.push_back("appid");
    if (webpay_is_placeholder(a.mch_id)) lack.push_back("mch_id");
    if (a.serial.empty()) lack.push_back("serial_no（商户 API 证书序列号，Authorization 头要用）");
    if (!webpay::credential_readable(a.key_file)) lack.push_back("key_file（商户 API 证书私钥，当前既不是 PEM 内容也读不到文件）");
    if (!webpay::credential_readable(a.cert_file)) lack.push_back("cert_file（商户 API 证书，mTLS 双向 TLS 握手必填）");
    if (a.notifyurl.empty()) lack.push_back("notifyurl（native 下单必填）");
    if (a.total.empty() || a.total == "0") lack.push_back("total（金额：正整数，单位分）");

    client << "<h2>请求参数</h2><table>";
    client << "<tr><th>商户号 mchid</th><td>"
           << (webpay_is_placeholder(a.mch_id) ? "<span class=\"err\">未填</span>" : html_encode(a.mch_id)) << "</td></tr>";
    client << "<tr><th>应用 appid</th><td>"
           << (webpay_is_placeholder(a.appid) ? "<span class=\"err\">未填</span>" : html_encode(a.appid)) << "</td></tr>";
    client << "<tr><th>证书序列号</th><td>"
           << (a.serial.empty() ? std::string("<span class=\"err\">未填</span>") : html_encode(a.serial)) << "</td></tr>";
    client << "<tr><th>商户私钥</th><td>" << html_encode(a.key_file)
           << (webpay::credential_readable(a.key_file) ? " <span class=\"ok\">可读取</span>" : " <span class=\"err\">读取失败，检查路径/内容</span>")
           << "</td></tr>";
    client << "<tr><th>商户证书</th><td>" << html_encode(a.cert_file)
           << (webpay::credential_readable(a.cert_file) ? " <span class=\"ok\">可读取</span>" : " <span class=\"err\">读取失败，检查路径/内容</span>")
           << "</td></tr>";
    client << "<tr><th>商户订单号</th><td>" << html_encode(a.out_trade_no) << "</td></tr>";
    client << "<tr><th>金额(分)</th><td>" << html_encode(a.total) << "</td></tr>";
    client << "<tr><th>回调地址</th><td>" << html_encode(a.notifyurl) << "</td></tr>";
    client << "<tr><th>配置来源</th><td>";
    if (a.cfg_loaded)
    {
        client << html_encode(a.cfg_file) << " <span class=\"ok\">已加载</span>";
    }
    else
    {
        client << "<span class=\"err\">webpay.conf 未加载，请检查 conf/webpay.conf</span>";
    }
    if (!a.url_over.empty())
    {
        client << "，URL 覆盖了 " << html_encode(a.url_over);
    }
    // 本次实际取用的商户段：kPayTag 命中时它是 <tag>.<kind>，
    // 缺段/缺关键凭据而软回落到裸段时它是裸段名 —— 两种情况在页面上必须能分辨。
    client << "，商户段 ";
    if (a.cfg_section.empty())
    {
        client << "<span class=\"err\">未取得（这一段在 conf 里不存在）</span>";
    }
    else
    {
        client << "<b>[" << html_encode(a.cfg_section) << "]</b>";
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
        client << "<p>请写进 " << html_encode(a.cfg_file)
               << " 的 [" << html_encode(a.cfg_section.empty() ? std::string("wxpayv3_web") : a.cfg_section)
               << "] 段（appid / mch_id / key_file / v3_serial_no / notifyurl），改完重启 http 服务器。"
                  "凭据只从配置文件来，URL 参数不再覆盖（只有 <code>?total=</code> / <code>?openid=</code> 例外）。"
                  "换收款方的两条路：改这一段，或改 controller/src/test_wxpayv3.cpp 里的 kPayTag 再重新编译。</p>";
        return false;
    }
    if (webpay_is_placeholder(a.notifyurl))
    {
        client << "<p class=\"warn\">回调地址看着还是占位域名：下单不受影响，但微信回调投不到，通知判据本轮不判。</p>";
    }
    return true;
}

// 网关那一趟没成 ⇒ 把刚插进去的待付行标成已撤销，别留在列表页当孤儿单。
// 撤不动只说明这一行的 status 已经不是 0，文案里不去宣称"已撤"。
static void twxv3_store_cancelled(httppeer &client, const std::string &wxorder)
{
    if (!webpay::order_cancel_by_wxorder(wxorder))
    {
        client << "<p class=\"warn\">撤单未生效（cms.orderlist 里这一行 status 已不是 0），需要手工清理。</p>";
    }
}

static asio::awaitable<void> twxv3_store_cancelled_co(httppeer &client, const std::string &wxorder)
{
    if (!co_await webpay::async_order_cancel_by_wxorder(wxorder))
    {
        client << "<p class=\"warn\">撤单未生效（cms.orderlist 里这一行 status 已不是 0），需要手工清理。</p>";
    }
    co_return;
}
#endif// ENABLE_WEBPAY

// 落库这一趟没写进去 ⇒ 后面一步都不许做：库里没这一行，微信那一侧收到的钱就没有可对账的
// 单据（回调 order not found ⇒ 一直重投）。放在 #ifdef 外面：落库不依赖支付 SDK，
// order_new 那两条路由关不关 ENABLE_WEBPAY 都要能落。
static void twxv3_store_failed(httppeer &client)
{
    client << "<p class=\"err\">闸门：订单没写进 cms.orderlist（save 没回自增 ID），"
              "本次没有向微信发出任何请求。请查 [cms] 段的库连接与 orderlist 表。</p>";
}

#ifdef ENABLE_WEBPAY
// 二维码发出后让浏览器轮询支付结果：落账由 /wxpayv3notify 回调完成（status 0→1），
// 本脚本每 3 秒打一次 /testwxpayv3_order_status 读库，见到已支付就跳订单详情。
// 不能整页自动刷新：二维码页每次载入都会新落一单并向微信下单，刷新会刷出一堆待支付垃圾单。
// wxorder 与 orderid 都是本页服务端生成的，拼进 JS 前再过一道 id_safe，不给请求数据留路径。
static void twx_qr_poll(httppeer &client, long long orderid, const std::string &wxorder)
{
    client << "<p id=\"paystate\" class=\"ok\">等待扫码支付，支付完成后本页自动跳转。</p>"
              "<script>(function(){var wx=\""
           << webpay::id_safe(wxorder)
           << "\",oid=" << orderid
           << ",t0=Date.now(),h=setInterval(function(){fetch(\"/testwxpayv3_order_status?wxorder=\"+wx)"
              ".then(function(r){return r.json();}).then(function(d){"
              "if(d.status==1){clearInterval(h);location.href=\"/testwxpayv3_order_detail?orderid=\"+oid;}"
              "else if(Date.now()-t0>600000){clearInterval(h);"
              "document.getElementById(\"paystate\").className=\"err\";"
              "document.getElementById(\"paystate\").textContent=\"超过10分钟仍未支付，二维码可能已过期，请重新下单。\";}"
              "}).catch(function(){});},3000);})();</script>";
}
#endif

//@urlpath(null,testwxpay)
std::string testwxpay(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY

    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);

    twxv3_arg a;
    a.cfg_loaded = cfg.is_load();
    a.cfg_file   = cfg.file.empty() ? std::string("conf/webpay.conf") : cfg.file;

    twxv3_fill_cfg_only(a, mchcfg);

    // str2uint 滤非数字、不抛；0 ＝ 没传，回落 1 分
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want > 0)
    {
        a.url_over += "total ";
    }
    else
    {
        want = 1;
    }
    a.total = std::to_string(want);

    a.out_trade_no = "native" + std::to_string(timeid()) + rand_string(6, 1);

    twx_page_head(client, "微信支付 APIv3 native 下单测试");
    client << "<p>协议口径：JSON 报文 + RSA 签名的 <code>Authorization</code> 头"
              "（<code>pay::wxpay::createNative()</code>，POST <code>/v3/pay/transactions/native</code>）。"
              "V2 的 XML 接口在 <a href=\"/testwxpaynative\">/testwxpaynative</a>。</p>";

    if (!twx_v3_gate(client, a))
    {
        twx_page_foot(client);
        return "";
    }

    // 落库排在下单之前：这一行没写进去就一步都不许往下走
    std::string storeorder_n = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    long long oid = webpay::order_insert(order_paytype::wxpayv3, a.out_trade_no, storeorder_n, "", "APIv3测试商品",
                                         (unsigned int)want);
    if (oid <= 0)
    {
        twxv3_store_failed(client);
        twx_page_foot(client);
        return "";
    }

    pay::wxpay p;
    p.setAppId(a.appid);
    p.setMchId(a.mch_id);
    p.setPrivateKey(a.key_file);
    p.setCertFile(a.cert_file);
    p.setSerialNo(a.serial);
    p.setOutTradeNo(a.out_trade_no);
    p.setDescription("APIv3测试商品");
    p.setTotalAmount(a.total);
    p.setNotifyUrl(a.notifyurl);

    std::string resp = p.createNative();
    webpay::wx_reply rj(resp);

    client << "<h2>微信应答原文（POST /v3/pay/transactions/native）</h2><pre>";
    client << html_encode(resp);
    client << "</pre>";

    std::string code_url = rj["code_url"];
    if (code_url.empty())
    {
        std::string errmsg = rj["message"];
        client << "<p class=\"err\">下单未返回 code_url";
        if (!errmsg.empty())
        {
            client << "：" << html_encode(errmsg);
            client << "（code=" << html_encode(rj["code"]) << "）";
        }
        client << "</p>";
        twxv3_store_cancelled(client, a.out_trade_no);
        client << "<p>常见原因：证书序列号与商户号不匹配、商户私钥不是该证书对应的那把、appid 与 mchid 未绑定。</p>";
    }
    else
    {
        client << "<h2>支付二维码（微信扫码支付 " << html_encode(a.total) << " 分）</h2>";
        client << "<div>" << webpay::qr_svg(code_url) << "</div>";
        client << "<p>code_url：<code>" << html_encode(code_url) << "</code></p>";
        twx_qr_poll(client, oid, a.out_trade_no);
    }

    twx_page_foot(client);
#else
    twx_page_head(client, "微信支付 APIv3 native 下单测试");
    client << "<p class=\"warn\">本二进制编译时没开 ENABLE_WEBPAY，支付路由不产出。</p>";
    twx_page_foot(client);
#endif
    return "";
}

//@urlpath(null,testwxpayv3jsapi)
std::string testwxpayv3jsapi(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY

    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);

    twxv3_arg a;
    a.cfg_loaded = cfg.is_load();
    a.cfg_file   = cfg.file.empty() ? std::string("conf/webpay.conf") : cfg.file;

    twxv3_fill_cfg_only(a, mchcfg);

    // str2uint 滤非数字、不抛；0 ＝ 没传，回落 1 分
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want > 0) a.url_over += "total ";
    else want = 1;
    a.total = std::to_string(want);

    // JSAPI / 小程序支付必须带 openid，由调用方（小程序前端）通过 ?openid= 传入
    a.openid = client.get["openid"].to_string();
    if (!a.openid.empty()) a.url_over += "openid ";

    a.out_trade_no = "jsapi" + std::to_string(timeid()) + rand_string(6, 1);

    twx_page_head(client, "微信支付 APIv3 小程序/JSAPI 支付下单测试");
    client << "<p>协议口径：JSON 报文 + RSA 签名的 <code>Authorization</code> 头"
              "（<code>pay::wxpay::createMiniProgram()</code>，POST <code>/v3/pay/transactions/jsapi</code>）。"
              "用于公众号/小程序支付，必须传 <code>openid</code>（URL 加 <code>?openid=xxx</code>）。"
              "Native 扫码在 <a href=\"/testwxpay\">/testwxpay</a>。</p>";

    if (!twx_v3_gate(client, a))
    {
        twx_page_foot(client);
        return "";
    }

    if (a.openid.empty())
    {
        client << "<p class=\"err\">闸门：JSAPI/小程序支付必须传 openid（URL 加 ?openid=xxx）。</p>";
        twx_page_foot(client);
        return "";
    }

    // 落库排在下单之前：这一行没写进去就一步都不许往下走
    std::string storeorder_j = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    if (webpay::order_insert(order_paytype::wxpayv3, a.out_trade_no, storeorder_j, a.openid, "APIv3小程序支付测试",
                             (unsigned int)want) <= 0)
    {
        twxv3_store_failed(client);
        twx_page_foot(client);
        return "";
    }

    pay::wxpay p;
    p.setAppId(a.appid);
    p.setMchId(a.mch_id);
    p.setPrivateKey(a.key_file);
    p.setCertFile(a.cert_file);
    p.setSerialNo(a.serial);
    p.setOutTradeNo(a.out_trade_no);
    p.setDescription("APIv3小程序支付测试");
    p.setTotalAmount(a.total);
    p.setNotifyUrl(a.notifyurl);
    p.setOpenId(a.openid);

    std::string resp = p.createMiniProgram();
    webpay::wx_reply rj(resp);

    client << "<h2>微信应答原文（POST /v3/pay/transactions/jsapi）</h2><pre>";
    client << html_encode(resp);
    client << "</pre>";

    // 成功时返回 {appId,timeStamp,nonceStr,package,signType,paySign,out_trade_no,...}
    std::string pay_sign = rj["paySign"];
    if (pay_sign.empty())
    {
        std::string errmsg = rj["message"];
        client << "<p class=\"err\">下单未返回 paySign";
        if (!errmsg.empty())
        {
            client << "：" << html_encode(errmsg);
            client << "（code=" << html_encode(rj["code"]) << "）";
        }
        client << "</p>";
        twxv3_store_cancelled(client, a.out_trade_no);
        client << "<p>常见原因：证书序列号与商户号不匹配、openid 与该 appid 不匹配、appid 与 mchid 未绑定、"
                  "或小程序尚未在商户平台绑定该 appid。</p>";
    }
    else
    {
        client << "<h2>小程序调起支付参数（wx.requestPayment 用）</h2><table>";
        client << "<tr><th>appId</th><td>" << html_encode(rj["appId"]) << "</td></tr>";
        client << "<tr><th>timeStamp</th><td>" << html_encode(rj["timeStamp"]) << "</td></tr>";
        client << "<tr><th>nonceStr</th><td>" << html_encode(rj["nonceStr"]) << "</td></tr>";
        client << "<tr><th>package</th><td>" << html_encode(rj["package"]) << "</td></tr>";
        client << "<tr><th>signType</th><td>" << html_encode(rj["signType"]) << "</td></tr>";
        client << "<tr><th>paySign</th><td>" << html_encode(pay_sign) << "</td></tr>";
        client << "<tr><th>out_trade_no</th><td>" << html_encode(rj["out_trade_no"]) << "</td></tr>";
        client << "</table>";
    }

    twx_page_foot(client);
#else
    twx_page_head(client, "微信支付 APIv3 小程序/JSAPI 支付下单测试");
    client << "<p class=\"warn\">本二进制编译时没开 ENABLE_WEBPAY，支付路由不产出。</p>";
    twx_page_foot(client);
#endif
    return "";
}

//@urlpath(testwxpaycert_islogin,testwxpaydownloadcert)
std::string testwxpaydownloadcert(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();

    // 1) 选段：这页的工作对象就是"某一段"（它要往段里写 platform_cert_file），所以只有这里
    //    还认 ?section=；白名单和默认顺序在 webpay::select_wxpay_section()，只放 wxpay* 段。
    std::string wp_section = webpay::select_wxpay_section(cfg, client.get["section"].to_string());
    std::vector<std::string> all_secs = cfg.sections();
    webpay_merchant_t mchcfg = cfg.get_merchant(wp_section);

    // 凭据全部来自 conf 对应段，URL 不能覆盖任何一项：否则拿到 URL 的人既能换下载对象，
    // 又能把换来的 api_v3_key 写进 conf。改凭据只有"改 conf + 重启"一条路。
    const std::string v3key    = mchcfg.api_v3_key;
    const std::string keyfile  = mchcfg.key_file;
    const std::string certfile = mchcfg.cert_file;
    const std::string serial   = mchcfg.v3_serial_no;
    const std::string mch_id   = mchcfg.mch_id;
    // 默认自动保存：只有显式 ?save=0 才只预览不写盘
    bool need_save = (client.get["save"].to_string() != "0");

    twx_page_head(client, "微信支付配置 / 平台证书下载");

    twx_cert_render_nav(client, all_secs, wp_section, "/testwxpaydownloadcert");
    twx_cert_render_panel(client, cfg, wp_section, mchcfg);

    // 5) 下载 + 自动保存平台证书
    if (twx_cert_render_gate(client, cfg, wp_section, v3key, serial, keyfile, certfile))
    {
        twx_page_foot(client);
        return "";
    }

    pay::wxpay p;
    p.setMchId(mch_id);
    p.setPrivateKey(keyfile);
    p.setCertFile(certfile);
    p.setSerialNo(serial);
    p.setApiV3Key(v3key);

    std::string raw = p.downloadCertificates();   // 唯一一趟外发：原始应答既用来解析，也用来做失败诊断
    std::map<std::string, pay::wx_cert_t> certs = p.parsePlatformCerts(raw);

    if (!twx_cert_render_result(client, cfg, wp_section, mch_id, raw, certs, need_save))
    {
        twx_page_foot(client);
        return "";
    }

    twx_page_foot(client);
#else
    twx_page_head(client, "微信支付配置 / 平台证书下载");
    client << "<p class=\"warn\">本二进制编译时没开 ENABLE_WEBPAY，支付路由不产出。</p>";
    twx_page_foot(client);
#endif
    return "";
}

// ===== 协程镜像控制函数（异步非阻塞，内部 co_await pay::wxpay 的 async_ 方法）=====

//@urlpath(testwxpaycert_islogin_co,testwxpaydownloadcert_co)
asio::awaitable<std::string> testwxpaydownloadcert_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();

    // 选段同同步版：只有这页认 ?section=，且只放 has_section() 查得到的 wxpay* 段
    std::string wp_section = webpay::select_wxpay_section(cfg, client.get["section"].to_string());
    std::vector<std::string> all_secs = cfg.sections();

    webpay_merchant_t mchcfg = cfg.get_merchant(wp_section);

    // 同同步版：凭据只从 conf 来，URL 不覆盖。
    const std::string v3key    = mchcfg.api_v3_key;
    const std::string keyfile  = mchcfg.key_file;
    const std::string certfile = mchcfg.cert_file;
    const std::string serial   = mchcfg.v3_serial_no;
    const std::string mch_id   = mchcfg.mch_id;
    bool need_save = (client.get["save"].to_string() != "0");

    twx_page_head(client, "微信支付配置 / 平台证书下载（协程）");

    twx_cert_render_nav(client, all_secs, wp_section, "/testwxpaydownloadcert_co");
    twx_cert_render_panel(client, cfg, wp_section, mchcfg);

    if (twx_cert_render_gate(client, cfg, wp_section, v3key, serial, keyfile, certfile))
    {
        twx_page_foot(client);
        co_return "";
    }

    pay::wxpay p;
    p.setMchId(mch_id);
    p.setPrivateKey(keyfile);
    p.setCertFile(certfile);
    p.setSerialNo(serial);
    p.setApiV3Key(v3key);

    std::string raw = co_await p.async_download_certificates();   // 唯一一趟外发，与同步版同形
    std::map<std::string, pay::wx_cert_t> certs = p.parsePlatformCerts(raw);

    if (!twx_cert_render_result(client, cfg, wp_section, mch_id, raw, certs, need_save))
    {
        twx_page_foot(client);
        co_return "";
    }

    twx_page_foot(client);
#else
    twx_page_head(client, "微信支付配置 / 平台证书下载（协程）");
    client << "<p class=\"warn\">本二进制编译时没开 ENABLE_WEBPAY，支付路由不产出。</p>";
    twx_page_foot(client);
#endif
    co_return "";
}

//@urlpath(null,testwxpayv3native_co)
asio::awaitable<std::string> testwxpayv3native_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);

    twxv3_arg a;
    a.cfg_loaded = cfg.is_load();
    a.cfg_file   = cfg.file.empty() ? std::string("conf/webpay.conf") : cfg.file;

    twxv3_fill_cfg_only(a, mchcfg);

    // str2uint 滤非数字、不抛；0 ＝ 没传，回落 1 分
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want > 0) a.url_over += "total ";
    else want = 1;
    a.total = std::to_string(want);

    a.out_trade_no = "native" + std::to_string(timeid()) + rand_string(6, 1);

    twx_page_head(client, "微信支付 APIv3 native 下单测试（协程）");
    client << "<p>协议口径：JSON 报文 + RSA 签名的 <code>Authorization</code> 头"
              "（<code>pay::wxpay::async_create_native()</code>，POST <code>/v3/pay/transactions/native</code>，协程非阻塞）。"
              "同步版在 <a href=\"/testwxpay\">/testwxpay</a>。</p>";

    if (!twx_v3_gate(client, a))
    {
        twx_page_foot(client);
        co_return "";
    }

    // 落库排在下单之前：这一行没写进去就一步都不许往下走
    std::string storeorder_n = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    long long oid = co_await webpay::async_order_insert(order_paytype::wxpayv3, a.out_trade_no, storeorder_n, "", "APIv3测试商品",
                                                        (unsigned int)want);
    if (oid <= 0)
    {
        twxv3_store_failed(client);
        twx_page_foot(client);
        co_return "";
    }

    pay::wxpay p;
    p.setAppId(a.appid);
    p.setMchId(a.mch_id);
    p.setPrivateKey(a.key_file);
    p.setCertFile(a.cert_file);
    p.setSerialNo(a.serial);
    p.setOutTradeNo(a.out_trade_no);
    p.setDescription("APIv3测试商品");
    p.setTotalAmount(a.total);
    p.setNotifyUrl(a.notifyurl);

    std::string resp = co_await p.async_create_native();
    webpay::wx_reply rj(resp);

    client << "<h2>微信应答原文（POST /v3/pay/transactions/native）</h2><pre>";
    client << html_encode(resp);
    client << "</pre>";

    std::string code_url = rj["code_url"];
    if (code_url.empty())
    {
        std::string errmsg = rj["message"];
        client << "<p class=\"err\">下单未返回 code_url";
        if (!errmsg.empty())
        {
            client << "：" << html_encode(errmsg);
            client << "（code=" << html_encode(rj["code"]) << "）";
        }
        client << "</p>";
        co_await twxv3_store_cancelled_co(client, a.out_trade_no);
    }
    else
    {
        client << "<h2>支付二维码（微信扫码支付 " << html_encode(a.total) << " 分）</h2>";
        client << "<div>" << webpay::qr_svg(code_url) << "</div>";
        client << "<p>code_url：<code>" << html_encode(code_url) << "</code></p>";
        twx_qr_poll(client, oid, a.out_trade_no);
    }

    twx_page_foot(client);
#else
    twx_page_head(client, "微信支付 APIv3 native 下单测试（协程）");
    client << "<p class=\"warn\">本二进制编译时没开 ENABLE_WEBPAY，支付路由不产出。</p>";
    twx_page_foot(client);
#endif
    co_return "";
}

//@urlpath(null,testwxpayv3jsapi_co)
asio::awaitable<std::string> testwxpayv3jsapi_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);

    twxv3_arg a;
    a.cfg_loaded = cfg.is_load();
    a.cfg_file   = cfg.file.empty() ? std::string("conf/webpay.conf") : cfg.file;

    twxv3_fill_cfg_only(a, mchcfg);

    // str2uint 滤非数字、不抛；0 ＝ 没传，回落 1 分
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want > 0) a.url_over += "total ";
    else want = 1;
    a.total = std::to_string(want);

    a.openid = client.get["openid"].to_string();
    if (!a.openid.empty()) a.url_over += "openid ";

    a.out_trade_no = "jsapi" + std::to_string(timeid()) + rand_string(6, 1);

    twx_page_head(client, "微信支付 APIv3 小程序/JSAPI 支付下单测试（协程）");
    client << "<p>协议口径：JSON 报文 + RSA 签名的 <code>Authorization</code> 头"
              "（<code>pay::wxpay::async_create_mini_program()</code>，POST <code>/v3/pay/transactions/jsapi</code>，协程非阻塞）。"
              "同步版在 <a href=\"/testwxpayv3jsapi\">/testwxpayv3jsapi</a>。</p>";

    if (!twx_v3_gate(client, a))
    {
        twx_page_foot(client);
        co_return "";
    }

    if (a.openid.empty())
    {
        client << "<p class=\"err\">闸门：JSAPI/小程序支付必须传 openid（URL 加 ?openid=xxx）。</p>";
        twx_page_foot(client);
        co_return "";
    }

    // 落库排在下单之前：这一行没写进去就一步都不许往下走
    std::string storeorder_j = "SO" + std::to_string(timeid()) + rand_string(6, 0);
    if (co_await webpay::async_order_insert(order_paytype::wxpayv3, a.out_trade_no, storeorder_j, a.openid,
                                            "APIv3小程序支付测试", (unsigned int)want) <= 0)
    {
        twxv3_store_failed(client);
        twx_page_foot(client);
        co_return "";
    }

    pay::wxpay p;
    p.setAppId(a.appid);
    p.setMchId(a.mch_id);
    p.setPrivateKey(a.key_file);
    p.setCertFile(a.cert_file);
    p.setSerialNo(a.serial);
    p.setOutTradeNo(a.out_trade_no);
    p.setDescription("APIv3小程序支付测试");
    p.setTotalAmount(a.total);
    p.setNotifyUrl(a.notifyurl);
    p.setOpenId(a.openid);

    std::string resp = co_await p.async_create_mini_program();
    webpay::wx_reply rj(resp);

    client << "<h2>微信应答原文（POST /v3/pay/transactions/jsapi）</h2><pre>";
    client << html_encode(resp);
    client << "</pre>";

    std::string pay_sign = rj["paySign"];
    if (pay_sign.empty())
    {
        std::string errmsg = rj["message"];
        client << "<p class=\"err\">下单未返回 paySign";
        if (!errmsg.empty())
        {
            client << "：" << html_encode(errmsg);
            client << "（code=" << html_encode(rj["code"]) << "）";
        }
        client << "</p>";
        co_await twxv3_store_cancelled_co(client, a.out_trade_no);
    }
    else
    {
        client << "<h2>小程序调起支付参数（wx.requestPayment 用）</h2><table>";
        client << "<tr><th>appId</th><td>" << html_encode(rj["appId"]) << "</td></tr>";
        client << "<tr><th>timeStamp</th><td>" << html_encode(rj["timeStamp"]) << "</td></tr>";
        client << "<tr><th>nonceStr</th><td>" << html_encode(rj["nonceStr"]) << "</td></tr>";
        client << "<tr><th>package</th><td>" << html_encode(rj["package"]) << "</td></tr>";
        client << "<tr><th>signType</th><td>" << html_encode(rj["signType"]) << "</td></tr>";
        client << "<tr><th>paySign</th><td>" << html_encode(pay_sign) << "</td></tr>";
        client << "<tr><th>out_trade_no</th><td>" << html_encode(rj["out_trade_no"]) << "</td></tr>";
        client << "</table>";
    }

    twx_page_foot(client);
#else
    twx_page_head(client, "微信支付 APIv3 小程序/JSAPI 支付下单测试（协程）");
    client << "<p class=\"warn\">本二进制编译时没开 ENABLE_WEBPAY，支付路由不产出。</p>";
    twx_page_foot(client);
#endif
    co_return "";
}

// ===== 订单管理 / 退款管理（基于 cms.orderlist 模拟，不要求登录，全量展示）=====
//
// 打标与查单的口径不一样，别当成漂移：写侧一律打 paytype=order_paytype::wxpayv3，列表页按它过滤；
// 按 orderid/wxorder 查单时**不带**这个过滤 —— 打标之前落库的老单 paytype=0（未标记），
// 带上过滤就判成"订单不存在"，回填与退款都对不上号；唯一索引 uk_wxorder 保证按单号只命中一行。

// 落库统一走 libs/webpay/webpay_order.h 的 http::webpay::order_insert / async_order_insert。

static void twx_order_rows(httppeer &client, const orm::cms::Orderlist &m)
{
    client << "<h2>订单列表（cms.orderlist，渠道 wxpayv3）</h2>";
    if (m.record.empty())
    {
        client << "<p>暂无订单。</p>";
        return;
    }
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
               << "<td><a href=\"/testwxpayv3_order_detail?orderid=" << o.orderid << "\">详情</a> "
               << "<a href=\"/testwxpayv3_refund?orderid=" << o.orderid << "\">退款</a> "
               << "<a href=\"/testwxpayv3_refund_query?orderid=" << o.orderid << "\">退款查询</a></td></tr>";
    }
    client << "</table>";
}

// 渲染订单列表（本渠道，按 orderid 倒序）
static void twx_order_list_render(httppeer &client)
{
    orm::cms::Orderlist m;
    m.where("paytype", order_paytype::wxpayv3).order("orderid", "desc").limit(100).fetch();
    twx_order_rows(client, m);
}

static asio::awaitable<void> twx_order_list_render_co(httppeer &client)
{
    orm::cms::Orderlist m;
    m.where("paytype", order_paytype::wxpayv3).order("orderid", "desc").limit(100);
    co_await m.async_fetch();
    twx_order_rows(client, m);
}

//@urlpath(null,testwxpayv3_order_new)
std::string testwxpayv3_order_new(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // str2uint 滤非数字、不限长度、不抛；上限自己判（std::stoul 遇超长串抛 out_of_range）
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    unsigned int price = (unsigned int)want;
    std::string total = std::to_string(price);
    std::string desc = client.get["desc"].to_string();
    if (desc.empty()) desc = "APIv3测试商品";
    std::string openid = client.get["openid"].to_string();
    // native / jsapi：微信的 trade_type（下单方式），不是 orderlist.paytype 那个渠道列
    std::string trade_type = client.get["type"].to_string();
    if (trade_type.empty()) trade_type = "native";

    std::string wxorder = "v3o" + std::to_string(timeid()) + rand_string(6, 1);
    std::string storeorder = "SO" + std::to_string(timeid()) + rand_string(6, 1);
    long long oid = webpay::order_insert(order_paytype::wxpayv3, wxorder, storeorder, openid, desc, price);

    twx_page_head(client, "微信支付 V3 新建订单（落库 cms.orderlist）");
    if (oid <= 0)
    {
        // "已落库"只能是"库里真有这一行"的同义词：save 没回自增 ID 就不许这么印，
        // 更不能接着去下单 —— 付成功也找不到账，回调会一直 FAIL。
        twxv3_store_failed(client);
        twx_page_foot(client);
        return "";
    }
    client << "<p>已生成订单并落库（测试用，不要求登录）。商户单号 <code>" << html_encode(wxorder)
           << "</code>，内部单号 <code>" << html_encode(storeorder) << "</code>，订单ID " << oid << "。</p>";

#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);
    twxv3_arg a;
    a.cfg_loaded = cfg.is_load();
    a.cfg_file = cfg.file.empty() ? std::string("conf/webpay.conf") : cfg.file;
    twxv3_fill_cfg_only(a, mchcfg);
    a.openid = openid;   // 闸门与 setOpenId 都读 a.openid；不填这一行 ?type=jsapi 恒被判"缺 openid"
    a.total = total;
    if (!twx_v3_gate(client, a))
    {
        client << "<p class=\"warn\">订单已落库，但凭据未就绪，未向微信发起下单。</p>";
        twx_order_list_render(client); twx_page_foot(client); return "";
    }
    if (trade_type == "jsapi" && a.openid.empty())
    {
        client << "<p class=\"err\">JSAPI/小程序支付必须传 openid（?openid=xxx）。订单已落库。</p>";
        twx_order_list_render(client); twx_page_foot(client); return "";
    }
    pay::wxpay p;
    p.setAppId(a.appid); p.setMchId(a.mch_id); p.setPrivateKey(a.key_file);
    p.setCertFile(a.cert_file); p.setSerialNo(a.serial);
    p.setOutTradeNo(wxorder); p.setDescription(desc); p.setTotalAmount(total); p.setNotifyUrl(a.notifyurl);
    if (trade_type == "jsapi") p.setOpenId(a.openid);
    std::string resp = (trade_type == "jsapi") ? p.createMiniProgram() : p.createNative();
    webpay::wx_reply rj(resp);
    client << "<h2>微信应答原文</h2><pre>" << html_encode(resp) << "</pre>";
    if (trade_type != "jsapi")
    {
        std::string code_url = rj["code_url"];
        if (!code_url.empty())
        {
            client << "<div>" << webpay::qr_svg(code_url) << "</div>";
            twx_qr_poll(client, oid, wxorder);
        }
    }
    // 网关没接单 ⇒ 撤掉这一行；native 看出 code_url，jsapi 看出 paySign。
    const bool got_pay = (trade_type == "jsapi") ? !rj["paySign"].empty()
                                                : !rj["code_url"].empty();
    if (!got_pay) twxv3_store_cancelled(client, wxorder);
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，仅落库，未向微信下单。</p>";
#endif
    twx_order_list_render(client);
    twx_page_foot(client);
    return "";
}

//@urlpath(null,testwxpayv3_order_list)
std::string testwxpayv3_order_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    twx_page_head(client, "微信支付 V3 订单列表");
    twx_order_list_render(client);
    twx_page_foot(client);
    return "";
}

//@urlpath(null,testwxpayv3_order_detail)
std::string testwxpayv3_order_detail(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    unsigned int rows = m.fetch_one();
    twx_page_head(client, "微信支付 V3 订单详情");
    if (rows == 0)
    {
        client << "<p class=\"err\">订单不存在（orderid=" << oid << "）。</p>";
        twx_page_foot(client); return "";
    }
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

#ifdef ENABLE_WEBPAY
    if (!o.wxorder.empty())
    {
        webpay_config_t &cfg = get_webpay_config();
        webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);
        pay::wxpay p;
        p.setAppId(mchcfg.appid); p.setMchId(mchcfg.mch_id);
        p.setPrivateKey(mchcfg.key_file); p.setCertFile(mchcfg.cert_file); p.setSerialNo(mchcfg.v3_serial_no);
        std::string resp = p.queryTrade(o.wxorder);
        webpay::wx_reply rj(resp);
        client << "<h2>微信查单（queryTrade）</h2><pre>" << html_encode(resp) << "</pre>";
        std::string trade_state = rj["trade_state"];
        // 回填要过两道：金额相符，且本地还没落过账。第二道在 order_set_paid 的 CAS 里
        // （WHERE wxorder AND status=0）—— 少了它，已退款/已撤销的单会被这条查单改回 1，
        // 那笔退款就在页面上"消失"了。
        int settle = webpay::wx_trade_settle(resp, o.payprice);
        if (settle < 0)
        {
            client << "<p class=\"err\">trade_state=SUCCESS，但应答金额与库里的 payprice（"
                   << o.payprice << " 分）不符，不回填。</p>";
        }
        else if (settle > 0)
        {
            if (webpay::order_set_paid(webpay::order_ref_of(m)))
                client << "<p class=\"ok\">trade_state=SUCCESS 且金额相符，已回填 paytime / status=1。</p>";
            else
                client << "<p>已是 status=" << (int)o.status << "，不重复回填（paytime 保持第一次落账的时间）。</p>";
        }
        else if (!trade_state.empty())
        {
            client << "<p>trade_state=" << html_encode(trade_state) << "</p>";
        }
    }
#endif
    client << "<p><a href=\"/testwxpayv3_refund?orderid=" << o.orderid << "\">申请退款</a> | "
              "<a href=\"/testwxpayv3_refund_query?orderid=" << o.orderid << "\">退款查询</a></p>";
    twx_page_foot(client);
    return "";
}

//@urlpath(null,testwxpayv3_refund)
std::string testwxpayv3_refund(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    unsigned int rows = m.fetch_one();
    twx_page_head(client, "微信支付 V3 申请退款");
    if (rows == 0) { client << "<p class=\"err\">订单不存在。</p>"; twx_page_foot(client); return ""; }
    auto &o = m.data;
    webpay::order_ref ref = webpay::order_ref_of(m);

    // 退款闸门走 webpay::can_refund：status 必须已被支付回调推进到 1，金额落在 (0, payprice]。
    // 各渠道共用这一个闸门，V3 不自写一份。
    // str2uint 滤非数字、不抛；0 ＝ 没传，按订单金额全额退（口径在 resolve_refund_fen）。
    unsigned long long rfee =
        webpay::resolve_refund_fen(ref, str2uint(client.get["refund"].to_string()));
    std::string refund = std::to_string(rfee);
    std::string why;
    if (!webpay::can_refund(ref, rfee, why))
    {
        client << "<p class=\"err\">" << html_encode(why) << "未向微信发出任何请求。</p>";
        twx_page_foot(client);
        return "";
    }
    std::string reason = client.get["reason"].to_string();
    if (reason.empty()) reason = "测试退款";
    // 幂等键由订单侧生成：同一次退款的两路重放拿到同一个号，网关只受理一次（键形态见订单层注释）
    std::string refund_no = webpay::refund_no_gen(ref);

    client << "<p>订单 #" << o.orderid << " 微信单号 <code>" << html_encode(o.wxorder)
           << "</code>，退款单号 <code>" << html_encode(refund_no)
           << "</code>，退款金额 " << refund << " 分（原单 " << o.payprice << " 分）。</p>";
    client << "<p>" << html_encode(webpay::refund_once_notice()) << "</p>";

#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);
    pay::wxpay p;
    p.setAppId(mchcfg.appid); p.setMchId(mchcfg.mch_id);
    p.setPrivateKey(mchcfg.key_file); p.setCertFile(mchcfg.cert_file); p.setSerialNo(mchcfg.v3_serial_no);
    p.setApiV3Key(mchcfg.api_v3_key);
    // amount.total 必须是原单总额（分）：不 set 的话 refundTrade 拼出 "total":0，微信直接拒
    p.setTotalAmount(std::to_string(o.payprice));
    p.setOutRefundNo(refund_no);
    std::string resp = p.refundTrade(refund, reason, o.wxorder);
    webpay::wx_reply rj(resp);
    client << "<h2>微信应答</h2><pre>" << html_encode(resp) << "</pre>";
    // 受理只认 status(SUCCESS/PROCESSING) + refund_id；CLOSED / ABNORMAL 不落账。
    // 别用"code 空 + 正文里出现过 out_refund_no"来认——网关回一张含该子串的 HTML 错误页就会误判。
    if (webpay::refund_reply_accepted(rj["status"], rj["refund_id"]))
    {
        // 落账是一条 CAS（WHERE wxorder AND status=1）：isrefund=1 / status=3 / refundnum 自增
        if (webpay::order_set_refunded(ref))
            client << "<p class=\"ok\">退款受理成功，已更新 isrefund=1 / status=3，refundnum 自增 1。</p>";
        else
            client << "<p class=\"warn\">退款受理成功，但订单状态已不是 1（已付），未改写落库 —— 请核对是否已被另一笔推进。</p>";
    }
    else
    {
        client << "<p class=\"err\">退款未受理（status=" << html_encode(rj["status"])
               << "，code=" << html_encode(rj["code"]) << "），未改写落库。</p>";
    }
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，未向微信发起退款。</p>";
#endif
    twx_page_foot(client);
    return "";
}

//@urlpath(null,testwxpayv3_refund_query)
std::string testwxpayv3_refund_query(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    unsigned int rows = m.fetch_one();
    twx_page_head(client, "微信支付 V3 退款查询");
    if (rows == 0) { client << "<p class=\"err\">订单不存在。</p>"; twx_page_foot(client); return ""; }
    auto &o = m.data;
    client << "<p>微信单号 <code>" << html_encode(o.wxorder) << "</code></p>";
#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);
    pay::wxpay p;
    p.setAppId(mchcfg.appid); p.setMchId(mchcfg.mch_id);
    p.setPrivateKey(mchcfg.key_file); p.setCertFile(mchcfg.cert_file); p.setSerialNo(mchcfg.v3_serial_no);
    p.setApiV3Key(mchcfg.api_v3_key);
    std::string resp = p.queryRefund(o.wxorder);
    client << "<h2>微信退款单</h2><pre>" << html_encode(resp) << "</pre>";
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启。</p>";
#endif
    twx_page_foot(client);
    return "";
}

//@urlpath(null,testwxpayv3_order_new_co)
asio::awaitable<std::string> testwxpayv3_order_new_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // 同同步孪生：str2uint 滤非数字、不抛；上限自己判
    unsigned long long want = str2uint(client.get["total"].to_string());
    if (want == 0 || want > 100000) want = 1;
    unsigned int price = (unsigned int)want;
    std::string total = std::to_string(price);
    std::string desc = client.get["desc"].to_string();
    if (desc.empty()) desc = "APIv3测试商品";
    std::string openid = client.get["openid"].to_string();
    // native / jsapi：微信的 trade_type（下单方式），不是 orderlist.paytype 那个渠道列
    std::string trade_type = client.get["type"].to_string();
    if (trade_type.empty()) trade_type = "native";

    std::string wxorder = "v3o" + std::to_string(timeid()) + rand_string(6, 1);
    std::string storeorder = "SO" + std::to_string(timeid()) + rand_string(6, 1);
    long long oid = co_await webpay::async_order_insert(order_paytype::wxpayv3, wxorder, storeorder, openid, desc, price);

    twx_page_head(client, "微信支付 V3 新建订单（协程，落库 cms.orderlist）");
    if (oid <= 0)
    {
        // "已落库"只能是"库里真有这一行"的同义词：save 没回自增 ID 就不许这么印。
        twxv3_store_failed(client);
        twx_page_foot(client);
        co_return "";
    }
    client << "<p>已落库（协程非阻塞）。商户单号 <code>" << html_encode(wxorder)
           << "</code>，内部单号 <code>" << html_encode(storeorder) << "</code>，订单ID " << oid << "。</p>";

#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);
    twxv3_arg a;
    a.cfg_loaded = cfg.is_load();
    a.cfg_file = cfg.file.empty() ? std::string("conf/webpay.conf") : cfg.file;
    twxv3_fill_cfg_only(a, mchcfg);
    a.openid = openid;   // 同同步孪生：闸门与 setOpenId 都读 a.openid
    a.total = total;
    if (!twx_v3_gate(client, a))
    {
        client << "<p class=\"warn\">订单已落库，但凭据未就绪，未向微信发起下单。</p>";
        co_await twx_order_list_render_co(client); twx_page_foot(client); co_return "";
    }
    if (trade_type == "jsapi" && a.openid.empty())
    {
        client << "<p class=\"err\">JSAPI/小程序支付必须传 openid（?openid=xxx）。订单已落库。</p>";
        co_await twx_order_list_render_co(client); twx_page_foot(client); co_return "";
    }
    pay::wxpay p;
    p.setAppId(a.appid); p.setMchId(a.mch_id); p.setPrivateKey(a.key_file);
    p.setCertFile(a.cert_file); p.setSerialNo(a.serial);
    p.setOutTradeNo(wxorder); p.setDescription(desc); p.setTotalAmount(total); p.setNotifyUrl(a.notifyurl);
    if (trade_type == "jsapi") p.setOpenId(a.openid);
    std::string resp = co_await (trade_type == "jsapi" ? p.async_create_mini_program() : p.async_create_native());
    webpay::wx_reply rj(resp);
    client << "<h2>微信应答原文（协程）</h2><pre>" << html_encode(resp) << "</pre>";
    if (trade_type != "jsapi")
    {
        std::string code_url = rj["code_url"];
        if (!code_url.empty())
        {
            client << "<div>" << webpay::qr_svg(code_url) << "</div>";
            twx_qr_poll(client, oid, wxorder);
        }
    }
    // 网关没接单 ⇒ 撤掉这一行；native 看出 code_url，jsapi 看出 paySign。
    const bool got_pay = (trade_type == "jsapi") ? !rj["paySign"].empty()
                                                : !rj["code_url"].empty();
    if (!got_pay) co_await twxv3_store_cancelled_co(client, wxorder);
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，仅落库。</p>";
#endif
    co_await twx_order_list_render_co(client);
    twx_page_foot(client);
    co_return "";
}

//@urlpath(null,testwxpayv3_order_query_co)
asio::awaitable<std::string> testwxpayv3_order_query_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    unsigned int rows = co_await m.async_fetch_one();
    twx_page_head(client, "微信支付 V3 订单查单同步（协程）");
    if (rows == 0) { client << "<p class=\"err\">订单不存在。</p>"; twx_page_foot(client); co_return ""; }
    auto &o = m.data;
    client << "<h2>订单 #" << o.orderid << " 微信单号 <code>" << html_encode(o.wxorder) << "</code></h2>";
#ifdef ENABLE_WEBPAY
    if (!o.wxorder.empty())
    {
        webpay_config_t &cfg = get_webpay_config();
        webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);
        pay::wxpay p;
        p.setAppId(mchcfg.appid); p.setMchId(mchcfg.mch_id);
        p.setPrivateKey(mchcfg.key_file); p.setCertFile(mchcfg.cert_file); p.setSerialNo(mchcfg.v3_serial_no);
        std::string resp = co_await p.async_query_trade(o.wxorder);
        webpay::wx_reply rj(resp);
        client << "<h2>微信查单（async_query_trade）</h2><pre>" << html_encode(resp) << "</pre>";
        std::string trade_state = rj["trade_state"];
        // 回填要过两道：金额相符，且本地还没落过账（同 /testwxpayv3_order_detail 的口径）。
        // 第二道在 order_set_paid 的 CAS 里（WHERE wxorder AND status=0）。
        int settle = webpay::wx_trade_settle(resp, o.payprice);
        if (settle < 0)
        {
            client << "<p class=\"err\">trade_state=SUCCESS，但应答金额与库里的 payprice（"
                   << o.payprice << " 分）不符，不回填。</p>";
        }
        else if (settle > 0)
        {
            if (co_await webpay::async_order_set_paid(webpay::order_ref_of(m)))
                client << "<p class=\"ok\">trade_state=SUCCESS 且金额相符，已回填 paytime / status=1。</p>";
            else
                client << "<p>已是 status=" << (int)o.status << "，不重复回填（paytime 保持第一次落账的时间）。</p>";
        }
        else if (!trade_state.empty())
        {
            client << "<p>trade_state=" << html_encode(trade_state) << "</p>";
        }
    }
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启。</p>";
#endif
    twx_page_foot(client);
    co_return "";
}

// 二维码页轮询用的轻量状态接口：只按商户单号读库里的 status，不碰网关
// （落账是 /wxpayv3notify 的职责，轮询端再查一次网关只会白增外发流量）。
// 不受 ENABLE_WEBPAY 管辖：orderlist 查询不依赖支付 SDK，两档都要能答。
// status=-1 表示查无此单（单号非法或还没落库）。
//@urlpath(null,testwxpayv3_order_status)
asio::awaitable<std::string> testwxpayv3_order_status(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string wxorder = webpay::id_safe(client.get["wxorder"].to_string());
    client.val["status"] = -1;
    if (!wxorder.empty())
    {
        webpay::order_ref ref;
        if (co_await webpay::async_order_find(wxorder, ref))
            client.val["status"] = ref.status;
    }
    client.out_json();
    co_return "";
}

//@urlpath(null,testwxpayv3_refund_co)
asio::awaitable<std::string> testwxpayv3_refund_co(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    int oid = client.get["orderid"].to_int();
    orm::cms::Orderlist m;
    m.where("orderid", oid);
    unsigned int rows = co_await m.async_fetch_one();
    twx_page_head(client, "微信支付 V3 申请退款（协程）");
    if (rows == 0) { client << "<p class=\"err\">订单不存在。</p>"; twx_page_foot(client); co_return ""; }
    auto &o = m.data;
    webpay::order_ref ref = webpay::order_ref_of(m);

    // 与同步孪生同一套闸门与金额口径：闸门走 can_refund，0 ＝ 没传按全额退
    unsigned long long rfee =
        webpay::resolve_refund_fen(ref, str2uint(client.get["refund"].to_string()));
    std::string refund = std::to_string(rfee);
    std::string why;
    if (!webpay::can_refund(ref, rfee, why))
    {
        client << "<p class=\"err\">" << html_encode(why) << "未向微信发出任何请求。</p>";
        twx_page_foot(client);
        co_return "";
    }
    std::string reason = client.get["reason"].to_string();
    if (reason.empty()) reason = "测试退款";
    std::string refund_no = webpay::refund_no_gen(ref);
    client << "<p>订单 #" << o.orderid << " 微信单号 <code>" << html_encode(o.wxorder)
           << "</code>，退款单号 <code>" << html_encode(refund_no)
           << "</code>，退款金额 " << refund << " 分（原单 " << o.payprice << " 分）。</p>";
    client << "<p>" << html_encode(webpay::refund_once_notice()) << "</p>";
#ifdef ENABLE_WEBPAY
    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);
    pay::wxpay p;
    p.setAppId(mchcfg.appid); p.setMchId(mchcfg.mch_id);
    p.setPrivateKey(mchcfg.key_file); p.setCertFile(mchcfg.cert_file); p.setSerialNo(mchcfg.v3_serial_no);
    p.setApiV3Key(mchcfg.api_v3_key);
    p.setTotalAmount(std::to_string(o.payprice));
    p.setOutRefundNo(refund_no);
    std::string resp = co_await p.async_refund_trade(refund, reason, o.wxorder);
    webpay::wx_reply rj(resp);
    client << "<h2>微信应答（协程）</h2><pre>" << html_encode(resp) << "</pre>";
    // 受理检查与同步孪生同一个（webpay::refund_reply_accepted），实现只有一份
    if (webpay::refund_reply_accepted(rj["status"], rj["refund_id"]))
    {
        // 落账是一条 CAS（WHERE wxorder AND status=1）：isrefund=1 / status=3 / refundnum 自增
        if (co_await webpay::async_order_set_refunded(ref))
            client << "<p class=\"ok\">退款受理成功，已更新 isrefund=1 / status=3，refundnum 自增 1。</p>";
        else
            client << "<p class=\"warn\">退款受理成功，但订单状态已不是 1（已付），未改写落库 —— 请核对是否已被另一笔推进。</p>";
    }
    else
    {
        client << "<p class=\"err\">退款未受理（status=" << html_encode(rj["status"])
               << "，code=" << html_encode(rj["code"]) << "），未改写落库。</p>";
    }
#else
    client << "<p class=\"warn\">ENABLE_WEBPAY 未开启，未向微信发起退款。</p>";
#endif
    twx_page_foot(client);
    co_return "";
}

// ===== 微信支付 APIv3 异步回调处理 =====
// 路由：/wxpayv3notify  必须在 conf/webpay.conf 对应商户段把 notifyurl 指到这个地址
// 微信发送 POST，带 Wechatpay-Timestamp / Wechatpay-Nonce / Wechatpay-Signature / Wechatpay-Serial 四个头
// 响应必须是 HTTP 200 + JSON {"code":"SUCCESS","message":"成功"} 才会停重投
//@urlpath(null,ordernotify)
asio::awaitable<std::string> test_wxpay_v3_notify(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("application/json; charset=utf-8");
#ifdef ENABLE_WEBPAY

    webpay_config_t &cfg = get_webpay_config();
    webpay_merchant_t mchcfg = cfg.wxpayv3_web(kPayTag);

    // 1) 收集 V3 回调四个头（大小写不敏感，httppeer::get_header 内部已处理）
    std::map<std::string, std::string> headers;
    headers["Wechatpay-Timestamp"] = client.get_header("Wechatpay-Timestamp");
    headers["Wechatpay-Nonce"]     = client.get_header("Wechatpay-Nonce");
    headers["Wechatpay-Signature"] = client.get_header("Wechatpay-Signature");
    headers["Wechatpay-Serial"]    = client.get_header("Wechatpay-Serial");

    // 2) 实例化 wxpay 并加载商户凭据
    pay::wxpay p;
    p.setMchId(mchcfg.mch_id);
    p.setPrivateKey(mchcfg.key_file);
    p.setCertFile(mchcfg.cert_file);
    p.setSerialNo(mchcfg.v3_serial_no);
    p.setApiV3Key(mchcfg.api_v3_key);

    // 3) 平台证书：本地优先。/testwxpaydownloadcert 已把每张证书按 webpay::platform_cert_path()
    //    的规则落盘（conf 文件同目录，绝对路径），回调头里就带着 Wechatpay-Serial，按它直接找
    //    本地那份即可，不必为每笔回调都去网关下载。本地没有（微信换了新证书、还没下载过）才
    //    best-effort 下载兜底；下载失败也不影响，还有 platform_cert_file 那张主证书在。
    //    读写两侧共用同一个路径函数——从别的工作目录起进程也不会"下到了却读不到"。
    const std::string &wx_serial = headers["Wechatpay-Serial"];
    std::string local_cert = webpay::local_platform_cert(cfg.file, wx_serial);
    if (!local_cert.empty())
        p.setPlatformCert(local_cert);
    else if (!mchcfg.platform_cert_file.empty())
        p.setPlatformCert(mchcfg.platform_cert_file);

    // 本地没有这张证书时，那趟下载有两个必败前提：① APIv3 密钥不对 ⇒ 拿回来的密文解不开，
    // 纯烧一趟外网；② 上一次真去下载已经失败了 ⇒ 配置不会在两次重投之间自己变好。是否外发
    // 由 webpay::plan_cert_fetch() 的三道闸决定（先后次序：本地证书在位 > 密钥可用 > 冷却
    // 窗口），这里只执行它给的那一格。挡住/失败的原因写进 FAIL 应答，让商户平台能区分是
    // 配置问题还是报文问题（SDK 那句 "decrypt failed" 本身分不出来）。
    // 冷却表按商户号分格，这一趟只在"本地没有证书"时才可能发——正常成交那条路不受影响。
    std::string certwhy;
    const unsigned long long cert_now = webpay::steady_seconds();
    switch (webpay::plan_cert_fetch(!local_cert.empty(), mchcfg.api_v3_key, mchcfg.mch_id, cert_now, certwhy))
    {
    case webpay::cert_fetch_plan::fetch:
    {
        const bool fetched = co_await p.async_download_and_save_cert();
        webpay::cert_fetch_report(mchcfg.mch_id, cert_now, fetched);
        if (!fetched) certwhy = "本地无平台证书，下载没拿到可解密的证书";
        break;
    }
    default: break;// use_local / key_unusable / cooling_down：都不外发，后两格在 certwhy 里给了原因
    }

    // 4) 验签 + AES-GCM 解密
    auto resp = p.handleNotify(client.rawcontent, headers);
    if (resp["status_code"] != "0")
    {
        // 验签失败 / 解密失败 → 回 FAIL 让微信重投。上面那道下载闸被触发过（或本来就该触发）
        // 时把原因并进去，商户平台的回调记录里就能看出是配置问题。
        client.output = p.notifyFailReply(certwhy.empty() ? resp["error_msg"]
                                                         : resp["error_msg"] + " / " + certwhy);
        co_return "";
    }

    // 这里不判回调归属：落账字段取自本商户 api_v3_key 解出的明文，跨商户通知与取错段都会在
    // 上面那道解密就失败。哪些渠道承重写在 webpay_notify.h。

    // 5) trade_state 判定：只 SUCCESS / SUCCESS/REFUND 算真正成交并落账
    //    其余状态（ORDER_NOT_EXIST / PAYERROR / CLOSED）回 SUCCESS 收下，不改库
    //    —— 回 SUCCESS 才会让微信停重投，否则它会一直来
    const std::string &trade_state = resp["trade_state"];
    if (trade_state != "SUCCESS")
    {
        client.output = p.notifySuccessReply();
        co_return "";
    }

    // 6) 查单 → 金额逐分比对 → 0→1 CAS：四条通知路由共用 webpay::settle_notify()，
    //    这里只留"该回哪种应答"。漏单绝不能回 SUCCESS（那等于把钱收了却不入账）；
    //    settled 与 duplicate 都回 SUCCESS——重复通知不再写，但要让微信停手。
    //    fee 用 str2uint：滤非数字、不限长度、不抛，0 会被共用体判成金额不符。
    auto st = co_await webpay::settle_notify(resp["out_trade_no"], str2uint(resp["total"]));
    client.output = st.ok() ? p.notifySuccessReply()
                            : p.notifyFailReply(st.code == webpay::settle_code::amount_mismatch
                                                    ? "amount mismatch" : "order not found");
#else
    client.output = "{\"code\":\"FAIL\",\"message\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    co_return "";
}

}// namespace http
