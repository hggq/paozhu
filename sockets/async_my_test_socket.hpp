#pragma once

// Socket 业务示例的协程版，和 my_test_socket.hpp 成对读：
//   isco=true     → 框架调 async_on_open / async_on_close（后者经 co_socket_on_close 收口），
//                   这两个钩子留在协程线程上跑，体内可以 co_await；
//   isco=false    → 同步 on_open / on_close 交给业务线程池（clientrunpool）跑，续体回到本连接
//                   协程，所以报文先后次序照旧——同步版那份文件就是这个形状。
// 报文这一层有两种分派：默认走 async_on_message（就是这两份示例的形状），
// 把 issyncmsg 置成 true 则改走「接收队列 + 同步 on_message」，见 sockets/sync_my_test_socket.hpp。

#include <chrono>
#include <functional>
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

namespace async_sock_probe_detail
{
// 线程身份的十进制指纹。算法与 my_test_socket.hpp 里的 sock_probe_detail 逐字相同——
// 两份头各自独立（同 mqtt 那对示例的做法），但回显里的 tid= 要能和那边直接对上
// （同一个数代表同一条线程），换了算法两边就再也对不上，所以这里照抄而不是另起一份。
inline std::string tid_text(std::thread::id id)
{
    return std::to_string(std::hash<std::thread::id>{}(id));
}
inline std::string tid_now()
{
    return tid_text(std::this_thread::get_id());
}
}// namespace async_sock_probe_detail

class async_my_test_socket : public socket_api
{
  public:
    // 构造参数决定 isco/isloopco，默认取协程版那一档：
    // isloopco=true → tick 把 async_run_loop co_spawn 到本会话的 strand 上
    // 第一个构造参数 socket_api(7, ...) 里的 7 是 timeloop_num，tick 的除数（fps % 7 == 0 那一拍跑一次）
    async_my_test_socket(unsigned int m, unsigned int g, std::shared_ptr<client_session> s_sock, bool co_style = true, bool loop_co = true)
        : socket_api(7, m, g, 0, s_sock)
    {
        isco     = co_style;
        isloopco = loop_co;
    }
    ~async_my_test_socket() override
    {
        DEBUG_LOG(" ~async_my_test_socket ");
        isclose = true;
    };

  public:
    // on_open 的执行线程指纹，和同步版同名同义：拿它跟 async_on_message 里的 now= 一起
    // 就能读出"这段代码跑在哪条线程上"。只存 id 本身，格式化留到真有人问才做。
    std::thread::id open_tid;

    // — 三条纯虚的同步钩子 —
    // isco=true 时框架不会调它们，但 socket_api 把这三条定成纯虚，不给实现就编不过。
    // 各自留一行日志，万一把 isco 关掉（构造参数）就看得出走的到底是哪条。
    // 注意这三条一旦被走到，线程归属都不在 io 线程族：on_open / on_close 跑业务线程池，
    // run_loop 跑 tick 线程——别把它们和下面协程钩子的线程指纹混进同一批里比。
    void on_open() override
    {
        DEBUG_LOG(" sync on_open (isco=false) ");
        open_tid = std::this_thread::get_id();
    }
    void on_close() override
    {
        DEBUG_LOG(" sync on_close (isco=false) ");
        isclose = true;
    }
    void run_loop() override
    {
        DEBUG_LOG(" sync run_loop (isloopco=false) ");
        if (session_sock)
        {
            session_sock->time_limit.store(timeid());
        }
    }

    // — 协程版钩子 —
    asio::awaitable<void> async_on_open() override
    {
        open_tid = std::this_thread::get_id();
        DEBUG_LOG(" async_on_open ");
        // 协程版 on_open 买到的就是这里能 co_await：连接放行前先做一件异步的事，例如查授权表
        //   auto u = orm::cms::Sysuser();
        //   u.where("username", name_from_handshake).limit(1);
        //   co_await u.async_fetch_one();
        //   if (u.effect() == 0) { isclose = true; co_return; }// 表里没有 ⇒ 置 isclose 否决本连接
        // 反面约束也记在这里：这条钩子不在业务线程池上（协程钩子一律留在 io 线程族），
        // 所以体内别放阻塞调用——那正是同步钩子下池要解决的问题，见 my_test_socket.hpp 的 on_open。
        co_return;
    }

