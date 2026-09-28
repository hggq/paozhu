// HTTP MQTT 5 client — 实现
// 风格对齐 http_socket_client / http_websocket_client：
//   - 构造函数零参，strand_ 从 get_client_context_obj().ioc 构造
//   - public 字段 iserror/error_msg/ec/sock/strand_/iswait_exit/exptime/timeout_end
//   - reset() 清所有状态
//   - async_tcp_connect() 裸 TCP（不发 proxy CONNECT 串）
//   - run_loop() / async_run_loop() + run_loop_fun / async_run_loop_fun 回调注入
//   - close_connect()
//   - manager map + get_http_mqtt_obj()

#include "http_mqtt_client.h"
#include "client_context.h"
#include "atomic_guard.h"

#include <future>
#include <thread>

namespace http
{

// =====================================================================
// manager（对齐 get_http_socket_obj / get_http_websocket_obj）
// =====================================================================
std::map<std::string, std::shared_ptr<mqtt_client>> &get_http_mqtt_obj()
{
    static std::map<std::string, std::shared_ptr<mqtt_client>> obj;
    return obj;
}

// =====================================================================
// client → broker 报文构造（static helper，不抢公共接口）
// =====================================================================

std::vector<uint8_t> mqtt_client::make_client_connect_(const mqtt_client_config_t &cfg)
{
    std::vector<uint8_t> body;
    put_utf8_string(body, "MQTT");
    body.push_back(5);   // level

    uint8_t flags = 0;
    if (cfg.clean_start)          flags |= 0x02;
    if (!cfg.username.empty())    flags |= 0x80;
    if (!cfg.password.empty())    flags |= 0x40;
    if (cfg.has_will)
    {
        flags |= 0x04;
        flags |= (cfg.will_qos & 0x03) << 3;
        if (cfg.will_retain)      flags |= 0x20;
    }
    body.push_back(flags);

    body.push_back(static_cast<uint8_t>((cfg.keepalive >> 8) & 0xFF));
    body.push_back(static_cast<uint8_t>(cfg.keepalive & 0xFF));

    // CONNECT Property block
    mqtt_prop_builder pb;
    if (cfg.receive_maximum > 0)
        pb.u16(mqtt_prop::receive_maximum, cfg.receive_maximum);
    if (cfg.maximum_packet_size > 0)
        pb.u32(mqtt_prop::maximum_packet_size, cfg.maximum_packet_size);
    if (cfg.topic_alias_maximum > 0)
        pb.u16(mqtt_prop::topic_alias_maximum, cfg.topic_alias_maximum);
    if (cfg.has_will && cfg.will_delay_interval > 0)
        pb.u32(mqtt_prop::will_delay_interval, cfg.will_delay_interval);
    pb.encode_to(body);

    put_utf8_string(body, cfg.client_id);

    // Will（如果有）
    if (cfg.has_will)
    {
        // Will Property（简化：只放 message_expiry，后续可加 content_type / response_topic / correlation_data）
        mqtt_prop_builder wpb;
        if (cfg.will_message_expiry > 0)
            wpb.u32(mqtt_prop::message_expiry_interval, cfg.will_message_expiry);
        wpb.encode_to(body);
        put_utf8_string(body, cfg.will_topic);
        put_binary(body, cfg.will_payload);
    }

    if (!cfg.username.empty()) put_utf8_string(body, cfg.username);
    if (!cfg.password.empty()) put_utf8_string(body, cfg.password);

    std::vector<uint8_t> pkt;
    pkt.push_back(0x10);
    encode_remaining_length(pkt, body.size());
    pkt.insert(pkt.end(), body.begin(), body.end());
    return pkt;
}

std::vector<uint8_t> mqtt_client::make_client_subscribe_(
    uint16_t pid, std::string_view filter, uint8_t qos,
    bool no_local, bool retain_as_published, uint8_t retain_handling)
{
    std::vector<uint8_t> body;
    body.push_back(static_cast<uint8_t>((pid >> 8) & 0xFF));
    body.push_back(static_cast<uint8_t>(pid & 0xFF));
    body.push_back(0);   // Property Length = 0
    put_utf8_string(body, filter);
    uint8_t opt = static_cast<uint8_t>(retain_handling << 4) |
                  static_cast<uint8_t>(retain_as_published ? 0x08 : 0x00) |
                  static_cast<uint8_t>(no_local ? 0x04 : 0x00) |
                  (qos & 0x03);
    body.push_back(opt);

    std::vector<uint8_t> pkt;
    pkt.push_back(0x82);
    encode_remaining_length(pkt, body.size());
    pkt.insert(pkt.end(), body.begin(), body.end());
    return pkt;
}

std::vector<uint8_t> mqtt_client::make_client_unsubscribe_(uint16_t pid, std::string_view filter)
{
    std::vector<uint8_t> body;
    body.push_back(static_cast<uint8_t>((pid >> 8) & 0xFF));
    body.push_back(static_cast<uint8_t>(pid & 0xFF));
    body.push_back(0);
    put_utf8_string(body, filter);

    std::vector<uint8_t> pkt;
    pkt.push_back(0xA2);
    encode_remaining_length(pkt, body.size());
    pkt.insert(pkt.end(), body.begin(), body.end());
    return pkt;
}

std::vector<uint8_t> mqtt_client::make_client_pingreq_()
{
    return {0xC0, 0x00};
}

// =====================================================================
// mqtt_client 构造 / 析构 / reset —— 完全对齐 socket_client
// =====================================================================

mqtt_client::mqtt_client()
    : strand_(asio::make_strand(*(get_client_context_obj().ioc)))
    , server_ioc_(nullptr)
{
}

mqtt_client::mqtt_client(asio::io_context &strand_ioc, asio::io_context &server_ioc)
    : strand_(asio::make_strand(strand_ioc))
    , server_ioc_(&server_ioc)
{
}

mqtt_client::~mqtt_client()
{
    close_connect();
}

void mqtt_client::reset()
{
    iserror = false;
    isco = false;
    iswait_exit = false;
    exptime = 0;
    timeout_end = 0;

    url.clear();
    host.clear();
    port = 0;
    error_msg.clear();
    ec.clear();

    config = mqtt_client_config_t{};
    connack_rc = mqtt_reason::success;
    session_present = false;
    connected_ = false;

    next_packet_id_ = 0;

    reply_mu_.lock();
    pending_subacks_.clear();
    pending_unsubacks_.clear();
    pending_pubacks_.clear();
    reply_mu_.unlock();

    rdBuf_.clear();
    rdyPkts_.clear();
    received.clear();
    received_bytes_  = 0;
    received_dropped = 0;

    run_loop_fun = nullptr;
    async_run_loop_fun = nullptr;
    on_connect_fun = nullptr;
    on_disconnect_fun = nullptr;
    on_puback_fun = nullptr;
    on_suback_fun = nullptr;
    on_unsuback_fun = nullptr;
    run_task_fun = nullptr;
    async_run_task_fun = nullptr;

    close_connect();
    sock.reset();
    sslsock.reset();
    ssl_context.reset();
}

void mqtt_client::set_host(std::string_view name)  { host = name; }
void mqtt_client::set_port(unsigned int n)         { port = n; }

void mqtt_client::set_url(std::string_view name)
{
    // 复用 socket_client 风格: url 里包含 host:port
    if (name.size() > 7)
    {
        // 跳过 "mqtt://" 或 "tcp://" 前缀
        std::string_view prefix = name.substr(0, 7);
        size_t skip = 0;
        if (prefix == "mqtt://" || prefix == "tcp://") skip = 7;
        else if (name.substr(0, 8) == "mqtts://") skip = 8;

        std::string_view rest = name.substr(skip);
        auto colon = rest.find(':');
        auto slash = rest.find('/');
        size_t end = (colon != std::string_view::npos && (slash == std::string_view::npos || colon < slash))
                     ? colon : slash;
        if (end == std::string_view::npos) end = rest.size();
        host = std::string(rest.substr(0, end));
        if (colon != std::string_view::npos)
        {
            std::string port_str(rest.substr(colon + 1, (slash != std::string_view::npos ? slash - colon - 1 : std::string_view::npos)));
            try { port = static_cast<unsigned int>(std::stoul(port_str)); } catch (...) { port = 0; }
        }
    }
    url = std::string(name);
}

void mqtt_client::set_config(const mqtt_client_config_t &cfg)
{
    config = cfg;
}

uint16_t mqtt_client::gen_packet_id_()
{
    auto v = ++next_packet_id_;
    if (v == 0) v = ++next_packet_id_;
    return v;
}

// =====================================================================
// async_tcp_connect —— 裸 TCP（不发 proxy CONNECT 串）
// =====================================================================

asio::awaitable<bool> mqtt_client::async_tcp_connect(std::string_view name, unsigned int time_out_num)
{
    set_url(name);
    exptime = time_out_num;
    co_return co_await async_tcp_connect();
}

asio::awaitable<bool> mqtt_client::async_tcp_connect()
{
    if (host.empty())
    {
        iserror = true;
        error_msg = "host empty";
        co_return false;
    }
    if (port == 0)
    {
        iserror = true;
        error_msg = "port empty";
        co_return false;
    }

    try
    {
        asio::ip::tcp::resolver resolver(strand_);
        auto endpoints = co_await resolver.async_resolve(host, std::to_string(port), asio::use_awaitable);
        sock = std::make_shared<asio::ip::tcp::socket>(strand_);
        for (auto &ep : endpoints)
        {
            ec.clear();
            co_await sock->async_connect(ep, asio::use_awaitable);
            if (!ec) break;
        }
        if (ec)
        {
            iserror = true;
            error_msg = ec.message();
            co_return false;
        }
    }
    catch (const std::exception &e)
    {
        iserror = true;
        error_msg = e.what();
        co_return false;
    }

    if (exptime > 0) reset_timeout();

    co_return true;
}

// =====================================================================
// async_mqtt_connect —— CONNECT packet + 等 CONNACK + 启 run_loop
// =====================================================================

asio::awaitable<bool> mqtt_client::async_mqtt_connect(unsigned int time_out_num)
{
    if (!sock)
    {
        if (!co_await async_tcp_connect()) co_return false;
    }

    auto self = shared_from_this();

    // 发 CONNECT（同步写 + write_mu_，与 dispatch_ 的 ACK 写串行）
    auto pkt = make_client_connect_(config);
    asio::error_code wec;
    {
        std::lock_guard<std::mutex> lk(write_mu_);
        asio::write(*sock, asio::buffer(pkt), wec);
    }
    if (wec)
    {
        iserror = true;
        error_msg = wec.message();
        co_return false;
    }

    // 启 run_loop（CONNACK 它来收）
    run_loop_alive_ = true;
    self->run_loop();

    // 等 CONNACK
    auto start = std::chrono::steady_clock::now();
    unsigned int timeout_ms = (time_out_num > 0) ? time_out_num : 5000;
    while (!connected_ && !iserror)
    {
        if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(timeout_ms))
        {
            iserror = true;
            error_msg = "CONNACK timeout";
            co_return false;
        }
        if (!run_loop_alive_)
        {
            // run_loop 已因对端断链 / broker DISCONNECT 退出，不再空等满超时
            iserror   = true;
            error_msg = "connection closed before CONNACK";
            co_return false;
        }
        co_await asio::steady_timer(strand_, std::chrono::milliseconds(10)).async_wait(asio::use_awaitable);
    }
    if (!connected_) co_return false;

