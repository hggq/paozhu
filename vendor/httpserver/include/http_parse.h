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

// #include "datetime.hpp"
// #include "md5.hpp"
// #include "mime.hpp"
// #include "unicode.hpp"
#include "cookie.h"
// #include "urlcode.hpp"
#include "request.h"
#include "client_session.h"
#include "httppeer.h"
// #include "WebSocket.h"

namespace http
{
    // struct headstate_t
    // {
    //     bool gzip = false;
    //     bool deflate = false;
    //     bool br = false;
    //     bool avif = false;
    //     bool webp = false;
    //     bool keeplive = false;
    //     bool websocket = false;
    //     bool h2c = false;
    //     bool upgradeconnection = false;
    //     bool rangebytes = false;
    //     unsigned char language[6] = {0};
    //     unsigned char version;
    //     unsigned int port;
    //     unsigned long long ifmodifiedsince = 0;
    //     unsigned long long rangebegin = 0;
    //     unsigned long long rangeend = 0;
    // };
    // struct websocket_t
    // {
    //     bool deflate = false;
    //     bool permessagedeflate = false;
    //     bool perframedeflate = false;
    //     bool deflateframe = false;
    //     bool isopen = false;
    //     unsigned char version;
    //     std::string key;
    //     std::string ext;
    // };
    // struct poststate_t
    // {
    //     unsigned long long content_length;
    //     unsigned char posttype = 0;
    //     std::string chartset;
    //     std::string type;
    //     std::string xrequestedwith;
    //     std::string boundary;
    // };
    // struct uploadfile_t
    // {
    //     std::string name;
    //     std::string filename;
    //     std::string tempfile;
    //     std::string type;
    //     unsigned int size;
    //     unsigned char error;
    // };
    // enum HEAD_METHOD
    // {
    //     UNKNOW,
    //     GET,
    //     POST,
    //     OPTIONS,
    //     HEAD,
    //     PUT,
    //     DELETE,
    //     TRACE,
    //     CONNECT,
    // };
    class httpparse
    {
    public:
        httpparse();
        //void setsession(std::shared_ptr<client_session> peer);
        void setpeer(std::shared_ptr<httppeer>  peer);
        unsigned int header_valuetoint();
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
        struct poststate_t poststate;
        struct uploadfile_t upfile;
        bool isnextpage = false;
        HEAD_METHOD method;
        unsigned char httpversion;
        unsigned char posttype = 0;
        unsigned char headerstep = 0;
        unsigned char postfieldtype = 0;

        unsigned char headendhitnum = 0;
        unsigned char headerfinish = 0;
        unsigned int error = 0;
        
        unsigned int readoffset = 0;

        unsigned int changetype = 0;
        std::FILE *uprawfile = NULL;

        // http::OBJ_VALUE get;
        // http::OBJ_VALUE post;
        // http::OBJ_VALUE files;
        // http::OBJ_VALUE json;
        // Cookie cookie;

        // std::string url;
        // std::string urlpath;
        // std::string version;
        // std::string host;
        unsigned int port = 0;
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
}
#endif