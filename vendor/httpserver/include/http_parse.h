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
// #include "WebSocket.h"

namespace http
{

class httpparse
{
  public:
    httpparse();
    //void setsession(std::shared_ptr<client_session> peer);
    void setpeer(std::shared_ptr<httppeer> peer);
    unsigned long long header_valuetoint();
    unsigned int checkheadend(const unsigned char *buffer, unsigned int buffersize);
    void procssparamter();
    void procssxformurlencoded();
    bool checkmethod();
    void methodprocess();
    void callposttype();
    void getcontenttype();
    void getrange();
    void readheaderline(const unsigned char *buffer, unsigned int buffersize);
    void getaccept();
    void getacceptlanguage();
    void getifunmodifiedsince();
    void getifmodifiedsince();
    void getifnonematch();
    void getwebsocketextensions();
    void getupgrade();
    void getconnection();
    void getacceptencoding();
    void getcookie();
    void getheaderhost();
    void readboundaryline(const unsigned char *buffer, unsigned int buffersize);
    void readformfilename(const unsigned char *buffer, unsigned int buffersize);
    void readformfielditem(const unsigned char *buffer, unsigned int buffersize);
    void procssformfile();
    void readformfilecotent(const unsigned char *buffer, unsigned int buffersize);
    void readmultipartformdata(const unsigned char *buffer, unsigned int buffersize);
    void readformjson(const unsigned char *buffer, unsigned int buffersize);
    void readformurlencoded(const unsigned char *buffer, unsigned int buffersize);

    void readformxml(const unsigned char *buffer, unsigned int buffersize);
    void readformraw(const unsigned char *buffer, unsigned int buffersize);

    void process(const unsigned char *buffer, unsigned int buffersize);
    bool getfinish();
    void clear();

  public:
    //struct headstate_t state;
    //struct websocket_t websocket;
    //Improve this, use std::unique_ptr
    //struct poststate_t poststate;
    //struct uploadfile_t upfile;

    bool isnextpage = false;
    HEAD_METHOD method;
    unsigned char httpversion   = 0;
    unsigned char posttype      = 0;
    unsigned char headerstep    = 0;
    unsigned char postfieldtype = 0;
    unsigned char headendhitnum = 0;
    unsigned char headerfinish  = 0;

    unsigned int error      = 0;
    unsigned int readoffset = 0;
    unsigned int changetype = 0;
    unsigned int port       = 0;

    unsigned long long content_length;
    //std::FILE *uprawfile = NULL;
    std::unique_ptr<poststate_t> poststate;
    std::unique_ptr<uploadfile_t> upfile;
    std::unique_ptr<std::FILE, int (*)(FILE *)> uprawfile;
    // http::OBJ_VALUE get;
    // http::OBJ_VALUE post;
    // http::OBJ_VALUE files;
    // http::OBJ_VALUE json;
    // Cookie cookie;

    // std::string url;
    // std::string urlpath;
    // std::string version;
    // std::string host;

    // std::string useragent;
    //std::string upgrade;
    // std::string querystring;
    std::string contentline;
    // std::string etag;
    //std::string acceptLanguage;
    std::string header_key;
    std::string header_value;
    std::string header_temp;
    std::string header_input;
    // std::vector<std::string> headerrawcontent;
    // std::map<std::string, std::string> header;
    std::string buffer_key;
    std::string buffer_value;

    // std::vector<std::string> pathinfo;
    //std::shared_ptr<client_session> peer_session;
    std::shared_ptr<httppeer> peer;
};
}// namespace http
#endif