    // keepalive：CONNECT 成功后起定时协程，每 keepalive 秒发一条 PINGREQ
    if (config.keepalive > 0)
    {
        co_spawn(strand_, [self] { return self->async_keepalive_loop(); }, asio::detached);
    }

    if (on_connect_fun) on_connect_fun(self, connack_rc);
    co_return true;
}

// =====================================================================
// async_publish / async_subscribe / async_unsubscribe / async_pingreq / async_disconnect
// =====================================================================

asio::awaitable<bool> mqtt_client::async_publish(std::string_view topic, std::string_view payload,
                                                   uint8_t qos, bool retain)
{
    // 不支持 QoS2 且刻意不做：全仓调用点 qos 只用 0/1，没有需求方；做全协议要补 PUBREL 超时
    // 重发、inflight 上限、DUP 去重、会话级持久化四件配套，给死路径背这套债不值。显式拒
    // 比"发出去必谎报超时"诚实——出站 qos>1 拒绝且不写 socket，免得半途状态不一致
    if (qos > 1) { iserror = true; error_msg = "qos 2 not supported"; co_return false; }
    if (!sock) { iserror = true; error_msg = "not connected"; co_return false; }
    uint16_t pid = (qos == 0) ? 0 : gen_packet_id_();
    auto pkt = make_publish(topic, payload, qos, pid, retain, nullptr);
    asio::error_code wec;
    {
        std::lock_guard<std::mutex> lk(write_mu_);
        asio::write(*sock, asio::buffer(pkt), wec);
    }
    if (wec) { iserror = true; error_msg = wec.message(); co_return false; }
    if (qos == 0) co_return true;

    // 等 PUBACK（qos 已限定 ≤1）
    std::promise<mqtt_reason> p;
    auto fut = p.get_future();
    {
        std::lock_guard<std::mutex> lk(reply_mu_);
        pending_pubacks_[pid] = std::move(p);
    }
    auto start = std::chrono::steady_clock::now();
    while (fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
    {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5))
        {
            std::lock_guard<std::mutex> lk(reply_mu_);
            pending_pubacks_.erase(pid);
            iserror = true; error_msg = "PUBACK timeout";
            co_return false;
        }
        if (!run_loop_alive_)// 对端断链即刻终结等待，不空等满 5s
        {
            std::lock_guard<std::mutex> lk(reply_mu_);
            pending_pubacks_.erase(pid);
            iserror = true; error_msg = "connection closed before publish ack";
            co_return false;
        }
        co_await asio::steady_timer(strand_, std::chrono::milliseconds(10)).async_wait(asio::use_awaitable);
    }
    auto rc = fut.get();
    auto self = shared_from_this();
    if (on_puback_fun) on_puback_fun(self, pid, rc);
    co_return rc == mqtt_reason::success;
}

