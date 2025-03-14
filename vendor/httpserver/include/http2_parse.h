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
        unsigned char END_STREAM : 1;
        unsigned char e1 : 1;
        unsigned char END_HEADERS : 1;
        unsigned char PADDED : 1;
        unsigned char e2 : 1;
        unsigned char PRIORITY : 1;

    } data;
    char value;
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
    void processblockheader(const unsigned char *buffer, unsigned int buffersize);
    void readheaders(const unsigned char *buffer, unsigned int buffersize);
    // void setstaticheader(const unsigned char, unsigned int, unsigned int);
    void readsetting(const unsigned char *buffer, unsigned int buffersize);
    void readpriority(const unsigned char *buffer, unsigned int buffersize);
    void readdata(const unsigned char *buffer, unsigned int buffersize);
    void readdatablock(const unsigned char *buffer, unsigned int buffersize);

    void readgoaway(const unsigned char *buffer, unsigned int buffersize);
    void readcontinuation(const unsigned char *buffer, unsigned int buffersize);
    void readping(const unsigned char *buffer, unsigned int buffersize);
    void readrst_stream(const unsigned char *buffer, unsigned int buffersize);
    void readsubdata(const unsigned char *buffer, unsigned int buffersize);
    void readwinupdate(const unsigned char *buffer, unsigned int buffersize);
    void readblock(const unsigned char *buffer, unsigned int buffersize);

    void headertype1(unsigned char c, std::string_view buffer, unsigned int &begin, unsigned int end);
    void headertype2(unsigned char c, std::string_view buffer, unsigned int &begin, unsigned int end);
    void headertype3(unsigned char c, std::string_view buffer, unsigned int &begin, unsigned int end);
    void headertype4(unsigned char c, std::string_view buffer, unsigned int &begin, unsigned int end);

    void readformurlencoded();
    void procssxformurlencoded();

    void readmultipartformdata(const unsigned char *buffer, unsigned int buffersize);
    void readrawfileformdata(const unsigned char *buffer, unsigned int buffersize);
    void readboundaryline(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize);
    void readformfilename(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize);
    void readformfieldcontent(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize);
    void readformfieldfilecontent(const unsigned char *buffer, unsigned int &begin, unsigned int buffersize);

    void fieldname_process(const std::string &tfheader_name, std::string &fieldname, std::string &fieldfilename);
    void fieldtype_process(const std::string &tfheader_name);
    void procssformfile();

  public:
    void headers_parse();
    void process(const unsigned char *buffer, unsigned int buffersize);
    void data_process();
    void getacceptencoding(const std::string &, const std::string &);
    void header_process(const std::string &, const std::string &, int);
    void cookie_process(const std::string &, const std::string &);
    void getacceptlanguage(const std::string &, const std::string &);
    void range_process(const std::string &, const std::string &);
    void path_process(const std::string &, const std::string &);
    void getaccept(const std::string &, const std::string &);
    void getcontenttype(const std::string &, const std::string &);
    void getifnonematch(const std::string &, const std::string &);

    void callposttype();
    void clsoesend(asio::io_context &ioc);
    void procssparamter();

  public:
    // std::string buffer_key;
    // std::string buffer_value;
    // std::string field_value;

    unsigned int headendhitnum = 0;
    // struct uploadfile_t upfile;

    std::string header;
    unsigned int blocklength   = 0;
    unsigned int block_steamid = 0;
    unsigned char headerfinish = 0;
    unsigned char frame_type   = 0;
    unsigned char flag_type    = 0;
    unsigned int error         = 0;
    unsigned int readoffset    = 0;
    unsigned int processheader = 0;

    unsigned int steam_id    = 0;
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
    std::map<unsigned int, std::shared_ptr<http2_data_t>> data_info;
    std::map<unsigned int, std::shared_ptr<httppeer>> http_data;
    // std::map<unsigned int, struct http2_priority_t> priority_data;
    std::map<unsigned int, std::shared_ptr<std::string>> stream_data;

    std::queue<unsigned int> stream_list;
    std::queue<std::shared_ptr<httppeer>> httppeer_lists;

    std::mutex http2loop_mutex;
    std::atomic<bool> http2_loop_in = false;

    std::shared_ptr<httppeer> block_steam_httppeer;
    std::shared_ptr<http2_data_t> block_data_info_ptr;
    std::shared_ptr<std::string> stream_data_ptr;
};
}// namespace http
#endif
