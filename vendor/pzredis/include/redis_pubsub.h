#ifndef PZ_REDIS_PUBSUB_H
#define PZ_REDIS_PUBSUB_H
/*
 * pzredis 发布订阅（Pub/Sub）订阅者
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 *
 * 设计：redis_subscriber 继承 redis_conn_base，独占一条连接做 SUBSCRIBE/PSUBSCRIBE。
 *  - 连接后启动后台 pump 协程（唯一 reader），持续读取服务端推送帧并分发回调。
 *  - message/pmessage 推送 → 触发回调（回调在框架共享 io_context 的某条线程上、strand_ 里串行执行，
 *    所以回调里做慢事会拖住那条线程名下的其他会话）。
 *  - subscribe/unsubscribe/psubscribe/punsubscribe 的确认帧 → 唤醒等待中的 async_subscribe；
 *    确认最多等 cfg.timeout_sec 秒，等不到就算这次订阅失败，不会永久挂住。
 * 所有协程共享一条 strand_ 来串行化内部状态的访问。
 * 生死：pump 自己持有订阅者的强引用，业务侧拿的是 redis_subscription 句柄；
 *       最后一个句柄析构（或显式 stop()）就把停止动作投递进 strand，订阅随之结束。
 * 注：PUBLISH 走普通连接池命令（redis_pool::async_publish / redis_conn_base::publish），
 *     本类只负责接收侧；发布侧不会在本连接上发送。
 */
#include <asio.hpp>
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <asio/steady_timer.hpp>
#include <algorithm>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "redis_conn.h"

namespace pz
{
namespace redis
{

// 频道消息回调：channel + payload
using message_cb = std::function<void(const std::string &channel, const std::string &payload)>;
// 模式消息回调：pattern + channel + payload
using pmessage_cb = std::function<void(const std::string &pattern, const std::string &channel, const std::string &payload)>;

// 订阅者此刻在哪一步（跨线程只读安全）
enum class subscriber_state : unsigned char
{
    idle = 0,  // 还没连上
    connecting,// 已连接，订阅确认还没收齐
    subscribed,// 至少一次订阅确认成功
    stopped    // 已结束：主动 stop / 连接断开 / 确认超时后被关闭
};

class redis_subscriber : public redis_conn_base
    , public std::enable_shared_from_this<redis_subscriber>
{
  public:
    explicit redis_subscriber(asio::io_context &ioc);
    ~redis_subscriber();

    // 连接并启动后台消息泵（pump 会持续读取推送并触发回调）。
    // 成功返回 true；之后即可调用 async_subscribe / async_psubscribe。
    asio::awaitable<bool> async_start(const conn_config_t &cfg);

    // 订阅频道；返回是否成功（含确认帧收齐）。cb 在 io_context 线程上、strand_ 里触发。
    // 同一条命令里重复的频道名，服务端只回一格确认帧，这里按去重后的条数等。
    asio::awaitable<bool> async_subscribe(const std::vector<std::string> &channels, message_cb cb);
    // 订阅模式（通配符，如 news.*）；确认帧只有 1 个。
    asio::awaitable<bool> async_psubscribe(const std::string &pattern, pmessage_cb cb);

    // 取消订阅（确认后移除对应回调）
    asio::awaitable<bool> async_unsubscribe(const std::vector<std::string> &channels);
    asio::awaitable<bool> async_punsubscribe(const std::vector<std::string> &patterns);

    // 结束订阅。任何线程调用都安全：置位与关连接是投递进 redis io_context 的，
    // 那里才是这条连接此刻唯一的持有者。重复调用没有副作用。
    void stop();

    subscriber_state state() const { return state_.load(); }
    // 只读：回调表改动只在 strand 上，这里返回的是镜像计数。
    std::size_t channel_count() const { return channel_count_.load(); }
    std::size_t pattern_count() const { return pattern_count_.load(); }

    // 所有协程共享这一条 strand。
    asio::strand<asio::io_context::executor_type> strand() const { return strand_; }

  private:
    // 一次订阅/取消订阅操作的等待信号（用 steady_timer 当一次性信号，等确认帧时不占住那条 io_context 线程）
    struct sub_op
    {
        explicit sub_op(asio::io_context &ioc) : timer(ioc) {}
        int expect      = 0;// 期望的确认帧数（按服务端去重后的口径）
        int got         = 0;// 已收确认帧数
        bool ok         = false;
        bool is_unsub   = false;
        bool is_pattern = false;       // names 是模式名还是频道名：两张表的擦除不能互相误删
        std::vector<std::string> names;// 取消订阅时的频道/模式名，确认后移除回调
        asio::steady_timer timer;
    };

    asio::awaitable<void> pump();
    asio::awaitable<bool> wait_confirm(std::shared_ptr<sub_op> op);
    void on_frame(const reply_t &frame);
    void handle_confirm(std::shared_ptr<sub_op> op);
    // 真正干活的那一步：只在 io_context 线程调用
    void stop_now();
    void sync_counts();

    std::map<std::string, message_cb> channel_cbs_;
    std::map<std::string, pmessage_cb> pattern_cbs_;
    std::shared_ptr<sub_op> active_cmd_;// 当前等待确认的命令（单连接串行，同一时刻只有一个）
    std::atomic<std::size_t> channel_count_{0};
    std::atomic<std::size_t> pattern_count_{0};
    std::atomic<subscriber_state> state_{subscriber_state::idle};

    // 所有协程共享这一条 strand。
    asio::strand<asio::io_context::executor_type> strand_;
};

// 订阅句柄：redis_pool::subscribe() 的返回值。可复制，多份句柄共享同一订阅；
// 最后一份析构（或调用 stop()）就结束订阅。析构所在线程不限 —— 动作是投递进 redis io_context 的。
class redis_subscription
{
  public:
    redis_subscription() = default;

