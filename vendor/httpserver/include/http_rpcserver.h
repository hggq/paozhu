#ifndef HTTP_RPC_H
#define HTTP_RPC_H

#include "request.h"
#include "httppeer.h"

namespace http
{
    class rpc_server : public std::enable_shared_from_this<rpc_server>
    {
    public:
        rpc_server();
        void reset();
        void set_chunk(bool c);
        void process_headkv();
        void process_body(const unsigned char *buffer, unsigned int buffersize);
        void process_value(const unsigned char *buffer, unsigned int buffersize);
        void process_parameter(const unsigned char *buffer, unsigned int buffersize);
        void process(const unsigned char *buffer, unsigned int buffersize);
        void process_append(const unsigned char *buffer, unsigned int buffersize);

        void async_send_error();
        asio::awaitable<void> async_send();
        void build_header();
        void send_header();
        void send_body();
    public:
        bool isfinish=false;
        bool isbegin =false;
        bool isbody =false;
        bool iserror = false;
        bool ischunked = false;
        bool is_send_header = false;
        bool is_ostream = false;
        bool is_send = false;
        unsigned char cur_process_type = 0;
        unsigned int offsetnum = 0;
        int val_size = 0;
        long long body_size = 0;
        long long content_size = 0;
        long long body_sub = 0;

        std::string read_key;   //read temp var in the header
        std::string read_value;
        std::string read_temp_file;
        std::string status_msg;
        std::string send_content;
        std::shared_ptr<httppeer> peer;
        std::unique_ptr<std::FILE, int (*)(FILE *)> uprawfile;
    };
}
#endif