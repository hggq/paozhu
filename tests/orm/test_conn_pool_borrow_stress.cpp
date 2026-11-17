/*
 * test_conn_pool_borrow_stress.cpp — ORM 连接池跨槽借用并发压测
 *
 * 验证 ORM 连接池跨槽借用的并发行为：
 * 自托管 4 ioc，协程 + 同步混合 get/back，默认 300 秒（argv[1] 可改）。
 * 前一半只跑协程，断言 Σ total_count <= max_pool（每角色）；
 * 后一半加入同步 worker（同步路径允许超建，只记录峰值）。
 * 借用归因：g_init_mtx 串行化"发起"，发起时复刻选槽决策（rr 候选 + 半环外候选
 * 取 idle_count 多者，相等回退 rr 候选），conn_link->slot_index != 决策槽
 * 即跨槽借用（协程/同步分别计数）。决策与实际 get 之间计数可能变化，归因含少量竞态噪声。
 * 判据：errors==0（含 wait timeout）、quota_violations==0、borrows>0（闭环模式除外）、
 * 进程正常退出；stderr 重定向到日志后 grep "duplicate return" 必须为 0。
 * argv[4]=closed：闭环零持有模式，worker 等上一次 get→back 完成才发下一次，
 * 在途数 ≤ worker 数，gets/s 反映取还路径单次开销而非池容量。
 * 编译期 -DORM_POOL_PRE_IDLE_COUNT：归因退回纯 rr（给改前对照臂用，头文件无 idle_count）。
 */
#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/steady_timer.hpp>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "orm_conn_pool.h"
#include "mysql_conn.h"

using namespace orm;
using clk = std::chrono::steady_clock;

static const char *kConfPath = "temp/test_conn_pool_borrow_stress.conf";
static const char *kTag      = "borrowstress";

// argv[2]/argv[3] 覆盖 edit/select 的 maxpool（默认 32/32 平衡档；8/16 为超载对照档）
// argv[5]/argv[6] 覆盖 edit/select 的 minpool（每槽预建数，默认 0/0 全懒建）
static std::string conf_text(int max_edit, int max_select, int min_edit, int min_select)
{
    std::string s = R"([borrowstress]
type = main
host = 127.0.0.1
port = 3306
dbname = hello_world
user = benchmarkdbuser
password = benchmarkdbpass
minpool = )";
    s += std::to_string(min_edit);
    s += R"(
maxpool = )";
    s += std::to_string(max_edit);
    s += R"(
dbtype = mysql
charset = utf8mb4

type = second
host = 127.0.0.1
port = 3306
dbname = hello_world
user = benchmarkdbuser
password = benchmarkdbpass
minpool = )";
    s += std::to_string(min_select);
    s += R"(
maxpool = )";
    s += std::to_string(max_select);
    s += R"(
dbtype = mysql
charset = utf8mb4
)";
    return s;
}

static std::atomic<bool> g_async_only{true};
static std::atomic<long long> g_gets{0}, g_borrows_async{0}, g_borrows_sync{0};
static std::atomic<long long> g_errors{0}, g_quota_viol{0};
static std::atomic<unsigned int> g_peak_edit{0}, g_peak_select{0};
static std::mutex g_init_mtx;// 串行化 get 发起，保证 rr 归因精确
static std::mutex g_err_mtx;
static std::string g_first_err;
static clk::time_point g_deadline;
// 闭环模式（argv[4]=closed）：每个 worker 等上一次 get→back 完成才发下一次，
// 零持有。在途数 ≤ worker 数，远低于配额，不触发门闩；gets/s 直接反映取还路径
// 单次开销，而不是开环 firehose 下的池容量。
static bool g_closed = false;
static std::atomic<int> g_closed_done{0};

static void record_err(const std::string &msg)
{
    g_errors.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard<std::mutex> l(g_err_mtx);
    if (g_first_err.empty())
        g_first_err = msg;
}

static void note_borrow(unsigned int actual_slot, unsigned int expected_slot, bool is_sync)
{
    if (actual_slot != expected_slot)
    {
        if (is_sync)
            g_borrows_sync.fetch_add(1, std::memory_order_relaxed);
        else
            g_borrows_async.fetch_add(1, std::memory_order_relaxed);
    }
}

