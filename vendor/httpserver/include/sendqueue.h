#ifndef __SEND_QUEUE_H__
#define __SEND_QUEUE_H__

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <mutex>
#include <atomic>
#include "terminal_color.h"

namespace http
{
     struct sendqueue_t
     {
        unsigned char type=0;
        unsigned char *data;
        unsigned int  capacity_size=0;
        unsigned int  data_size=0;
        unsigned int  data_index=0;
        unsigned long long timeid=0;
     };
     
     class sendqueue
     {
        public:
        sendqueue(){};
        ~sendqueue();
           void inti_sendqueue(unsigned int);
           bool back_item(unsigned int index);
           sendqueue_t *get_cache_ptr();
           bool back_cache_ptr(sendqueue_t *);
           bool add_item();
           unsigned int del_item();
           unsigned int size();
        public:
         std::vector<sendqueue_t> cache_data;
         std::list<int> empty_list;
         std::list<int> busy_list;
         std::atomic_uint cache_size=0;
         std::mutex locklist;
     };
     sendqueue &get_sendqueue();
     class sendqueue_back
      {
        public:

        struct sendqueue_t *mcache;
        public:
        void setptr(struct sendqueue_t *p)
        {
            mcache=p;
        }
        ~sendqueue_back()
        {
            mcache->type=0;
            DEBUG_LOG(" ~sendqueue_back  ");
        }
      };

}
#endif
