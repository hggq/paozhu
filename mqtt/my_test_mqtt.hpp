#pragma once

// MQTT 5.0 业务示例。
// Client ID 约定: "xxx@@@group-device" → reg_key="xxx", group="group", device="device"
// 客户端必须使用 mosquitto_sub/pub -V 5（或其它 MQTT 5 客户端）连接。

#include <atomic>
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

namespace demo_detail
{
inline void log_line(const char *tag, std::string_view topic, uint8_t qos, std::string_view payload)
{
    // payload 截断防日志膨胀
    constexpr size_t kMaxLog = 256;
    std::string_view brief   = payload.substr(0, kMaxLog);
    (void)tag;
    (void)topic;
    (void)qos;
    (void)brief;
    DEBUG_LOG("[%s] topic=%.*s qos=%u payload(%zu/%zu)=%.*s", tag, static_cast<int>(topic.size()), topic.data(), qos, brief.size(), payload.size(), static_cast<int>(brief.size()), brief.data());
}

// 执行线程的指纹。同步钩子跑在业务线程池上、协程钩子跑在 io 线程上，
// 把两边的指纹对一下就知道这段代码到底落在哪条线程上（下面 sleep/arm/ 在用）。
inline std::string tid_text()
{
    return std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
}
}// namespace demo_detail

// ========== 同步版 ==========
class my_test_mqtt : public mqtt_api
{
  public:
    using mqtt_api::mqtt_api;

    ~my_test_mqtt() override = default;

    // ====== 认证（必须覆盖，否则基类默认拒绝连接）======
    // on_auth 默认 return false（deny-by-default），这里返回 true 放行演示链路。
    // 真业务代码：在这里查用户名/密码或 token，查不过就 return false，
    //            身份字段都在 client_info_ 里（username / password / client_id）。
    // 同步版跑在 CONNECT 协程里，里面别发阻塞 IO——要查库就把这个类改成
    // is_coroutine()=true，实现 async_on_auth（协程版演示在 mqtt/async_my_test_mqtt.hpp）。
    bool on_auth() override
    {
        if (client_info_.username.empty())
            return true;
        if (client_info_.password.empty())
        {
            DEBUG_LOG("[MQTT] auth rejected: empty password for user=%s",
                      client_info_.username.c_str());
            return false;
        }
        return true;
    }

    // ====== 主题级授权（默认放行所有主题，生产环境替换为真实 ACL）======
    // MQTT 与 HTTP 同端口、无独立开关，基类默认匿名放行所有主题。
    // 这里覆盖钩子但默认 return true，留作接入真实 ACL 的切入点。
    // 完整说明见 mqtt_api.h 中 on_auth 上方的注释。
    //
    // 框架只给否决通道，策略表长什么样是业务的事：
    // 调用时机：客户端发来 SUBSCRIBE / PUBLISH 报文时，一条 SUBSCRIBE 里的每个
    //          filter 各判一次（逐条独立，不整包拒）。
    // 业务自己 broker_subscribe / broker_publish 主动收发的不过这两个钩子，
    // 否则业务发出去的消息被自己的钩子拒掉，就成了自锁。
    // 设备身份不用参数传：读 client_info_（client_id / reg_key / group / device）。
    // 返回 false 之后协议答复由框架负责，业务不用管：
    //   on_can_subscribe → 该 filter 的 SUBACK 回 0x87，不写进订阅表，也不投 retained
    //   on_can_publish   → 不转发给任何订阅者、不写 retained；QoS1 回 PUBACK 0x87，
    //                      QoS2 回 PUBREC 0x87（报文照常收完，只是不投递），QoS0 静默丢弃
    bool on_can_subscribe(std::string_view topic_filter, uint8_t qos) override
    {
        (void)qos;// qos 也在手里：可以只允许某些主题订到 QoS1
        // 演示分支：主题 throw/sub/... 从这里抛出，看框架怎么收口——本连接不会被踢掉，
        // 这个 filter 的 SUBACK 按"业务否决"回 0x87（和直接 return false 结果相同），
        // 并往错误日志里记一行 "mqtt on_can_subscribe <异常正文> <ip> <port>"。
        // 同步钩子跑在业务线程池上，抛出的东西由框架捕获，不会串到别的连接。
        if (topic_filter.starts_with("throw/sub/"))
        {
            throw std::runtime_error("demo: on_can_subscribe threw");
        }
        // 示例 ACL：只拦 acl/denied/ 前缀，其余放行。真业务一般用白名单，表外 return false。
        // if (topic_filter.starts_with("acl/denied/"))
        // {
        //     DEBUG_LOG("[MQTT] sub denied topic=%.*s",
        //               static_cast<int>(topic_filter.size()), topic_filter.data());
        //     return false;
        // }
        return true;
    }

