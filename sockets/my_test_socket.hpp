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

    my_test_socket(unsigned int m, unsigned int g) : socket_api(7, m, g, 0) {}
    ~my_test_socket() { DEBUG_LOG(" ~my_test_socket "); }

  public:
    void on_open() override { DEBUG_LOG(" onopen "); }
    void on_close() override { DEBUG_LOG(" onclose "); }
    asio::awaitable<void> async_on_message(const unsigned char *buffer, unsigned int readoffset, unsigned int readnum) override
    {
      for(; readoffset < readnum; readoffset++)
      {
          content.push_back(buffer[readoffset]);
      }
      co_await session_sock->co_send_writer(content);
      content.clear();
      co_return;
    }
    void run_loop() override
    {

    }
    asio::awaitable<void> async_run_loop() override
    {
      if(session_sock)
      {
        content="server socket loop send";
        co_await session_sock->co_send_writer(content);
        session_sock->time_limit.store(timeid());
      }
      
      co_return;
    }
};

}// namespace http