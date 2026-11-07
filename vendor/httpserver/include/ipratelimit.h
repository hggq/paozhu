#ifndef PAOZHU_IPRATELIMIT_H
#define PAOZHU_IPRATELIMIT_H

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace http
{
// 把 IP 转成限流 key：v4 用 to_uint()，v6 取前 8 字节（/64 前缀）并把最高位置 1 作标记。
// 解析失败返回 0，调用方按"不参与限流"处理。
// Turns an IP into a limiter key: v4 as to_uint(), v6 as its /64 prefix with the top bit set.
// Returns 0 when parsing fails; callers treat that as "skip rate limiting".
std::uint64_t ip_rate_key(const std::string &ip);

// 固定窗口限流器：每个路由条目 + IP 一个桶，窗口翻篇时计数归零。
// Fixed-window limiter: one bucket per route entry + IP, the counter resets when the window rolls over.
class ip_rate_limiter
{
  public:
    // rate_time 是窗口秒数，rate_num 是窗口内每 IP 允许的次数；任一为 0 直接放行。
    // rate_time is the window in seconds, rate_num the per-IP allowance; either being 0 means allow all.
    bool allow(unsigned int route_id, const std::string &ip, unsigned char rate_time, unsigned char rate_num);

  private:
    struct bucket
    {
        unsigned int timeid = 0;
        unsigned int count  = 0;
    };
    struct route_table
    {
        std::unordered_map<std::uint64_t, bucket> buckets;
        unsigned int last_sweep = 0;
    };

    std::mutex mutex_;
    std::unordered_map<unsigned int, route_table> routes_;
};

// 全局单例：router 的同步链和协程链共用同一份计数。
// Process-wide singleton shared by the sync and coroutine dispatch chains.
ip_rate_limiter &get_ip_rate_limiter();

}// namespace http

#endif
