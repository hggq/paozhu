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
    unsigned int field_offset;
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

    void process(const unsigned char *buffer, unsigned int buffersize);
    bool getfinish();
    void clear();

  public:
    bool isfinish_header = false;
    bool isfinish_url    = false;
    HEAD_METHOD method;
    unsigned char posttype          = 0;
    unsigned char http_action_setup = 0;
    unsigned char headerfinish      = 0;
    unsigned int error              = 0;
    unsigned int readoffset         = 0;
    unsigned int port               = 0;

    unsigned long long http_content_length = 0;

    std::unique_ptr<std::FILE, int (*)(FILE *)> uprawfile;
    std::shared_ptr<httppeer> peer;
    std::unique_ptr<websocket_t> websocket{nullptr};
    std::shared_ptr<HTTP_POST_DATA_T> temp_post_data{nullptr};
    std::string header_line;
    std::string raw_header_content;
    std::string pre_http_header;
};
}// namespace http
#endif
