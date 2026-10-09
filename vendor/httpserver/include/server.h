#pragma once
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>

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
#include <set>

#include <cstdlib>
#include <fstream>
#include <algorithm>
// #include <sys/types.h>
// #include <sys/stat.h>

// #ifndef _MSC_VER
// #include <sys/fcntl.h>
// #include <unistd.h>
// #endif

// #ifndef _WIN32
// #include <sys/wait.h>
// #endif

// #ifdef WIN32
// #define stat _stat
// #endif

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

#include "zlib.h"
#include "terminal_color.h"
#include "http_socket.h"
#include "client_session.h"
#include "mqtt_frame.h"
#include "mqtt_session.h"
#include "mqtt_api.h"
#ifdef ENABLE_REDIS_CLIENT
#include "redis_subpub.h"
#endif
#ifdef ENABLE_WEBSOCKETS_CLIENT
#include "ws_subpub_reg.h"
#endif
#ifdef ENABLE_SOCKETS_CLIENT
#include "http_socket_client.h"
#include "sock_subpub_reg.h"
#endif
#ifdef ENABLE_MQTT_CLIENT
#include "mqtt_subpub_reg.h"
#endif
#include "http2_parse.h"
#include "threadpool.h"
#include "httppeer.h"
#include "http2_flow.h"
#include "http2_send_queue.h"
#include "orm_conn_pool.h"
// namespace this_coro = asio::this_coro;

namespace http
{

namespace fs = std::filesystem;
class httpserver
{
  public:
    httpserver() {}
    asio::awaitable<void> clientpeerfun(std::shared_ptr<client_session>, bool isssl);
    asio::awaitable<unsigned int> client_http1_loop(bool isssl, unsigned int readnum, std::shared_ptr<client_session>);
    asio::awaitable<unsigned int> client_http2_loop(unsigned int offset, unsigned int readnum, std::shared_ptr<client_session>);
    // HTTP/2 统一入口（明文直连 / TLS ALPN h2 / h2c 升级三条路径共用）：补读并校验 24 字节连接前言、
    // 必要时追加 h2c 首帧，再分配发送环 + 启动 ring_client_server + 回 SETTINGS，最后进入 client_http2_loop。
    // 返回 3 表示进入失败（连接已不可用）。
    //
    // Unified HTTP/2 entry shared by all three paths (cleartext direct / TLS ALPN h2 / h2c upgrade):
    // read and verify the 24-byte connection preface, append the h2c first frame if needed,
    // then allocate the send ring + start ring_client_server + reply SETTINGS,
    // and finally enter client_http2_loop. Returns 3 on enter failure (connection no longer usable).
    asio::awaitable<unsigned int> client_h2_enter(std::shared_ptr<client_session>, unsigned int &readnum, std::string_view h2c_first_request);
    // 记录首包阶段的错误日志（客户端 ip/port + 已读到的原始字节）。
    // Log an error snapshot at the first-packet stage (client ip/port + raw bytes already read).
    void log_client_first_error(std::shared_ptr<client_session>, unsigned int readnum);
    asio::awaitable<unsigned int> client_websocket_loop(std::shared_ptr<httppeer>, std::shared_ptr<websocket_t>, std::shared_ptr<client_session>);
    asio::awaitable<unsigned int> client_rpc_loop(unsigned int readnum, std::shared_ptr<client_session>);
    asio::awaitable<unsigned int> client_tcp_loop(unsigned int readnum, std::shared_ptr<client_session>);
    asio::awaitable<unsigned int> client_mqtt_loop(unsigned int readnum, std::shared_ptr<client_session>);
    asio::awaitable<void> handle_mqtt_subscribe(std::shared_ptr<http::mqtt_session>,
                                                std::shared_ptr<http::mqtt_api>,
                                                const http::mqtt_session::packet &,
                                                std::function<void(http::mqtt_reason)>);
    asio::awaitable<void> handle_mqtt_publish(std::shared_ptr<http::mqtt_session>,
                                              std::shared_ptr<http::mqtt_api>,
                                              const http::mqtt_session::packet &,
                                              std::function<void(http::mqtt_reason)>);

