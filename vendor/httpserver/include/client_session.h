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
#include <deque>
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
class client_session : public std::enable_shared_from_this<client_session>
{
  public:
    client_session(asio::io_context &);
    ~client_session();
    unsigned int send_writer(const std::string &msg);
    unsigned int send_writer(std::string_view msg);
    unsigned int send_writer(const unsigned char *, unsigned int);
    // post_write — 跨线程入队写数据，不阻塞调用方（MQTT 定时线程 / HTTP handler 用）。
    // 数据进本连接发送环，由 消费者协程（唯一写者）串行写出。
    // 返回 false = 数据被丢弃：连接已关闭 / 未分配发送环 / 环满（capacity_）/
    // 积压字节过 MQTT_SEND_RING_BYTE_LIMIT（环空时单帧不受闸）。调用方必须检查。
    // 其中"环满 / 积压过闸"那几次会累加进 http2_ring_overflow_count；
    // 连接已关或未分配环那一类不算背压，不计数。
    bool post_write(std::string_view msg);
    bool post_write(const unsigned char *, unsigned int);

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
    void grant_conn_window(unsigned int up_num);
    void grant_stream_window(unsigned int stmid, unsigned int up_num);
    bool has_pending_window_update() const
    {
        return conn_grant_pending > 0 || stream_grant_pending.empty() == false;
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

    asio::awaitable<unsigned int> async_send_writer(const std::string &msg);
    asio::awaitable<unsigned int> async_send_writer(std::string_view msg);
    // 头+正文两块 buffer 一次 gather 写出，省掉把正文 append 进头串的重分配与拷贝。
    // 两个 view 指向的内存必须活过本协程的 co_await（调用方协程帧里的局部串/peer 成员）。
    asio::awaitable<unsigned int> async_send_writer(std::string_view head, std::string_view body);
    asio::awaitable<unsigned int> async_send_writer(const unsigned char *, unsigned int);
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
    asio::awaitable<bool> http2_wait_window_episode(unsigned int sid, const std::atomic_bool &dead);
    void http2_cancel_conn_waiters();
    void http2_cancel_stream_waiter(unsigned int sid);
    void http2_cancel_all_stream_waiters();
    void http2_close_waiters();
    std::mutex http2_sync_window_mutex;
    std::condition_variable http2_sync_window_cv;
    unsigned long long http2_sync_window_gen = 0;
    void http2_wake_sync_window_waiters();
    asio::awaitable<bool> http2_send_rst(unsigned int s_stream_id, unsigned int stream_error_code);
    bool push_rst_stream(unsigned int sid, unsigned int code);

    std::mutex http2_feed_mutex;
    bool push_control_frame(const unsigned char *fr, unsigned int n);

    struct pending_h2_peer
    {
        std::shared_ptr<httppeer> peer;
        // 无发送窗口时为 true；窗口恢复后重新进入可运行轮转队列。
        bool blocked = false;
        std::chrono::steady_clock::time_point deadline{};
    };
    // 待发送和暂时阻塞的 peer 共用此队列，取出可运行项后放到队尾以实现轮转。
    std::deque<pending_h2_peer> http2_pending_peers;
    std::shared_ptr<httppeer> http2_active_peer;
    std::unordered_map<unsigned int, std::shared_ptr<httppeer>> http2_refilling_peers;
    std::mutex http2_pending_peers_mutex;
    void http2_register_pending_peer(const std::shared_ptr<httppeer> &peer);
    std::shared_ptr<httppeer> http2_take_next_peer();
    void http2_requeue_peer(const std::shared_ptr<httppeer> &peer, bool blocked);
    bool http2_begin_peer_refill(const std::shared_ptr<httppeer> &peer);
    void http2_finish_peer_refill(const std::shared_ptr<httppeer> &peer, bool ready);
    bool http2_has_refilling_peers();
    void http2_complete_active_peer(const std::shared_ptr<httppeer> &peer, bool success);
    void http2_cancel_active_peer();
    void http2_cancel_pending_peer(unsigned int sid);
    void http2_cancel_all_pending_peers(bool wake = true);
    void http2_begin_content_stream(unsigned int sid);
    void http2_end_content_stream(unsigned int sid);
    std::vector<std::shared_ptr<httppeer>> http2_cancel_expired_peers();
    void http2_unblock_pending_peers(bool connection, unsigned int sid);
    bool http2_has_pending_peers();
    bool http2_has_queued_peers();
    bool http2_has_runnable_peer();
    asio::awaitable<void> http2_wait_send_event(std::chrono::nanoseconds wait_time);

    std::mutex http2_dead_streams_mutex;
    std::set<unsigned int> http2_dead_streams;
    std::unordered_map<unsigned int, unsigned int> http2_content_producers;
    std::unordered_map<unsigned int, unsigned int> http2_inflight_frames;
    void http2_mark_stream_dead(unsigned int sid);
    bool http2_stream_is_dead(unsigned int sid);
    void http2_try_reap_dead_mark_locked(unsigned int sid);
    void http2_note_frame_dequeued(unsigned int sid, unsigned char type);
    struct h2_content_producer_guard
    {
        h2_content_producer_guard(client_session *session, unsigned int stream_id);
        ~h2_content_producer_guard();
        h2_content_producer_guard(const h2_content_producer_guard &)            = delete;
        h2_content_producer_guard &operator=(const h2_content_producer_guard &) = delete;

      private:
        client_session *sess;
        unsigned int sid;
    };
    void http2_content_stream_failed(unsigned int sid, unsigned long long refund_bytes, bool rst);
    unsigned int http2_send_content_frame(std::string_view frame);
    asio::awaitable<unsigned int> async_http2_send_content_frame(std::string_view frame);
    unsigned int http2_send_data_payload(unsigned int sid, std::string_view payload, unsigned char end_flags);
    asio::awaitable<unsigned int> async_http2_send_data_payload(unsigned int sid, std::string_view payload, unsigned char end_flags);
    bool http2_try_push_content_raw(std::string_view frame, std::string *owned, unsigned int sid);
    bool http2_push_content_raw_sync(std::string &&frame, unsigned int sid);
    bool http2_push_content_raw_sync(const char *frame, unsigned int frame_size, unsigned int sid);
    asio::awaitable<bool> http2_push_content_raw(std::string frame, unsigned int sid);
    asio::awaitable<bool> http2_push_content_raw(const char *frame, unsigned int frame_size, unsigned int sid);

  public:
    unsigned char *_cache_data  = nullptr;
    std::atomic_uint time_limit = 0;

    bool isssl   = false;
    bool isgoway = false;
    // 连接关闭旗：发送协程每帧读它决定还要不要发。跨线程读写，所以是原子。
    std::atomic_bool isclose{false};
    bool iserror    = false;
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
    // 发送侧流控窗口：拆成「连接级 + 每流」两套记账，与接收侧
    // conn_recv_window_num / stream_recv_window 对称。两套的口径**不同**，别混用：
    //   window_update_num      连接级「累计授予额」，初值 RFC 9113 默认 65535，
    //                          仅由对端 stream id = 0 的 WINDOW_UPDATE 抬升，只增不减。
    //   has_send_update_num    连接级「累计已发额」，发送 DATA 时累加。
    //                          连接级剩余额 = window_update_num - has_send_update_num。
    //   stream_send_window     每流「剩余额度」unsigned int（与上面相反，发送 DATA 时直接扣减），
    //                          初值取对端 SETTINGS_INITIAL_WINDOW_SIZE；
    //                          SETTINGS IWS 下调时地板到 0，不跟踪负值（h2spec 6.9.2 #2 依赖负值
    //                          跟踪，但流控窗口本质是"还有多少额度可以发"，unsigned 更直接）。
    //                          Floor to 0 on SETTINGS IWS reduction — flow-control window is
    //                          "remaining quota", unsigned models that naturally; h2spec 6.9.2 #2
    //                          expects negative tracking but that's a spec edge we trade for simplicity.
    //   remote_initial_window_size  对端 SETTINGS_INITIAL_WINDOW_SIZE，新流首次
    //                          发送/首次收到该流 WINDOW_UPDATE 时据此懒初始化。
    // 头两个还借给 websocket 用一次：client_websocket_loop 从 URL 里取出两个数字段，
    // 先各自清零、逐位拼好，交给注册工厂当 (myid, groupid)，交完再清一次。
    // 借得干净的前提是两条路不重叠——websocket 只从 HTTP/1.1 升级进来，同一条连接不会
    // 既走 http2 流控又走这段解析，所以谁也不会读到对方留下的数；反过来说，
    // websocket 连接上这两个数不是窗口额度，是 URL 参数（收尾时被清回 0）。
    std::atomic<unsigned long long> window_update_num    = 65535;
    std::atomic<unsigned long long> has_send_update_num  = 0;
    std::atomic<unsigned int> remote_initial_window_size = 65535;
    // 对端 SETTINGS_MAX_FRAME_SIZE（RFC 9113 §6.5.2，合法区间 [16384, 16777215]，
    // 默认 16384）。发送侧拿它给单帧载荷封顶，超过就是对端必须拒绝的帧。
    std::atomic<unsigned int> remote_max_frame_size = 16384;
    std::map<unsigned int, unsigned int> stream_send_window;
    std::mutex stream_send_window_mutex;

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
    // 两个欠账字段只由本连接 strand 上的两个协程读写（解帧协程记账、发送环消费协程冲刷），
    // strand 把它们串行化，所以不需要原子量也不需要锁。
    unsigned int conn_grant_pending = 0;
    std::map<unsigned int, unsigned int> stream_grant_pending;
    // 旧发送闸门让路计数，现在恒 0，留着保收尾日志口径。
    // 这一支在 push() 之前就 return，所以它不体现在 overflow 里；不单独记的话
    // "环从没满过" 和 "环满了但每次都提前让路" 两种情况读起来一模一样。
    // 同样是 http2 专用，websocket / mqtt / 原生 socket 上恒为 0。
    std::atomic<unsigned long long> http2_ring_backpressure_count = 0;

    std::atomic_bool http2_need_wakeup = false;
    std::atomic_bool http2_fifo_write_active = false;
    //std::string http2_ring_queue_temp;

    std::mutex http2_sock_mutex;
    std::unique_ptr<http2_send_queue_cache> http2_ring_queue = nullptr;

    std::mutex wake_ring_consumer_mutex;
    std::list<asio::detail::awaitable_handler<asio::any_io_executor, size_t>> user_code_handler_call;
    std::shared_ptr<asio::steady_timer> http2_consumer_timer;

    // ---- HTTP/2 停等状态（都在 strand 上）----
    // 窗口停等登记：流级一流一个 timer；连接级按登记序 FIFO。
    std::multimap<unsigned int, asio::steady_timer *> http2_stream_window_waiters;
    std::vector<asio::steady_timer *> http2_conn_window_waiters;
    // 关闭旗：置位后等待者都退出，不再有新发送。
    std::atomic_bool http2_closing{false};
    // 本连接活跃发送协程数，收尾日志用，收尾后应归零。
    std::atomic<unsigned int> http2_active_senders{0};

    // HPACK 编码器动态表（响应头方向）。只在喂入锁临界区内编码时读写，
    // SETTINGS_HEADER_TABLE_SIZE 解析同在 strand 上更新上限；连接关闭即销毁。
    hpack_enc_table hpack_enc;
};
}// namespace http

#endif
