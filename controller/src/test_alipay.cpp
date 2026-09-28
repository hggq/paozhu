#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include "httppeer.h"
#include "test_alipay.h"
#include "func.h"
#include "request.h"
#include "httpclient.h"
#include "server_localvar.h"
#ifdef ENABLE_WEBPAY
#include "alipay.h"
#include "webpay_config.h"
#include "md5.h"
#include "webpay/webpay_order.h"
#include "webpay/webpay_notify.h"
#include "webpay/webpay_cert_gate.h"
#include "orm.h"
#include "orm_query.h"
#endif
// 二维码这一层同样不依赖支付 SDK，也不依赖 ENABLE_IMAGE：没编图像时 qr_svg 回空串，
// 由调用方退化成一句话提示（三处二维码路由口径一致）。
#include "webpay/webpay_qr.h"
#include "webpay/webpay_alipay.h"

#ifdef ENABLE_WEBPAY
// 必须在 namespace http 之外定义：在这里写 namespace orm::cust 会先造出 http::orm，
// 之后同一 namespace 里的 orm::cms::Orderlist 就会被解析成 http::orm::cms 而编译失败。
namespace orm::cust
{
    struct TalDbIdent : orm::Base<TalDbIdent>
    {
        std::string db;
        std::string ver;
        std::string cnt;
        ORM_NAMES(db, ver, cnt);
    };
} // namespace orm::cust
#endif

namespace http
{

#ifdef ENABLE_WEBPAY
// 凭据装配与闸门在 libs/webpay/webpay_alipay.h（webpay::alipay_fill）：凭据只来自
// conf/webpay.conf，不接受请求参数覆盖；appid/notifyurl/私钥三者都只能是运维写进 conf 的值。
// kPayTag 留空 = 公共裸段 [alipay]。要按业务隔离收款方：改成 "sitea" 这类 tag 并重新编译，
// 走 [<tag>.alipay]；缺段或关键凭据缺项时配置层软回落回裸段，实际取用的段名在 mch.section。
constexpr const char *kPayTag = "";

// 排障页：把"这台机器到底能不能把 [alipay] 的密钥读出来"一路判到浏览器里。
// 零外发：createTradePage() 只在本地拼串+签名，一页跑完不往任何地方发消息。
// 不打印任何密钥内容 —— 只有路径、字节数、以及一堆"是/否"结论词。
static std::string tald_yesno(bool v)
{
    return v ? "是" : "否";
}

// 一个密钥字段值判到底：三种形态各判一遍，再问交付代码能不能读成密钥对象
struct tald_keyform
{
    bool isfile   = false;
    bool isdir    = false;
    bool pemtext  = false; // 值里直接写着 -----BEGIN
    bool pemfile  = false; // 是个文件，且文件开头有 -----BEGIN
    bool bare     = false; // 是个文件，但没有 PEM 头 ⇒ 裸 base64 一行
    bool readable = false;
    unsigned long long bytes = 0;
    std::string form;
};

static tald_keyform tald_inspect(const std::string &value, bool is_private)
{
    tald_keyform r;
    std::error_code ec;
    r.pemtext = value.find("-----BEGIN") != std::string::npos;
    r.isfile  = webpay::file_exists_regular(value);
    r.isdir   = !value.empty() && std::filesystem::is_directory(value, ec);
    if (r.isfile)
    {
        r.bytes = (unsigned long long)std::filesystem::file_size(value, ec);
        char head[65] = {0};
        std::ifstream in(value, std::ios::binary);
        in.read(head, 64);
        r.pemfile = std::string(head).find("-----BEGIN") != std::string::npos;
        r.bare    = !r.pemfile;
    }
    if (r.pemtext) r.form = "PEM 文本直填";
    else if (r.pemfile) r.form = "PEM 文件";
    else if (r.bare) r.form = "裸 base64 文件";
    else if (value.empty()) r.form = "空值";
    else r.form = "裸 base64 值（没当成文件，因为盘上没有这个文件）";

    pay::alipay probe;
    if (is_private)
    {
        probe.setPrivateKey(value);
        r.readable = probe.privateKeyReadable();
    }
    else
    {
        probe.setPublicKey(value);
        r.readable = probe.publicKeyReadable();
    }
    return r;
}

// 进 href 的单号转义。
static std::string tal_url_escape(const std::string &s)
{
    static const char *hex = "0123456789ABCDEF";
    std::string out;
    out.reserve(s.size() + 16);
    for (unsigned char c : s)
    {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' ||
            c == '.')
        {
            out.push_back((char)c);
        }
        else
        {
            out.push_back('%');
            out.push_back(hex[c >> 4]);
            out.push_back(hex[c & 0x0f]);
        }
    }
    return out;
}

// 元 → 分用 vendor 的 http::yuan_to_fen（webpay_config.h），不在这里再写一份。

// INSERT orderlist —— 下单时写一条待支付记录，同步版与协程版共用同一份列填法
// （tal_order_insert / tal_order_insert_async）。返回值是自增 orderid（save() 后自动回填），0 表示失败。
// err：成败都填一行结果文本（effect / lastid / error_msg / 语句原文）——INSERT 静默返回 {0,0}
// 的路径有 iserror 和 conn_empty() 两条，页面少打一行 orderid 分不清是哪一种。
// 不并入 libs/webpay 的 order_insert：这一路额外写 payip/content/isship/isfinsh/isremove/jifen
// 且不写 openid，公共签名装不下（硬统会把那几列写成空）。
static void tal_order_fill(orm::cms::Orderlist &order,
                           const std::string &out_trade_no,
                           const std::string &total_amount,
                           const std::string &subject,
                           const std::string &body,
                           const std::string &client_ip)
{
    order.data.paytype    = order_paytype::alipay;
    order.data.wxorder    = out_trade_no;
    order.data.totalnum   = 1;
    order.data.addtime    = (unsigned int)std::time(nullptr);
    order.data.payip      = client_ip;
    order.data.payprice   = yuan_to_fen(total_amount);
    order.data.paytitle   = subject;
    order.data.content    = body;
    order.data.status     = order_status::pending;
    order.data.isrefund   = 0;
    order.data.isship     = 0;
    order.data.isfinsh    = 0;
    order.data.isremove   = 0;
    order.data.refundnum  = 0;
    order.data.jifen      = 0;
}

// 同步版与协程版共用这一行 INSERT 结果文本，两条路的输出保持完全一致。
static void tal_order_insrep(orm::cms::Orderlist &order, unsigned int effect,
                             unsigned long long lastid, std::string *err)
{
    if (err == nullptr) return;
    *err = "effect=" + std::to_string(effect) + " lastid=" + std::to_string(lastid)
         + " iserror=" + (order.iserror ? "1" : "0") + " 表=" + order.tablename
         + " 库标签=" + order.dbtag + " error_msg=[" + order.error_msg + "]"
         + "<br>语句=[" + html_encode(order.get_query()) + "]";
}

static unsigned int tal_order_insert(const std::string &out_trade_no,
                                     const std::string &total_amount,
                                     const std::string &subject,
                                     const std::string &body,
                                     const std::string &client_ip,
                                     std::string *err = nullptr)
{
    auto order                = orm::cms::Orderlist();
    tal_order_fill(order, out_trade_no, total_amount, subject, body, client_ip);
    auto [effect, lastid]     = order.save();
    tal_order_insrep(order, effect, lastid, err);
    if (lastid == 0) return 0;
    return order.data.orderid;
}

static asio::awaitable<unsigned int> tal_order_insert_async(const std::string &out_trade_no,
                                                            const std::string &total_amount,
                                                            const std::string &subject,
                                                            const std::string &body,
                                                            const std::string &client_ip,
                                                            std::string *err = nullptr)
{
    auto order                = orm::cms::Orderlist();
    tal_order_fill(order, out_trade_no, total_amount, subject, body, client_ip);
    auto [effect, lastid]     = co_await order.async_save();
    tal_order_insrep(order, effect, lastid, err);
    if (lastid == 0) co_return 0;
    co_return order.data.orderid;
}

