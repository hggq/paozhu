#ifndef __CLIENT_DATA_CACHE_H__
#define __CLIENT_DATA_CACHE_H__

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include "terminal_color.h"

namespace http
{

    class client_data_cache
    {
    public:
        client_data_cache(){};
        ~client_data_cache();
        void inti_sendqueue(unsigned int);
        unsigned char *get_data_ptr();
        bool back_data_ptr(unsigned char *);
        bool add_item();
        unsigned int del_item();
        unsigned int size();

    public:
        std::queue<unsigned char *> data_list;
        std::mutex locklist;
    };
    client_data_cache &get_client_data_cache();
    class client_data_cache_back
    {
    public:
        unsigned char *mcache;

    public:
        void setptr(unsigned char *p)
        {
            mcache = p;
        }
        ~client_data_cache_back()
        {
            get_client_data_cache().back_data_ptr(mcache);
        }
    };

}
#endif
