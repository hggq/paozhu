#ifndef __HTTP_CLIENT_SESSION_H
#define __HTTP_CLIENT_SESSION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif// defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <thread>
#include <memory>
#include <string_view>
#include <span>
#include <atomic>

#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>

#ifndef _MSC_VER
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#endif

#ifndef WIN32
#include <sys/wait.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include <array>
#include <iostream>
#include <ctime>
#include <map>
#include <map>
#include <thread>
#include <mutex>
#include <filesystem>
#include <future>
#include <functional>
#include <stdexcept>

#include <zlib.h>

#include "sendqueue.h"
#include "http_header.h"
#include "http2_frame.h"
#include "cookie.h"
#include "clientdatacache.h"
#include "send_ring_queue.h"

namespace http
{
class httppeer;

void h2_write_data_header(char *header, unsigned int sid, unsigned char flags, unsigned int len);
// 组一枚完整 DATA 帧（9 字节帧头 + 载荷深拷贝）。
std::string h2_make_data_chunk(unsigned int sid, unsigned char flags, const char *payload, unsigned int len);

inline constexpr std::size_t HTTP1_OPTIMISTIC_WRITE_LIMIT = 4096;

inline asio::awaitable<std::size_t> http1_optimistic_write(asio::ip::tcp::socket &socket, std::string_view data)
{
    if (data.empty() || data.size() > HTTP1_OPTIMISTIC_WRITE_LIMIT || !socket.is_open())
    {
        co_return 0;
    }
    if (!socket.non_blocking())
    {
        asio::error_code ec;
        socket.non_blocking(true, ec);
        if (ec)
        {
            co_return 0;
        }
    }

    asio::error_code ec;
    auto written = socket.write_some(asio::buffer(data.data(), data.size()), ec);
    if (written == data.size())
    {
        co_return written;
    }
    if (written > 0)
    {
        co_await asio::async_write(socket,
                                   asio::buffer(data.data() + written, data.size() - written),
                                   asio::use_awaitable);
        co_return data.size();
    }
    co_return 0;
}

// post_write 字节闸：单连接发送环积压总量上限（累计闸——环空时任何单帧放行）。
inline constexpr unsigned long long SEND_RING_BYTE_LIMIT = 1024 * 1024;

static std::string make_h2c_switch101_response();
struct http2parse;
class client_session : public std::enable_shared_from_this<client_session>
{
  public:
    client_session(asio::io_context &);
    ~client_session();
    unsigned int send_writer(const std::string &msg, const std::shared_ptr<httppeer> &peer = {});
    unsigned int send_writer(std::string_view msg, const std::shared_ptr<httppeer> &peer = {});
    unsigned int send_writer(const unsigned char *, unsigned int, const std::shared_ptr<httppeer> &peer = {});
    // post_write — 跨线程入队写数据，不阻塞调用方（WebSocket / MQTT / 原生 Socket 用）。
    // 数据进本连接发送环，由消费者协程（唯一写者）串行写出。
    // 返回 false = 数据被丢弃：连接已关闭 / 未分配发送环 / 环满（capacity_）/
    // 积压字节过 SEND_RING_BYTE_LIMIT（环空时单帧不受闸）。调用方必须检查。
    // 其中“环满 / 积压过闸”那几次会累加进 send_ring_overflow_count；
    // 连接已关或未分配环那一类不算背压，不计数。
    bool post_write(std::string_view msg);
    bool post_write(const unsigned char *, std::size_t);

    bool isopensocket();
    std::shared_ptr<client_session> get_ptr();
    std::string getremoteip();
    unsigned int getremoteport();

    std::string getlocalip();
    unsigned int getlocalport();

    asio::awaitable<void> co_send_setting();

    asio::awaitable<bool> co_send_switch101();

    void send_recv_setting();
    // WINDOW_UPDATE 必须区分层级：连接级只认 stream id = 0，流级只认具体流。
    bool push_window_update_conn(unsigned int up_num);
    bool push_window_update_stream(unsigned int stmid, unsigned int up_num);
    void grant_conn_window(unsigned int up_num);

