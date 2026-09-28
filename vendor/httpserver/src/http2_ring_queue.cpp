/*
 * http2 send queue
 * 黄自权(huang ziquan)
 * 2025-01-02
 */
#include <iostream>
#include <string>
#include <mutex>
#include <atomic>
#include "http2_ring_queue.h"

namespace http
{
bool http2_send_queue_cache::push(const unsigned char *buffer, unsigned int buffersize)
{
    std::unique_lock lk(http2_queue_send_mutex);
    std::size_t tail      = tail_.load(std::memory_order_relaxed);
    std::size_t next_tail = (tail + 1) & (capacity_ - 1);
    if (next_tail != head_.load(std::memory_order_acquire))
    {
        data[tail].clear();
        data[tail].append(reinterpret_cast<const char *>(buffer), buffersize);
        tail_.store(next_tail, std::memory_order_release);
        lk.unlock();
        return true;
    }
    lk.unlock();
    return false;
}
bool http2_send_queue_cache::push(const std::string &item)
{
    std::unique_lock lk(http2_queue_send_mutex);
    std::size_t tail      = tail_.load(std::memory_order_relaxed);
    std::size_t next_tail = (tail + 1) & (capacity_ - 1);
    if (next_tail != head_.load(std::memory_order_acquire))
    {
        data[tail] = item;
        tail_.store(next_tail, std::memory_order_release);
        lk.unlock();
        return true;
    }
    lk.unlock();
    return false;
}
unsigned char http2_send_queue_cache::has_size()
{

    unsigned char tail      = tail_.load(std::memory_order_relaxed);
    unsigned char next_tail = (tail + 1) & (capacity_ - 1);
    unsigned char head_num  = head_.load(std::memory_order_acquire);
    if (next_tail == head_num)
    {
        return capacity_;
    }

    unsigned char send_queue_size = 0;

    if (tail > head_num)
    {
        send_queue_size = tail - head_num;
    }
    else if (tail < head_num)
    {
        send_queue_size = ((capacity_ - 1) - head_num) + tail + 1;
    }
    return send_queue_size;
}

bool http2_send_queue_cache::pop(std::string &item)
{
    unsigned char head = head_.load(std::memory_order_relaxed);
    if (head == tail_.load(std::memory_order_acquire))
    {
        return false;
    }

    item = data[head];
    head_.store((head + 1) & (capacity_ - 1), std::memory_order_release);

    return true;
}

void http2_send_queue_cache::reset()
{
    head_ = 0;
    tail_ = 0;
    bytes.store(0, std::memory_order_relaxed);
    for (auto &pd : data)
    {
        pd.clear();
        // clear() 不释放 capacity；归还对象池前把巨槽（>128KB）交换释放，
        // 避免一条大帧让槽永久带着巨容量在池里滞留。http2 帧 ≤16KB 不触阈值。
        if (pd.capacity() > 131072)
        {
            std::string().swap(pd);
        }
    }
}

http2_ring_queue_obj &get_http2_ring_queue_obj()
{
    // 故意泄漏：~client_session() 在退出期回桩到这里要锁 lock_queue，
    // 而本单例首次使用晚于 get_server_app()，按 __cxa_atexit 逆序它先被销毁，
    // 那时锁已是尸体，lock() 抛 system_error 撞上 destructor 的隐式 noexcept ⇒ terminate。
    static http2_ring_queue_obj *instance = new http2_ring_queue_obj();
    return *instance;
}
http2_ring_queue_obj::~http2_ring_queue_obj()
{
    isclose = true;
    for (auto iter = queue_list.begin(); iter != queue_list.end();)
    {
        iter->reset();
        queue_list.erase(iter++);
    }
}

void http2_ring_queue_obj::fix_queue_list()
{
    std::unique_lock<std::mutex> lock(lock_queue);
    unsigned int queue_size = queue_list.size();
    if (queue_size > 120)
    {
        // 保留 120 个缓存对象，超出部分清理
        unsigned int remove_count = queue_size - 120;
        for (auto iter = queue_list.begin(); iter != queue_list.end() && remove_count > 0;)
        {
            iter->reset();
            queue_list.erase(iter++);
            remove_count--;
        }
    }
    lock.unlock();
}
std::unique_ptr<http2_send_queue_cache> http2_ring_queue_obj::get_cache_ptr()
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
    std::unique_ptr<http2_send_queue_cache> sp = std::make_unique<http2_send_queue_cache>();
    return sp;
}

void http2_ring_queue_obj::back_cache_ptr(std::unique_ptr<http2_send_queue_cache> sp)
{
    sp->reset();
    std::unique_lock<std::mutex> lock(lock_queue);
    queue_list.emplace_back(std::move(sp));
    lock.unlock();
}

}// namespace http