asio::awaitable<bool> mqtt_client::async_subscribe(std::string_view filter, uint8_t qos,
                                                     bool no_local, bool retain_as_published,
                                                     uint8_t retain_handling)
{
    // 订阅请求侧同样封顶 QoS1：订到 QoS2 等于请 broker 用 QoS2 下发，而本客户端对入站
    // QoS2 PUBLISH 是断链拒收，不封顶就会让会话中途被自己订的主题踢下线
    if (qos > 1) { iserror = true; error_msg = "qos 2 not supported"; co_return false; }
    if (!sock) { iserror = true; error_msg = "not connected"; co_return false; }
    auto pid = gen_packet_id_();
    auto pkt = make_client_subscribe_(pid, filter, qos, no_local, retain_as_published, retain_handling);
    asio::error_code wec;
    {
        std::lock_guard<std::mutex> lk(write_mu_);
        asio::write(*sock, asio::buffer(pkt), wec);
    }
    if (wec) { iserror = true; error_msg = wec.message(); co_return false; }

    std::promise<std::vector<mqtt_reason>> p;
    auto fut = p.get_future();
    {
        std::lock_guard<std::mutex> lk(reply_mu_);
        pending_subacks_[pid] = std::move(p);
    }
    auto start = std::chrono::steady_clock::now();
    while (fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
    {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5))
        {
            std::lock_guard<std::mutex> lk(reply_mu_);
            pending_subacks_.erase(pid);
            iserror = true; error_msg = "SUBACK timeout";
            co_return false;
        }
        if (!run_loop_alive_)// 对端断链即刻终结等待，不空等满 5s
        {
            std::lock_guard<std::mutex> lk(reply_mu_);
            pending_subacks_.erase(pid);
            iserror = true; error_msg = "connection closed before SUBACK";
            co_return false;
        }
        co_await asio::steady_timer(strand_, std::chrono::milliseconds(10)).async_wait(asio::use_awaitable);
    }
    auto codes = fut.get();
    auto self = shared_from_this();
    if (on_suback_fun) on_suback_fun(self, pid, codes);
    co_return !codes.empty() && codes.front() != mqtt_reason::topic_filter_invalid;
}

