#pragma once
/*
 * WebSocket 长连接客户端业务注册入口
 *
 * 业务侧的 ws_subpub_client 子类写在 websockets/ 目录里（vendor 里不能放业务代码），
 * 在这个文件里 include 并 emplace 进注册表。
 * server.cpp 启动时遍历注册表，对每个客户端 co_spawn 一个长期运行协程：
 *   async_connect → handshake → pump async_text_read → 断连重连循环
 * websocket_loop 每秒一拍扫 ws_subpub_tasks，按 durtime 分频调 run_loop/async_run_loop。
 *
 * 线程模型：完全对称 redis_subpub_client
 *   is_coroutine_ = false  →  pump 回调丢 clientrunpool
 *   is_coroutine_ = true   →  pump 回调 co_spawn 到 io_context 新协程
 */
#ifdef ENABLE_WEBSOCKETS_CLIENT

#include "ws_subpub_reg.h"

#include "websockets/echo_ws_client.hpp"     // 同步版（框架级验证）
#include "websockets/echo_ws_client_co.hpp"  // 协程版（框架级验证）

namespace http
{

inline void _initwssubpubregto(http::WS_SUBPUB_REG &reg)
{
    // reg.emplace("my_ws_client",
    //             [] { return std::make_shared<my_ns::my_ws_client>(); });

    // 框架级验证客户端（跑通后业务侧自行增删）
    // 注意：conf/websockets.conf 的 [default] 段指向本 server 自己的 127.0.0.1:80 /wstest，
    // 两条循环都读那一段；该段 enable = 0 可以把它们一起软关掉（只在启动时读一次）。
    reg.emplace("echo_ws_client",
                [] { return std::make_shared<ws_framework_test::echo_ws_client>(); });
    reg.emplace("echo_ws_client_co",
                [] { return std::make_shared<ws_framework_test::echo_ws_client_co>(); });
}

} // namespace http

#endif // ENABLE_WEBSOCKETS_CLIENT
