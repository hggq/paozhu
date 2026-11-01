// 过期时间收归框架之后的行为判据：只吃 orm_cache.hpp 这一枚头文件，不连数据库。
#include "orm_cache.hpp"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

namespace
{
int g_fail = 0;

void report(const char *name, bool ok, const std::string &detail)
{
    std::printf("%s %s : %s\n", ok ? "PASS" : "FAIL", name, detail.c_str());
    if (!ok)
    {
        g_fail++;
    }
}
}// namespace

int main()
{
    auto &cache = orm::model_meta_cache<std::vector<std::string>>::getinstance();
    const std::vector<std::string> rows = {"a", "b"};

    // A1 业务不给档时走框架默认档，且绝不再是"0 = 永久"
    cache.save(1001, rows, 0);
    int c1 = cache.check(1001);
    report("A1_default_not_permanent", c1 > 0 && c1 <= ORM_CACHE_DEFAULT_TTL, "check=" + std::to_string(c1));

    // A2 上界钳死
    cache.save(1002, rows, 999999);
    int c2 = cache.check(1002);
    report("A2_ttl_clamped", c2 > ORM_CACHE_MAX_TTL - 60 && c2 <= ORM_CACHE_MAX_TTL, "check=" + std::to_string(c2));

    // A3 正数仍按业务给的档（取一个与默认档不同的值，才分得开"钳制"和"一律默认"）
    cache.save(1003, rows, 15);
    int c3 = cache.check(1003);
    report("A3_positive_hint_kept", c3 > 13 && c3 <= 15, "check=" + std::to_string(c3));

    // A4 update(键, 0) 这个"刷成永久"的口子封掉：返回 1，且落回框架默认档。
    // 种子档必须明显高于默认档（300 > 120），否则"刷回默认 = 变短"这件事证不到。
    cache.save(1004, rows, 300);
    int before4 = cache.check(1004);
    int rc4     = cache.update(1004, 0);
    int c4      = cache.check(1004);
    report("A4_update_zero_expires",
           before4 > ORM_CACHE_DEFAULT_TTL && rc4 == 1 && c4 > 0 && c4 <= ORM_CACHE_DEFAULT_TTL,
           "before=" + std::to_string(before4) + " rc=" + std::to_string(rc4) + " check=" + std::to_string(c4));

    // A5 到期后清扫器真能收走（1 秒档 + 2 秒等待）
    cache.save(1005, rows, 1);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    cache.remove_exptime();
    bool gone5 = !cache.try_get(1005).has_value();
    report("A5_sweeper_collects", gone5, std::string("try_get=") + (gone5 ? "miss" : "hit"));

    // A6 默认档条目在有效期内必须还能命中（防"改成过期"过手把缓存整个打死）
    cache.save(1006, rows, 0);
    auto hit6 = cache.try_get(1006);
    report("A6_default_entry_hits", hit6.has_value() && hit6->size() == 2,
           std::string("hit=") + (hit6.has_value() ? "yes" : "no"));

    // A7 删哨兵时别把"键不存在"的两条返回一起改掉
    int c7  = cache.check(424242);
    int u7  = cache.update(424242, 60);
    report("A7_absent_codes", c7 == -1 && u7 == -1, "check=" + std::to_string(c7) + " update=" + std::to_string(u7));

    std::printf("RESULT %s fails=%d\n", g_fail == 0 ? "ALLPASS" : "HASFAIL", g_fail);
    return g_fail == 0 ? 0 : 1;
}
