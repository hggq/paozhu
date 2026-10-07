#ifndef PZ_SOCK_SUBPUB_CLIENT_H
#define PZ_SOCK_SUBPUB_CLIENT_H
/*
 * 框架级 Socket 长连接客户端基类
 *
 * 业务侧继承本类，实现 section_name() / on_open / on_message / on_close / run_loop。
 * 启动时 server.cpp 遍历注册表 co_spawn async_ws_subpub_loop，负责：
 *   async_connect → handshake → pump async_text_read → 断连重连（3→6→9→12 指数退避）
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
#include <queue>
#include <string>

#include "http_socket_client.h"
#include "atomic_guard.h"

#ifdef ENABLE_SOCKETS_CLIENT

namespace http
{

class sock_subpub_client : public std::enable_shared_from_this<sock_subpub_client>
{
  public:
    sock_subpub_client()          = default;
    virtual ~sock_subpub_client() = default;

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
    virtual void on_message(std::string_view payload) {}
    virtual void run_loop() {}

    // 协程钩子（跑 io_context 新协程）
    virtual asio::awaitable<void> async_on_open() { co_return; }
    virtual asio::awaitable<void> async_on_close() { co_return; }
    virtual asio::awaitable<void> async_on_message(std::string_view payload) { co_return; }
    virtual asio::awaitable<void> async_run_loop() { co_return; }

    // 业务主动发消息：入队即返回，由单写者 pump 在 sock->strand_ 上串行写出。
    // 与服务端 post_write 发送环同构：单写者 + 背压（队列满挤最旧），不在高并发下静默丢数。
    // 返回 false 仅当"当前没有活跃连接"。
    bool send(std::string_view msg)
    {
        if (!conn())
            return false;
        enqueue_and_pump(std::string(msg));
        return true;
    }
    // 协程版 send：同样入队，不阻塞业务线程
    asio::awaitable<bool> async_send(std::string_view msg)
    {
        if (!conn())
            co_return false;
        enqueue_and_pump(std::string(msg));
        co_return true;
    }

    // 活跃连接的快照，理由同 ws 侧：常驻循环在 io_context 上换/清它。
    std::shared_ptr<http::socket_client> conn()
    {
        std::lock_guard<std::mutex> lk(conn_mtx_);
        return sock_;
    }
    // 框架内部：只有常驻循环该调（业务别改）
    void set_conn(std::shared_ptr<http::socket_client> sock)
    {
        std::lock_guard<std::mutex> lk(conn_mtx_);
        sock_ = std::move(sock);
    }

    // 业务主动收摊（任意线程）：置旗 + 取消这条连接上的在途读。
    // 只写 isclose 收不了摊：常驻 pump 正 co_await 在 async_read() 上，不取消在途操作就
    // 永远不会回来查这一面旗。close_connect() 只 cancel + close、不 reset 槽位，
    // 看护线程关非常驻连接走的也是同一句。
    void stop()
    {
        isclose = true;
        // 副本出了锁再关：不握着锁跑别人的代码
        if (auto sock = conn())
            sock->close_connect();
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
    // 出站队列 + 单写者 pump（见 private 段）已取代原 send_lock_/send_busy_drop 静默丢弃模型

  private:
    std::shared_ptr<http::socket_client> sock_;// 活跃连接，只能经 conn()/set_conn() 碰
    std::mutex conn_mtx_;

    // ===== 出站队列 + 单写者 pump（与服务端 post_write 发送环同构）=====
    static constexpr std::size_t kMaxOutQueue = 1024;
    std::queue<std::string> out_q_;
    std::mutex out_q_mtx_;
    std::atomic<bool> pump_running_{false};

    void enqueue_and_pump(std::string msg)
    {
        {
            std::lock_guard<std::mutex> lk(out_q_mtx_);
            while (out_q_.size() >= kMaxOutQueue)   // 背压：挤掉最旧，而非丢当前条
                out_q_.pop();
            out_q_.push(std::move(msg));
        }
        // 无 pump 在跑才启动；running 标志保证同一连接只有一个写出协程
        if (!pump_running_.exchange(true))
        {
            if (auto s = conn())
            {
                auto self = shared_from_this();   // 保活：pump 运行期间本对象不被析构
                asio::co_spawn(s->strand_, [self]() -> asio::awaitable<void>
                               { co_await self->pump_outbound(); }, asio::detached);
            }
            else
            {
                pump_running_ = false;   // 没连接：释放标志，等重连后由新连接再起
            }
        }
    }

    asio::awaitable<void> pump_outbound()
    {
        std::shared_ptr<http::socket_client> s = conn();
        if (!s)
        {
            pump_running_ = false;
            co_return;
        }
        while (true)
        {
            std::string msg;
            {
                std::lock_guard<std::mutex> lk(out_q_mtx_);
                if (out_q_.empty())
                {
                    pump_running_ = false;             // 先标记停止
                    if (!out_q_.empty())              // 同锁内再确认，避免与生产者竞态丢唤醒
                    {
                        pump_running_ = true;
                        continue;
                    }
                    co_return;
                }
                msg = std::move(out_q_.front());
                out_q_.pop();
            }
            auto n = co_await s->async_write(msg);
            if (n == 0)
            {
                pump_running_ = false;
                co_return;   // 连接已断；残留消息等下次重连后由新连接 flush
            }
        }
    }
};

}// namespace http

#endif// ENABLE_SOCKETS_CLIENT
#endif
