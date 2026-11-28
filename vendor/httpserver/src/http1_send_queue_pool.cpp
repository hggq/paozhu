/*
 * HTTP/1 静态文件、Range 与压缩作业对象池
 * 黄自权(huang ziquan)
 * 2025-01-02
 */
#include <iostream>
#include <string>
#include <mutex>
#include <atomic>
#include "http1_send_queue_pool.h"

namespace http
{
http1_send_queue_pool &get_http1_send_queue_pool()
{
    static http1_send_queue_pool instance;
    return instance;
}
http1_send_queue_pool::~http1_send_queue_pool()
{
    for (auto iter = queue_list.begin(); iter != queue_list.end();)
    {
        iter->reset();
        queue_list.erase(iter++);
    }
}
void http1_send_queue_pool::fix_queue_list(unsigned int total)
{
    std::unique_lock<std::mutex> lock(lock_queue);
    unsigned int queue_size = queue_list.size();
    if (queue_size > total)
    {
        unsigned int j = queue_size / 4;
        if (j > 150)
        {
            for (auto iter = queue_list.begin(); iter != queue_list.end();)
            {
                iter->reset();
                queue_list.erase(iter++);

                j--;
                if (j < 1)
                {
                    break;
                }
            }
        }
    }
    lock.unlock();
}
std::shared_ptr<http1_send_data_t> http1_send_queue_pool::get_cache_ptr()
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
    std::shared_ptr<http1_send_data_t> sp = std::make_shared<http1_send_data_t>();
    return sp;
}

void http1_send_queue_pool::back_cache_ptr(std::shared_ptr<http1_send_data_t> sp)
{
    if (!sp)
    {
        return;
    }
    sp->reset();
    std::unique_lock<std::mutex> lock(lock_queue);
    queue_list.emplace_back(sp);
    lock.unlock();
}

}// namespace http