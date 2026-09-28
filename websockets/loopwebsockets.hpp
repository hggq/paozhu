#include <iostream>
#include <memory>
#include <string_view>

#include "orm.h"
#include "websockets.h"
#include "terminal_color.h"

namespace http
{

class loopwebsockets : public websockets_api
{
  public:

    // isco/isloopco 必须在构造时决定，server.cpp 构造后立即判断
    loopwebsockets(unsigned int m, unsigned int g) : websockets_api(8, m, g, 0)
    {
        // 保持同步版（isco=false → onopen/onmessage 同步分支；isloopco=false → run_loop 被定时线程调用）
        isco = false;
        isloopco = false;
    }
    ~loopwebsockets() { std::cout << "~loopwebsockets" << std::endl; }

  public:
    void onopen() override
    { 
        // isco 已由构造函数决定，onopen 不再改
        loop_num = 8; 
        std::cout << "onopen" << std::endl; 
    }

    asio::awaitable<void> async_onopen() override
    { 
        loop_num = 8; 
        std::cout << "async_onopen" << std::endl; 
        co_return;
    }

    void onclose() override
    {
        isclose = true;
        std::cout << "onclose" << std::endl; 
    }

    asio::awaitable<void> async_onclose() override
    { 
        std::cout << "async_onclose" << std::endl; 
        co_return;
    }

    void onpong() override {}
    void run_loop() override
    {
        if (session_sock)
        {
            std::cout << "timeloop:" << std::endl;
            // 统一走 send() → 环，不再直接 post_write
            send("test run_loop");

            if (loop_num == 4)
            {
                loop_num = 0;
                return;
            }
            loop_num--;
        }
        else
        {
            isclose = true;
            loop_num = 0;
            std::cout << "session_sock is die!" << std::endl;
        }
    }
    asio::awaitable<void> async_run_loop() override
    {
        if (session_sock)
        {
            std::cout << "async async_run_loop" << std::endl;
            // 统一走 send() → 环，不再直接 async_send_writer
            send("test async_run_loop");
            if (loop_num == 4)
            {
                loop_num = 0;
                co_return;
            }
            loop_num--;
        }
        else
        {
            isclose = true;
            loop_num = 0;
            std::cout << "session_sock is die!" << std::endl;
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
    void onmessage() override
    {
        auto self = shared_from_this();
        std::unique_lock<std::mutex> lock(content_list_mutex);
        if(content_list.empty())
        {
            return;
        }
        auto msg = std::move(content_list.front());
        content_list.pop_front();
        lock.unlock();
        
        // 统一走 send() → 环，不再直接 send_writer
        self->send(msg.value);
        return;
    }
 
};

}// namespace http
