/*
 * 框架级 WebSocket 长连接验证客户端（同步版）
 *
 * 连接 conf/websockets.conf 的 [default] 段（echo.websocket.org:443/），
 * pump 收 echo 回来的消息，每 10s run_loop 发一次 "FRAMEWORK_TEST #N"。
 *
 * 线程模型：
 *   is_coroutine_ = false  →  pump 回调丢 clientrunpool 业务线程池
 *   run_loop()             →  tick 线程直接调
 */
#pragma once

#include "ws_subpub_client.h"
#include <atomic>
#include <cstdio>
#include <string>

namespace ws_framework_test
{

class echo_ws_client : public http::ws_subpub_client
{
  public:
    echo_ws_client()
    {
        is_coroutine_ = false;
        is_loop_co_   = false;
        durtime       = 10;// 每 10s 一拍
        loop_num      = 999999;
    }

    std::string section_name() const override { return "default"; }

    void on_open() override
    {
        auto ws = conn();
        if (!ws)
            return;
        std::fprintf(stderr, "[echo_ws] ✅ OPEN  → %s:%u%s\n", ws->host.c_str(), ws->port, ws->url.c_str());
    }

    void on_close() override
    {
        auto ws = conn();
        std::fprintf(stderr, "[echo_ws] ⚠️  CLOSE  iserror=%d\n", ws ? (int)ws->iserror : -1);
    }

    void on_message(const std::string &payload, bool is_binary) override
    {
        std::fprintf(stderr, "[echo_ws] 📨  %s  payload=%s\n", is_binary ? "BINARY" : "TEXT", payload.c_str());
    }

    void run_loop() override
    {
        static int round = 0;
        auto ws          = conn();
        if (!ws || !ws->sock)
            return;
        std::string msg = "FRAMEWORK_TEST #" + std::to_string(++round);
        if (send_text(msg))
            std::fprintf(stderr, "[echo_ws] 📤  SENT %s\n", msg.c_str());
        else
            std::fprintf(stderr, "[echo_ws] ❌  SEND FAIL (socket=%p)\n", (void *)ws->sock.get());
    }
};

}// namespace ws_framework_test