    static redis_subscription attach(std::weak_ptr<redis_subscriber> sub);

    bool valid() const { return ref_ != nullptr && ref_->sub.lock() != nullptr; }
    void stop()
    {
        if (!ref_)
            return;
        if (auto s = ref_->sub.lock())
            s->stop();
    }

    // 在已有订阅者（本句柄对应的那条连接）上动态追加/退订频道或模式。
    // 返回是否成功（含确认帧收齐）。注意契约：redis_subscriber 的 active_cmd_ 是单槽，
    // 必须串行 co_await 完一次再发下一次，不要并行 fire-and-forget，否则确认帧会错位。
    // 句柄失效（订阅已停止或连接断开）时返回 false，不抛异常。
    asio::awaitable<bool> async_subscribe(const std::vector<std::string> &channels, message_cb cb) const;
    asio::awaitable<bool> async_psubscribe(const std::string &pattern, pmessage_cb cb) const;
    asio::awaitable<bool> async_unsubscribe(const std::vector<std::string> &channels) const;
    asio::awaitable<bool> async_punsubscribe(const std::vector<std::string> &patterns) const;

    subscriber_state state() const
    {
        if (!ref_)
            return subscriber_state::stopped;
        auto s = ref_->sub.lock();
        return s ? s->state() : subscriber_state::stopped;
    }
    std::size_t channel_count() const
    {
        auto s = ref_ ? ref_->sub.lock() : nullptr;
        return s ? s->channel_count() : 0;
    }
    std::size_t pattern_count() const
    {
        auto s = ref_ ? ref_->sub.lock() : nullptr;
        return s ? s->pattern_count() : 0;
    }

  private:
    // 引用计数归零 ⇒ 投递 stop；订阅者的生死在 pump 手里，所以这里只会"叫醒它结束"，不会踩到已析构对象。
    struct ticket
    {
        explicit ticket(std::weak_ptr<redis_subscriber> s) : sub(std::move(s)) {}
        ~ticket()
        {
            if (auto s = sub.lock())
                s->stop();
        }
        std::weak_ptr<redis_subscriber> sub;
    };

    explicit redis_subscription(std::shared_ptr<ticket> r) : ref_(std::move(r)) {}
    std::shared_ptr<ticket> ref_;
};

}// namespace redis
}// namespace pz

#endif
