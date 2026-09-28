#ifndef HTTP_MQTT_API_H
#define HTTP_MQTT_API_H

// MQTT 业务基类。业务代码继承本类并按需 override 回调。
// broker_publish / on_connect / on_message / on_subscribe 等由 broker 框架调用。

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "mqtt_broker.h"
#include "mqtt_frame.h"
#include "mqtt_session.h"

namespace http
{

// 单调秒时钟：用于 Message Expiry / retained 过期计算。
// 放在 class mqtt_api 之前，因为类内的 inline 便捷方法要用到它。
namespace mqtt_detail
{
inline uint64_t now_monotonic_sec()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<seconds>(steady_clock::now().time_since_epoch()).count());
}
}// namespace mqtt_detail

class mqtt_api : public std::enable_shared_from_this<mqtt_api>
{
  public:
    mqtt_api(const mqtt_client_info &info, std::shared_ptr<mqtt_session> session)
        : client_info_(info), session_(std::move(session))
    {
    }
    virtual ~mqtt_api() = default;

    // ====== 模式 ======
    // true → 框架调用 async_on_xxx；false → 调用同步 on_xxx
    virtual bool is_coroutine() const { return false; }

    // ====== CONNECT 阶段 ======
    // 【与 HTTP 同端口，无独立 MQTT 端口/开关】
    // MQTT 没有独立监听端口，复用 HTTP 端口按首字节 0x10 分流。
    // 认证默认拒绝（deny-by-default）：on_auth / async_on_auth 默认 return false，
    // 业务 Handler 必须覆盖并返回 true 才放行连接——否则匿名连接一律 0x87 拒绝。
    // 主题级授权（on_can_subscribe / on_can_publish）默认放行所有主题，
    // 生产环境必须覆盖这两个钩子做 ACL。
    // 参考实现见 mqtt/my_test_mqtt.hpp（同步）和 mqtt/async_my_test_mqtt.hpp（协程），
    // 其中 on_auth 必须保留（否则连不上），on_can_subscribe / on_can_publish 示例默认注释掉。
    //
    // on_auth 返回 false → CONNACK not_authorized 并关闭连接。
    // on_auth 在 CONNECT 协程里被同步直调：实现里做阻塞 IO 会占住当前协程线程。
    // 要查库/查 Redis 校验凭据的用 async_on_auth（is_coroutine()=true 时框架调它），
    // 其余场景用同步版即可。
    virtual bool on_auth() { return false; }
    virtual asio::awaitable<bool> async_on_auth() { co_return false; }

    // ====== 授权否决（只挂在客户端报文入口，业务主动出口不过这里）======
    // 默认 true = 框架不做任何主题级判定，与 on_auth 一样是"业务不实现就不检查"。
    // 设备身份不用参数传：实现方读自己的 client_info_（client_id/reg_key/group/device）。
    // 返回 false → 该 filter 的 SUBACK 回 not_authorized(0x87)，不写入 broker 订阅表，
    //              也不给它投 retained
    virtual bool on_can_subscribe(std::string_view topic_filter, uint8_t qos)
    {
        (void)topic_filter;
        (void)qos;
        return true;
    }
    virtual asio::awaitable<bool> async_on_can_subscribe(std::string_view topic_filter, uint8_t qos)
    {
        (void)topic_filter;
        (void)qos;
        co_return true;
    }
    // 返回 false → 不转发、不写 retained。QoS1 回 PUBACK 0x87，QoS2 回 PUBREC 0x87
    // （两者都仍要把报文收完，只是不投递），QoS0 静默丢弃
    virtual bool on_can_publish(std::string_view topic, std::string_view payload, uint8_t qos)
    {
        (void)topic;
        (void)payload;
        (void)qos;
        return true;
    }
    virtual asio::awaitable<bool> async_on_can_publish(std::string_view topic, std::string_view payload, uint8_t qos)
    {
        (void)topic;
        (void)payload;
        (void)qos;
        co_return true;
    }

    // ====== CONNECT 完成后 ======
    virtual void on_connect() {}
    virtual asio::awaitable<void> async_on_connect() { co_return; }

