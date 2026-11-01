// S6 代价读数：同一枚不存在的键，冷 miss 分别走「get() + try/catch」与「try_get()」各 N 次。
// 只依赖 orm_cache.hpp，不需要生成物；这是量，不是判据。
#include "orm_cache.hpp"

#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

using cache_t = orm::model_meta_cache<std::vector<std::string>>;

int main()
{
    constexpr int N = 2000000;
    cache_t &cache  = cache_t::getinstance();

    // 槽里先放一条活的，保证两条计时路都真的走到「找到 key -> 比到期」之外的分支差为零。
    // 测的是 miss，所以用两个都不存在的键。
    const std::size_t miss_a = 900000001;
    const std::size_t miss_b = 900000002;

    auto t0 = std::chrono::steady_clock::now();
    long caught = 0;
    for (int i = 0; i < N; i++)
    {
        try
        {
            auto rows = cache.get(miss_a);
            (void)rows;
        }
        catch (const std::exception &)
        {
            caught++;
        }
    }
    auto t1 = std::chrono::steady_clock::now();
    long hits = 0;
    for (int i = 0; i < N; i++)
    {
        if (cache.try_get(miss_b))
        {
            hits++;
        }
    }
    auto t2 = std::chrono::steady_clock::now();

    double ns_throw = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / double(N);
    double ns_opt   = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / double(N);
    printf("N=%d caught=%ld hits=%ld\n", N, caught, hits);
    printf("COLD_MISS_THROW_NS_PER_OP=%.1f\n", ns_throw);
    printf("COLD_MISS_TRYGET_NS_PER_OP=%.1f\n", ns_opt);
    printf("SAVED_NS_PER_COLD_READ=%.1f ratio=%.1fx\n", ns_throw - ns_opt, ns_throw / ns_opt);
    return (caught == N && hits == 0) ? 0 : 1;
}
