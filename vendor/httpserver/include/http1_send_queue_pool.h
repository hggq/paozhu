#ifndef HTTP1_SEND_QUEUE_POOL_H__
#define HTTP1_SEND_QUEUE_POOL_H__
/*
 * HTTP/1 静态文件、Range 与压缩作业的池化发送载体。
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

struct http1_send_data_t
{
    unsigned char type            = 0;//1 file 11 peer.output 16 compress gzip content 17 compress br content

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
    void reset()
    {
        type              = 0;
        content_length    = 0;
        current_num        = 0;

        fp.reset();
        peer.reset();

        file_name.clear();
        etag.clear();
        file_ext.clear();
        content_type.clear();
        header.clear();
        content.clear();
    }
};

class http1_send_queue_pool
{
  public:
    http1_send_queue_pool() {};
    ~http1_send_queue_pool();
    std::shared_ptr<http1_send_data_t> get_cache_ptr();
    void fix_queue_list(unsigned int total);
    void back_cache_ptr(std::shared_ptr<http1_send_data_t>);

  public:
    std::list<std::shared_ptr<http1_send_data_t>> queue_list;
    std::mutex lock_queue;
};
http1_send_queue_pool &get_http1_send_queue_pool();

}// namespace http
#endif