// SELECT orderlist 渲染列表 HTML。取最近 20 条，addtime DESC。
// 库身份直接用 SQL 问服务器本身，回答"这个进程连的是哪个库"。
static asio::awaitable<std::string> tal_db_ident()
{
    std::vector<orm::cust::TalDbIdent> rows;
    auto link = std::make_unique<orm::db_conn>("cms");
    if (co_await link->async_query("SELECT DATABASE() AS db, VERSION() AS ver, "
                                   "(SELECT COUNT(*) FROM orderlist) AS cnt",
                                   rows) == 0
        || rows.empty())
    {
        co_return "取不到（[cms] 段连不上或 orderlist 不在这个库里）";
    }
    co_return "库名 [" + rows[0].db + "]  " + rows[0].ver + "  直连 COUNT(*) orderlist = " + rows[0].cnt;
}

// probe = false（默认）时只发列表那一条 SQL；三条诊断读数（全表 COUNT、不过滤最近 5 条、
// 问服务器身份）只在 ?probe=1 时发——订单表长起来之后全表 COUNT(*) 会越来越贵，
// tal_db_ident() 还要另取一条连接，不该由每次刷新列表页付这个钱。
static asio::awaitable<std::string> tal_order_html(bool probe)
{
    // 列表本身：唯一无条件发的一条。
    auto order = orm::cms::Orderlist();
    order.where("paytype", order_paytype::alipay);
    order.desc("addtime");
    order.limit(20);
    co_await order.async_fetch();
    bool  list_err  = order.iserror;
    std::string list_msg = order.error_msg;

    std::string diag;
    if (probe)
    {
        // 列表空着有三种可能，长得一模一样：库连不上/SQL 报错、库里真没行、过滤用的 paytype 列在
        // 线上库里不存在（ALTER 漏跑 ⇒ where("paytype",…) 置 iserror、fetch() 回 0 行）。
        // 先把不带渠道过滤的读数和 error_msg 打出来，再打那张表。
        auto cnt = orm::cms::Orderlist();
        unsigned long long total     = co_await cnt.async_count();
        bool probe_err               = cnt.iserror;
        std::string probe_msg        = cnt.error_msg;
        std::string probe_sql        = cnt.get_query();

        auto last = orm::cms::Orderlist();
        last.desc("addtime");
        last.limit(5);
        co_await last.async_fetch();
        bool  last_err = last.iserror;
        std::string last_msg = last.error_msg;
        std::string last_rows;
        for (auto &m : last.record)
        {
            if (!last_rows.empty()) last_rows += "<br>";
            last_rows += "orderid=" + std::to_string(m.orderid) + "  " + html_encode(m.wxorder)
                       + "  addtime=" + std::to_string(m.addtime) + "  paytype=" + std::to_string(m.paytype)
                       + "  status=" + std::to_string(m.status) + "  payprice=" + std::to_string(m.payprice);
        }

        // 服务器身份那一问要 co_await，所以先算好再拼这串
        const std::string dbident = html_encode(co_await tal_db_ident());

        diag = "<p>库自证：全表 count=" + std::to_string(total)
             + (probe_err ? " <b>失败</b> error_msg=[" + html_encode(probe_msg) + "]" : " 无报错")
             + "<br>用的语句=[" + html_encode(probe_sql) + "]  表名=" + html_encode(order.tablename)
             + "  库标签=" + html_encode(order.dbtag) + "<br>服务器身份：" + dbident
             + "<br>不过滤、按 addtime 倒序最近 5 条："
             + (last_rows.empty()
                    ? (last_err ? " <b>失败</b> error_msg=[" + html_encode(last_msg) + "]" : " 一条都没有")
                    : "<br>" + last_rows)
             + "<br>下面这张表用了 where(paytype=" + std::to_string((unsigned int)order_paytype::alipay) + ")："
             + (list_err ? " <b>失败</b> error_msg=[" + html_encode(list_msg) + "]" : " 无报错")
             + "</p>";
    }
    else if (order.record.empty())
    {
        // 默认视图也得留一条"列表为什么空"的提示，别让人不知道 ?probe=1 的存在。
        // 这两个读数来自上面那条查询本身，不额外发 SQL。
        diag = "<p>列表是空的。成因有三种、页面上长得一样：库连不上或 SQL 报错、库里真没有这一渠道的行、"
               "paytype 列不存在（过滤条件会让 iserror 置位、fetch() 回 0 行）。"
             + (list_err ? "<b>这一条查询已报错</b> error_msg=[" + html_encode(list_msg) + "]。" : "")
             + "加 <code>?probe=1</code> 看库自证（全表 count、不过滤的最近 5 条、服务器身份）。</p>";
    }

    static const char *status_map[] = { "待支付", "已支付", "已撤销", "已退款" };
    std::string h;
    h += "<h2>支付宝订单（orderlist 表，最近 20 条）</h2>";
    h += diag;
    h += "<table border=\"1\" cellpadding=\"4\"><tr>"
        "<th>orderid</th><th>时间</th><th>商户单号</th><th>支付宝交易号</th>"
        "<th>金额(分)</th><th>标题</th><th>状态</th><th>动作</th></tr>";

    for (auto &m : order.record)
    {
        std::string st = (m.status >= 0 && m.status <= 3) ? status_map[m.status] : "未知";
        std::string q  = tal_url_escape(m.wxorder);
        h += "<tr><td>" + std::to_string(m.orderid) + "</td>"
           + "<td>" + std::to_string(m.addtime) + "</td>"
           + "<td><code>" + html_encode(m.wxorder) + "</code></td>"
           + "<td>" + html_encode(m.storeorder) + "</td>"
           + "<td>" + std::to_string(m.payprice) + "</td>"
           + "<td>" + html_encode(m.paytitle) + "</td>"
           + "<td>" + st + "</td>"
           + "<td><a href=\"/testalipayquery?out_trade_no=" + q + "\">查单</a> "
           + "<a href=\"/testalipaycancel?out_trade_no=" + q + "\">取消</a></td></tr>";
    }
    h += "</table>";
    co_return h;
}

// 网关应答按包装键取字段，已下沉 libs/webpay/webpay_alipay.h（webpay::alipay_resp_field）。

