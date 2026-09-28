#ifndef HTTP_MQTT_SESSION_H
#define HTTP_MQTT_SESSION_H

// 单条 MQTT 连接的会话状态机：
//   * 读缓冲 + 帧切分（嗅探阶段预读数据不丢）
//   * keepalive 与框架 time_limit 联动，到期自动断
//   * 入站 / 出站 inflight 表，配额上限防 OOM
//   * 出站走 post_write 串行化，析构时清理订阅与索引

#include <asio.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "client_session.h"
#include "mqtt_frame.h"
#include "mqtt_framing.h"

namespace http
{

class mqtt_session : public std::enable_shared_from_this<mqtt_session>
{
  public:
    // 已从 socket 读取的一帧
    struct packet
    {
        uint8_t fixed         = 0;
        mqtt_packet_type type = mqtt_packet_type::reserved;
        std::vector<uint8_t> body;
        // 组帧时使用的原位 body 视图
        const uint8_t *body_data() const { return body.data(); }
        size_t body_size() const { return body.size(); }
    };

    // 配额 / 缓冲默认值
    static constexpr size_t kReadChunk             = 4096;
    static constexpr size_t kMaxInboundQos2        = 1024;// 等待 PUBREL 的消息数上限
    static constexpr size_t kMaxOutboundInflight   = 1024;
    static constexpr size_t kMaxTopicAliasMappings = 512;
    // 字节配额：仅按条数不足以防 OOM（单帧可达 MQTT_MAX_PACKET_SIZE=16MB），叠加总字节上限。
    // 口径：入站暂存按 mqtt_publish_info::accounted_bytes()（topic + payload + 属性块，
    //       只算 payload 会被 16MB 的 user_properties 绕过）；出站按 mqtt_inflight_bytes()
    //       （outbound_ 只登记 topic + 共享 payload）。
    // 超出时 store_inbound_qos2 / deliver 直接拒绝，由调用方决定丢弃策略。
    static constexpr size_t kMaxInboundQos2Bytes  = 16 * 1024 * 1024;
    static constexpr size_t kMaxOutboundBytes     = 16 * 1024 * 1024;
    // 挂起队列上限 = CONST_MQTT_SESSION_BODY_SIZE（common/cost_define.h，默认 2MB）：
    // 超了丢最旧、新帧总要发，见 enqueue_pending()

    mqtt_session(std::shared_ptr<client_session> sess, const unsigned char *pre, size_t prelen);
    ~mqtt_session();

    // make_shared 之后必须调用一次：建立 self_ 弱引用
    void init();

    // ============ 读 ============
    // 三态拆帧：仅 need_more 继续读；malformed/too_large 由 read_packet 回 DISCONNECT 后断链
    mqtt_frame_state read_packet_from_buffer(packet &out);
    asio::awaitable<std::optional<packet>> read_packet();

    // ============ 写 ============
    // 统一走 client_session 的写队列 → 与本连接其它写操作严格串行（不重叠 async_write）
    bool write(std::vector<uint8_t> pkt);
    bool write(std::string_view raw);

    // ============ 生命周期 ============
    void set_client_info(const mqtt_client_info &info) { info_ = info; }
    const mqtt_client_info &client_info() const { return info_; }
    const std::string &client_id() const { return info_.client_id; }

    void touch_activity();               // 刷新 time_limit，实现 keepalive 语义
    uint16_t effective_keepalive() const;// CONNACK 宣告后的最终值
    // 本连接入站单帧上限（CONNACK 协商后设置；0 或超协议上限则维持协议上限）
    void set_max_inbound_packet(size_t v)
    {
        if (v > 0 && v <= static_cast<size_t>(MQTT_MAX_PACKET_SIZE)) max_in_packet_ = v;
    }
    void set_server_keep_alive(uint16_t v)
    {
        server_keep_alive_override_ = v;
        touch_activity();
    }
    // CONNACK 实际宣告的 Topic Alias Maximum（服务端入站方向），供 resolve_inbound_alias 校验
    void set_server_topic_alias_max(uint16_t v) { server_topic_alias_max_ = v; }

    // 幂等清理：broker 订阅 + client_id 索引
    void cleanup();

    bool is_closed() const;

    // ============ QoS 状态 ============

    // 入站 QoS2：收到 PUBLISH 暂存，收到 PUBREL 取出
    bool store_inbound_qos2(uint16_t packet_id, const mqtt_publish_info &pub);
    std::optional<mqtt_publish_info> take_inbound_qos2(uint16_t packet_id);

