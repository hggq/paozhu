#pragma once

#ifdef ENABLE_REDIS_CLIENT
/*
 * Redis pubsub 业务订阅客户端注册入口
 *
 * 业务侧的 redis_subpub_client 子类写在 redis/ 目录里（vendor 里不能放业务代码），
 * 在这个文件里 include 并 emplace 进注册表。
 * server.cpp 启动时遍历注册表，对每个订阅类 co_spawn 一个长期运行协程：
 *   async_start → async_subscribe → pump → 断连重连循环
 * websocket_loop 每秒一拍扫 redis_subpub_tasks，按 durtime 分频调 run_loop/async_run_loop。
 *
 * 线程模型：
 *   is_coroutine_ = false  →  pump 回调丢 clientrunpool 业务线程池（post_conn_step/co_pool_run_void）
 *   is_coroutine_ = true   →  pump 回调 co_spawn 到 io_context 新协程，钩子 co_await async_* 版
 */
#include "redis_subpub_reg.h"
#include "redis/echo_subscriber.hpp"     // 同步版（框架级验证用）
#include "redis/echo_subscriber_co.hpp"  // 协程版（框架级验证用）

namespace http
{

inline void _initredissubpubregto(pz::redis::REDIS_SUBPUB_REG &reg)
{
    // reg.emplace("my_channel_sub", [] { return std::make_shared<my_channel_sub>(); });
    // reg.emplace("my_channel_sub_co", [] { return std::make_shared<my_channel_sub_co>(); });

    // 框架级验证订阅（跑通后业务侧自行增删）
    reg.emplace("echo_test_sub",
                [] { return std::make_shared<redis_framework_test::echo_subscriber>(); });
    reg.emplace("echo_test_sub_co",
                [] { return std::make_shared<redis_framework_test::echo_subscriber_co>(); });
}

} // namespace http

#endif // ENABLE_REDIS_CLIENT
