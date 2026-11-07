#include "route_cache.h"

#include <cstdlib>
#include <iostream>

#include "rbac.h"

namespace http
{
namespace rbac
{
namespace
{
// Catalog load for one user costs a fixed six statements:
// sysuserrole -> sysuser -> sysrole -> sysroleperms -> syscompanyperms -> sysperms.
// An empty allocation set skips the final sysperms query, so that path costs five.
// 单个用户的权限目录装载固定需要 6 条语句：
// sysuserrole -> sysuser -> sysrole -> sysroleperms -> syscompanyperms -> sysperms。
// 企业的授权集合为空时会跳过最后一条 sysperms 查询，因此该路径只花 5 条。
constexpr unsigned int load_db_queries = 6;

// Whether the grant table holds a permission for this urlpath whose maskcode exactly equals the
// route entry's mask. A mismatch means the maskcode in the permission catalog has drifted from the
// mask registered in code: deny the request and report the reason.
// 判断授权表中是否存在该 urlpath 的权限项，且其 maskcode 与路由条目的 mask 精确相等。
// 不相等说明权限目录里的 maskcode 已和代码里注册的掩码发生漂移：拒绝请求并给出原因。
bool grant_matches(const std::unordered_map<std::string, std::vector<unsigned int>> &grants,
                   const std::string &urlpath,
                   unsigned int mask_code,
                   std::string &reason)
{
    auto it = grants.find(urlpath);
    if (it == grants.end())
    {
        reason = "not in cached grant set";
        return false;
    }
    for (auto const &granted : it->second)
    {
        if (granted == mask_code)
        {
            return true;
        }
    }
    std::string have;
    for (unsigned int granted : it->second)
    {
        if (!have.empty())
        {
            have += "|";
        }
        have += std::to_string(granted);
    }
    reason = "maskcode drift, granted=" + have + " route=" + std::to_string(mask_code);
    return false;
}
}// namespace

std::size_t route_cache::cache_key_hash::operator()(const cache_key &k) const noexcept
{
    // Pack the two 32-bit fields into 64 bits, then mix once so the low field cannot flatten the
    // shard distribution.
    // 把两个 32 位字段打包进 64 位，再做一次混合，避免低位字段（companyid）把分片分布压平。
    std::uint64_t v = (static_cast<std::uint64_t>(k.adminid) << 32) | static_cast<std::uint64_t>(k.companyid);
    v ^= v >> 33;
    v *= 0xff51afd7ed558ccdULL;
    v ^= v >> 33;
    v *= 0xc4ceb9fe1a85ec53ULL;
    v ^= v >> 33;
    return static_cast<std::size_t>(v);
}

route_cache::route_cache()
{
    // Read the environment variable exactly once at construction; trace is a pure diagnostic
    // switch, so a disabled flag costs zero formatting work.
    // 构造时只读一次环境变量；trace 是纯诊断开关，关闭时不会产生任何格式化开销。
    const char *trace = std::getenv("PAOZHU_RBAC_CACHE_TRACE");
    trace_            = (trace != nullptr && trace[0] != '\0' && trace[0] != '0');
}

unsigned int route_cache::shard_of(const cache_key &k) const
{
    return static_cast<unsigned int>(cache_key_hash{}(k) % shard_count);
}

void route_cache::log_deny(unsigned int adminid,
                           unsigned int companyid,
                           const std::string &urlpath,
                           unsigned int mask_code,
                           const std::string &reason) const
{
    // Denials never re-query sysperms: the reason comes from this cache decision, so it costs
    // zero queries and cannot reach the database even under request flooding.
    // 拒绝路径从不重新查询 sysperms：原因来自本次缓存判定，因此零查询开销，
    // 即使请求洪峰也打不到数据库。
    std::cerr << "[RBAC-DENY] adminid=" << adminid << " companyid=" << companyid
              << " urlpath=" << urlpath << " mask=" << mask_code
              << " reason=" << reason << std::endl;
}

void route_cache::evict_if_needed(shard_t &shard, std::time_t now, std::uint64_t gen)
{
    constexpr unsigned int shard_capacity = max_entries / shard_count;
    if (shard.entries.size() < shard_capacity)
    {
        return;
    }

    // Drop expired or invalidated entries first; when the table is full, these are usually the
    // rows holding the slots.
    // 先清掉已过期或已失效的条目；表满时，占着槽位的基本就是这些行。
    for (auto it = shard.entries.begin(); it != shard.entries.end();)
    {
        bool expired = now - it->second.loaded_at >= static_cast<std::time_t>(ttl_seconds);
        if (expired || it->second.generation != gen)
        {
            it = shard.entries.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (shard.entries.size() < shard_capacity)
    {
        return;
    }

    // Still full: evict the oldest-loaded entry in this shard. A shard holds only 256 entries, so
    // a linear scan is cheap enough.
    // 仍然满：淘汰本分片中装载时间最旧的一条。单个分片只有 256 个条目，线性扫描足够便宜。
    auto oldest = shard.entries.begin();
    for (auto it = shard.entries.begin(); it != shard.entries.end(); ++it)
    {
        if (it->second.loaded_at < oldest->second.loaded_at)
        {
            oldest = it;
        }
    }
    shard.entries.erase(oldest);
    ++evict_;
}

bool route_cache::allow(unsigned int adminid,
                        unsigned int companyid,
                        const std::string &urlpath,
                        unsigned int mask_code)
{
    // companyid==0 is not a valid tenant: deny here instead of running the six queries that are
    // guaranteed to come back empty.
    // companyid==0 不是合法租户：直接在这里拒绝，不必执行那 6 条注定返回空结果的查询。
    if (adminid == 0 || companyid == 0 || urlpath.empty())
    {
        return false;
    }

    std::string deny_reason;
    cache_key key{adminid, companyid};
    shard_t &shard = shards_[shard_of(key)];

    // The shard lock is held for the whole load: concurrent requests in the same shard produce at
    // most one database load (singleflight, no stampede). The cost is that other users in the shard
    // wait for those six queries; 16 shards spread that contention out.
    // 装载全程持有分片锁：同一分片内的并发请求最多只触发一次数据库装载
    // （singleflight，不会击穿）。代价是同分片的其他用户要等这 6 条查询；
    // 16 个分片把这部分争用摊开。
    std::lock_guard<std::mutex> lock(shard.mutex);

    std::time_t now   = std::time(nullptr);
    std::uint64_t gen = generation_.load(std::memory_order_acquire);

    auto it = shard.entries.find(key);
    if (it != shard.entries.end() && it->second.generation == gen &&
        now - it->second.loaded_at < static_cast<std::time_t>(ttl_seconds))
    {
        ++hit_;
        bool allowed = grant_matches(it->second.grants, urlpath, mask_code, deny_reason);
        if (trace_)
        {
            std::cerr << "[RBAC-CACHE] HIT adminid=" << adminid << " companyid=" << companyid
                      << " urlpath=" << urlpath << " allow=" << (allowed ? 1 : 0) << std::endl;
        }
        if (!allowed)
        {
            log_deny(adminid, companyid, urlpath, mask_code, deny_reason);
        }
        return allowed;
    }

    ++miss_;

    // Read the generation before loading from the database. If an invalidation happens meanwhile,
    // this entry carries the old generation and is reloaded on the next request. The reverse order
    // (load first, then read the generation) would tag pre-write data as current, keeping it stale
    // for up to the full TTL.
    // 先读代次，再从数据库装载。若期间发生失效，这条记录带的是旧代次，下次请求会重新装载。
    // 反过来（先装载、后读代次）会把写入前的数据标记为最新，最多要让旧数据残留一整个 TTL。
    std::unordered_map<std::string, std::vector<unsigned int>> grants;
    bool ok = false;
    try
    {
        ok = load_user_grants(adminid, companyid, grants);
    }
    catch (const char *e)
    {
        std::cerr << "[RBAC-CACHE] load exception: " << e << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[RBAC-CACHE] load exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[RBAC-CACHE] load exception: unknown" << std::endl;
    }

    if (!ok)
    {
        // Database failure is fail-closed: write nothing to the cache and reuse no stale value.
        // Once the database recovers, the next request reloads immediately.
        // 数据库失败时 fail-closed：不写缓存、也不复用任何过期值。
        // 数据库恢复后，下一次请求会立即重新装载。
        ++load_fail_;
        std::cerr << "[RBAC-CACHE] load failed adminid=" << adminid << " companyid=" << companyid
                  << " -> fail-closed (deny)" << std::endl;
        return false;
    }

    bool is_new = (it == shard.entries.end());
    if (is_new)
    {
        evict_if_needed(shard, now, gen);
    }
    shard.entries[key] = grant_entry{std::move(grants), now, gen};

    auto &loaded = shard.entries[key].grants;
    bool allowed = grant_matches(loaded, urlpath, mask_code, deny_reason);
    if (trace_)
    {
        std::cerr << "[RBAC-CACHE] MISS adminid=" << adminid << " companyid=" << companyid
                  << " urlpath=" << urlpath << " loaded=" << loaded.size()
                  << " db=" << load_db_queries << " allow=" << (allowed ? 1 : 0) << std::endl;
    }
    if (!allowed)
    {
        log_deny(adminid, companyid, urlpath, mask_code, deny_reason);
    }
    return allowed;
}

void route_cache::invalidate_all()
{
    // Bump the generation before clearing the shards: anything inserted during the clear either
    // carries the new generation (it was read after the write, so it is safe) or carries the old
    // one and is cleared afterwards. Worst case is one extra reload.
    // 先递增代次再清空分片：清理期间插入的条目，要么带新代次
    // （它是在读代次之后才写入的，因而是安全的），要么带旧代次并在随后被清掉。
    // 最坏情况只是多一次重新装载。
    std::uint64_t gen = generation_.fetch_add(1, std::memory_order_acq_rel) + 1;

    unsigned int removed = 0;
    for (auto &shard : shards_)
    {
        std::lock_guard<std::mutex> lock(shard.mutex);
        removed += static_cast<unsigned int>(shard.entries.size());
        shard.entries.clear();
    }
    ++invalidate_all_;
    std::cerr << "[RBAC-CACHE] invalidate all gen=" << gen << " removed=" << removed << std::endl;
}

void route_cache::invalidate_user(unsigned int adminid, unsigned int companyid)
{
    if (adminid == 0)
    {
        return;
    }
    cache_key key{adminid, companyid};
    shard_t &shard       = shards_[shard_of(key)];
    unsigned int removed = 0;
    {
        std::lock_guard<std::mutex> lock(shard.mutex);
        removed = static_cast<unsigned int>(shard.entries.erase(key));
    }
    ++invalidate_user_;
    std::cerr << "[RBAC-CACHE] invalidate user adminid=" << adminid << " companyid=" << companyid
              << " removed=" << removed << std::endl;
}

cache_stats route_cache::stats() const
{
    cache_stats s;
    s.hit             = hit_.load();
    s.miss            = miss_.load();
    s.load_fail       = load_fail_.load();
    s.evict           = evict_.load();
    s.invalidate_all  = invalidate_all_.load();
    s.invalidate_user = invalidate_user_.load();
    return s;
}

route_cache &get_route_cache()
{
    static route_cache instance;
    return instance;
}

}// namespace rbac
}// namespace http
