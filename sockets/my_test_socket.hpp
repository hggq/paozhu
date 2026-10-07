#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>

#include "orm.h"
#include "httppeer.h"
#include "http_socket.h"
#include "terminal_color.h"

namespace http
{

namespace sock_probe_detail
{
// 线程身份的十进制指纹，用来判断"这段代码跑在哪条线程上"。
inline std::string tid_text(std::thread::id id)
{
    return std::to_string(std::hash<std::thread::id>{}(id));
}
inline std::string tid_now()
{
    return tid_text(std::this_thread::get_id());
}
}// namespace sock_probe_detail

class my_test_socket : public socket_api
{
  public:
    // 构造参数决定(Construction parameters determine) isco/isloopco
    my_test_socket(unsigned int m, unsigned int g, std::shared_ptr<client_session> s_sock, bool co_style = false, bool loop_co = false)
        : socket_api(7, m, g, 0, s_sock)
    {
        isco     = co_style;// 报文钩子只有 async_on_message 这一条；同步版在 sockets/sync_my_test_socket.hpp
        isloopco = loop_co; // false → server 调 run_loop()；true → co_spawn(async_run_loop)
    }
    ~my_test_socket() override
    {
        DEBUG_LOG(" ~my_test_socket ");
        isclose = true;
    };

  public:
    // on_open 的执行线程指纹。同步 on_open 现在跑在业务线程池上，async_on_open 还在 io 线程上，
    // 存下这条 id 就能把两段代码各自的落点读出来（下面 probe/tid 在用）。
    // 只存 std::thread::id 本身：按连接一次赋值，格式化留到真有人问才做。
    std::thread::id open_tid;

    void on_open() override
    {
        DEBUG_LOG(" onopen ");
        open_tid = std::this_thread::get_id();
    }
    asio::awaitable<void> async_on_open() override
    {
        DEBUG_LOG(" async_on_open ");
        open_tid = std::this_thread::get_id();
        co_return;
    }
    void on_close() override
    {
        DEBUG_LOG(" onclose ");
        isclose = true;
    }

    asio::awaitable<void> async_on_close() override
    {
        DEBUG_LOG(" async_on_close ");
        isclose = true;
        co_return;
    }

    asio::awaitable<void> async_on_message(const std::string &buffer) override
    {
        auto self = shared_from_this();
        DEBUG_LOG(" async_on_message:%s", buffer.c_str());
        // 出站一律用 send()：数据进这条连接的发送环，由环的消费者串行写出去。
        // 这几段回显不做重投（一次一行，环要满 15 行没写出去才会拒），要看背压怎么自己接住，
        // 读 sockets/sync_my_test_socket.hpp 那一份。send() 的返回值必须检查：环满时这一片
        // 没进去、仍在调用方手里——演示里只记一行日志（真实业务应当压着重投，见 sync 那份）。
        // 演示分支一：probe/tid 回两个指纹——open= 是 on_open 的执行线程（同步版跑在业务线程池上，
        // 协程版 async_on_open 跑在 io 线程上），now= 是这条协程钩子的执行线程。两者不该相同。
        // 演示分支二：probe/anchor 只回 now=，拿来当 io 线程的样本集，与 open= 那一批求交集即可
        // 判"同步钩子到底有没有离开 io 线程"。
        if (buffer.compare(0, 9, "probe/tid") == 0)
        {
            if (!self->send("open=" + sock_probe_detail::tid_text(this->open_tid) +
                            " now=" + sock_probe_detail::tid_now() + "\n"))
                DEBUG_LOG("socket send refused (probe/tid)");
            co_return;
        }
        if (buffer.compare(0, 12, "probe/anchor") == 0)
        {
            if (!self->send("anchor=" + sock_probe_detail::tid_now() + "\n"))
                DEBUG_LOG("socket send refused (probe/anchor)");
            co_return;
        }
        // 演示分支三：probe/ids 回这条会话握手串里的 myid/groupid。握手名对上不等于这两个参数
        // 也对上——名字段之后的 /myid/groupid 是服务端切段后传进构造函数的，回显出来才看得见。
        if (buffer.compare(0, 9, "probe/ids") == 0)
        {
            if (!self->send("myid=" + std::to_string(this->myid) + " groupid=" + std::to_string(this->groupid) + "\n"))
                DEBUG_LOG("socket send refused (probe/ids)");
            co_return;
        }
        if (!self->send(buffer))
            DEBUG_LOG("socket send refused, %zu bytes dropped", buffer.size());
        co_return;
    }
    void run_loop() override
    {
        DEBUG_LOG(" run_loop ");
    }
    asio::awaitable<void> async_run_loop() override
    {
        auto self = shared_from_this();// Phase 1 #14
        if (self->session_sock)
        {
            // 这一拍的心跳没进环就跳过，下一拍还会再发一次。
            // 结尾这个 \n 不能省：raw socket 是一条字节流，框架不替业务分行，
            // 少了它这一拍就会和下一片（比如一条回显）粘成同一行，读端按行切就切错了。
            if (!self->send("server socket loop send\n"))
                DEBUG_LOG("socket loop send refused");
            self->session_sock->time_limit.store(timeid());
        }
        DEBUG_LOG(" async_run_loop ");
        co_return;
    }
};

}// namespace http