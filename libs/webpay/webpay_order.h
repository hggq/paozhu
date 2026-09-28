#ifndef __WEB_PAY_ORDER_H__
#define __WEB_PAY_ORDER_H__
// 有意不受 ENABLE_WEBPAY 管辖：orderlist 表跟 SDK 没关系，而多条路由站在自己的 #ifdef 之外
// 用它（先问库、再谈凭据的那一半）。符号必须始终可见，否则 OFF 档连编译都过不去。

#include "orm.h"

namespace http
{

// 订单状态。终态判定走 is_terminal()，别只比 3。
namespace order_status {
    constexpr int pending   = 0; // 待支付
    constexpr int paid      = 1; // 已支付
    constexpr int cancelled = 2; // 已撤销（alipay）
    constexpr int refunded  = 3; // 已退款（现在所有退款路由都写这个）
    constexpr int refunded_legacy = 9; // 历史退款值，不再写入，只在旧数据里出
    inline bool is_terminal(int s)
    {
        return s == cancelled || s == refunded || s == refunded_legacy;
    }
} // namespace order_status

// 支付类型，列类型 tinyint unsigned。取值是外部契约，只增不改。
// 查单一律按 wxorder（唯一索引），不带 paytype 过滤，避免老单（paytype=0）被误判不存在。
namespace order_paytype {
    constexpr unsigned char none      = 0; // 未标记（老数据）
    constexpr unsigned char alipay    = 1;
    constexpr unsigned char wxpayv2   = 2;
    constexpr unsigned char wxpayv3   = 3;
    constexpr unsigned char weixinxcx = 4;
} // namespace order_paytype

namespace webpay
{

// 单号生成
std::string make_storeorder();                              // 内部单号 "SO"+时间戳+随机
std::string make_wxorder(const std::string &prefix = "WX"); // 给网关的商户单号

// 只留单号 / openid 允许的字符 [0-9a-zA-Z_-]，其余丢弃。给来自请求参数的标识串用。
std::string id_safe(const std::string &s);

// 订单行引用：一次查询后复用，避免闸门/幂等键/落账各走一趟 SELECT
struct order_ref
{
    bool         found     = false;
    long long    oid       = 0;
    std::string  wxorder;
    unsigned int payprice  = 0;
    unsigned int refundnum = 0;
    int          status    = -1;
};

// 按 wxorder 查一行（uk_wxorder ⇒ 至多一行）。true = 命中。
bool order_find(const std::string &wxorder, order_ref &out);
asio::awaitable<bool> async_order_find(const std::string &wxorder, order_ref &out);

// ORM 对象 → order_ref，纯转换不发 SQL。服务按 orderid 查的路由。
order_ref order_ref_of(const orm::cms::Orderlist &m);

// 下单落库。返回 orderid（<=0 失败）。storeorder 空串则自动生成。
// 覆盖 wxpayv2/v3/weixinxcx 三路；alipay 有额外字段另走一层。
long long order_insert(unsigned char paytype,
                       const std::string &wxorder,
                       const std::string &storeorder,
                       const std::string &openid,
                       const std::string &paytitle,
                       unsigned int payprice);
asio::awaitable<long long> async_order_insert(unsigned char paytype,
                                              const std::string &wxorder,
                                              const std::string &storeorder,
                                              const std::string &openid,
                                              const std::string &paytitle,
                                              unsigned int payprice);

// 状态跃迁：全写成一条 CAS UPDATE，WHERE 带 wxorder + 期望的原状态，
// 并发重投只有一路能命中。返回 true = 本次真翻转了该行。
// 调用方据此决定副作用（发货、积分、通知），不要用 find 读到的状态判。

// 支付回调 0→1。trade_no 非空时写进 storeorder。
bool order_set_paid(const order_ref &o, const std::string &trade_no = "");
asio::awaitable<bool> async_order_set_paid(const order_ref &o, const std::string &trade_no = "");

// 回调落账三步（查单 → 金额逐分比对 → 0→1 CAS）的共用体；四条通知路由的差别只在验签调用、
// 成交状态字段、应答格式。判定本身在 settle_decide()（纯函数），这里只有执行。
enum class settle_code
{
    settled,         // 校验通过且本次 CAS 真翻转了该行
    duplicate,       // 校验通过，但该行已不是待支付（重复通知 / 已撤销 / 已退款）
    not_found,       // 按商户单号查不到行 —— 漏单，让对方重投
    amount_mismatch  // 金额对不上（含通知金额 0 分）
};

// 只回答"这条通知该不该落账"，不发 SQL，所以 duplicate 不会出现在这里。
// 空单号直接 not_found：不拿空串去查 —— CAS 的 WHERE 带上 wxorder='' 会把所有空单号的待付行
// 一起翻掉。
// fee_fen == 0 也判 amount_mismatch：只比"相等"的话，一条 0 金额的通知能把库里 payprice=0
// 的行对账成功。
settle_code settle_decide(const std::string &wxorder,
                          bool found,
                          unsigned long long fee_fen,
                          unsigned int payprice);

struct settle_out
{
    settle_code code  = settle_code::not_found;
    order_ref   order;
    // SUCCESS / FAIL 的分界：settled 与 duplicate 都答 SUCCESS（答 FAIL 会让对方无限重投
    // 一条已落账的通知），差别只在"这条有没有真把行翻成已支付"。
    bool ok() const { return code == settle_code::settled || code == settle_code::duplicate; }
};

// 查单 → settle_decide → 0→1 CAS。任何一道校验不过就把 SQL 停在读那一趟，不写。
// trade_no 非空才写 storeorder（支付宝回调手里有 trade_no，微信侧留空）。
asio::awaitable<settle_out> settle_notify(const std::string &wxorder,
                                          unsigned long long fee_fen,
                                          const std::string &trade_no = "");

// 只补 storeorder，不碰 status：同步回跳参数里没有 trade_status，不是落账依据。
bool order_set_storeorder(const order_ref &o, const std::string &trade_no);
asio::awaitable<bool> async_order_set_storeorder(const order_ref &o, const std::string &trade_no);

// 撤单 0→2
bool order_set_cancelled(const order_ref &o);
asio::awaitable<bool> async_order_set_cancelled(const order_ref &o);

// 只按商户单号撤单：下单那一趟刚把行插进去、还没查过它，手里只有单号。
// 空单号直接 false —— 否则 CAS 退化成"把所有 wxorder='' 的待付行一起撤掉"。
bool order_cancel_by_wxorder(const std::string &wxorder);
asio::awaitable<bool> async_order_cancel_by_wxorder(const std::string &wxorder);

// 退款 1→3，同时 refundnum++
bool order_set_refunded(const order_ref &o);
asio::awaitable<bool> async_order_set_refunded(const order_ref &o);

// 退款资格：status==paid 且 0<fen<=payprice。why 只讲状态或金额，不区分"没这单"——
// 调用方先判 found，再自己补上渠道相关的说明。
// status 一旦推进到终态（2 撤销 / 3 已退 / 9 历史已退）就不能再退：落账那条 CAS 是
// status=1→3 的一跳，同一单不存在第二次退。
bool can_refund(const order_ref &o, unsigned long long fen, std::string &why);

// 退款金额口径：req==0 表示"没传金额、按订单金额全额退"，其余原样返回。只做换算不做判定，
// 能不能退、金额超没超仍归 can_refund。支付宝不走这里：它的 refund_amount 是元字符串且必传，
// 空值直接拒收 —— 漏参数不等于全额退款。
unsigned long long resolve_refund_fen(const order_ref &o, unsigned long long req);

// V3 退款应答算不算"受理成功"：只认 status 与 refund_id，"正文里出现过 out_refund_no"不算
// —— 网关的错误页也可能含那个子串。CLOSED / ABNORMAL 一律不落账：钱没退出去，落了是假账。
bool refund_reply_accepted(const std::string &status, const std::string &refund_id);

// 一句口径提示，给所有退款页共用（一笔订单只允许退一次，要多次得分单退）。
std::string refund_once_notice();

// 退款幂等键 "R"+wxorder+"-"+(refundnum+1)。发网关那一刻 refundnum 恒为 0（自增在落账那条
// CAS 里），所以后缀实际永远是 "-1"：同一次退款的两路重放拿到同一个号，网关只受理一次。
// ⚠ 尾巴上的序号不能顺手删：已发出但本地落账没成功（CAS 未命中 ⇒ status 仍是 1）的那笔退款
// 若被重放，换成新键会让网关看到两个不同的 out_refund_no ⇒ 可能重复退款。
// 要先确认线上无在途退款才动它。
std::string refund_no_gen(const order_ref &o);

} // namespace webpay
}//namespace http
#endif
