#ifndef HTTP2_SEND_QUEUE_H__
#define HTTP2_SEND_QUEUE_H__
/*
 * http2 send queue
 * 黄自权(huang ziquan)
 * 2025-01-02
 */
#include <iostream>
#include <string>
#include <list>
#include <mutex>
#include <atomic>
#include <chrono>
#include "terminal_color.h"
#include "httppeer.h"

namespace http
{

struct http2_send_data_t
{
    bool isfinish                 = false;
    bool is_sendheader            = false;
    bool only_send_header         = false;
    unsigned char send_thread_id  = 0;
    unsigned char type            = 0;//1 file 2 not use 3 gip file 4 br file    10 data (php fast-cgi) 11 peer.output  16 compress gzip content 16 compress br content
    std::atomic_bool standby_next = true;
    unsigned int stream_id        = 0;

    //unsigned int file_modify_time=0; //modify time

    unsigned long long send_data_window_num = 4096;//send data window num max 16384 min 4096

    unsigned long long content_length = 0;//content_length
    unsigned long long current_num    = 0;

    std::chrono::time_point<std::chrono::steady_clock> last_time=std::chrono::steady_clock::now();
    long long sleep_time = 0;//duartion time  milliseconds * 1 = microseconds * 1000 = nanoseconds * 1000 * 1000

    std::unique_ptr<std::FILE, int (*)(FILE *)> fp = {nullptr, std::fclose};
    std::shared_ptr<httppeer> peer;

    std::string file_name;
    std::string etag;
    std::string file_ext;
    std::string content_type;
    std::string header;
    std::string content;
    std::string cache_data;
    void reset()
    {
        isfinish         = false;
        standby_next     = true;
        is_sendheader    = false;
        only_send_header = false;
        type             = 0;
        stream_id        = 0;
        //        file_modify_time=0;

        send_data_window_num = 4096;
        content_length       = 0;
        current_num          = 0;
        sleep_time           = 0;

        fp.reset();
        peer.reset();

        file_name.clear();
        etag.clear();
        file_ext.clear();
        content_type.clear();
        header.clear();
        content.clear();
        cache_data.clear();
        cache_data.resize(16384);
    }
};

class http2_send_queue
{
  public:
    http2_send_queue() {};
    ~http2_send_queue();
    std::shared_ptr<http2_send_data_t> get_cache_ptr();
    void fix_queue_list();
    void back_cache_ptr(std::shared_ptr<http2_send_data_t>);

  public:
    std::list<std::shared_ptr<http2_send_data_t>> queue_list;
    std::mutex lock_queue;
    bool isclose = false;
};
http2_send_queue &get_http2_send_queue();

}// namespace http
#endif
