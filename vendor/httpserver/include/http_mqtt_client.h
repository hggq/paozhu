#ifndef HTTP_MQTT_CLIENT_H
#define HTTP_MQTT_CLIENT_H

// MQTT 5 client — 风格对齐 http_socket_client / http_websocket_client。
// 网络: 裸 asio::ip::tcp::socket，async_tcp_connect 不发 HTTP proxy CONNECT 串
//       （broker 跑在 HTTP 端口，靠首字节 0x10 嗅探分流）。
// 编解码: 复用 mqtt_frame.h 的 put_*/encode_remaining_length 原语 + parse_* 函数。
// 错误处理: 同 socket_client —— iserror + error_msg + asio::error_code ec。
// 线程模型: strand_ 从 get_client_context_obj().ioc 构造，加入 client_context 的任务队列和 timeout 队列。

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/strand.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <memory>
#include <mutex>
#include <chrono>
#include "mqtt_frame.h"
#include "mqtt_framing.h"
#include "datetime.h"

namespace http
{

class mqtt_client;

// CONNECT 参数（MQTT 5）—— 扁平字段，和 socket_client 的 parameter 风格一致
struct mqtt_client_config_t
{
    std::string client_id;
    bool clean_start   = true;
    uint16_t keepalive = 60;

    // CONNECT Property
    uint16_t receive_maximum     = 65535;
    uint32_t maximum_packet_size = 0;// 0 = 不宣告
    uint16_t topic_alias_maximum = 0;

    // Username / Password
    std::string username;
    std::string password;

    // Will（可选）
    bool has_will = false;
    std::string will_topic;
    std::string will_payload;
    uint8_t will_qos             = 0;
    bool will_retain             = false;
    uint32_t will_delay_interval = 0;
    uint32_t will_message_expiry = 0;
};

// 入站 PUBLISH 的极简收集窗口默认字节界：与 received_max（条数界）同时生效，
// 谁先撞到按谁挤旧。独立于服务端的会话体界，改一处不应连带另一处。
inline constexpr std::size_t MQTT_CLIENT_RECEIVED_BYTE_LIMIT = 2ull * 1024ull * 1024ull;

// 入站 PUBLISH 回调参数
struct mqtt_recv_packet_t
{
    std::string topic;
    std::string payload;
    uint8_t qos        = 0;
    uint16_t packet_id = 0;
    bool retain        = false;
    uint8_t dup        = false;
    mqtt_publish_props props;
};

// 返回码（同 mqtt_reason 但语义只保留调用者关心的）
enum class mqtt_client_rc : uint8_t
{
    ok                = 0,
    timeout           = 1,
    tcp_error         = 2,
    connack_rejected  = 3,
    malformed_packet  = 4,
    already_connected = 5,
    disconnected      = 6,
};

class mqtt_client : public std::enable_shared_from_this<mqtt_client>
{
  public:
    // ---- 超时系列（完全对齐 socket_client）----
    void timeout(unsigned int t) { exptime = t; }
    unsigned int timeout() { return exptime; }
    void reset_timeout() { timeout_end.store(timeid() + exptime); }
    void set_timeout(unsigned int t) { timeout_end.store(timeid() + t); }
    unsigned int get_timeout() { return timeout_end.load(); }

    mqtt_client();
    mqtt_client(asio::io_context &strand_ioc, asio::io_context &server_ioc);
    ~mqtt_client();
    void reset();

    // ---- 设置 ----
    void set_host(std::string_view name);
    void set_port(unsigned int n);
    void set_url(std::string_view name);
    void set_config(const mqtt_client_config_t &cfg);// 一步到位（比逐字段 set 方便）

    // ---- 连接 ----
    // 裸 TCP 连接（不发 proxy CONNECT）—— socket_client 的 async_tcp_connect 带 proxy 串
    asio::awaitable<bool> async_tcp_connect(std::string_view url, unsigned int time_out_num = 0);
    asio::awaitable<bool> async_tcp_connect();

    // MQTT CONNECT + 等待 CONNACK
    asio::awaitable<bool> async_mqtt_connect(unsigned int time_out_num = 0);

