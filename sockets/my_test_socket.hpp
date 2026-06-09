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

    my_test_socket(unsigned int m, unsigned int g, std::shared_ptr<client_session> s_sock) : socket_api(7, m, g, 0, s_sock) {}
    ~my_test_socket()
    {
      DEBUG_LOG(" ~my_test_socket ");
      isclose = true;
      notify_timer_.cancel();
    };

  public:
    void on_open() override { DEBUG_LOG(" onopen "); }
    asio::awaitable<void> async_on_open() override { DEBUG_LOG(" async_on_open "); co_return; }
    void on_close() override 
    {
      DEBUG_LOG(" onclose "); 
      isclose = true;
      notify_timer_.cancel();
    }
    asio::awaitable<void> async_on_message(std::string &&buffer) override
    {
      co_await session_sock->async_send_writer(buffer);
      co_return;
    }
    void run_loop() override
    {

    }
    asio::awaitable<void> async_run_loop() override
    {
      if(session_sock)
      {
        std::string content="server socket loop send";
        co_await session_sock->async_send_writer(content);
        session_sock->time_limit.store(timeid());
      }
      
      co_return;
    }
};

}// namespace http