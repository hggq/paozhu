#ifndef HTTP_MQTT_FRAME_H
#define HTTP_MQTT_FRAME_H

// MQTT v5.0 codec layer (pure binary, no I/O, unit-testable).
// 本实现只支持 MQTT v5.0：protocol name = "MQTT" 且 level = 5。
// 非 v5 的 CONNECT 一律由 parse_connect 回 mqtt_reason::unsupported_protocol_version。

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace http
{

// ===================== 基础常量 =====================

static constexpr uint8_t MQTT_PROTOCOL_LEVEL    = 5;
static constexpr const char *MQTT_PROTOCOL_NAME = "MQTT";

// 服务端能力默认值（在 CONNACK 中向客户端宣告）
static constexpr uint16_t MQTT_DEFAULT_RECEIVE_MAXIMUM = 16;// QoS>0 并发上限
static constexpr uint8_t MQTT_DEFAULT_MAXIMUM_QOS      = 2;
static constexpr uint8_t MQTT_DEFAULT_TOPIC_ALIAS_MAX  = 0;                  // 暂不提供出站别名
static constexpr size_t MQTT_MAX_PACKET_SIZE           = 16u * 1024u * 1024u;// 16MB 入站上限

enum class mqtt_packet_type : uint8_t
{
    reserved    = 0,
    CONNECT     = 1,
    CONNACK     = 2,
    PUBLISH     = 3,
    PUBACK      = 4,
    PUBREC      = 5,
    PUBREL      = 6,
    PUBCOMP     = 7,
    SUBSCRIBE   = 8,
    SUBACK      = 9,
    UNSUBSCRIBE = 10,
    UNSUBACK    = 11,
    PINGREQ     = 12,
    PINGRESP    = 13,
    DISCONNECT  = 14,
    AUTH        = 15
};

inline const char *mqtt_packet_type_name(mqtt_packet_type t)
{
    switch (t)
    {
    case mqtt_packet_type::CONNECT: return "CONNECT";
    case mqtt_packet_type::CONNACK: return "CONNACK";
    case mqtt_packet_type::PUBLISH: return "PUBLISH";
    case mqtt_packet_type::PUBACK: return "PUBACK";
    case mqtt_packet_type::PUBREC: return "PUBREC";
    case mqtt_packet_type::PUBREL: return "PUBREL";
    case mqtt_packet_type::PUBCOMP: return "PUBCOMP";
    case mqtt_packet_type::SUBSCRIBE: return "SUBSCRIBE";
    case mqtt_packet_type::SUBACK: return "SUBACK";
    case mqtt_packet_type::UNSUBSCRIBE: return "UNSUBSCRIBE";
    case mqtt_packet_type::UNSUBACK: return "UNSUBACK";
    case mqtt_packet_type::PINGREQ: return "PINGREQ";
    case mqtt_packet_type::PINGRESP: return "PINGRESP";
    case mqtt_packet_type::DISCONNECT: return "DISCONNECT";
    case mqtt_packet_type::AUTH: return "AUTH";
    default: return "RESERVED";
    }
}

// MQTT v5 Reason Code
enum class mqtt_reason : uint8_t
{
    success                      = 0x00,
    granted_qos1                 = 0x01,
    granted_qos2                 = 0x02,
    disconnect_with_will_message = 0x04,
    no_matching_subscribers      = 0x10,
    no_subscription_existed      = 0x11,
    continue_authentication      = 0x18,
    reauthenticate               = 0x19,

    unspecified_error                   = 0x80,
    malformed_packet                    = 0x81,
    protocol_error                      = 0x82,
    implementation_specific_error       = 0x83,
    unsupported_protocol_version        = 0x84,
    client_identifier_not_valid         = 0x85,
    bad_user_name_or_password           = 0x86,
    not_authorized                      = 0x87,
    server_unavailable                  = 0x88,
    server_busy                         = 0x89,
    banned                              = 0x8A,
    server_shutting_down                = 0x8B,
    bad_authentication_method           = 0x8C,
    keep_alive_timeout                  = 0x8D,
    session_taken_over                  = 0x8E,
    topic_filter_invalid                = 0x8F,
    topic_name_invalid                  = 0x90,
    packet_identifier_in_use            = 0x91,
    packet_identifier_not_found         = 0x92,
    receive_maximum_exceeded            = 0x93,
    topic_alias_invalid                 = 0x94,
    packet_too_large                    = 0x95,
    message_rate_too_high               = 0x96,
    quota_exceeded                      = 0x97,
    administrative_action               = 0x98,
    payload_format_invalid              = 0x99,
    retain_not_supported                = 0x9A,
    qos_not_supported                   = 0x9B,
    use_another_server                  = 0x9C,
    server_moved                        = 0x9D,
    shared_subscription_not_supported   = 0x9E,
    connection_rate_exceeded            = 0x9F,
    maximum_connect_time                = 0xA0,
    subscription_id_not_supported       = 0xA1,
    wildcard_subscription_not_supported = 0xA2
};

inline bool is_error_reason(mqtt_reason r)
{
    return static_cast<uint8_t>(r) >= 0x80;
}

// MQTT v5 Property Identifier
enum class mqtt_prop : uint8_t
{
    payload_format_indicator          = 0x01,
    message_expiry_interval           = 0x02,
    content_type                      = 0x03,
    response_topic                    = 0x08,
    correlation_data                  = 0x09,
    subscription_identifier           = 0x0B,
    session_expiry_interval           = 0x11,
    assigned_client_identifier        = 0x12,
    server_keep_alive                 = 0x13,
    authentication_method             = 0x15,
    authentication_data               = 0x16,
    request_problem_information       = 0x17,
    will_delay_interval               = 0x18,
    request_response_information      = 0x19,
    response_information              = 0x1A,
    server_reference                  = 0x1C,
    reason_string                     = 0x1F,
    receive_maximum                   = 0x21,
    topic_alias_maximum               = 0x22,
    topic_alias                       = 0x23,
    maximum_qos                       = 0x24,
    retain_available                  = 0x25,
    user_property                     = 0x26,
    maximum_packet_size               = 0x27,
    wildcard_subscription_available   = 0x28,
    subscription_identifier_available = 0x29,
    shared_subscription_available     = 0x2A
};

// Subscription Options（SUBSCRIBE 每个 filter 一字节）
struct mqtt_subscription_options
{
    uint8_t qos              = 0;
    bool no_local            = false;// NL
    bool retain_as_published = false;// RAP
    uint8_t retain_handling  = 0;    // 0/1/2
};

// ===================== 数据结构 =====================

struct mqtt_user_property
{
    std::string key;
    std::string value;
};

struct mqtt_will_message
{
    bool has    = false;
    uint8_t qos = 0;
    bool retain = false;
    std::string topic;
    std::string payload;

    uint32_t will_delay_interval     = 0;
    uint32_t message_expiry_interval = 0;
    bool has_payload_format          = false;
    uint8_t payload_format_indicator = 0;
    std::string content_type;
    std::string response_topic;
    std::string correlation_data;
    std::vector<mqtt_user_property> user_properties;
};

struct mqtt_connect_props
{
    uint32_t session_expiry_interval  = 0;
    uint16_t receive_maximum          = 65535;
    uint32_t maximum_packet_size      = 0;// 0 = 不限制
    uint16_t topic_alias_maximum      = 0;
    bool request_response_information = false;
    bool request_problem_information  = true;
    std::string authentication_method;
    std::string authentication_data;
    std::vector<mqtt_user_property> user_properties;
};

struct mqtt_client_info
{
    // ---- 协议字段 ----
    std::string client_id;
    std::string username;
    std::string password;// 二进制安全
    uint16_t keepalive = 60;
    bool clean_start   = true;
    mqtt_will_message will;
    mqtt_connect_props props;

    // ---- 框架派生字段（由 client_id 解析而来）----
    std::string reg_key;// @@@ 之前
    std::string group;  // @@@ 之后、第一个 '-' 之前
    std::string device; // 之后
};

struct mqtt_publish_props
{
    bool has_payload_format          = false;
    uint8_t payload_format_indicator = 0;
    bool has_message_expiry          = false;
    uint32_t message_expiry_interval = 0;
    bool has_topic_alias             = false;
    uint16_t topic_alias             = 0;
    std::string response_topic;
    std::string correlation_data;
    std::string content_type;
    std::vector<uint32_t> subscription_identifiers;
    std::vector<mqtt_user_property> user_properties;

    // 属性块实际占用的字节（只算变长部分，标量字段可忽略）。
    // 用途：内存配额口径。user_properties 单项上限 65535 字节、总长只受帧上限
    // （MQTT_MAX_PACKET_SIZE=16MB）约束，所以只按 payload 记账会被属性块绕过配额。
    size_t accounted_bytes() const
    {
        size_t n = response_topic.size() + correlation_data.size() + content_type.size();
        n += subscription_identifiers.size() * sizeof(uint32_t);
        for (const auto &p : user_properties)
            n += p.key.size() + p.value.size();
        return n;
    }
};

// 共享 payload：一次发布的同一份字节在多个订阅者的 inflight / retained 间共享引用，
// 避免 N 份拷贝放大内存（单条 payload 可达 MQTT_MAX_PACKET_SIZE=16MB）。
using mqtt_payload_ptr = std::shared_ptr<const std::string>;

struct mqtt_publish_info
{
    uint8_t qos = 0;
    bool dup    = false;
    bool retain = false;
    std::string topic;
    uint16_t packet_id = 0;
    mqtt_payload_ptr payload;
    mqtt_publish_props props;
    // 单调秒（0 = 未知）。转发时用于把 Message Expiry Interval 换算成"剩余寿命"
    // （MQTT 5 §3.3.2.3.3：转发出去的必须是接收值减去在服务端停留的时间）。
    uint64_t received_at = 0;

    // 常驻内存记账口径：topic + payload + 属性块。
    // 入站 QoS2 暂存与 broker retained 都按本对象整份保存，所以配额必须用这个口径；
    // 只算 payload 的话，构造「1 字节 payload + 16MB user_properties」即可绕过配额。
    size_t accounted_bytes() const
    {
        return topic.size() + (payload ? payload->size() : 0) + props.accounted_bytes();
    }
};

// 出站 inflight 记账口径：inbound 那条口径不适用于 outbound_——
// outbound_ 里只登记 topic + 共享 payload（不含 props，帧里那份是瞬时的、已由发送环限界），
// 所以登记与回收都必须用这个函数，避免两侧口径不一致导致字节账永久漂移。
inline size_t mqtt_inflight_bytes(std::string_view topic, const mqtt_payload_ptr &payload)
{
    return topic.size() + (payload ? payload->size() : 0);
}

struct mqtt_subscribe_entry
{
    std::string topic;
    mqtt_subscription_options options;
    // false → 该 filter 非法：SUBACK 逐条回 0x8F topic_filter_invalid，
    //         但不得因此切断连接（MQTT 5 §3.9.3）
    bool valid = true;
};

struct mqtt_unsubscribe_entry
{
    std::string topic;
    // false → 该 filter 非法：UNSUBACK 回 0x8F topic_filter_invalid
    bool valid = true;
};

// CONNACK 中服务端宣告的能力
struct mqtt_connack_props
{
    uint32_t session_expiry_interval       = 0;
    uint16_t receive_maximum               = MQTT_DEFAULT_RECEIVE_MAXIMUM;
    uint32_t maximum_packet_size           = static_cast<uint32_t>(MQTT_MAX_PACKET_SIZE);
    uint16_t topic_alias_maximum           = MQTT_DEFAULT_TOPIC_ALIAS_MAX;
    uint8_t maximum_qos                    = MQTT_DEFAULT_MAXIMUM_QOS;
    bool retain_available                  = true;
    bool wildcard_subscription_available   = true;
    bool subscription_identifier_available = true;
    bool shared_subscription_available     = true;
    uint16_t server_keep_alive             = 0;// 0 = 不覆盖客户端 keepalive
    std::string assigned_client_identifier;
    std::string response_information;
    std::string reason_string;// 仅当客户端 Request Problem Information=1 时下发
    std::vector<mqtt_user_property> user_properties;
};

// ===================== 剩余长度 =====================

void encode_remaining_length(std::vector<uint8_t> &out, size_t len);
// 返回消耗的字节数；数据不足或编码非法返回 0
size_t decode_remaining_length(const uint8_t *data, size_t maxlen, size_t &value);

// ===================== 原语读取 =====================
// 所有 true 表示成功。失败时不推进 off（或由调用方终止解析）。

bool read_u8(const uint8_t *d, size_t len, size_t &off, uint8_t &v);
bool read_u16(const uint8_t *d, size_t len, size_t &off, uint16_t &v);
bool read_u32(const uint8_t *d, size_t len, size_t &off, uint32_t &v);
// 变长整数（最多 4 字节）
bool read_varint(const uint8_t *d, size_t len, size_t &off, uint32_t &v);
void put_varint(std::vector<uint8_t> &out, uint32_t v);
// UTF-8 字符串：长度 + 内容（校验 UTF-8 且禁止 U+0000）
bool read_utf8_string(const uint8_t *d, size_t len, size_t &off, std::string &out);
// 二进制数据：长度 + 内容（不做字符集校验）
bool read_binary(const uint8_t *d, size_t len, size_t &off, std::string &out);
void put_utf8_string(std::vector<uint8_t> &out, std::string_view s);
void put_binary(std::vector<uint8_t> &out, std::string_view s);

bool is_valid_utf8(std::string_view s);

// ===================== 主题合法性 =====================

// 主题名（PUBLISH）：不得含通配符与 U+0000，长度 >= 1
bool is_valid_topic_name(std::string_view topic);
// 主题过滤器（SUBSCRIBE/UNSUBSCRIBE）：'#' 只能在末尾且独占一层，'+' 独占一层
bool is_valid_topic_filter(std::string_view filter);

// ===================== Property 游标 =====================
// 解析时：把整个 Property 块当作只读游标遍历，未识别的 property 也能被安全跳过。

class mqtt_prop_cursor
{
  public:
    mqtt_prop_cursor(const uint8_t *d, size_t len, size_t off);

    bool valid() const { return ok_; }
    size_t position() const { return off_; }// cursor 当前位置（Property 块之后，含后续字节）

    // 推进到下一个 property；返回 false 表示结束（正常）或畸形（bad() 为 true）
    bool next(mqtt_prop &id);
    bool bad() const { return bad_; }

    bool read_u8(uint8_t &v);
    bool read_u16(uint16_t &v);
    bool read_u32(uint32_t &v);
    bool read_varint32(uint32_t &v);
    bool read_str(std::string &v);
    bool read_bin(std::string &v);
    // 跳过当前 property 的值（依据 id 的既定类型）
    void skip_value(mqtt_prop id);

  private:
    const uint8_t *data_ = nullptr;
    size_t off_          = 0;
    size_t end_          = 0;
    bool ok_             = false;
    bool bad_            = false;
    bool finished_       = false;
};

// 构造 Property 块：先按需写入 (id,value)，最后 encode 到目标 buffer
class mqtt_prop_builder
{
  public:
    void u8(mqtt_prop id, uint8_t v);
    void u16(mqtt_prop id, uint16_t v);
    void u32(mqtt_prop id, uint32_t v);
    void varint(mqtt_prop id, uint32_t v);
    void str(mqtt_prop id, std::string_view v);
    void bin(mqtt_prop id, std::string_view v);
    void pair(std::string_view k, std::string_view v);

    bool empty() const { return buf_.empty(); }
    const std::vector<uint8_t> &data() const { return buf_; }
    // 写入 "变长长度 + 内容"（内容为空时仍写入长度 0 —— MQTT 5 中该字段可选，
    // 调用方通过 with_props 开关决定是否写入 Property Length）
    void encode_to(std::vector<uint8_t> &out) const;

  private:
    std::vector<uint8_t> buf_;
};

// ===================== 报文解析 =====================

// CONNECT：非 MQTT 5 / 格式错误 → false + reject 给出应回的 reason code
bool parse_connect(const uint8_t *body, size_t len, mqtt_client_info &out, mqtt_reason &reject);

// Client ID → reg_key + group + device（格式 "xxx@@@group-device"）
bool parse_client_id(const std::string &raw, mqtt_client_info &out);

bool parse_publish(uint8_t fixed_header, const uint8_t *body, size_t len, mqtt_publish_info &out);
bool parse_subscribe(const uint8_t *body, size_t len, uint16_t &packet_id, std::vector<mqtt_subscribe_entry> &out);
bool parse_unsubscribe(const uint8_t *body, size_t len, uint16_t &packet_id, std::vector<mqtt_unsubscribe_entry> &out);

// 确认类报文（PUBACK/PUBREC/PUBREL/PUBCOMP/DISCONNECT/AUTH）。
// 结构为 [packet_id(部分类型)] [reason code(可选)] [properties(可选)]
bool parse_ack(mqtt_packet_type type, const uint8_t *body, size_t len, uint16_t &packet_id, mqtt_reason &reason, bool &has_packet_id);

// ===================== 报文构造 =====================

std::vector<uint8_t> make_connack(bool session_present, mqtt_reason rc, const mqtt_connack_props &props, bool with_reason_string = true);
std::vector<uint8_t> make_suback(uint16_t packet_id, const std::vector<mqtt_reason> &codes);
std::vector<uint8_t> make_unsuback(uint16_t packet_id, const std::vector<mqtt_reason> &codes);
std::vector<uint8_t> make_puback(uint16_t packet_id, mqtt_reason rc);
std::vector<uint8_t> make_pubrec(uint16_t packet_id, mqtt_reason rc);
std::vector<uint8_t> make_pubrel(uint16_t packet_id, mqtt_reason rc);
std::vector<uint8_t> make_pubcomp(uint16_t packet_id, mqtt_reason rc);
std::vector<uint8_t> make_pingresp();
std::vector<uint8_t> make_disconnect(mqtt_reason rc);

// 转发 PUBLISH。qos>0 时必须提供非 0 packet_id。
// props 为 nullptr 时不写任何属性；非 nullptr 时按 MQTT 5 §3.3.2.3 写出可转发的属性。
// 不转发的属性：
//   Topic Alias(0x23)               —— 别名是连接方向独立的，出站不得复用入站映射
//   Subscription Identifier(0x0B)   —— 由服务端按匹配到的订阅重新生成，不得沿用客户端带来的值
std::vector<uint8_t> make_publish(std::string_view topic, std::string_view payload, uint8_t qos, uint16_t packet_id = 0, bool retain = false, const mqtt_publish_props *props = nullptr);

// 把 PUBLISH 属性写入 builder（只写可转发的属性），供 make_publish 复用
void put_publish_props(mqtt_prop_builder &pb, const mqtt_publish_props &props);

// 入站 PUBLISH 属性 → 出站 PUBLISH 属性（MQTT 5 §3.3.2.3）：
//   * 保留 payload_format / content_type / response_topic / correlation_data / user_properties
//   * Message Expiry Interval 换算为剩余寿命：interval - (now_sec - received_at)
//   * 丢弃 topic_alias（别名是连接方向独立的）与客户端带来的 subscription_identifiers
// 返回 false 表示消息在服务端停留期间已过期，按 §3.3.2.3.3 不应再投递。
bool forward_publish_props(const mqtt_publish_info &pub, uint64_t now_sec, mqtt_publish_props &out);

// 底层核心：原始 props + 已等待秒数 → 出站 props。
// brokering（用 received_at 算 waited）和 retained（用 created_at 算 waited）共用。
// 外部不应直接调用 forward_publish_props — 除非你已经算出 waited 秒数。
bool forward_publish_props_core(const mqtt_publish_props &in, uint64_t waited, mqtt_publish_props &out);

}// namespace http

#endif