asio::awaitable<bool> mqtt_client::async_unsubscribe(std::string_view filter)
{
    if (!sock) { iserror = true; error_msg = "not connected"; co_return false; }
    auto pid = gen_packet_id_();
    auto pkt = make_client_unsubscribe_(pid, filter);
    asio::error_code wec;
    {
        std::lock_guard<std::mutex> lk(write_mu_);
        asio::write(*sock, asio::buffer(pkt), wec);
    }
    if (wec) { iserror = true; error_msg = wec.message(); co_return false; }

    std::promise<std::vector<mqtt_reason>> p;
    auto fut = p.get_future();
    {
        std::lock_guard<std::mutex> lk(reply_mu_);
        pending_unsubacks_[pid] = std::move(p);
    }
    auto start = std::chrono::steady_clock::now();
    while (fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
    {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5))
        {
            std::lock_guard<std::mutex> lk(reply_mu_);
            pending_unsubacks_.erase(pid);
            iserror = true; error_msg = "UNSUBACK timeout";
            co_return false;
        }
        if (!run_loop_alive_)// 对端断链即刻终结等待，不空等满 5s
        {
            std::lock_guard<std::mutex> lk(reply_mu_);
            pending_unsubacks_.erase(pid);
            iserror = true; error_msg = "connection closed before UNSUBACK";
            co_return false;
        }
        co_await asio::steady_timer(strand_, std::chrono::milliseconds(10)).async_wait(asio::use_awaitable);
    }
    auto codes = fut.get();
    auto self = shared_from_this();
    if (on_unsuback_fun) on_unsuback_fun(self, pid, codes);
    co_return true;
}