    // ====== 收到客户端 PUBLISH ======
    virtual void on_message(std::string_view topic, std::string_view payload, uint8_t qos) {}
    virtual asio::awaitable<void> async_on_message(std::string_view topic, std::string_view payload, uint8_t qos) { co_return; }

    // ====== 收到 SUBSCRIBE / UNSUBSCRIBE ======
    virtual void on_subscribe(std::string_view topic, uint8_t qos) {}
    virtual asio::awaitable<void> async_on_subscribe(std::string_view topic, uint8_t qos)
    {
        (void)topic;
        (void)qos;
        co_return;
    }
    // 取消订阅：UNSUBSCRIBE 报文解析通过后、回 UNSUBACK 之前，逐条 filter 回调。
    // 纯通知，不接受否决（退订本身不需要授权，UNSUBACK 一律 0x00）。
    // 谁在取消不用参数传：实现方读 client_id() / client_info_（reg_key/group/device）。
    // 同步版跑在本连接的协程里，里面别发阻塞 IO——要查库（回收授权、订阅关系落库）
    // 就实现 async_on_unsubscribe（is_coroutine()=true 时框架调它）。
    virtual void on_unsubscribe(std::string_view topic) {}
    virtual asio::awaitable<void> async_on_unsubscribe(std::string_view topic)
    {
        (void)topic;
        co_return;
    }

    // ====== 断开（含异常断开，由框架保证必达）======
    // 调用点在本连接所有清理之后：会话已退订、已从 broker 注销，钩子里不得再读写该连接，
    // 只能做纯业务收尾（掉线落库、离线通知）。身份仍可读 client_id()/client_info_。
    // 要 co_await 查库的实现 async_on_disconnect；默认两个都是空实现，不 override 无行为变化。
    virtual void on_disconnect() {}
    virtual asio::awaitable<void> async_on_disconnect() { co_return; }

    // ====== 周期 tick（对齐 websockets_api / socket_api 的 tick 结构，默认不启用）======
    // isloopco：true → tick 把 async_run_loop() co_spawn 到本连接 strand_；
    //          false → tick 线程直接调用 run_loop()。
    //          与 is_coroutine()（只管 on_connect/on_message 族）互相独立。
    // durtime ：每 fps % durtime == 0 执行一轮；fps 只在跨过 1 秒时才 +1，
    //          所以分频单位是秒——durtime=5 即约 5 秒一轮，与 tick 内层 4.1fps 无关。
    // loop_num：剩余轮数，>0 才被 tick 扫描，归零即摘除；
    //          须在 on_connect（首次 tick 扫描之前）置数，否则会以 0 被摘除。
    // 同步版 run_loop() 跑在 tick 线程：只许调用 broker_publish / broker_subscribe /
    // close 这类线程安全出口（最终走带锁发送环），不得触碰会话读缓冲与协议状态。
    bool isloopco = false;
    unsigned int durtime = 8;
    unsigned int loop_num = 0;
    virtual void run_loop() {}
    virtual asio::awaitable<void> async_run_loop() { co_return; }

    // ====== 业务便捷方法 ======

    // 订阅：走 broker 唯一路径，与客户端 SUBSCRIBE 报文的处理完全一致
    // 返回 false 的两种情形：topic filter 非法，或订阅数配额已满
    //（会话级 kMaxSubscriptionsPerSession / 全局 kMaxSubscriptionsTotal）。
    bool broker_subscribe(std::string_view topic, uint8_t qos = 0, bool no_local = false, bool retain_as_published = false, uint8_t retain_handling = 0)
    {
        if (!session_)
            return false;
        return mqtt_broker::subscribe_ok(mqtt_broker::instance().subscribe(
            client_info_.reg_key,
            client_info_.group,
            client_info_.device,
            std::string(topic),
            qos,
            no_local,
            retain_as_published,
            retain_handling,
            session_));
    }

    void broker_unsubscribe(std::string_view topic)
    {
        if (!session_)
            return;
        mqtt_broker::instance().unsubscribe(std::string(topic), session_);
    }

