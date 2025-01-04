/*
 * http2 send queue
 * 黄自权(huang ziquan)
 * 2025-01-02
 */
#include <iostream>
#include <string>
#include <mutex>
#include <atomic>
#include "http2_send_queue.h"

namespace http
{
http2_send_queue &get_http2_send_queue()
{
    static http2_send_queue instance;
    return instance;
}
http2_send_queue::~http2_send_queue()
{
    isclose = true;
    for (auto iter = queue_list.begin(); iter != queue_list.end();)
    {
        iter->reset();
        queue_list.erase(iter++);
    }
}
void http2_send_queue::fix_queue_list()
{
    std::unique_lock<std::mutex> lock(lock_queue);
    unsigned int queue_size = queue_list.size();
    if (queue_size > 200)
    {
        unsigned int j = queue_size / 20;
        if (j < 200)
        {
            j = 200;
        }
        for (auto iter = queue_list.begin(); iter != queue_list.end();)
        {
            iter->reset();
            queue_list.erase(iter++);

            j--;
            if (j < 200)
            {
                break;
            }
        }
    }
    lock.unlock();
}
std::shared_ptr<http2_send_data_t> http2_send_queue::get_cache_ptr()
{
    std::unique_lock<std::mutex> lock(lock_queue);
    if (queue_list.size() > 0)
    {
        auto sp = std::move(queue_list.front());
        queue_list.pop_front();
        lock.unlock();
        return sp;
    }
    lock.unlock();
    std::shared_ptr<http2_send_data_t> sp = std::make_shared<http2_send_data_t>();
    return sp;
}

void http2_send_queue::back_cache_ptr(std::shared_ptr<http2_send_data_t> sp)
{
    sp->reset();
    std::unique_lock<std::mutex> lock(lock_queue);
    queue_list.emplace_back(sp);
    lock.unlock();
}
}// namespace http