    // 同步业务钩子交给业务线程池（clientrunpool）跑的收口：钩子在池线程执行，续体回到本连接的协程，
    // 所以协议报文的先后次序照旧。抛出与"池没接单"都算业务没给出结论——bool 钩子按否决处理
    // （fail-closed），void 钩子只记日志，两种都不踢连接。
    asio::awaitable<bool> co_sync_hook_bool(const char *, std::function<bool()>, const std::shared_ptr<client_session> &);
    asio::awaitable<void> co_sync_hook_void(const char *, std::function<void()>, const std::shared_ptr<client_session> &);
    // socket 的 on_close 三条出口收敛成的唯一提交点；协程版业务直接走 async_on_close。
    asio::awaitable<void> co_socket_on_close(const std::shared_ptr<socket_api> &,
                                             const std::shared_ptr<client_session> &);
    // socket 入站（同步钩子那一档）唯一的投递点：片进这条连接自己的接收队列，队列满就停读歇拍
    // 等它退下去（一片不丢），然后投一条任务给业务线程池、立刻返回。
    // 返回 false 表示这条连接不能再收了：歇满上限（已置 isclose 并写过日志）或会话已读错，
    // 调用方直接走关闭路径。
    asio::awaitable<bool> co_socket_inbound_push(const std::shared_ptr<socket_api> &,
                                                 const std::shared_ptr<client_session> &,
                                                 socket_data_list_t &&);

    unsigned int make_h2c_header(std::shared_ptr<httppeer> peer, std::shared_ptr<client_session>, std::string &log_item);
    asio::awaitable<void>
        sslhandshake(std::shared_ptr<client_session>);

    // PHP 处理：找 php_root_document 里的 .php 文件 / 重写规则。
    // 返回 true → 已设 compress=10/linktype，调用方走 fastcgi；
    // 返回 false → 没找到，调用方发 404。不查路由表、不碰磁盘静态文件判断。
    bool check_php_dispatch(std::shared_ptr<httppeer>);

    // void http2pool(int threadid);
    asio::awaitable<void> http2_fastcgi(std::shared_ptr<httppeer>);
    asio::awaitable<void> http2loop(std::shared_ptr<httppeer>);

    asio::awaitable<void> http2_send_content(unsigned int stream_id, std::string &_send_data, const unsigned char *buffer, unsigned int begin_end, bool is_end = false);
    asio::awaitable<void> http2_send_content(unsigned int stream_id, std::string &_send_data, const std::string &_source_data, bool is_end = false);
    asio::awaitable<void> http2_send_content_append(unsigned int stream_id, std::string &_send_data, const unsigned char *buffer, unsigned int begin_end, bool is_end = false);
    asio::awaitable<void> http2_send_content_append(unsigned int stream_id, std::string &_send_data, const std::string &_source_data, bool is_end = false);

    asio::awaitable<void> http2_co_send_file(std::shared_ptr<httppeer> peer);
    asio::awaitable<void> http2_co_send_304(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t>);
    asio::awaitable<void> http2_co_send_compress(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t> send_file_obj);

    // Shared helpers to remove duplicated blocks across the http2 send paths.
    void http2_compress_output(std::shared_ptr<httppeer> &peer, std::shared_ptr<http2_send_data_t> &send_file_obj);
    void clear_peer_data(std::shared_ptr<httppeer> &peer);

    bool http2_send_file(std::shared_ptr<httppeer>);
    asio::awaitable<void> http2_send_file_range(std::shared_ptr<httppeer> peer);

    void add_error_lists(const std::string &);
    asio::awaitable<void> http1_fastcgi(std::shared_ptr<httppeer>);
    asio::awaitable<void> http1loop(std::shared_ptr<httppeer>, std::shared_ptr<client_session>);

