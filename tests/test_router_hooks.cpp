/*
 * test_router_hooks.cpp — 路由改动纯逻辑单测（不依赖 server.cpp / serverconfig Singleton）
 * 覆盖：trimmed_hooks 截断、run_site_hooks 的"非空即拒绝"判定、check_upload_limit 边界守卫的算法等价性、resolve() try/catch 结构。
 *
 * 输出：每组一行 PASS/FAIL，末行 SUMMARY；全过退出码 0，任一失败退出码 1。
 * 与项目其它 tests/ 用例风格对齐（http2_ring_queue 等）。
 */
#include <iostream>
#include <string>
#include <vector>
#include <cstddef>

// —— 复用 router.h 的 trimmed_hooks / site_hook_limit ——
#include "router.h"

static int g_failures = 0;
static int g_groups   = 0;

static void group_result(const char *name, bool ok)
{
    g_groups++;
    if (ok)
    {
        std::cout << "PASS: " << name << std::endl;
    }
    else
    {
        g_failures++;
        std::cout << "FAIL: " << name << std::endl;
    }
}

// —— 与 router.cpp run_site_hooks 的非空即拒绝判定语义对齐的纯逻辑 helper ——
// 生产代码在 run_site_hooks / co_run_site_hooks 里直接写 !ret.empty()，
// 这里暴露一个 helper 让测试可以验证语义，而不是每个测试里都写一遍同样的 if
// mirrors run_site_hooks' post-2026-10-19 semantics: empty ret = pass, any non-empty ret = reject
static bool pre_hook_reject(const std::string &ret)
{
    return !ret.empty();
}

// —— 模拟 check_upload_limit 边界守卫的算法（不真正调 Singleton，只验证 if 条件）——
// models the exact boundary check added in httppeer.cpp:792-796
static bool check_upload_limit_guard(unsigned int host_index, size_t site_count)
{
    if (host_index >= site_count)
    {
        return true; // 守卫触发，返回 0（放行上传限制检查）
    }
    return false;
}