// isv.invalid-signature 的临时诊断（?dbg=1 分支）：把这一次真发给网关的两串字节放在一起比对。
// 只回显随请求外发的内容（含签名值 sign），私钥字节不在两串里、也不打印。
// 输出三段各答一问：[3] 编码/还原丢不丢字节；[1] 的 hex 看中文是 UTF-8 还是 GBK；[5] 看应答是不是 GBK。
static std::string tal_dbg_report(const pay::alipay &apay, const std::string &response)
{
    const std::string &sign_content = apay.lastSignContent();
    const std::string &body         = apay.lastRequestBody();

    const char hx[] = "0123456789ABCDEF";
    auto hexbyte = [&](unsigned char c) -> std::string {
        std::string s;
        s += hx[c >> 4];
        s += hx[c & 0xF];
        return s;
    };
    auto nonascii = [&](const std::string &s) -> std::string {
        std::string out;
        for (std::size_t i = 0; i < s.size(); i++)
        {
            unsigned char c = (unsigned char)s[i];
            if (c < 0x80) continue;
            if (!out.empty()) out += " ";
            out += std::to_string(i) + ":" + hexbyte(c);
        }
        return out.empty() ? std::string("(无，整串纯 ASCII)") : out;
    };
    auto hexdump = [&](const std::string &s, std::size_t n) -> std::string {
        std::string out;
        for (std::size_t i = 0; i < n && i < s.size(); i++)
        {
            if (i) out += " ";
            out += hexbyte((unsigned char)s[i]);
        }
        return out;
    };
    auto xdigit = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    // application/x-www-form-urlencoded 的还原口径：%XX → 那一字节，'+' → 空格，其余照抄。
    auto form_decode = [&](const std::string &s) -> std::string {
        std::string out;
        for (std::size_t i = 0; i < s.size(); i++)
        {
            if (s[i] == '%' && i + 2 < s.size())
            {
                int h = xdigit(s[i + 1]), l = xdigit(s[i + 2]);
                if (h >= 0 && l >= 0)
                {
                    out += (char)(h * 16 + l);
                    i += 2;
                    continue;
                }
            }
            out += (s[i] == '+') ? ' ' : s[i];
        }
        return out;
    };

    // 拆开 body：sign= 这一段要摘掉再还原比对（待签名串里没有 sign），签名值本身留给 [4]。
    // 比的是 "sign=" 整键而不是前缀 "sign"，否则 sign_type= 会被一起摘掉。
    std::string body_wo_sign, sign_value;
    {
        std::size_t pos = 0;
        while (pos < body.size())
        {
            std::size_t amp = body.find('&', pos);
            std::string pair = (amp == std::string::npos) ? body.substr(pos) : body.substr(pos, amp - pos);
            pos = (amp == std::string::npos) ? body.size() : amp + 1;
            if (pair.compare(0, 5, "sign=") == 0)
            {
                sign_value = pair.substr(5);
                continue;
            }
            if (!body_wo_sign.empty()) body_wo_sign += "&";
            body_wo_sign += pair;
        }
    }
    const std::string decoded = form_decode(body_wo_sign);
    std::size_t same = 0;
    while (same < decoded.size() && same < sign_content.size() && decoded[same] == sign_content[same])
    {
        same++;
    }

    // 只回答"整段能不能按 UTF-8 解完"，解不动的位置带出来 —— 判网关是不是回了 GBK。
    std::size_t utf8_bad = std::string::npos;
    {
        std::size_t i = 0;
        while (i < response.size())
        {
            unsigned char c   = (unsigned char)response[i];
            std::size_t       need = 0;
            if (c < 0x80) need = 0;
            else if (c >= 0xC2 && c <= 0xDF) need = 1;
            else if (c >= 0xE0 && c <= 0xEF) need = 2;
            else if (c >= 0xF0 && c <= 0xF4) need = 3;
            else { utf8_bad = i; break; }
            for (std::size_t k = 1; k <= need; k++)
            {
                if (i + k >= response.size()) { utf8_bad = i; break; }
                unsigned char cc = (unsigned char)response[i + k];
                if (cc < 0x80 || cc > 0xBF) { utf8_bad = i; break; }
            }
            if (utf8_bad != std::string::npos) break;
            i += need + 1;
        }
    }

    std::string t = "=== 支付宝报文自证（临时诊断分支）===\n\n";
    t += "[1] 待签名串（原值，未经 percent 编码）len=" + std::to_string(sign_content.size()) + "\n";
    t += sign_content + "\n";
    t += "    非 ASCII 字节 偏移:hex = " + nonascii(sign_content) + "\n";
    t += "    期望：中文走 UTF-8。\"测试退款\" = E6B58B E8AF95 E98080 E6ACBE；对不上就是源码串被转码了。\n\n";

    t += "[2] 实际发出的 body（每个值已 percent 编码）len=" + std::to_string(body.size()) + "\n";
    t += body + "\n\n";

    t += "[3] 把 [2] 摘掉 sign= 一段、按 form 规则还原，与 [1] 逐字节比：还原串 len="
       + std::to_string(decoded.size()) + "\n";
    if (same == decoded.size() && same == sign_content.size())
    {
        t += "    结论：完全一致 ⇒ 编码/还原这一环不丢字节也不改字节，网关拿到的就是 [1] 这串。\n";
    }
    else
    {
        t += "    结论：不一致，首个差异在偏移 " + std::to_string(same) + "\n";
        std::size_t from = (same > 24) ? (same - 24) : 0;
        t += "      [1] 上下文 = " + sign_content.substr(from, 48) + "\n";
        t += "      [3] 上下文 = " + decoded.substr(from, 48) + "\n";
        t += "      [1] 差异字节 = "
           + (same < sign_content.size() ? hexbyte((unsigned char)sign_content[same]) : std::string("(串已尽)")) + "\n";
        t += "      [3] 差异字节 = "
           + (same < decoded.size() ? hexbyte((unsigned char)decoded[same]) : std::string("(串已尽)")) + "\n";
        t += "    还原串全文：\n" + decoded + "\n";
    }
    t += "\n";

    t += "[4] sign（签名值 base64，本来就随请求外发）len=" + std::to_string(sign_value.size()) + "\n";
    t += sign_value + "\n\n";

    t += "[5] 网关应答 len=" + std::to_string(response.size()) + " ；按 UTF-8 解码："
       + (utf8_bad == std::string::npos
              ? std::string("整段合法")
              : "偏移 " + std::to_string(utf8_bad) + " 处解不动 ⇒ 应答是 GBK，不是 UTF-8")
       + "\n";
    t += response + "\n";
    t += "    前 96 字节 hex = " + hexdump(response, 96) + "\n";
    if (utf8_bad != std::string::npos)
    {
        t += "    坏字节起 16 字节 hex = " + hexdump(response.substr(utf8_bad, 16), 16) + "\n";
    }
    return t;
}

// 这行字符串要打到页面上："没编 ENABLE_IMAGE"和"编了但网关没给 qr_code"两种失败的输出
// 外观一模一样，不写明编译状态就分不出来。
static const char *tal_img_state()
{
#ifdef ENABLE_IMAGE
    return "已编译";
#else
    return "未编译 ⇒ 这一版画不出二维码（SVG 和 PNG 都要它），只给一句话提示 + 扫码内容原文";
#endif
}

// createTradePage() 只拼一条跳转地址、不发网络，直接 output 出来浏览器里就是一串裸字符，
// 看着像"什么都没生成"。包一层可点的链接，原文同时留在下面。
// 列表那一问是库跳 ⇒ 这一层跟着走协程。
static asio::awaitable<std::string> tal_paypage(const std::string &pay_url, bool probe)
{
    const std::string esc = html_encode(pay_url);
    std::string html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                       "<title>支付宝页面支付</title></head><body>";
    html += "<p><a href=\"" + esc + "\">打开支付宝收银台</a></p>";
    html += "<p>页面支付（alipay.trade.page.pay）给的是跳转地址，二维码由支付宝那一侧出。";
    html += "本站自己画二维码的是另一条路由：<a href=\"/testalipayqrcode\">/testalipayqrcode</a>";
    html += "（alipay.trade.precreate，点下去会真的下一笔 0.01 元预下单）。</p>";
    html += "<pre>" + esc + "</pre>";
    html += "<p>下面这张表每刷新一次本页就多一行（这个页面本身就是一个下单动作）；"
            "只想看列表不去下单：<a href=\"/testalipayorders\">/testalipayorders</a>。</p>";
    html += co_await tal_order_html(probe);
    html += "</body></html>";
    co_return html;
}

