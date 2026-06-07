
#pragma once
#ifndef HTTP_ATOM_COUNT_H
#define HTTP_ATOM_COUNT_H
#include <atomic>

namespace http
{
class atomic_count 
{
private:
    std::atomic_uint &counter;
public:
    explicit atomic_count(std::atomic_uint &cnt);
    ~atomic_count();
    atomic_count(const atomic_count&) = delete;
    atomic_count& operator=(const atomic_count&) = delete;
};

}
#endif