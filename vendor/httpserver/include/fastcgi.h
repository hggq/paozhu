#ifndef FRAME_FASTCGI_H
#define FRAME_FASTCGI_H

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <cstdarg>
#include <cstring>
#include <stdexcept>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>
#include <asio.hpp>

#include "client_session.h"
#include "httppeer.h"

namespace http
{

typedef struct
{
    unsigned char version;// fastCGI协议号
    unsigned char type;   // fastCGI 消息类型
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
} FASTCGI_Record;

struct FASTCGI_Header
{
    unsigned char version;
    unsigned char type;
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
};

// 允许传送的最大数据 65536
#define FASTCGI_MAX_LENGTH 0xffff
// fast_cgi header长度
#define FASTCGI_HEADER_LENGTH 8
// fast_cgi 版本
#define FASTCGI_VERSION 1

#define FASTCGI_BEGIN_REQUEST 1
#define FASTCGI_ABORT_REQUEST 2
#define FASTCGI_END_REQUEST 3
#define FASTCGI_PARAMS 4
#define FASTCGI_STDIN 5
#define FASTCGI_STDOUT 6
#define FASTCGI_STDERR 7
#define FASTCGI_DATA 8
#define FASTCGI_GET_VALUES 9
#define FASTCGI_GET_VALUES_RESULT 10
#define FASTCGI_UNKNOWN_TYPE 11
#define FASTCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

#define FASTCGI_KEEP_CONN 1
#define FASTCGI_RESPONDER 1
#define FASTCGI_AUTHORIZER 2
#define FASTCGI_FILTER 3

struct FASTCGI_BeginRequestBody
{
    unsigned char roleB1;
    unsigned char roleB0;
    unsigned char flags;
    unsigned char reserved[5];
};

struct FASTCGI_BeginRequestRecord
{
    FASTCGI_Header header;
    FASTCGI_BeginRequestBody body;
};

struct FASTCGI_EndRequestBody
{
    unsigned char appStatusB3;
    unsigned char appStatusB2;
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocolStatus;
    unsigned char reserved[3];
};

class fastcgi
{
  public:
    void make_Header(FASTCGI_Header &, int type, int requestId, int contentLength, int paddingLength);
    void make_BeginRequestBody(FASTCGI_BeginRequestBody &, int role, int keepConnection);
    asio::awaitable<bool> co_init_http_sock();
    asio::awaitable<void> async_send();
    asio::awaitable<void> send_postbody(std::shared_ptr<httppeer> peer);
    asio::awaitable<void> send_endheader();
    asio::awaitable<void> send_startheader();
    asio::awaitable<void> send_exit(std::shared_ptr<httppeer> peer);
    std::string pack_length(unsigned int, unsigned int);

  public:
    std::string host;
    std::string port;
    std::string request_url;
    //std::string request_query;
    std::string document_root;
    // FASTCGI_Header header_data;
    // FASTCGI_BeginRequestBody request_body{};
    int request_id;
    bool issock = false;
    //unsigned char method_type = 0;

    std::function<void(const std::string &)> add_error_msg;

    std::shared_ptr<asio::ip::tcp::socket> sock                     = {nullptr};
    std::shared_ptr<asio::local::stream_protocol::socket> socklocal = {nullptr};
    std::weak_ptr<httppeer> peer_ptr;
    std::string error_msg;
    asio::error_code ec;
    asio::io_context *server_ioc;
};
}// namespace http
#endif