// 收银台框进本页 + 一盏只读的状态灯。当面付（precreate）没签约 ⇒ 本站画不出二维码，
// 还能扫码的通路只剩 page.pay 的收银台，那就把它框过来。
// 现在有两个 caller 用同一副外壳：`/testalipayfront` 的前置模式档（带 qr_pay_mode，网关给的是
// 专为内嵌画的那一版二维码区）与 none 档（不发键，等于整页收银台硬塞进 iframe）。
// 尺寸与"实际发出去了什么"由调用点算好传进来（sent_note 必须已由调用方转义），外壳只管排版。
//
// 三条不能含混：
//   1) 灯报的是"支付宝那一侧怎么说"，不是"本地已落账"。查单说 TRADE_SUCCESS 也不代表库里那一行
//      已经是 status=1 —— 落账只有 /alipaynotify 和主动查单两条通路，这个页面对库一个字节都不写。
//   2) "嵌没嵌进去"不由代码宣称。iframe 的 onload 在"被拒之后浏览器自己画错误页"时同样触发，
//      所以它只报次数、不下结论，最终由人眼判；selfloc 那一行是被 top 接管的读数 ——
//      整页被接管时，它会连同页面一起消失。
//   3) 轮询有上限，且到点要明确说"未见到"。静默的灰灯会被读成"付好了"。
static std::string tal_paypage_embed(const std::string &pay_url, const std::string &out_trade_no,
                                     unsigned int oid, const std::string &insrep,
                                     const std::string &sent_note, const std::string &frame_style)
{
    const std::string esc_url = html_encode(pay_url);
    const std::string esc_no = html_encode(out_trade_no);

    std::string html;
    html += "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
            "<title>支付宝扫码支付（本站内嵌）</title></head><body>";
    html += "<h2>支付宝页面支付：收银台内嵌在本页</h2>";
    html += "<p>商户单号 <b id=\"orderno\" data-no=\"" + esc_no + "\">" + esc_no + "</b> "
            "<a href=\"/testalipayquery?out_trade_no=" + esc_no + "\">查单原文</a> "
            "<a href=\"/testalipayorders\">订单表</a></p>";
    html += "<p style=\"font-size:13px;color:#555\">" + sent_note + "</p>";
    html += "<p id=\"lamp\" style=\"padding:6px 10px;border:1px solid #99a;background:#eef\">"
            "状态灯：启动中</p>";
    // 两条退款链接默认不替你选：不带 reason 的走服务端默认的中文，带 REFUND-ASCII 的走 ASCII。
    // 当前传输形态下中文 reason 会被网关拒签，出问题时靠这一对对比定位。
    html += "<p id=\"after\" style=\"display:none\"><span id=\"tradeno\"></span><br>"
            "<a href=\"/testalipayrefund?out_trade_no=" + esc_no
            + "&amp;refund_amount=0.01\">退款（默认中文 reason）</a> "
            "<a href=\"/testalipayrefund?out_trade_no=" + esc_no
            + "&amp;refund_amount=0.01&amp;reason=REFUND-ASCII\">退款（ASCII reason）</a></p>";
    html += "<iframe id=\"cashier\" name=\"cashier\" src=\"" + esc_url + "\" style=\""
            + html_encode(frame_style) + "\"></iframe>";
    html += "<p style=\"color:#a33;font-size:13px\">上面这一格要是空白，或者整页被支付宝接管"
            "（地址栏变成 openapi.alipay.com）⇒ 那是拒嵌 / 反框架，把 J1、J2 的读数记下来就行，"
            "别去改代码找别的因。</p>";
    // load 事件区分不了"渲染成功"和"被拒后浏览器自己画的错误页"，所以它只报次数、单独一格；
    // 并且它随时可能在轮询之后才触发，写进灯里会把两条读数搅成一串。
    html += "<p id=\"frameload\" style=\"color:#888;font-size:12px\">iframe load 计数：0"
            "（只当辅助，\"嵌进去了\"这条永远由人眼判）</p>";
    html += "<p id=\"selfloc\" style=\"color:#888;font-size:12px\"></p>";
    html += "<p style=\"color:#888;font-size:12px\">"
            + (oid > 0 ? "orderlist.orderid=" + std::to_string(oid) + "（已落库）"
                       : "<b>存根写入失败</b>")
            + "<br>" + insrep + "</p>";

    html += R"JS(
<script>
(function () {
  var EVERY = 4000, LIMIT = 75;
  var lamp = document.getElementById('lamp');
  var frame = document.getElementById('cashier');
  var frameload = document.getElementById('frameload');
  var after = document.getElementById('after');
  var no = document.getElementById('orderno').getAttribute('data-no');
  var qurl = '/testalipayquery?out_trade_no=' + encodeURIComponent(no);
  var tries = 0, loaded = 0, timer = null;

  function set(bg, text) { lamp.style.background = bg; lamp.textContent = text; }
  function stop() { if (timer) { clearInterval(timer); timer = null; } }

  document.getElementById('selfloc').textContent =
      '本页仍是 ' + location.href + ' ⇒ 没被 top 接管（反框架读数：这一行在 = 没发生）';

  frame.addEventListener('load', function () {
    loaded++;
    frameload.textContent = 'iframe load 计数：' + loaded + '（只当辅助，"嵌进去了"这条永远由人眼判）';
  });

  async function poll() {
    tries++;
    var txt = '';
    try {
      var r = await fetch(qurl, { credentials: 'same-origin' });
      txt = await r.text();
    } catch (e) {
      if (tries >= LIMIT) { stop(); set('#fee', '轮询到 ' + LIMIT + ' 次，一次 fetch 都没成：' + e); }
      else set('#fee', '第 ' + tries + ' 次查单没发出去：' + e + '（继续）');
      return;
    }

    // 应答不是 JSON 就停：那是"线上跑的是旧二进制"这一层的问题，不是订单的问题。
    var j = null;
    try { j = JSON.parse(txt); }
    catch (e) {
      stop();
      set('#fee', '应答不是 JSON ⇒ 线上二进制还是旧的，或者网关那一跳挂了。原文前 200 字节：'
          + txt.slice(0, 200));
      return;
    }

    var resp = j.alipay_trade_query_response || j.error_response || j;
    var code = String(resp.code || '');
    var sub = String(resp.sub_code || '');
    var st = String(resp.trade_status || '');

    if (code === '10000' && (st === 'TRADE_SUCCESS' || st === 'TRADE_FINISHED')) {
      stop();
      set('#cfc', '支付宝说这笔已付：' + st + '。这不是"本地已落账"，那一格去 /testalipayorders 看。');
      document.getElementById('tradeno').textContent = '支付宝交易号 ' + String(resp.trade_no || '');
      after.style.display = '';
      return;
    }
    if (code === '10000') {
      set('#ffe', '网关有这笔单，状态 ' + st + '（还没付成，继续等）');
      return;
    }
    if (code === '40004' && sub === 'ACQ.TRADE_NOT_EXIST') {
      if (tries >= LIMIT) {
        stop();
        set('#eee', LIMIT + ' 次都没等到 —— 支付宝侧始终没有这笔交易 ⇒ 没扫、没付。');
        return;
      }
      set('#eee', '支付宝侧还没有这笔交易（没付款的 page.pay 就是这个形状），第 '
          + tries + '/' + LIMIT + ' 次');
      return;
    }
    stop();
    set('#fee', '查单回了 code=' + code + ' sub_code=' + sub
        + ' ⇒ 停止轮询（再打也不会自己变绿）。' + String(resp.sub_msg || ''));
  }

  poll();
  timer = setInterval(function () {
    poll();
    if (tries >= LIMIT) stop();
  }, EVERY);
})();
</script>
)JS";

    html += "</body></html>";
    return html;
}
#endif

