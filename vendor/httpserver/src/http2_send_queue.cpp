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
    // 挂起表里的对象拽着 httppeer 和它的文件句柄，进程收尾时同样要放掉
    std::unique_lock<std::mutex> lock(parked_mutex);
    for (auto iter = parked_list.begin(); iter != parked_list.end();)
    {
        iter->reset();
        parked_list.erase(iter++);
    }
    parked.store(0);
    lock.unlock();
}
void http2_send_queue::fix_queue_list(unsigned int total)
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
std::shared_ptr<http2_send_data_t> http2_send_queue::get_cache_ptr()
{
    std::unique_lock<std::mutex> lock(lock_queue);
    if (queue_list.size() > 0)
    {
        auto sp = std::move(queue_list.front());
        queue_list.pop_front();
        lock.unlock();
        sp->own_state = 1;
        outstanding.fetch_add(1);
        return sp;
    }
    lock.unlock();
    std::shared_ptr<http2_send_data_t> sp = std::make_shared<http2_send_data_t>();
    sp->own_state                         = 1;
    outstanding.fetch_add(1);
    return sp;
}

void http2_send_queue::back_cache_ptr(std::shared_ptr<http2_send_data_t> sp)
{
    if (!sp)
    {
        return;
    }
    outstanding.fetch_sub(1);
    sp->reset();
    std::unique_lock<std::mutex> lock(lock_queue);
    queue_list.emplace_back(sp);
    lock.unlock();
}

void http2_send_queue::park(std::shared_ptr<http2_send_data_t> sp, unsigned char reason)
{
    if (!sp)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(parked_mutex);
    sp->block_reason = reason;
    sp->own_state    = 3;
    parked_list.emplace_back(sp);
    parked.fetch_add(1);
    park_total.fetch_add(1, std::memory_order_relaxed);
    lock.unlock();
}

bool http2_send_queue::detach_parked(const client_session *session_obj, std::list<std::shared_ptr<http2_send_data_t>> &out)
{
    if (parked.load() == 0)
    {
        return false;
    }
    std::unique_lock<std::mutex> lock(parked_mutex);
    for (auto iter = parked_list.begin(); iter != parked_list.end();)
    {
        // session_obj 为空表示摘出全部（兜底扫描逐条重算闸门）
        if (session_obj == nullptr ||
            ((*iter)->peer && (*iter)->peer->socket_session.get() == session_obj))
        {
            out.splice(out.end(), parked_list, iter++);
            parked.fetch_sub(1);
        }
        else
        {
            ++iter;
        }
    }
    lock.unlock();
    return !out.empty();
}

void http2_send_queue::reattach_parked(std::list<std::shared_ptr<http2_send_data_t>> &in)
{
    if (in.empty())
    {
        return;
    }
    std::unique_lock<std::mutex> lock(parked_mutex);
    parked.fetch_add(static_cast<unsigned int>(in.size()));
    parked_list.splice(parked_list.end(), in);
    lock.unlock();
}
}// namespace http