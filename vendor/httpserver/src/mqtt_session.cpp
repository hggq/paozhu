#include "mqtt_session.h"

#include <algorithm>

#include "datetime.h"
#include "cost_define.h"
#include "mqtt_broker.h"
#include "terminal_color.h"

namespace http
{

mqtt_session::mqtt_session(std::shared_ptr<client_session> sess, const unsigned char *pre, size_t prelen)
    : sess_(std::move(sess))
{
    // 关键修复：框架在协议嗅探阶段已经 read_first 读过一段数据放进 _cache_data，
    // 这里把它作为读缓冲的初始内容，避免把 CONNECT 的首字节丢弃。
    if (pre != nullptr && prelen > 0)
    {
        buf_.assign(pre, pre + prelen);
    }
    touch_activity();
}

mqtt_session::~mqtt_session()
{
    cleanup();
}

void mqtt_session::init()
{
    self_ = weak_from_this();
}

void mqtt_session::cleanup()
{
    if (cleaned_) return;
    cleaned_ = true;

    {
        std::lock_guard<std::mutex> lk(out_mu_);
        pending_.clear();
        pending_bytes_ = 0;
    }

    // 注意：析构期间 shared_from_this() 不可用，必须使用 init() 时保存的 self_
    auto self_sp = self_.lock();
    if (!self_sp) return;

    mqtt_broker::instance().unsubscribe_all(self_sp);
    if (!info_.client_id.empty())
    {
        mqtt_broker::instance().unregister_client(info_.client_id, self_sp);
    }
}

bool mqtt_session::is_closed() const
{
    return !sess_ || sess_->isclose || sess_->iserror;
}

// ===================== keepalive =====================

uint16_t mqtt_session::effective_keepalive() const
{
    if (server_keep_alive_override_ > 0) return server_keep_alive_override_;
    return info_.keepalive;
}

void mqtt_session::touch_activity()
{
    if (!sess_) return;
    uint16_t ka = effective_keepalive();
    // MQTT 规范：服务端应在 1.5 倍 keepalive 内没收报文时断开连接。
    // 框架用 time_limit 做空闲回收，这里直接把该机制复用为 keepalive 超时。
    if (ka == 0)
    {
        sess_->time_limit.store(timeid() + 3600);   // 不启用保活：给一个很长的兜底
    }
    else
    {
        uint32_t limit = static_cast<uint32_t>(ka) + static_cast<uint32_t>(ka / 2) + 2;
        sess_->time_limit.store(timeid() + limit);
    }
}

// ===================== 缓冲管理 =====================

void mqtt_session::compact_buffer()
{
    if (buf_pos_ == 0) return;
    if (buf_pos_ >= buf_.size())
    {
        buf_.clear();
        buf_pos_ = 0;
        return;
    }
    buf_.erase(buf_.begin(), buf_.begin() + static_cast<std::ptrdiff_t>(buf_pos_));
    buf_pos_ = 0;
}

bool mqtt_session::ensure_buffer(size_t need)
{
    if (buf_.size() - buf_pos_ >= need) return true;
    // 只按「已攒下的字节数」判畸形：单帧合法上限就是 max_in_packet_，攒过它说明对端在灌
    // 超长帧——而这种情况解析器读完 varint 头就会给出 too_large（read_packet() 回 0x95）。
    // 原判据 buf_.size() + need > max_in_packet_ + 64 会让一个正好等于 max_in_packet_
    // 的合法帧在还差不到 4KB 时被拒，且是静默断链（不发 DISCONNECT）。
    if (buf_.size() - buf_pos_ > max_in_packet_) return false;
    return true;
}

// ===================== 读帧 =====================

mqtt_frame_state mqtt_session::read_packet_from_buffer(packet &out)
{
    const size_t avail = buf_.size() - buf_pos_;
    mqtt_frame_view fv;
    const mqtt_frame_state st = mqtt_frame_parse(buf_.data() + buf_pos_, avail, max_in_packet_, fv);
    if (st == mqtt_frame_state::complete)
    {
        out.fixed = fv.fixed;
        out.type  = static_cast<mqtt_packet_type>((fv.fixed >> 4) & 0x0F);
        out.body.assign(fv.body, fv.body + fv.body_len);
        buf_pos_ += fv.total;
    }
    compact_buffer();
    return st;
}

asio::awaitable<std::optional<mqtt_session::packet>> mqtt_session::read_packet()
{
    for (;;)
    {
        packet pkt;
        const auto st = read_packet_from_buffer(pkt);
        if (st == mqtt_frame_state::complete) co_return pkt;
        if (st == mqtt_frame_state::malformed)
        {
            write(make_disconnect(mqtt_reason::malformed_packet));
            co_return std::nullopt;
        }
        if (st == mqtt_frame_state::too_large)
        {
            write(make_disconnect(mqtt_reason::packet_too_large));
            co_return std::nullopt;
        }

        // need_more：收数据重试
        if (is_closed()) co_return std::nullopt;

        if (!ensure_buffer(kReadChunk))
        {
            // 兜底（正常路径由解析器先给出 too_large）：不静默断链，按 §3.1.4.7 回 0x95
            write(make_disconnect(mqtt_reason::packet_too_large));
            co_return std::nullopt;
        }

        size_t old = buf_.size();
        buf_.resize(old + kReadChunk);

        std::size_t n = 0;
        try
        {
            if (sess_->sslsocket)
            {
                n = co_await sess_->sslsocket->async_read_some(
                    asio::buffer(buf_.data() + old, kReadChunk), asio::use_awaitable);
            }
            else if (sess_->socket)
            {
                n = co_await sess_->socket->async_read_some(
                    asio::buffer(buf_.data() + old, kReadChunk), asio::use_awaitable);
            }
            else
            {
                co_return std::nullopt;
            }
        }
        catch (...)
        {
            co_return std::nullopt;
        }

        if (n == 0) co_return std::nullopt;
        buf_.resize(old + n);
    }
}

// ===================== 写 =====================

bool mqtt_session::write(std::vector<uint8_t> pkt)
{
    if (!sess_ || sess_->isclose || sess_->iserror) return false;
    if (pkt.empty()) return false;
    // post_write 现在会在"写队列满被丢弃"时返回 false，调用方必须据此处理
    // （deliver() 靠它避免在丢弃后仍登记 inflight）
    bool ok = sess_->post_write(std::string_view(reinterpret_cast<const char *>(pkt.data()), pkt.size()));
    if (!ok) dropped_writes_++;
    return ok;
}

bool mqtt_session::write(std::string_view raw)
{
    if (!sess_ || sess_->isclose || sess_->iserror) return false;
    if (raw.empty()) return false;
    bool ok = sess_->post_write(raw);
    if (!ok) dropped_writes_++;
    return ok;
}

// ===================== packet id =====================

uint16_t mqtt_session::next_packet_id()
{
    static std::atomic<uint16_t> counter{1};
    uint16_t id = 0;
    do
    {
        id = counter.fetch_add(1, std::memory_order_relaxed);
    } while (id == 0);   // packet id 必须非 0
    return id;
}

// ===================== QoS 2 入站 =====================

bool mqtt_session::store_inbound_qos2(uint16_t packet_id, const mqtt_publish_info &pub)
{
    // 记账口径 = topic + payload + 属性块（accounted_bytes）。
    // 只算 payload 的话，「1 字节 payload + 16MB user_properties」×1024 条即可撑爆 16MB 预算。
    size_t bytes = pub.accounted_bytes();

    auto it      = inbound_qos2_.find(packet_id);
    size_t old   = 0;
    bool replace = (it != inbound_qos2_.end());
    if (replace)
        old = it->second.accounted_bytes();     // 同一 packet id 重发（DUP=1）→ 替换而非叠加
    else if (inbound_qos2_.size() >= kMaxInboundQos2)
        return false;                           // 条数配额（重发不占新条目，故放行）

    if (inbound_qos2_bytes_ - old + bytes > kMaxInboundQos2Bytes) return false;// 字节配额

    if (replace) inbound_qos2_.erase(it);
    inbound_qos2_[packet_id] = pub;
    inbound_qos2_bytes_      = inbound_qos2_bytes_ - old + bytes;
    return true;
}

std::optional<mqtt_publish_info> mqtt_session::take_inbound_qos2(uint16_t packet_id)
{
    auto it = inbound_qos2_.find(packet_id);
    if (it == inbound_qos2_.end()) return std::nullopt;
    auto v = std::move(it->second);
    inbound_qos2_bytes_ -= v.accounted_bytes();// 与 store_inbound_qos2 同口径
    inbound_qos2_.erase(it);
    return v;
}

// ===================== 出站 =====================

bool mqtt_session::deliver(const std::string &topic, const mqtt_payload_ptr &payload, uint8_t qos, bool retain)
{
    static const mqtt_publish_props kNoProps;
    return deliver(topic, payload, qos, retain, kNoProps);
}

bool mqtt_session::deliver(const std::string &topic, const mqtt_payload_ptr &payload, uint8_t qos, bool retain,
                           const mqtt_publish_props &props)
{
    if (is_closed()) return false;
    if (qos > 2) qos = 2;

    std::lock_guard<std::mutex> lk(out_mu_);

    // 出站记账户：outbound_ 里只登记 topic + 共享 payload（不含 props——帧里那份是瞬时的，
    // 已由发送环 / 挂起队列限界）。登记、挤旧、确认三处必须同式，见 mqtt_inflight_bytes()。
    size_t inflight_bytes = mqtt_inflight_bytes(topic, payload);
    uint16_t pid = 0;
    if (qos > 0)
    {
        // 出站 QoS>0 并发上限 = min(本端硬上限, 客户端 CONNECT 宣告的 Receive Maximum)。
        // MQTT 5 §3.1.2.11.3：超出则客户端会以 0x93 断链。客户端值为 0 时按协议视为 65535。
        uint16_t client_rm = info_.props.receive_maximum;
        if (client_rm == 0) client_rm = 65535;
        size_t inflight_cap = std::min<size_t>(kMaxOutboundInflight, client_rm);
        if (outbound_.size() >= inflight_cap) return false;                       // 条数配额（含挂起帧）
        if (outbound_bytes_ + inflight_bytes > kMaxOutboundBytes) return false;    // 字节配额
        pid = next_packet_id();
    }

    // payload 为共享引用，make_publish 按 string_view 拷贝进帧（帧序列化必须的一次拷贝）
    std::string_view payload_sv = payload ? std::string_view(*payload) : std::string_view{};
    auto pkt = make_publish(topic, payload_sv, qos, pid, retain, &props);

    // MQTT 5 §3.2.2.3.6：服务端不得发送超过客户端在 CONNECT 中声明的
    // Maximum Packet Size 的报文（0 = 客户端不限制），否则客户端会直接断链。
    // 注意此时尚未登记 outbound_，直接返回不会留下僵尸 inflight 条目。
    if (info_.props.maximum_packet_size > 0 &&
        pkt.size() > info_.props.maximum_packet_size)
    {
        return false;
    }

    std::string_view raw(reinterpret_cast<const char *>(pkt.data()), pkt.size());
    // 环满不 sleep 也不丢：挂 pending 队列，等分发协程回灌。队列非空时新帧同样
    // 先挂队，保 FIFO。
    bool sent = pending_.empty() && write(raw);
    if (!sent)
    {
        enqueue_pending(std::move(pkt), pid, qos);
    }

    if (qos > 0)
    {
        mqtt_publish_info info;
        info.topic = topic;
        info.payload = payload;            // 共享引用，不拷贝字节
        info.qos = qos;
        info.retain = retain;
        info.packet_id = pid;
        // packet id 来自全局计数器，回绕后可能撞上本会话仍在途的条目：
        // 直接覆盖写会把旧条目的字节永久留在 outbound_bytes_ 里（账只增不减 → 后续投递被误拒），
        // 所以先摘旧、再登记。
        auto oit = outbound_.find(pid);
        if (oit != outbound_.end())
        {
            outbound_bytes_ -= mqtt_inflight_bytes(oit->second.topic, oit->second.payload);
            outbound_.erase(oit);
        }
        outbound_bytes_ += inflight_bytes;
        outbound_[pid] = std::move(info);
    }
    return true;
}

void mqtt_session::enqueue_pending(std::vector<uint8_t> &&pkt, uint16_t pid, uint8_t qos)
{
    // 调用方持 out_mu_
    pending_frame f;
    f.bytes        = std::move(pkt);
    f.pid          = pid;
    f.qos          = qos;
    pending_bytes_ += f.bytes.size();
    pending_.push_back(std::move(f));

    // 界 = CONST_MQTT_SESSION_BODY_SIZE：超了从最旧端丢弃，最新这条总要发。
    // 被挤掉的 QoS1/2 帧同步摘掉 inflight 登记，否则 packet id 白占配额；
    // at-least-once 对持续慢的订阅者就此失守（计入 dropped_writes_ 供诊断）。
    while (pending_.size() > 1 && pending_bytes_ > CONST_MQTT_SESSION_BODY_SIZE)
    {
        auto &old = pending_.front();
        if (old.qos > 0 && old.pid != 0)
        {
            auto oit = outbound_.find(old.pid);
            if (oit != outbound_.end())
            {
                outbound_bytes_ -= mqtt_inflight_bytes(oit->second.topic, oit->second.payload);
                outbound_.erase(oit);
            }
        }
        pending_bytes_ -= old.bytes.size();
        pending_.pop_front();
        ++dropped_writes_;
    }
}

size_t mqtt_session::flush_pending()
{
    std::lock_guard<std::mutex> lk(out_mu_);
    if (pending_.empty()) return 0;
    if (is_closed())
    {
        // 连接已废：挂起帧连同 inflight 登记一起清掉（会话对象马上销毁，
        // 但两本账都归零可避免销毁前其它线程 deliver 时读到虚高字节数而误拒）
        pending_.clear();
        pending_bytes_ = 0;
        outbound_.clear();
        outbound_bytes_ = 0;
        return 0;
    }
    size_t moved = 0;
    while (!pending_.empty())
    {
        auto &f   = pending_.front();
        std::string_view raw(reinterpret_cast<const char *>(f.bytes.data()), f.bytes.size());
        if (!write(raw)) break;// 环又满，停在这里等下一轮扫描
        pending_bytes_ -= f.bytes.size();
        pending_.pop_front();
        ++moved;
    }
    return moved;
}

bool mqtt_session::has_pending() const
{
    std::lock_guard<std::mutex> lk(out_mu_);
    return !pending_.empty();
}

void mqtt_session::ack_outgoing(mqtt_packet_type ack_type, uint16_t packet_id, mqtt_reason rc)
{
    (void)rc;
    std::lock_guard<std::mutex> lk(out_mu_);
    auto it = outbound_.find(packet_id);
    if (it == outbound_.end())
    {
        // 未知 packet id：MQTT 5 建议回 packet_identifier_not_found，这里仅记录
        return;
    }

    switch (ack_type)
    {
    case mqtt_packet_type::PUBACK:
        // QoS 1 完成（与登记时同口径扣减）
        outbound_bytes_ -= mqtt_inflight_bytes(it->second.topic, it->second.payload);
        outbound_.erase(it);
        break;
    case mqtt_packet_type::PUBREC:
    {
        // QoS 2 第一步确认 → 必须回送 PUBREL。
        // 把 inflight 条目整体 move 出再 move 回（登记 PUBREL 状态），payload 字节不变，
        // 故 outbound_bytes_ 无需调整（净零）。
        auto info = std::move(it->second);
        outbound_.erase(it);
        // PUBREL 入环被拒时挂 pending 队列而不是丢弃：丢了 PUBREL 会永远等不到 PUBCOMP。
        // 队列挤旧可能把它挤掉（慢订阅者），那时下面的登记也一并被摘，不留僵尸配额。
        auto pubrel = make_pubrel(packet_id, mqtt_reason::success);
        std::string_view raw(reinterpret_cast<const char *>(pubrel.data()), pubrel.size());
        if (!write(raw)) enqueue_pending(std::move(pubrel), packet_id, 2);
        // PUBREL 本身也需要被 PUBCOMP 确认，重新登记直到收到 PUBCOMP
        outbound_[packet_id] = std::move(info);
        break;
    }
    case mqtt_packet_type::PUBCOMP:
        // QoS 2 完成（与登记时同口径扣减）
        outbound_bytes_ -= mqtt_inflight_bytes(it->second.topic, it->second.payload);
        outbound_.erase(it);
        break;
    default:
        break;
    }
}

// ===================== Topic Alias =====================

bool mqtt_session::resolve_inbound_alias(mqtt_publish_info &pub)
{
    if (!pub.props.has_topic_alias) return true;

    uint16_t alias = pub.props.topic_alias;
    // 用 CONNACK 实际宣告的服务端入站 Topic Alias Maximum，而非客户端宣告值
    uint16_t max_allowed = server_topic_alias_max_;

    if (max_allowed == 0)
    {
        return false;   // 未协商 Topic Alias → topic_alias_invalid
    }
    if (alias == 0 || alias > max_allowed) return false;

    if (pub.topic.empty())
    {
        // 别名引用：必须已建立映射
        auto it = alias_to_topic_.find(alias);
        if (it == alias_to_topic_.end()) return false;
        pub.topic = it->second;
    }
    else
    {
        // 建立 / 更新映射
        if (alias_to_topic_.size() >= kMaxTopicAliasMappings) return false;
        alias_to_topic_[alias] = pub.topic;
    }
    return true;
}

}// namespace http
