#include "session_id.h"
#include <stdexcept>
#include <cstdint>
#include <string>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#elif defined(__linux__)
#include <sys/random.h>
#include <cerrno>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <stdlib.h>
#else
#include <random>
#endif
namespace http
{
void fill_secure_random(unsigned char *buf, size_t len)
{
#if defined(_WIN32)
    NTSTATUS st = BCryptGenRandom(nullptr, buf, static_cast<ULONG>(len), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (st != 0)
        throw std::runtime_error("BCryptGenRandom failed");

#elif defined(__linux__)
    size_t off = 0;
    while (off < len)
    {
        ssize_t n = ::getrandom(buf + off, len - off, 0);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("getrandom failed");
        }
        off += static_cast<size_t>(n);
    }

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    arc4random_buf(buf, len);

#else
    std::random_device rd;
    for (size_t i = 0; i < len; ++i)
        buf[i] = static_cast<unsigned char>(rd());
#endif
}

std::string get_rand_session_id()
{
    constexpr size_t kBytes = 16;// 128 位
    unsigned char buf[kBytes];
    fill_secure_random(buf, kBytes);

    static const char hex[] = "0123456789abcdef";
    std::string out;
    out.resize(kBytes * 2);
    for (size_t i = 0; i < kBytes; ++i)
    {
        out[i * 2]     = hex[buf[i] >> 4];
        out[i * 2 + 1] = hex[buf[i] & 0x0F];
    }
    return out;
}

std::string get_rand_32char()
{
    constexpr size_t kBytes = 32;// 128 位
    unsigned char buf[kBytes];
    fill_secure_random(buf, kBytes);

    static const char hex[] = "0123456789abcdef";
    std::string out;
    out.resize(kBytes * 2);
    for (size_t i = 0; i < kBytes; ++i)
    {
        out[i * 2]     = hex[buf[i] >> 4];
        out[i * 2 + 1] = hex[buf[i] & 0x0F];
    }
    return out;
}
}// namespace http