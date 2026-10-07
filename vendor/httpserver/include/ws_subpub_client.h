#ifndef PZ_WS_SUBPUB_CLIENT_H
#define PZ_WS_SUBPUB_CLIENT_H
/*
 * 框架级 WebSocket 长连接客户端基类
 *
 * 业务侧继承本类，实现 section_name() / on_open / on_message / on_close / run_loop。
 * 启动时 server.cpp 遍历注册表 co_spawn async_ws_subpub_loop，负责：
 *   async_connect → handshake → pump async_text_read → 断连重连（3→6→9→12 线性退避）
 * websocket_loop 每秒拍扫 ws_subpub_tasks，按 durtime 分频调 run_loop/async_run_loop。
 *
 * 线程模型完全对称 redis_subpub_client：
 *   is_coroutine_ = false  →  pump 回调丢 clientrunpool
 *   is_coroutine_ = true   →  pump 回调 co_spawn 到 io_context 新协程
 */
#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "http_websocket_client.h"

#ifdef ENABLE_WEBSOCKETS_CLIENT

namespace http
{

class ws_subpub_client : public std::enable_shared_from_this<ws_subpub_client>
{
  public:
    ws_subpub_client()          = default;
    virtual ~ws_subpub_client() = default;

    // ===== 业务实现 =====
    virtual std::string section_name() const = 0;// 对应 conf/websockets.conf 的段名

    // 业务侧构造时调用，注入自定义握手 header（可多次调）
    // 非虚：业务构造时调用，直接灌 user_headers_ vector
    void add_header(std::string_view name, std::string_view value)
    {
        user_headers_.emplace_back(std::string(name), std::string(value));
    }
    void add_headers(std::string_view raw)
    {
        // 批量: "K1:V1\r\nK2:V2" 或 "K1:V1;K2:V2"
        std::string s(raw);
        for (auto &c : s)
            if (c == '\r' || c == ';')
                c = '\n';
        std::size_t start = 0;
        while (start <= s.size())
        {
            auto pos         = s.find('\n', start);
            std::string line = (pos == std::string::npos) ? s.substr(start) : s.substr(start, pos - start);
            while (!line.empty() && (line.back() == '\0' || line.back() == ' '))
                line.pop_back();
            auto colon = line.find(':');
            if (colon != std::string::npos && colon > 0)
                user_headers_.emplace_back(line.substr(0, colon), line.substr(colon + 1));
            if (pos == std::string::npos)
                break;
            start = pos + 1;
        }
    }

    // 虚方法：业务可 override 返回握手 header 字符串，框架 loop 自动调
    // 默认返回空。业务 override 示例:
    //   std::string add_headers() const override { return "Auth: Bearer xxx;X-Custom: val"; }
    virtual std::string add_headers() const { return {}; }

    // 同步钩子（跑业务线程池）
    virtual void on_open() {}
    virtual void on_close() {}
    virtual void on_message(const std::string &payload, bool is_binary) {}
    virtual void run_loop() {}

    // 协程钩子（跑 io_context 新协程）
    virtual asio::awaitable<void> async_on_open() { co_return; }
    virtual asio::awaitable<void> async_on_close() { co_return; }
    virtual asio::awaitable<void> async_on_message(const std::string &payload, bool is_binary) { co_return; }
    virtual asio::awaitable<void> async_run_loop() { co_return; }

    // 业务主动发消息（线程安全，进 websocket_client 的出站队列）
    // enqueue_frame 返回 true 表示本次调用接管队列消费，必须有人 pump；
    // 这里是同步入口，不能 co_await，所以在 strand 上 co_spawn 一个 pump 协程。
    bool send_text(std::string_view msg)
    {
        auto ws = conn();
        if (!ws)
            return false;
        auto frame = ws->serialize_frame(ws_opcode::text, msg);
        if (!ws->enqueue_frame(std::move(frame)))
            return false;
        take_over_send_pump(ws);
        return true;
    }
    bool send_binary(std::string_view msg)
    {
        auto ws = conn();
        if (!ws)
            return false;
        auto frame = ws->serialize_frame(ws_opcode::binary, msg);
        if (!ws->enqueue_frame(std::move(frame)))
            return false;
        take_over_send_pump(ws);
        return true;
    }

    // 活跃连接的快照。常驻循环在 io_context 线程上换/清这条连接，业务在 tick 线程或
    // 业务线程池上读它 —— shared_ptr 本身不是原子的，裸读裸写就是数据竞争。
    // 拿副本用是安全的：即使这一轮连接随后就被换掉，副本也让对象活到用完。
    std::shared_ptr<http::websocket_client> conn()
    {
        std::lock_guard<std::mutex> lk(conn_mtx_);
        return ws_;
    }
    // 框架内部：只有常驻循环该调（业务别改）
    void set_conn(std::shared_ptr<http::websocket_client> ws)
    {
        std::lock_guard<std::mutex> lk(conn_mtx_);
        ws_ = std::move(ws);
    }

    // 业务主动收摊（任意线程）：置旗 + 取消这条连接上的在途读。
    // 只写 isclose 收不了摊：常驻 pump 正 co_await 在 async_text_read() 上，读不到新字节
    // 也等不到对端断开时，它根本不会回来查这一面旗，fd、协程帧和连接对象一起长驻。
    // close_connect() 只做 cancel + close，不 reset 那个 shared_ptr 槽位，所以从 tick 线程
    // 直接调是可以的——看护线程关超时连接走的就是同一个入口。
    void stop()
    {
        isclose = true;
        // 副本出了锁再关：不握着锁跑别人的代码
        if (auto ws = conn())
            ws->close_connect();
    }

    // ===== 框架字段（业务构造时设置）=====
    bool is_coroutine_   = false; // 钩子走协程版
    bool is_loop_co_     = false; // run_loop 走协程版
    unsigned int durtime = 8;     // tick 分频（1 拍 = 1s）
    int loop_num         = 999999;// tick 剩余次数（0 = 擦除）

    // ===== 框架内部字段（只读，业务别改）=====
    asio::io_context *io_ctx = nullptr;                            // 框架注入
    std::vector<std::pair<std::string, std::string>> user_headers_;// 业务注入（section header 先，业务后）
    std::atomic<bool> isclose = false;

  private:
    std::shared_ptr<http::websocket_client> ws_;// 活跃连接，只能经 conn()/set_conn() 碰
    std::mutex conn_mtx_;

    // 接管出站队列的消费：pump 跑在这条连接自己的 strand 上。
    // 同一时刻只有一个 pump 在跑（sending_ 由 send_queue_mutex_ 保护），
    // 所以和 async_send_frame 自己的 pump 撞上了也只是多一个空转就返回的协程。
    void take_over_send_pump(const std::shared_ptr<http::websocket_client> &ws)
    {
        asio::co_spawn(ws->strand_, [ws]() -> asio::awaitable<void>
                       {
                try
                {
                    co_await ws->pump_send_queue();
                }
                catch (...)
                {
                }
                co_return; },
                       asio::detached);
    }
};

}// namespace http

#endif// ENABLE_WEBSOCKETS_CLIENT
#endif
