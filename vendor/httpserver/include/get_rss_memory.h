

#ifndef GET_RSS_MEMORY_H
#define GET_RSS_MEMORY_H
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <atomic>
 
namespace http
{
size_t get_rss_kb();
size_t get_total_memory_kb();
}
#endif