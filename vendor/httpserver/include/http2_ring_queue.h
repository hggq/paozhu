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
#include "http2_borrow_state.h"
#include "terminal_color.h"
#include "cost_define.h"

namespace http
{

// 环容量的编译期约束：2 的幂（下标用掩码推进）且能塞进 unsigned char（head_/tail_ 的类型）。
static_assert(CONST_HTTP2_RING_QUEUE_CAPACITY >= 2, "http2 ring capacity must be at least 2");
static_assert((CONST_HTTP2_RING_QUEUE_CAPACITY & (CONST_HTTP2_RING_QUEUE_CAPACITY - 1)) == 0,
              "http2 ring capacity must be a power of two (index uses capacity_-1 mask)");
static_assert(CONST_HTTP2_RING_QUEUE_CAPACITY <= 255,
              "http2 ring capacity must fit unsigned char (head_/tail_ type)");

struct data_frame_borrowed
{
    char header[9]{};
    const char *payload_ptr = nullptr;
    unsigned int payload_len = 0;
};

struct http2_send_queue_cache
{
    std::array<std::string, CONST_HTTP2_RING_QUEUE_CAPACITY> data;
    const unsigned char capacity_ = CONST_HTTP2_RING_QUEUE_CAPACITY;
    std::atomic<unsigned char> head_{0};
    std::atomic<unsigned char> tail_{0};
    std::mutex http2_queue_send_mutex;
    // 积压字节数：只有 post_write（mqtt/websocket 入口）累加，消费者写完整帧后饱和减；
    // http2 直 push 点不记账，恒为 0。累加必须早于 push 发布 tail_：消费者看到帧后就会
    // 做减法，先减后加会让这一帧的字节永久留在计数里。
    std::atomic<unsigned long long> bytes{0};
    std::atomic_bool closed{false};

    // push 把完整帧深拷贝进槽，入环成功后调用方可立即释放源缓冲。
    bool push(const unsigned char *buffer, unsigned int buffersize);
    bool push(const std::string &item);
    bool push(std::string &&item);

    void close();
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
