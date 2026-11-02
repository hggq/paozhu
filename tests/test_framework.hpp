/*
 * test_framework.hpp — 项目自用极简单元测试框架
 * C++20 特性：std::source_location / concept / inline variable
 *
 * 设计约束：与项目现有 tests/ 风格对齐——退出码 0 全过、1 有失败；
 *          不强求 fixture、参数化、matchers；够用就行。
 * Design goals: align with existing tests/ style — exit 0 all pass, 1 any failure;
 *              no fixture, no parametrize, no matchers; keep it tiny.
 */

#pragma once

#include <concepts>
#include <exception>
#include <iostream>
#include <source_location>
#include <string>
#include <vector>

namespace test {

// —— 跨 TU 共享的计数器（C++17 inline variable，C++20 继续可用）——
// Shared counters across all translation units.
inline int pass_count = 0;
inline int fail_count = 0;

// —— 测试注册表（单例）——
// Test registry (singleton).
struct Registry {
    struct Entry {
        const char *name;
        void (*fn)();
    };

    std::vector<Entry> entries;

    static Registry &instance()
    {
        static Registry r;
        return r;
    }

    void add(const char *name, void (*fn)())
    {
        entries.push_back({name, fn});
    }

    int run_all()
    {
        for (auto &e : entries)
        {
            std::cout << "CASE " << e.name << " ... ";
            try
            {
                e.fn();
            }
            catch (const std::exception &ex)
            {
                std::cerr << "FAIL  uncaught exception: " << ex.what() << std::endl;
                fail_count++;
            }
            catch (...)
            {
                std::cerr << "FAIL  uncaught unknown exception" << std::endl;
                fail_count++;
            }
        }
        return fail_count;
    }
};

// —— C++20 concept：可比较相等性的类型（给 REQUIRE_EQ/NE 做约束）——
// C++20 concept: equality-comparable types, used to constrain REQUIRE_EQ/NE.
template <typename T, typename U>
concept equality_comparable_with = requires(const T &a, const U &b) {
    { a == b } -> std::convertible_to<bool>;
};

// —— 内部：打印失败位置（用 std::source_location 替代 __FILE__/__LINE__）——
// Internal: print failure location via std::source_location instead of __FILE__/__LINE__.
inline void fail_location(const std::source_location &loc)
{
    std::cerr << "\n  FAIL "
              << loc.file_name() << ":" << loc.line()
              << "  ";
}

} // namespace test


// —— 注册宏：利用静态对象构造时自动注册 ——
// Registration via a dummy static object's constructor.
#define TEST_CASE(name)                                                       \
    static void test_##name();                                                \
    static struct Reg_##name {                                                \
        Reg_##name() { ::test::Registry::instance().add(#name, test_##name); } \
    } g_reg_##name;                                                           \
    static void test_##name()

// —— 基础断言 ——
// Basic assertion.
#define REQUIRE(expr)                                                         \
    do {                                                                      \
        if (!(expr)) {                                                        \
            ::test::fail_location(std::source_location::current());           \
            std::cerr << "REQUIRE(" #expr ")" << std::endl;                   \
            ::test::fail_count++;                                             \
        } else {                                                              \
            ::test::pass_count++;                                              \
        }                                                                     \
    } while (0)

// —— 相等断言：额外打印期望值 vs 实际值 ——
// Equality assertion: prints expected vs actual on failure.
#define REQUIRE_EQ(a, b)                                                      \
    do {                                                                      \
        auto _req_a = (a);                                                    \
        auto _req_b = (b);                                                    \
        if (!(_req_a == _req_b)) {                                            \
            ::test::fail_location(std::source_location::current());           \
            std::cerr << "REQUIRE_EQ(" #a ", " #b ")"                         \
                      << "  expected=" << _req_a                              \
                      << "  actual=" << _req_b << std::endl;                  \
            ::test::fail_count++;                                             \
        } else {                                                              \
            ::test::pass_count++;                                              \
        }                                                                     \
    } while (0)

// —— 不等断言 ——
// Inequality assertion.
#define REQUIRE_NE(a, b)                                                      \
    do {                                                                      \
        auto _req_a = (a);                                                    \
        auto _req_b = (b);                                                    \
        if (_req_a == _req_b) {                                               \
            ::test::fail_location(std::source_location::current());           \
            std::cerr << "REQUIRE_NE(" #a ", " #b ")"                         \
                      << "  got=" << _req_a << std::endl;                     \
            ::test::fail_count++;                                             \
        } else {                                                              \
            ::test::pass_count++;                                              \
        }                                                                     \
    } while (0)

// —— 主函数宏：跑全部已注册的 TEST_CASE，输出 SUMMARY，返回 0/1 ——
// Main macro: run all registered cases, print SUMMARY, return 0 or 1.
#define TEST_MAIN()                                                           \
    int main() {                                                              \
        int _fail = ::test::Registry::instance().run_all();                   \
        std::cout << "\n=== SUMMARY: "                                        \
                  << (::test::pass_count) << " passed, "                      \
                  << (::test::fail_count) << " failed ==="                    \
                  << std::endl;                                               \
        return _fail == 0 ? 0 : 1;                                            \
    }


/*
 * Usage example:
 *
 *   #include "test_framework.hpp"
 *
 *   TEST_CASE(basic_math) {
 *       REQUIRE(1 + 1 == 2);
 *       REQUIRE_EQ(2 + 3, 5);
 *       REQUIRE_NE(1, 2);
 *   }
 *
 *   TEST_CASE(string_ops) {
 *       REQUIRE_EQ(std::string("abc").size(), 3u);
 *   }
 *
 *   TEST_MAIN()
 *
 * Build:  cmake .. -DENABLE_TESTS=ON && make
 * Run:    ./test_my_feature      (exit 0 if all pass, 1 if any fail)
 * Or:     ctest --output-on-failure
 */
