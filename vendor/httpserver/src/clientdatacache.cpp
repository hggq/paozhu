#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include "terminal_color.h"
#include "clientdatacache.h"

namespace http
{

    client_data_cache::~client_data_cache()
    {
        for (; !data_list.empty();)
        {
            unsigned char *a = data_list.front();
            data_list.pop();
            std::free(a);
        }
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

        unsigned char *a=nullptr;
        if(data_list.empty())
        {
            a=reinterpret_cast<unsigned char *>(std::malloc(4096 * sizeof(unsigned char)));
        }
        else
        {
            a= data_list.front();
            std::unique_lock<std::mutex> lock(locklist);
            data_list.pop();
        }
         
        return a;
    }
    bool client_data_cache::back_data_ptr(unsigned char *a)
    {
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
}