// 诊断页自身放在 ENABLE_WEBPAY 之外：编译没开 webpay 也要能答上一句，
// 否则运维只能看到一个 404，分不清"二进制是旧的"还是"根本没编支付"。
//@urlpath(null,testalipaydiag)
std::string test_alipay_diag(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    peer->type("text/plain; charset=UTF-8");
#ifdef ENABLE_WEBPAY
    std::string out;
    auto say = [&out](const std::string &s) { out += s; out += '\n'; };

    say(std::string("=== 支付宝配置与密钥诊断 diag=20260922-1  二进制编译于 ") + __DATE__ + " " + __TIME__ + " ===");
    say("（零外发：本页只本地拼串+签名，不向支付宝或任何第三方发请求；不打印密钥内容）");

    webpay_config_t &cfg = get_webpay_config();
    say("");
    say(std::string("[1] 配置文件 ") + (cfg.is_load() ? "已加载" : "未加载!") + "  实际路径 " + cfg.file);

    // [1b] "多商户配置到底生效了哪一份"的唯一读数：tag 命中情况、应取的段、该段摘要一起打出来。
    // 域名→tag 只是诊断读数，业务路由（webpay::alipay_fill）取的是代码钉死的 kPayTag；
    // 要按业务隔离商户，改的是上面那行 kPayTag 并重新编译。
    const std::string peer_host = client.host;
    const std::string peer_tag  = cfg.host_tag(peer_host);
    say("");
    say("[1b] 域名 → 商户段（多域名 tag 化）");
    say("    Host        = " + (peer_host.empty() ? "(空，请求没带 Host)" : peer_host));
    say("    host_tag    = " + (peer_tag.empty() ? std::string("(无) → 用裸段") : peer_tag));
    const std::string want_sec = webpay_config_t::section_name("alipay", peer_tag);
    say("    应取的段    = [" + want_sec + "]  存在 = " + (cfg.has_section(want_sec) ? "是" : "否"));
    const webpay_merchant_t tagmch = cfg.alipay(peer_tag);
    say("    该段 appid 长度 = " + std::to_string(tagmch.appid.size())
        + "  mch_id 摘要 = " + (tagmch.found() ? md5(tagmch.mch_id).substr(0, 8) : "(段不存在)"));
    say("    kPayTag（业务路由钉死的 tag）= " + std::string(kPayTag) + "  ← 空串就是裸段");
    webpay_merchant_t mch = cfg.alipay(kPayTag);
    say("    业务路由实际用的段 = [" + std::string(mch.found() ? mch.section : "(无)") + "]");
    say("");
    say("[2] [" + std::string(mch.found() ? mch.section : "alipay（段不存在）") + "] 段读到什么");
    say("    appid 长度 " + std::to_string(mch.appid.size()) + " 字符  sandbox "
        + (mch.sandbox ? "1（沙箱网关）" : "0（正式网关）"));
    say("    notifyurl " + mch.notifyurl);
    say("    returnurl " + mch.returnurl);
    say("    api_aes 长度 " + std::to_string(mch.content_aes_key.size()) + " 字符（只报长度）");

    // 下面演示的是"业务路由实际那一段"里的原始写法，所以段名跟着 mch.section 走
    const std::string use_sec = mch.section.empty() ? std::string("alipay") : mch.section;
    const std::string rawkey  = cfg.get(use_sec, "key_file");
    const std::string rawpub  = cfg.get(use_sec, "public_key_file");
    say("");
    say("[3] 相对路径怎么拼（前三档拿原值拼：配置上级 / 配置目录 / 进程 cwd；第四档只拿文件名去配置目录）");
    std::error_code ec;
    std::filesystem::path confabs = std::filesystem::absolute(std::filesystem::path(cfg.file), ec);
    const std::filesystem::path bases[3] = {confabs.parent_path().parent_path(), confabs.parent_path(),
                                           std::filesystem::current_path(ec)};
    const char *bname[3] = {"项目根  ", "配置目录", "cwd     "};
    for (int i = 0; i < 3; i++)
    {
        std::string hit = "未命中";
        ec.clear();
        if (std::filesystem::is_regular_file(bases[i] / rawkey, ec)) hit = "命中";
        say("    " + std::string(bname[i]) + " = " + bases[i].string() + "   该基准 + key_file ⇒ " + hit);
    }
    say("    配置里写的 key_file        = [" + rawkey + "]");
    say("    配置里写的 public_key_file = [" + rawpub + "]");
    say("    get_merchant 给出的 key_file（规整后）= " + mch.key_file);
    // 四档：前三档拼原值，第四档只拿文件名去配置目录找（密钥与 webpay.conf 平级的那种部署）
    {
        std::filesystem::path only(rawkey);
        std::filesystem::path fourth = confabs.parent_path() / only.filename();
        ec.clear();
        say("    第四档   配置目录 + 仅文件名 = " + fourth.string() + "   "
            + (std::filesystem::is_regular_file(fourth, ec) ? "命中" : "未命中"));
    }

    pay::alipay apay;
    const tald_keyform kf = tald_inspect(mch.key_file, true);
    const tald_keyform pf = tald_inspect(mch.public_key_file, false);
    say("");
    say("[4] 两个密钥字段的形态与读取结果（三态：PEM 文本 / PEM 文件 / 裸 base64）");
    const tald_keyform *kv[2] = {&kf, &pf};
    const char *kname[2]      = {"key_file（应用私钥）    ", "public_key_file（支付宝公钥）"};
    for (int i = 0; i < 2; i++)
    {
        const tald_keyform &k = *kv[i];
        say("    " + std::string(kname[i]));
        say("      常规文件 " + tald_yesno(k.isfile) + "  目录 " + tald_yesno(k.isdir)
            + "  字节 " + std::to_string(k.bytes) + "  形态 " + k.form);
        say("      交付代码能不能读成密钥对象：" + tald_yesno(k.readable));
    }

    std::string cfgmsg;
    bool filled = webpay::alipay_fill(apay, kPayTag, cfgmsg);
    say("");
    say("[5] webpay::alipay_fill（各支付路由取配置的那个函数）" + std::string(filled ? "通过" : "拒绝：" + cfgmsg));

    apay.setAppId(mch.appid);
    apay.setPrivateKey(mch.key_file);
    apay.setPublicKey(mch.public_key_file);
    apay.setNotifyUrl(mch.notifyurl);
    apay.setReturnUrl(mch.returnurl);
    apay.setSandbox(mch.sandbox);
    apay.setOutTradeNo("DIAG" + get_date("%Y%m%d%H%M%S"));
    apay.setSubject("诊断用，不产生订单");
    apay.setTotalAmount("0.01");
    const std::string order = apay.createTradePage();
    std::string signval;
    std::size_t spos = order.find("sign=");
    if (spos != std::string::npos)
    {
        std::size_t epos = order.find('&', spos);
        signval = order.substr(spos + 5, epos == std::string::npos ? std::string::npos : epos - spos - 5);
    }
    say("    createTradePage() 整串 " + std::to_string(order.size()) + " 字节  sign 参数 "
        + std::to_string(signval.size()) + " 字符 ⇒ " + tald_yesno(!signval.empty()));

    say("");
    say("[6] 结论");
    if (rawkey.empty())
        say("    key_file 在配置里就是空的 ⇒ 先把它填上（文件路径 / PEM 文本 / 裸 base64 都行）。");
    else if (kf.isdir)
        say("    key_file 指向的是目录 ⇒ 改成密钥文件本身，程序不会去目录里猜哪个是私钥。");
    else if (!kf.isfile && !kf.pemtext)
        say("    key_file 既不是盘上现成的文件、也不是 PEM 文本 ⇒ 值写错了，或者文件没上到这台机器。"
            "对照 [3] 三个基准，哪个都没命中就把 key_file 改成绝对路径。");
    else if (kf.bare && !kf.readable)
        say("    文件在、是裸 base64，但仍读不成私钥 ⇒ 这份二进制的 alipay_read_key 还不认裸 base64"
            "（三态读取 2026-09-19 12:38 才落地）。要么用今天的源码重编重部署，"
            "要么临时把 key_file 指回同目录那个 PEM。");
    else if (!kf.readable)
        say("    文件在、也不是裸 base64，仍然读不出来 ⇒ 文件内容不是能用的 RSA 私钥（或已被截断/改坏），"
            "把 [4] 那两行原样回给我。");
    else if (signval.empty())
        say("    私钥明明读得出来却仍签出空 sign ⇒ 不是密钥读取环节，把整页原样贴回来。");
    else
        say("    私钥读得出、签名非空 ⇒ 这台机器此刻没问题。若支付仍失败，问题在签名之外"
            "（网关侧公钥不一致、内容加密、回调地址可达性）。");
    say("");
    say("判据补充：这一页能打开 = 线上跑的已经是带 testalipaydiag 的这一版；"
        "404 就是旧二进制，跟签名对不对无关。");
    client.output = out;
#else
    client.output = "ENABLE_WEBPAY not enabled\r\n";
#endif
    return "";
}

