#pragma once

// MQTT 5.0 业务示例。
// Client ID 约定: "xxx@@@group-device" → reg_key="xxx", group="group", device="device"
// 客户端必须使用 mosquitto_sub/pub -V 5（或其它 MQTT 5 客户端）连接。

#include <chrono>
#include <cstdio>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include "mqtt_api.h"
#include "mqtt_broker.h"
#include "terminal_color.h"

namespace http
{

// ========== 协程版 ==========
class async_my_test_mqtt : public mqtt_api
{
  public:
    using mqtt_api::mqtt_api;

    ~async_my_test_mqtt() override = default;

    bool is_coroutine() const override { return true; }

    // ====== 认证（必须覆盖，否则基类默认拒绝连接）======
    // async_on_auth 默认 co_return false（deny-by-default），这里放行演示链路。
    // CONNECT 阶段的协程版认证：is_coroutine()=true 时框架调这个而不是同步 on_auth，
    // 所以这里可以 co_await 查用户名/密码（下表用 orm 的异步读做形状演示）。
    // 返回 false → CONNACK 0x87 + 断链，和同步版一模一样，只是不占协程线程。
    asio::awaitable<bool> async_on_auth() override
    {
        if (client_info_.username.empty())
            co_return true;// 演示链路不带凭据
        if (client_info_.password.empty())
        {
            DEBUG_LOG("[MQTT-CO] auth rejected: empty password for user=%s",
                      client_info_.username.c_str());
            co_return false;
        }
        // 例：
        //   auto u = orm::mqtt::Localuser();
        //   u.eqUsername(client_info_.username);
        //   co_await u.async_fetch_one();
        //   co_return u.effect() > 0 && u.data.password == client_info_.password;
        co_return true;
    }

    // ====== 主题级授权（默认放行所有主题，生产环境替换为真实 ACL）======
    // MQTT 与 HTTP 同端口、无独立开关，基类默认匿名放行所有主题。
    // 这里覆盖钩子但默认 co_return true，留作接入真实 ACL 的切入点。
    // 完整说明见 mqtt_api.h 中 on_auth 上方的注释。
    //
    // 主题级授权的协程版孪生：is_coroutine()=true 时框架调这两个而不是同步版。
    // 价值就在于这里可以 co_await——授权表在数据库/Redis 里时不必阻塞协程线程。
    // 语义与同步版一致：false → SUBACK/PUBACK/PUBREC 回 0x87 且不入库、不转发、不投 retained。
    asio::awaitable<bool> async_on_can_subscribe(std::string_view topic_filter, uint8_t qos) override
    {
        (void)qos;
        // 演示分支：主题 throw/sub/... 从这里抛出。协程版和同步版结果相同——框架把抛出当成
        // "业务没给出结论"，这个 filter 的 SUBACK 照样回 0x87，本连接不会被踢掉，
        // 错误日志里记一行 "mqtt async_on_can_subscribe <异常正文> <ip> <port>"。
        if (topic_filter.starts_with("throw/sub/"))
        {
            throw std::runtime_error("demo: async_on_can_subscribe threw");
        }
        // 示例 ACL：只拦 acl/denied/ 前缀，其余放行。真业务一般用白名单，表外 co_return false。
        // 例：查授权表
        //   auto acl = orm::mqtt::Acl();
        //   acl.eqDevice(client_info_.device).eqPrefix(std::string(topic_filter));
        //   co_await acl.async_count();
        //   if (acl.effect() == 0) co_return false;
        // if (topic_filter.starts_with("acl/denied/"))
        // {
        //     DEBUG_LOG("[MQTT-CO] sub denied topic=%.*s",
        //               static_cast<int>(topic_filter.size()), topic_filter.data());
        //     co_return false;
        // }
        co_return true;
    }

    asio::awaitable<bool> async_on_can_publish(std::string_view topic, std::string_view payload, uint8_t qos) override
    {
        (void)qos;
        (void)payload;
        // 演示分支：与同步版 on_can_publish 同名前缀 throw/arm/，行为也一样——抛出按"业务否决"
        // 收口（QoS1 回 PUBACK 0x87、QoS2 回 PUBREC 0x87、QoS0 静默丢），连接不断。
        if (topic.starts_with("throw/arm/"))
        {
            throw std::runtime_error("demo: async_on_can_publish threw");
        }
        // 示例 ACL：只拦 acl/denied/ 前缀，其余放行。
        // 这里是协程，发布前可以先做一件异步的事再回话，例如落一条审计流水。
        // if (topic.starts_with("acl/denied/"))
        // {
        //     DEBUG_LOG("[MQTT-CO] pub denied topic=%.*s payload=%zuB",
        //               static_cast<int>(topic.size()), topic.data(), payload.size());
        //     co_return false;
        // }
        co_return true;
    }