    asio::awaitable<void> async_on_close() override
    {
        auto self = shared_from_this();
        DEBUG_LOG(" async_on_close ");
        // 收尾钩子真的可以挂起：这颗定时器就是证明（真业务换成 co_await 一条异步 UPDATE 落流水）。
        // 这里不往回写东西：on_close 的三条出口里有一条是"读已经出错"之后，写不保证送达。
        asio::steady_timer timer(co_await asio::this_coro::executor);
        timer.expires_after(std::chrono::milliseconds(20));
        co_await timer.async_wait(asio::use_awaitable);
        isclose = true;
        co_return;
    }

    asio::awaitable<void> async_on_message(const std::string &buffer) override
    {
        auto self = shared_from_this();
        DEBUG_LOG(" async_on_message:%s", buffer.c_str());
        // 两把探针的拼写和同步版完全一致，于是同一个客户端可以分别连
        // "tcp mytestsocket"（同步钩子，跑业务线程池）和 "tcp mytestsocketasync"（这一份），
        // 把两边的线程指纹求交集——应当为空：协程钩子留在 io 线程族，同步钩子已经下池。
        //
        // probe/tid 回两个字段：open= 是 async_on_open 的执行线程，now= 是这条报文钩子的执行线程。
        // 和同步版不同，这两个都落在 io 线程族里；而且 strand 只保证同会话不重叠、不钉线程，
        // 所以它们自己也可以不相等——别把"open= 和 now= 不同"当回归。
        if (buffer.compare(0, 9, "probe/tid") == 0)
        {
            if (!self->send("open=" + async_sock_probe_detail::tid_text(this->open_tid) +
                            " now=" + async_sock_probe_detail::tid_now() + "\n"))
                DEBUG_LOG("socket send refused (probe/tid)");
            co_return;
        }
        // probe/anchor 只回一个 now=：报的是"这条连接的协程钩子此刻跑在哪条线程上"，
        // 拿它和上面 probe/tid 的回显对照着看，就能看出两种钩子各在哪一类线程上执行。
        if (buffer.compare(0, 12, "probe/anchor") == 0)
        {
            if (!self->send("anchor=" + async_sock_probe_detail::tid_now() + "\n"))
                DEBUG_LOG("socket send refused (probe/anchor)");
            co_return;
        }
        // 出站一律走 send()：数据进这条连接的发送环，由环的消费者串行写出去。
        // 这几段回显不做重投，要看背压怎么自己接住，读 sockets/sync_my_test_socket.hpp。
        // send() 的返回值必须检查：环满时这一片没进去、仍在调用方手里——演示只记一行日志。
        if (!self->send(buffer))
            DEBUG_LOG("socket send refused, %zu bytes dropped", buffer.size());
        co_return;
    }

    asio::awaitable<void> async_run_loop() override
    {
        auto self = shared_from_this();
        if (self->session_sock)
        {
            tick_seq++;
            std::string content = "async loop ";
            content.append(std::to_string(tick_seq));
            content.append(" from ");
            content.append(self->url);
            content.push_back('\n');
            // 这一拍的心跳没进环就跳过，下一拍还会再发一次。
            if (!self->send(content))
                DEBUG_LOG("socket loop send refused");
            // 心跳刷新：会话的空闲回收看的是 time_limit，不刷就会被摘
            self->session_sock->time_limit.store(timeid());
        }
        DEBUG_LOG(" async_run_loop ");
        co_return;
    }

  private:
    unsigned int tick_seq = 0;// async_run_loop 的轮次计数（strand 上访问）
};

}// namespace http
