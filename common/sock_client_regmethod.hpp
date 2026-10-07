#pragma once
/*
 * Socket 长连接客户端业务注册入口
 *
 * 业务侧的 sock_subpub_client 子类写在 sockets/ 目录里，
 * 在这个文件里 include 并 emplace 进注册表。
 * server.cpp 启动时遍历注册表，对每个客户端 co_spawn async_sock_subpub_loop：
 *   async_tcp_connect → pump async_read → 断连重连循环
 * websocket_loop 每秒一拍扫 sockets_clients，按 durtime 分频调 run_loop/async_run_loop。
 *
 * 线程模型：完全对称 ws_subpub_client
 *   is_coroutine_ = false  →  pump 回调丢 clientrunpool
 *   is_coroutine_ = true   →  pump 回调 co_spawn 到 io_context 新协程
 */
#ifdef ENABLE_SOCKETS_CLIENT

#include "sock_subpub_reg.h"

#include "sockets/echo_sock_client.hpp"
#include "sockets/echo_sock_client_co.hpp"

namespace http
{

inline void _initsockssubpubregto(http::SOCK_SUBPUB_REG &reg)
{
    // reg.emplace("my_sock_client",
    //             [] { return std::make_shared<my_ns::my_sock_client>(); });

    // 框架级验证客户端（跑通后业务侧自行增删）
    // 注: conf/sockets.conf [default] 的 host/port 指向本 server（裸 TCP 握手复用 httpport），
    //     url 是 sockets_method_reg.hpp 里注册的 handler 名
    reg.emplace("echo_sock_client",
                [] { return std::make_shared<sock_framework_test::echo_sock_client>(); });
    reg.emplace("echo_sock_client_co",
                [] { return std::make_shared<sock_framework_test::echo_sock_client_co>(); });
}

} // namespace http

#endif // ENABLE_SOCKETS_CLIENT