// 复刻选槽决策做借用归因。改前对照臂（无 idle_count）编译期退回纯 rr。
static unsigned int pick_expected(orm_conn_pool *pool, unsigned int before, unsigned int n,
                                  unsigned int role)
{
#ifdef ORM_POOL_PRE_IDLE_COUNT
    (void)pool;
    (void)role;
    return before % n;
#else
    const unsigned int a = before % n;
    const unsigned int b = (a + (n >> 1)) % n;
    return pool->slots[b]->idle_count[role].load(std::memory_order_relaxed) >
                   pool->slots[a]->idle_count[role].load(std::memory_order_relaxed)
               ? b
               : a;
#endif
}

// 协程 worker：每轮发起一个 get-持有-归还 协程，发起段在 g_init_mtx 内完成归因。
static void async_worker(orm_conn_pool *pool, asio::io_context &ioc, unsigned int seed)
{
    unsigned int rng = seed * 2654435761u + 1u;
    const unsigned int n = static_cast<unsigned int>(pool->slots.size());
    while (clk::now() < g_deadline)
    {
        rng = rng * 1664525u + 1013904223u;
        const bool use_edit = (rng & 1u) != 0;
        const unsigned int hold_ms = 1u + (rng >> 8) % 3u;

        unsigned int expected;
        {
            std::lock_guard<std::mutex> l(g_init_mtx);
            const unsigned int before = pool->rr_counter.load(std::memory_order_relaxed);
            expected                  = pick_expected(pool, before, n, use_edit ? 0u : 1u);
            asio::co_spawn(ioc,
                           [pool, use_edit, hold_ms, expected]() -> asio::awaitable<void>
                           {
                               std::shared_ptr<mysql_conn_base> c;
                               try
                               {
                                   c = use_edit ? co_await pool->async_get_mysql_edit_conn()
                                                : co_await pool->async_get_mysql_select_conn();
                               }
                               catch (const std::exception &e)
                               {
                                   record_err(std::string("async get: ") + e.what());
                                   co_return;
                               }
                               g_gets.fetch_add(1, std::memory_order_relaxed);
                               note_borrow(c->conn_link->slot_index, expected, false);
                               asio::steady_timer t(co_await asio::this_coro::executor);
                               t.expires_after(std::chrono::milliseconds(hold_ms));
                               co_await t.async_wait(asio::use_awaitable);
                               if (use_edit)
                                   pool->back_mysql_edit_conn(std::move(c));
                               else
                                   pool->back_mysql_select_conn(std::move(c));
                           },
                           asio::detached);
            // fetch_add 发生在协程体开头（executor await 立即完成）；rr 变了才算发起落地
            while (pool->rr_counter.load(std::memory_order_relaxed) == before)
                std::this_thread::yield();
        }
    }
}

// 闭环协程 worker：get→立即 back，无持有定时器，循环到 deadline。
// 归因在 get 前复刻选槽决策；读 rr 与 get 内 fetch_add 之间有其他 worker 插队，
// 归因含少量竞态噪声（借用数只作参考，不作判据）。
static asio::awaitable<void> closed_async_loop(orm_conn_pool *pool, unsigned int seed)
{
    unsigned int rng = seed * 2654435761u + 1u;
    const unsigned int n = static_cast<unsigned int>(pool->slots.size());
    while (clk::now() < g_deadline)
    {
        rng = rng * 1664525u + 1013904223u;
        const bool use_edit = (rng & 1u) != 0;
        const unsigned int role = use_edit ? 0u : 1u;
        const unsigned int before = pool->rr_counter.load(std::memory_order_relaxed);
        const unsigned int expected = pick_expected(pool, before, n, role);
        std::shared_ptr<mysql_conn_base> c;
        try
        {
            c = use_edit ? co_await pool->async_get_mysql_edit_conn()
                         : co_await pool->async_get_mysql_select_conn();
        }
        catch (const std::exception &e)
        {
            record_err(std::string("closed async get: ") + e.what());
            continue;
        }
        g_gets.fetch_add(1, std::memory_order_relaxed);
        note_borrow(c->conn_link->slot_index, expected, false);
        if (use_edit)
            pool->back_mysql_edit_conn(std::move(c));
        else
            pool->back_mysql_select_conn(std::move(c));
    }
    g_closed_done.fetch_add(1, std::memory_order_relaxed);
}

