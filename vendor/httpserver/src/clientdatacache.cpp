#include <cstdlib>
#include <iostream>
#include <string>
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
    unsigned int list_size = data_list.size();
    if (list_size < 1025)
    {
        return false;
    }
    std::unique_lock<std::mutex> lock(locklist);
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
    lock.unlock();
    return true;
}
void client_data_cache::inti_sendqueue(unsigned int a)
{
    for (unsigned int i = 0; i < a; i++)
    {
        data_list.emplace(reinterpret_cast<unsigned char *>(std::malloc(4096 * sizeof(unsigned char))));
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
            try
            {
                a = reinterpret_cast<unsigned char *>(std::malloc(4096 * sizeof(unsigned char)));
                if (a != nullptr)
                {
                    break;
                }
            }
            catch (...)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }
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
    if(isclose)
    {
        free(a);
        return true;
    }
    std::unique_lock<std::mutex> lock(locklist);
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
