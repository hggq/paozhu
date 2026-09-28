#include <iostream>
#include <memory>
#include <string_view>

#include "orm.h"
#include "websockets.h"
#include "terminal_color.h"

namespace http
{

// 协程版 ws demo。isco=true → onopen/onmessage/onclose 全协程；isloopco=true → run_loop 走 async 分支
class loopwebsockets_co : public websockets_api
{
  public:

    loopwebsockets_co(unsigned int m, unsigned int g) : websockets_api(8, m, g, 0)
    {
        isco = true;       // 消息回调走协程（async_onopen/async_onmessage/async_onclose）
        isloopco = true;   // 周期回调走协程（async_run_loop）
    }
    ~loopwebsockets_co() { std::cout << "~loopwebsockets_co" << std::endl; }

  public:
    void onopen() override
    {
        // isco=true 时 server.cpp 不会走这里
        loop_num = 8;
    }

    asio::awaitable<void> async_onopen() override
    {
        loop_num = 8;
        std::cout << "loopwebsockets_co async_onopen" << std::endl;
        co_return;
    }

    void onclose() override
    {
        isclose = true;
    }

    asio::awaitable<void> async_onclose() override
    {
        std::cout << "loopwebsockets_co async_onclose" << std::endl;
        co_return;
    }

    void onpong() override {}

    // 被 isloopco=true 选中（server.cpp L3491 走 co_spawn(async_run_loop)）
    void run_loop() override
    {
        // isloopco=true 时 server.cpp 不会走这里；留空占位满足纯虚函数
    }

    asio::awaitable<void> async_run_loop() override
    {
        auto self = shared_from_this();
        if (self->session_sock)
        {
            std::cout << "loopwebsockets_co async_run_loop" << std::endl;
            // 统一走 send() → 环
            self->send("test co run_loop");
            if (self->loop_num == 4)
            {
                self->loop_num = 0;
                co_return;
            }
            self->loop_num--;
        }
        else
        {
            self->isclose = true;
            self->loop_num = 0;
        }
        co_return;
    }

    asio::awaitable<void> async_onmessage(websockets_data_list_t &&msg) override
    {
        auto self = shared_from_this();
        // 统一走 send() → 环
        self->send(msg.value);
        co_return;
    }

    // isco=true 时 server.cpp 不会走这里
    void onmessage() override {}
};

}// namespace http
