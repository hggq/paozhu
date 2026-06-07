#include <atomic>
#include "atomic_count.h"

namespace http
{
    atomic_count::atomic_count(std::atomic_uint &cnt) : counter(cnt) 
    {
        counter.fetch_add(1, std::memory_order_relaxed);
    }

    atomic_count::~atomic_count()
    {
        counter.fetch_sub(1, std::memory_order_relaxed);
    }
}