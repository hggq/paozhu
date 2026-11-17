#ifndef HTTP2_SEND_QUEUE_H__
#define HTTP2_SEND_QUEUE_H__
/*
 * http2 send queue —— 池化发送作业载体（h1 发送路径仍借用）。
 * h2 直发后不再承载发送线程/挂起表，仅保留对象池与在途计数。
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
    unsigned char type            = 0;//1 file 11 peer.output 16 compress gzip content 17 compress br content
    unsigned int stream_id        = 0;

    //unsigned int file_modify_time=0; //modify time

    unsigned long long content_length = 0;//content_length
    unsigned long long current_num    = 0;
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
        is_sendheader    = false;
        only_send_header = false;
        type             = 0;
        stream_id        = 0;
        //        file_modify_time=0;

        content_length     = 0;
        current_num        = 0;

        fp.reset();
        peer.reset();

        file_name.clear();
        etag.clear();
        file_ext.clear();
        content_type.clear();
        header.clear();
        content.clear();
        cache_data.clear();
    }
};

class http2_send_queue
{
  public:
    http2_send_queue() {};
    ~http2_send_queue();
    std::shared_ptr<http2_send_data_t> get_cache_ptr();
    void fix_queue_list(unsigned int total);
    void back_cache_ptr(std::shared_ptr<http2_send_data_t>);

  public:
    std::list<std::shared_ptr<http2_send_data_t>> queue_list;
    std::mutex lock_queue;
    bool isclose = false;

    // 已取出但未回池的对象数：get_cache_ptr ++、back_cache_ptr --。
    // 连接全部收尾后必须归零，不归零就是泄漏。
    std::atomic<unsigned int> outstanding = 0;
};
http2_send_queue &get_http2_send_queue();

}// namespace http
#endif
