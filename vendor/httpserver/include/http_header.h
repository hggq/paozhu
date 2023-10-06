#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <filesystem>

#include <string_view>
#include <unistd.h>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include "request.h"
#define RECV_WINDOW_UPDATE_NUM 16711680

namespace http
{
struct headstate_t
{
    bool gzip              = false;
    bool deflate           = false;
    bool br                = false;
    bool avif              = false;
    bool webp              = false;
    bool h2c               = false;
    bool keepalive         = false;
    bool websocket         = false;
    bool upgradeconnection = false;
    bool rangebytes        = false;

    unsigned char version;
    unsigned int port;
    unsigned char language[8]          = {0};
    unsigned long long ifmodifiedsince = 0;
    unsigned long long rangebegin      = 0;
    unsigned long long rangeend        = 0;
};
struct websocket_t
{
    bool deflate           = false;
    bool permessagedeflate = false;
    bool perframedeflate   = false;
    bool deflateframe      = false;
    bool isopen            = false;
    unsigned char version;
    std::string key;
    std::string ext;
};
struct poststate_t
{
    unsigned long long content_length;
    unsigned char posttype = 0;
    std::string chartset;
    std::string type;
    std::string xrequestedwith;
    std::string boundary;
};
struct uploadfile_t
{
    std::string name;
    std::string filename;
    std::string tempfile;
    std::string type;
    unsigned int size;
    unsigned char error;
};
#ifdef _WIN32
#undef DELETE
#endif
enum HEAD_METHOD
{
    UNKNOW,
    GET,
    POST,
    OPTIONS,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    CONNECT,
};

struct httpinfo
{
    std::string host;
    std::string url;
    std::string urlpath;
    std::string querystring;

    std::string type;
    std::string chartset;
    std::string boundary;
    std::string etag;
    std::string accept_language;
    std::map<std::string, std::string> header;
    http::OBJ_VALUE get;
    http::OBJ_VALUE post;
    http::OBJ_VALUE files;
    http::OBJ_VALUE json;
    std::vector<std::string> pathinfos;
    unsigned char posttype      = 0;
    unsigned char changetype    = 0;
    unsigned char postfieldtype = 0;
    bool issend                 = false;
    bool isfinish               = false;
    bool isrange                = false;
    bool isclose                = false;// rst_stream flag

    bool gzip              = false;
    bool deflate           = false;
    bool br                = false;
    bool avif              = false;
    bool webp              = false;
    bool keepalive         = false;
    bool websocket         = false;
    bool upgradeconnection = false;

    //    struct
    //    {
    //       unsigned char br:1;
    //       unsigned char deflate:1;
    //       unsigned char gzip:1;
    //       unsigned char avif:1;
    //       unsigned char webp:1;
    //       unsigned char websocket:1;
    //       unsigned char upgradeconnection:1;
    //       unsigned char isrange:1;

    //    }state;

    long long range_begin = 0;
    long long range_end   = 0;
    long long content_length;
};
extern std::map<unsigned int, std::string> http_status_static_table;
std::string make_header_etag(unsigned long long, unsigned long long);
// bool make_file_mime(std::string &, const std::string &filename);

}// namespace http
#endif