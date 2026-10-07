#ifndef HTTP_WEBSOCKETS_CLIENT_H
#define HTTP_WEBSOCKETS_CLIENT_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/strand.hpp>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <string_view>
#include "request.h"
#include "datetime.h"
#include "ws_wire.h"

namespace http
{
struct websocket_parameter_t
{
    std::string name;
    std::string value;
};

// WebSocket 帧 opcode（RFC 6455 §5.2）。
// 客户端私有定义，刻意不与服务端 http::ws::opcode 共用。
enum class ws_opcode : unsigned char
{
    continuation = 0x00,
    text         = 0x01,
    binary       = 0x02,
    close        = 0x08,
    ping         = 0x09,
    pong         = 0x0A
};

class websocket_client : public std::enable_shared_from_this<websocket_client>
{
  public:
    void timeout(unsigned int t) { exptime = t; };
    unsigned int timeout() { return exptime; };
    // timeout==0 视为"无限期但仍受看护"：写入大哨兵值，永不过期，
    // 但连接仍留在超时链表里，由 dur 心跳（dur_time_loop_fun）每拍刷新，不会被 idle 误杀。
    static constexpr unsigned int kInfiniteTimeoutSec = 0x3FFFFFFF; // ~34 年
    void reset_timeout()
    {
        timeout_end.store(timeid() + (exptime == 0 ? kInfiniteTimeoutSec : exptime));
    };
    void set_timeout(unsigned int t)
    {
        timeout_end.store(timeid() + t);
    };
    unsigned int get_timeout()
    {
        return timeout_end.load();
    };

  public:
    websocket_client();
    ~websocket_client();
    void reset();
    void set_header(std::string_view name, std::string_view value);
    void add_headers(std::string_view raw);// 批量: "K1:V1\r\nK2:V2" 或 "K1:V1;K2:V2"
    void set_host(std::string_view name);
    void set_port(unsigned int);
    void set_url(std::string_view name);
    asio::awaitable<bool> async_init_https_sock();
    asio::awaitable<bool> async_init_http_sock();

    asio::awaitable<bool> async_connect(std::string_view url, unsigned int time_out_num = 0);
    asio::awaitable<bool> async_connect();

    asio::awaitable<unsigned int> async_write(unsigned char *data, unsigned int buffersize);
    asio::awaitable<unsigned int> async_write(std::string_view value);
    asio::awaitable<unsigned int> async_read(unsigned char *data, unsigned int buffersize);
    asio::awaitable<unsigned int> async_read(std::string &data);

    asio::awaitable<unsigned int> async_text_read();

    unsigned int write(unsigned char *data, unsigned int buffersize);
    unsigned int write(std::string_view value);
    unsigned int read(unsigned char *data, unsigned int buffersize);
    unsigned int read(std::string &data);

    //asio::awaitable<unsigned int> async_text_write(unsigned char *data, unsigned int buffersize);
    asio::awaitable<unsigned int> async_text_write(std::string_view value);
    //asio::awaitable<unsigned int> async_data_write(unsigned char *data, unsigned int buffersize);
    asio::awaitable<unsigned int> async_data_write(std::string_view value);

    void run_loop();
    asio::awaitable<void> async_run_loop();

    void close_connect();
    std::string make_http_header();
    bool process_handshake(unsigned char *read_data, unsigned int buffersize);
    void process_header(unsigned char *read_data, unsigned int data_bein, unsigned int data_end);
    asio::awaitable<bool> websocket_handshake();

    // ---- 出站帧编码（客户端出站必须掩码，RFC 6455 §5.1）----
    bool is_control_frame(ws_opcode op) const;
    void make_mask_key();
    std::string serialize_frame(ws_opcode op, std::string_view payload, bool fin = true, unsigned char rsv = 0);

    // ---- 出站写队列 ----
    // 所有 WebSocket 帧出站必须经 async_send_frame()，由队列串行化，
    // 避免自动 pong 与业务写并发交错。业务协程与 strand 上的 pong 可能来自
    // 不同线程，队列由 send_queue_mutex_ 保护；enqueue_frame 返回 true
    // 表示调用者接管队列消费（pump_send_queue）。
    asio::awaitable<unsigned int> async_send_frame(std::string frame);
    bool enqueue_frame(std::string frame);
    asio::awaitable<void> pump_send_queue();

    std::string make_pong();
    std::string make_pong(std::string_view payload);
    // 注：make_ping/make_ws_header/make_mark 无调用方， 已删除。
    // make_ws_text/make_ws_data 为向后兼容接口，内部转发 serialize_frame()。

    int make_ws_data(char *msg, unsigned int msgLen, std::string &outBuf);
    int make_ws_data(std::string_view msg, std::string &outBuf);
    int make_ws_text(char *msg, unsigned int msgLen, std::string &outBuf);
    int make_ws_text(std::string_view msg, std::string &outBuf);

    unsigned int process_data(unsigned char *inputdata, unsigned int buffersize);
    asio::awaitable<void> async_recv_finish();
    void reset_recv_status();