asio::awaitable<void> mqtt_client::async_pingreq()
{
    if (!sock) co_return;
    auto pkt = make_client_pingreq_();
    {
        std::lock_guard<std::mutex> lk(write_mu_);
        asio::error_code wec;
        asio::write(*sock, asio::buffer(pkt), wec);
    }
}

asio::awaitable<void> mqtt_client::async_disconnect(mqtt_reason rc)
{
    run_loop_alive_ = false;
    connected_ = false;
    if (sock)
    {
        auto pkt = make_disconnect(rc);
        {
            std::lock_guard<std::mutex> lk(write_mu_);
            asio::error_code wec;
            asio::write(*sock, asio::buffer(pkt), wec);
        }
        close_connect();
    }
    auto self = shared_from_this();
    if (on_disconnect_fun) on_disconnect_fun(self, rc);
    co_return;// 全同步写后函数体里没有 co_await，不写这句就不是协程而是 UB 落空返回
}

// =====================================================================
// run_loop / async_run_loop / close_connect —— 对齐 socket_client
// =====================================================================

void mqtt_client::close_connect()
{
    run_loop_alive_ = false;
    if (sock)
    {
        std::error_code e;
        sock->close(e);
        sock.reset();
    }
    if (sslsock)
    {
        std::error_code e;
        sslsock->lowest_layer().close(e);
        sslsock.reset();
    }
    ssl_context.reset();
}

void mqtt_client::run_loop()
{
    auto self = shared_from_this();
    co_spawn(strand_, [self]{ return self->async_run_loop(); }, asio::detached);
}

