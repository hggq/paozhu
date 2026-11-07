#include "ipratelimit.h"

#include <asio/ip/address.hpp>
#include <ctime>

namespace http
{
std::uint64_t ip_rate_key(const std::string &ip)
{
    asio::error_code ec;
    asio::ip::address addr = asio::ip::make_address(ip, ec);
    if (ec)
    {
        return 0;
    }
    if (addr.is_v4())
    {
        return (std::uint64_t)addr.to_v4().to_uint();
    }
    if (addr.is_v6())
    {
        // 取 /64 前缀，把最高位置 1 当 v6 标记，避免与 v4 的整数空间混在一起。
        // Take the /64 prefix and set the top bit as the v6 marker.
        auto bytes        = addr.to_v6().to_bytes();
        std::uint64_t key = 0;
        for (unsigned int i = 0; i < 8; i++)
        {
            key = (key << 8) | bytes[i];
        }
        key |= (std::uint64_t(1) << 63);
        return key;
    }
    return 0;
}

bool ip_rate_limiter::allow(unsigned int route_id, const std::string &ip, unsigned char rate_time, unsigned char rate_num)
{
    if (rate_time == 0 || rate_num == 0)
    {
        return true;
    }

    std::uint64_t key = ip_rate_key(ip);
    if (key == 0)
    {
        return true;
    }

    unsigned int now = (unsigned int)std::time(nullptr);
    std::lock_guard<std::mutex> lock(mutex_);
    route_table &route = routes_[route_id];

    // 惰性清理：每个窗口最多扫一次本路由的桶，把过期的删掉，避免只增不减。
    // Lazy cleanup: sweep this route's buckets at most once per window and drop the expired ones.
    if (route.last_sweep == 0 || now - route.last_sweep >= (unsigned int)rate_time)
    {
        for (auto it = route.buckets.begin(); it != route.buckets.end();)
        {
            if (now - it->second.timeid >= (unsigned int)rate_time)
            {
                it = route.buckets.erase(it);
            }
            else
            {
                ++it;
            }
        }
        route.last_sweep = now;
    }

    auto it = route.buckets.find(key);
    if (it == route.buckets.end())
    {
        route.buckets.emplace(key, bucket{now, 1});
        return true;
    }

    bucket &b = it->second;
    if (now - b.timeid >= (unsigned int)rate_time)
    {
        // 窗口翻篇：从当前请求重新开始计数。
        // New window: restart the count at the current request.
        b.timeid = now;
        b.count  = 1;
        return true;
    }

    if (b.count >= rate_num)
    {
        return false;
    }
    b.count++;
    return true;
}

ip_rate_limiter &get_ip_rate_limiter()
{
    static ip_rate_limiter instance;
    return instance;
}

}// namespace http
