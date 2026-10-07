#ifndef PZ_MQTT_SUBPUB_CLIENT_H
#define PZ_MQTT_SUBPUB_CLIENT_H
/*
 * 框架级 MQTT 长连接客户端基类（出站 — 连接外部 broker）
 *
 * 业务侧继承本类，实现 section_name() / on_open / on_message / on_close / run_loop。
 * 启动时 server.cpp 遍历注册表 co_spawn async_mqtt_subpub_loop，负责：
 *   async_tcp_connect → async_mqtt_connect → pump 回调分发 → 断连重连（3→6→9→12 指数退避）
 * websocket_loop 每秒拍扫 mqtt_clients，按 durtime 分频调 run_loop/async_run_loop。
 *
 * 线程模型完全对称 ws_subpub_client / sock_subpub_client：
 *   is_coroutine_ = false  →  收到 broker PUBLISH 丢 clientrunpool 调 on_message
 *   is_coroutine_ = true   →  收到 broker PUBLISH co_spawn 到 io_context 调 async_on_message
 */
#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "http_mqtt_client.h"

#ifdef ENABLE_MQTT_CLIENT

namespace http
{

class mqtt_subpub_client : public std::enable_shared_from_this<mqtt_subpub_client>
{
  public:
    mqtt_subpub_client()          = default;
    virtual ~mqtt_subpub_client() = default;

    // ===== 业务实现 =====
    virtual std::string section_name() const = 0;// 对应 conf/mqtt.conf 的段名

    // 同步钩子（on_message / run_loop 跑业务线程池；on_open / on_close 由框架 loop 直接调）
    virtual void on_open() {}
    virtual void on_close() {}
    virtual void on_message(std::string_view topic, std::string_view payload, uint8_t qos) {}
    virtual void run_loop() {}

    // 协程钩子（全部跑 io_context 新协程）
    virtual asio::awaitable<void> async_on_open() { co_return; }
    virtual asio::awaitable<void> async_on_close() { co_return; }
    virtual asio::awaitable<void> async_on_message(std::string_view topic, std::string_view payload, uint8_t qos) { co_return; }
    virtual asio::awaitable<void> async_run_loop() { co_return; }

    // ===== 业务主动操作 =====

    // 发布消息（MQTT broker 客户端风格；内部走 mqtt_client::async_publish 的 write_mu_ 串行化）
    asio::awaitable<bool> async_publish(std::string_view topic,
                                        std::string_view payload,
                                        uint8_t qos = 0,
                                        bool retain = false)
    {
        auto mqc = conn();
        if (!mqc || !mqc->sock)
            co_return false;
        co_return co_await mqc->async_publish(topic, payload, qos, retain);
    }
    // 订阅主题（通常在 on_open / async_on_open 里调）
    asio::awaitable<bool> async_subscribe(std::string_view filter,
                                          uint8_t qos              = 1,
                                          bool no_local            = false,
                                          bool retain_as_published = false,
                                          uint8_t retain_handling  = 0)
    {
        auto mqc = conn();
        if (!mqc || !mqc->sock)
            co_return false;
        co_return co_await mqc->async_subscribe(filter, qos, no_local, retain_as_published, retain_handling);
    }
    // 取消订阅
    asio::awaitable<bool> async_unsubscribe(std::string_view filter)
    {
        auto mqc = conn();
        if (!mqc || !mqc->sock)
            co_return false;
        co_return co_await mqc->async_unsubscribe(filter);
    }

    // ===== 活跃连接的快照 =====
    // 常驻循环在 io_context 线程上换/清这条连接，业务在 tick 线程或业务线程池上读它 ——
    // shared_ptr 本身不是原子的，裸读裸写就是数据竞争。拿副本用是安全的。
    std::shared_ptr<http::mqtt_client> conn()
    {
        std::lock_guard<std::mutex> lk(conn_mtx_);
        return mqc_;
    }
    // 框架内部：只有常驻循环该调（业务别改）
    void set_conn(std::shared_ptr<http::mqtt_client> mqc)
    {
        std::lock_guard<std::mutex> lk(conn_mtx_);
        mqc_ = std::move(mqc);
    }

    // 业务主动收摊（任意线程）：置旗 + 关掉这条 broker 连接。
    // mqtt_client::close_connect() 自己会把动作投递进它的 strand_，所以这里从 tick 线程
    // 调也是安全的；pump 正 parked 在 async_read_some 上，靠那一句里的 cancel 才被叫醒。
    void stop()
    {
        isclose = true;
        // 副本出了锁再关：不握着锁跑别人的代码
        if (auto mqc = conn())
            mqc->close_connect();
    }

    // ===== 框架字段（业务构造时设置）=====
    bool is_coroutine_   = false; // 钩子走协程版
    bool is_loop_co_     = false; // run_loop 走协程版
    unsigned int durtime = 8;     // tick 分频（1 拍 = 1s）
    int loop_num         = 999999;// tick 剩余次数（0 = 擦除）

    // ===== 框架内部字段（只读，业务别改）=====
    asio::io_context *io_ctx  = nullptr;// 框架注入
    std::atomic<bool> isclose = false;

  private:
    std::shared_ptr<http::mqtt_client> mqc_;// 活跃连接，只能经 conn()/set_conn() 碰
    std::mutex conn_mtx_;
};

}// namespace http

#endif// ENABLE_MQTT_CLIENT
#endif