    asio::awaitable<void> http2_send_sequence_header(std::shared_ptr<httppeer> peer, std::shared_ptr<http2_send_data_t>);

    bool http2_loop_send_sequence(std::shared_ptr<http2_send_data_t>);
    void http2_send_queue_loop(unsigned char index_id);

    // RFC 9113 §6.9 发送侧「查余额 + 扣减」原子预留：必须在 fread / push 之前调用。
    // 返回实际到手额度（0 = 本次调用无额度，调用方让路，不得推进任何进度）。
    // want 超过余额时按余额缩帧；连接级扣了、流级不够时把差额退还连接级。
    //
    // Atomic send-side 'check balance + deduct' reservation per RFC 9113 §6.9.
    // MUST be called before fread / push. Returns the quota actually granted (0 = nothing this round,
    // caller must yield; do not advance any progress). When want exceeds balance the frame shrinks
    // to fit the balance; when the connection-level bucket is charged but the stream bucket is short,
    // the difference is returned to the connection-level bucket.
    unsigned long long http2_reserve_send_window(client_session *session_obj, unsigned int stream_id, unsigned long long want);
    // 把没真正发出的额度退回两套窗口。
    // Return quota that didn't actually go out to both window buckets.
    void http2_refund_send_window(client_session *session_obj, unsigned int stream_id, unsigned long long back);

    // 挂起 / 回灌。park 只在发送线程里调用：返回 false 表示本连接已经冲刷过，
    // 调用方必须自己把对象回池，不能让它留在自己的链表里。
    //
    // Park / refill. park must be called from a send thread only. Returns false if this
    // connection has already been flushed — caller must recycle the object itself and
    // must NOT leave it in its own linked list.
    bool http2_park_send(std::shared_ptr<http2_send_data_t> &sp, unsigned char reason);
    // 事件边统一入口：重算本连接所有挂起对象的闸门，够格的移进 sent_data_list 并唤醒。
    // Unified event-edge entry: recompute gates for all parked objects on this connection,
    // move eligible ones into sent_data_list and notify the send thread.
    void requeue_parked(client_session &session_obj);
    // 摘出来的一批按闸门分流：够格入队并唤醒，其余留在参数里由调用方挂回。返回入队条数。
    // Split a drained batch by gate: eligible ones enqueued + notified; the rest stay in the
    // parameter for the caller to re-park. Returns the number enqueued.
    unsigned int dispatch_parked(std::list<std::shared_ptr<http2_send_data_t>> &in);
    // 兜底扫描：事件边漏了某次闸门抬升时，挂起的流最迟 CONST_HTTP2_BELT_SWEEP_SECONDS 秒后
    // 被重新评估，顺带给仍在表里的对象续 time_limit、给永久停读的流撤流。
    // 由发送线程在两处调用（带期限的等待之后、内层每轮之前），内部按秒级 CAS 自判归属：
    // 没到一拍时只是一次原子读，不遍历、不抢锁。
    //
    // Belt-sweep catch-all: when event edges miss a gate lift, parked streams are re-evaluated
    // at most CONST_HTTP2_BELT_SWEEP_SECONDS later. It also refreshes time_limit for objects still
    // in the table and RSTs permanently-stop-reading streams. Invoked by send threads in two places
    // (after the timed wait and before the inner loop each round). Internally a per-second CAS
    // decides ownership; when not due it's just one atomic read — no walks, no locks.
    void requeue_stuck_parked();

    asio::awaitable<void> ring_client_server(std::shared_ptr<client_session> peer_session);