asio::awaitable<void> mqtt_client::async_run_loop()
{
    std::vector<uint8_t> rd(4096);
    while (run_loop_alive_ && sock && sock->is_open())
    {
        ec.clear();
        // 用 redirect_error 让读错误写入 ec 而非抛异常——
        // 本协程由 co_spawn(..., asio::detached) 启动，未捕获异常会 std::terminate。
        auto n = co_await sock->async_read_some(asio::buffer(rd), asio::redirect_error(asio::use_awaitable, ec));
        if (ec || n == 0)
        {
            if (run_loop_alive_ && !iserror)
            {
                iserror   = true;
                error_msg = "connection closed by peer";
            }
            close_connect();
            co_return;
        }
        rdBuf_.insert(rdBuf_.end(), rd.begin(), rd.begin() + n);

        // 拆帧（与服务端共用 mqtt_framing；三态终局：畸形/超限回 DISCONNECT 后断链）
        const size_t max_in = (config.maximum_packet_size > 0 &&
                               config.maximum_packet_size < static_cast<uint32_t>(MQTT_MAX_PACKET_SIZE))
                                  ? static_cast<size_t>(config.maximum_packet_size)
                                  : static_cast<size_t>(MQTT_MAX_PACKET_SIZE);
        while (!rdBuf_.empty())
        {
            mqtt_frame_view fv;
            const auto st = mqtt_frame_parse(rdBuf_.data(), rdBuf_.size(), max_in, fv);
            if (st == mqtt_frame_state::need_more) break;
            if (st == mqtt_frame_state::malformed || st == mqtt_frame_state::too_large)
            {
                const mqtt_reason rc = (st == mqtt_frame_state::malformed)
                                           ? mqtt_reason::malformed_packet
                                           : mqtt_reason::packet_too_large;
                auto dp = make_disconnect(rc);
                asio::error_code ignore;
                {
                    std::lock_guard<std::mutex> lk(write_mu_);
                    asio::write(*sock, asio::buffer(dp), ignore);// 尽力发出再断
                }
                iserror   = true;
                error_msg = (st == mqtt_frame_state::malformed)
                                ? "broker sent malformed frame"
                                : "broker frame exceeds maximum_packet_size";
                close_connect();
                co_return;
            }
            std::vector<uint8_t> body(fv.body, fv.body + fv.body_len);
            rdBuf_.erase(rdBuf_.begin(), rdBuf_.begin() + static_cast<std::ptrdiff_t>(fv.total));
            rdyPkts_.emplace_back(fv.fixed, std::move(body));
        }

        while (!rdyPkts_.empty())
        {
            auto [fixed, body] = std::move(rdyPkts_.front());
            rdyPkts_.pop_front();
            dispatch_(fixed, body.data(), body.size());
        }
    }
}

// =====================================================================
// dispatch_ —— 把 broker 发来的报文路由到回调 / promise
// =====================================================================