//@urlpath(null,testalipayapp)
asio::awaitable<std::string> test_alipay_app(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::alipay apay;
    std::string cfgmsg;
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg))
    {
        client.output = cfgmsg;
        co_return "";
    }

    apay.setOutTradeNo(out_trade_no);
    apay.setSubject("测试商品");
    apay.setBody("测试商品描述");
    apay.setTotalAmount("0.01");
    apay.setProductCode("QUICK_MSECURITY_PAY");

    // 落库排在交付之前：库里没这一行就不把 orderStr 交出去（付成功也找不到账，回调一直 FAIL）。
    // createTrade 只在本地签名、不外发，所以这一路的"外发"就是这一串交出去的那一刻。
    std::string insrep;
    unsigned int oid = co_await tal_order_insert_async(out_trade_no, "0.01", "测试商品", "测试商品描述",
                                                       client.client_ip, &insrep);
    if (oid == 0)
    {
        // 这一路对客户端是 orderStr 原样透传，往里塞 HTML 会把 SDK 的解析带坏 ⇒ 闸门回 JSON。
        // insrep 那行是 HTML 片段，不进 JSON；要看它去 /testalipayorders 一侧。
        client.val["code"] = -1;
        client.val["msg"]  = "order not stored: cms.orderlist save returned no id";
        client.out_json();
        co_return "";
    }
    std::string pay_params = apay.createTrade();
    // orderStr 是客户端 SDK 原样送网关的载荷，尾部多一个字符就不是同一份交付串了。
    // 要对着单号看：/testalipayorders（列表页按 out_trade_no 能找到这一单）。
    client.output = pay_params;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    co_return "";
}

//@urlpath(null,testalipayqrcode)
std::string test_alipay_qrcode(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // 不设类型就没有 Content-Type，浏览器会把 <img …> 当纯文本显示 ⇒ "有图"看着像"没图"
    client.type("text/html; charset=UTF-8");
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::alipay apay;
    std::string cfgmsg;
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg))
    {
        client.output = cfgmsg;
        return "";
    }

    apay.setOutTradeNo(out_trade_no);
    apay.setSubject("测试商品");
    apay.setTotalAmount("0.01");

    std::string insrep;
    unsigned int oid = tal_order_insert(out_trade_no, "0.01", "测试商品", "", client.client_ip, &insrep);

    // "网关没给 qr_code"和"这版没编图像支持"输出同形，这两行不分分支先打出来
    client.output  = std::string("<p>ENABLE_IMAGE = ") + tal_img_state() + "</p>";
    client.output += "<p>out_trade_no = " + out_trade_no
                   + "  <a href=\"/testalipayquery?out_trade_no=" + out_trade_no + "\">查单</a>"
                   + "  <a href=\"/testalipaycancel?out_trade_no=" + out_trade_no + "\">撤单</a></p>";
    client.output += "<p style=\"font-size:12px\">"
                   + (oid > 0 ? "orderlist.orderid = " + std::to_string(oid) : "<b>存根写入失败</b>") + "<br>"
                   + insrep + "</p>";

    // 落库排在下单之前：precreate 是真下一笔 0.01 元的单，库里没这一行就一个字节都不许出网
    if (oid == 0)
    {
        client.output += "<p class=\"err\">闸门：存根没写进 cms.orderlist，本次没有向支付宝发出 precreate。</p>";
        return "";
    }
    std::string response = apay.createTradeQRCode();

    // 网关应答包了一层：{"alipay_trade_precreate_response":{"qr_code":…}}，从顶层取永远取不到
    std::string qrcontent = webpay::alipay_resp_field(response, "alipay_trade_precreate_response", "qr_code");

    if (qrcontent.empty())
    {
        client.output += "<pre>" + html_encode(response) + "</pre>";
        // 没拿到 qr_code ⇒ 这一单在支付宝那一侧也扫不出东西，把刚插的待付行撤掉，别留孤儿单
        if (!webpay::order_cancel_by_wxorder(out_trade_no))
            client.output += "<p class=\"warn\">撤单未生效（cms.orderlist 里这一行 status 已不是 0），需要手工清理。</p>";
    }
    else
    {
        // 二维码走内联 SVG，不落盘。文件名必须按单号隔离：固定名会让并发下单互相覆盖图片，
        // 谁先扫谁付钱 ⇒ 资金错配，不是显示瑕疵。
        std::string qrhtml = webpay::qr_svg(qrcontent);
        if (!qrhtml.empty())
            client.output += "<div>" + qrhtml + "</div>";
        else
            client.output += "<p>本二进制没编 ENABLE_IMAGE，画不出二维码。扫码内容：" + html_encode(qrcontent) + "</p>";
        client.output += "<p>二维码内容：" + html_encode(qrcontent) + "</p>";

        // ===== PNG 落盘通路：能力保留，默认关闭 =====
        // 当面付要打印纸质码时 SVG 用不了，所以这条路留着；启用 = 把 libs/webpay/webpay_qr.h
        // 里的 k_qr_png_enabled 改成 true 重新编译。文件名按单号隔离是启用它的前提。
        if constexpr (webpay::k_qr_png_enabled)
        {
            if (webpay::qr_png_save(client.get_sitepath(), out_trade_no, qrcontent))
                client.output += "<img src=\"/upload/qr_" + html_encode(out_trade_no) + ".png\">";
            else
                client.output += "<p>PNG 没能写进 " + html_encode(webpay::qr_png_path(client.get_sitepath(), out_trade_no))
                               + " ⇒ 这是落盘失败（没编 ENABLE_IMAGE 或磁盘/权限问题），不是网关没给码</p>";
        }
    }
#else
    client.output = "本二进制编译时没开 ENABLE_WEBPAY，支付路由不产出。";
#endif
    return "";
}

//@urlpath(null,testalipaypage)
asio::awaitable<std::string> test_alipay_page(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::alipay apay;
    std::string cfgmsg;
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg))
    {
        client.output = cfgmsg;
        co_return "";
    }

    apay.setOutTradeNo(out_trade_no);
    apay.setSubject("测试商品");
    apay.setBody("测试商品描述");
    apay.setTotalAmount("0.01");

    // 落库排在交付之前：收银台链接交出去之前，库里必须已经有这一行（付成功要能对账）。
    // createTradePage 只在本地拼 URL + 签名、不外发，所以挪到落库之后不影响任何字节。
    std::string insrep;
    unsigned int oid = co_await tal_order_insert_async(out_trade_no, "0.01", "测试商品", "测试商品描述",
                                                       client.client_ip, &insrep);
    client.type("text/html; charset=UTF-8");
    if (oid == 0)
    {
        // 闸门页也带 insrep：这行的存在本身就说明 INSERT 这条路跑过了。
        // 不放收银台链接 —— 放了就是"钱能付、账没处对"。
        client.output = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                        "<title>支付宝页面支付：未下单</title></head><body>"
                        "<p><b>存根写入失败</b>：订单没写进 cms.orderlist，这一页没有给出收银台链接。</p>"
                        "<p style=\"font-size:12px\">" + insrep + "</p></body></html>";
        co_return "";
    }
    std::string pay_url = apay.createTradePage();
    client.output = co_await tal_paypage(pay_url, client.get["probe"].to_int() != 0);
    {
        // tal_paypage 已拼完整 </body></html>，在 body 关闭前插一行说明。
        // 成败都打：这行的存在本身就说明 INSERT 这条路跑过了。
        std::string note = std::string("<p style=\"color:#888;font-size:12px\">")
                         + "orderlist.orderid=" + std::to_string(oid) + "（已落库）"
                         + "<br>" + insrep + "</p>";
        std::string html = client.output;
        auto pos = html.find("</body>");
        if (pos != std::string::npos)
            html.insert(pos, note);
        client.output = html;
    }
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    co_return "";
}

