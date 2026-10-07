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
    auto here = std::prev(parked_list.end());
    parked.fetch_add(1);
    park_total.fetch_add(1, std::memory_order_relaxed);

    // 复查闸门和入表必须在同一把锁里：flush_parked_send 可能在调用方读 send_park_closed
    // 之后、我们入锁之前就已经 store(true)+detach，这条挂进来就没人再摘，
    // 要等 CONST_HTTP2_BELT_SWEEP_SECONDS(6s) 的兜底扫描。
    // Re-check under the same lock: flush_parked_send may have stored(true) + detached
    // between the caller's load and our lock, orphaning this entry until the belt sweep.
    // 解锁之后本函数不再碰表，因此两种次序都只有一个执行者：它先拿锁 ⇒ 它的 store 对我们
    // 入锁后的 load 可见，这条由自己摘掉；它等我们解锁 ⇒ 它必然在表里看到这条，由它摘走回池。
    if (sp->peer && sp->peer->socket_session &&
        sp->peer->socket_session->send_park_closed.load())
    {
        parked_list.erase(here);
        parked.fetch_sub(1);
        lock.unlock();
        back_cache_ptr(sp);
        return;
    }
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