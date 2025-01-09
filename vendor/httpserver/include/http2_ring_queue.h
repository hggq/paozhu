#ifndef HTTP2_RING_QUEUE_H__
#define HTTP2_RING_QUEUE_H__
/*
 * http2 send queue
 * 黄自权(huang ziquan)
 * 2025-01-02
 */
#include <iostream>
#include <string>
#include <list>
#include <array>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include "terminal_color.h"

namespace http
{

struct http2_send_queue_cache
{
    std::array<std::string, 16> data;
    const unsigned char capacity_ = 16;
    std::atomic<unsigned char> head_{0};
    std::atomic<unsigned char> tail_{0};
    std::mutex http2_queue_send_mutex;

    bool push(const unsigned char *buffer, unsigned int buffersize);
    bool push(const std::string &item);

    unsigned char has_size();
    bool pop(std::string &item);
    void reset();
};

class http2_ring_queue_obj
{
  public:
    http2_ring_queue_obj() {};
    ~http2_ring_queue_obj();
    std::unique_ptr<http2_send_queue_cache> get_cache_ptr();
    void fix_queue_list();
    void back_cache_ptr(std::unique_ptr<http2_send_queue_cache>);

  public:
    std::list<std::unique_ptr<http2_send_queue_cache>> queue_list;
    std::mutex lock_queue;
    bool isclose = false;
};
http2_ring_queue_obj &get_http2_ring_queue_obj();

}// namespace http
#endif