// 电脑网站支付的「前置模式」：告诉网关"这一版页面我要嵌在自己页里"，让它吐二维码区而不是
// 整页收银台。模式编号语义官方文档页取不到 ⇒ 以网关行为为准，mode 原样透传不校验，
// 并把实际出网的串回显在页面上——"发没发这个键"不该靠翻代码确认。
//   /testalipayfront                  默认档（mode=4 & width 由网关自己定）
//   /testalipayfront?mode=0|1|3|4     四档对照
//   /testalipayfront?mode=4&width=300 模式 4 的宽度
//   /testalipayfront?mode=none        对照档：一个键都不发
//@urlpath(null,testalipayfront)
asio::awaitable<std::string> test_alipay_front(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("text/html; charset=UTF-8");
#ifdef ENABLE_WEBPAY
    std::string mode  = client.get["mode"].to_string();
    std::string width = client.get["width"].to_string();
    if (mode.empty()) mode = "4";

    const bool front = (mode != "none");
    if (!front) width.clear();

    std::string out_trade_no = get_date("%Y%m%d%H%M%S") + rand_string(4, 4);

    pay::alipay apay;
    std::string cfgmsg;
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg))
    {
        client.output = cfgmsg;
        co_return "";
    }
    apay.setOutTradeNo(out_trade_no);
    apay.setSubject("测试商品");
    apay.setBody("测试商品描述");
    apay.setTotalAmount("0.01");
    if (front)
    {
        apay.setQrPayMode(mode);
        if (!width.empty()) apay.setQrcodeWidth(width);
    }

    std::string pay_url = apay.createTradePage();
    // buildRequest() 把出网那一串（值已 percent 编码）留在 last_request_body_ 里；
    // qr_pay_mode 这类键名本身是 ASCII，编码前后长得一样，所以直接按字面找就是"发没发"的读数。
    const std::string sent = apay.lastRequestBody();
    const bool sent_mode  = (sent.find("qr_pay_mode") != std::string::npos);
    const bool sent_width = (sent.find("qrcode_width") != std::string::npos);

    std::string sent_note = "前置模式档 " + html_encode(mode)
                          + "  qrcode_width " + html_encode(width.empty() ? "(不发)" : width)
                          + "<br>出网串里含 <b>qr_pay_mode</b>：" + (sent_mode ? "是" : "<b>否</b>")
                          + "  含 <b>qrcode_width</b>：" + (sent_width ? "是" : "否")
                          + "  串长 " + std::to_string(sent.size()) + " 字节"
                          + "<details><summary>出网那一串原文（gateway.do? 之后，含 sign）</summary><pre>"
                          + html_encode(sent) + "</pre></details>";

    std::string insrep;
    unsigned int oid = co_await tal_order_insert_async(out_trade_no, "0.01", "测试商品", "测试商品描述", client.client_ip, &insrep);

    // 高度按档给：前置模式那一版只有二维码区，780px 会剩一大片空白，看着像"没渲染出来"；
    // none 档装的是整页收银台，得留够高度。
    std::string frame_style = "width:100%;height:" + std::string(front ? "520px" : "780px")
                            + ";border:1px solid #bbb";

    if (oid == 0)
    {
        // 闸门：库里没这一行 ⇒ 不内嵌收银台（那一格里钱付出去就没账可对）。
        // 出网串读数照打：它是本地签名的结果，跟库连不连得上无关。
        client.output = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                        "<title>支付宝前置模式：未下单</title></head><body>"
                        "<p><b>存根写入失败</b>：订单没写进 cms.orderlist，这一页没有内嵌收银台。</p>"
                        "<p style=\"font-size:13px\">" + sent_note + "</p>"
                        "<p style=\"font-size:12px\">" + insrep + "</p></body></html>";
        co_return "";
    }
    client.output = tal_paypage_embed(pay_url, out_trade_no, oid, insrep, sent_note, frame_style);
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    co_return "";
}

// 只看订单列表，不下单
//@urlpath(null,testalipayorders)
asio::awaitable<std::string> test_alipay_orders(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("text/html; charset=UTF-8");
#ifdef ENABLE_WEBPAY
    client.output = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>支付宝订单</title></head><body>" +
                    co_await tal_order_html(client.get["probe"].to_int() != 0) +
                    "<p><a href=\"/testalipaypage\">去下一单</a></p></body></html>";
#else
    client.output = "本二进制编译时没开 ENABLE_WEBPAY，支付路由不产出。";
#endif
    co_return "";
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
    std::string cfgmsg;
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg))
    {
        client.output = cfgmsg;
        return "";
    }

    std::string response = apay.queryTrade(out_trade_no);
    // 临时诊断分支（?dbg=1）：query 的 biz_content 是纯 ASCII，与 refund 的报文对照着看。
    if (client.get["dbg"].to_string() == "1")
    {
        client.type("text/plain; charset=utf-8");
        client.output = tal_dbg_report(apay, response);
        return "";
    }
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

// 上面那条的协程版：唯一区别是网关那一跳换成 co_await async_query_trade()。
// 应答体与同步版保持一致（不追加说明文字），接口约定就是两版共用同一份判定。
// 想确认走的是异步通路，看 URL 本身（/testalipayaquery）。
//@urlpath(null,testalipayaquery)
asio::awaitable<std::string> test_alipay_aquery(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = client.get["out_trade_no"].to_string();

    if (out_trade_no.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少out_trade_no\"}";
        co_return "";
    }

    pay::alipay apay;
    std::string cfgmsg;
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg))
    {
        client.output = cfgmsg;
        co_return "";
    }

    client.output = co_await apay.async_query_trade(out_trade_no);
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    co_return "";
}

//@urlpath(null,testalipaycancel)
std::string test_alipay_cancel(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // 应答是网关的 JSON 原文，不给 Content-Type 浏览器会当纯文本
    client.type("application/json; charset=UTF-8");
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = client.get["out_trade_no"].to_string();

    if (out_trade_no.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少out_trade_no\"}";
        return "";
    }

    // 撤单闸门：支付宝 trade.cancel 对**已付款**的交易实际执行的是退款，而且照样回 code=10000。
    // 不在本地先判状态，就会出现钱退了、账上写 status=2 已撤销、isrefund 却不置位。
    // 所以 status!=0 一律拒撤，已付款的单走 /testalipayrefund。
    webpay::order_ref o;
    if (!webpay::order_find(out_trade_no, o))
    {
        client.output = "{\"code\":\"-1\",\"msg\":\"order not found\"}";
        return "";
    }
    if (o.status != order_status::pending)
    {
        client.output = "{\"code\":\"-1\",\"msg\":\"status=" + std::to_string(o.status)
                      + " 不是待支付，不能撤单；已付款的请走退款\"}";
        return "";
    }

    pay::alipay apay;
    std::string cfgmsg;
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg))
    {
        client.output = cfgmsg;
        return "";
    }

    std::string response = apay.cancelTrade(out_trade_no);
    // 列表里的状态由这一行的 code 决定，所以取的是应答里那个包装键下面的字段，不是页面文案
    const std::string code = webpay::alipay_resp_field(response, "alipay_trade_cancel_response", "code");
    if (code == "10000")
    {
        // 0→2 是一条 CAS；返回 false 只意味着这一行在应答期间被回调改走了，
        // 应答体是网关 JSON 原文、不容追加字段，真实状态看订单列表。
        webpay::order_set_cancelled(o);
    }
    // 撤单失败不 UPDATE，订单保持 status=0 供后续重试；失败原因（sub_code/sub_msg）本来就在
    // 下面这份网关原文里，不再另外抄一遍
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
    client.type("application/json; charset=UTF-8");
