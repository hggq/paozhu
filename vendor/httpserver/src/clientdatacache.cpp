#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>
#include <list>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include "terminal_color.h"
#include "clientdatacache.h"

namespace http
{

client_data_cache::~client_data_cache()
{
    std::unique_lock<std::mutex> lock(locklist);
    isclose = true;
    for (; !data_list.empty();)
    {
        unsigned char *a = data_list.front();
        data_list.pop();
        std::free(a);
    }
}
bool client_data_cache::fix_lists()
{
    std::unique_lock<std::mutex> lock(locklist);
    unsigned int list_size = data_list.size();
    if (list_size < 1025)
    {
        return false;
    }
    for (; !data_list.empty();)
    {
        unsigned char *a = data_list.front();
        data_list.pop();
        std::free(a);
        --list_size;
        if (list_size < 1025)
        {
            break;
        }
    }
    return true;
}
void client_data_cache::inti_sendqueue(unsigned int a)
{
    for (unsigned int i = 0; i < a; i++)
    {
        unsigned char *ptr = reinterpret_cast<unsigned char *>(std::malloc(4096));
        if (ptr != nullptr) data_list.emplace(ptr);
    }
}
unsigned char *client_data_cache::get_data_ptr()
{

    unsigned char *a = nullptr;
    std::unique_lock<std::mutex> lock(locklist);
    if (data_list.empty())
    {
        lock.unlock();
        for (int i = 0; i < 10; i++)
        {
            a = reinterpret_cast<unsigned char *>(std::malloc(4096 * sizeof(unsigned char)));
            if (a != nullptr)
            {
                break;
            }
            // malloc失败时短暂休眠后重试
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
        // 如果10次尝试都失败，抛出异常或返回nullptr
        if (a == nullptr)
        {
            throw std::bad_alloc();
        }
    }
    else
    {
        a = data_list.front();
        data_list.pop();
        lock.unlock();
    }

    return a;
}
bool client_data_cache::back_data_ptr(unsigned char *a)
{
    if (a == nullptr) return false;
    std::unique_lock<std::mutex> lock(locklist);
    if (isclose)
    {
        lock.unlock();
        free(a);
        return true;
    }
    data_list.push(a);
    return true;
}
bool client_data_cache::add_item() { return true; }
unsigned int client_data_cache::del_item() { return 0; }
unsigned int client_data_cache::size() { return 0; }

client_data_cache &get_client_data_cache()
{
    static client_data_cache instance;
    return instance;
}
}// namespace http
