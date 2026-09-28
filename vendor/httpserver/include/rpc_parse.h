#ifndef RPC_PARSE_H
#define RPC_PARSE_H

#include "request.h"
#include "httppeer.h"

namespace http
{
    // RPC 协议解析：解析请求行 / KV 头 / body，并构建响应帧。
    // client_rpc_loop 每读到一块数据就喂给 process/process_append，
    // 解析完成（isfinish）后交给路由分发，再由 build_header 生成响应。
    class rpc_parse : public std::enable_shared_from_this<rpc_parse>
    {
    public:
        rpc_parse();
        void reset();
        void set_chunk(bool c);
        void process_headkv();
        void process_body(const unsigned char *buffer, unsigned int buffersize);
        void process_value(const unsigned char *buffer, unsigned int buffersize);
        void process_parameter(const unsigned char *buffer, unsigned int buffersize);
        void process(const unsigned char *buffer, unsigned int buffersize);
        void process_append(const unsigned char *buffer, unsigned int buffersize);

        void async_send_error();
        void build_header();
    public:
        bool isfinish=false;
        bool isbegin =false;
        bool isbody =false;
        bool iserror = false;
        bool ischunked = false;
        bool is_send = false;
        unsigned char cur_process_type = 0;
        unsigned int offsetnum = 0;
        int val_size = 0;
        long long body_size = 0;
        long long content_size = 0;
        long long body_sub = 0;

        std::string read_key;
        std::string read_value;
        std::string read_temp_file;
        std::string status_msg;
        std::string send_content;
        std::shared_ptr<httppeer> peer;
        std::unique_ptr<std::FILE, int (*)(FILE *)> uprawfile;
    };
}
#endif
