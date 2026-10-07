/*
 * test_redis_bench.cpp - Redis async_str_set / async_str_get stress test
 * author Huang ziquan (黄自权)
 * date 2026-10-02
 *
 * Routes:
 *   redis/bench/sync     - sync client (std::string handler, business thread pool)
 *   redis/bench/async    - coroutine client sequential (asio::awaitable handler)
 *   redis/bench/async_concurrent - coroutine concurrent with fan-out
 *   redis/bench/pool     - direct redis_pool::async_exec_obj
 *   redis/bench/direct   - direct redis_conn_base (no pool, baseline)
 *
 * Params:
 *   ?n=1000&concurrency=10&key=bench_key&val=bench_value&only=set|get
 *
 * Returns:
 *   total_ops, duration_ms, avg_us, ops_per_sec, min_us, max_us, p95_us, counters
 */

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>
#include <condition_variable>

#include "httppeer.h"

#ifdef ENABLE_REDIS
#include "redis_client.h"
#include "redis_conn.h"
#include "redis_pool.h"
#include "pzredis_config.h"
#endif

namespace http
{

// ===== Shared: latency stats =====
struct bench_stats
{
    std::vector<double> latencies_us;// per-op latency in microseconds
    bool sorted_ = false;            // 排过序没有，必须跟着实例走

    void add(double us)
    {
        latencies_us.push_back(us);
        sorted_ = false;
    }

    // sorted_ 早先是函数里的 static thread_local：那是「每线程一枚、所有 bench_stats 实例
    // 共享且永不复位」，同一根线程第二次跑这个页起，任何实例都不再排序，分位数读的是未排序
    // 向量。标记搬成成员，add() 一写就作废。
    void sort_if_needed()
    {
        if (!sorted_)
        {
            std::sort(latencies_us.begin(), latencies_us.end());
            sorted_ = true;
        }
    }

    double avg_us()
    {
        if (latencies_us.empty())
            return 0;
        double sum = 0;
        for (auto v : latencies_us)
            sum += v;
        return sum / latencies_us.size();
    }
    double p50_us()
    {
        sort_if_needed();
        if (latencies_us.empty())
            return 0;
        return latencies_us[latencies_us.size() / 2];
    }
    double p95_us()
    {
        sort_if_needed();
        if (latencies_us.empty())
            return 0;
        auto i = std::min(latencies_us.size() - 1, (size_t)(latencies_us.size() * 0.95));
        return latencies_us[i];
    }
    double p99_us()
    {
        sort_if_needed();
        if (latencies_us.empty())
            return 0;
        auto i = std::min(latencies_us.size() - 1, (size_t)(latencies_us.size() * 0.99));
        return latencies_us[i];
    }
    double max_us()
    {
        if (latencies_us.empty())
            return 0;
        // 排完再取末尾：back() 在未排序向量上是「最后一条样本」，不是最大值。
        sort_if_needed();
        return latencies_us.back();
    }

    http::obj_val report()
    {
        http::obj_val r;
        r.set_object();

        if (latencies_us.empty())
        {
            r["count"] = 0LL;
            return r;
        }
        sort_if_needed();
        auto count = latencies_us.size();
        double sum = 0;
        for (auto v : latencies_us)
            sum += v;
        r["count"]   = static_cast<long long>(count);
        r["avg_us"]  = sum / count;
        r["min_us"]  = latencies_us.front();
        r["max_us"]  = latencies_us.back();
        auto p95_idx = static_cast<size_t>(count * 0.95);
        if (p95_idx >= count)
            p95_idx = count - 1;
        r["p95_us"]  = latencies_us[p95_idx];
        auto p99_idx = static_cast<size_t>(count * 0.99);
        if (p99_idx >= count)
            p99_idx = count - 1;
        r["p99_us"] = latencies_us[p99_idx];
        return r;
    }
};

// ===== Parse params =====
struct bench_params
{
    int n           = 1000;// total commands
    int concurrency = 1;   // parallel workers
    std::string key = "bench_key";
    std::string val = "bench_value_1234567890";
    bool do_set     = true;
    bool do_get     = true;

