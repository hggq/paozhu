#include <atomic>
#include "atomic_guard.h"

namespace http
{
    atomic_guard::atomic_guard(std::atomic_flag &cnt) : flag_guard(cnt) 
    {
    }
    atomic_guard::~atomic_guard()
    {
        flag_guard.clear(); 
    }
}