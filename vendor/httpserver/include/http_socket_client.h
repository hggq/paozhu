#ifndef HTTP_SOCKETS_CLIENT_H
#define HTTP_SOCKETS_CLIENT_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/strand.hpp>
#include "request.h"
#include "httppeer.h"

namespace http
{
    struct socket_parameter_t
    {
        std::string name;
        std::string value;
    };
    
 
    class socket_client : public std::enable_shared_from_this<socket_client>
    {
    public:
            void timeout(unsigned int t) { exptime = t; };
            unsigned int timeout() { return exptime; };
            void reset_timeout()
            {
                timeout_end.store(timeid() + exptime);
            };
            void reset_connect_timeout()
            {
                timeout_end.fetch_add(exptime, std::memory_order_relaxed);
                exptime = 30;
            };
            void set_timeout(unsigned int t)
            {
                timeout_end.store(timeid() + t);
            };
            unsigned int get_timeout()
            {
                return timeout_end.load();
            };

    public:
        socket_client();
        ~socket_client();
        void reset();
        void set_header(std::string_view name,std::string_view value);
        void set_host(std::string_view name);
        void set_port(unsigned int);
        void set_url(std::string_view name);
        asio::awaitable<bool> async_init_https_sock();
        asio::awaitable<bool> async_init_http_sock();

        bool connect(std::string_view url,unsigned int time_out_num=0);
        bool connect();
        asio::awaitable<bool> async_connect(std::string_view url,unsigned int time_out_num=0);
        asio::awaitable<bool> async_connect();

        asio::awaitable<unsigned int> async_write(unsigned char *data, unsigned int buffersize);
        asio::awaitable<unsigned int> async_write(std::string_view value);
        asio::awaitable<unsigned int> async_read(unsigned char *data,unsigned int buffersize);
        asio::awaitable<unsigned int> async_read(std::string &data);

        void run_loop();
        asio::awaitable<void> async_run_loop();

        void dur_time_loop();
        asio::awaitable<void> async_dur_time_loop();

        void make_send();
        void close_connect();    
    public:
        bool iserror = false;
        bool isssl = false;
        bool isbody =false;
        bool isfinish = false;
        bool iswait_exit = false;
        bool isco = false;
        std::atomic_flag socket_read_lock = ATOMIC_FLAG_INIT;
        unsigned char exptime = 0;
        unsigned char cur_process_type = 0;
        
        unsigned int offsetnum = 0;
        unsigned int durtime=0;
        unsigned int port=0;
        unsigned int val_size = 0;
        unsigned int myid = 0;
        unsigned int groupid = 0;
        std::atomic<unsigned int> timeout_end = 0;

        std::string url;
        std::string host;
        std::string error_msg;

        unsigned char *data = nullptr;
 
        std::vector<socket_parameter_t> parameter;
        std::shared_ptr<asio::ip::tcp::socket> sock                       = {nullptr};
        std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsock = {nullptr};
        std::shared_ptr<asio::ssl::context> ssl_context                   = {nullptr};
        asio::error_code ec;
        asio::strand<asio::io_context::executor_type> strand_;
        std::function<void(std::shared_ptr<socket_client>, unsigned int)> run_loop_fun = nullptr;
        std::function<asio::awaitable<void>(std::shared_ptr<socket_client>, unsigned int)> async_run_loop_fun = nullptr;
        
        std::function<void(std::shared_ptr<socket_client>)> dur_time_loop_fun = nullptr;
        std::function<asio::awaitable<void>(std::shared_ptr<socket_client>)> async_dur_time_loop_fun = nullptr;

    };

    std::map<std::string,std::shared_ptr<socket_client>> &get_http_socket_obj();
}
#endif