    static bench_params parse(httppeer &client)
    {
        bench_params p;
        std::string s;
        s = client.get["n"].to_string();
        if (!s.empty())
            p.n = std::atoi(s.c_str());
        s = client.get["concurrency"].to_string();
        if (!s.empty())
            p.concurrency = std::atoi(s.c_str());
        s = client.get["key"].to_string();
        if (!s.empty())
            p.key = s;
        s = client.get["val"].to_string();
        if (!s.empty())
            p.val = s;
        s = client.get["only"].to_string();
        if (!s.empty())
        {
            p.do_set = (s == "set");
            p.do_get = (s == "get");
        }
        if (p.n <= 0)
            p.n = 1000;
        if (p.concurrency <= 0)
            p.concurrency = 1;
        if (p.concurrency > 256)
            p.concurrency = 256;
        return p;
    }
};

// ===== Helper: fill result JSON =====
// 只在 ENABLE_REDIS=ON 档存在：五个调用点全在 #ifdef ENABLE_REDIS 里，OFF 档留着这一份就是
// -Wunused-function；OFF 档那几条路由自己会回 404，不会调到它。
#ifdef ENABLE_REDIS
static void fill_result(httppeer &client,
                        const std::string &mode,
                        const bench_params &p,
                        long long total_ms,
                        bench_stats &set_stats,
                        bench_stats &get_stats)
{
    client.val.set_object();
    client.val["mode"]        = mode;
    client.val["n"]           = static_cast<long long>(p.n);
    client.val["concurrency"] = static_cast<long long>(p.concurrency);
    client.val["duration_ms"] = total_ms;
    double total_ops          = (p.do_set ? p.n : 0) + (p.do_get ? p.n : 0);
    if (total_ms > 0)
        client.val["ops_per_sec"] = total_ops * 1000.0 / total_ms;
    if (p.do_set)
        client.val["set_stats"] = set_stats.report();
    if (p.do_get)
        client.val["get_stats"] = get_stats.report();

    auto &pc = pz::redis::redis_pool::counters();
    auto &cc = pz::redis::redis_conn_base::counters();
    http::obj_val counters;
    counters.set_object();
    counters["pool_created"]      = static_cast<long long>(pc.created.load());
    counters["pool_busy_peak"]    = static_cast<long long>(pc.peak_busy.load());
    counters["pool_pending_peak"] = static_cast<long long>(pc.peak_pending.load());
    counters["conn_timeout"]      = static_cast<long long>(cc.timeout.load());
    counters["conn_poisoned"]     = static_cast<long long>(cc.poisoned.load());
    counters["conn_stale_drop"]   = static_cast<long long>(cc.stale_drop.load());
    counters["worker_count"]      = static_cast<long long>(pz::redis::get_redis_pool().worker_count());
    client.val["counters"]        = counters;
}
#endif

// ==================== 1. Sync client ====================
// Runs on business thread pool, can block
//@urlpath(null,redis/bench/sync)
std::string test_redis_bench_sync(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    auto params = bench_params::parse(client);
    auto t0     = std::chrono::steady_clock::now();
    bench_stats set_stats, get_stats;

    pz::redis::redis_client rc("default");

    if (params.do_set)
    {
        for (int i = 0; i < params.n; ++i)
        {
            auto st = std::chrono::steady_clock::now();
            rc.str_set(params.key + "_" + std::to_string(i), params.val);
            auto en = std::chrono::steady_clock::now();
            auto us = std::chrono::duration<double, std::micro>(en - st).count();
            set_stats.add(us);
        }
    }
    if (params.do_get)
    {
        for (int i = 0; i < params.n; ++i)
        {
            auto st = std::chrono::steady_clock::now();
            rc.str_get(params.key + "_" + std::to_string(i));
            auto en = std::chrono::steady_clock::now();
            auto us = std::chrono::duration<double, std::micro>(en - st).count();
            get_stats.add(us);
        }
    }

    auto t1            = std::chrono::steady_clock::now();
    long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    fill_result(client, "sync_client", params, total_ms, set_stats, get_stats);
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif
    client.out_json();
    return "";
}

// ==================== 2. Coroutine client (sequential) ====================
//@urlpath(null,redis/bench/async)
asio::awaitable<std::string> test_redis_bench_async(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    auto params = bench_params::parse(client);
    auto t0     = std::chrono::steady_clock::now();
    bench_stats set_stats, get_stats;

    pz::redis::redis_client rc("default");

    if (params.do_set)
    {
        for (int i = 0; i < params.n; ++i)
        {
            auto st = std::chrono::steady_clock::now();
            co_await rc.async_str_set(params.key + "_" + std::to_string(i), params.val);
            auto en = std::chrono::steady_clock::now();
            auto us = std::chrono::duration<double, std::micro>(en - st).count();
            set_stats.add(us);
        }
    }
    if (params.do_get)
    {
        for (int i = 0; i < params.n; ++i)
        {
            auto st = std::chrono::steady_clock::now();
            co_await rc.async_str_get(params.key + "_" + std::to_string(i));
            auto en = std::chrono::steady_clock::now();
            auto us = std::chrono::duration<double, std::micro>(en - st).count();
            get_stats.add(us);
        }
    }

    auto t1            = std::chrono::steady_clock::now();
    long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    fill_result(client, "async_client_sequential", params, total_ms, set_stats, get_stats);
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif
    client.out_json();
    co_return "";
}

// ==================== 3. Coroutine concurrent (fan-out via co_spawn) ====================
//@urlpath(null,redis/bench/async_concurrent)
asio::awaitable<std::string> test_redis_bench_async_concurrent(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    auto params = bench_params::parse(client);
    auto t0     = std::chrono::steady_clock::now();

    // 所有 worker 协程共享的状态（堆上，不引用主 handler 的栈变量）。
    // 这么做是因为 worker 会 co_spawn 到框架 io_context 的其他线程上，
    // 不能持有主 handler 栈上变量的引用（wait_cv 是 sync wait，主 handler 卡在 strand 上，
    // ASan 会把这种"引用栈变量 + 协程 detached 跨线程"组合误报成 stack-use-after-scope）。
    struct SharedState
    {
        std::atomic<int> next_idx{0};
        // 每个 worker 独占一格，只有那条线程读写自己那一格，全部结束后由主协程合并。
        // 早先所有 worker 往同一个 bench_stats 追加：vector 的 push_back 会扩容搬走整块内存，
        // 并发扩容就是 data race（最多 256 条线程同时写同一个 vector）。
        std::vector<bench_stats> per_worker_set;
        std::vector<bench_stats> per_worker_get;
        bench_stats set_stats;
        bench_stats get_stats;
        std::atomic<int> finished{0};
        int total_workers;
    };
    auto state           = std::make_shared<SharedState>();
    state->total_workers = params.concurrency;
    // 必须在 post 出第一条 worker 之前把格子摆好：worker 只按下标读写，不再改容器长度。
    state->per_worker_set.resize(params.concurrency);
    state->per_worker_get.resize(params.concurrency);

    auto *ioc = pz::redis::get_redis_pool().get_io_context();
    if (!ioc)
    {
        client.val.set_object();
        client.val["error"] = "redis pool not initialized";
        client.out_json();
        co_return "";
    }

    for (int w = 0; w < params.concurrency; ++w)
    {
        // post 到框架 io_context（绕过 HTTP strand），worker 用 sync str_set/str_get
        // 阻塞 ioc 的某个 worker 线程，真正并行，不会卡 HTTP strand
        asio::post(
            *ioc,
            [state, params, w]()
            {
                try
                {
                    pz::redis::redis_client rc("default");
                    while (true)
                    {
                        int idx = state->next_idx.fetch_add(1);
                        if (idx >= params.n)
                            break;
                        if (params.do_set)
                        {
                            auto st = std::chrono::steady_clock::now();
                            rc.str_set(params.key + "_" + std::to_string(idx), params.val);
                            auto en = std::chrono::steady_clock::now();
                            auto us = std::chrono::duration<double, std::micro>(en - st).count();
                            state->per_worker_set[w].add(us);
                        }
                        if (params.do_get)
                        {
                            auto st = std::chrono::steady_clock::now();
                            rc.str_get(params.key + "_" + std::to_string(idx));
                            auto en = std::chrono::steady_clock::now();
                            auto us = std::chrono::duration<double, std::micro>(en - st).count();
                            state->per_worker_get[w].add(us);
                        }
                    }
                }
                catch (...)
                {
                }
                state->finished.fetch_add(1);
            });
    }

    // 纯协程等待：定时器轮询，不阻塞 strand 线程。
    // 不能用 std::condition_variable::wait() —— 它会阻塞 strand 线程，
    // 而 worker 里 co_await 的 completion handler 可能需要 strand 来调度。
    // 这里 co_spawn 到 ioc 上做等待，确保 polling timer 不会卡 strand。
    co_await asio::co_spawn(
        *ioc,
        [ioc, state]() -> asio::awaitable<void>
        {
            asio::steady_timer timer(*ioc);
            while (state->finished.load() < state->total_workers)
            {
                timer.expires_after(std::chrono::milliseconds(5));
                co_await timer.async_wait(asio::use_awaitable);
            }
        },
        asio::use_awaitable);

    // 全部 worker 收尾之后才合并（worker 的 finished 计数是原子自增，主协程原子读，
    // 读到的那一格之后不再被任何线程写）。放在等待之前合并会读到半成品向量。
    for (auto &s : state->per_worker_set)
    {
        for (auto us : s.latencies_us)
            state->set_stats.add(us);
    }
    for (auto &s : state->per_worker_get)
    {
        for (auto us : s.latencies_us)
            state->get_stats.add(us);
    }

    auto t1            = std::chrono::steady_clock::now();
    long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    fill_result(client, "async_client_concurrent", params, total_ms, state->set_stats, state->get_stats);
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif
    client.out_json();
    co_return "";
}

// ==================== 4. Direct redis_pool::async_exec_obj ====================
//@urlpath(null,redis/bench/pool)
asio::awaitable<std::string> test_redis_bench_pool(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    auto params = bench_params::parse(client);
    auto t0     = std::chrono::steady_clock::now();
    bench_stats set_stats, get_stats;

    auto &pool = pz::redis::get_redis_pool();
    auto sec   = pool.section("default");

    if (params.do_set)
    {
        for (int i = 0; i < params.n; ++i)
        {
            auto st = std::chrono::steady_clock::now();
            co_await pool.async_exec_obj(sec, {"SET", params.key + "_" + std::to_string(i), params.val});
            auto en = std::chrono::steady_clock::now();
            auto us = std::chrono::duration<double, std::micro>(en - st).count();
            set_stats.add(us);
        }
    }
    if (params.do_get)
    {
        for (int i = 0; i < params.n; ++i)
        {
            auto st = std::chrono::steady_clock::now();
            co_await pool.async_exec_obj(sec, {"GET", params.key + "_" + std::to_string(i)});
            auto en = std::chrono::steady_clock::now();
            auto us = std::chrono::duration<double, std::micro>(en - st).count();
            get_stats.add(us);
        }
    }

    auto t1            = std::chrono::steady_clock::now();
    long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    fill_result(client, "pool_async_exec", params, total_ms, set_stats, get_stats);
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif
    client.out_json();
    co_return "";
}

// ==================== 5. Direct redis_conn_base (no pool, pure sync) ====================
// Standalone io_context + single connection, bypass pool entirely
//@urlpath(null,redis/bench/direct)
std::string test_redis_bench_direct(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    auto params = bench_params::parse(client);
    auto t0     = std::chrono::steady_clock::now();
    bench_stats set_stats, get_stats;

    asio::io_context ioc;
    pz::redis::redis_conn_base conn(ioc);
    auto cfg_opt = pz::redis::redis_conf("default");
    if (!cfg_opt || !conn.connect(*cfg_opt))
    {
        client.val.set_object();
        client.val["error"] = "connect fail: " + conn.last_error_msg();
        client.out_json();
        return "";
    }

    if (params.do_set)
    {
        for (int i = 0; i < params.n; ++i)
        {
            auto st = std::chrono::steady_clock::now();
            conn.command({"SET", params.key + "_" + std::to_string(i), params.val});
            auto en = std::chrono::steady_clock::now();
            auto us = std::chrono::duration<double, std::micro>(en - st).count();
            set_stats.add(us);
        }
    }
    if (params.do_get)
    {
        for (int i = 0; i < params.n; ++i)
        {
            auto st = std::chrono::steady_clock::now();
            conn.command({"GET", params.key + "_" + std::to_string(i)});
            auto en = std::chrono::steady_clock::now();
            auto us = std::chrono::duration<double, std::micro>(en - st).count();
            get_stats.add(us);
        }
    }
    conn.close();

    auto t1            = std::chrono::steady_clock::now();
    long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    fill_result(client, "direct_conn_no_pool", params, total_ms, set_stats, get_stats);
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
#endif
    client.out_json();
    return "";
}

// ==================== 6. Framework io_context 并行度探针 ====================
//@urlpath(null,redis/bench/ioc_probe)
asio::awaitable<std::string> test_ioc_probe(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    int m = 20, sleep_ms = 50;
    auto *ioc = pz::redis::get_redis_pool().get_io_context();
    if (!ioc)
    {
        client.val.set_object();
        client.val["error"] = "no ioc";
        client.out_json();
        co_return "";
    }

    // 也对比一下自建独立 io_context
    asio::io_context local_ioc;
    asio::executor_work_guard<asio::io_context::executor_type> guard(local_ioc.get_executor());
    int local_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> lts;
    for (int i = 0; i < local_threads; ++i)
        lts.emplace_back([&]()
                         { local_ioc.run(); });

    auto run_probe = [&](asio::io_context &target, int count, int slp) -> double
    {
        std::atomic<int> done{0};
        auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < count; ++i)
        {
            asio::post(target, [&done, slp]()
                       {
                std::this_thread::sleep_for(std::chrono::milliseconds(slp));
                done.fetch_add(1); });
        }
        while (done.load() < count)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto t1 = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli>(t1 - t0).count();
    };