    // ---- 高层操作 ----
    asio::awaitable<bool> async_subscribe(std::string_view filter,
                                          uint8_t qos              = 1,
                                          bool no_local            = false,
                                          bool retain_as_published = false,
                                          uint8_t retain_handling  = 0);
    asio::awaitable<bool> async_unsubscribe(std::string_view filter);

    asio::awaitable<bool> async_publish(std::string_view topic,
                                        std::string_view payload,
                                        uint8_t qos = 0,
                                        bool retain = false);
    asio::awaitable<void> async_pingreq();
    asio::awaitable<void> async_disconnect(mqtt_reason rc = mqtt_reason::success);

    // ---- run_loop（与 socket_client 同签名风格）----
    void run_loop();
    asio::awaitable<void> async_run_loop();
    void close_connect();

    // ---- 自定义 awaitable: co_user_task() —— 和 co_user_fastcgi_task 同模式 ----
    // initiate: 把 handler 存自己的 handler_queue_，丢 client_context 队列
    // 完成后 notify_on_done_ 取 handler dispatch 回 server_ioc_ → co_await 恢复
    asio::awaitable<size_t> co_user_task(asio::use_awaitable_t<> h = {});

    // 完成 async_run_task_fun 后手动调 —— 也可以用 enable_auto_notify_ 自动调
    void notify_on_done_();

    // ---- 状态（public 字段，socket_client 风格）----
    bool iserror                          = false;
    bool isco                             = false;
    bool iswait_exit                      = false;
    unsigned int exptime                  = 0;
    std::atomic<unsigned int> timeout_end = 0;

    std::string url;
    std::string host;
    unsigned int port = 0;
    std::string error_msg;

    asio::error_code ec;
    asio::strand<asio::io_context::executor_type> strand_;

    // sock / sslsock 同 socket_client 字段名（ssl 暂不用，先占位）
    std::shared_ptr<asio::ip::tcp::socket> sock                       = {nullptr};
    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsock = {nullptr};
    std::shared_ptr<asio::ssl::context> ssl_context                   = {nullptr};

    // 业务注入: server_ioc_ 回落 handler 用（co_user_task 完成后 dispatch 到这个 ioc）
    asio::io_context *server_ioc_ = nullptr;
    // 自定义 awaitable co_user_task 存进来的 handler（任务完成后取出来 dispatch 回落）
    std::deque<asio::detail::awaitable_handler<asio::any_io_executor, size_t>> handler_queue_;
    std::mutex handler_mu_;

    // mqtt 专用状态
    mqtt_client_config_t config;
    mqtt_reason connack_rc = mqtt_reason::success;
    mqtt_connack_props server_props;// CONNACK 里 server 宣告的能力（receive_maximum 等）
    bool session_present = false;
    bool connected_      = false;

    // ---- 回调注入（socket_client 的 run_loop_fun 风格）----
    // run_loop 收到 broker 发来的 PUBLISH 时调这个
    std::function<void(std::shared_ptr<mqtt_client>, const mqtt_recv_packet_t &)> run_loop_fun                        = nullptr;
    std::function<asio::awaitable<void>(std::shared_ptr<mqtt_client>, const mqtt_recv_packet_t &)> async_run_loop_fun = nullptr;

    // CONNACK 收完后 / 被动 DISCONNECT / 主动 DISCONNECT 完成后的钩子
    std::function<void(std::shared_ptr<mqtt_client>, mqtt_reason rc)> on_connect_fun    = nullptr;
    std::function<void(std::shared_ptr<mqtt_client>, mqtt_reason rc)> on_disconnect_fun = nullptr;

    // PUBACK / SUBACK / UNSUBACK 回调（可选，默认不设则高层 awaitable 直接返回）
    std::function<void(std::shared_ptr<mqtt_client>, uint16_t packet_id, mqtt_reason rc)> on_puback_fun                          = nullptr;
    std::function<void(std::shared_ptr<mqtt_client>, uint16_t packet_id, const std::vector<mqtt_reason> &codes)> on_suback_fun   = nullptr;
    std::function<void(std::shared_ptr<mqtt_client>, uint16_t packet_id, const std::vector<mqtt_reason> &codes)> on_unsuback_fun = nullptr;

