/*
 * 框架级 WebSocket 长连接验证客户端（协程版）
 *
 * 镜像 echo_ws_client.hpp，但钩子全走协程版。
 * pump 回调 → co_spawn(io_context, async_on_message) 新协程
 * run_loop → co_spawn(io_context, async_run_loop) 新协程
 *
 * 线程模型：
 *   is_coroutine_ = true  →  pump 回调 co_spawn 到 io_context
 *   is_loop_co_   = true  →  async_run_loop 在 io_context 里 co_await
 */
#pragma once

#include "ws_subpub_client.h"
#include <atomic>
#include <cstdio>
#include <string>

namespace ws_framework_test
{

class echo_ws_client_co : public http::ws_subpub_client
{
  public:
    echo_ws_client_co()
    {
        is_coroutine_ = true;
        is_loop_co_   = true;
        durtime       = 10;
        loop_num      = 999999;
    }

    std::string section_name() const override { return "default"; }

    asio::awaitable<void> async_on_open() override
    {
        auto ws = conn();
        if (ws)
            std::fprintf(stderr, "[echo_ws_co] ✅ OPEN  → %s:%u%s\n", ws->host.c_str(), ws->port, ws->url.c_str());
        co_return;
    }

    asio::awaitable<void> async_on_close() override
    {
        std::fprintf(stderr, "[echo_ws_co] ⚠️  CLOSE\n");
        co_return;
    }

    asio::awaitable<void> async_on_message(const std::string &payload,
                                           bool is_binary) override
    {
        std::fprintf(stderr, "[echo_ws_co] 📨  %s  payload=%s\n", is_binary ? "BINARY" : "TEXT", payload.c_str());
        co_return;
    }

    asio::awaitable<void> async_run_loop() override
    {
        static int round = 0;
        auto ws          = conn();
        if (!ws || !ws->sock)
            co_return;
        std::string msg = "CO_FRAMEWORK_TEST #" + std::to_string(++round);
        if (send_text(msg))
            std::fprintf(stderr, "[echo_ws_co] 📤  SENT %s\n", msg.c_str());
        else
            std::fprintf(stderr, "[echo_ws_co] ❌  SEND FAIL\n");
        co_return;
    }
};

}// namespace ws_framework_test
