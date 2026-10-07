/*
 * 框架级裸 TCP Socket 长连接验证客户端（协程版）
 *
 * 镜像 echo_sock_client.hpp，钩子全走协程版。
 * pump 回调 → co_spawn(io_context, async_on_message)
 */
#pragma once

#include "sock_subpub_client.h"
#include <cstdio>
#include <string>

namespace sock_framework_test
{

class echo_sock_client_co : public http::sock_subpub_client
{
  public:
    echo_sock_client_co()
    {
        is_coroutine_ = true;
        is_loop_co_   = true;
        durtime       = 10;
        loop_num      = 999999;
    }

    std::string section_name() const override { return "default"; }

    asio::awaitable<void> async_on_open() override
    {
        std::fprintf(stderr, "[echo_sock_co] ✅ OPEN\n");
        std::fflush(stderr);
        co_return;
    }

    asio::awaitable<void> async_on_close() override
    {
        std::fprintf(stderr, "[echo_sock_co] ⚠️  CLOSE\n");
        std::fflush(stderr);
        co_return;
    }

    asio::awaitable<void> async_on_message(std::string_view payload) override
    {
        std::fprintf(stderr, "[echo_sock_co] 📨  RECV %zu bytes = %s\n", payload.size(), std::string(payload).c_str());
        std::fflush(stderr);
        co_return;
    }

    asio::awaitable<void> async_run_loop() override
    {
        static int round = 0;
        auto sock        = conn();
        if (!sock || !sock->sock)
            co_return;
        std::string msg = "CO_SOCK_TEST #" + std::to_string(++round);
        if (co_await async_send(msg))
            std::fprintf(stderr, "[echo_sock_co] 📤  SENT %s\n", msg.c_str());
        else
            std::fprintf(stderr, "[echo_sock_co] ❌  SEND FAIL\n");
        std::fflush(stderr);
        co_return;
    }
};

}// namespace sock_framework_test