    asio::awaitable<void> async_on_connect() override
    {
        DEBUG_LOG("[MQTT-CO] async_on_connect clientid=%s", client_info_.client_id.c_str());
        broker_subscribe("chat/#", 1, /*no_local=*/true);
        broker_subscribe("sys/online", 0);
        // 启用协程版周期 tick：isloopco=true → tick 把 async_run_loop co_spawn 到本连接 strand
        isloopco = true;
        durtime  = 1;
        loop_num = 4;
        co_return;
    }

    asio::awaitable<void> async_run_loop() override
    {
        if (loop_num == 0)
        {
            co_return;
        }
        tick_seq++;
        std::string payload = "async tick ";
        payload.append(std::to_string(tick_seq));
        payload.append(" from ");
        payload.append(client_info_.client_id);
        broker_publish("demo/async_tick", payload, 0);
        loop_num--;
        co_return;
    }

    asio::awaitable<void> async_on_message(std::string_view topic, std::string_view payload, uint8_t qos) override
    {
        (void)topic;
        (void)payload;
        (void)qos;// Release 档 DEBUG_LOG 编掉后不留未用参数
        // 本文件不 include 同步版那份头，所以这里不调 demo_detail::log_line，自己截一条
        constexpr size_t kMaxLog = 256;
        std::string_view brief   = payload.substr(0, kMaxLog);
        (void)brief;
        DEBUG_LOG("[MQTT-CO] topic=%.*s qos=%u payload(%zu/%zu)=%.*s",
                  static_cast<int>(topic.size()),
                  topic.data(),
                  qos,
                  brief.size(),
                  payload.size(),
                  static_cast<int>(brief.size()),
                  brief.data());
        // 这里可以 co_await 异步 ORM 查询，再 broker_publish 结果
        // 演示分支：主题 anchor/arm/... 从这条协程钩子里读取执行线程指纹，用作对照——
        // 协程版钩子仍在 io 线程上跑（asio awaitable 要 io_context 驱动），而同步版钩子
        // 交给业务线程池（见 mqtt/my_test_mqtt.hpp 的 sleep/arm/），两边的线程指纹应当完全不相交。
        if (topic.starts_with("anchor/arm/"))
        {
            broker_publish("probe/anchor",
                           "tid=" + std::to_string(
                                        std::hash<std::thread::id>{}(std::this_thread::get_id())),
                           0);
        }
        co_return;
    }

    asio::awaitable<void> async_on_subscribe(std::string_view topic, uint8_t qos) override
    {
        (void)topic;
        (void)qos;
        DEBUG_LOG("[MQTT-CO] async_on_subscribe topic=%.*s qos=%u",
                  static_cast<int>(topic.size()),
                  topic.data(),
                  qos);
        co_return;
    }

    // 取消订阅通知的协程版：is_coroutine()=true 时框架调这个而不是同步 on_unsubscribe。
    // 价值就在于这里可以 co_await —— 把"谁退了哪一层"落到库里不必阻塞协程线程。
    asio::awaitable<void> async_on_unsubscribe(std::string_view topic) override
    {
        // 例：
        //   auto sub = orm::mqtt::Subscription();
        //   sub.eqDevice(client_info_.device).eqTopic(std::string(topic));
        //   co_await sub.async_remove();
        std::string who = client_id();
        who.push_back(' ');
        who.append(topic);
        DEBUG_LOG("[MQTT-CO] async_on_unsubscribe %s", who.c_str());
        broker_publish("sys/unsub", who, 0);
        co_return;
    }

    // 断开通知的协程版。约束同同步版：会话已经 cleanup（订阅全摘、client_id 已注销），
    // 这里不能再读写本连接，只能做纯业务收尾；下面那颗定时器就是"收尾真的可以挂起"的证明
    // （真业务换成 co_await 一条异步 UPDATE），它不会卡住随后同 client_id 的新连接。
    asio::awaitable<void> async_on_disconnect() override
    {
        asio::steady_timer timer(co_await asio::this_coro::executor);
        timer.expires_after(std::chrono::milliseconds(50));
        co_await timer.async_wait(asio::use_awaitable);
        DEBUG_LOG("[MQTT-CO] async_on_disconnect clientid=%s", client_info_.client_id.c_str());
        broker_publish("sys/offline", client_id(), 0);
        co_return;
    }

  private:
    unsigned int tick_seq = 0;// async_run_loop 轮次计数（strand 上访问）
};

}// namespace http
