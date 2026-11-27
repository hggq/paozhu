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
#include "http2_ring_queue.h"

namespace http
{
struct http2_send_data_t;
class httppeer;

void h2_write_data_header(char *header, unsigned int sid, unsigned char flags, unsigned int len);
// 组一枚完整 DATA 帧（9 字节帧头 + 载荷深拷贝），入环前的标准组帧形状。
std::string h2_make_data_chunk(unsigned int sid, unsigned char flags, const char *payload, unsigned int len);

// post_write 字节闸：单连接发送环积压总量上限（累计闸——环空时任何单帧放行）。
inline constexpr unsigned long long MQTT_SEND_RING_BYTE_LIMIT = 1024 * 1024;

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
    // post_write — 跨线程入队写数据，不阻塞调用方（MQTT 定时线程 / HTTP handler 用）。
    // 数据进本连接发送环，由 消费者协程（唯一写者）串行写出。
    // 返回 false = 数据被丢弃：连接已关闭 / 未分配发送环 / 环满（capacity_）/
    // 积压字节过 MQTT_SEND_RING_BYTE_LIMIT（环空时单帧不受闸）。调用方必须检查。
    // 其中"环满 / 积压过闸"那几次会累加进 http2_ring_overflow_count；
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

    bool send_switch101();
    asio::awaitable<bool> co_send_switch101();

    void send_ping();
    asio::awaitable<void> http2_send_ping();
    void send_recv_setting();
    // WINDOW_UPDATE 必须区分层级：连接级只认 stream id = 0，流级只认具体流。
    // 不能一次推入「连接级 + 流级」两个帧且用同一个增量：那等于每来一个 POST 就把
    // 对端连接窗口顶高 16MB，约 129 次后超过 2^31-1，对端必须按 FLOW_CONTROL_ERROR 断连。
    // 这两个只负责组帧投递，返回是否进了发送环；调用方用下面的 grant_*，不要直接用它们。
    bool push_window_update_conn(unsigned int up_num);
    bool push_window_update_stream(unsigned int stmid, unsigned int up_num);
    // 补量额度先记在账上再投递。投递失败（发送环满）时额度留在账里，由本连接发送环
    // 每腾出一个槽位重投一次，所以「这一帧没发出去」不再等于「这条链路停摆」。
    // 流级补量已移至 httppeer::grant_recv_window(peer_session, up_num)，
    // client_session 只负责连接级 grant。
    void grant_conn_window(unsigned int up_num);
    bool has_pending_window_update() const
    {
        return h2_conn_send.conn_grant_pending > 0 || h2_conn_send.grant_count > 0;
    }
    // 由 消费者协程在 head_ 前移之后调用：把记在账上、还没投递出去的补量发走。
    void flush_pending_window_update();

    // 返回撤流帧有没有真的进发送环；false 表示环满被丢弃，调用方不能当作已收尾。
    void stop();
    void cancel();
    void half_stop();
    asio::awaitable<std::string> async_stop();
    // GOAWAY 入 FIFO 环，环满时重试并受 2 秒预算限制；入环后等待它写出再返回。
    // 调用方随后会 cancel()，因此这里必须等 GOAWAY 真正上线。
    asio::awaitable<void> async_send_goway(unsigned int last_stream_id = 0, unsigned int error_code = 0);
    // 有界等待发送环和待发送 peer 排空。只等待并让出 strand，不直接写 socket；
    // 唯一写者是发送协程。上限 2 秒，避免客户端停读时无限阻塞关闭流程。
    // include_producers=false 时只检查已排队 peer，不等待尚未注册的响应生产者。
    asio::awaitable<bool> flush_send_ring(bool include_producers = true);

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
    // 控制帧和兼容流式接口的完整帧仍走同一个 FIFO 环。
    // 窗口停等只存在于 ring_client_http2_send_loop 自己的 http2_wait_send_event；
    // WINDOW_UPDATE / 退款只更新窗口值并唤醒发送环逐流重查。
    bool push_rst_stream(unsigned int sid, unsigned int code);

    bool push_control_frame(const unsigned char *fr, unsigned int n);

    // ---- http2 send loop perf stats (per connection) ----
    // 循环迭代次数 / FIFO 单帧发送次数 / peer 组批发送次数
    std::atomic<unsigned long long> h2_send_loop_iters{0};
    std::atomic<unsigned long long> h2_send_fifo_count{0};
    std::atomic<unsigned long long> h2_send_batch_count{0};
    // async_write 累计耗时 (ns)
    std::atomic<unsigned long long> h2_async_write_ns{0};
    // wait_send_event 协程休眠累计 (ns)
    std::atomic<unsigned long long> h2_wait_event_ns{0};

