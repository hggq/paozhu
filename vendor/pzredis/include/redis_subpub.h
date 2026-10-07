#ifndef PZ_REDIS_SUBPUB_H
#define PZ_REDIS_SUBPUB_H

#ifdef ENABLE_REDIS_CLIENT
/*
 * pzredis 框架级 Pub/Sub 订阅客户端基类
 * author 黄自权
 * date 2026-10-03
 *
 * 业务代码继承此类，在 common/redis_regmethod.hpp 里注册，
 * 框架在 server.cpp 启动时自动实例化、自动 co_spawn 一个长期运行协程：
 *   async_start → async_subscribe → pump（重连循环）
 * websocket_loop 每秒一拍扫 redis_subpub_tasks，按 durtime 分频调 run_loop/async_run_loop。
 *
 * 线程模型：
 *   - 框架长期协程跑在 server io_context 上，subscribe/unsubscribe 回调也在 io_context 线程触发
 *   - pump 收到 message/pmessage → 直接调 on_message()（同线程）
 *   - is_coroutine()==true 时，业务重写 async_on_message()，框架在同 ioc 上调协程版
 *   - run_loop() 在 tick 线程直接调用；async_run_loop() 在 io_context 上调
 *
 * 生命周期：
 *   - 业务 stop() / 析构 → isclose=true → 长期协程 while 条件终止 → 协程退出
 *   - Redis 断连 → pump 退出 → state_=stopped → 长期协程检测到 → 3s 后 async_start 重建
 *   - server shutdown → io_context.run() 退出 → 协程里的 steady_timer/async_connect 抛 → 协程自然退出
 */
#include <asio.hpp>
#include <asio/io_context.hpp>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "redis_conn.h"
#include "redis_pubsub.h"

namespace pz
{
namespace redis
{

class redis_subpub_client : public std::enable_shared_from_this<redis_subpub_client>
{
  public:
    virtual ~redis_subpub_client() = default;

    // ===== 业务侧实现（框架回调）=====

    // 订阅的段名（对应 pool 配置里的 section）
    virtual std::string section_name() const { return "default"; }
    // 频道（SUBSCRIBE 列表）
    virtual std::vector<std::string> channels() const { return {}; }
    // 模式（PSUBSCRIBE 列表；多 pattern 需要多次订阅，框架会循环调 async_psubscribe）
    virtual std::vector<std::string> patterns() const { return {}; }

    // 订阅建立成功后（一次调用，在长期协程线程里）
    virtual void on_subscribe_ok() {}
    virtual asio::awaitable<void> async_on_subscribe_ok() { co_return; }

    // 消息到达（pump 协程线程触发）
    virtual void on_message(const std::string &channel, const std::string &payload) {}
    virtual void on_pmessage(const std::string &pattern, const std::string &channel, const std::string &payload) {}
    // 协程版（is_coroutine()==true 时业务可以重写 async_on_message）
    virtual asio::awaitable<void> async_on_message(const std::string &channel, const std::string &payload) { co_return; }
    virtual asio::awaitable<void> async_on_pmessage(const std::string &pattern, const std::string &channel, const std::string &payload) { co_return; }

    // 定时推送（websocket_loop 按 durtime 分频调用；durtime=0 不 tick）
    virtual void run_loop() {}
    virtual asio::awaitable<void> async_run_loop() { co_return; }

    // 订阅断连重连时（每轮新连接建成功后都会调一次）
    virtual void on_disconnect() {}
    virtual asio::awaitable<void> async_on_disconnect() { co_return; }

    // ===== 框架字段 / 业务也可以读 =====

    // is_coroutine_=true → 框架走 async_* 协程版回调；构造时设
    bool is_coroutine_ = false;
    // tick 分频：每秒一拍 websocket_loop 按 durtime 整除触发 run_loop；0=不 tick
    unsigned int durtime = 0;
    // tick 扫描用；loop_num==0 时 websocket_loop 扫到就从 list 擦除
    unsigned int loop_num = 1;
    // 框架 / 业务都可以置；置 true 后长期协程退出、tick 剔除。
    // 置位方是业务线程（stop()），读的一方是 io_context 与 tick 线程 ⇒ 必须原子。
    std::atomic<bool> isclose = false;
    // 当前 subscriber 的 state（框架维护，业务只读）
    std::atomic<subscriber_state> sub_state_{subscriber_state::idle};
    // 框架长期协程的 io_context 指针；业务在 on_subscribe_ok / async_on_subscribe_ok 等回调里可以 co_spawn 自己的协程
    asio::io_context *io_ctx = nullptr;

    // 当前订阅者的快照。长期协程每轮重连会换掉它，业务线程访问请通过 active_sub() 拿副本。
    std::shared_ptr<redis_subscriber> active_sub()
    {
        std::lock_guard<std::mutex> lk(active_sub_mtx_);
        return active_sub_;
    }
    // 框架内部：只有长期协程该调（业务别改）
    void set_active_sub(std::shared_ptr<redis_subscriber> sub)
    {
        std::lock_guard<std::mutex> lk(active_sub_mtx_);
        active_sub_ = std::move(sub);
    }

    // 业务主动停（任何线程安全：置 isclose + 订阅 stop）
    void stop()
    {
        isclose = true;
        // 副本出了锁再 stop()：不握着锁跑别人的代码
        if (auto s = active_sub())
            s->stop();
    }

  private:
    // 当前 async_subscribe 是否正在 pump 运行（供长期协程检测用）
    std::shared_ptr<redis_subscriber> active_sub_;
    std::mutex active_sub_mtx_;
};

}// namespace redis
}// namespace pz

#endif// ENABLE_REDIS_CLIENT

#endif