    // 置关闭旗并唤醒发送消费者；socket 的取消与关闭也在这里完成。
    void stop();
    void cancel();
    void half_stop();
    asio::awaitable<std::string> async_stop();
    // GOAWAY 先刷新已入队响应，再入控制帧链表；入链后等待它写出或达到 2 秒预算。
    // 调用方随后会 cancel()，因此这里必须等 GOAWAY 真正上线。
    asio::awaitable<void> async_send_goway(unsigned int last_stream_id = 0, unsigned int error_code = 0);
    // 有界等待控制帧链表和待发送 peer 排空。只等待并让出 strand，不直接写 socket；
    // 唯一写者是发送协程。上限 2 秒，避免客户端停读时无限阻塞关闭流程。
    // include_cleanup=false 时只检查队列与 RST 重试，true 还检查控制帧清理计数；
    // 两者都不等待尚未入队的业务协程。
    asio::awaitable<bool> flush_http2_send_queue(bool include_cleanup = true);

    asio::awaitable<unsigned int> async_send_writer(const std::string &msg, std::shared_ptr<httppeer> peer = {});
    asio::awaitable<unsigned int> async_send_writer(std::string_view msg, std::shared_ptr<httppeer> peer = {});
    // 头+正文两块 buffer 一次 gather 写出，省掉把正文 append 进头串的重分配与拷贝。
    // 两个 view 指向的内存必须活过本协程的 co_await（调用方协程帧里的局部串/peer 成员）。
    asio::awaitable<unsigned int> async_send_writer(std::string_view head, std::string_view body);
    asio::awaitable<unsigned int> async_send_writer(const unsigned char *, unsigned int, std::shared_ptr<httppeer> peer = {});
    asio::awaitable<bool> read_some(unsigned int &readnum, std::string &log_item);
    asio::awaitable<bool> read_first(unsigned int &readnum);
    // 在已读 readnum 字节的基础上继续补读，直到累计 >= need 或出错/关闭。
    // read_first/read_some 都是单次 async_read_some（不保证读满），协议判定与
    // HTTP/2 连接前言校验不能假设「一次读够」，否则 TCP 拆包会误杀合法连接。
    // limit 为本次允许占用的缓冲上限（_cache_data 是 CACHE_DATA_LENGTH 的池缓冲，
    // h2c 升级时调用方需为待追加的首帧留出尾部空间）。
    // 返回 true 表示失败（已置 isclose/iserror），readnum 为实际累计字节数。
    asio::awaitable<bool> read_at_least(unsigned int need, unsigned int &readnum, unsigned int limit = CACHE_DATA_LENGTH);
    asio::awaitable<bool> read_socket(unsigned int &readnum, std::string &log_item);
    // void append(const std::string &item);
    // void append(const unsigned char *buffer, unsigned int buffersize);

    void wake_ring_consumer();

    // ---- h2 发送原语 ----
    // 普通响应注册为待发送 peer，由唯一发送协程按轮转顺序构造 HEADERS/DATA；
    // 控制帧和同步流式接口的完整帧进入控制帧链表，不与普通 peer 共用队列。
    // 窗口停等只存在于 ring_client_http2_send_loop 自己的 http2_wait_send_event；
    // WINDOW_UPDATE / SETTINGS 只更新窗口值并唤醒 HTTP/2 发送协程逐流重查。
    bool push_rst_stream(unsigned int sid, unsigned int code,
                         const std::shared_ptr<httppeer> &peer = {});

    bool push_control_frame(const unsigned char *fr, unsigned int n,
                            const std::shared_ptr<httppeer> &peer = {});

    std::atomic<std::int64_t> http2_send_window{65535};
    std::atomic<unsigned long long> http2_send_totoal_bytes{0};
    std::atomic<unsigned long long> http2_send_reserved{0};
    std::atomic<unsigned int> http2_rst_retry_count{0};
    std::atomic<unsigned int> http2_control_cleanup_count{0};

    struct h2_control_frame_t
    {
       std::weak_ptr<httppeer> stream_peer;
       std::vector<unsigned char> content;
    };

    std::list<h2_control_frame_t> http2_send_control_frame_list; // HTTP/2 控制帧和 httppeer 同步流式帧
    // 同步业务线程会直接入帧，strand 发送环负责取出；链表操作必须串行。
    mutable std::mutex http2_send_control_frame_mutex;
    std::atomic_bool http2_control_frames_ready{false};
    mutable std::mutex h2_send_window_mutex;

    std::list<std::shared_ptr<httppeer>> http2_send_queue;

    bool http2_send_queue_add(std::shared_ptr<httppeer> peer);
    asio::awaitable<bool> http2_wait_peer_completion(const std::shared_ptr<httppeer> &peer);

