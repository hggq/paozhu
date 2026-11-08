/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file http_parse.hpp
 *  @date 2022-04-17
 *
 *  html protocol parse file
 *
 *
 */

#ifndef PROJECT_HTTP_PARSE_HPP
#define PROJECT_HTTP_PARSE_HPP

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

#include "cookie.h"
// #include "urlcode.hpp"
#include "request.h"
#include "client_session.h"
#include "httppeer.h"

namespace http
{

struct HTTP_POST_DATA_T
{
    bool isfile = false;
    unsigned int field_offset = 0;
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
    HTTP_POST_DATA_T() : fp(nullptr, std::fclose) {};
};
class httpparse
{
  public:
    httpparse();
    //void setsession(std::shared_ptr<client_session> peer);
    void setpeer(std::shared_ptr<httppeer> peer);
    unsigned long long header_valuetoint(std::string_view header_value);
    void procssparamter(std::string_view header_temp, std::string_view header_input);
    void methodprocess(std::string_view contentline);
    void callposttype(std::string_view buffer_value);
    void getcontenttype(std::string_view header_value);
    void getrange(std::string_view header_value);
    void process_header_line(std::string_view line_str);
    void getaccept(std::string_view header_value);
    void getacceptlanguage(std::string_view header_value);
    void getifunmodifiedsince(std::string_view header_value);
    void getifmodifiedsince(std::string_view header_value);
    void getifnonematch(std::string_view header_value);
    void getwebsocketextensions(std::string_view header_value);
    void getupgrade(std::string_view header_value);
    void getconnection(std::string_view header_value);
    void getacceptencoding(std::string_view header_value);
    void getcookie(std::string_view header_value);
    void getheaderhost(std::string_view header_value);
    void read_http_header_block(const unsigned char *buffer, unsigned int buffersize);
    void post_form_to_postfield(std::string_view form_post_name, std::string_view form_post_value);
    void post_www_form_urlencoded(std::string_view raw_post_urlencode_content);
    void read_rawfile_formdata(std::string_view raw_http_post_data);
    void multipart_post_file_field();
    void post_multipart_itemcontent_append();
    void post_multipart_itemcontent(bool);
    void reset_uploadfile();
    void post_multipart_formdata();
    void read_http_post_block(const unsigned char *buffer, unsigned int buffersize);
    void read_http_chunked_block(const unsigned char *buffer, unsigned int buffersize);
    void append_post_body(const unsigned char *data, unsigned int len, bool is_final);

    void process(const unsigned char *buffer, unsigned int buffersize);
    bool getfinish();
    void clear();

  public:
    bool isfinish_header = false;
    bool isfinish_url    = false;
    // H1 修复：是否已出现过 Content-Length（重复头是 CL.CL 走私的典型载体）
    bool has_content_length = false;
    // H3 补齐：命中有意不支持的方法（PUT/DELETE/TRACE/CONNECT），由 server 回 405 并关闭连接
    bool method_not_allowed = false;
    // 方法名不在框架认得的表里（例如 PATCH），由 server 回 501 并关闭连接
    bool method_unknown = false;
    HEAD_METHOD method;
    unsigned char posttype          = 0;
    unsigned char http_action_setup = 0;
    unsigned char headerfinish      = 0;
    unsigned int error              = 0;
    unsigned int readoffset         = 0;
    unsigned int port               = 0;

    unsigned long long http_content_length = 0;

    // 请求体使用 Transfer-Encoding: chunked；与响应侧的 peer->ischunked 无关，不能复用。
    // Request body uses Transfer-Encoding: chunked; unrelated to response-side peer->ischunked.
    bool request_chunked = false;
    // Expect: 100-continue 已见 / 已回过 100，避免重复发送
    // Expect: 100-continue seen / already answered, so 100 is never sent twice
    bool expect_continue = false;
    bool continue_sent   = false;
    // chunked 解码状态：0=chunk-size 行 1=size 行的 LF 2=数据 3=数据的 CR 4=trailer 5=完成
    // 6=数据的 LF 7=trailer 行的 LF
    // chunked decode state: 0=chunk-size line 1=LF after size 2=data 3=CR after data
    // 4=trailer 5=done 6=LF after data 7=LF after trailer
    unsigned char chunk_state            = 0;
    unsigned long long chunk_remaining   = 0;
    unsigned long long chunk_total       = 0;
    unsigned long long chunk_size_tmp    = 0;
    unsigned int chunk_line_bytes        = 0;
    unsigned int chunk_trailer_bytes     = 0;
    unsigned int chunk_trailer_line      = 0;
    bool chunk_in_ext                    = false;
    bool chunk_size_has_digit            = false;
    bool chunked_payload_closed          = false;

    std::unique_ptr<std::FILE, int (*)(FILE *)> uprawfile;
    std::shared_ptr<httppeer> peer;
    std::unique_ptr<websocket_t> websocket{nullptr};
    std::unique_ptr<HTTP_POST_DATA_T> temp_post_data{nullptr};
    std::string header_line;
    std::string pre_http_header;
    std::string chunk_trailer_current;
};
}// namespace http
#endif