void mqtt_client::dispatch_(uint8_t fixed, const uint8_t *body, size_t len)
{
    auto self = shared_from_this();
    auto type = static_cast<mqtt_packet_type>(fixed >> 4);
    switch (type)
    {
    case mqtt_packet_type::CONNACK:
    {
        if (len < 2) { iserror = true; error_msg = "CONNACK too short"; break; }
        // DEBUG: 打印前 16 字节 raw
        std::string hex;
        for (size_t i = 0; i < len && i < 16; ++i)
        {
            char buf[4]; snprintf(buf, sizeof(buf), "%02x ", body[i]);
            hex += buf;
        }
        DEBUG_LOG("[MQTT CLIENT] CONNACK len=%zu hex=%s", len, hex.c_str());

        session_present = (body[0] & 0x01) != 0;
        connack_rc = static_cast<mqtt_reason>(body[1]);
        connected_ = (connack_rc == mqtt_reason::success);

        // Property Length + Property block（MQTT 5 强制存在，至少是 varint 0）
        size_t prop_off = 2;
        uint32_t prop_len = 0;
        if (prop_off < len && read_varint(body, len, prop_off, prop_len))
        {
            mqtt_prop_cursor cur(body, len, prop_off);
            while (cur.valid())
            {
                mqtt_prop id;
                if (!cur.next(id)) break;
                switch (id)
                {
                case mqtt_prop::receive_maximum:
                    if (!cur.read_u16(server_props.receive_maximum)) return;
                    if (server_props.receive_maximum == 0)
                        server_props.receive_maximum = MQTT_DEFAULT_RECEIVE_MAXIMUM;
                    break;
                default:
                    cur.skip_value(id);
                    break;
                }
            }
        }

        if (!connected_)
        {
            iserror = true;
            error_msg = "CONNACK rc=" + std::to_string(static_cast<int>(connack_rc));
        }
        break;
    }
    case mqtt_packet_type::PUBLISH:
    {
        mqtt_publish_info info;
        if (!parse_publish(fixed, body, len, info)) break;

        // 本客户端刻意不支持 QoS2（理由见 async_publish 入口的注释）：入站 QoS2 PUBLISH
        // 回 DISCONNECT qos_not_supported 断链——收下不回 ACK 会让 broker inflight 悬挂、
        // DUP 重发刷同一条消息，静默坏掉不如摊在明面上。
        if (info.qos == 2)
        {
            iserror   = true;
            error_msg = "qos 2 not supported";
            run_loop_alive_ = false;
            auto dp = make_disconnect(mqtt_reason::qos_not_supported);
            {
                std::lock_guard<std::mutex> lk(write_mu_);
                if (sock)
                {
                    asio::error_code wec;
                    asio::write(*sock, asio::buffer(dp), wec);// 尽力发出再断
                }
            }
            close_connect();
            break;
        }

        deliver_publish_(info);

        // QoS 1 → 立即发 PUBACK（同步写 + write_mu_，与高层操作串行）
        if (info.qos == 1)
        {
            auto reply = make_puback(info.packet_id, mqtt_reason::success);
            std::lock_guard<std::mutex> lk(write_mu_);
            if (sock)
            {
                asio::error_code wec;
                asio::write(*sock, asio::buffer(reply), wec);
            }
        }
        break;
    }
    case mqtt_packet_type::PUBACK:
    {
        uint16_t pid = 0; mqtt_reason rc = mqtt_reason::success; bool has = false;
        if (parse_ack(type, body, len, pid, rc, has) && has)
        {
            std::lock_guard<std::mutex> lk(reply_mu_);
            auto it = pending_pubacks_.find(pid);
            if (it != pending_pubacks_.end()) { it->second.set_value(rc); pending_pubacks_.erase(it); }
        }
        break;
    }
    // 客户端不涉 QoS2 握手（出站/订阅入口即拒，入站 QoS2 直接断链），PUBREC/PUBREL/PUBCOMP
    // 没有对应在手状态，来了也只会是协议外杂包——落 default 静默忽略，不留半截状态机。
    case mqtt_packet_type::SUBACK:
    {
        if (len < 2) break;
        uint16_t pid = (body[0] << 8) | body[1];
        size_t off = 2;
        uint32_t plen = 0;
        read_varint(body, len, off, plen);
        off += plen;
        std::vector<mqtt_reason> codes;
        while (off < len) codes.push_back(static_cast<mqtt_reason>(body[off++]));
        std::lock_guard<std::mutex> lk(reply_mu_);
        auto it = pending_subacks_.find(pid);
        if (it != pending_subacks_.end()) { it->second.set_value(codes); pending_subacks_.erase(it); }
        break;
    }
    case mqtt_packet_type::UNSUBACK:
    {
        if (len < 2) break;
        uint16_t pid = (body[0] << 8) | body[1];
        size_t off = 2;
        uint32_t plen = 0;
        read_varint(body, len, off, plen);
        off += plen;
        std::vector<mqtt_reason> codes;
        while (off < len) codes.push_back(static_cast<mqtt_reason>(body[off++]));
        std::lock_guard<std::mutex> lk(reply_mu_);
        auto it = pending_unsubacks_.find(pid);
        if (it != pending_unsubacks_.end()) { it->second.set_value(codes); pending_unsubacks_.erase(it); }
        break;
    }
    case mqtt_packet_type::PINGRESP:
        break;
    case mqtt_packet_type::DISCONNECT:
    {
        mqtt_reason rc = mqtt_reason::success;
        if (len >= 1) rc = static_cast<mqtt_reason>(body[0]);
        const bool before_connack = !connected_;
        connected_ = false;
        run_loop_alive_ = false;
        close_connect();
        if (before_connack && !iserror)
        {
            // CONNACK 之前 broker 就断开：把终态报给 async_mqtt_connect 的等待循环
            iserror   = true;
            error_msg = "broker disconnected before CONNACK: rc=" + std::to_string(static_cast<int>(rc));
        }
        if (on_disconnect_fun) on_disconnect_fun(self, rc);
        break;
    }
    case mqtt_packet_type::AUTH:
        break;
    default:
        break;
    }
}

