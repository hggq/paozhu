#ifndef __HTTP_OBJ_SOCKET_API_H
#define __HTTP_OBJ_SOCKET_API_H

#include <vector>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include "request.h"
#include "httppeer.h"
#include "client_session.h"

namespace http
{
    struct socket_api_data_t
    {
        std::string name;
        std::string value;
    };
    class socket_api : public std::enable_shared_from_this<socket_api> 
    {
    public:
        socket_api(unsigned int t, unsigned int m, unsigned int g, unsigned char s, std::shared_ptr<client_session> s_sock) : state(s), timeloop_num(t), myid(m), groupid(g),session_sock(s_sock){}
        virtual void on_open()                    = 0;
        virtual asio::awaitable<void> async_on_open()= 0;
        virtual void on_close()                    = 0;
        virtual asio::awaitable<void> async_on_close()= 0;
        virtual asio::awaitable<void> async_on_message(const std::string &buffer)
        {
            unsigned int readoffset = 0;
            for(; readoffset < buffer.size(); readoffset++)
            {
                if(buffer[readoffset]=='\n')
                {
                    break;
                }
            }
            co_return;
        }
        virtual void run_loop()                  = 0;
        virtual asio::awaitable<void> async_run_loop()                  = 0;
        virtual ~socket_api()
        {
            isclose = true;
        };
public:
        bool isclose = false;
        bool isfinish = false;
        bool isco = false;
        bool isloopco = false;
        unsigned char state;
        unsigned int timeloop_num;
        unsigned int myid=0;
        unsigned int groupid = 0;
        unsigned int current_time=0;

        std::string error_msg;
        std::string url;
        std::string host;
        std::shared_ptr<client_session> session_sock;
        std::vector<socket_api_data_t> header;
    };
    typedef std::map<std::string, std::function<std::shared_ptr<socket_api>(unsigned int myid_, unsigned int groupid_, std::shared_ptr<client_session>)>> HTTP_SOCKET_REG;
    HTTP_SOCKET_REG &get_http_socket_reg();

}

#endif