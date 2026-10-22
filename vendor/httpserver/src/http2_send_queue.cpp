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
    client_session *owner = (sp->peer && sp->peer->socket_session)
                                ? sp->peer->socket_session.get()
                                : nullptr;
    sp->block_reason = reason;
    sp->own_state    = 3;
    // 先加计数再入表：事件边在「入表之后、计数之前」到达会读到 0 而跳过这一条，
    // 它就只能等 6 秒兜底。反过来最多让一次并发回灌扑空，不会漏唤醒。
    if (owner)
    {
        owner->parked_count.fetch_add(1, std::memory_order_relaxed);
    }
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
        if (owner)
        {
            owner->parked_count.fetch_sub(1, std::memory_order_relaxed);
        }
        lock.unlock();
        back_cache_ptr(sp);
        return;
    }
    lock.unlock();
}

bool http2_send_queue::detach_parked(const client_session *session_obj, unsigned char reason,
                                     std::list<std::shared_ptr<http2_send_data_t>> &out)
{
    if (parked.load() == 0)
    {
        return false;
    }
    std::unique_lock<std::mutex> lock(parked_mutex);
    for (auto iter = parked_list.begin(); iter != parked_list.end();)
    {
        // session_obj 为空表示摘出全部（兜底扫描逐条重算闸门）；
        // peer 已空的条目不属于任何连接，按会话摘时也一并摘走，否则它只能等 6s 兜底扫描；
        // 摘下后由 dispatch_parked 的无 peer 闸门路径（gate 直接放行 → ready → 发送线程
        // 顶部存活过滤）收口回池，不会被 reattach 挂回。
        // null session = drain all (belt sweep); a null peer belongs to no connection, so
        // drain it on a per-session call too — otherwise it waits for the 6s belt sweep.
        // dispatch_parked's no-peer gate path (gate passes → ready → send-thread liveness
        // filter) recycles it; it is never reattached.
        const bool reason_match  = (reason == 0 || (*iter)->block_reason.load() == reason);
        const bool session_match = session_obj == nullptr || !(*iter)->peer ||
                                   (*iter)->peer->socket_session.get() == session_obj;
        if (reason_match && session_match)
        {
            // 减的是条目自己的连接，不是调用方传进来的 session_obj：
            // 兜底扫描传 nullptr，若按调用方减，所有连接的计数都降不下来。
            client_session *owner = ((*iter)->peer && (*iter)->peer->socket_session)
                                        ? (*iter)->peer->socket_session.get()
                                        : nullptr;
            out.splice(out.end(), parked_list, iter++);
            parked.fetch_sub(1);
            if (owner)
            {
                owner->parked_count.fetch_sub(1, std::memory_order_relaxed);
            }
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
    for (auto &sp : in)
    {
        if (sp && sp->peer && sp->peer->socket_session)
        {
            sp->peer->socket_session->parked_count.fetch_add(1, std::memory_order_relaxed);
        }
    }
    parked.fetch_add(static_cast<unsigned int>(in.size()));
    parked_list.splice(parked_list.end(), in);
    lock.unlock();
}
}// namespace http