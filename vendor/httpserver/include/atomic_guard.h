
#pragma once
#ifndef HTTP_ATOM_GUARD_H
#define HTTP_ATOM_GUARD_H
#include <atomic>

namespace http
{
class atomic_guard
{
private:
    std::atomic_flag &flag_guard;
public:
    explicit atomic_guard(std::atomic_flag &cnt);
    ~atomic_guard();
    atomic_guard(const atomic_guard&) = delete;
    atomic_guard& operator=(const atomic_guard&) = delete;
};

}
#endif