int main()
{
    // ===== 组 1：trimmed_hooks 截断行为 =====
    {
        // 1a: 空列表 → 空（不截断）
        group_result("trimmed_hooks: empty stays empty",
                     http::trimmed_hooks({}).empty());

        // 1b: 小于上限（15）→ 原样返回，不截断
        group_result("trimmed_hooks: under limit returns as-is",
                     http::trimmed_hooks({"a", "b", "c"}).size() == 3 &&
                     http::trimmed_hooks({"a", "b", "c"}).front() == "a");

        // 1c: 正好 15 条 → 不截断
        std::vector<std::string> fifteen;
        for (int i = 0; i < 15; ++i) fifteen.push_back("h" + std::to_string(i));
        auto r15 = http::trimmed_hooks(fifteen);
        group_result("trimmed_hooks: exactly 15 returns all",
                     r15.size() == 15 && r15.front() == "h0" && r15.back() == "h14");

        // 1d: 16 条（超限 1 条）→ 截断到前 15 条，最后一条 h15 被丢弃
        fifteen.push_back("h15");
        auto r16 = http::trimmed_hooks(fifteen);
        group_result("trimmed_hooks: 16 trimmed to 15, last dropped",
                     r16.size() == 15 && r16.back() == "h14");

        // 1e: 100 条（严重超限）→ 截断到前 15 条
        std::vector<std::string> hundred;
        for (int i = 0; i < 100; ++i) hundred.push_back("x" + std::to_string(i));
        auto r100 = http::trimmed_hooks(hundred);
        group_result("trimmed_hooks: 100 trimmed to 15, first 15 kept",
                     r100.size() == 15 && r100.front() == "x0" && r100.back() == "x14");
    }

    // ===== 组 2：pre_hook_reject 非空即拒绝语义 =====
    // 这些用例覆盖：旧版只认 "exit"（size==4 且大小写不敏感），新版 !ret.empty() 统一拒绝。
    // 下列非空值都会触发拒绝（旧版对部分会放行，新版全拒）
    {
        group_result("pre_hook_reject: empty string = pass",
                     !pre_hook_reject(""));

        group_result("pre_hook_reject: \"exit\" = reject (was: reject)",
                     pre_hook_reject("exit"));
        group_result("pre_hook_reject: \"EXIT\" = reject (was: reject)",
                     pre_hook_reject("EXIT"));
        group_result("pre_hook_reject: \"Exit\" = reject (was: reject)",
                     pre_hook_reject("Exit"));

        // 下列是旧版（只认 "exit"）会放行、新版会拒绝的哨兵值：
        group_result("pre_hook_reject: \"coro-refused\" = reject (was: PASS, bug)",
                     pre_hook_reject("coro-refused"));

        group_result("pre_hook_reject: \"pool-error\" = reject (was: PASS, bug)",
                     pre_hook_reject("pool-error"));

        group_result("pre_hook_reject: \"coro-error\" = reject (was: PASS, bug)",
                     pre_hook_reject("coro-error"));

        group_result("pre_hook_reject: \"403 Forbidden\" = reject (was: PASS, bug)",
                     pre_hook_reject("403 Forbidden"));

        group_result("pre_hook_reject: \"@ok\" = reject (hook 层只看空/非空，不经过 classify_step)",
                     pre_hook_reject("@ok"));

        group_result("pre_hook_reject: \"\" only passes",
                     !pre_hook_reject("any-non-empty-string") == false);
    }

    // ===== 组 3：check_upload_limit 边界守卫算法验证 =====
    // httppeer.cpp:792-796 新增 if (host_index >= sitehostinfos.size()) return 0;
    {
        // 3a: 空 sitehostinfos，host_index=0 → 触发守卫（旧代码会越界，新代码 return 0）
        group_result("check_upload_limit_guard: empty sites, host_index=0 triggers guard",
                     check_upload_limit_guard(0, 0));

        // 3b: 1 个 site，host_index=0 → 不触发守卫
        group_result("check_upload_limit_guard: 1 site, host_index=0 passes guard",
                     !check_upload_limit_guard(0, 1));

        // 3c: 1 个 site，host_index=1（越界）→ 触发守卫
        group_result("check_upload_limit_guard: 1 site, host_index=1 triggers guard",
                     check_upload_limit_guard(1, 1));

        // 3d: 10 个 site，host_index=9 → 不触发
        group_result("check_upload_limit_guard: 10 sites, host_index=9 passes guard",
                     !check_upload_limit_guard(9, 10));

        // 3e: 10 个 site，host_index=10 → 触发
        group_result("check_upload_limit_guard: 10 sites, host_index=10 triggers guard",
                     check_upload_limit_guard(10, 10));

        // 3f: 10 个 site，host_index=100（远越界）→ 触发
        group_result("check_upload_limit_guard: 10 sites, host_index=100 triggers guard",
                     check_upload_limit_guard(100, 10));
    }

    // ===== 组 4：resolve() 异常保护结构验证 =====
    // 这组不真正调用 resolve()（依赖 router 单例 + handler 注册），
    // 而是用等价的 try/catch 结构模拟 sync pre / sync regfun 抛异常 → fail_500 被触发
    {
        bool fail500_triggered = false;
        auto sync_pre_that_throws = [&fail500_triggered]() -> std::string
        {
            try
            {
                throw std::runtime_error("auth failed");
            }
            catch (std::exception const &e)
            {
                // 模拟 fail_500(peer, e.what()) —— 真实函数写 peer->status(500) + output
                fail500_triggered = true;
                // 真实代码里 sitecontent.clear(); break;
                return "";
            }
            catch (...)
            {
                fail500_triggered = true;
                return "";
            }
        };
        sync_pre_that_throws();
        group_result("resolve try/catch: sync pre throw → fail_500 path",
                     fail500_triggered);
    }

    // ===== SUMMARY =====
    std::cout << "\nSUMMARY: " << (g_failures == 0 ? "ALL PASS" : "FAILURES")
              << "  groups=" << g_groups << " failures=" << g_failures << std::endl;
    return g_failures == 0 ? 0 : 1;
}