#ifdef ENABLE_WEBPAY
    std::string out_trade_no = client.get["out_trade_no"].to_string();
    std::string refund_amount = client.get["refund_amount"].to_string();

    if (out_trade_no.empty()) {
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少out_trade_no\"}";
        return "";
    }
    if (refund_amount.empty()) {
        // 支付宝这里不给"漏参数＝全额退"留口子：这条路由是机器调的接口，参数缺失该报错，
        // 而不是替调用方退一整单。V2/V3/小程序那三个页面路由走 resolve_refund_fen 的 0＝全额。
        client.output = "{\"code\":\"-1\",\"msg\":\"缺少refund_amount\"}";
        return "";
    }

    // 退款闸门：status 必须已经被异步通知推进到 1（已支付）。0=待付时钱根本没进来，
    // 2/3/9 是终态 ⇒ 一笔订单只允许退一次，要分次退得拆单。金额按分封顶到原单 payprice。
    webpay::order_ref o;
    if (!webpay::order_find(out_trade_no, o))
    {
        client.output = "{\"code\":\"-1\",\"msg\":\"order not found\"}";
        return "";
    }
    unsigned int rfen = yuan_to_fen(refund_amount);
    std::string why;
    if (!webpay::can_refund(o, rfen, why))
    {
        client.output = "{\"code\":\"-1\",\"msg\":\"" + why + "\"}";
        return "";
    }

    pay::alipay apay;
    std::string cfgmsg;
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg))
    {
        client.output = cfgmsg;
        return "";
    }

    // reason 是可覆盖的排障参数：同一套密钥、同一个通道，只把中文换成 ASCII，
    // 验签过了就说明非 ASCII 字节是问题所在。
    std::string reason = client.get["reason"].to_string();
    if (reason.empty()) reason = "测试退款";
    // 幂等键：号里的 refundnum+1 与 order_set_refunded 那次自增同口径，同一次退款的两路
    // 重投拿到同一个号，网关只退一次。缺了它，本地 CAS 只保护账、不保护外发这一跳。
    apay.setOutRequestNo(webpay::refund_no_gen(o));
    std::string response = apay.refundTrade(refund_amount, reason, out_trade_no);
    // 临时诊断分支：真发一次之后再打，绝不"只预览不发"（预览的串和真发的串可能不同形）。
    // 放在落账之前：40002 这一支本来就不该落账。
    if (client.get["dbg"].to_string() == "1")
    {
        client.type("text/plain; charset=utf-8");
        client.output = tal_dbg_report(apay, response);
        return "";
    }
    const std::string code = webpay::alipay_resp_field(response, "alipay_trade_refund_response", "code");
    if (code == "10000")
    {
        // 1→3 是 CAS，refundnum 自增；应答体是网关 JSON 原文，落账结果看订单列表。
        webpay::order_set_refunded(o);
    }
    client.output = response;
#else
    client.output = "{\"code\":\"-1\",\"msg\":\"ENABLE_WEBPAY not enabled\"}";
#endif
    return "";
}

//@urlpath(null,alipaynotify)
asio::awaitable<std::string> test_alipay_notify(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("text/plain; charset=utf-8");
#ifdef ENABLE_WEBPAY
    pay::alipay apay;
    std::string cfgmsg;
    // 回调只验签、不签报文：不要求应用私钥可用，否则私钥文件一坏，“钱已收”的单子再也落不了账
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg, false))
    {
        client.output = "fail";
        co_return "";
    }

    auto resp = apay.handleNotify(client.rawcontent);

    // 验签不过 → fail，一个字段都不采信。
    if (resp["status_code"] != "0")
    {
        client.output = "fail";
        co_return "";
    }

    // 归属校验：验签过只说明"支付宝签的"，不说明"发给本站的"（多商户配置下 A 商户的通知
    // 拿到 B 商户的公钥上照样验得过）。支付宝报文里能区分收款方的只有 app_id。
    // 放在 trade_status 之前：不是本站的单子，连"要不要收下"都不该由我们来判。
    // 这里不答 why（应答体只会被当成"没送达"而重投，写什么都一样），原因回显在 /alipayreturn。
    std::string ownwhy;
    if (!webpay::notify_match_merchant({resp["app_id"], ""}, {apay.getAppId(), ""}, ownwhy))
    {
        client.output = "fail";
        co_return "";
    }

    // 验签过 → 只有 TRADE_SUCCESS / TRADE_FINISHED 才算真正落账。
    // 其余状态（WAIT_BUYER_PAY、TRADE_CLOSED）回 success 收下，不改库，
    // 否则支付宝会一直重发这条它认为没送达的通知。
    const std::string &trade_status = resp["trade_status"];
    if (trade_status != "TRADE_SUCCESS" && trade_status != "TRADE_FINISHED")
    {
        client.output = "success";
        co_return "";
    }

    // 查单 → 金额逐分比对 → 0→1 CAS：三步与三条微信通知路由共用 webpay::settle_notify()。
    // 支付宝金额是元、要 yuan_to_fen 换成分，trade_no 顺手写进 storeorder。
    // 应答只有两个词：success 停手、fail 重投；漏单与金额不符都答 fail，绝不在金额不明时置已支付。
    auto st = co_await webpay::settle_notify(resp["out_trade_no"],
                                             yuan_to_fen(resp["total_amount"]), resp["trade_no"]);
    client.output = st.ok() ? "success" : "fail";
#else
    client.output = "fail";
#endif
    co_return "";
}

//@urlpath(null,alipayreturn)
asio::awaitable<std::string> test_alipay_return(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.type("text/html; charset=UTF-8");
#ifdef ENABLE_WEBPAY
    std::string query_str = client.querystring;

    pay::alipay apay;
    std::string cfgmsg;
    // 回调只验签、不签报文：不要求应用私钥可用，否则私钥文件一坏，“钱已收”的单子再也落不了账
    if (!webpay::alipay_fill(apay, kPayTag, cfgmsg, false))
    {
        client << "<h1>支付宝未配置</h1><p>" << cfgmsg << "</p>";
        co_return "";
    }

    auto resp = apay.handleReturn(query_str);

    // 这一跳带回来的参数原样回显：判"支付宝给了什么"不必去翻日志，页面上就能对。
    client << "<pre>" << html_encode(query_str) << "</pre>";

    if (resp["status_code"] == "0") {
        // 归属校验：回跳的参数表里同样带 app_id。这一跳本来不落账，但它会补 storeorder，
        // 所以也要闸 —— 不是本站的单，那个 trade_no 不该写进我们的行。
        // 这支同时承担 /alipaynotify 那侧看不见的诊断信息：那边只能答 fail，这里能打出是哪一字段不符。
        std::string ownwhy;
        if (!webpay::notify_match_merchant({resp["app_id"], ""}, {apay.getAppId(), ""}, ownwhy))
        {
            client << "<h1>验签通过，但这条不是发给本站的</h1>";
            client << "<p>" << html_encode(ownwhy) << "，本页不落库。</p>";
            client << "<p>报文里的 app_id：<code>" << html_encode(resp["app_id"]) << "</code></p>";
            client << "<p>验签只证明报文是支付宝签的，不证明它是发给本站的；两边的 app_id 请各自核对"
                      "<code>conf/webpay.conf</code> 的 [alipay] 段（线上实际加载的那一份见 "
                      "<code>/testalipaydiag</code>）。</p>";
            co_return "";
        }
        client << "<h1>回跳已验签</h1>";
        client << "<p>订单号: " << html_encode(resp["out_trade_no"]) << "</p>";
        client << "<p>支付宝交易号: " << html_encode(resp["trade_no"]) << "</p>";
        client << "<p>金额: " << html_encode(resp["total_amount"]) << "</p>";
        // 只有验签过的那一支才把字段打进页面，其余字段一律不采信（红支路的值是谁递的都说不清）。
        client << "<p>这一跳的参数表里没有 <code>trade_status</code>，所以它不作落账依据："
                  "订单结论看 <code>/alipaynotify</code> 那一条，或去 <code>/testalipayquery</code> 主动查单。</p>";
        // 可选：把 trade_no 补到 storeorder，不碰 status（同步回跳没有 trade_status，不作落账依据）。
        if (!resp["trade_no"].empty())
        {
            webpay::order_ref o;
            if (co_await webpay::async_order_find(resp["out_trade_no"], o))
            {
                co_await webpay::async_order_set_storeorder(o, resp["trade_no"]);
            }
        }
    } else {
        client << "<h1>验签失败</h1>";
        client << "<p>" << html_encode(resp["error_msg"]) << "</p>";
        client << "<p>验签失败的请求不落库，建议用上面那串 querystring 配合支付宝工具包在本地重现。</p>";
    }
#else
    client << "<h1>ENABLE_WEBPAY not enabled</h1>";
#endif
    co_return "";
}

}
