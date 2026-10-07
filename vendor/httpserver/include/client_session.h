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
    // 数据进本连接发送环，由 ring_client_server 消费者协程串行写出。
    // 返回 false = 数据被丢弃：连接已关闭 / 未分配发送环 / 环满（16 槽）/
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

    // 收尾帧返回「是否成功入环」：16 槽环满时 push 返回 false，而 END_STREAM 一旦
    // 丢失这条流就永远不结束，调用方必须重投。
    bool http2_send_enddata(unsigned int s_stream_id);
    bool send_zero_data(unsigned int stream_id);
    asio::awaitable<void> co_send_setting();

    bool send_switch101();
    asio::awaitable<bool> co_send_switch101();

    void send_ping();
    asio::awaitable<void> http2_send_ping();
    void send_recv_setting();
    // WINDOW_UPDATE 必须区分层级：连接级只认 stream id = 0，流级只认具体流。
    // 不能一次推入「连接级 + 流级」两个帧且用同一个增量：那等于每来一个 POST 就把
    // 对端连接窗口顶高 16MB，约 129 次后超过 2^31-1，对端必须按 FLOW_CONTROL_ERROR 断连。
    void send_window_update_conn(unsigned int up_num);
    void send_window_update_stream(unsigned int stmid, unsigned int up_num);

    void http2_send_rst_stream(unsigned int s_stream_id, unsigned int stream_error_code);
    void stop();
    void cancel();
    void half_stop();
    asio::awaitable<std::string> async_stop();
    asio::awaitable<void> async_send_goway();

    asio::awaitable<unsigned int> async_send_writer(const std::string &msg);
    asio::awaitable<unsigned int> async_send_writer(std::string_view msg);
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

    void waituphttp2();

    // 关闭路径冲刷挂起表：把本连接停在 parked_list 里的发送对象全部回池。
    // 挂起表 -> sq_obj -> peer -> 本 session 是条引用回路，冲刷点没走到就会一直
    // 拽着 httppeer 和它打开的文件句柄，连接永远析构不掉。
    void flush_parked_send();

  public:
    unsigned char *_cache_data  = nullptr;
    std::atomic_uint time_limit = 0;

    bool isssl      = false;
    bool isgoway    = false;
    bool isclose    = false;
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
    //   stream_send_window     每流「剩余额度」（与上面相反，发送 DATA 时直接扣减），
    //                          初值取对端 SETTINGS_INITIAL_WINDOW_SIZE。
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
    // 现在往这一个数里记的有三处，字段名的 http2 是历史遗留，语义已经是
    // "本连接的发送环拒过多少片"：
    //   - http2_send_queue_loop 里两处 http2 帧直接 push 被拒；
    //   - post_write 的三道闸（16 槽满 / 积压过 MQTT_SEND_RING_BYTE_LIMIT / push 失败），
    //     也就是 websocket_api::send()、mqtt 的出站、socket_api::send() 三条路共用这一个数。
    //     post_write 最前面那条"连接已关或没环"不算背压，不进这个数。
    // 这个数目前在哪里打印：http2 连接收尾时的 "http2 ring stats overflow"；原生 socket 收尾时的
    // "tcp ring close ... refuse="。websocket / mqtt 的连接目前没有落这一行的地方。
    std::atomic<unsigned long long> http2_ring_overflow_count = 0;
    // 无法重投的控制帧（RST / GOAWAY / WINDOW_UPDATE）被环满丢掉的次数：
    // http2_send_rst_stream / async_send_goway / send_window_update_conn /
    // send_window_update_stream 里 push 被拒各加一次。
    // DATA 与收尾帧走重投，不计在这里；两者分开才看得出「背压」与「真丢帧」的差别。
    // 只有 http2 会话会走到那几个函数，websocket / mqtt / 原生 socket 上它恒为 0。
    std::atomic<unsigned long long> http2_ring_queue_drop_count = 0;
    // 发送侧看到环里积压超过阈值（16 槽里的 10 槽）而主动让过这一轮的次数：
    // http2_send_gate_open 判闸时加一次，http2_send_queue_loop 里让路时再加。
    // 这一支在 push() 之前就 return，所以它不体现在 overflow 里；不单独记的话
    // "环从没满过" 和 "环满了但每次都提前让路" 两种情况读起来一模一样。
    // 同样是 http2 专用，websocket / mqtt / 原生 socket 上恒为 0。
    std::atomic<unsigned long long> http2_ring_backpressure_count = 0;

    std::mutex http2_loop_send_mutex;
    std::atomic_bool http2_need_wakeup = false;
    // 关闭冲刷已发生：此后不再接收新的挂起（发送线程直接把对象回池）。
    // 没有这个闸门，冲刷之后挂进来的对象要等兜底扫描才回得来。
    std::atomic_bool send_park_closed = false;
    //std::string http2_ring_queue_temp;

    std::mutex http2_sock_mutex;
    std::unique_ptr<http2_send_queue_cache> http2_ring_queue = nullptr;

    std::mutex waituphttp2_mutex;
    std::list<asio::detail::awaitable_handler<asio::any_io_executor, size_t>> user_code_handler_call;
};
}// namespace http

#endif
