#ifndef RPC_PARSE_H
#define RPC_PARSE_H

#include "request.h"
#include "httppeer.h"

namespace http
{
// RPC 协议解析：解析请求行 / KV 头 / body，并构建响应帧。
// client_rpc_loop 每读到一块数据就喂给 process/process_append，
// 解析完成（isfinish）后交给路由分发，再由 build_header 生成响应。
//
// 线格式：'rpc ' 前缀 + 请求行（路径，可带 ?k=v&… 查询，0x0A 结束）
// + KV 段（每行 "key 0x20 2字节大端值长 值 [0x0A]"；HOST/cookie/size
// 是保留字，其余 key 原样进 peer->post）+ 空行 + body（二进制，长度由
// size 声明；超长 body 落 temp_path 临时文件）。
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
    void process_line(const unsigned char *buffer, unsigned int buffersize);
    void after_line(const unsigned char *buffer, unsigned int buffersize);

    void async_send_error();
    void build_header();

  public:
    bool isfinish                  = false;
    bool isbegin                   = false;
    bool isbody                    = false;
    bool iserror                   = false;
    bool ischunked                 = false;
    bool is_send                   = false;
    unsigned char cur_process_type = 0;
    unsigned int offsetnum         = 0;
    int val_size                   = 0;
    long long body_size            = 0;
    long long content_size         = 0;
    long long body_sub             = 0;
    unsigned int line_bytes        = 0;// 请求行累计字节（上限 CONST_RPC_LINE_MAX）
    unsigned int kv_num            = 0;// 本请求 KV 段条数（上限 CONST_RPC_KV_MAX_ITEMS）
    unsigned long long kv_bytes    = 0;// 本请求 KV 段累计字节（上限 CONST_RPC_KV_MAX_BYTES）
    bool has_host                  = false;
    bool has_size                  = false;

    std::string read_key;
    std::string read_value;
    std::string read_temp_file;
    std::string status_msg;
    std::string send_content;
    std::shared_ptr<httppeer> peer;
    std::unique_ptr<std::FILE, int (*)(FILE *)> uprawfile;
};
}// namespace http
#endif