    struct h2_conn_send_state_t
    {
        std::int64_t window = 65535;
        std::uint64_t reserved = 0;
        unsigned int initial_window_size = 65535;
        unsigned int max_frame_size = 16384;
        httppeer *send_head = nullptr;
        httppeer *send_tail = nullptr;
        unsigned int send_count = 0;
        httppeer *grant_head = nullptr;
        httppeer *grant_tail = nullptr;
        unsigned int grant_count = 0;
        std::atomic<unsigned int> active_producers{0};
        unsigned int rst_retry_count = 0;
        unsigned int fifo_cleanup_count = 0;
        unsigned int conn_grant_pending = 0;
    } h2_conn_send;
    mutable std::mutex h2_send_window_mutex;
    void http2_enqueue_peer(const std::shared_ptr<httppeer> &peer);
    void http2_unlink_peer(httppeer *peer);
    void http2_mark_peer_rst(const std::shared_ptr<httppeer> &peer, unsigned int error_code);
    std::shared_ptr<httppeer> http2_take_next_peer();
    bool http2_peer_is_blocked(const std::shared_ptr<httppeer> &peer);
    bool http2_begin_peer_refill(const std::shared_ptr<httppeer> &peer);
    void http2_finish_peer_refill(const std::shared_ptr<httppeer> &peer, bool ready);
    void http2_complete_active_peer(const std::shared_ptr<httppeer> &peer, bool success);
    void http2_cancel_all_pending_peers(bool wake = true);
    void http2_begin_content_stream(const std::shared_ptr<httppeer> &peer);
    void http2_end_content_stream(const std::shared_ptr<httppeer> &peer);
    void http2_expire_blocked_peers();
    bool http2_has_pending_peers();
    bool http2_has_queued_peers();
    bool http2_has_runnable_peer();
    asio::awaitable<void> http2_wait_send_event(std::chrono::nanoseconds wait_time);

    std::int64_t http2_conn_window_available() const;
    std::int64_t http2_conn_window_available_locked() const;
    std::int64_t http2_peer_window_available(const httppeer *peer) const;
    unsigned long long http2_try_reserve_send_window(httppeer *peer,
                                                     unsigned long long want,
                                                     bool allow_sub_frame = false);
    void http2_refund_peer_window(httppeer *peer);
    void http2_commit_conn_window(unsigned long long bytes);
    bool http2_apply_initial_window_size(unsigned int size, const http2parse &parser);

    void http2_mark_stream_dead(const std::shared_ptr<httppeer> &peer);
    struct h2_content_producer_guard
    {
        h2_content_producer_guard(client_session *session, const std::shared_ptr<httppeer> &peer);
        ~h2_content_producer_guard();
        h2_content_producer_guard(const h2_content_producer_guard &)            = delete;
        h2_content_producer_guard &operator=(const h2_content_producer_guard &) = delete;

      private:
        client_session *sess;
        std::shared_ptr<httppeer> peer;
    };
    void http2_content_stream_failed(const std::shared_ptr<httppeer> &peer, bool rst);
    void http2_enqueue_grant(httppeer *peer);
    void http2_unlink_grant(httppeer *peer);
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

    // 发送环满（push 返回 false）导致帧没能入队的次数。非零即说明发送侧出现过背压，
    // 这一片留在调用方手里等下一轮，是排查「响应缺洞 / 客户端挂死」的第一手线索。
    // 现在往这一个数里记的有四处，字段名的 http2 是历史遗留，语义已经是
    // "本连接的发送环拒过多少片"：
    //   - 环满时控制帧 push 被拒（RST/GOAWAY 已直写，不入环）；
    //   - post_write 的三道闸（环满 / 积压过 MQTT_SEND_RING_BYTE_LIMIT / push 失败），
    //     也就是 websocket_api::send()、mqtt 的出站、socket_api::send() 三条路共用这一个数。
    //     post_write 最前面那条"连接已关或没环"不算背压，不进这个数。
    //   - FIFO 环满时流式帧 push 被拒（http2_send_content_frame 原地重试期间的每一次拒绝）。
    // 这个数目前在哪里打印：http2 连接收尾时的 "http2 ring stats overflow"；原生 socket 收尾时的
    // "tcp ring close ... refuse="。websocket / mqtt 的连接目前没有落这一行的地方。
    std::atomic<unsigned long long> http2_ring_overflow_count = 0;
    // 无法重投的控制帧（RST / GOAWAY / 本端 SETTINGS / SETTINGS ACK / PING ACK）被环满丢掉的次数：
    // co_send_setting / send_recv_setting /
    // readping 里 push 被拒各加一次。
    // DATA 与收尾帧走重投，WINDOW_UPDATE 走欠账重投（下面的 requeue 计数），都不计在这里；
    // 分开才看得出「背压」「待重投」「真丢帧」三种差别。
    // 只有 http2 会话会走到那几个函数，websocket / mqtt / 原生 socket 上它恒为 0。
    std::atomic<unsigned long long> http2_ring_queue_drop_count = 0;
    // 本端接收窗口的补量帧被发送环拒过多少次（每一次都留在账上等下一个腾出的槽位）。
    // 收尾时账上如果还有余额，说明这些额度到连接关闭都没发出去。
    std::atomic<unsigned long long> http2_window_update_requeue_count = 0;
    // 补量欠账由 strand 串行化：连接级在 h2_conn_send，流级在 peer->h2recv。
    // 旧发送闸门让路计数，现在恒 0，留着保收尾日志口径。
    // 这一支在 push() 之前就 return，所以它不体现在 overflow 里；不单独记的话
    // "环从没满过" 和 "环满了但每次都提前让路" 两种情况读起来一模一样。
    // 同样是 http2 专用，websocket / mqtt / 原生 socket 上恒为 0。
    std::atomic<unsigned long long> http2_ring_backpressure_count = 0;

    std::atomic_bool http2_need_wakeup = false;
    std::atomic_bool http2_fifo_write_active = false;
    std::unique_ptr<http2_send_queue_cache> http2_ring_queue = nullptr;
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