    // MQTT 全局分发协程：扫 mqtt_sessions 清单，把有挂起帧的会话按 FIFO 回灌发送环。
    // 全部排空即退出并置 mqtt_sender_need_spawn，由 websocket_loop 每秒拍重新 spawn。
    //
    // MQTT global dispatch coroutine: scans the mqtt_sessions roster and FIFO-refills the
    // send ring of every session that has pending frames. Exits when everything is drained,
    // sets mqtt_sender_need_spawn, and gets respawned every second by websocket_loop.
    asio::awaitable<void> mqtt_send_loop();
#ifdef ENABLE_REDIS_CLIENT
    asio::awaitable<void> async_redis_subpub_loop(
        std::shared_ptr<pz::redis::redis_subpub_client> client);
#endif
#ifdef ENABLE_WEBSOCKETS_CLIENT
    asio::awaitable<void> async_ws_subpub_loop(
        std::shared_ptr<http::ws_subpub_client> client);
#endif
#ifdef ENABLE_SOCKETS_CLIENT
    asio::awaitable<void> async_sock_subpub_loop(
        std::shared_ptr<http::sock_subpub_client> client);
#endif
#ifdef ENABLE_MQTT_CLIENT
    asio::awaitable<void> async_mqtt_subpub_loop(
        std::shared_ptr<http::mqtt_subpub_client> client);
#endif

    void websocket_loop(int myid);
    asio::awaitable<void> clientpeerstop(std::shared_ptr<client_session> peer_session);
    asio::awaitable<void> orm_connect_clear(std::shared_ptr<orm::orm_conn_pool> peer_connect);

    void listeners();
    void listener();

    asio::awaitable<void> http1_send_status_content(std::shared_ptr<httppeer> peer, unsigned int status_code, const std::string &bodycontent);
    asio::awaitable<bool> http1_static_file_authority(std::shared_ptr<httppeer> peer);

    asio::awaitable<void> http2_send_status_content(std::shared_ptr<httppeer> peer, unsigned int status_code, const std::string &bodycontent);
    asio::awaitable<bool> http2_static_file_authority(std::shared_ptr<httppeer> peer);

    // 集中处理 HTTP/2 解析期错误：连接级→GOAWAY 断连（返回 true，调用方 break）；
    // 流级→RST_STREAM 重置该流、cleanup、清错，并标记 need_wakeup_send_threads 由主循环末尾
    // 统一唤醒发送协程（返回 false，连接继续复用）。解析器只需 set_conn_error/set_stream_error 置位，
    // 错误分流与收发细节全部收敛于此，业务帧处理代码无需关心 GOAWAY/RST 差异。
    asio::awaitable<bool> http2_handle_parse_error(http2parse &h2, std::shared_ptr<client_session> sess);

    // CORS 预检自动回复：按站点白名单判定 Allow-Origin，补上 Allow-Methods 等预检专用头，
    // 并发送 200 空响应（解析期 cors_origin_process() 设的 ACAO/Vary 会先被清掉再重设）。
    //
    // Automatic CORS preflight reply: decide Allow-Origin from the site whitelist, emit
    // Allow-Methods and other preflight-only headers, then reply 200 with an empty body.
    // Note: ACAO/Vary set during parsing by cors_origin_process() are cleared first and then re-set.
    asio::awaitable<void> send_cors_domain(std::shared_ptr<httppeer> peer);

    // 旧同步派单通路，无调用点（流程说明见 server.cpp 同位置注释）；活的通路是下一行 fastcgi。
    // asio::awaitable<size_t> co_user_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h = {});
    asio::awaitable<size_t> co_user_fastcgi_task(std::shared_ptr<httppeer> peer, asio::use_awaitable_t<> h = {});
    asio::awaitable<size_t> co_client_session_task(std::shared_ptr<client_session> peer, asio::use_awaitable_t<> h = {});

    void add_runsocketthread();
    int checkhttp2(std::shared_ptr<client_session> peer_session);
    asio::awaitable<void> http1_send_bad_request(unsigned int, std::shared_ptr<client_session>);
    asio::awaitable<void> http1_send_method_not_allowed(std::shared_ptr<client_session>);
    asio::awaitable<void> http1_send_bad_server(std::shared_ptr<httppeer>, std::shared_ptr<client_session>);