// 同步 worker：get 是同步 fetch_add，锁内直接归因。
static void sync_worker(orm_conn_pool *pool, unsigned int seed)
{
    unsigned int rng = seed * 2246822519u + 7u;
    const unsigned int n = static_cast<unsigned int>(pool->slots.size());
    while (clk::now() < g_deadline)
    {
        rng = rng * 1664525u + 1013904223u;
        const bool use_edit = (rng & 1u) != 0;
        std::shared_ptr<mysql_conn_base> c;
        unsigned int expected;
        {
            std::lock_guard<std::mutex> l(g_init_mtx);
            const unsigned int before = pool->rr_counter.load(std::memory_order_relaxed);
            expected                  = pick_expected(pool, before, n, use_edit ? 0u : 1u);
            try
            {
                c = use_edit ? pool->get_mysql_edit_conn() : pool->get_mysql_select_conn();
            }
            catch (const std::exception &e)
            {
                record_err(std::string("sync get: ") + e.what());
            }
        }
        if (!c)
            continue;
        g_gets.fetch_add(1, std::memory_order_relaxed);
        note_borrow(c->conn_link->slot_index, expected, true);
        if (!g_closed)// 闭环零持有：不 sleep，立即归还
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (use_edit)
            pool->back_mysql_edit_conn(std::move(c));
        else
            pool->back_mysql_select_conn(std::move(c));
    }
}

