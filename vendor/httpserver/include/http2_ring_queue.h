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
    // 积压字节数：只有 post_write（mqtt/websocket 入口）累加，消费者写完整帧后饱和减；
    // http2 直 push 点不记账，恒为 0。累加必须早于 push 发布 tail_：消费者看到帧后就会
    // 做减法，先减后加会让这一帧的字节永久留在计数里。
    std::atomic<unsigned long long> bytes{0};

    // 两个 push 都必须把帧字节深拷贝进槽：入环成功后发送对象即可回收，回池的 reset() 会清掉
    // 它的 content / cache_data / fp。生产者只写 tail_ 那格、消费者只读 head_ 那格，环非空时
    // 二者必不同格。改成零拷贝入环就要同时给回收加一道「等这一格写出」的闸门，否则消费者
    // 会读到已归还给对象池的那份内存。
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
