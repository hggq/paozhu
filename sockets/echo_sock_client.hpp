/*
 * 框架级裸 TCP Socket 长连接验证客户端（同步版）
 *
 * 连接 conf/sockets.conf [default] 段 → server.conf 配的 TCP listen port
 * 裸 TCP 模式: async_tcp_connect() 发 "tcp /mytestsocket\n\n" 握手
 * pump 读原始字节 → on_message(payload)（无 opcode）
 *
 * 线程模型：
 *   is_coroutine_ = false  →  pump 回调丢 clientrunpool
 */
#pragma once

#include "sock_subpub_client.h"
#include <cstdio>
#include <string>

namespace sock_framework_test
{

class echo_sock_client : public http::sock_subpub_client
{
  public:
    echo_sock_client()
    {
        is_coroutine_ = false;
        is_loop_co_   = false;
        durtime       = 10;
        loop_num      = 999999;
    }

    std::string section_name() const override { return "default"; }

    void on_open() override
    {
        std::fprintf(stderr, "[echo_sock] ✅ OPEN\n");
        std::fflush(stderr);
    }

    void on_close() override
    {
        std::fprintf(stderr, "[echo_sock] ⚠️  CLOSE\n");
        std::fflush(stderr);
    }

    void on_message(std::string_view payload) override
    {
        std::fprintf(stderr, "[echo_sock] 📨  RECV %zu bytes = %s\n", payload.size(), std::string(payload).c_str());
        std::fflush(stderr);
    }

    void run_loop() override
    {
        static int round = 0;
        auto sock        = conn();
        if (!sock || !sock->sock)
            return;
        std::string msg = "SOCK_TEST #" + std::to_string(++round);
        if (send(msg))
            std::fprintf(stderr, "[echo_sock] 📤  SENT %s\n", msg.c_str());
        else
            std::fprintf(stderr, "[echo_sock] ❌  SEND FAIL\n");
        std::fflush(stderr);
    }
};

}// namespace sock_framework_test