    // 主动向 broker 发布消息。是否回发给自身由目标订阅的 No Local 决定
    // （No Local=1 不回发，No Local=0 回发 —— MQTT 5 §3.8.3.1）。
    // MQTT 5 语义：retain=true 且 payload 非空时会写入/更新 retained 存储；
    // payload 为空且 retain=true 时清除该 topic 的 retained 消息。
    // 返回 false 仅表示「retain 落库被配额拒绝」（普通投递不受影响，仍已完成）；
    // 框架据此回 0x97，业务侧可忽略。
    bool broker_publish(std::string_view topic, std::string_view payload, uint8_t qos = 0, bool retain = false)
    {
        mqtt_publish_info pub;
        pub.topic.assign(topic);
        pub.payload = std::make_shared<std::string>(payload);
        pub.qos         = qos;
        pub.retain      = retain;
        pub.received_at = mqtt_detail::now_monotonic_sec();
        return publish_to_broker(pub, session_);
    }

    // 供框架内部复用：把消息投递给除 exclude 之外的所有匹配订阅者。
    // 返回 false = 请求了 retain 但 retained 落库被配额拒绝（见 broker_publish 说明）。
    static bool publish_to_broker(const mqtt_publish_info &pub,
                                  const std::shared_ptr<mqtt_session> &exclude)
    {
        return publish_impl(pub, exclude);
    }

    // ====== 其它 ======
    void close()
    {
        if (session_)
        {
            session_->transport()->isclose = true;
        }
    }

    // 底层传输连接（tick 侧取 strand / 判活用；未建会话时为空）
    std::shared_ptr<client_session> transport() const
    {
        return session_ ? session_->transport() : nullptr;
    }

    const mqtt_client_info &client_info() const { return client_info_; }
    const std::string &client_id() const { return client_info_.client_id; }
    const std::string &reg_key() const { return client_info_.reg_key; }
    const std::string &group() const { return client_info_.group; }
    const std::string &device() const { return client_info_.device; }

    // 内部实现：过期检查 → 匹配订阅 → 逐个投递 → 按需写入 retained
    // 返回 false = retained 落库被配额拒绝（投递本身已完成）
    static bool publish_impl(const mqtt_publish_info &pub,
                             const std::shared_ptr<mqtt_session> &exclude);

  protected:
    mqtt_client_info client_info_;
    std::shared_ptr<mqtt_session> session_;
};

inline bool mqtt_api::publish_impl(const mqtt_publish_info &pub,
                                   const std::shared_ptr<mqtt_session> &exclude)
{
    if (pub.qos > 2)
        return false;
    if (!is_valid_topic_name(pub.topic))
        return false;

    uint64_t now_sec = mqtt_detail::now_monotonic_sec();

    mqtt_publish_props out_props;
    if (!forward_publish_props(pub, now_sec, out_props))
    {
        // 消息在服务端停留期间已过期：不投递、也不写 retained
        // 写进去等于让过期消息带着新的 created_at 复活。
        // 唯一例外是 retain + 空 payload 的「清除 retained」语义——与是否过期无关，永远执行。
        if (pub.retain && (!pub.payload || pub.payload->empty()))
            return mqtt_broker::instance().retain(pub, now_sec);
        return true;
    }

    auto targets = mqtt_broker::instance().publish(pub.topic, pub.qos, exclude);
    for (auto &d : targets)
    {
        if (!d.session)
            continue;
        // Retain As Published 为 false 时，转发给订阅者的 retain 标志清零
        bool out_retain = pub.retain && d.retain_as_published;
        d.session->deliver(pub.topic, pub.payload, d.out_qos, out_retain, out_props);
    }

    if (pub.retain)
    {
        // payload 为空表示清除该 topic 的 retained 消息。
        // 返回 false = retained 库配额已满而拒绝落库 → 调用方用 0x97 回 PUBACK/PUBCOMP，
        // 不再像以前那样静默丢弃却回 success。
        return mqtt_broker::instance().retain(pub, now_sec);
    }
    return true;
}

}// namespace http

#endif