    void http2_mark_peer_rst(const std::shared_ptr<httppeer> &peer, unsigned int error_code);
    bool http2_begin_peer_refill(const std::shared_ptr<httppeer> &peer);
    void http2_finish_peer_refill(const std::shared_ptr<httppeer> &peer, bool ready);
    void http2_complete_active_peer(const std::shared_ptr<httppeer> &peer, bool success);
    void http2_cancel_all_pending_peers(bool wake = true);
    bool http2_has_pending_peers();
    bool http2_has_queued_peers();
    asio::awaitable<void> http2_wait_send_event(std::chrono::nanoseconds wait_time);

    bool http2_apply_initial_window_size(unsigned int size, const http2parse &parser);

    void http2_mark_stream_dead(const std::shared_ptr<httppeer> &peer);
    void http2_content_stream_failed(const std::shared_ptr<httppeer> &peer, bool rst);
    unsigned int http2_send_content_frame(std::string_view frame, const std::shared_ptr<httppeer> &peer);
    asio::awaitable<unsigned int> async_http2_send_content_frame(std::string_view frame, std::shared_ptr<httppeer> peer);
    unsigned int http2_send_data_payload(const std::shared_ptr<httppeer> &peer, std::string_view payload, unsigned char end_flags);
    asio::awaitable<unsigned int> async_http2_send_data_payload(std::shared_ptr<httppeer> peer, std::string_view payload, unsigned char end_flags);
    bool http2_try_push_content_raw(std::string_view frame, std::string *owned, const std::shared_ptr<httppeer> &peer);
    bool http2_push_content_raw_sync(std::string &&frame, const std::shared_ptr<httppeer> &peer);
    bool http2_push_content_raw_sync(const char *frame, unsigned int frame_size, const std::shared_ptr<httppeer> &peer);
    asio::awaitable<bool> http2_push_content_raw(std::string frame, std::shared_ptr<httppeer> peer);
    asio::awaitable<bool> http2_push_content_raw(const char *frame, unsigned int frame_size, std::shared_ptr<httppeer> peer);

  public:
    unsigned char *_cache_data  = nullptr;
    std::atomic_uint time_limit = 0;

    bool isssl   = false;
    bool isgoway = false;
    // 关闭旗：拒绝新出站数据；发送协程排空现有环后退出。跨线程读写，所以是原子。
    std::atomic_bool isclose{false};
    std::atomic_bool iserror{false};
    bool half_close = false;
    bool ws_deflate = false;// WebSocket 已协商 permessage-deflate（101 回带扩展头时置位）

    unsigned char httpv = 0;
    std::string client_ip;
    unsigned int client_port = 0;
    unsigned int time_begin  = 0;
    asio::error_code ec;

    std::unique_ptr<asio::ip::tcp::socket> socket                       = nullptr;
    std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket = nullptr;
    asio::strand<asio::io_context::executor_type> strand_;

    std::atomic<unsigned int> last_time_interval = 0;
    std::atomic<unsigned int> remote_initial_window_size = 65535;
    // 对端 SETTINGS_MAX_FRAME_SIZE（RFC 9113 §6.5.2，合法区间 [16384, 16777215]，
    // 默认 16384）。发送侧拿它给单帧载荷封顶，超过就是对端必须拒绝的帧。
    std::atomic<unsigned int> remote_max_frame_size = 16384;

    // 普通协议发送环拒绝的帧数；HTTP/2 已改走控制帧链表，不再计入这里。
    std::atomic<unsigned long long> send_ring_overflow_count = 0;
    std::atomic_bool http2_need_wakeup = false;
    std::atomic_bool http2_control_write_active = false;
    std::unique_ptr<send_ring_cache> send_ring_queue = nullptr;
    std::shared_ptr<asio::steady_timer> http2_consumer_timer;

    // 关闭旗：置位后发送生产者在循环边界静音退出，不再有新发送。
    std::atomic_bool http2_closing{false};
    // 本连接活跃发送协程数，收尾日志用，收尾后应归零。
    std::atomic<unsigned int> http2_active_senders{0};

    // HPACK 编码器动态表（响应头方向）。只在喂入锁临界区内编码时读写，
    // SETTINGS_HEADER_TABLE_SIZE 解析同在 strand 上更新上限；连接关闭即销毁。
    hpack_enc_table hpack_enc;
};
}// namespace http

#endif
