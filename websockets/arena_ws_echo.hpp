#pragma once
//
// arena_ws_echo.hpp — HttpArena /ws echo handler (text + binary).
//
// Registered under key "ws" by common/websockets_method_reg.hpp; the framework
// looks the key up as peer->pathinfos[0] in client_websocket_loop (server.cpp).
// Coroutine mode (isco=true): one async_onmessage per inbound message, so a
// -p 16 batch is drained without a business-side queue.
//
// is_binary is filled by ws_parser from message_assembler::message_opcode, so
// text and binary frames keep their opcode on the way back — validate-ws.py
// checks both.
//

#include "websockets.h"

#include <mutex>
#include <utility>

namespace http
{

class arena_ws_echo : public websockets_api
{
  public:
    // durtime=0 keeps the connection out of the websocket_loop timer queue:
    // this handler never sends anything on its own, it only echoes.
    arena_ws_echo(unsigned int m, unsigned int g) : websockets_api(0, m, g, 0)
    {
        isco     = true;
        isloopco = true;
    }

    void onopen() override {}
    asio::awaitable<void> async_onopen() override { co_return; }

    void onclose() override { isclose = true; }
    asio::awaitable<void> async_onclose() override
    {
        isclose = true;
        co_return;
    }

    void run_loop() override {}
    asio::awaitable<void> async_run_loop() override { co_return; }

    asio::awaitable<void> async_onmessage(websockets_data_list_t &&msg) override
    {
        // Spilled messages (> CONST_WEBSOCKET_SPILL_THRESHOLD) carry a temp file
        // name in value; benchmark/validation frames never spill.
        if (!msg.isfile)
        {
            if (msg.is_binary)
            {
                send_binary(msg.value);
            }
            else
            {
                send(msg.value);
            }
        }
        co_return;
    }

    // Synchronous branch is never used (isco=true), but the interface is pure virtual.
    void onmessage() override
    {
        websockets_data_list_t msg;
        {
            std::unique_lock<std::mutex> lock(content_list_mutex);
            if (content_list.empty())
            {
                return;
            }
            msg = std::move(content_list.front());
            content_list.pop_front();
        }

        if (msg.isfile)
        {
            return;
        }
        if (msg.is_binary)
        {
            send_binary(msg.value);
        }
        else
        {
            send(msg.value);
        }
    }
};

}// namespace http
