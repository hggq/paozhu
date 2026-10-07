/*
 * pzredis 发布订阅（Pub/Sub）订阅者实现
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 */
#include "redis_pubsub.h"

#include <asio/co_spawn.hpp>
#include <asio/use_awaitable.hpp>

#include <algorithm>
#include <chrono>

namespace pz
{
namespace redis
{

redis_subscriber::redis_subscriber(asio::io_context &ioc)
    : redis_conn_base(ioc), strand_(asio::make_strand(ioc.get_executor()))
{
}

redis_subscriber::~redis_subscriber()
{
    // 析构一定发生在最后一个强引用释放处，而那只可能是 pump 的协程帧（跑在 io_context 线程），
    // 所以这里直接干活，不再投递。
    stop_now();
}

redis_subscription redis_subscription::attach(std::weak_ptr<redis_subscriber> sub)
{
    return redis_subscription(std::make_shared<ticket>(std::move(sub)));
}

// ===================== 句柄透传：在已有订阅者上动态增删频道 =====================
// 契约：active_cmd_ 单槽，调用方必须串行 co_await 完一次再发下一次（见头文件声明）。
// 所有真正改动内部状态的动作都 co_spawn 到 sub->strand()，与 pump 串行，沿用 redis_pool::subscribe 的既定模式。

asio::awaitable<bool> redis_subscription::async_subscribe(const std::vector<std::string> &channels,
                                                          message_cb cb) const
{
    auto s = ref_ ? ref_->sub.lock() : nullptr;
    if (!s)
        co_return false;
    co_return co_await asio::co_spawn(
        s->strand(),
        [s, channels, cb = std::move(cb)]() mutable -> asio::awaitable<bool>
        {
            co_return co_await s->async_subscribe(channels, std::move(cb));
        },
        asio::use_awaitable);
}

asio::awaitable<bool> redis_subscription::async_psubscribe(const std::string &pattern,
                                                           pmessage_cb cb) const
{
    auto s = ref_ ? ref_->sub.lock() : nullptr;
    if (!s)
        co_return false;
    co_return co_await asio::co_spawn(
        s->strand(),
        [s, pattern, cb = std::move(cb)]() mutable -> asio::awaitable<bool>
        {
            co_return co_await s->async_psubscribe(pattern, std::move(cb));
        },
        asio::use_awaitable);
}

asio::awaitable<bool> redis_subscription::async_unsubscribe(const std::vector<std::string> &channels) const
{
    auto s = ref_ ? ref_->sub.lock() : nullptr;
    if (!s)
        co_return false;
    co_return co_await asio::co_spawn(
        s->strand(),
        [s, channels]() mutable -> asio::awaitable<bool>
        {
            co_return co_await s->async_unsubscribe(channels);
        },
        asio::use_awaitable);
}

asio::awaitable<bool> redis_subscription::async_punsubscribe(const std::vector<std::string> &patterns) const
{
    auto s = ref_ ? ref_->sub.lock() : nullptr;
    if (!s)
        co_return false;
    co_return co_await asio::co_spawn(
        s->strand(),
        [s, patterns]() mutable -> asio::awaitable<bool>
        {
            co_return co_await s->async_punsubscribe(patterns);
        },
        asio::use_awaitable);
}

// ===================== 启动：连接 + 后台 pump =====================
asio::awaitable<bool> redis_subscriber::async_start(const conn_config_t &cfg)
{
    // 句柄可能在我们排队期间就被业务丢掉了（stop 已经投递并执行），这时不能再连
    if (state_.load() == subscriber_state::stopped)
        co_return false;
    if (!co_await async_connect(cfg))
    {
        state_.store(subscriber_state::stopped);
        co_return false;
    }
    if (state_.load() == subscriber_state::stopped)
    {
        // 等连接的那拍里 stop 进来了：刚开的连接当场收掉，不启动 pump
        close();
        co_return false;
    }
    state_.store(subscriber_state::connecting);
    // pump 持有 self，保证本对象在消息泵运行期间不被析构；
    // 业务侧的句柄只负责"投递一次 stop"，不负责供养对象。
    auto self = shared_from_this();
    asio::co_spawn(strand_, [self]() -> asio::awaitable<void>
                   { co_await self->pump(); },
                   asio::detached);
    co_return true;
}

// ===================== 后台消息泵（唯一 reader）=====================
asio::awaitable<void> redis_subscriber::pump()
{
    while (state_.load() != subscriber_state::stopped && connected())
    {
        // 消息泵不设期限：推送帧可能几十分钟才来一条，给它一份 timeout_sec 等于把空闲订阅打成永久失败。
        // 这一条是协程收发里唯一"就该一直等"的调用点，其余（订阅命令的写、确认帧）各有自己的期限。
        auto r = co_await async_read_reply(std::chrono::steady_clock::time_point::max());
        if (!r)
            break;// 连接断开 / 出错
        on_frame(*r);
    }
    // 读不下去就是结束了：置停 + 清回调表，别让 channel_count() 继续报旧值
    stop_now();
}

// ===================== 帧分发 =====================
void redis_subscriber::on_frame(const reply_t &frame)
{
    // 兼容 RESP2（array，普通多 bulk 数组）与 RESP3（push，'>' 推送）
    if (frame.type != reply_type::array && frame.type != reply_type::push)
        return;
    if (frame.array_value.empty())
        return;

    const std::string &kind = frame.array_value[0].str_value;

    if (kind == "message")
    {
        if (frame.array_value.size() >= 3)
        {
            auto it = channel_cbs_.find(frame.array_value[1].str_value);
            if (it != channel_cbs_.end())
                it->second(frame.array_value[1].str_value, frame.array_value[2].str_value);
        }
        return;
    }
    if (kind == "pmessage")
    {
        if (frame.array_value.size() >= 4)
        {
            auto it = pattern_cbs_.find(frame.array_value[1].str_value);
            if (it != pattern_cbs_.end())
                it->second(frame.array_value[1].str_value,
                           frame.array_value[2].str_value,
                           frame.array_value[3].str_value);
        }
        return;
    }
    // 订阅确认帧：subscribe / unsubscribe / psubscribe / punsubscribe
    if (kind == "subscribe" || kind == "unsubscribe" ||
        kind == "psubscribe" || kind == "punsubscribe")
    {
        if (active_cmd_ && active_cmd_->got < active_cmd_->expect)
        {
            active_cmd_->got++;
            if (active_cmd_->got == active_cmd_->expect)
                handle_confirm(active_cmd_);
        }
    }
}

void redis_subscriber::handle_confirm(std::shared_ptr<sub_op> op)
{
    if (op->is_unsub)
    {
        // 频道名与模式名可以恰好同名，两张表不能一起擦
        if (op->is_pattern)
            for (const auto &n : op->names)
                pattern_cbs_.erase(n);
        else
            for (const auto &n : op->names)
                channel_cbs_.erase(n);
    }
    sync_counts();
    op->ok = true;
    if (!op->is_unsub)
        state_.store(subscriber_state::subscribed);
    op->timer.cancel();// 唤醒等待中的 async_subscribe / async_unsubscribe
}

void redis_subscriber::sync_counts()
{
    channel_count_.store(channel_cbs_.size());
    pattern_count_.store(pattern_cbs_.size());
}

// 同一条 SUBSCRIBE/PSUBSCRIBE 里重复的名字，服务端按集合去重、只回一个确认帧（实测），
// 所以期望帧数必须按去重后的条数算；命令本身也按去重后的名单发。
namespace
{
std::vector<std::string> dedup_names(const std::vector<std::string> &in)
{
    std::vector<std::string> out;
    out.reserve(in.size());
    for (const auto &n : in)
        if (std::find(out.begin(), out.end(), n) == out.end())
            out.push_back(n);
    return out;
}
}// namespace

// ===================== 订阅 / 取消订阅 =====================
asio::awaitable<bool> redis_subscriber::async_subscribe(const std::vector<std::string> &channels, message_cb cb)
{
    if (state_.load() == subscriber_state::stopped || channels.empty())
        co_return false;
    auto uniq = dedup_names(channels);
    for (const auto &ch : uniq)
        channel_cbs_[ch] = cb;
    sync_counts();

    std::vector<std::string> cmd = {"SUBSCRIBE"};
    cmd.insert(cmd.end(), uniq.begin(), uniq.end());

    auto op     = std::make_shared<sub_op>(*raw_ioc());
    op->expect  = static_cast<int>(uniq.size());
    active_cmd_ = op;

    bool wrote = co_await async_write_raw(make_command(cmd), make_deadline());
    if (!wrote)
    {
        active_cmd_ = nullptr;
        co_return false;
    }
    co_return co_await wait_confirm(op);
}

asio::awaitable<bool> redis_subscriber::async_psubscribe(const std::string &pattern, pmessage_cb cb)
{
    if (state_.load() == subscriber_state::stopped || pattern.empty())
        co_return false;
    pattern_cbs_[pattern] = cb;
    sync_counts();

    std::vector<std::string> cmd = {"PSUBSCRIBE", pattern};
    auto op                      = std::make_shared<sub_op>(*raw_ioc());
    op->expect                   = 1;
    op->is_pattern               = true;
    active_cmd_                  = op;

    bool wrote = co_await async_write_raw(make_command(cmd), make_deadline());
    if (!wrote)
    {
        active_cmd_ = nullptr;
        co_return false;
    }
    co_return co_await wait_confirm(op);
}

asio::awaitable<bool> redis_subscriber::async_unsubscribe(const std::vector<std::string> &channels)
{
    if (state_.load() == subscriber_state::stopped || channels.empty())
        co_return false;
    auto uniq                    = dedup_names(channels);
    std::vector<std::string> cmd = {"UNSUBSCRIBE"};
    cmd.insert(cmd.end(), uniq.begin(), uniq.end());

    auto op      = std::make_shared<sub_op>(*raw_ioc());
    op->expect   = static_cast<int>(uniq.size());
    op->is_unsub = true;
    op->names    = uniq;
    active_cmd_  = op;

    bool wrote = co_await async_write_raw(make_command(cmd), make_deadline());
    if (!wrote)
    {
        active_cmd_ = nullptr;
        co_return false;
    }
    co_return co_await wait_confirm(op);
}

asio::awaitable<bool> redis_subscriber::async_punsubscribe(const std::vector<std::string> &patterns)
{
    if (state_.load() == subscriber_state::stopped || patterns.empty())
        co_return false;
    auto uniq                    = dedup_names(patterns);
    std::vector<std::string> cmd = {"PUNSUBSCRIBE"};
    cmd.insert(cmd.end(), uniq.begin(), uniq.end());

    auto op        = std::make_shared<sub_op>(*raw_ioc());
    op->expect     = static_cast<int>(uniq.size());
    op->is_unsub   = true;
    op->is_pattern = true;
    op->names      = uniq;
    active_cmd_    = op;

    bool wrote = co_await async_write_raw(make_command(cmd), make_deadline());
    if (!wrote)
    {
        active_cmd_ = nullptr;
        co_return false;
    }
    co_return co_await wait_confirm(op);
}

// 等确认帧。有期限：确认帧丢了、被错误回复顶掉了，都不能把这条协程永久挂住。
// 期限就是这条连接的 timeout_sec（0 才是"不设限"，那时订阅确认允许一直等）。
asio::awaitable<bool> redis_subscriber::wait_confirm(std::shared_ptr<sub_op> op)
{
    asio::error_code ig;
    if (config().timeout_sec > 0)
        op->timer.expires_after(std::chrono::seconds(config().timeout_sec));
    else
        op->timer.expires_at(std::chrono::steady_clock::time_point::max());
    co_await op->timer.async_wait(asio::redirect_error(asio::use_awaitable, ig));
    active_cmd_ = nullptr;
    // 已经被 stop 过就别把状态改回 connecting，那等于把"已停"这条事实擦掉
    if (!op->ok && state_.load() != subscriber_state::stopped)
    {
        // 没等齐确认：这次订阅算失败。连接还活着，但状态不再报"已订阅"，
        // 让调用方从 state() 就能看出这条订阅者此刻不可信。
        state_.store(subscriber_state::connecting);
    }
    co_return op->ok;
}

void redis_subscriber::stop()
{
    // 从任意线程进来都只做一件事：把"停"投递回连接真正归属的那条 strand
    // （stop_now 会改回调表 / active_cmd_，必须和 pump / 订阅协程在同一串行域）。
    // 没被 shared_ptr 持有时（栈上对象、启动即失败）投递也没人接，就地收。
    auto wp = weak_from_this();
    if (wp.use_count() == 0)
    {
        stop_now();
        return;
    }
    asio::post(strand_, [wp]()
               {
        if (auto s = wp.lock())
            s->stop_now(); });
}

void redis_subscriber::stop_now()
{
    if (state_.exchange(subscriber_state::stopped) == subscriber_state::stopped)
        return;// 谁先到谁收尾（pump 退出、句柄析构、显式 stop 三条路都会进来），重复调用不再动表也不再 close
    if (active_cmd_)
    {
        active_cmd_->ok = false;
        active_cmd_->timer.cancel();
    }
    // 订阅已经不存在了，两张回调表要跟着清，channel_count() 才不撒谎
    channel_cbs_.clear();
    pattern_cbs_.clear();
    sync_counts();
    close();// 基类方法：只关 socket（不发 QUIT），pump 的 async_read_reply 会因此返回 nullopt
}

}// namespace redis
}// namespace pz
