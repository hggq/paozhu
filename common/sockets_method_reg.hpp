#pragma once
#include <map>
#include "httppeer.h"
#include "client_session.h"
#include "http_socket.h"
#include "my_test_socket.hpp"
#include "async_my_test_socket.hpp"
#include "sync_my_test_socket.hpp"

namespace http
{
void _inithttpsocketmethodregto(HTTP_SOCKET_REG &methodcallback)
{

    // 同步 tick 版（构造走默认参数 isco=false + isloopco=false）：周期 tick 由 socket_loop 直接
    // 调 run_loop()，报文仍在 async_on_message 里。下面 mytestsocketco 是同一份代码换成协程 tick，
    // 两条对照才能同时看到"同步 tick"和"协程 tick"两种分派。
    methodcallback.emplace("mytestsocket", [](unsigned int myid_,unsigned int groupid_, std::shared_ptr<client_session> s_sock) -> std::shared_ptr<socket_api>
                           { return std::make_shared<my_test_socket>(myid_, groupid_, s_sock); });

    // tcp 协程 tick 版（只有 isloopco=true：on_open/on_close 仍是同步钩子、跑业务线程池，
    // 周期 tick 换成 co_spawn 到会话 strand 上的 async_run_loop）
    methodcallback.emplace("mytestsocketco", [](unsigned int myid_,unsigned int groupid_, std::shared_ptr<client_session> s_sock) -> std::shared_ptr<socket_api>
                           { return std::make_shared<my_test_socket>(myid_, groupid_, s_sock, false, true); });

    // tcp 全协程版（isco=true 且 isloopco=true）：on_open/on_close 走 async_ 钩子、留在协程线程上
    // 并且体内可以 co_await，tick 也走协程。和上面两条对照读，见 sockets/async_my_test_socket.hpp
    methodcallback.emplace("mytestsocketasync", [](unsigned int myid_,unsigned int groupid_, std::shared_ptr<client_session> s_sock) -> std::shared_ptr<socket_api>
                           { return std::make_shared<async_my_test_socket>(myid_, groupid_, s_sock); });

    // tcp 同步钩子版（issyncmsg=true）：报文进接收队列，on_message() 在业务线程池上执行，
    // 读环投完就走 —— 钩子里放阻塞不会把这条连接的入站读停住。
    // 上面三条的报文都在 async_on_message 里，对照着读见 sockets/sync_my_test_socket.hpp
    methodcallback.emplace("mytestsocketsync", [](unsigned int myid_,unsigned int groupid_, std::shared_ptr<client_session> s_sock) -> std::shared_ptr<socket_api>
                           { return std::make_shared<sync_my_test_socket>(myid_, groupid_, s_sock); });

}

}// namespace http