    asio::awaitable<void> http1_send_file_header(std::shared_ptr<httppeer> peer,
                                                 std::shared_ptr<client_session> peer_session,
                                                 std::shared_ptr<http2_send_data_t> sq_ob);
    asio::awaitable<void> http1_co_send_304(std::shared_ptr<httppeer> peer,
                                            std::shared_ptr<client_session> peer_session,
                                            std::shared_ptr<http2_send_data_t> sq_ob);

    asio::awaitable<void> http1_send_file(std::shared_ptr<httppeer> peer,
                                          std::shared_ptr<client_session> peer_session);

    asio::awaitable<void> http1_send_file_range(std::shared_ptr<httppeer> peer,
                                                std::shared_ptr<client_session> peer_session);

    void set_thread_priority(std::thread &thread, int priority);
    void run(const std::string &);

    void add_nullptrlog(const std::string &logstrb);
    void httpwatch();

    // httpwatch sub-methods
    void httpwatch_init_paths(std::string &currentpath, std::string &error_path, std::string &traffic_switch_file, std::string &restart_file, std::string &restart_ssl_file, std::string &orm_log_file);
    void httpwatch_parse_reboot_cron(unsigned char &cron_type, unsigned char &cron_day, unsigned char &cron_hour);
    void httpwatch_parse_clean_cron(unsigned int &clean_cron_min, unsigned int &clean_cron_time_ago);
    // 解析 [default] temp_clean_time（temp_path 临时文件存活秒数）。
    // Parse [default] temp_clean_time — TTL in seconds for temp files under temp_path.
    void httpwatch_parse_temp_clean(unsigned int &temp_live_time);
    void httpwatch_parse_links_restart(unsigned int &restart_process_num,
                                       int &restart_process_time_start,
                                       int &restart_process_time_end);
    void httpwatch_adjust_thread_pool(unsigned int &updatetimetemp);
    void httpwatch_memory_monitor(unsigned int mysqlpool_time);
    void httpwatch_flush_access_log(const std::string &access_path);
    void httpwatch_flush_error_log(const std::string &error_path);
    void httpwatch_mysql_pool_maintenance(unsigned int &mysqlpool_time, const std::tm *now, unsigned int old_total_count);
    void httpwatch_check_cron_reboot(unsigned char cron_type, unsigned char cron_day, unsigned char cron_hour, const std::tm *now);
    void httpwatch_clear_timeout_sessions(unsigned int clean_cron_min, unsigned int clean_cron_time_ago);
    // 回收 temp_path 下框架自己的请求落盘临时文件（pzraw_/pzup_）与遗留孤儿文件。
    // Reap framework-owned temp files under temp_path (pzraw_/pzup_) and lingering orphan files.
    void httpwatch_clear_temp_files(unsigned int temp_live_time);
    void httpwatch_check_deadlock(unsigned char &plan_http1_exit, unsigned char &plan_http2_exit, unsigned int &old_ten_total_count, unsigned int old_total_count);
    void httpwatch_check_restart_threshold(unsigned int restart_process_num,
                                           int restart_process_time_start,
                                           int restart_process_time_end,
                                           const std::tm *now,
                                           unsigned int old_total_count);

    void acme_task();
    void acme_update();
    void save_traffic_arrays();
    void stop();
    asio::io_context &get_ctx();
    ~httpserver()
    {
        std::printf("~httpserver\n");
        isstop = true;
        // 必须先唤醒再 stop/join：发送线程阻塞在 send_data_condition 上时不会因为
        // io_context.stop() 而退出。旧实现只置位不通知，两个发送线程会成为 joinable
        // 的残留 std::thread，vector 析构时直接 std::terminate。
        //
        // Must wake first, then stop/join: send threads blocked on send_data_condition do NOT
        // exit because io_context.stop() was called. The old code only set a flag without
        // notifying — the two send threads would remain joinable leftover std::thread objects,
        // and vector destructor then calls std::terminate.
        send_data_condition.notify_all();
        websocketcondition.notify_all();
        io_context.stop();

        for (unsigned int i = 0; i < runthreads.size(); ++i)
        {
            if (runthreads[i].joinable())
            {
                runthreads[i].join();
            }
        }
        for (unsigned int i = 0; i < http2_send_data_threads.size(); ++i)
        {
            if (http2_send_data_threads[i].joinable())
            {
                http2_send_data_threads[i].join();
            }
        }
    }