    bool add_client_task_loop();
    void set_deflate(bool isstatus);
    bool un_pack(const std::string &pack_data, std::string &outBuf);
    bool un_pack(std::string &outBuf);
    int compress(const std::string &pack_data, std::string &outBuf);

  public:
    bool iserror     = false;
    bool isssl       = false;
    bool isbody      = false;
    bool isfinish    = false;
    bool iswait_exit = false;
    bool isco        = false;
    bool isdeflate   = false;
    // 握手时自动 offer permessage-deflate（双 no-context 参数），默认开；
    // 对端回包参数不符会判协商失败断连，set_deflate(false) 可整体关闭
    bool open_deflate              = true;
    unsigned char ready_state      = 0;
    unsigned int exptime           = 0;
    unsigned char cur_process_type = 0;
    unsigned char key_str[16];
    unsigned char mask_key[4];
    // 测试用：置 true 时 make_mask_key() 沿用已填入的 mask_key，
    // 便于与 RFC 6455 §5.7 的固定掩码向量核对掩码结果
    bool mask_key_fixed               = false;
    unsigned int offsetnum            = 0;
    unsigned int durtime              = 8;
    unsigned int port                 = 0;
    unsigned int val_size             = 0;
    unsigned int myid                 = 0;
    unsigned int groupid              = 0;
    std::atomic_flag socket_read_lock = ATOMIC_FLAG_INIT;
    std::deque<std::string> send_queue_;          // 出站帧队列（元素为完整帧）
    std::mutex send_queue_mutex_;                 // 队列跨线程访问（业务协程 vs strand 上的 pong）
    bool sending_                         = false;// 是否有发送协程正在消费队列
    std::atomic<unsigned int> timeout_end = 0;

    // ---- 入站帧流状态机（process_data 内部记账，帧边界与 TCP 读边界解耦）----
    bool in_payload_phase_ = false;                       // 当前处于载荷消费阶段
    unsigned char in_hdr_[12];                            // 半截帧头缓存（2 基 + 8 扩展 + 无掩码位）
    unsigned int in_hdr_have_           = 0;              // 已收帧头字节数
    unsigned int in_hdr_need_           = 2;              // 当前帧头所需总字节数
    ws_opcode in_op_                    = ws_opcode::text;// 当前帧 opcode
    bool in_fin_                        = false;          // 当前帧 fin
    bool in_ctl_                        = false;          // 当前帧是控制帧
    bool in_msg_open_                   = false;          // 分片消息组包中（recv_data.content 尚未完整）
    unsigned long long in_payload_left_ = 0;
    std::string in_ctl_buf_;         // 控制帧载荷缓冲
    std::string stream_pending_;     // 一条消息交付后本轮未消费的剩余字节
    ws::utf8_stream_checker in_utf8_;// text 消息 UTF-8 流式校验，跨分片保持状态
    // ---- 入站 permessage-deflate（已协商 isdeflate 时，RSV1 消息先解压再交付）----
    bool in_frame_rsv1_       = false;  // 当前帧头 RSV1 位，用于拒绝续帧/控制帧误用
    bool in_msg_deflated_     = false;  // 当前消息（含其续帧）属于一条压缩流
    bool in_msg_too_big_      = false;  // 解压后字节超消息上限（sink 归因 1009）
    bool in_inflate_utf8_bad_ = false;  // 解压后 text 字节非法 UTF-8（sink 归因 1007）
    ws::permessage_inflate in_inflator_;// no-context：每条消息末帧冲净并重置
    static bool inflate_sink(const unsigned char *out, std::size_t n, void *ud);
    bool on_inflated_bytes(const unsigned char *out, std::size_t n);

    std::string url;
    std::string host;
    std::string error_msg;

    unsigned char *data = nullptr;
    struct ws_pack_data
    {
        bool isfile                     = false;
        bool isfinish                   = false;
        bool isdeflate                  = false;
        bool isclose_stream             = false;
        unsigned char fin               = 0;
        unsigned char opcode            = 0;
        unsigned long long length       = 0;
        unsigned long long read_length  = 0;
        unsigned long long total_length = 0;
        std::string content;
    } recv_data;
    std::vector<websocket_parameter_t> parameter;
    std::shared_ptr<asio::ip::tcp::socket> sock                       = {nullptr};
    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsock = {nullptr};
    std::shared_ptr<asio::ssl::context> ssl_context                   = {nullptr};
    asio::error_code ec;
    asio::strand<asio::io_context::executor_type> strand_;

    std::function<void(std::shared_ptr<websocket_client>)> run_loop_fun                                                = nullptr;
    std::function<asio::awaitable<void>(std::shared_ptr<websocket_client>, ws_pack_data pack_data)> async_run_loop_fun = nullptr;

    std::function<void(std::shared_ptr<websocket_client>)> dur_time_loop_fun                        = nullptr;
    std::function<asio::awaitable<void>(std::shared_ptr<websocket_client>)> async_dur_time_loop_fun = nullptr;
    std::function<asio::awaitable<void>(std::shared_ptr<websocket_client>)> async_recv_finish_fun   = nullptr;

    std::function<void(std::shared_ptr<websocket_client>)> run_task_fun = nullptr;//time-consuming task
};
}// namespace http
#endif