#ifndef __HTTP_OBJ_SOCKET_API_H
#define __HTTP_OBJ_SOCKET_API_H

#include <atomic>
#include <list>
#include <mutex>
#include <vector>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include "cost_define.h"
#include "request.h"
#include "httppeer.h"
#include "client_session.h"

namespace http
{
struct socket_api_data_t
{
    std::string name;
    std::string value;
};
struct socket_data_list_t
{
    unsigned long long seqid = 0;
    // 这一片被读上来的时刻（毫秒，单调时钟）。框架在入队前填好，业务只读。
    // 同步钩子在池线程上排队，靠它算出这一片在队列里等了多久。
    unsigned long long arrived_ms = 0;
    std::string value;
};
class socket_api : public std::enable_shared_from_this<socket_api>
{
  public:
    socket_api(unsigned int t, unsigned int m, unsigned int g, unsigned char s, std::shared_ptr<client_session> s_sock) : state(s), timeloop_num(t), myid(m), groupid(g), session_sock(s_sock) {}
    virtual void on_open()                         = 0;
    virtual asio::awaitable<void> async_on_open()  = 0;
    virtual void on_close()                        = 0;
    virtual asio::awaitable<void> async_on_close() = 0;
    unsigned int keepalive_ms                      = 0;
    virtual void on_keepalive() {}
    virtual asio::awaitable<void> async_on_keepalive() { co_return; }
    virtual asio::awaitable<void> async_on_message(const std::string &buffer)
    {
        unsigned int readoffset = 0;
        for (; readoffset < buffer.size(); readoffset++)
        {
            if (buffer[readoffset] == '\n')
            {
                break;
            }
        }
        co_return;
    }
    // 同步入站钩子：跑在业务线程池上，允许在里面阻塞（查库、调下游都行）。
    // 入参是一次 read 拿到的字节切片，seqid 是这条连接上的到达序号。
    // 只有把 issyncmsg 置 true 的连接才会走到这里；置了却又不覆盖本函数，
    // 表现是"连得上、回声永远不回来"（默认体把这一片丢掉）。
    virtual void on_message(socket_data_list_t &&msg)
    {
        (void)msg;
    }
    // 入队带水位闸。返回 false 表示队列已满、msg 没有被移走，调用方可以拿同一片重试。
    // 双闸：条数（防海量极小包炸 list 节点）+ 字节（钉单连接入站内存上界，读窗口放大后也只到 512KB）。
    bool push(socket_data_list_t &&msg)
    {
        std::unique_lock<std::mutex> lock(content_list_mutex);
        if (content_list.size() >= CONST_SOCKET_QUEUE_HIGH_ITEMS ||
            content_bytes + msg.value.size() > CONST_SOCKET_QUEUE_HIGH_BYTES)
        {
            return false;
        }
        content_list.push_back(std::move(msg));
        content_bytes += content_list.back().value.size();
        return true;
    }
    // 队列深度现问现取：出队由框架做，任何累减式记账都会漂。
    unsigned long long queue_items() const
    {
        std::unique_lock<std::mutex> lock(content_list_mutex);
        return content_list.size();
    }
    // 从队头取一片；队列空时返回 false，msg 不被赋值。
    bool pop_front(socket_data_list_t &msg)
    {
        std::unique_lock<std::mutex> lock(content_list_mutex);
        if (content_list.empty())
        {
            return false;
        }
        msg = std::move(content_list.front());
        content_bytes -= msg.value.size();
        content_list.pop_front();
        return true;
    }
    // 统一出站接口 —— 和 websocket 同一形状：数据一律进本连接的发送环，
    // ring_client_server 是这条连接唯一的写者。
    // 禁止业务代码直接调用 session_sock->send_writer / async_send_writer 发数据：
    // 报文钩子在业务线程池、周期 tick 在定时线程、协程钩子在这条连接的 strand 上，
    // 两边同时裸写同一个 socket 就会把字节咬交错。
    // 返回 true = 环已经把这一片拷走，调用方的缓冲可以立刻释放；
    // 返回 false = 没进去（环满 15 片 / 积压过 1 MiB / 连接已关 / 还没建环），
    // 数据仍然完整地在调用方手里，自己留着下一拍重投。调用方必须检查返回值。
    bool send(std::string_view payload)
    {
        if (!session_sock)
            return false;
        return session_sock->post_write(payload);
    }
    bool line_mode = false;
    std::string line_buffer_;
    // sync 入站统一入口：框架入站派发调这里；line_mode 时下钻到 on_message_line。
    void feed(socket_data_list_t &&msg)
    {
        if (!line_mode)
        {
            on_message(std::move(msg));
            return;
        }
        line_buffer_.append(msg.value);
        std::size_t pos = 0;
        for (;;)
        {
            std::size_t nl = line_buffer_.find('\n', pos);
            if (nl == std::string::npos)
                break;
            std::string line = line_buffer_.substr(pos, nl - pos);
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            on_message_line(line);
            pos = nl + 1;
        }
        line_buffer_.erase(0, pos);
    }
    // 行模式钩子：默认把行当整片交给 on_message（保持兼容）；业务可 override 直接收行。
    virtual void on_message_line(const std::string &line)
    {
        socket_data_list_t m;
        m.value = line;
        on_message(std::move(m));
    }
    // async 入站统一入口：读环 / 首片调这里；line_mode 时下钻到 async_on_message_line。
    asio::awaitable<void> feed_async(std::string slice)
    {
        if (!line_mode)
        {
            co_await async_on_message(slice);
            co_return;
        }
        line_buffer_.append(slice);
        std::size_t pos = 0;
        for (;;)
        {
            std::size_t nl = line_buffer_.find('\n', pos);
            if (nl == std::string::npos)
                break;
            std::string line = line_buffer_.substr(pos, nl - pos);
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            co_await async_on_message_line(line);
            pos = nl + 1;
        }
        line_buffer_.erase(0, pos);
    }
    virtual asio::awaitable<void> async_on_message_line(const std::string &line)
    {
        co_await async_on_message(line);
    }
    virtual void run_loop()                        = 0;
    virtual asio::awaitable<void> async_run_loop() = 0;
    virtual ~socket_api()
    {
        isclose = true;
    };

  public:
    bool isclose  = false;
    bool isfinish = false;
    bool isco     = false;
    bool isloopco = false;
    // true → 入站走上面的接收队列，业务钩子用同步版 on_message（跑在业务线程池上）；
    // false（默认）→ 框架照旧在会话协程上 co_await async_on_message。
    bool issyncmsg = false;
    // 每连接单飞：同一时刻只允许一个入站任务在跑，保证同一条流的切片按到达顺序被处理。
    std::atomic_flag inbound_running = ATOMIC_FLAG_INIT;
    std::list<socket_data_list_t> content_list;
    // 入站队列里所有片的字节数总和（与 content_list_mutex 同一把锁保护），供字节闸判定。
    std::size_t content_bytes = 0;
    mutable std::mutex content_list_mutex;
    unsigned char state;
    unsigned int timeloop_num;
    unsigned int myid         = 0;
    unsigned int groupid      = 0;
    unsigned int current_time = 0;

    std::string error_msg;
    std::string url;
    std::string host;
    std::shared_ptr<client_session> session_sock;
    std::vector<socket_api_data_t> header;
};
typedef std::map<std::string, std::function<std::shared_ptr<socket_api>(unsigned int myid_, unsigned int groupid_, std::shared_ptr<client_session>)>> HTTP_SOCKET_REG;
HTTP_SOCKET_REG &get_http_socket_reg();

}// namespace http

#endif