    // run_task_fun: client_context 启动 client 时调 —— 放整段 CONNECT/SUBSCRIBE/PUBLISH 协程进去
    //                完成后通知 handler（完全对齐 socket_client 的 run_task_fun 模式）
    std::function<void(std::shared_ptr<mqtt_client>)> run_task_fun                        = nullptr;
    std::function<asio::awaitable<void>(std::shared_ptr<mqtt_client>)> async_run_task_fun = nullptr;

    // ---- packet id（同 socket_client 的 myid 但按 client 自己命名空间计数）----
    std::atomic<uint16_t> next_packet_id_{0};

    // ---- reply promise/future 表（public 方便外部等；dispatch_ 线程写，高层线程轮询）----
    // 注: socket_client 没这个 —— mqtt 因同步等待 ACK 必须有
    std::mutex reply_mu_;
    std::unordered_map<uint16_t, std::promise<std::vector<mqtt_reason>>> pending_subacks_;
    std::unordered_map<uint16_t, std::promise<std::vector<mqtt_reason>>> pending_unsubacks_;
    std::unordered_map<uint16_t, std::promise<mqtt_reason>> pending_pubacks_;

    // ---- 拆帧缓冲（public 方便 run_loop 直接操作）----
    std::vector<uint8_t> rdBuf_;
    std::deque<std::pair<uint8_t, std::vector<uint8_t>>> rdyPkts_;
    std::atomic<bool> run_loop_alive_{false};

    // ---- 已收到的 PUBLISH（极简用法：没挂 run_loop_fun / async_run_loop_fun 时才往这里攒）----
    // 挂了回调的会话不再产生第二份拷贝，received 保持空。
    // 这里是滑动窗口：攒到 received_max 条或 received_bytes_max 字节后丢最旧（新条总要收），
    // 被挤掉的条数计入 received_dropped。读完请自行 clear()（同时把 received_bytes_ 归零）。
    std::vector<mqtt_publish_info> received;
    std::size_t received_max       = 1024;                           // 条数界
    std::size_t received_bytes_max = MQTT_CLIENT_RECEIVED_BYTE_LIMIT;// 字节界（topic + payload 长度和）
    std::size_t received_bytes_    = 0;                              // received 现有条目的 topic + payload 累计
    std::size_t received_dropped   = 0;                              // 被挤旧的条数

  private:
    uint16_t gen_packet_id_();
    void dispatch_(uint8_t fixed, const uint8_t *body, size_t len);
    // 把一条已收完的 PUBLISH 投递给业务（回调或 received 窗口）
    void deliver_publish_(mqtt_publish_info &info);
    // keepalive 定时协程：connected_ 期间每 keepalive 秒发一条 PINGREQ
    asio::awaitable<void> async_keepalive_loop();

    // 出站写串行化：dispatch_（strand_ 线程）回 ACK 与高层协程操作（ioc 线程）
    // 都可能写同一个 sock，统一在 write_mu_ 内做同步 asio::write，杜绝并发
    // async_write 导致的发送缓冲交错 / Asio 未定义行为。
    std::mutex write_mu_;

    // client → broker 报文构造（static，不抢公共接口）
    static std::vector<uint8_t> make_client_connect_(const mqtt_client_config_t &cfg);
    static std::vector<uint8_t> make_client_subscribe_(uint16_t pid, std::string_view filter, uint8_t qos, bool no_local, bool retain_as_published, uint8_t retain_handling);
    static std::vector<uint8_t> make_client_unsubscribe_(uint16_t pid, std::string_view filter);
    static std::vector<uint8_t> make_client_pingreq_();
};

// manager —— 对齐 get_http_socket_obj / get_http_websocket_obj
std::map<std::string, std::shared_ptr<mqtt_client>> &get_http_mqtt_obj();

}// namespace http

#endif
