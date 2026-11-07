#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <ctime>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace http
{
namespace rbac
{
// 缓存计数，用于观察命中率与失效动作。
// Counters for cache hits, misses and invalidation actions.
struct cache_stats
{
    std::uint64_t hit             = 0;
    std::uint64_t miss            = 0;
    std::uint64_t load_fail       = 0;
    std::uint64_t evict           = 0;
    std::uint64_t invalidate_all  = 0;
    std::uint64_t invalidate_user = 0;
};

// 运行期授权快照缓存（按用户装载，惰性填充）。
// Runtime grant snapshot cache (loaded per user, filled lazily).
//
// 设计要点：
// Design notes:
//  * 键 = (adminid, companyid)，值 = 该用户当前生效的授权表（urlpath → 该路径上各权限行的
//    maskcode 列表）；一次冷启动 6 条查询换来该用户之后所有带 maskcode 路由 0 查询，比按
//    (adminid, urlpath) 缓存更省（后者每条 URL 都要重跑同一套角色/权限链）。
//  * Key = (adminid, companyid), value = the user's currently effective grant table
//    (urlpath -> maskcode list of the permission rows on that path). One cold load of six
//    queries buys zero-query decisions for all masked routes of that user afterwards, which is
//    cheaper than caching by (adminid, urlpath): the latter reruns the same role/permission
//    chain for every URL.
//  * 只有注解里带 maskcode 的路由会走到这里；没有 maskcode 的路由在 router 里就分流出去了。
//  * Only routes annotated with maskcode reach here; routes without maskcode are filtered out
//    earlier in the router.
//  * 判定 = urlpath 在授权表里 且 路由条目的掩码与表里某一条 maskcode 精确相等；
//    不相等说明权限目录里的 maskcode 和代码里的注册掩码已经漂移，按无权处理。位覆盖
//    （granted 包住 route）会让目录行改宽后静默放行，所以逐条比而不是先并再比。
//  * Allow = urlpath is present in the grant table AND the route entry's mask exactly equals
//    one of the stored maskcodes; a mismatch means the permission catalog has drifted from the
//    registered mask and is treated as no permission. Bitwise coverage (granted covers route)
//    would silently allow after a catalog row is widened, so compare entry by entry instead of
//    OR-ing first.
//  * 失效 = 全局代次号 + 按用户精确清除；写操作成功后主动调用，TTL 兜底。
//  * Invalidation = global generation counter + exact per-user erase; writers call it after a
//    successful change, with TTL as a fallback.
//  * 失败 = fail-closed（返回 false 由调用方写 403），不缓存失败结果。
//  * Failure = fail-closed (return false and the caller writes 403); failed loads are not cached.
class route_cache
{
  public:
    // 分片数：16 个互斥锁分别保护一张表，避免所有请求抢同一把锁。
    // Shard count: 16 separate mutexes each guard one table, so requests do not all contend on
    // a single lock.
    static constexpr unsigned int shard_count = 16;
    // 总量上限，按分片均分；超出后先清过期，再淘汰该分片最旧的一条。
    // Global entry cap, split evenly across shards; when full, purge expired or old-generation
    // rows first, then evict the oldest entry in that shard.
    static constexpr unsigned int max_entries = 4096;
    // 兜底 TTL：进程内主动失效之外，最坏的陈旧窗口就是这个值。
    // Fallback TTL: beyond explicit in-process invalidation, this is the worst-case staleness
    // window.
    static constexpr unsigned int ttl_seconds = 60;

    route_cache();

    // 判定用户是否有权访问带 mask_code 的该 urlpath。只由路由钩子（route_guard）调用，
    // 会话键由调用方读好后传进来，这里不碰 session。
    // Decide whether the user may access this urlpath with mask_code. Called only by the route
    // hook (route_guard); the caller reads the session keys and passes them in, this layer never
    // touches the session.
    // 返回 false 时由调用方写 403。
    // When it returns false, the caller writes 403.
    bool allow(unsigned int adminid,
               unsigned int companyid,
               const std::string &urlpath,
               unsigned int mask_code);

    // 全量失效：权限目录 / 角色授权 / 角色状态变化后调用。O(1) 提升代次，再清表。
    // Invalidate everything: call after permission catalog, role grant or role status changes.
    // The generation bump is O(1); the shards are cleared afterwards.
    void invalidate_all();

    // 单用户失效：用户与角色绑定、用户自身所属租户或历史 roleid 变化后调用。
    // Invalidate one user: call after user-role binding changes, or when the user's own tenant
    // or historical roleid changes.
    void invalidate_user(unsigned int adminid, unsigned int companyid);

    // Snapshot of the counters; read-only and lock-free.
    cache_stats stats() const;

  private:
    struct cache_key
    {
        unsigned int adminid   = 0;
        unsigned int companyid = 0;
        bool operator==(const cache_key &other) const
        {
            return adminid == other.adminid && companyid == other.companyid;
        }
    };

    struct cache_key_hash
    {
        std::size_t operator()(const cache_key &k) const noexcept;
    };

    struct grant_entry
    {
        // 该用户生效的授权表：urlpath → 这些权限行各自的 maskcode，判定按精确相等逐条比
        // Effective grant table for this user: urlpath -> each permission row's maskcode;
        // decisions compare exact equality entry by entry.
        std::unordered_map<std::string, std::vector<unsigned int>> grants;
        std::time_t loaded_at = 0;// 装载时刻，用于 TTL
        // Load timestamp, used by the TTL check.
        std::uint64_t generation = 0;// 装载时代次，失效后不匹配即视为过期
        // Generation at load time; any mismatch after invalidation marks the entry stale.
    };

    struct shard_t
    {
        std::mutex mutex;
        std::unordered_map<cache_key, grant_entry, cache_key_hash> entries;
    };

    unsigned int shard_of(const cache_key &k) const;
    void evict_if_needed(shard_t &shard, std::time_t now, std::uint64_t gen);
    void log_deny(unsigned int adminid,
                  unsigned int companyid,
                  const std::string &urlpath,
                  unsigned int mask_code,
                  const std::string &reason) const;

    std::array<shard_t, shard_count> shards_;
    std::atomic<std::uint64_t> generation_{1};
    std::atomic<std::uint64_t> hit_{0};
    std::atomic<std::uint64_t> miss_{0};
    std::atomic<std::uint64_t> load_fail_{0};
    std::atomic<std::uint64_t> evict_{0};
    std::atomic<std::uint64_t> invalidate_all_{0};
    std::atomic<std::uint64_t> invalidate_user_{0};
    // 每请求逐条打印 HIT/MISS，用于压测与排查；默认关闭。
    // Print HIT/MISS per request for load testing and debugging; off by default.
    bool trace_ = false;
};

// 进程内单例。所有调用点（路由钩子、管理页写操作）共用同一份缓存。
// Process-wide singleton. All call sites (route hook, admin-page write operations) share this
// one cache.
route_cache &get_route_cache();

}// namespace rbac
}// namespace http
