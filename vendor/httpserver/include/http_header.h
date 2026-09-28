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

#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

#include "request.h"

// 旧实现这里定义 RECV_WINDOW_UPDATE_NUM = 16711680，而本端广告的
// SETTINGS_INITIAL_WINDOW_SIZE 是 0xFFFFFF = 16777215，两者相差正好一个默认连接
// 窗口（65535），补窗口基线系统性偏小。现已统一到 cost_define.h：
//   CONST_HTTP2_LOCAL_INITIAL_WINDOW  每流窗口目标水位（= 广告值 0xFFFFFF）
//   CONST_HTTP2_DEFAULT_WINDOW        连接级窗口初值（RFC 固定 65535）
//   CONST_HTTP2_WINDOW_UPDATE_STEP    连接级窗口一次性抬升量（= 16711680）

namespace http
{
struct headstate_t
{
    bool gzip              = false;
    bool zstd              = false;
    bool deflate           = false;
    bool br                = false;
    bool avif              = false;
    bool webp              = false;
    bool h2c               = false;
    bool keepalive         = false;
    bool websocket         = false;
    bool upgradeconnection = false;
    bool rangebytes        = false;
    // M5 修复：后缀范围(bytes=-N) / 是否显式给出末端(bytes=N-M)。
    // 旧实现没有这两个标志，只能靠 "rangeend > 0" 判断，故 bytes=0-0 与 bytes=-N 都解析错误。
    bool range_suffix      = false;
    bool range_has_end     = false;
    bool accept_json       = false;
    bool accept_xml        = false;
    bool accept_html       = false;

    unsigned char version;
    unsigned int port;
    unsigned char language[8]          = {0};
    unsigned int ifmodifiedsince = 0;
    unsigned int ifunmodifiedsince = 0;
    unsigned long long rangebegin      = 0;
    unsigned long long rangeend        = 0;
};

// RFC 7233 首段 byte-range 解析；h1 与 h2 共用，两方言必须逐字节同行为。
// 成功解析时置 rangebytes / rangebegin / rangeend / range_suffix / range_has_end。
// 三种结果必须可区分：h1 语法不合法要回 400，而非 bytes 单位要当没有这个头；
// h2 没有「单条头错误」通道，语法不合法只能按 RFC 9110 §14.2 忽略该头。
enum class range_parse_t : unsigned char
{
    applied      = 0,
    not_bytes    = 1,
    syntax_error = 2
};
range_parse_t parse_range_header(std::string_view header_value, headstate_t &state);

struct websocket_t
{
    bool deflate           = false;
    bool permessagedeflate = false;
    bool perframedeflate   = false;
    bool deflateframe      = false;
    bool gzip              = false;
    bool zstd              = false;
    bool br                = false;

    unsigned char bits    = 0;
    unsigned char version = 0;
    std::string key;
    std::string ext;
};
struct poststate_t
{
    // unsigned long long content_length;
    // unsigned char posttype = 0;
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
    QUERY,
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
    http::obj_val get;
    http::obj_val post;
    http::obj_val files;
    http::obj_val json;
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