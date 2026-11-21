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
#include <thread>
#include <memory>
#include <string_view>
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
    // GOAWAY 入控制环（满则 20ms 一拍重试，2 秒预算），再有限等控制环排空后返回。
    // 调用方（client_http2_loop 的各 break 路径、http2loop 异常路径）随后会 cancel()，
    // 不等排空就会把环里的 GOAWAY 与在途响应一起杀掉。
    asio::awaitable<void> async_send_goway(unsigned int last_stream_id = 0, unsigned int error_code = 0);
    // 关闭前冲刷：有界等待控制环排空。绝不直接写 socket——写者只有消费者协程，
    // 这里只 co_await 定时器，每次让出 strand 都让消费者的写完成推进。
    // pop 在 async_write 完成之后（环空 ⇒ 队尾帧已写出）。上限 2 秒：
    // 客户端停读时不无限拖住 teardown。返回 false = 超时/连接已坏仍未排空。
    asio::awaitable<bool> flush_send_ring();

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
    // h2 帧一律入环（控制环 / 内容环），由消费者协程在 strand 上串行写出，
    // 生产者只 push + 停等，不碰 socket，也没有跨 I/O 持有的写锁。
    // 等一次窗口：true=额度到了（调用方马上 reserve，中间别 co_await），
    // false=关闭/流已死/预算耗尽，调用方负责撤流。被叫醒后额度没到就用剩余预算重挂，
    // 小增量唤醒不续命。dead 传 peer->isclose。
    asio::awaitable<bool> http2_wait_window_episode(unsigned int sid, const std::atomic_bool &dead);
    // 解析侧收到 WU / IWS 上调 / 流关闭时调用，叫醒对应的停等者。
    void http2_cancel_conn_waiters();
    void http2_cancel_stream_waiter(unsigned int sid);
    void http2_cancel_all_stream_waiters();
    // 关闭冲刷：置 closing，叫醒所有等待者。
    void http2_close_waiters();
    // 同步停等者的唤醒边（业务池线程不能挂 strand timer）：上面四个函数收尾时调用，
    // 锁内递增世代再 notify_all。停等者先快照世代再 reserve，reserve 失败后按
    // 「世代已变」谓词挂 CV——reserve 与挂起之间到达的唤醒不会丢。
    std::mutex http2_sync_window_mutex;
    std::condition_variable http2_sync_window_cv;
    unsigned long long http2_sync_window_gen = 0;
    void http2_wake_sync_window_waiters();
    // RST 入控制环，满则 20ms 一拍重试（2 秒预算）；false = 预算耗尽或连接已关。
    asio::awaitable<bool> http2_send_rst(unsigned int s_stream_id, unsigned int stream_error_code);
    // RST 入环版：解析循环里用，不在读路径上等写锁。false = 环满，连接已过载。
    bool push_rst_stream(unsigned int sid, unsigned int code);

    // ---- 喂入锁：本连接所有 h2 生产者（内容批 / HEADERS / 控制帧）的唯一竞争点 ----
    // 谁拿到谁入环：内容批持锁内查 24 闸、一次推 ≤6 帧再放；HEADERS 持锁内编码+入环
    // （动态表变更序 = 入环序 = 上线序）；控制帧持锁直推、不查闸。锁内只做 memcpy/编码，
    // 绝不跨 co_await / I/O；环满或闸满都先放锁再等 20ms。环自带 push 锁嵌套在内
    // （消费者/pop 与 post_write 仍只用环锁），全局锁序恒为 喂入锁 → 环 push 锁。
    std::mutex http2_feed_mutex;
    // 控制帧入控制环（持喂入锁，不查闸）：仅 push，唤醒/计数由调用方自理。false = 环满。
    bool push_control_frame(const unsigned char *fr, unsigned int n);

    // ---- 内容环：流式响应（HEADERS/DATA）的发送队列 ----
    // 与控制环同款同池，但只装内容帧；消费者每帧先看控制环再看内容环，
    // 控制帧（SETTINGS/PING/WU）永远优先于排队中的内容；RST 例外——内容环
    // 还有同流帧时先排空它们再放 RST（HEADERS 入环即必须上线）。
    // 只有 h2 会话分配它；websocket/mqtt/原生 socket 恒为 nullptr，消费者判空跳过。
    // 流式生产者（send_writer/async_send_writer 的 httpv==2 分支）把 DATA 按
    // min(对端 MAX_FRAME_SIZE, CONST_HTTP2_SEND_FRAME_BUF) 分片、逐片预留发送窗口后入环；
    // 内联/文件生产者每轮至多 CONST_HTTP2_SEND_BATCH_FRAMES 帧、积压到
    // CONST_HTTP2_CONTENT_RING_FEED_GATE 槽就地等；环满/窗口见底原地等
    // （async 挂 timer 让出 strand；sync 窗口停等挂 CV、环满自适应 sleep），预算耗尽撤流。
    std::unique_ptr<http2_send_queue_cache> http2_content_queue = nullptr;
    // 死流标记：cleanup_stream（对端 RST/流级错误）与本地撤流写入，消费者出队时复查，
    // 命中即丢帧（DATA 退还已预留的窗口额度）。内容环排空时清掉没有在途生产者的标记，
    // 内存有界。跨线程读写（业务池 sync 生产者 / strand 解析与消费者），自带锁。
    std::mutex http2_dead_streams_mutex;
    std::set<unsigned int> http2_dead_streams;
    // 在途流式生产者登记表（与死流标记同锁）：生产者进 http2_send_content_frame /
    // http2_send_data_payload 注册、退出注销。消费者清死流标记时跳过仍有在途生产者的流——否则停等中的生产者醒来
    // 可能见不到标记，往已重置的流上重新预留窗口并推帧。
    std::set<unsigned int> http2_content_producers;
    void http2_mark_stream_dead(unsigned int sid);
    bool http2_stream_is_dead(unsigned int sid);
    // 流式发送失败的统一收尾（只在 strand 上调）：退额度、标死流、清窗口条目，
    // rst=true 再投 RST(CANCEL)（对端已 RST/连接将关时传 false，不对 closed 流发 RST）。
    // sync 生产者经 asio::post(strand_) 投递到这里，不跨线程碰 strand 专属的等待者容器。
    void http2_content_stream_failed(unsigned int sid, unsigned long long refund_bytes, bool rst);
    // 流式帧入内容环（满则原地等，预算 CONST_HTTP2_WINDOW_STALL_TIMEOUT 秒）。
    unsigned int http2_send_content_frame(std::string_view frame);
    asio::awaitable<unsigned int> async_http2_send_content_frame(std::string_view frame);
    // DATA payload 直入：预留窗口 + 按 min(对端 MAX_FRAME_SIZE, CONST_HTTP2_SEND_FRAME_BUF)
    // 拆片 + 组帧一步完成，payload 只 memcpy 一次（调用方不必先组整帧再被拆散重组）。
    // end_flags 是最后一帧的 flags（0x01=END_STREAM / 0x00），非末帧自动剥 END_STREAM；
    // 空 payload 发零长 DATA（不占窗口额度）。返回入环字节数（含帧头），0 = 一字节未入。
    // 停等/环满/失败收尾语义与 http2_send_content_frame 的 DATA 分支完全一致。
    unsigned int http2_send_data_payload(unsigned int sid, std::string_view payload, unsigned char end_flags);
    asio::awaitable<unsigned int> async_http2_send_data_payload(unsigned int sid, std::string_view payload, unsigned char end_flags);
    // 裸内容帧入环（窗口额度由调用方预留/退还）。
    // sync 版：一次尝试立刻返回，不 sleep、不循环；环满/流已死/连接已关返回 false，
    // 重试策略和预算由调用方决定。按右值引用收帧：成功才移动进槽，拒收时源缓冲原样
    // 保留，调用方可复用同一帧重试。
    bool http2_push_content_raw_sync(std::string &&frame, unsigned int sid);
    // async 版：按值收帧，环满原地挂 timer 自适应重试，预算 CONST_HTTP2_WINDOW_STALL_TIMEOUT
    // 秒；false = 预算耗尽或连接/流已死。
    asio::awaitable<bool> http2_push_content_raw(std::string frame, unsigned int sid);
    // 内联/文件 DATA 批入环：持喂入锁一次推完整批（≤CONST_HTTP2_SEND_BATCH_FRAMES 帧），
    // 锁内查 CONST_HTTP2_CONTENT_RING_FEED_GATE 闸，满则放锁等 20ms 再竞争。闸+批 ≤ 可用槽，
    // 持锁期间别的生产者进不来，本批必然装得下。reserved_bytes 是本批预留的窗口额度；
    // 未入环的差额由本函数内部退款，已入环后被消费者丢弃的死流 DATA 仍由消费者退款。
    // 返回实际入环字节数；返回 0 表示预算耗尽或流/连接已死（一帧未推，frames 原样保留）。
    asio::awaitable<unsigned long long> http2_push_content_batch(std::vector<std::string> &frames,
                                                                unsigned int sid,
                                                                unsigned long long reserved_bytes);

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
    //   - 内容环满时流式帧 push 被拒（http2_send_content_frame 原地重试期间的每一次拒绝）。
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
    //std::string http2_ring_queue_temp;

    std::mutex http2_sock_mutex;
    std::unique_ptr<http2_send_queue_cache> http2_ring_queue = nullptr;

    std::mutex wake_ring_consumer_mutex;
    std::list<asio::detail::awaitable_handler<asio::any_io_executor, size_t>> user_code_handler_call;

    // ---- h2 停等状态（都在 strand 上）----
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
