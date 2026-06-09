#include <iostream>
#include <memory>
#include <string_view>

#include "orm.h"
#include "websockets.h"

namespace http
{

class loopwebsockets : public websockets_api
{
  public:

    loopwebsockets(unsigned int m, unsigned int g) : websockets_api(8, m, g, 0) {}
    ~loopwebsockets() { std::cout << "~loopwebsockets" << std::endl; }

  public:
    void onopen() override
    { 
        isco=true;
        loop_num = 8; 
        std::cout << "onopen" << std::endl; 
    }

    asio::awaitable<void> async_onopen() override
    { 
        isco=true;
        loop_num = 8; 
        std::cout << "async_onopen" << std::endl; 
        co_return;
    }

    void onclose() override
    {
        isclose = true;
        std::cout << "onclose" << std::endl; 
    }
    void onpong() override {}
    void run_loop() override
    {
        if (session_sock)
        {
            std::cout << "timeloop:" << std::endl;
            std::string aa = "test run_loop";
            std::string outhello;
            ws_parse->make_ws_text(aa, outhello);
            session_sock->send_writer(outhello);

            //   peer->send(aa);
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
            std::cout << "async timeloop:" << std::endl;
            std::string aa = "test async_run_loop";
            std::string outhello;
            ws_parse->make_ws_text(aa, outhello);
            co_await session_sock->co_send_writer(outhello);
            //   peer->send(aa);
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
    void onfiles() override { std::cout << "--------onfiles:--------"  << std::endl; }
    asio::awaitable<void> async_onfiles() override
    { 
        std::cout << "--------onfiles:--------" << std::endl;
        co_return; 
    }

    asio::awaitable<void> async_onmessage() override 
    {
        std::unique_lock<std::mutex> lock(mtx_list_lock);
        if(content_list.empty())
        {
            co_return;
        }

        auto process_data = std::move(content_list.front());
        content_list.pop_front();
        lock.unlock();
        
        std::string outhello;
        ws_parse->make_ws_text(process_data.value, outhello);
        co_await session_sock->co_send_writer(outhello);
        co_return;
    }
    void onmessage() override
    {
        std::unique_lock<std::mutex> lock(mtx_list_lock);
        if(content_list.empty())
        {
            return;
        }

        auto process_data = std::move(content_list.front());
        content_list.pop_front();
        lock.unlock();
        
        std::string outhello;
        ws_parse->make_ws_text(process_data.value, outhello);
        session_sock->send_writer(outhello);
        return;
    }
 
};

}// namespace http