  public:
    // httpheader begin
    unsigned char runhands_num = 4;
    asio::io_context io_context{0};
    std::vector<std::thread> runthreads;
    std::vector<std::thread> websocketthreads;
    std::vector<std::thread> http2_send_data_threads;
    std::list<std::weak_ptr<websockets_api>> websockettasks;
    std::list<std::weak_ptr<socket_api>> sockettasks;
    unsigned int socket_broadcast(unsigned int groupid, std::string_view payload);
    std::list<std::weak_ptr<mqtt_api>> mqtttasks;
#ifdef ENABLE_REDIS_CLIENT
    std::list<std::weak_ptr<pz::redis::redis_subpub_client>> redis_subpub_tasks;
#endif
#ifdef ENABLE_WEBSOCKETS_CLIENT
    std::list<std::weak_ptr<http::ws_subpub_client>> ws_subpub_tasks;
#endif
#ifdef ENABLE_SOCKETS_CLIENT
    std::list<std::weak_ptr<http::sock_subpub_client>> sockets_clients;
#endif
#ifdef ENABLE_MQTT_CLIENT
    std::list<std::weak_ptr<http::mqtt_subpub_client>> mqtt_clients;
#endif
    // MQTT 会话注册清单（client_mqtt_loop 建会话后登记，expired 由分发协程剪除）；
    // size() 兼作在线 MQTT 连接数的诊断计数。
    //
    // MQTT session registry (client_mqtt_loop registers on session creation; the dispatch
    // coroutine prunes expired ones). size() doubles as a diagnostic read of online MQTT connections.
    std::mutex mqtt_sessions_mutex;
    std::vector<std::weak_ptr<mqtt_session>> mqtt_sessions;
    // true ⟺ 全局分发协程不在跑；websocket_loop 每秒拍 exchange(false) 认领并 spawn。
    // true ⟺ the global dispatch coroutine is not running; websocket_loop exchanges(false)
    // every second to claim and respawn it.
    std::atomic_bool mqtt_sender_need_spawn = true;
    std::list<std::pair<std::size_t, std::shared_ptr<httppeer>>> clientlooptasks;
    // 保护 clientlooptasks：tick 线程（websocket_loop）遍历/erase 与业务线程
    // （controller/src/serverwatch.cpp 的 frametasks_timeloop 经路由登记）push_back 并发访问，
    // 无锁会令 std::list 迭代器失效（UB，可崩）。与 mqtt_sessions_mutex 同理。
    std::mutex clientlooptasks_mutex;
    // 间隔任务被跳过的拍数（同一条 peer 还在跑，这一拍不重复投）。跳拍是有意的，
    // 但它对业务不可见，没计数的话就只剩"某条任务好像慢了几拍"这一种说法。
    std::atomic_uint clientloop_skipped{0};
    // 每秒拍扫过间隔任务表多少次。跳拍计数只在"抢旗失败"那一支涨，扫不到表它就是 0，
    // 于是"表里没有任务"和"这一拍根本没走到这儿"两种情况从这两个计数上看长得一样。
    std::atomic_uint clientloop_ticks{0};

    std::string traffic_arrays;