    double fw_ms    = run_probe(*ioc, m, sleep_ms);
    double local_ms = run_probe(local_ioc, m, sleep_ms);

    client.val.set_object();
    client.val["framework_post_ms"]  = fw_ms;
    client.val["local_post_ms"]      = local_ms;
    client.val["tasks"]              = m;
    client.val["sleep_ms_per_task"]  = sleep_ms;
    client.val["framework_parallel"] = (fw_ms < sleep_ms * m * 0.7) ? true : false;
    client.val["local_parallel"]     = (local_ms < sleep_ms * m * 0.7) ? true : false;
    client.out_json();

    guard.reset();
    local_ioc.stop();
    for (auto &t : lts)
        t.join();
    co_return "";
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
    client.out_json();
    co_return "";
#endif
}

// ==================== 7. Async path 分段 micro-profile ====================
//@urlpath(null,redis/bench/async_profile)
asio::awaitable<std::string> test_async_profile(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_REDIS
    auto params = bench_params::parse(client);
    int n       = params.n;
    auto sec    = pz::redis::get_redis_pool().section("default");
    // section() 段没加载时回 nullptr（redis_pool::section 找不到就 return nullptr）。
    // 下面 warmup、取连接、back_conn 一共六处 sec-> 裸解引用，缺这一段就是 conf 里少一个
    // [default] 时一个 GET 把业务线程 SIGSEGV。早退形状照本文件 ioc 判空那一段。
    if (!sec)
    {
        client.val.set_object();
        client.val["error"] = "redis section [default] not loaded";
        client.out_json();
        co_return "";
    }

    // warmup
    for (int i = 0; i < 5; ++i)
    {
        auto c = co_await sec->async_get_conn();
        if (c)
        {
            co_await c->async_command({"PING"});
            sec->back_conn(std::move(c));
        }
    }

    auto stats_of = [](const std::vector<double> &v) -> http::obj_val
    {
        http::obj_val o;
        o.set_object();
        if (v.empty())
        {
            o["count"] = 0LL;
            return o;
        }
        auto copy = v;
        std::sort(copy.begin(), copy.end());
        double sum = 0;
        for (auto x : copy)
            sum += x;
        size_t c    = copy.size();
        o["count"]  = (long long)c;
        o["avg_us"] = sum / c;
        o["p50_us"] = copy[c / 2];
        o["p95_us"] = copy[std::min(c - 1, (size_t)(c * 0.95))];
        o["p99_us"] = copy[std::min(c - 1, (size_t)(c * 0.99))];
        o["max_us"] = copy.back();
        return o;
    };

    auto c = co_await sec->async_get_conn();
    if (!c)
    {
        client.val["error"] = "no conn";
        client.out_json();
        co_return "";
    }

    // seg1: async_command 单次 (已有连接, 纯 async_write + async_read)
    std::vector<double> seg1;
    for (int i = 0; i < n; ++i)
    {
        auto st = std::chrono::steady_clock::now();
        co_await c->async_command({"SET", "prof_" + std::to_string(i), "v"});
        auto en = std::chrono::steady_clock::now();
        seg1.push_back(std::chrono::duration<double, std::micro>(en - st).count());
    }
    sec->back_conn(std::move(c));

    // seg2: async_get_conn + async_command + back_conn 完整循环 （每次借还）
    std::vector<double> seg2;
    for (int i = 0; i < n; ++i)
    {
        auto st = std::chrono::steady_clock::now();
        auto cc = co_await sec->async_get_conn();
        if (cc)
        {
            co_await cc->async_command({"SET", "prof2_" + std::to_string(i), "v"});
            sec->back_conn(std::move(cc));
        }
        auto en = std::chrono::steady_clock::now();
        seg2.push_back(std::chrono::duration<double, std::micro>(en - st).count());
    }

    // seg3: async_exec_obj 完整走 pool 的 async_exec
    std::vector<double> seg3;
    for (int i = 0; i < n; ++i)
    {
        auto st = std::chrono::steady_clock::now();
        co_await pz::redis::get_redis_pool().async_exec_obj(
            sec,
            {"SET", "prof3_" + std::to_string(i), "v"});
        auto en = std::chrono::steady_clock::now();
        seg3.push_back(std::chrono::duration<double, std::micro>(en - st).count());
    }

    // seg4: 纯调度开销 —— co_await timer 0µs 就回，测 strand 上的 dispatch/suspend/resume
    std::vector<double> seg4;
    for (int i = 0; i < n; ++i)
    {
        auto st = std::chrono::steady_clock::now();
        co_await asio::steady_timer(
            co_await asio::this_coro::executor,
            std::chrono::microseconds(0))
            .async_wait(asio::use_awaitable);
        auto en = std::chrono::steady_clock::now();
        seg4.push_back(std::chrono::duration<double, std::micro>(en - st).count());
    }

    client.val.set_object();
    client.val["seg1_async_command_only"]   = stats_of(seg1);
    client.val["seg2_getconn_cmd_backconn"] = stats_of(seg2);
    client.val["seg3_full_async_exec_obj"]  = stats_of(seg3);
    client.val["seg4_timer_yield_overhead"] = stats_of(seg4);
    client.val["seg_minus_seg3_minus_seg1"] = stats_of(std::vector<double>{
        seg3.empty() ? 0 : seg3[0] - (seg1.empty() ? 0 : seg1[0])});
    client.out_json();
    co_return "";
#else
    // 本构建没编进 redis：按"路由不存在"回 404，不把"这个二进制编了哪些功能"写到公开路由面上
    client.status(404);
    client.val.set_object();
    client.val["error"] = "not found";
    client.out_json();
    co_return "";
#endif
}

}// namespace http
