// test_mqtt_client.cpp — controller business handlers (minimal pattern)
// 1. Construct mqtt_client, set all callbacks
// 2. http::get_client_context_obj().add_mqtt_task(cli) — drop directly into queue
// 3. async_run_task_fun completion → cv.notify_one() — wake handler
// 4. handler waits on condition_variable, then writes HTML response

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <asio.hpp>
#include <atomic>
#include <cstdio>
#include "httppeer.h"
#include "http_mqtt_client.h"
#include "mqtt_config.h"
#include "mqtt_method_reg.hpp"
#include "client_context.h"
#include "server.h"

namespace http
{

// ==================== 演示页的 broker 地址 ====================
// 这几条演示路由连的就是本进程自己在 httpport 上认出的 MQTT，地址以前写死成 127.0.0.1:80，
// 换端口/换机器的部署里它们只会连一个不存在的地址然后按退避一直重连。
// 现在和常驻的 [echo_sync]/[echo_co] 一个口径：地址读 conf/mqtt.conf 的 [demo] 段。
struct demo_broker_t
{
    std::string host;
    unsigned short port = 80;
};

static demo_broker_t demo_broker()
{
    demo_broker_t out;
    // mqtt_config.h 整个头在 ENABLE_MQTT_CLIENT 关闭时是空的，所以读段的那一步要收在函数体里
#ifdef ENABLE_MQTT_CLIENT
    auto cfg = mqtt_conf("demo");
    if (cfg && !cfg->host.empty() && cfg->port > 0)
    {
        out.host = cfg->host;
        out.port = cfg->port;
        return out;
    }
#endif
    // 段不存在（旧 conf、隔离实例自己的 conf 副本）就退回写死的 127.0.0.1:80，等同于地址一直写死在代码里，只是第一次回落时往 stderr 打一行说明
    out.host = "127.0.0.1";
    out.port = 80;
    static std::atomic<bool> logged{false};
    if (!logged.exchange(true))
        std::fprintf(stderr, "[mqtt_demo] no usable [demo] section in mqtt.conf, use %s:%u\n", out.host.c_str(), (unsigned)out.port);
    return out;
}

// ==================== Long-lived MQTT loops ====================
// client_id → weak_ptr<mqtt_client> + shared_ptr<atomic<bool>> stop flag
// HTTP handler starts but does not wait; the task runs in a while loop
// inside client_context's io_context.

struct mqtt_loop_registry_t
{
    std::mutex mu;
    std::unordered_map<std::string,
                       std::pair<std::weak_ptr<mqtt_client>,
                                 std::shared_ptr<std::atomic<bool>>>>
        items;
};

static mqtt_loop_registry_t &get_mqtt_loop_registry()
{
    static mqtt_loop_registry_t r;
    return r;
}

// ==================== mqtt_loop config registry ====================
// Same style as broker-side _initmqttmethodregto: cfg_key → mqtt_loop_config_t
// Business layer emplaces a new entry here to start a new long-lived client.

// mqtt_loop_config_t — one configuration for a long-lived loop
// Three field groups: connection params / loop params / business hook
struct mqtt_loop_config_t
{
    // Connection params；host/port 留空 = 跟 conf/mqtt.conf 的 [demo] 段（见 demo_broker()），
    // 业务侧要连别的 broker 才在这里显式写死。
    std::string host;
    uint16_t port = 0;
    std::string client_id;
    bool clean_start         = true;
    int keepalive            = 30;
    uint16_t receive_maximum = 65535;

    // Loop params
    int interval_sec = 10;

    // Subscribe/publish (empty topic = skip)
    std::string subscribe_topic;
    std::string publish_topic;
    std::string publish_payload;// may contain %round% placeholder
    uint8_t publish_qos = 0;
    bool publish_retain = false;

