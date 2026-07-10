#ifndef __HTTP2_PARSE_H
#define __HTTP2_PARSE_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>

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
#include <atomic>
#include <string_view>

#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <cstring>

#include "http2_frame.h"
#include "client_session.h"
#include "httppeer.h"

#ifdef WIN32
#define stat _stat
#endif

namespace http
{

union HTTP2_HEADER_FLAG
{
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char END_STREAM : 1;
        unsigned char e3 : 1;
        unsigned char END_HEADERS : 1;
        unsigned char PADDED : 1;
        unsigned char e4 : 1;
        unsigned char PRIORITY : 1;
    };
    char value;
};

//0x01
struct HTTP2_HEADER_FRAME_T
{
    unsigned int length      = 0;
    unsigned char frame_type = 0;
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char END_STREAM : 1;
        unsigned char e3 : 1;
        unsigned char END_HEADERS : 1;
        unsigned char PADDED : 1;
        unsigned char e4 : 1;
        unsigned char PRIORITY : 1;

    } flags;
    unsigned int stream_id         = 0;
    unsigned char padded_length    = 0;
    unsigned int stream_dependency = 0;
    unsigned char priority_weight  = 0;
    std::string content;
};

//0x00
struct HTTP2_DATA_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char e3 : 1;
        unsigned char e4 : 1;
        unsigned char e5 : 1;
        unsigned char PADDED : 1;
        unsigned char e6 : 1;
        unsigned char END_STREAM : 1;
    } flags;
    unsigned int stream_id;
    unsigned char padded_length;
    std::string content;
};

//0x03
struct HTTP2_RST_STREAM_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    unsigned char flags;
    unsigned int stream_id;
    unsigned int error_code;
};

//0x04
struct HTTP2_SETTING_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char e3 : 1;
        unsigned char e4 : 1;
        unsigned char e5 : 1;
        unsigned char PADDED : 1;
        unsigned char e6 : 1;
        unsigned char END_STREAM : 1;
    } flags;
    unsigned int stream_id;
    std::string content;
};

//0x05
struct HTTP2_PUSH_PROMISE_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char e3 : 1;
        unsigned char e4 : 1;
        unsigned char PADDED : 1;
        unsigned char END_HEADERS : 1;
        unsigned char e6 : 1;
        unsigned char e7 : 1;
    } flags;
    unsigned int stream_id;
    unsigned char padded_length;
    std::string content;
};
//0x06
struct HTTP2_PING_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    unsigned char flags;
    unsigned int stream_id;
    unsigned char data;
};
//0x07
struct HTTP2_GOWAY_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    unsigned char flags;
    unsigned int stream_id;
    unsigned int last_stream_id;
    unsigned int error_code;
    std::string content;
};

//0x08
struct HTTP2_WINDOW_UPDATE_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    unsigned char flags;
    unsigned int stream_id;
    unsigned int window_size_increment;
};

//data pack
struct HTTP2_PACK_DATA_T
{
    unsigned char frame_type = 0;
    unsigned char flags      = 0;
    unsigned int length      = 0;
    unsigned int stream_id   = 0;
    std::string subpad;
    std::string payload;
};

struct HTTP2_POST_DATA_T
{
    bool isfile                   = false;
    bool end_stream               = false;
    unsigned char posttype        = 0;
    unsigned char mach_pos        = 0;
    unsigned int stream_id        = 0;
    unsigned int field_offset     = 0;
    unsigned long long exp_length = 0;//需要的长度
    unsigned long long cur_length = 0;//当前长度
    std::string_view content;
    std::string chartset;
    std::string mimetype;
    std::string boundary;
    std::string pre_content;//为匹配剩余的字节
    std::string filename;
    std::string field_name;//post name;
    std::string field_item;//post value
    std::string temp_filename;
    std::unique_ptr<std::FILE, int (*)(FILE *)> fp;
    std::shared_ptr<httppeer> peer;

    HTTP2_POST_DATA_T() : fp(nullptr, std::fclose) {};
};

class http2parse
{