    // 出站 QoS1/2：投递 PUBLISH 时登记，收到 PUBACK/PUBREC/PUBCOMP 时清理
    // 带 props 的重载会把可转发的 MQTT 5 属性写进出站 PUBLISH（§3.3.2.3）
    // 发送环满时成品帧挂本会话 pending 队列，由全局分发协程（httpserver::mqtt_send_loop）
    // 按 FIFO 回灌（http2 生产者 standby 同款）。队列界 = CONST_MQTT_SESSION_BODY_SIZE：
    // 超界丢最旧、新帧总要，所以本方法只在连接已关 / inflight 配额满 / 帧超过客户端
    // 宣告的 Maximum Packet Size 时返回 false。
    bool deliver(const std::string &topic, const mqtt_payload_ptr &payload, uint8_t qos, bool retain);
    bool deliver(const std::string &topic, const mqtt_payload_ptr &payload, uint8_t qos, bool retain, const mqtt_publish_props &props);
    void ack_outgoing(mqtt_packet_type ack_type, uint16_t packet_id, mqtt_reason rc);

    // 把 pending 队列按 FIFO 回灌发送环，环再次满即停（持 out_mu_ 调 post_write 安全，
    // 跨线程调用安全：post_write 入环本就是线程安全出口）
    // ——调用者：分发协程 httpserver::mqtt_send_loop（慢/暂停读者的兜底通道），
    //   以及本会话读循环检查点（对端刚消费/确认过报文，此时它的环多半腾了槽）；
    //   返回本轮真正灌进环的帧数
    size_t flush_pending();
    bool has_pending() const;

    // Topic Alias（入站方向）
    bool resolve_inbound_alias(mqtt_publish_info &pub);

    size_t inbound_qos2_count() const { return inbound_qos2_.size(); }
    size_t outbound_inflight_count() const
    {
        std::lock_guard<std::mutex> lk(out_mu_);
        return outbound_.size();
    }

    // 因写队列满被拒、或挂起队列超容量被挤掉的最旧帧数（N19 诊断用）
    uint64_t dropped_write_count() const { return dropped_writes_; }

    // ============ broker 订阅计数（订阅数配额用）============
    // 只在 mqtt_broker 持有其 mu_ 时读写，故本类不再单独加锁；
    // 计数住在会话对象里，会话销毁即随之消失，broker 侧无需额外清理陈旧计数。
    size_t subscription_count() const { return sub_count_; }
    void on_broker_subscribe(size_t added) { sub_count_ += added; }
    void on_broker_unsubscribe(size_t removed)
    {
        sub_count_ = (sub_count_ > removed) ? (sub_count_ - removed) : 0;
    }
    // 当前挂起队列字节数（判"有界"用的诊断读数）
    size_t pending_bytes() const
    {
        std::lock_guard<std::mutex> lk(out_mu_);
        return pending_bytes_;
    }

    const std::shared_ptr<client_session> &transport() const { return sess_; }

  private:
    // 发送环满时暂存的成品帧（FIFO，回灌时整帧入环）
    struct pending_frame
    {
        std::vector<uint8_t> bytes;
        uint16_t             pid = 0;// qos0 为 0
        uint8_t              qos = 0;
    };

    std::shared_ptr<client_session> sess_;
    std::weak_ptr<mqtt_session> self_;

    mqtt_client_info info_;

    std::vector<uint8_t> buf_;
    size_t buf_pos_ = 0;

    // outbound_ 与 pending_/pending_bytes_ 共用这把锁——deliver 在发布方线程、
    // ack_outgoing 在订阅方 strand，同锁收口了 inflight 表的跨线程写
    mutable std::mutex out_mu_;
    std::unordered_map<uint16_t, mqtt_publish_info> inbound_qos2_;
    std::unordered_map<uint16_t, mqtt_publish_info> outbound_;
    size_t inbound_qos2_bytes_ = 0;// inbound_qos2_ 中 accounted_bytes() 总字节
    size_t outbound_bytes_     = 0;// outbound_ 中 mqtt_inflight_bytes() 总字节
    size_t sub_count_          = 0;// broker 订阅表里属于本会话的 filter 条数（broker 在 mu_ 下维护）
    std::unordered_map<uint16_t, std::string> alias_to_topic_;
    std::deque<pending_frame> pending_;
    size_t pending_bytes_ = 0;

    uint16_t next_packet_id();
    void compact_buffer();
    bool ensure_buffer(size_t need);
    // 成品帧挂起队列（调用方必须持 out_mu_）：新帧一定受理，
    // 总量超过 CONST_MQTT_SESSION_BODY_SIZE 时从最旧端丢弃直到回到界内
    void enqueue_pending(std::vector<uint8_t> &&pkt, uint16_t pid, uint8_t qos);

    uint16_t server_keep_alive_override_ = 0;// CONNACK Server Keep Alive（0 = 不覆盖）
    size_t   max_in_packet_ = MQTT_MAX_PACKET_SIZE;// 入站单帧上限，默认协议值
    uint16_t server_topic_alias_max_ = MQTT_DEFAULT_TOPIC_ALIAS_MAX;// CONNACK 实际宣告值
    bool cleaned_                        = false;

    uint64_t dropped_writes_ = 0;// 入环被拒 / 挂起队列挤旧的丢弃计数（N19）
};

}// namespace http

#endif
