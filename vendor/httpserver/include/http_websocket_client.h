#ifndef HTTP_WEBSOCKETS_CLIENT_H
#define HTTP_WEBSOCKETS_CLIENT_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/strand.hpp>
#include "request.h"
#include "datetime.h"

namespace http
{
    struct websocket_parameter_t
    {
        std::string name;
        std::string value;
    };
    enum WS_FRAME
    {
        APP    = 0x00,
        APP_TEXT   = 0x01,
        APP_BINARY = 0x02,
        TEXT   = 0x81,
        BINARY = 0x82,
        CLOSE = 0x88,
        PING = 0x89,
        PONG = 0x8A
    };
 
    class websocket_client : public std::enable_shared_from_this<websocket_client>
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
        websocket_client();
        ~websocket_client();
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
        std::string make_http_header();
        bool process_handshake(unsigned char *read_data, unsigned int buffersize);
        void process_header(unsigned char *read_data, unsigned int data_bein, unsigned int data_end);
        asio::awaitable<bool> websocket_handshake();

        std::string make_pong();
        std::string make_ping();

        int make_ws_data(char *msg,unsigned int msgLen, std::string &outBuf);
        int make_ws_data(std::string_view msg, std::string &outBuf);
        int make_ws_text(char *msg, unsigned int msgLen, std::string &outBuf);
        int make_ws_text(std::string_view msg, std::string &outBuf);  
        int make_ws_header(unsigned int len, std::string &header, unsigned char t);  
        void make_mark();
        
        unsigned long long first_parsedata(unsigned char *inputdata, unsigned int buffersize);
        unsigned long long append_parsedata(unsigned char *inputdata, unsigned int buffersize);
        unsigned int process_data(unsigned char *inputdata, unsigned int buffersize);
        asio::awaitable<void> async_recv_finish();
        void reset_recv_status();

        bool add_client_task_loop();
    public:
        bool iserror = false;
        bool isssl = false;
        bool isbody =false;
        bool isfinish = false;
        bool iswait_exit = false;
        bool isco = false;
        unsigned char ready_state = 0;
        unsigned char exptime = 0;
        unsigned char cur_process_type = 0;
        unsigned char key_str[16];
        unsigned char mask_key[4];
        unsigned int offsetnum = 0;
        unsigned int durtime=8;
        unsigned int port=0;
        unsigned int val_size = 0;
        unsigned int myid = 0;
        unsigned int groupid = 0;
        std::atomic_flag socket_read_lock = ATOMIC_FLAG_INIT;
        std::atomic<unsigned int> timeout_end = 0;

        std::string url;
        std::string host;
        std::string error_msg;

        unsigned char *data = nullptr;
        struct ws_pack_data
        {
          bool isfile = false;
          bool isfinish = false;
          unsigned char fin;
          unsigned char opcode = 0;
          unsigned char mask;
          unsigned char mask_key[4];
          unsigned long long length = 0;
          unsigned long long read_length = 0;
          std::string content;
        } recv_data;
        std::vector<websocket_parameter_t> parameter;
        std::shared_ptr<asio::ip::tcp::socket> sock                       = {nullptr};
        std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsock = {nullptr};
        std::shared_ptr<asio::ssl::context> ssl_context                   = {nullptr};
        asio::error_code ec;
        asio::strand<asio::io_context::executor_type> strand_;

        std::function<void(std::shared_ptr<websocket_client>, unsigned int)> run_loop_fun = nullptr;
        std::function<asio::awaitable<void>(std::shared_ptr<websocket_client>, unsigned int)> async_run_loop_fun = nullptr;

        std::function<void(std::shared_ptr<websocket_client>)> dur_time_loop_fun = nullptr;
        std::function<asio::awaitable<void>(std::shared_ptr<websocket_client>)> async_dur_time_loop_fun = nullptr;
        std::function<asio::awaitable<void>(std::shared_ptr<websocket_client>)> async_recv_finish_fun= nullptr;
    };

    std::map<std::string,std::shared_ptr<websocket_client>> &get_http_websocket_obj();
}
#endif