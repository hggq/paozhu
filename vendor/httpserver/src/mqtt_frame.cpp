#include "mqtt_frame.h"

#include <algorithm>

namespace http
{

// ===================== 剩余长度 =====================

void encode_remaining_length(std::vector<uint8_t> &out, size_t len)
{
    do
    {
        uint8_t b = static_cast<uint8_t>(len % 128);
        len /= 128;
        if (len > 0) b |= 0x80;
        out.push_back(b);
    } while (len > 0);
}

size_t decode_remaining_length(const uint8_t *data, size_t maxlen, size_t &value)
{
    value = 0;
    size_t multiplier = 1;
    for (size_t i = 0; i < 4 && i < maxlen; i++)
    {
        uint8_t b = data[i];
        value += (b & 0x7F) * multiplier;
        if (!(b & 0x80))
        {
            if (value > 0x0FFFFFFF) return 0; // 超过协议上限 268435455
            return i + 1;
        }
        multiplier *= 128;
    }
    return 0; // 超过 4 字节仍未结束 → 畸形
}

// ===================== UTF-8 校验 =====================

bool is_valid_utf8(std::string_view s)
{
    size_t i = 0;
    const size_t n = s.size();
    while (i < n)
    {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t extra = 0;
        uint32_t cp = 0;
        if (c < 0x80)
        {
            if (c == 0x00) return false; // U+0000 不允许
            i++;
            continue;
        }
        else if ((c & 0xE0) == 0xC0) { extra = 1; cp = c & 0x1F; }
        else if ((c & 0xF0) == 0xE0) { extra = 2; cp = c & 0x0F; }
        else if ((c & 0xF8) == 0xF0) { extra = 3; cp = c & 0x07; }
        else return false;

        if (i + extra >= n) return false;
        for (size_t k = 1; k <= extra; k++)
        {
            unsigned char cc = static_cast<unsigned char>(s[i + k]);
            if ((cc & 0xC0) != 0x80) return false;
            cp = (cp << 6) | (cc & 0x3F);
        }
        // 拒绝过长编码、代理区与超出 U+10FFFF
        if (extra == 1 && cp < 0x80) return false;
        if (extra == 2 && cp < 0x800) return false;
        if (extra == 3 && cp < 0x10000) return false;
        if (cp >= 0xD800 && cp <= 0xDFFF) return false;
        if (cp > 0x10FFFF) return false;
        // 控制字符（U+0001-U+001F、U+007F-U+009F）在 MQTT 字符串中不推荐使用，
        // 这里按规范允许但业务通常不用，不做拒绝。
        i += extra + 1;
    }
    return true;
}

// ===================== 原语 =====================

bool read_u8(const uint8_t *d, size_t len, size_t &off, uint8_t &v)
{
    if (off + 1 > len) return false;
    v = d[off++];
    return true;
}

bool read_u16(const uint8_t *d, size_t len, size_t &off, uint16_t &v)
{
    if (off + 2 > len) return false;
    v = static_cast<uint16_t>((static_cast<uint16_t>(d[off]) << 8) | d[off + 1]);
    off += 2;
    return true;
}

bool read_u32(const uint8_t *d, size_t len, size_t &off, uint32_t &v)
{
    if (off + 4 > len) return false;
    v = (static_cast<uint32_t>(d[off]) << 24) | (static_cast<uint32_t>(d[off + 1]) << 16) |
        (static_cast<uint32_t>(d[off + 2]) << 8) | static_cast<uint32_t>(d[off + 3]);
    off += 4;
    return true;
}

bool read_varint(const uint8_t *d, size_t len, size_t &off, uint32_t &v)
{
    uint32_t multiplier = 1;
    uint32_t value = 0;
    for (int i = 0; i < 4; i++)
    {
        if (off >= len) return false;
        uint8_t b = d[off++];
        value += (b & 0x7F) * multiplier;
        if (!(b & 0x80))
        {
            v = value;
            return true;
        }
        multiplier *= 128;
    }
    return false;
}

void put_varint(std::vector<uint8_t> &out, uint32_t v)
{
    do
    {
        uint8_t b = static_cast<uint8_t>(v % 128);
        v /= 128;
        if (v > 0) b |= 0x80;
        out.push_back(b);
    } while (v > 0);
}

bool read_utf8_string(const uint8_t *d, size_t len, size_t &off, std::string &out)
{
    uint16_t sl = 0;
    if (!read_u16(d, len, off, sl)) return false;
    if (off + sl > len) return false;
    std::string_view sv(reinterpret_cast<const char *>(d + off), sl);
    if (!is_valid_utf8(sv)) return false;
    out.assign(sv);
    off += sl;
    return true;
}

bool read_binary(const uint8_t *d, size_t len, size_t &off, std::string &out)
{
    uint16_t sl = 0;
    if (!read_u16(d, len, off, sl)) return false;
    if (off + sl > len) return false;
    out.assign(reinterpret_cast<const char *>(d + off), sl);
    off += sl;
    return true;
}

void put_utf8_string(std::vector<uint8_t> &out, std::string_view s)
{
    out.push_back(static_cast<uint8_t>((s.size() >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(s.size() & 0xFF));
    out.insert(out.end(), s.begin(), s.end());
}

void put_binary(std::vector<uint8_t> &out, std::string_view s)
{
    put_utf8_string(out, s);
}

// ===================== 主题合法性 =====================

bool is_valid_topic_name(std::string_view topic)
{
    if (topic.empty()) return false;
    if (topic.size() > 65535) return false;
    if (!is_valid_utf8(topic)) return false;
    // 主题名中不得出现通配符（MQTT 5: topic name MUST NOT contain wildcard characters）
    for (char c : topic)
    {
        if (c == '+' || c == '#') return false;
    }
    return true;
}

bool is_valid_topic_filter(std::string_view filter)
{
    if (filter.empty()) return false;
    if (filter.size() > 65535) return false;
    if (!is_valid_utf8(filter)) return false;

    // 共享订阅 $share/{ShareName}/{filter}（§4.8.2）：ShareName 不得为空、不得含 '+' '#',
    // 且必须真的带 {filter} 部分。这些是协议非法形态，交给调用方逐条回 0x8F。
    constexpr std::string_view kShare = "$share/";
    if (filter.size() >= kShare.size() && filter.compare(0, kShare.size(), kShare) == 0)
    {
        size_t second = filter.find('/', kShare.size());
        if (second == std::string_view::npos || second == kShare.size()) return false;
        if (second + 1 >= filter.size()) return false;
        std::string_view group = filter.substr(kShare.size(), second - kShare.size());
        if (group.find('+') != std::string_view::npos ||
            group.find('#') != std::string_view::npos)
            return false;
    }

    // 按层拆分并检查
    std::string_view rest = filter;
    while (!rest.empty())
    {
        size_t pos = rest.find('/');
        std::string_view level = (pos == std::string_view::npos) ? rest : rest.substr(0, pos);
        bool last = (pos == std::string_view::npos);

        if (!level.empty())
        {
            bool has_hash = level.find('#') != std::string_view::npos;
            bool has_plus = level.find('+') != std::string_view::npos;
            if (has_hash)
            {
                // '#' 必须独占该层
                if (level.size() != 1) return false;
                // 且必须是最后一层
                if (!last) return false;
            }
            if (has_plus)
            {
                // '+' 必须独占该层
                if (level.size() != 1) return false;
            }
        }
        if (last) break;
        rest = rest.substr(pos + 1);
    }
    return true;
}

// ===================== Property 游标 =====================

mqtt_prop_cursor::mqtt_prop_cursor(const uint8_t *d, size_t len, size_t off)
    : data_(d), off_(off)
{
    uint32_t plen = 0;
    if (!read_varint(d, len, off_, plen))
    {
        ok_ = false;
        return;
    }
    if (off_ + plen > len)
    {
        ok_ = false;
        return;
    }
    end_ = off_ + plen;
    ok_ = true;
}

bool mqtt_prop_cursor::next(mqtt_prop &id)
{
    if (!ok_ || bad_ || finished_) return false;
    if (off_ >= end_)
    {
        finished_ = true;
        return false;
    }
    uint8_t raw = data_[off_++];
    id = static_cast<mqtt_prop>(raw);
    return true;
}

bool mqtt_prop_cursor::read_u8(uint8_t &v)
{
    if (!ok_) return false;
    if (!http::read_u8(data_, end_, off_, v)) { bad_ = true; ok_ = false; return false; }
    return true;
}

bool mqtt_prop_cursor::read_u16(uint16_t &v)
{
    if (!ok_) return false;
    if (!http::read_u16(data_, end_, off_, v)) { bad_ = true; ok_ = false; return false; }
    return true;
}

bool mqtt_prop_cursor::read_u32(uint32_t &v)
{
    if (!ok_) return false;
    if (!http::read_u32(data_, end_, off_, v)) { bad_ = true; ok_ = false; return false; }
    return true;
}

bool mqtt_prop_cursor::read_varint32(uint32_t &v)
{
    if (!ok_) return false;
    if (!http::read_varint(data_, end_, off_, v)) { bad_ = true; ok_ = false; return false; }
    return true;
}

bool mqtt_prop_cursor::read_str(std::string &v)
{
    if (!ok_) return false;
    if (!http::read_utf8_string(data_, end_, off_, v)) { bad_ = true; ok_ = false; return false; }
    return true;
}

bool mqtt_prop_cursor::read_bin(std::string &v)
{
    if (!ok_) return false;
    if (!http::read_binary(data_, end_, off_, v)) { bad_ = true; ok_ = false; return false; }
    return true;
}

void mqtt_prop_cursor::skip_value(mqtt_prop id)
{
    switch (id)
    {
    case mqtt_prop::payload_format_indicator:
    case mqtt_prop::request_problem_information:
    case mqtt_prop::request_response_information:
    case mqtt_prop::retain_available:
    case mqtt_prop::wildcard_subscription_available:
    case mqtt_prop::subscription_identifier_available:
    case mqtt_prop::shared_subscription_available:
    case mqtt_prop::maximum_qos:
        { uint8_t t = 0; read_u8(t); break; }
    case mqtt_prop::receive_maximum:
    case mqtt_prop::topic_alias_maximum:
    case mqtt_prop::topic_alias:
    case mqtt_prop::server_keep_alive:
        { uint16_t t = 0; read_u16(t); break; }
    case mqtt_prop::message_expiry_interval:
    case mqtt_prop::session_expiry_interval:
    case mqtt_prop::will_delay_interval:
    case mqtt_prop::maximum_packet_size:
        { uint32_t t = 0; read_u32(t); break; }
    case mqtt_prop::subscription_identifier:
        { uint32_t t = 0; read_varint32(t); break; }
    case mqtt_prop::content_type:
    case mqtt_prop::response_topic:
    case mqtt_prop::correlation_data:
    case mqtt_prop::assigned_client_identifier:
    case mqtt_prop::authentication_method:
    case mqtt_prop::authentication_data:
    case mqtt_prop::response_information:
    case mqtt_prop::server_reference:
    case mqtt_prop::reason_string:
        { std::string t; read_str(t); break; }
    case mqtt_prop::user_property:
        { std::string k, v; read_str(k); read_str(v); break; }
    default:
        // 未知 property：无法安全跳过具体值，只能判定整块不可解析
        bad_ = true;
        ok_ = false;
        break;
    }
}

// ===================== Property 构造 =====================

void mqtt_prop_builder::u8(mqtt_prop id, uint8_t v)
{
    buf_.push_back(static_cast<uint8_t>(id));
    buf_.push_back(v);
}

void mqtt_prop_builder::u16(mqtt_prop id, uint16_t v)
{
    buf_.push_back(static_cast<uint8_t>(id));
    buf_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    buf_.push_back(static_cast<uint8_t>(v & 0xFF));
}

void mqtt_prop_builder::u32(mqtt_prop id, uint32_t v)
{
    buf_.push_back(static_cast<uint8_t>(id));
    buf_.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    buf_.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    buf_.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    buf_.push_back(static_cast<uint8_t>(v & 0xFF));
}

void mqtt_prop_builder::varint(mqtt_prop id, uint32_t v)
{
    buf_.push_back(static_cast<uint8_t>(id));
    put_varint(buf_, v);
}

void mqtt_prop_builder::str(mqtt_prop id, std::string_view v)
{
    buf_.push_back(static_cast<uint8_t>(id));
    put_utf8_string(buf_, v);
}

void mqtt_prop_builder::bin(mqtt_prop id, std::string_view v)
{
    str(id, v);
}

void mqtt_prop_builder::pair(std::string_view k, std::string_view v)
{
    buf_.push_back(static_cast<uint8_t>(mqtt_prop::user_property));
    put_utf8_string(buf_, k);
    put_utf8_string(buf_, v);
}

void mqtt_prop_builder::encode_to(std::vector<uint8_t> &out) const
{
    put_varint(out, static_cast<uint32_t>(buf_.size()));
    out.insert(out.end(), buf_.begin(), buf_.end());
}

// ===================== CONNECT =====================

bool parse_connect(const uint8_t *body, size_t len, mqtt_client_info &out, mqtt_reason &reject)
{
    reject = mqtt_reason::success;
    size_t off = 0;

    std::string proto_name;
    if (!read_utf8_string(body, len, off, proto_name))
    {
        reject = mqtt_reason::malformed_packet;
        return false;
    }

    uint8_t level = 0;
    if (!read_u8(body, len, off, level))
    {
        reject = mqtt_reason::malformed_packet;
        return false;
    }

    // 只认 MQTT 5.0
    if (proto_name != MQTT_PROTOCOL_NAME || level != MQTT_PROTOCOL_LEVEL)
    {
        reject = mqtt_reason::unsupported_protocol_version;
        return false;
    }

    uint8_t flags = 0;
    if (!read_u8(body, len, off, flags))
    {
        reject = mqtt_reason::malformed_packet;
        return false;
    }
    out.clean_start = (flags & 0x02) != 0;
    const bool has_will = (flags & 0x04) != 0;
    const uint8_t will_qos = (flags >> 3) & 0x03;
    const bool will_retain = (flags & 0x20) != 0;
    const bool has_password = (flags & 0x40) != 0;
    const bool has_username = (flags & 0x80) != 0;

    if (!read_u16(body, len, off, out.keepalive))
    {
        reject = mqtt_reason::malformed_packet;
        return false;
    }

    // ---- Property 块 ----
    {
        mqtt_prop_cursor cur(body, len, off);
        if (!cur.valid())
        {
            reject = mqtt_reason::malformed_packet;
            return false;
        }
        mqtt_prop id{};
        bool first_sess_expiry = true, first_recv_max = true, first_max_pkt = true;
        bool first_alias_max = true, first_req_resp = true, first_req_prob = true;
        bool first_auth_method = true, first_auth_data = true;
        while (cur.next(id))
        {
            switch (id)
            {
            case mqtt_prop::session_expiry_interval:
                if (first_sess_expiry) { if (!cur.read_u32(out.props.session_expiry_interval)) { reject = mqtt_reason::malformed_packet; return false; } first_sess_expiry = false; }
                else cur.skip_value(id);
                break;
            case mqtt_prop::receive_maximum:
                if (first_recv_max)
                {
                    if (!cur.read_u16(out.props.receive_maximum)) { reject = mqtt_reason::malformed_packet; return false; }
                    if (out.props.receive_maximum == 0) { reject = mqtt_reason::protocol_error; return false; }
                    first_recv_max = false;
                }
                else cur.skip_value(id);
                break;
            case mqtt_prop::maximum_packet_size:
                if (first_max_pkt)
                {
                    if (!cur.read_u32(out.props.maximum_packet_size)) { reject = mqtt_reason::malformed_packet; return false; }
                    if (out.props.maximum_packet_size == 0) { reject = mqtt_reason::protocol_error; return false; }
                    first_max_pkt = false;
                }
                else cur.skip_value(id);
                break;
            case mqtt_prop::topic_alias_maximum:
                if (first_alias_max) { if (!cur.read_u16(out.props.topic_alias_maximum)) { reject = mqtt_reason::malformed_packet; return false; } first_alias_max = false; }
                else cur.skip_value(id);
                break;
            case mqtt_prop::request_response_information:
                if (first_req_resp) { uint8_t v = 0; if (!cur.read_u8(v)) { reject = mqtt_reason::malformed_packet; return false; } out.props.request_response_information = v != 0; first_req_resp = false; }
                else cur.skip_value(id);
                break;
            case mqtt_prop::request_problem_information:
                if (first_req_prob) { uint8_t v = 1; if (!cur.read_u8(v)) { reject = mqtt_reason::malformed_packet; return false; } out.props.request_problem_information = v != 0; first_req_prob = false; }
                else cur.skip_value(id);
                break;
            case mqtt_prop::authentication_method:
                if (first_auth_method) { if (!cur.read_str(out.props.authentication_method)) { reject = mqtt_reason::malformed_packet; return false; } first_auth_method = false; }
                else cur.skip_value(id);
                break;
            case mqtt_prop::authentication_data:
                if (first_auth_data) { if (!cur.read_bin(out.props.authentication_data)) { reject = mqtt_reason::malformed_packet; return false; } first_auth_data = false; }
                else cur.skip_value(id);
                break;
            case mqtt_prop::user_property:
                {
                    mqtt_user_property up;
                    if (!cur.read_str(up.key) || !cur.read_str(up.value)) { reject = mqtt_reason::malformed_packet; return false; }
                    out.props.user_properties.push_back(std::move(up));
                    break;
                }
            default:
                cur.skip_value(id);
                break;
            }
            if (cur.bad())
            {
                reject = mqtt_reason::malformed_packet;
                return false;
            }
        }
        if (cur.bad())
        {
            reject = mqtt_reason::malformed_packet;
            return false;
        }
        // cursor 已推进到 Property 块末尾
        off = cur.position();
    }

    // ---- Client ID ----
    if (!read_utf8_string(body, len, off, out.client_id))
    {
        reject = mqtt_reason::malformed_packet;
        return false;
    }

    // ---- Will ----
    if (has_will)
    {
        if (will_qos > 2)
        {
            reject = mqtt_reason::malformed_packet;
            return false;
        }
        {
            mqtt_prop_cursor cur(body, len, off);
            if (!cur.valid()) { reject = mqtt_reason::malformed_packet; return false; }
            mqtt_prop id{};
            while (cur.next(id))
            {
                switch (id)
                {
                case mqtt_prop::will_delay_interval:
                    if (!cur.read_u32(out.will.will_delay_interval)) { reject = mqtt_reason::malformed_packet; return false; }
                    break;
                case mqtt_prop::payload_format_indicator:
                    if (!cur.read_u8(out.will.payload_format_indicator)) { reject = mqtt_reason::malformed_packet; return false; }
                    out.will.has_payload_format = true;
                    break;
                case mqtt_prop::message_expiry_interval:
                    if (!cur.read_u32(out.will.message_expiry_interval)) { reject = mqtt_reason::malformed_packet; return false; }
                    break;
                case mqtt_prop::content_type:
                    if (!cur.read_str(out.will.content_type)) { reject = mqtt_reason::malformed_packet; return false; }
                    break;
                case mqtt_prop::response_topic:
                    if (!cur.read_str(out.will.response_topic)) { reject = mqtt_reason::malformed_packet; return false; }
                    break;
                case mqtt_prop::correlation_data:
                    if (!cur.read_bin(out.will.correlation_data)) { reject = mqtt_reason::malformed_packet; return false; }
                    break;
                case mqtt_prop::user_property:
                    {
                        mqtt_user_property up;
                        if (!cur.read_str(up.key) || !cur.read_str(up.value)) { reject = mqtt_reason::malformed_packet; return false; }
                        out.will.user_properties.push_back(std::move(up));
                        break;
                    }
                default:
                    cur.skip_value(id);
                    break;
                }
                if (cur.bad()) { reject = mqtt_reason::malformed_packet; return false; }
            }
            if (cur.bad()) { reject = mqtt_reason::malformed_packet; return false; }
        }
        {
            uint32_t wplen = 0;
            if (!read_varint(body, len, off, wplen)) { reject = mqtt_reason::malformed_packet; return false; }
            off += wplen;
        }

        if (!read_utf8_string(body, len, off, out.will.topic)) { reject = mqtt_reason::malformed_packet; return false; }
        if (!is_valid_topic_name(out.will.topic)) { reject = mqtt_reason::topic_name_invalid; return false; }
        if (!read_binary(body, len, off, out.will.payload)) { reject = mqtt_reason::malformed_packet; return false; }

        out.will.has = true;
        out.will.qos = will_qos;
        out.will.retain = will_retain;
    }

    // ---- Username / Password ----
    if (has_username)
    {
        if (!read_utf8_string(body, len, off, out.username)) { reject = mqtt_reason::malformed_packet; return false; }
    }
    if (has_password)
    {
        if (!read_binary(body, len, off, out.password)) { reject = mqtt_reason::malformed_packet; return false; }
    }

    // trailing garbage → malformed
    if (off != len)
    {
        reject = mqtt_reason::malformed_packet;
        return false;
    }

    // MQTT 5 §3.1.5: Clean Start=1 时 Client ID 允许为空（服务端可通过 Assigned Client Identifier 分配）。
    // 这里不再硬拒，由上层 parse_client_id / server 逻辑决定如何处理。

    return true;
}

bool parse_client_id(const std::string &raw, mqtt_client_info &out)
{
    out.reg_key.clear();
    out.group.clear();
    out.device.clear();

    if (raw.empty()) return false;

    // 取 reg_key（@@@ 前；无 @@@ 则整体）
    std::string reg_key;
    auto sep = raw.find("@@@");
    if (sep != std::string::npos)
    {
        reg_key = raw.substr(0, sep);
        std::string rest = raw.substr(sep + 3);
        auto dash = rest.find('-');
        if (dash != std::string::npos)
        {
            out.group = rest.substr(0, dash);
            out.device = rest.substr(dash + 1);
        }
        else
        {
            out.group = rest;
            out.device.clear();
        }
    }
    else
    {
        reg_key = raw;
    }

    // reg_key 校验: 非空、≤32 字符、只含字母数字 _ -
    if (reg_key.empty() || reg_key.size() > 32) return false;
    for (char c : reg_key)
    {
        if (!(std::isalnum((unsigned char)c) || c == '_' || c == '-')) return false;
    }

    out.reg_key = std::move(reg_key);
    return true;
}

// ===================== PUBLISH =====================

bool parse_publish(uint8_t fixed_header, const uint8_t *body, size_t len, mqtt_publish_info &out)
{
    out.qos = (fixed_header >> 1) & 0x03;
    out.dup = (fixed_header & 0x08) != 0;
    out.retain = (fixed_header & 0x01) != 0;
    // QoS 3 为保留值
    if (out.qos == 3) return false;

    size_t off = 0;
    if (!read_utf8_string(body, len, off, out.topic)) return false;

    if (out.qos > 0)
    {
        if (!read_u16(body, len, off, out.packet_id)) return false;
        if (out.packet_id == 0) return false; // packet id 必须非 0
    }

    // ---- Property 块 ----
    {
        mqtt_prop_cursor cur(body, len, off);
        if (!cur.valid()) return false;
        mqtt_prop id{};
        while (cur.next(id))
        {
            switch (id)
            {
            case mqtt_prop::payload_format_indicator:
                if (!cur.read_u8(out.props.payload_format_indicator)) return false;
                out.props.has_payload_format = true;
                break;
            case mqtt_prop::message_expiry_interval:
                if (!cur.read_u32(out.props.message_expiry_interval)) return false;
                out.props.has_message_expiry = true;
                break;
            case mqtt_prop::topic_alias:
                {
                    uint16_t alias = 0;
                    if (!cur.read_u16(alias)) return false;
                    if (alias == 0) return false; // Topic Alias 不得为 0
                    out.props.topic_alias = alias;
                    out.props.has_topic_alias = true;
                    break;
                }
            case mqtt_prop::response_topic:
                if (!cur.read_str(out.props.response_topic)) return false;
                break;
            case mqtt_prop::correlation_data:
                if (!cur.read_bin(out.props.correlation_data)) return false;
                break;
            case mqtt_prop::content_type:
                if (!cur.read_str(out.props.content_type)) return false;
                break;
            case mqtt_prop::subscription_identifier:
                {
                    uint32_t sid = 0;
                    if (!cur.read_varint32(sid)) return false;
                    if (sid == 0) return false;
                    out.props.subscription_identifiers.push_back(sid);
                    break;
                }
            case mqtt_prop::user_property:
                {
                    mqtt_user_property up;
                    if (!cur.read_str(up.key) || !cur.read_str(up.value)) return false;
                    out.props.user_properties.push_back(std::move(up));
                    break;
                }
            default:
                cur.skip_value(id);
                break;
            }
            if (cur.bad()) return false;
        }
        if (cur.bad()) return false;
        off = cur.position();
    }

    out.payload = std::make_shared<std::string>(reinterpret_cast<const char *>(body + off), len - off);
    return true;
}

// ===================== SUBSCRIBE / UNSUBSCRIBE =====================

bool parse_subscribe(const uint8_t *body, size_t len, uint16_t &packet_id,
                     std::vector<mqtt_subscribe_entry> &out)
{
    size_t off = 0;
    if (!read_u16(body, len, off, packet_id)) return false;
    if (packet_id == 0) return false;
    if (off >= len) return false;

    // Property 块（订阅无强关注属性，安全跳过）
    {
        mqtt_prop_cursor cur(body, len, off);
        if (!cur.valid()) return false;
        mqtt_prop id{};
        while (cur.next(id)) { cur.skip_value(id); if (cur.bad()) return false; }
        if (cur.bad()) return false;
        off = cur.position();
    }

    // 注意：非 UTF-8 / 长度越界属于"报文本身畸形"，必须整体失败（由调用方断链）；
    // 而"filter 语义非法"（空串、通配符位置错误、QoS 保留值、Retain Handling 保留值 3）
    // 按 MQTT 5 §3.9.3 只能逐条在 SUBACK 里回 0x8F，不得切断连接。
    while (off < len)
    {
        mqtt_subscribe_entry e;
        if (!read_utf8_string(body, len, off, e.topic)) return false;

        uint8_t opt = 0;
        if (!read_u8(body, len, off, opt)) return false;
        e.options.retain_handling = (opt >> 4) & 0x03;
        e.options.retain_as_published = (opt & 0x08) != 0;
        e.options.no_local = (opt & 0x04) != 0;
        e.options.qos = opt & 0x03;

        if (e.topic.empty() || !is_valid_topic_filter(e.topic) ||
            e.options.qos > 2 ||            // QoS 3 是保留值（§3.8.3.1）
            e.options.retain_handling == 3) // Retain Handling 3 是保留值（§3.8.3.1）
        {
            e.valid = false;
        }

        out.push_back(std::move(e));
    }

    return !out.empty();
}

bool parse_unsubscribe(const uint8_t *body, size_t len, uint16_t &packet_id,
                       std::vector<mqtt_unsubscribe_entry> &out)
{
    size_t off = 0;
    if (!read_u16(body, len, off, packet_id)) return false;
    if (packet_id == 0) return false;
    if (off >= len) return false;

    {
        mqtt_prop_cursor cur(body, len, off);
        if (!cur.valid()) return false;
        mqtt_prop id{};
        while (cur.next(id)) { cur.skip_value(id); if (cur.bad()) return false; }
        if (cur.bad()) return false;
        off = cur.position();
    }

    // 与 SUBSCRIBE 同理：语义非法的 filter 逐条标记，由 UNSUBACK 回 0x8F（§3.11.3）
    while (off < len)
    {
        mqtt_unsubscribe_entry e;
        if (!read_utf8_string(body, len, off, e.topic)) return false;
        if (e.topic.empty() || !is_valid_topic_filter(e.topic)) e.valid = false;
        out.push_back(std::move(e));
    }

    return !out.empty();
}

// ===================== 确认类报文 =====================

bool parse_ack(mqtt_packet_type type, const uint8_t *body, size_t len,
               uint16_t &packet_id, mqtt_reason &reason, bool &has_packet_id)
{
    size_t off = 0;
    has_packet_id = false;
    packet_id = 0;
    reason = mqtt_reason::success;

    // PUBACK/PUBREC/PUBREL/PUBCOMP 必有 packet_id；DISCONNECT/AUTH 没有
    switch (type)
    {
    case mqtt_packet_type::PUBACK:
    case mqtt_packet_type::PUBREC:
    case mqtt_packet_type::PUBREL:
    case mqtt_packet_type::PUBCOMP:
        if (!read_u16(body, len, off, packet_id)) return false;
        if (packet_id == 0) return false;
        has_packet_id = true;
        break;
    default:
        break;
    }

    if (off < len)
    {
        uint8_t rc = body[off++];
        reason = static_cast<mqtt_reason>(rc);
    }

    // MQTT 5 强制 Property Length 存在（即使为 0），必须消费掉
    if (off < len)
    {
        mqtt_prop_cursor cur(body, len, off);
        if (!cur.valid()) return false;
        mqtt_prop id{};
        while (cur.next(id)) { cur.skip_value(id); if (cur.bad()) return false; }
        if (cur.bad()) return false;
        off = cur.position();
    }

    return true;
}

// ===================== 报文构造 =====================

namespace
{
void push_fixed(std::vector<uint8_t> &pkt, uint8_t type_flags, const std::vector<uint8_t> &body)
{
    pkt.clear();
    pkt.push_back(type_flags);
    encode_remaining_length(pkt, body.size());
    pkt.insert(pkt.end(), body.begin(), body.end());
}
}// namespace

std::vector<uint8_t> make_connack(bool session_present, mqtt_reason rc,
                                  const mqtt_connack_props &props, bool with_reason_string)
{
    std::vector<uint8_t> body;

    mqtt_prop_builder pb;
    pb.u32(mqtt_prop::session_expiry_interval, props.session_expiry_interval);
    if (props.receive_maximum > 0) pb.u16(mqtt_prop::receive_maximum, props.receive_maximum);
    if (props.maximum_packet_size > 0) pb.u32(mqtt_prop::maximum_packet_size, props.maximum_packet_size);
    pb.u16(mqtt_prop::topic_alias_maximum, props.topic_alias_maximum);
    pb.u8(mqtt_prop::maximum_qos, props.maximum_qos);
    pb.u8(mqtt_prop::retain_available, props.retain_available ? 1 : 0);
    pb.u8(mqtt_prop::wildcard_subscription_available, props.wildcard_subscription_available ? 1 : 0);
    pb.u8(mqtt_prop::subscription_identifier_available, props.subscription_identifier_available ? 1 : 0);
    pb.u8(mqtt_prop::shared_subscription_available, props.shared_subscription_available ? 1 : 0);
    if (props.server_keep_alive > 0) pb.u16(mqtt_prop::server_keep_alive, props.server_keep_alive);
    if (!props.assigned_client_identifier.empty())
        pb.str(mqtt_prop::assigned_client_identifier, props.assigned_client_identifier);
    if (!props.response_information.empty())
        pb.str(mqtt_prop::response_information, props.response_information);
    if (with_reason_string && !props.reason_string.empty())
        pb.str(mqtt_prop::reason_string, props.reason_string);
    for (auto &up : props.user_properties) pb.pair(up.key, up.value);

    body.push_back(session_present ? 0x01 : 0x00);
    body.push_back(static_cast<uint8_t>(rc));
    pb.encode_to(body);

    std::vector<uint8_t> pkt;
    push_fixed(pkt, 0x20, body);
    return pkt;
}

std::vector<uint8_t> make_suback(uint16_t packet_id, const std::vector<mqtt_reason> &codes)
{
    std::vector<uint8_t> body;
    body.push_back(static_cast<uint8_t>((packet_id >> 8) & 0xFF));
    body.push_back(static_cast<uint8_t>(packet_id & 0xFF));

    mqtt_prop_builder pb;   // 本实现不返回 User Property / reason string
    pb.encode_to(body);

    for (auto c : codes) body.push_back(static_cast<uint8_t>(c));

    std::vector<uint8_t> pkt;
    push_fixed(pkt, 0x90, body);
    return pkt;
}

std::vector<uint8_t> make_unsuback(uint16_t packet_id, const std::vector<mqtt_reason> &codes)
{
    std::vector<uint8_t> body;
    body.push_back(static_cast<uint8_t>((packet_id >> 8) & 0xFF));
    body.push_back(static_cast<uint8_t>(packet_id & 0xFF));

    mqtt_prop_builder pb;
    pb.encode_to(body);

    for (auto c : codes) body.push_back(static_cast<uint8_t>(c));

    std::vector<uint8_t> pkt;
    push_fixed(pkt, 0xB0, body);
    return pkt;
}

namespace
{
std::vector<uint8_t> make_simple_ack(uint8_t type_flags, uint16_t packet_id, mqtt_reason rc)
{
    std::vector<uint8_t> body;
    body.push_back(static_cast<uint8_t>((packet_id >> 8) & 0xFF));
    body.push_back(static_cast<uint8_t>(packet_id & 0xFF));
    body.push_back(static_cast<uint8_t>(rc));

    mqtt_prop_builder pb;   // Property Length = 0
    pb.encode_to(body);

    std::vector<uint8_t> pkt;
    push_fixed(pkt, type_flags, body);
    return pkt;
}
}// namespace

std::vector<uint8_t> make_puback(uint16_t packet_id, mqtt_reason rc)
{
    return make_simple_ack(0x40, packet_id, rc);
}

std::vector<uint8_t> make_pubrec(uint16_t packet_id, mqtt_reason rc)
{
    return make_simple_ack(0x50, packet_id, rc);
}

std::vector<uint8_t> make_pubrel(uint16_t packet_id, mqtt_reason rc)
{
    return make_simple_ack(0x62, packet_id, rc);
}

std::vector<uint8_t> make_pubcomp(uint16_t packet_id, mqtt_reason rc)
{
    return make_simple_ack(0x70, packet_id, rc);
}

std::vector<uint8_t> make_pingresp()
{
    return {0xD0, 0x00};
}

std::vector<uint8_t> make_disconnect(mqtt_reason rc)
{
    std::vector<uint8_t> body;
    body.push_back(static_cast<uint8_t>(rc));
    mqtt_prop_builder pb;
    pb.encode_to(body);

    std::vector<uint8_t> pkt;
    push_fixed(pkt, 0xE0, body);
    return pkt;
}

// 核心：从原始 props → 出站 props，已知"已等待秒数"。
// brokering（用 received_at 算 waited）和 retained（用 created_at 算 waited）共用。
// 返回 false 表示已过期，不应再投递（MQTT 5 §3.3.2.3.3）。
bool forward_publish_props_core(const mqtt_publish_props &in, uint64_t waited,
                                mqtt_publish_props &out)
{
    out = in;
    // 方向相关属性：出站一律丢弃（MQTT 5 §3.3.2.3）
    out.has_topic_alias = false;
    out.topic_alias = 0;
    out.subscription_identifiers.clear();

    if (out.has_message_expiry)
    {
        if (in.message_expiry_interval <= waited) return false;  // 已过期
        out.message_expiry_interval =
            static_cast<uint32_t>(in.message_expiry_interval - waited);
    }
    return true;
}

bool forward_publish_props(const mqtt_publish_info &pub, uint64_t now_sec,
                           mqtt_publish_props &out)
{
    uint64_t waited = (pub.received_at > 0 && now_sec > pub.received_at)
                          ? (now_sec - pub.received_at)
                          : 0;
    return forward_publish_props_core(pub.props, waited, out);
}

void put_publish_props(mqtt_prop_builder &pb, const mqtt_publish_props &props)
{
    // MQTT 5 §3.3.2.3：以下属性在转发时必须原样保留
    if (props.has_payload_format)
        pb.u8(mqtt_prop::payload_format_indicator, props.payload_format_indicator);
    if (props.has_message_expiry)
        pb.u32(mqtt_prop::message_expiry_interval, props.message_expiry_interval);
    if (!props.content_type.empty())
        pb.str(mqtt_prop::content_type, props.content_type);
    if (!props.response_topic.empty())
        pb.str(mqtt_prop::response_topic, props.response_topic);
    if (!props.correlation_data.empty())
        pb.bin(mqtt_prop::correlation_data, props.correlation_data);
    for (auto &up : props.user_properties)
        pb.pair(up.key, up.value);
    // 这里刻意不写 topic_alias 与 subscription_identifiers，原因见 make_publish 声明处。
}

std::vector<uint8_t> make_publish(std::string_view topic, std::string_view payload,
                                  uint8_t qos, uint16_t packet_id, bool retain,
                                  const mqtt_publish_props *props)
{
    std::vector<uint8_t> body;
    put_utf8_string(body, topic);
    if (qos > 0)
    {
        body.push_back(static_cast<uint8_t>((packet_id >> 8) & 0xFF));
        body.push_back(static_cast<uint8_t>(packet_id & 0xFF));
    }

    mqtt_prop_builder pb;
    if (props != nullptr) put_publish_props(pb, *props);
    pb.encode_to(body);

    body.insert(body.end(), payload.begin(), payload.end());

    uint8_t fixed = static_cast<uint8_t>(0x30 | ((qos & 0x03) << 1) | (retain ? 0x01 : 0x00));
    std::vector<uint8_t> pkt;
    push_fixed(pkt, fixed, body);
    return pkt;
}

}// namespace http