// 采样 Σ total_count：纯协程段断言不超 max_pool；两段都记录峰值。
static void monitor(orm_conn_pool *pool)
{
    const unsigned int max_edit   = pool->slot_max_of(0, 0) * static_cast<unsigned int>(pool->slots.size());
    const unsigned int max_select = pool->slot_max_of(1, 0) * static_cast<unsigned int>(pool->slots.size());
    while (clk::now() < g_deadline)
    {
        unsigned int sum_edit = 0, sum_select = 0;
        for (auto &s : pool->slots)
        {
            sum_edit += s->total_count[0]->load(std::memory_order_relaxed);
            sum_select += s->total_count[1]->load(std::memory_order_relaxed);
        }
        if (sum_edit > g_peak_edit.load(std::memory_order_relaxed))
            g_peak_edit.store(sum_edit, std::memory_order_relaxed);
        if (sum_select > g_peak_select.load(std::memory_order_relaxed))
            g_peak_select.store(sum_select, std::memory_order_relaxed);
        if (g_async_only.load() && (sum_edit > max_edit || sum_select > max_select))
        {
            g_quota_viol.fetch_add(1, std::memory_order_relaxed);
            record_err("quota: async-only sum edit=" + std::to_string(sum_edit) + "/" +
                       std::to_string(max_edit) + " select=" + std::to_string(sum_select) + "/" +
                       std::to_string(max_select));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main(int argc, char **argv)
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    const int total_sec = (argc > 1) ? std::atoi(argv[1]) : 300;
    const int max_edit  = (argc > 2) ? std::atoi(argv[2]) : 32;
    const int max_select = (argc > 3) ? std::atoi(argv[3]) : 32;
    g_closed            = (argc > 4 && std::string(argv[4]) == "closed");
    const int min_edit  = (argc > 5) ? std::atoi(argv[5]) : 0;
    const int min_select = (argc > 6) ? std::atoi(argv[6]) : 0;
    const int phase_sec = total_sec / 2;
    try
    {
        {
            std::ofstream f(kConfPath, std::ios::trunc);
            f << conf_text(max_edit, max_select, min_edit, min_select);
        }
        std::cout << init_orm_conn_pool(kConfPath, 4, 8) << "\n";
        auto pool = get_orm_conn_pool_obj()[kTag];
        if (!pool || pool->slots.size() != 4)
        {
            std::cout << "[FATAL] pool init failed\n";
            return 1;
        }
        std::cout << "stress: 4 slots, edit max/slot=" << pool->slot_max_of(0, 0)
                  << " select max/slot=" << pool->slot_max_of(1, 0)
                  << ", duration=" << total_sec << "s (A=" << phase_sec << "s async-only, B="
                  << total_sec - phase_sec << "s mixed)"
                  << (g_closed ? " [closed-loop zero-hold]" : "") << "\n";

        // 调用方 ioc 与池 ioc 分离，模拟真实请求协程不在池执行域里
        asio::io_context ioc1, ioc2;
        auto w1 = asio::make_work_guard(ioc1);
        auto w2 = asio::make_work_guard(ioc2);
        std::thread ioc1_th([&] { ioc1.run(); });
        std::thread ioc2_th([&] { ioc2.run(); });

        g_deadline = clk::now() + std::chrono::seconds(phase_sec);
        std::thread mon(monitor, pool.get());
        std::vector<std::thread> workers;
        if (g_closed)
        {
            for (int i = 0; i < 16; ++i)
                asio::co_spawn(i % 2 ? ioc2 : ioc1,
                               closed_async_loop(pool.get(), static_cast<unsigned int>(i + 1)),
                               asio::detached);
            std::cout << "phase A started (16 closed-loop coroutines)\n";
            while (g_closed_done.load(std::memory_order_relaxed) < 16)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
        {
            for (int i = 0; i < 16; ++i)
                workers.emplace_back(async_worker, pool.get(), std::ref(i % 2 ? ioc2 : ioc1),
                                     static_cast<unsigned int>(i + 1));
            std::cout << "phase A started (16 async workers)\n";
            for (auto &w : workers)
                w.join();
            workers.clear();
        }
        mon.join();
        std::cout << "phase A done: gets=" << g_gets.load()
                  << " borrows_async=" << g_borrows_async.load()
                  << " errors=" << g_errors.load() << " quota_viol=" << g_quota_viol.load() << "\n";

        g_async_only.store(false);
        g_deadline = clk::now() + std::chrono::seconds(total_sec - phase_sec);
        std::thread mon2(monitor, pool.get());
        if (g_closed)
        {
            g_closed_done.store(0, std::memory_order_relaxed);
            for (int i = 0; i < 16; ++i)
                asio::co_spawn(i % 2 ? ioc2 : ioc1,
                               closed_async_loop(pool.get(), static_cast<unsigned int>(i + 101)),
                               asio::detached);
            for (int i = 0; i < 8; ++i)
                workers.emplace_back(sync_worker, pool.get(), static_cast<unsigned int>(i + 1));
            std::cout << "phase B started (16 closed-loop coroutines + 8 sync workers)\n";
            while (g_closed_done.load(std::memory_order_relaxed) < 16)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            for (auto &w : workers)
                w.join();
            workers.clear();
        }
        else
        {
            for (int i = 0; i < 16; ++i)
                workers.emplace_back(async_worker, pool.get(), std::ref(i % 2 ? ioc2 : ioc1),
                                     static_cast<unsigned int>(i + 101));
            for (int i = 0; i < 8; ++i)
                workers.emplace_back(sync_worker, pool.get(), static_cast<unsigned int>(i + 1));
            std::cout << "phase B started (16 async + 8 sync workers)\n";
            for (auto &w : workers)
                w.join();
            workers.clear();
        }
        mon2.join();

        w1.reset();
        w2.reset();
        ioc1.stop();
        ioc2.stop();
        ioc1_th.join();
        ioc2_th.join();
    }
    catch (const std::exception &e)
    {
        std::cout << "[FATAL] " << e.what() << "\n";
        g_errors.fetch_add(1);
    }
    init_orm_conn_pool_release();

    const long long gets    = g_gets.load();
    const long long ba      = g_borrows_async.load();
    const long long bs      = g_borrows_sync.load();
    const long long errs    = g_errors.load();
    const long long viol    = g_quota_viol.load();
    std::cout << "\n==== borrow stress report ====\n"
              << "mode              : " << (g_closed ? "closed-loop zero-hold" : "open firehose") << "\n"
              << "total gets        : " << gets << "\n"
              << "throughput        : " << (total_sec ? gets / total_sec : 0) << " gets/s\n"
              << "borrows async     : " << ba << "\n"
              << "borrows sync      : " << bs << "\n"
              << "borrow rate       : " << (gets ? 100.0 * (ba + bs) / gets : 0.0) << "%\n"
              << "errors            : " << errs << (errs ? " first: " + g_first_err : "") << "\n"
              << "quota violations  : " << viol << " (phase A only)\n"
              << "peak total edit   : " << g_peak_edit.load() << " (max_pool=" << max_edit << ")\n"
              << "peak total select : " << g_peak_select.load() << " (max_pool=" << max_select << ")\n";
    // 闭环模式测的是路径开销，全本槽命中（借用=0）是正常结果，不作判据。
    const bool ok = gets > 0 && errs == 0 && viol == 0 && (g_closed || (ba + bs) > 0);
    std::cout << "==== " << (ok ? "PASS" : "FAIL") << " ====\n";
    return ok ? 0 : 1;
}