#ifndef HTTP_RPC_CLIENT_H
#define HTTP_RPC_CLIENT_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/strand.hpp>
#include "request.h"
#include "httppeer.h"

namespace http
{
    struct rpc_parameter_t
    {
        std::string name;
        std::string value;
    };
    
    struct rpc_page_info_t
    {
        bool isfile=false;
        bool ischunked=false;
        unsigned int status_code;
        unsigned long long page_size=0;
        unsigned long long content_length=0;
        unsigned long long read_size=0;
        std::string status_msg;
        std::vector<rpc_parameter_t> header;
        std::vector<rpc_parameter_t> cookie;
        std::string content;
        std::string tempfile;
        std::string content_type;
        std::string encode_type;
    };

    class rpc_client : public std::enable_shared_from_this<rpc_client>
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
        rpc_client();
        ~rpc_client();
        void reset();
        void close_connect();
        void set_header(std::string_view name,std::string_view value);
        void set_host(std::string_view name);
        void set_url(std::string_view name);
        void send();
        void process_headkv();
        asio::awaitable<void> async_send();
        asio::awaitable<void> async_send_data(std::string_view send_content);
        asio::awaitable<void> async_ssl_send_data(std::string_view send_content);

        void send_data(std::string_view send_content);
        void ssl_send_data(std::string_view send_content);

        bool init_https_sock();
        bool init_http_sock();
        asio::awaitable<bool> async_init_https_sock();
        asio::awaitable<bool> async_init_http_sock();

        asio::awaitable<unsigned int> async_write(unsigned char *data, unsigned int buffersize);
        asio::awaitable<unsigned int> async_write(std::string_view value);
        asio::awaitable<unsigned int> async_read(unsigned char *data,unsigned int buffersize);
        asio::awaitable<unsigned int> async_read(std::string &data);

        unsigned int write(unsigned char *data, unsigned int buffersize);
        unsigned int write(std::string_view value);
        unsigned int read(unsigned char *data,unsigned int buffersize);
        unsigned int read(std::string &data);

        void process(const unsigned char *buffer, unsigned int buffersize);
        void process_parameter(const unsigned char *buffer, unsigned int buffersize);
        void process_append(const unsigned char *buffer, unsigned int buffersize);
        void process_value(const unsigned char *buffer, unsigned int readnum);
        void process_body_chunk(const unsigned char *buffer, unsigned int readnum);
        void process_body(const unsigned char *buffer, unsigned int readnum);

        asio::awaitable<void> async_run_loop();
        void run_loop();

        void dur_time_loop();
        asio::awaitable<void> async_dur_time_loop();

        bool connect(std::string_view url,unsigned int time_out_num=0);
        bool connect();
        asio::awaitable<bool> async_connect(std::string_view url,unsigned int time_out_num=0);
        asio::awaitable<bool> async_connect();

    public:
        bool iserror = false;
        bool isssl = false;
        bool isbody =false;
        bool isfinish = false;
        bool iswait_exit =false;
        bool isco = false;
        bool isclose = false;
        unsigned char exptime = 0;
        unsigned char cur_process_type = 0;
        unsigned int durtime=0;
        unsigned int offsetnum = 0;
        unsigned int port=0;
        unsigned int val_size = 0;
        std::atomic<unsigned int> timeout_end = 0;
        std::atomic_flag socket_read_lock = ATOMIC_FLAG_INIT;

        std::string url;
        std::string host;
        std::string content_type;
        std::string content;
        std::string error_msg;
        std::string read_key;   //read temp var in the header
        std::string read_value;

        unsigned char* data = nullptr;
        long long content_size = 0;
        struct rpc_page_info_t page;
        std::vector<rpc_parameter_t> parameter;
        std::shared_ptr<asio::ip::tcp::socket> sock                       = {nullptr};
        std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsock = {nullptr};
        std::shared_ptr<asio::ssl::context> ssl_context                   = {nullptr};
        std::function<void(std::shared_ptr<rpc_client>)> chunk_process  = nullptr;
        std::unique_ptr<std::FILE, int (*)(FILE *)> uprawfile;
        asio::error_code ec;
        asio::strand<asio::io_context::executor_type> strand_;

        std::function<void(std::shared_ptr<rpc_client>,unsigned int)> run_loop_fun = nullptr;
        std::function<asio::awaitable<void>(std::shared_ptr<rpc_client>,unsigned int)> async_run_loop_fun = nullptr;

        std::function<void(std::shared_ptr<rpc_client>)> dur_time_loop_fun = nullptr;
        std::function<asio::awaitable<void>(std::shared_ptr<rpc_client>)> async_dur_time_loop_fun = nullptr;
    };
}
#endif