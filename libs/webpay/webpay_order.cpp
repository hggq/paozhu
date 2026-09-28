// 闸门口径与头文件一致：这一层不受 ENABLE_WEBPAY 管辖。
// 只用到 orm 与 func/datetime，不碰 vendor/webpay，所以 OFF 档链接不出问题。

#include <string>
#include "webpay_order.h"
#include "func.h"
#include "datetime.h"

namespace http
{
namespace webpay
{

std::string make_storeorder()
{
    return "SO" + std::to_string(timeid()) + rand_string(6, 0);
}

std::string make_wxorder(const std::string &prefix)
{
    return prefix + std::to_string(timeid()) + rand_string(6, 0);
}

std::string id_safe(const std::string &s)
{
    std::string out;
    for (char c : s)
    {
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-')
        {
            out.push_back(c);
        }
    }
    return out;
}

order_ref order_ref_of(const orm::cms::Orderlist &m)
{
    order_ref r;
    const auto &o = m.data;
    // fetch_one() 查无此行会把 data 复位成全零，orderid != 0 即命中。
    r.found     = o.orderid != 0;
    if (!r.found) return r;
    r.oid       = o.orderid;
    r.payprice  = o.payprice;
    r.refundnum = o.refundnum;
    r.status    = o.status;
    r.wxorder   = o.wxorder;
    return r;
}

bool order_find(const std::string &wxorder, order_ref &out)
{
    orm::cms::Orderlist m;
    m.where("wxorder", wxorder);
    m.fetch_one();
    out = order_ref_of(m);
    return out.found;
}

asio::awaitable<bool> async_order_find(const std::string &wxorder, order_ref &out)
{
    orm::cms::Orderlist m;
    m.where("wxorder", wxorder);
    co_await m.async_fetch_one();
    out = order_ref_of(m);
    co_return out.found;
}

long long order_insert(unsigned char paytype,
                       const std::string &wxorder,
                       const std::string &storeorder,
                       const std::string &openid,
                       const std::string &paytitle,
                       unsigned int payprice)
{
    orm::cms::Orderlist order;
    order.setPaytype(paytype);
    order.setUserid(0);
    order.setWxorder(wxorder);
    order.setStoreorder(storeorder.empty() ? make_storeorder() : storeorder);
    order.setOpenid(openid);
    order.setPaytitle(paytitle);
    order.setPayprice(payprice);
    order.setTotalnum(1);
    order.setAddtime((unsigned int)timeid());
    order.setPaytime(0);
    order.setIsrefund(0);
    order.setRefundnum(0);
    order.setStatus(order_status::pending);
    auto [effect, last_id] = order.save();
    (void)effect;
    return (long long)last_id;
}

asio::awaitable<long long> async_order_insert(unsigned char paytype,
                                              const std::string &wxorder,
                                              const std::string &storeorder,
                                              const std::string &openid,
                                              const std::string &paytitle,
                                              unsigned int payprice)
{
    orm::cms::Orderlist order;
    order.setPaytype(paytype);
    order.setUserid(0);
    order.setWxorder(wxorder);
    order.setStoreorder(storeorder.empty() ? make_storeorder() : storeorder);
    order.setOpenid(openid);
    order.setPaytitle(paytitle);
    order.setPayprice(payprice);
    order.setTotalnum(1);
    order.setAddtime((unsigned int)timeid());
    order.setPaytime(0);
    order.setIsrefund(0);
    order.setRefundnum(0);
    order.setStatus(order_status::pending);
    auto [effect, last_id] = co_await order.async_save();
    (void)effect;
    co_return (long long)last_id;
}

// 状态跃迁全走 CAS：UPDATE WHERE wxorder + 期望原状态。并发重投只有一路能命中。
// 只写 fields 列出的列，避免带 PK 的全列 save() 覆盖读改窗口里别人改过的字段。
bool order_set_paid(const order_ref &o, const std::string &trade_no)
{
    orm::cms::Orderlist m;
    m.where("wxorder", o.wxorder).where("status", order_status::pending);
    std::string fields = "paytime,status";
    m.setPaytime((unsigned int)timeid());
    m.setStatus(order_status::paid);
    if (!trade_no.empty())
    {
        m.setStoreorder(trade_no);
        fields += ",storeorder";
    }
    return m.update(fields) > 0;
}

asio::awaitable<bool> async_order_set_paid(const order_ref &o, const std::string &trade_no)
{
    orm::cms::Orderlist m;
    m.where("wxorder", o.wxorder).where("status", order_status::pending);
    std::string fields = "paytime,status";
    m.setPaytime((unsigned int)timeid());
    m.setStatus(order_status::paid);
    if (!trade_no.empty())
    {
        m.setStoreorder(trade_no);
        fields += ",storeorder";
    }
    co_return co_await m.async_update(fields) > 0;
}

settle_code settle_decide(const std::string &wxorder, bool found, unsigned long long fee_fen,
                          unsigned int payprice)
{
    // 空单号排在"查不到"之前判：根本不许拿空串去查 —— WHERE wxorder='' AND status=0
    // 会把所有空单号的待付行一起翻掉。
    if (wxorder.empty() || !found) return settle_code::not_found;
    // 0 分单独挡掉：只比"相等"的话，一条 0 金额的通知能把库里 payprice=0 的行对账成功。
    if (fee_fen == 0 || fee_fen != (unsigned long long)payprice) return settle_code::amount_mismatch;
    return settle_code::settled;
}

asio::awaitable<settle_out> settle_notify(const std::string &wxorder,
                                          unsigned long long fee_fen,
                                          const std::string &trade_no)
{
    settle_out r;
    // 空单号连查询都不发：不让 WHERE wxorder='' 出门。
    bool found = false;
    if (!wxorder.empty()) found = co_await async_order_find(wxorder, r.order);
    r.code = settle_decide(wxorder, found, fee_fen, r.order.payprice);
    if (r.code != settle_code::settled) co_return r;
    // 校验通过才发那一枪。CAS 返回 false = 这行已经不是待支付（重投 / 已撤销 / 已退款），
    // 不许把状态拉回"已支付"，paytime 也保持第一次落账的时间。
    r.code = co_await async_order_set_paid(r.order, trade_no) ? settle_code::settled
                                                              : settle_code::duplicate;
    co_return r;
}

// 只补 storeorder，不改 status。按主键定位，也只列出这一列避免全列覆盖。
bool order_set_storeorder(const order_ref &o, const std::string &trade_no)
{
    orm::cms::Orderlist m;
    m.where("orderid", o.oid);
    m.setStoreorder(trade_no);
    return m.update("storeorder") > 0;
}

asio::awaitable<bool> async_order_set_storeorder(const order_ref &o, const std::string &trade_no)
{
    orm::cms::Orderlist m;
    m.where("orderid", o.oid);
    m.setStoreorder(trade_no);
    co_return co_await m.async_update("storeorder") > 0;
}

bool order_cancel_by_wxorder(const std::string &wxorder)
{
    if (wxorder.empty()) return false;
    orm::cms::Orderlist m;
    m.where("wxorder", wxorder).where("status", order_status::pending);
    m.setStatus(order_status::cancelled);
    return m.update("status") > 0;
}

asio::awaitable<bool> async_order_cancel_by_wxorder(const std::string &wxorder)
{
    if (wxorder.empty()) co_return false;
    orm::cms::Orderlist m;
    m.where("wxorder", wxorder).where("status", order_status::pending);
    m.setStatus(order_status::cancelled);
    co_return co_await m.async_update("status") > 0;
}

bool order_set_cancelled(const order_ref &o)
{
    return order_cancel_by_wxorder(o.wxorder);
}

asio::awaitable<bool> async_order_set_cancelled(const order_ref &o)
{
    co_return co_await async_order_cancel_by_wxorder(o.wxorder);
}

bool order_set_refunded(const order_ref &o)
{
    orm::cms::Orderlist m;
    m.where("wxorder", o.wxorder).where("status", order_status::paid);
    m.setIsrefund(1);
    m.setStatus(order_status::refunded);
    if (m.update("isrefund,status") == 0) return false;

    // refundnum 走数据库自增 update_col(n, 1)，避免"读旧值 +1 写回"吞掉并发累加。
    // 必须换新对象：上一条 update() 不清 wheresql，status=paid 还留在里面，
    // 直接打第二枪永远 0 行。
    orm::cms::Orderlist c;
    c.where("wxorder", o.wxorder);
    c.update_col("refundnum", 1);
    return true;
}

asio::awaitable<bool> async_order_set_refunded(const order_ref &o)
{
    orm::cms::Orderlist m;
    m.where("wxorder", o.wxorder).where("status", order_status::paid);
    m.setIsrefund(1);
    m.setStatus(order_status::refunded);
    if (co_await m.async_update("isrefund,status") == 0) co_return false;

    orm::cms::Orderlist c;
    c.where("wxorder", o.wxorder);
    co_await c.async_update_col("refundnum", 1);
    co_return true;
}

bool can_refund(const order_ref &o, unsigned long long fen, std::string &why)
{
    if (o.status != order_status::paid)
    {
        why = "订单 status=" + std::to_string(o.status) + "，不是 1（已付）："
              "退款要等回调把状态推进过之后才能发；一到 2/3/9 这些终态就不能再退，"
              "一笔订单只允许退一次。";
        return false;
    }
    if (fen == 0 || fen > o.payprice)
    {
        why = "退款金额必须落在 (0, " + std::to_string(o.payprice) + "] 分内。";
        return false;
    }
    return true;
}

unsigned long long resolve_refund_fen(const order_ref &o, unsigned long long req)
{
    return req == 0 ? o.payprice : req;
}

bool refund_reply_accepted(const std::string &status, const std::string &refund_id)
{
    if (refund_id.empty()) return false;
    return status == "SUCCESS" || status == "PROCESSING";
}

std::string refund_once_notice()
{
    return "一笔订单只支持一次退款，需要分次退请拆成多笔订单。";
}

std::string refund_no_gen(const order_ref &o)
{
    return "R" + o.wxorder + "-" + std::to_string(o.refundnum + 1);
}

} // namespace webpay
}//namespace http
