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
            ws_parse->makeWSText(aa, outhello);
            session_sock->send_data(outhello);

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
            ws_parse->makeWSText(aa, outhello);
            session_sock->send_data(outhello);

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
    void onfiles(std::string_view filename) override { std::cout << "--------onfiles:--------" << filename << std::endl; }
    asio::awaitable<void> async_onfiles(std::string_view filename) override
    { 
        std::cout << "--------onfiles:--------" << filename << std::endl;
        co_return; 
    }

    asio::awaitable<void> async_onmessage(std::string_view data) override 
    {
        std::string outhello;
        ws_parse->makeWSText(data, outhello);
        co_await session_sock->co_send_writer(outhello);
        co_return;
    }
    void onmessage(std::string_view data) override
    {
        if (session_sock)
        {
            std::string outhello;
            ws_parse->makeWSText(data, outhello);
            session_sock->send_data(outhello);
        }
    }
 
};

}// namespace http