    // Business hook: called on inbound PUBLISH (optional)
    std::function<void(std::shared_ptr<mqtt_client>, const mqtt_recv_packet_t &)> on_publish;
};

// _init_mqtt_loop_reg — pre-registered loop configs
// Each emplace is preceded by a short comment describing its purpose.
inline void _init_mqtt_loop_reg(std::unordered_map<std::string, mqtt_loop_config_t> &reg)
{
    // loop-worker — default heartbeat loop, 10s cadence, publishes to loop/heartbeat
    {
        mqtt_loop_config_t w;
        w.client_id       = "mydevice@@@loop-worker-001";
        w.interval_sec    = 10;
        w.subscribe_topic = "loop/#";
        w.publish_topic   = "loop/heartbeat";
        w.publish_payload = "tick round=%round%";
        reg.emplace("loop-worker", std::move(w));
    }

    // loop-chatter — chat-bubble loop, 5s cadence, publishes to chat/hello
    {
        mqtt_loop_config_t c;
        c.client_id       = "mydevice@@@loop-chatter-001";
        c.interval_sec    = 5;
        c.subscribe_topic = "chat/#";
        c.publish_topic   = "chat/hello";
        c.publish_payload = "hi from loop-chatter round=%round%";
        reg.emplace("loop-chatter", std::move(c));
    }
}

// get_mqtt_loop_cfg — returns the registered config table (lazy init)
static std::unordered_map<std::string, mqtt_loop_config_t> &get_mqtt_loop_cfg()
{
    static std::unordered_map<std::string, mqtt_loop_config_t> reg;
    static bool once = (_init_mqtt_loop_reg(reg), true);
    (void)once;
    return reg;
}

// ==================== Test utilities ====================

static std::string make_html(std::string_view title, std::string_view body)
{
    std::string out = "<!doctype html><html><head><meta charset=utf-8>"
                      "<title>";
    out += title;
    out += "</title></head><body><h2>";
    out += title;
    out += "</h2><pre style='white-space:pre-wrap;'>";
    out += body;
    out += "</pre></body></html>";
    return out;
}

// ==================== Handler: test_mqtt_client ====================
// Runs a set of B3 / S1 / B4 validation cases and echoes results as HTML.

// case_result — full output of a single test case
struct case_result
{
    std::string name;
    bool connected_;
    mqtt_reason connack_rc;
    bool session_present;
    bool iserror;
    std::string error_msg;
    uint16_t server_receive_maximum;
    std::vector<mqtt_recv_packet_t> recv;
    // This test installs run_loop_fun ⇒ the "received" window should never be
    // populated (the framework no longer keeps a second copy for containers
    // that nobody reads). We treat "window is empty" as a positive assertion
    // rather than printing a perpetually-zero size as a message count.
    bool received_window_empty = true;
};

// run_one_case — coroutine lambda that runs a single test case
// Flow: connect → subscribe (100ms to collect retained dump) → clear retained → publish → collect → disconnect
// Two io_contexts: handler polls server_ioc every 50ms; the MQTT task runs inside client_context.ioc.
auto run_one_case = [](std::string_view name, mqtt_client_config_t cfg) -> asio::awaitable<case_result>
{
    case_result r;
    r.name    = std::string(name);
    auto cli  = std::make_shared<mqtt_client>();
    auto demo = demo_broker();
    cli->set_host(demo.host);
    cli->set_port(demo.port);
    cli->set_config(cfg);

    std::vector<mqtt_recv_packet_t> recv_packets;
    cli->run_loop_fun = [&](std::shared_ptr<mqtt_client>, const mqtt_recv_packet_t &pkt)
    {
        recv_packets.push_back(pkt);
    };

    std::mutex mu;
    std::condition_variable cv;
    bool done = false;

    cli->async_run_task_fun = [&](std::shared_ptr<mqtt_client> c) -> asio::awaitable<void>
    {
        auto self = c;
        bool ok   = co_await self->async_mqtt_connect(/*ms=*/5000);
        if (ok)
        {
            // Clear state right after a successful connect so subsequent ops don't
            // pollute the CONNACK status readings we need to capture.
            self->iserror = false;
            self->error_msg.clear();

            co_await self->async_subscribe("chat/#", /*qos=*/1, /*no_local=*/true);

            // 100ms after subscribe — collect any broker retained dump (leftovers from the previous run).
            co_await asio::steady_timer(self->strand_, std::chrono::milliseconds(100))
                .async_wait(asio::use_awaitable);

            // Clear previous retained (empty payload + retain=true = delete retained).
            co_await self->async_publish("chat/retain/key", "", /*qos=*/0, /*retain=*/true);
            co_await asio::steady_timer(self->strand_, std::chrono::milliseconds(50))
                .async_wait(asio::use_awaitable);

            co_await self->async_publish("chat/hi", "ping-" + std::string(name), /*qos=*/0, /*retain=*/false);
            co_await asio::steady_timer(self->strand_, std::chrono::milliseconds(150))
                .async_wait(asio::use_awaitable);

            r.connected_             = self->connected_;
            r.connack_rc             = self->connack_rc;
            r.session_present        = self->session_present;
            r.iserror                = self->iserror;
            r.error_msg              = self->error_msg;
            r.server_receive_maximum = self->server_props.receive_maximum;
            r.recv                   = recv_packets;
            r.received_window_empty  = self->received.empty();

            co_await self->async_disconnect();
        }
        else
        {
            r.iserror   = self->iserror;
            r.error_msg = self->error_msg;
        }
        {
            std::lock_guard<std::mutex> lk(mu);
            done = true;
        }
        cv.notify_one();
        co_return;
    };

    http::get_client_context_obj().add_mqtt_task(cli);

    // Handler polls on server_ioc, waits for client_context.ioc to finish.
    for (int i = 0; i < 100 && !done; ++i)
    {
        co_await asio::steady_timer(get_server_app().get_ctx(), std::chrono::milliseconds(50))
            .async_wait(asio::use_awaitable);
    }
    co_return r;
};

// test_mqtt_client — main handler that chains 3 cases and assembles HTML
// Case 1: B4 No Local — no_local=true, expect 0 loopbacks
// Case 2: parse_client_id lenient — group may be empty and still allowed
// Case 3: S1 receive_max — server should advertise its own policy min(client, 16)
//@urlpath(null, test_mqtt_client)
asio::awaitable<std::string> test_mqtt_client(std::shared_ptr<httppeer> peer)
{
    httppeer &cp = peer->get_peer();

    mqtt_client_config_t cfg1;
    cfg1.client_id       = "mydevice@@@group-device";
    cfg1.clean_start     = true;
    cfg1.keepalive       = 60;
    cfg1.receive_maximum = 65535;

    auto r1 = co_await run_one_case("Case1_B4_no_local", cfg1);

    mqtt_client_config_t cfg2 = cfg1;
    cfg2.client_id            = "mydevice@@@";// reg_key=mydevice group=empty device=empty
    auto r2                   = co_await run_one_case("Case2_parse_client_id_reg_only", cfg2);

    mqtt_client_config_t cfg3 = cfg1;
    cfg3.client_id            = "mydevice@@@rm-client";
    cfg3.receive_maximum      = 8;// less than server policy 16 → server still advertises its own policy 16 (no longer min with client value)
    auto r3                   = co_await run_one_case("Case3_S1_rm8", cfg3);

    auto dump = [](const case_result &r) -> std::string
    {
        std::string s = std::string("=== ") + r.name + " ===\n";
        s += "  connected_=" + std::to_string((int)r.connected_) +
             "  rc=" + std::to_string(static_cast<int>(r.connack_rc)) +
             "  err=" + std::to_string((int)r.iserror);
        if (!r.error_msg.empty())
            s += " (" + r.error_msg + ")";
        s += "\n  session_present=" + std::to_string((int)r.session_present);
        s += "  server_receive_maximum=" + std::to_string(r.server_receive_maximum);
        s += "\n  run_loop_fun=" + std::to_string(r.recv.size());
        if (!r.recv.empty())
            for (auto &p : r.recv)
                s += " [" + p.topic + "]";
        s += "\n  received_window_empty=" + std::to_string((int)r.received_window_empty) +
             " (hook installed, should be 1)";
        s += "\n";
        return s;
    };

    std::string summary = dump(r1) + dump(r2) + dump(r3);

    summary += "\n=== Verification Summary ===\n";
    summary += "B3 empty Client ID: " + std::string((int)r2.connack_rc == 0 ? "PASS" : "FAIL") +
               "  (rc=" + std::to_string(static_cast<int>(r2.connack_rc)) + ")\n";
    summary += "S1 receive_max:    " + std::string(r3.server_receive_maximum == 16 ? "PASS (server advertises its own policy 16)" : "FAIL") +
               "  (server_rm=" + std::to_string(r3.server_receive_maximum) + ", expect 16)\n";
    summary += "B4 No Local:       " + std::string(r1.recv.empty() ? "PASS" : "FAIL (loopbacks received)") +
               "  (" + std::to_string(r1.recv.size()) + " loopbacks)\n";
    // All three cases install run_loop_fun and the "no message should land"
    // semantics of no_local makes the empty window check toothless here.
    // For real evidence see /test_mqtt_tick_push, which installs the same hook
    // but actually receives 5 messages and checks received in the callback.
    summary += "hook installed, no window writes: — (this group produces zero delivered messages, reading is meaningless; see /test_mqtt_tick_push)\n";

    cp.output = make_html("MQTT 5 Client Test — B3 + S1 + B4", summary);
    co_return "";
}

// ==================== Handler: server tick push test ====================
// test_mqtt_tick_push — exercises the server-side tick push channel (sync, isloopco=false):
// reg_key=mydevice → my_test_mqtt.on_connect sets durtime=5/loop_num=100000,
// the tick thread pushes a 5-color rotation (red/blue/yellow/#800080/off) to /led/set every durtime seconds.
// Exits after receiving all 5; if not filled in one round, reconnect and try again.
// Verified against a real ESP32-S3 — uses the same channel.

namespace tick_push_detail
{
// fps only increments across whole seconds, so tick cadence is in whole seconds:
// durtime=5 ⇒ one push ~every 5 seconds, and tick_seq is a session member —
// reconnect restarts the rotation (colors start over from red).
constexpr std::size_t kWant = 5;  // exit once kWant messages received
constexpr int kMaxRounds    = 2;  // max reconnect rounds
constexpr int kRoundPolls   = 108;// per-round window: 108 × 250ms ≈ 27s, enough for one full 5-message cycle
}// namespace tick_push_detail

struct tick_push_ctx_t
{
    std::mutex mu;
    std::vector<mqtt_recv_packet_t> msgs;
    bool task_done = false;
    bool got_want  = false;
    // Sampled directly in the callback (runs on the client's strand — same thread
    // as enqueue, no cross-thread diagnostic read):
    // Hook installed ⇒ "received" window must never be populated.
    bool recv_window_empty = true;
};

//@urlpath(null, test_mqtt_tick_push)
asio::awaitable<std::string> test_mqtt_tick_push(std::shared_ptr<httppeer> peer)
{
    httppeer &cp = peer->get_peer();

    auto ctx = std::make_shared<tick_push_ctx_t>();

    auto cli  = std::make_shared<mqtt_client>();
    auto demo = demo_broker();
    cli->set_host(demo.host);
    cli->set_port(demo.port);
    mqtt_client_config_t cfg;
    cfg.client_id   = "mydevice@@@tick-watch-1";
    cfg.clean_start = true;
    cfg.keepalive   = 30;
    cli->set_config(cfg);

    cli->run_loop_fun = [ctx](std::shared_ptr<mqtt_client> self, const mqtt_recv_packet_t &pkt)
    {
        // If any packet arrives the callback path is working — received must still be empty at this point.
        if (!self->received.empty())
        {
            std::lock_guard<std::mutex> lk(ctx->mu);
            ctx->recv_window_empty = false;
        }
        if (pkt.topic != "/led/set")
            return;
        std::lock_guard<std::mutex> lk(ctx->mu);
        ctx->msgs.push_back(pkt);
    };

    cli->async_run_task_fun =
        [cli, ctx](std::shared_ptr<mqtt_client> self) -> asio::awaitable<void>
    {
        for (int round = 0; round < tick_push_detail::kMaxRounds; ++round)
        {
            {
                std::lock_guard<std::mutex> lk(ctx->mu);
                if (ctx->msgs.size() >= tick_push_detail::kWant)
                    break;
            }
            if (!co_await self->async_mqtt_connect(/*ms=*/5000))
                break;
            self->iserror = false;
            self->error_msg.clear();

            co_await self->async_subscribe("/led/set", /*qos=*/0, /*no_local=*/false);

            // ~one every durtime seconds; 27s round window can receive 5~6 messages;
            // poll the counter every 250ms and stop early once we have enough.
            for (int i = 0; i < tick_push_detail::kRoundPolls; ++i)
            {
                std::size_t n = 0;
                {
                    std::lock_guard<std::mutex> lk(ctx->mu);
                    n = ctx->msgs.size();
                }
                if (n >= tick_push_detail::kWant)
                    break;
                co_await asio::steady_timer(self->strand_, std::chrono::milliseconds(250))
                    .async_wait(asio::use_awaitable);
            }
            co_await self->async_disconnect();
        }
        {
            std::lock_guard<std::mutex> lk(ctx->mu);
            ctx->task_done = true;
        }
        co_return;
    };

    http::get_client_context_obj().add_mqtt_task(cli);

    // Handler waits on server_ioc for "kWant received or task done", upper bound ~60s (2 rounds × 27s windows).
    bool finished = false;
    for (int i = 0; i < 1200 && !finished; ++i)
    {
        co_await asio::steady_timer(get_server_app().get_ctx(), std::chrono::milliseconds(50))
            .async_wait(asio::use_awaitable);
        std::lock_guard<std::mutex> lk(ctx->mu);
        finished = (ctx->msgs.size() >= tick_push_detail::kWant) || ctx->task_done;
    }

    std::string summary;
    {
        std::lock_guard<std::mutex> lk(ctx->mu);
        summary += "received=" + std::to_string(ctx->msgs.size()) +
                   " (want " + std::to_string(tick_push_detail::kWant) + ")\n";
        for (auto &m : ctx->msgs)
        {
            summary += "  [" + m.topic + "] " + m.payload + "\n";
        }
        summary += "\nserver tick push (sync run_loop): " +
                   std::string(ctx->msgs.size() >= tick_push_detail::kWant ? "PASS" : "FAIL") + "\n";
        summary += "hook installed, no window writes: " +
                   std::string(ctx->msgs.empty() ? "— (none received, reading is meaningless)" : (ctx->recv_window_empty ? "PASS" : "FAIL (received was populated)")) + "\n";
    }
    cp.output = make_html("MQTT Server Tick Push (Sync) — Collect 5 Messages", summary);
    co_return "";
}

// test_mqtt_tick_push_co — coroutine-version tick channel (isloopco=true) + mqtttasks zero-drop reading:
// reg_key=mydeviceco → my_test_mqtt_co.async_on_connect sets isloopco/durtime=1/loop_num=4,
// the tick async_run_loop is co_spawned onto this connection's strand; durtime=1 ⇒ ~one per second, 4 messages total pushed to demo/async_tick.
// After loop_num drops to zero, the next tick scan should remove this session from mqtttasks
// (while the connection is still alive — it should already drop at that point).
// mqtttasks is mutated on the websocketthreads thread; we only read .size() for diagnostics
// (same pattern used by the public listing path).

struct tick_push_co_ctx_t
{
    std::mutex mu;
    std::vector<mqtt_recv_packet_t> msgs;
    bool got_want  = false;// received all 4
    bool task_done = false;
};

constexpr std::size_t kCoWant = 4;// my_test_mqtt_co loop_num=4, exactly 4 messages expected

//@urlpath(null, test_mqtt_tick_push_co)
asio::awaitable<std::string> test_mqtt_tick_push_co(std::shared_ptr<httppeer> peer)
{
    httppeer &cp = peer->get_peer();

    auto ctx = std::make_shared<tick_push_co_ctx_t>();

    unsigned int baseline = static_cast<unsigned int>(get_server_app().mqtttasks.size());

    auto cli  = std::make_shared<mqtt_client>();
    auto demo = demo_broker();
    cli->set_host(demo.host);
    cli->set_port(demo.port);
    mqtt_client_config_t cfg;
    cfg.client_id   = "mydeviceco@@@tick-watch-co";
    cfg.clean_start = true;
    cfg.keepalive   = 30;
    cli->set_config(cfg);

    cli->run_loop_fun = [ctx](std::shared_ptr<mqtt_client>, const mqtt_recv_packet_t &pkt)
    {
        if (pkt.topic != "demo/async_tick")
            return;
        std::lock_guard<std::mutex> lk(ctx->mu);
        ctx->msgs.push_back(pkt);
        if (ctx->msgs.size() >= kCoWant)
            ctx->got_want = true;
    };

    cli->async_run_task_fun =
        [cli, ctx](std::shared_ptr<mqtt_client> self) -> asio::awaitable<void>
    {
        if (co_await self->async_mqtt_connect(/*ms=*/5000))
        {
            self->iserror = false;
            self->error_msg.clear();
            co_await self->async_subscribe("demo/#", /*qos=*/0, /*no_local=*/false);

            // Wait for kCoWant messages (upper bound 12s), then hold for another 2s —
            // give the tick scan a chance to zero-drop while the connection is still alive.
            for (int i = 0; i < 48; ++i)
            {
                {
                    std::lock_guard<std::mutex> lk(ctx->mu);
                    if (ctx->got_want)
                        break;
                }
                co_await asio::steady_timer(self->strand_, std::chrono::milliseconds(250))
                    .async_wait(asio::use_awaitable);
            }
            co_await asio::steady_timer(self->strand_, std::chrono::milliseconds(2000))
                .async_wait(asio::use_awaitable);
            co_await self->async_disconnect();
        }
        {
            std::lock_guard<std::mutex> lk(ctx->mu);
            ctx->task_done = true;
        }
        co_return;
    };

    http::get_client_context_obj().add_mqtt_task(cli);

    // Readings: peak value when filled (should be > baseline),
    //           ~1.5s after filled (zero-drop happens, connection still alive),
    //           ~1.5s after disconnect.
    unsigned int peak = baseline;
    auto snap         = [](unsigned int &out_peak)
    {
        unsigned int n = static_cast<unsigned int>(get_server_app().mqtttasks.size());
        if (n > out_peak)
            out_peak = n;
        return n;
    };

    bool saw_want = false;
    for (int i = 0; i < 240 && !saw_want; ++i)
    {
        co_await asio::steady_timer(get_server_app().get_ctx(), std::chrono::milliseconds(50))
            .async_wait(asio::use_awaitable);
        snap(peak);
        {
            std::lock_guard<std::mutex> lk(ctx->mu);
            saw_want = ctx->got_want;
        }
    }
    for (int i = 0; i < 30; ++i)// ~1.5s, across one tick scan cycle
        co_await asio::steady_timer(get_server_app().get_ctx(), std::chrono::milliseconds(50))
            .async_wait(asio::use_awaitable);
    unsigned int after_zero = snap(peak);// expect back to baseline (zero-drop, no disconnect yet)

    bool done = false;
    for (int i = 0; i < 100 && !done; ++i)
    {
        co_await asio::steady_timer(get_server_app().get_ctx(), std::chrono::milliseconds(50))
            .async_wait(asio::use_awaitable);
        snap(peak);
        std::lock_guard<std::mutex> lk(ctx->mu);
        done = ctx->task_done;
    }
    for (int i = 0; i < 30; ++i)
        co_await asio::steady_timer(get_server_app().get_ctx(), std::chrono::milliseconds(50))
            .async_wait(asio::use_awaitable);
    unsigned int after_disc = snap(peak);// expect still = baseline (disconnect doesn't leave residue)

    std::string summary;
    bool pass = false;
    {
        std::lock_guard<std::mutex> lk(ctx->mu);
        summary += "received=" + std::to_string(ctx->msgs.size()) +
                   " (want " + std::to_string(kCoWant) + ")\n";
        for (auto &m : ctx->msgs)
            summary += "  [" + m.topic + "] " + m.payload + "\n";
        summary += "\nmqtttasks: baseline=" + std::to_string(baseline) +
                   " peak=" + std::to_string(peak) +
                   " after_zero(loop_num=0, conn alive)=" + std::to_string(after_zero) +
                   " after_disconnect=" + std::to_string(after_disc) + "\n";
        pass = ctx->msgs.size() >= kCoWant && after_zero <= baseline && after_disc <= baseline;
        summary += "\nserver tick push (co async_run_loop) + zero-drop: " +
                   std::string(pass ? "PASS" : "FAIL") + "\n";
    }
    cp.output = make_html("MQTT Server Tick Push (Coroutine isloopco) — 4 Messages + mqtttasks Drop", summary);
    co_return "";
}

// ==================== Handler: long-lived loop ====================

// start_mqtt_loop — starts a while-loop MQTT client; the HTTP handler returns immediately.
// ?cfg=loop-worker picks the registered config (default: loop-worker).
//@urlpath(null, start_mqtt_loop)
asio::awaitable<std::string> start_mqtt_loop(std::shared_ptr<httppeer> peer)
{
    httppeer &cp        = peer->get_peer();
    std::string cfg_key = cp.get["cfg"].to_string();
    if (cfg_key.empty())
        cfg_key = "loop-worker";

    auto &cfg_reg = get_mqtt_loop_cfg();
    auto it       = cfg_reg.find(cfg_key);
    if (it == cfg_reg.end())
    {
        std::string available;
        for (auto &kv : cfg_reg)
            available += kv.first + " ";
        co_return make_html("start_mqtt_loop",
                            "unknown cfg: " + cfg_key + "\navailable: " + available + "\n");
    }
    auto &cfg = it->second;
    // ?hold=N: after subscribe, stay connected for N seconds before disconnecting,
    // giving the "receive and observe" probe a window.
    // Default 0 = each round is only online for the brief connect→subscribe→publish moment;
    // burst floods will most likely miss the window entirely.
    int hold_sec = cp.get["hold"].to_int();

    auto &reg = get_mqtt_loop_registry();
    {
        std::lock_guard<std::mutex> lk(reg.mu);
        auto e = reg.items.find(cfg.client_id);
        if (e != reg.items.end() && e->second.first.lock())
        {
            co_return make_html("start_mqtt_loop",
                                "already running: " + cfg.client_id + "\n");
        }
    }

    auto cli = std::make_shared<mqtt_client>();
    // 本条 loop 没显式写地址就取 conf 的 [demo] 段，写了就用它自己的
    demo_broker_t broker = demo_broker();
    cli->set_host(cfg.host.empty() ? broker.host : cfg.host);
    cli->set_port(cfg.port > 0 ? cfg.port : broker.port);
    mqtt_client_config_t mc;
    mc.client_id       = cfg.client_id;
    mc.clean_start     = cfg.clean_start;
    mc.keepalive       = cfg.keepalive;
    mc.receive_maximum = cfg.receive_maximum;
    cli->set_config(mc);

    if (cfg.on_publish)
        cli->run_loop_fun = cfg.on_publish;

    auto stop_flag = std::make_shared<std::atomic<bool>>(false);

    cli->async_run_task_fun =
        [cfg, stop_flag, hold_sec](std::shared_ptr<mqtt_client> self) -> asio::awaitable<void>
    {
        int round = 0;
        while (!stop_flag->load())
        {
            round++;
            DEBUG_LOG("[mqtt_loop:%s] round %d connect...", cfg.client_id.c_str(), round);

            bool ok = co_await self->async_mqtt_connect(/*ms=*/5000);
            if (!ok)
            {
                DEBUG_LOG("[mqtt_loop:%s] round %d connect failed: %s",
                          cfg.client_id.c_str(),
                          round,
                          self->error_msg.c_str());
                if (stop_flag->load())
                    break;
                co_await asio::steady_timer(self->strand_,
                                            std::chrono::seconds(cfg.interval_sec))
                    .async_wait(asio::use_awaitable);
                continue;
            }

            if (!cfg.subscribe_topic.empty())
                co_await self->async_subscribe(cfg.subscribe_topic, /*qos=*/1, /*no_local=*/true);

            if (!cfg.publish_topic.empty())
            {
                std::string payload = cfg.publish_payload;
                auto pos            = payload.find("%round%");
                if (pos != std::string::npos)
                    payload.replace(pos, 7, std::to_string(round));
                co_await self->async_publish(cfg.publish_topic, payload, cfg.publish_qos, cfg.publish_retain);
            }

            // Sleep in chunks: must also honour stop during hold, otherwise the old client
            // would still hold onto the client_id after stop — any new CONNECT with the same id
            // would then be rejected with CONNACK rc=135 until the old one drops.
            for (int t = 0; t < hold_sec * 2 && !stop_flag->load(); ++t)
                co_await asio::steady_timer(self->strand_,
                                            std::chrono::milliseconds(500))
                    .async_wait(asio::use_awaitable);

            DEBUG_LOG("[mqtt_loop:%s] round %d done", cfg.client_id.c_str(), round);
            co_await self->async_disconnect();

            if (stop_flag->load())
                break;
            co_await asio::steady_timer(self->strand_,
                                        std::chrono::seconds(cfg.interval_sec))
                .async_wait(asio::use_awaitable);
        }
        DEBUG_LOG("[mqtt_loop:%s] stopped after %d rounds", cfg.client_id.c_str(), round);
        co_return;
    };

    {
        std::lock_guard<std::mutex> lk(reg.mu);
        reg.items[cfg.client_id] = {cli, stop_flag};
    }

    http::get_client_context_obj().add_mqtt_task(cli);

    cp.output = make_html("start_mqtt_loop",
                          "started cfg=" + cfg_key + " client_id=" + cfg.client_id +
                              " interval=" + std::to_string(cfg.interval_sec) + "s" +
                              " hold=" + std::to_string(hold_sec) + "s\n");
    co_return "";
}

// stop_mqtt_loop — sends a stop signal to the specified loop
// ?cfg=loop-worker
//@urlpath(null, stop_mqtt_loop)
asio::awaitable<std::string> stop_mqtt_loop(std::shared_ptr<httppeer> peer)
{
    httppeer &cp        = peer->get_peer();
    std::string cfg_key = cp.get["cfg"].to_string();
    if (cfg_key.empty())
        cfg_key = "loop-worker";

    std::string client_id;
    auto &cfg_reg = get_mqtt_loop_cfg();
    auto it       = cfg_reg.find(cfg_key);
    if (it != cfg_reg.end())
        client_id = it->second.client_id;
    else
        client_id = cfg_key;

    auto &reg = get_mqtt_loop_registry();
    std::shared_ptr<std::atomic<bool>> stop;
    {
        std::lock_guard<std::mutex> lk(reg.mu);
        auto e = reg.items.find(client_id);
        if (e == reg.items.end())
        {
            co_return make_html("stop_mqtt_loop",
                                "not found: " + client_id + "\n");
        }
        stop = e->second.second;
        reg.items.erase(e);
    }
    stop->store(true);

    cp.output = make_html("stop_mqtt_loop",
                          "stop signal sent to " + client_id + "\n");
    co_return "";
}

// list_mqtt_loop — lists running loops + registered configs
//@urlpath(null, list_mqtt_loop)
asio::awaitable<std::string> list_mqtt_loop(std::shared_ptr<httppeer> peer)
{
    httppeer &cp    = peer->get_peer();
    std::string out = "=== running ===\n";
    auto &reg       = get_mqtt_loop_registry();
    std::lock_guard<std::mutex> lk(reg.mu);
    for (auto &kv : reg.items)
    {
        auto alive = kv.second.first.lock();
        out += "  " + kv.first + " alive=" + std::to_string((bool)alive);
        if (alive)
        {
            // received window reading (S3 diagnostic): only loops that did NOT install
            // run_loop_fun accumulate here. Enqueue on the strand and this read can race
            // with concurrent pushes — so we only take a scalar snapshot for diagnostics,
            // never traverse the container (no synchronization guarantee).
            out += " connected=" + std::to_string((bool)alive->connected_);
            out += " received=" + std::to_string(alive->received.size()) +
                   "/" + std::to_string(alive->received_max) +
                   " bytes=" + std::to_string(alive->received_bytes_) +
                   "/" + std::to_string(alive->received_bytes_max) +
                   " dropped=" + std::to_string(alive->received_dropped);
        }
        out += "\n";
    }
    out += "\n=== registered cfg ===\n";
    auto &cfg_reg = get_mqtt_loop_cfg();
    for (auto &kv : cfg_reg)
    {
        // 把这一条 loop 真会去连的地址一起打出来：cfg 里 host/port 留空就是跟 conf 的 [demo] 段，
        // 光看 client_id 读不出它连的是哪个端口
        demo_broker_t b = demo_broker();
        out += "  " + kv.first + " → client_id=" + kv.second.client_id +
               " interval=" + std::to_string(kv.second.interval_sec) + "s" +
               " broker=" + (kv.second.host.empty() ? b.host : kv.second.host) + ":" +
               std::to_string(kv.second.port > 0 ? kv.second.port : b.port) + "\n";
    }
    cp.output = make_html("list_mqtt_loop", out);
    co_return "";
}

}// namespace http