// 把一条收完的 PUBLISH 投递给业务：挂回调走回调，没挂才往 received 窗口攒
void mqtt_client::deliver_publish_(mqtt_publish_info &info)
{
    auto self = shared_from_this();
    mqtt_recv_packet_t pkt{info.topic, *info.payload, info.qos, info.packet_id,
                           info.retain, info.dup, info.props};

    if (async_run_loop_fun)
    {
        co_spawn(strand_, [self, pkt]{ return self->async_run_loop_fun(self, pkt); }, asio::detached);
    }
    else if (run_loop_fun)
    {
        try { run_loop_fun(self, pkt); }
        catch (const std::exception &e) { DEBUG_LOG("%s", e.what()); }
    }
    else
    {
        // 没挂回调 = 极简用法，才往 received 攒；挂回调的会话不留第二份拷贝
        if (received.empty()) received_bytes_ = 0;// 外部自行 clear() 过，记账跟着归零
        received_bytes_ += info.topic.size() + info.payload->size();
        received.push_back(std::move(info));
        // 双界挤旧：条数或字节任一超界就丢最旧；size()>1 的尾巴保证
        // 单条自己就超界时不会把自己挤掉（新条总要收下）
        while (received.size() > 1 &&
               (received.size() > received_max || received_bytes_ > received_bytes_max))
        {
            received_bytes_ -= received.front().topic.size() +
                               (received.front().payload ? received.front().payload->size() : 0);
            received.erase(received.begin());
            ++received_dropped;
        }
    }
}

// keepalive 定时协程：connected_ 期间每 keepalive 秒发一条 PINGREQ。
// 不追踪出站活动，固定间隔最简单，也必定落在 broker keepalive×1.5 的判死窗口内。
asio::awaitable<void> mqtt_client::async_keepalive_loop()
{
    const unsigned int ka = config.keepalive;
    while (run_loop_alive_ && connected_)
    {
        asio::steady_timer timer(strand_);
        timer.expires_after(std::chrono::seconds(ka));
        co_await timer.async_wait(asio::use_awaitable);
        if (!run_loop_alive_ || !connected_) co_return;
        auto pkt = make_client_pingreq_();
        std::lock_guard<std::mutex> lk(write_mu_);
        if (!sock) co_return;
        asio::error_code wec;
        asio::write(*sock, asio::buffer(pkt), wec);
        if (wec) co_return;// 写失败由 run_loop 的读错误路径统一收尾
    }
}

// async_run_task_fun 完成后调这个 —— 和 threadpool.cpp:http_clientrun 完全同模式
void mqtt_client::notify_on_done_()
{
    std::unique_lock<std::mutex> lock(handler_mu_);
    if (handler_queue_.empty()) return;
    auto handle = std::move(handler_queue_.front());
    handler_queue_.pop_front();
    lock.unlock();

    asio::io_context &ioc = server_ioc_ ? *server_ioc_ : get_server_app().get_ctx();
    asio::dispatch(ioc,
                   [handler = std::move(handle)]() mutable -> void
                   {
                       handler(1);
                   });
}

asio::awaitable<size_t> mqtt_client::co_user_task(asio::use_awaitable_t<> h)
{
    auto self = shared_from_this();
    auto initiate = [self](asio::detail::awaitable_handler<asio::any_io_executor, size_t> &&handler) mutable
    {
        std::lock_guard<std::mutex> lk(self->handler_mu_);
        self->handler_queue_.push_back(std::move(handler));
        get_client_context_obj().add_mqtt_task(self);
    };
    return asio::async_initiate<asio::use_awaitable_t<>, void(size_t)>(initiate, h);
}

}// namespace http