    // 这几个标志由关闭/限流路径写、被多个分发线程与协程读，必须是原子的。
    // 裸 bool 在这里是 data race（UB），编译器有权把 while (isstop == false) 优化成死循环。
    // 注意：原子化修的是「形式上的竞态」，不解决「条件变量谓词漏判退出条件」——见
    // http2_send_queue_loop 的 wait 谓词与 ~httpserver 的 notify_all()。
    //
    // These flags are written by the close/rate-limit paths and read by multiple dispatch
    // threads and coroutines — they must be atomic. A plain bool here is a data race (UB);
    // the compiler is free to hoist 'while (isstop == false)' into an infinite loop.
    // Note: atomicity fixes the formal race, not the predicate-miss problem — see
    // http2_send_queue_loop's wait predicate and ~httpserver's notify_all().
    std::atomic_bool isstop             = false;
    std::atomic_bool istraffic          = false;
    std::atomic_bool hard_kill_old_link = false;
    std::atomic_bool rate_limit_status  = false;
    bool server_ip6_listen              = false;

    // 各 listener 线程的 acceptor，建好监听后登记，stop() 负责唤醒挂起的 accept() 并关掉它。
    // 存 shared_ptr 而不是裸 fd：fd 号在 ::close 之后会被别的线程立刻复用，
    // 再由 stop() 关一次就打到了新 socket；而且只有拿到对象才谈得上下面的 cancel()。
    //
    // Acceptor per listener thread, registered after listen setup. stop() wakes blocked accept()
    // and closes them. Storing shared_ptr, not raw fd: after ::close the fd number is reused
    // immediately by another thread — a second close from stop() would hit the new socket.
    // Also, cancel() can only be called on the object itself.
    std::mutex acceptors_mutex;
    std::vector<std::shared_ptr<asio::ip::tcp::acceptor>> acceptors;

    std::atomic_uint total_count        = 0;
    std::atomic_uint live_link_count    = 0;
    std::atomic_uint http2_minute_count = 0;

    std::atomic_uint total_http2_count = 0;
    std::atomic_uint total_http1_count = 0;

    std::atomic_uint rate_limit_new_wait_num    = 300;
    std::atomic_uint rate_limit_accept_wait_num = 600;
    std::atomic_uint rate_limit_accept_time     = 500;

    std::mutex socket_session_lists_mutex;
    std::list<std::weak_ptr<client_session>> socket_session_lists;
    //std::list<std::shared_ptr<client_session>> socket_session_wait_clear;

    std::condition_variable send_data_condition;

    std::list<std::shared_ptr<http2_send_data_t>> sent_data_list;

    std::mutex send_data_mutex;
    std::mutex wait_clear_mutex;
    std::list<std::string> access_loglist;
    std::list<std::string> error_loglist;
    std::mutex log_mutex;

    ThreadPool clientrunpool{std::thread::hardware_concurrency() * 2 + 2};

    // std::mutex http2_task_mutex;
    // std::list<struct http2sendblock_t> http2send_tasks;

    std::mutex websocket_task_mutex;
    std::mutex socket_task_mutex;
    std::mutex mqtt_task_mutex;
#ifdef ENABLE_REDIS_CLIENT
    std::mutex redis_subpub_task_mutex;
#endif
#ifdef ENABLE_WEBSOCKETS_CLIENT
    std::mutex ws_subpub_task_mutex;
#endif
#ifdef ENABLE_SOCKETS_CLIENT
    std::mutex sockets_clients_mutex;
#endif
#ifdef ENABLE_MQTT_CLIENT
    std::mutex mqtt_clients_mutex;
#endif
    std::condition_variable websocketcondition;

    const unsigned char magicstr[24] = {0x50, 0x52, 0x49, 0x20, 0x2A, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2F, 0x32, 0x2E, 0x30, 0x0D, 0x0A, 0x0D, 0x0A, 0x53, 0x4D, 0x0D, 0x0A, 0x0D, 0x0A};
};
httpserver &get_server_app();
// 这个声明一直没有对应定义（定义在 server.cpp 里是 (keyname, peer) 两参数的，已随旧登记口注释掉），
// 也无人调用；间隔任务的实际登记口是路由 frametasks_timeloop（注册点在 controller/src/serverwatch.cpp）。
// 留在这儿只会引人误用。
// void add_server_timetask(std::shared_ptr<httppeer>);
}// namespace http
#endif
