#include <iostream>
#include <memory>
#include <string_view>

#include "orm.h"
#include "httppeer.h"
#include "http_socket.h"
#include "terminal_color.h"
 
namespace http
{

class my_test_socket : public socket_api
{
  public:

    // 构造参数决定(Construction parameters determine) isco/isloopco
    my_test_socket(unsigned int m, unsigned int g, std::shared_ptr<client_session> s_sock,
                   bool co_style = false, bool loop_co = false)
        : socket_api(7, m, g, 0, s_sock)
    {
        isco = co_style;           // socket_api 暂无同步 on_message；当前 co_style 保留留作未来扩展
        isloopco = loop_co;        // false → server 调 run_loop()；true → co_spawn(async_run_loop)
    }
    ~my_test_socket()
    {
      DEBUG_LOG(" ~my_test_socket ");
      isclose = true;
    };

  public:
    void on_open() override { DEBUG_LOG(" onopen "); }
    asio::awaitable<void> async_on_open() override { DEBUG_LOG(" async_on_open "); co_return; }
    void on_close() override 
    {
      DEBUG_LOG(" onclose "); 
      isclose = true;
    }
    
    asio::awaitable<void> async_on_close() override 
    {
      DEBUG_LOG(" async_on_close "); 
      isclose = true;
      co_return;
    }

    asio::awaitable<void> async_on_message(const std::string &buffer) override
    {
      auto self = shared_from_this();   // Phase 1 #14
      DEBUG_LOG(" async_on_message:%s", buffer.c_str()); 
      co_await self->session_sock->async_send_writer(buffer);
      co_return;
    }
    void run_loop() override
    {
      DEBUG_LOG(" run_loop "); 
    }
    asio::awaitable<void> async_run_loop() override
    {
      auto self = shared_from_this();   // Phase 1 #14
      if(self->session_sock)
      {
        std::string content="server socket loop send";
        co_await self->session_sock->async_send_writer(content);
        self->session_sock->time_limit.store(timeid());
      }
      DEBUG_LOG(" async_run_loop "); 
      co_return;
    }
};

}// namespace http