  public:
    http2parse()
    {
    }

    ~http2parse()
    {
    }
    void setsession(std::shared_ptr<client_session>);
    void readheaders(const HTTP2_PACK_DATA_T &temp_pack_data);
    // void setstaticheader(const unsigned char, unsigned int, unsigned int);
    void readsetting(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readpriority(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readpostdata(const HTTP2_PACK_DATA_T &temp_pack_data);
    void reset_uploadfile(HTTP2_POST_DATA_T &temp_pack_data);
    void post_data_process(HTTP2_POST_DATA_T &temp_pack_data, unsigned char islast_pack);
    void multipart_post_file_field(HTTP2_POST_DATA_T &temp_post_data);
    void post_form_to_postfield(std::string_view form_post_name, std::string_view form_post_value, std::shared_ptr<httppeer> steam_httppeer);
    void post_www_form_urlencoded(HTTP2_POST_DATA_T &temp_post_data);
    void post_multipart_formdata(HTTP2_POST_DATA_T &temp_post_data, unsigned char islast_pack);
    void post_multipart_itemcontent_append(HTTP2_POST_DATA_T &temp_post_data);
    void post_multipart_itemcontent(HTTP2_POST_DATA_T &temp_post_data, bool);
    void readrawfileformdata(HTTP2_POST_DATA_T &temp_pack_data, unsigned char islast_pack);
    void readgoaway(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readping(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readrst_stream(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readwinupdate(const HTTP2_PACK_DATA_T &temp_pack_data);

    void headertype1(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);
    void headertype2(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);
    void headertype3(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);
    void headertype4(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);

  public:
    void headers_parse(const HTTP2_HEADER_FRAME_T &, std::shared_ptr<httppeer>);
    void process(const unsigned char *buffer, unsigned int buffersize);
    void data_process();
    bool header_host_process(const std::string &header_value, std::shared_ptr<httppeer>);
    void getacceptencoding(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void header_process(const std::string &, const std::string &, int, std::shared_ptr<httppeer>);
    void cookie_process(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void getacceptlanguage(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void range_process(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void path_process(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void getaccept(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void getcontenttype(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void getifnonematch(const std::string &, const std::string &, std::shared_ptr<httppeer>);

    void callposttype(const std::string &header_value, std::shared_ptr<httppeer>, HTTP2_POST_DATA_T &pd);
    void clsoesend();
    void procssparamter(std::string_view buffer_key, std::string_view buffer_value, std::shared_ptr<httppeer> steam_httppeer);
    void process_pack();
    void read_pack_data(const unsigned char *buffer, unsigned int buffersize);

  public:
    unsigned int error      = 0;
    unsigned int readoffset = 0;

    unsigned int steam_count = 0;
    unsigned int isfinsish   = 0;

    struct http2_setting_t setting_data;

    //std::vector<std::pair<std::string, std::string>> header_lists;
    std::list<std::pair<std::string, std::string>> dynamic_lists;
    // struct http2_goaway_t goaway_data;
    // unsigned long long content_length;
    bool ispost                            = false;
    std::atomic_bool task_in               = false;
    std::atomic_bool need_wakeup_send_data = false;
    std::shared_ptr<client_session> peer_session;
    // const char *hextostr = "0123456789ABCDEF";
    std::atomic<unsigned int> window_update_recv_num;
    std::map<unsigned int, std::shared_ptr<httppeer>> http_data;
    std::map<unsigned int, std::weak_ptr<httppeer>> http_data_weak;
    std::map<unsigned int, HTTP2_POST_DATA_T> http_post_data;

    //new data
    std::map<unsigned int, HTTP2_HEADER_FRAME_T> http2_header_recvs;
    std::map<unsigned int, HTTP2_DATA_FRAME_T> http2_data_recvs;
    struct HTTP2_PACK_DATA_T pack_data;

    std::queue<unsigned int> stream_list;

    std::mutex http2loop_mutex;
    std::atomic<bool> http2_loop_in = false;
};
}// namespace http
#endif
