
#include <iostream>
#include <string>
#include <mutex>
#include <atomic>
#include "sendqueue.h"
#include "vector"
namespace http
{

    sendqueue &get_sendqueue()
    {
        static sendqueue instance;
        return instance;
    }
    sendqueue::~sendqueue()
    {
        unsigned int create_size = cache_data.size();
        for (int i = 0; i < create_size; i++)
        {
            std::free(cache_data[i].data);
        }
    }
    void sendqueue::inti_sendqueue(unsigned int create_size)
    {

        if (create_size > 256)
        {
            cache_data.reserve(create_size);
        }
        else
        {
            cache_data.reserve(256);
        }
        int j = cache_data.size();
        for (unsigned int i = 0; i < create_size; i++)
        {
            try
            {
                struct sendqueue_t temp_data;
                temp_data.data = reinterpret_cast<unsigned char *>(std::malloc(8192 * sizeof(unsigned char)));
                temp_data.capacity_size = 8192;
                temp_data.data_size = 0;
                temp_data.type = 0;
                temp_data.timeid = 0;
                temp_data.data_index = j;
                j++;

                cache_data.push_back(std::move(temp_data));
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        cache_size = cache_data.size();
    }
    unsigned int sendqueue::del_item()
    {
        unsigned int last_elm = cache_data.size() - 1;
        unsigned int total_count = 0;
        for (unsigned int i = last_elm; i > 256; i--)
        {
            if (cache_data[i].type == 0)
            {

                try
                {
                    std::unique_lock<std::mutex> lock(locklist);
                    std::free(cache_data[i].data);
                    cache_data.pop_back();
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }

                total_count += 1;
            }
            else
            {
                break;
            }
        }
        cache_size = cache_data.size();
        return total_count;
    }
    bool sendqueue::add_item()
    {
        try
        {
            struct sendqueue_t temp_data;
            temp_data.data = reinterpret_cast<unsigned char *>(std::malloc(8192 * sizeof(unsigned char)));
            temp_data.capacity_size = 8192;
            temp_data.data_size = 0;
            temp_data.type = 0;
            temp_data.timeid = 0;
            std::unique_lock<std::mutex> lock(locklist);
            cache_data.push_back(std::move(temp_data));
            cache_size = cache_data.size();
            cache_data[cache_size - 1].data_index = cache_size - 1;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
        return true;
    }
    bool sendqueue::back_item(unsigned int index)
    {
        if (index < cache_data.size())
        {
            std::unique_lock<std::mutex> lock(locklist);
            cache_data[index].type = 0;
            return true;
        }
        return false;
    }
    sendqueue_t *sendqueue::get_cache_ptr()
    {
        struct sendqueue_t *data_ptr = nullptr;
        unsigned int cache_size=cache_data.size();
        for (unsigned int i = 0; i < cache_size; i++)
        {
            if (cache_data[i].type == 0)
            {
                std::unique_lock<std::mutex> lock(locklist);
                cache_data[i].type = 1;
                data_ptr = &cache_data[i];
                break;
            }
        }
        if (data_ptr == nullptr)
        {
            for (unsigned int i = 0; i < 3; i++)
            {
                add_item();
                data_ptr = &cache_data[cache_data.size() - 1];

                if (data_ptr != nullptr)
                {
                    data_ptr->type = 1;
                    break;
                }
            }
        }
        return data_ptr;
    }
}