    bool on_can_publish(std::string_view topic, std::string_view payload, uint8_t qos) override
    {
        (void)qos;
        (void)payload;
        // 演示分支：主题 throw/arm/... 从这里抛出，看框架怎么收口——本连接不会被踢掉，
        // 这条报文按"业务否决"处理（QoS1 回 PUBACK 0x87、QoS2 回 PUBREC 0x87、QoS0 静默丢），
        // 并往错误日志里记一行 "mqtt on_can_publish <异常正文> <ip> <port>"。
        // 同步钩子现在跑在业务线程池上，抛出的东西由框架捕获，不会串到别的连接。
        if (topic.starts_with("throw/arm/"))
        {
            throw std::runtime_error("demo: on_can_publish threw");
        }
        // payload 同样给到手：命令类主题可以在这儿校验载荷形状，不合法就不转发。
        // 示例 ACL：只拦 acl/denied/ 前缀，其余放行。
        // if (topic.starts_with("acl/denied/"))
        // {
        //     DEBUG_LOG("[MQTT] pub denied topic=%.*s payload=%zuB",
        //               static_cast<int>(topic.size()), topic.data(), payload.size());
        //     return false;
        // }
        return true;
    }

    void on_connect() override
    {
        DEBUG_LOG("[MQTT] on_connect sync clientid=%s", client_info_.client_id.c_str());
        broker_subscribe("chat/#", 1, /*no_local=*/true);
        // 按 tick 间隔无限循环推 5 种颜色（durtime=5 ⇒ 约 5 秒一条，一圈 25 秒）
        durtime  = 5;
        loop_num = 100000;
    }

    // tick 线程直调（isloopco=false）
    void run_loop() override
    {
        if (loop_num == 0)
        {
            return;
        }
        tick_seq++;
        static const char *kColors[] = {"red", "blue", "yellow", "#800080", "off"};
        const char *color            = kColors[(tick_seq - 1) % 5];
        broker_publish("/led/set", color, 0);
        DEBUG_LOG("[MQTT] run_loop[%d] publish /led/set -> %s", tick_seq, color);
        loop_num--;
    }

    void on_subscribe(std::string_view topic, uint8_t qos) override
    {
        (void)topic;
        (void)qos;
        DEBUG_LOG("[MQTT] on_subscribe topic=%.*s qos=%u",
                  static_cast<int>(topic.size()),
                  topic.data(),
                  qos);
    }

    void on_message(std::string_view topic, std::string_view payload, uint8_t qos) override
    {
        demo_detail::log_line("MQTT", topic, qos, payload);

        // 演示分支：主题 sleep/arm/... 在这里阻塞 700 ms，模拟一次查库或调下游。
        // 同步钩子跑在业务线程池（clientrunpool）上，所以这种阻塞是可以用在这里的——
        // 它占的是池里一条线程，本连接的协程在 await 期间是挂起的，不占 io 线程；
        // 同一个连接上连发多条会各占一条池线程并行跑（回读 payload 里的 tid 互不相同）。
        // 回读走 probe/sleep：seq 是全局第几次触发，ms 是实测阻塞时长，tid 是执行线程指纹，
        // 拿它与协程版（mqtt/async_my_test_mqtt.hpp 的 anchor/arm/）的 tid 对照即可判归属。
        if (topic.starts_with("sleep/arm/"))
        {
            static std::atomic<unsigned int> arm_seq = 0;
            const auto t0                            = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(700));
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - t0)
                                .count();
            broker_publish("probe/sleep",
                           "seq=" + std::to_string(++arm_seq) + " ms=" + std::to_string(ms) +
                               " tid=" + demo_detail::tid_text(),
                           0);
        }

        // 业务侧主动向 broker 发布（broker_publish 现在是真实实现）
        std::string echo_topic = "echo/";
        echo_topic.append(topic);
        broker_publish(echo_topic, payload, 0);
    }

    // 取消订阅通知：broker 已经把这层订阅摘掉了，UNSUBACK 随后照回，这里只做业务收尾。
    // "谁在取消"不用参数传：client_id() / reg_key() / group() / device() 都读得到。
    // 真业务代码在这里把订阅关系落库（DELETE 或标成已退订）。
    // 演示做法是发一条 sys/unsub 审计帧（payload = "谁 退了哪一层"），
    // 外部客户端订 sys/unsub 就能看见 —— 注意别只写在日志里，Release 档 DEBUG_LOG 会被编掉。
    void on_unsubscribe(std::string_view topic) override
    {
        std::string who = client_id();
        who.push_back(' ');
        who.append(topic);
        DEBUG_LOG("[MQTT] on_unsubscribe %s", who.c_str());
        broker_publish("sys/unsub", who, 0);
    }

    // 断开通知：跑在本连接所有清理之后（订阅已全摘、client_id 已注销），
    // 不能再对这条连接做任何读写，只能做纯业务收尾：掉线落库、离线通知。
    void on_disconnect() override
    {
        DEBUG_LOG("[MQTT] on_disconnect clientid=%s", client_info_.client_id.c_str());
        broker_publish("sys/offline", client_id(), 0);
    }

  private:
    unsigned int tick_seq = 0;// run_loop 轮次计数（仅 tick 线程访问）
};

}// namespace http
