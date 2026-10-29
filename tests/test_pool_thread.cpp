// 线程切换测试：sync handler 经 co_pool_run_step / co_pool_run_bool / co_pool_run_void
// 投递后，业务函数必须在业务线程池（clientrunpool）工作线程上执行，协程续体必须回到
// 调用方所在的 strand 线程。本文件不启动网络服务，直接驱动框架的派单收口函数。
//
// Thread-switch test: the sync business function must run on a clientrunpool worker
// thread, and the coroutine must resume on its own strand thread after every hop.
//
// 本文件暂时没有构建目标：它调 get_server_app()，那个单例定义在 server.cpp，而 server.cpp 又调控制器
// 与视图的注册入口，能补齐符号的是整个框架而不是几个 .cpp；并且它必须和框架用同一套 ENABLE_* 宏编译——
// httpserver 与 ThreadPool 里有这些宏保护的成员，宏集不一致会读到错位的成员，用例红在别的地方。
// 所以 tests/CMakeLists.txt 把它排除在自动发现之外，等框架源码收成一份共享对象库再挂回测试目标。
// 手动编法：拿框架任一源文件的编译命令（宏与头路径都对）把本文件编成一枚 .o，再拿主程序的链接命令
// 去掉入口 TU、换上这枚 .o 重链。退出码 0 = 全过；非 0 = 有用例失败。
//
// There is no build target for this file yet: it calls get_server_app(), a singleton defined in
// server.cpp, which in turn calls the controller and view registration entry points — the symbol
// closure is the whole app, not a few extra sources. It also has to be compiled with the same
// ENABLE_* defines the framework uses, because httpserver and ThreadPool keep members behind those
// guards; a mismatched define set reads misaligned members and the cases fail for the wrong reason.
// Build it by hand with the framework's own compile flags for one object file, then relink with the
// main program's link line minus its entry translation unit. Exit 0 = all passed; non-zero = failures.

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <asio/use_awaitable.hpp>

#include "httppeer.h"
#include "pool_step.h"
#include "server.h"

using namespace std::chrono_literals;

namespace
{

std::mutex g_print_mtx;
unsigned   g_fail = 0;

#define CHECK_TRUE(cond, name)                                                                    \
    do                                                                                            \
    {                                                                                             \
        bool _ok = static_cast<bool>(cond);                                                       \
        {                                                                                         \
            std::lock_guard<std::mutex> _lk(g_print_mtx);                                         \
            std::cout << "  [" << (_ok ? "PASS" : "FAIL") << "] " << (name) << "\n";             \
        }                                                                                         \
        if (!_ok)                                                                                 \
            ++g_fail;                                                                             \
    } while (0)

// 在一条独占 io_context 线程的 strand 上跑完一个 awaitable 工厂。
// Run one awaitable factory on a strand owned by a single dedicated io thread.
template <typename F>
void run_on_strand(F &&factory)
{
    asio::io_context ioc;
    auto             strand = asio::make_strand(ioc);
    asio::co_spawn(strand, std::forward<F>(factory), asio::detached);
    std::thread runner([&] { ioc.run(); });
    runner.join();
}

// 用例 1：单步 —— 业务函数下池、返回值带回、续体回 strand。
// Case 1: single step — fn runs on a pool thread, result is delivered, resume on strand.
asio::awaitable<void> case_single_step()
{
    std::cout << "[case 1] single step: fn -> pool, resume -> strand\n";
    auto                     strand_tid = std::this_thread::get_id();
    auto                     peer       = std::make_shared<http::httppeer>();
    std::thread::id          fn_tid{};

    std::function<std::string(std::shared_ptr<http::httppeer> &)> fn =
        [&](std::shared_ptr<http::httppeer> &p) -> std::string
    {
        fn_tid = std::this_thread::get_id();
        CHECK_TRUE(p.get() == peer.get(), "peer is the same object end to end");
        std::this_thread::sleep_for(2ms);
        return "@ok";
    };

    auto r = co_await http::co_pool_run_step(fn, peer, asio::use_awaitable);

    CHECK_TRUE(!r.eptr && !r.rejected, "step accepted and no exception");
    CHECK_TRUE(r.ret == "ok", "business return value propagated");
    CHECK_TRUE(fn_tid != std::thread::id{}, "fn actually executed");
    CHECK_TRUE(fn_tid != strand_tid, "fn ran OFF the strand thread (on a pool worker)");
    CHECK_TRUE(std::this_thread::get_id() == strand_tid, "coroutine resumed back ON the strand thread");
    co_return;
}

// 用例 2：两步链（/admin/main = pre + reg 形态）——每一步都下池，每步之间都回 strand。
// Case 2: two-step chain (pre + reg) — every step hops to the pool and back.
asio::awaitable<void> case_two_step_chain()
{
    std::cout << "[case 2] two-step chain: each step off-strand, resumes on strand\n";
    auto            strand_tid = std::this_thread::get_id();
    auto            peer       = std::make_shared<http::httppeer>();
    std::thread::id fn1_tid{}, fn2_tid{};
    std::thread::id mid_tid{};
    unsigned        order = 0;
    unsigned        fn1_order = 0, fn2_order = 0;

    std::function<std::string(std::shared_ptr<http::httppeer> &)> pre =
        [&](std::shared_ptr<http::httppeer> &) -> std::string
    {
        fn1_order = ++order;
        fn1_tid   = std::this_thread::get_id();
        std::this_thread::sleep_for(2ms);
        return "@ok";// classify_step: "@ok" => 继续执行 regfun
    };
    std::function<std::string(std::shared_ptr<http::httppeer> &)> reg =
        [&](std::shared_ptr<http::httppeer> &) -> std::string
    {
        fn2_order = ++order;
        fn2_tid   = std::this_thread::get_id();
        std::this_thread::sleep_for(2ms);
        return "main";
    };

    auto r1 = co_await http::co_pool_run_step(pre, peer, asio::use_awaitable);
    mid_tid = std::this_thread::get_id();
    auto r2 = co_await http::co_pool_run_step(reg, peer, asio::use_awaitable);

    CHECK_TRUE(!r1.eptr && r1.ret == "ok" && !r2.eptr && r2.ret == "main", "both steps returned values");
    CHECK_TRUE(fn1_order == 1 && fn2_order == 2, "steps executed strictly in order pre then reg");
    CHECK_TRUE(fn1_tid != strand_tid && fn2_tid != strand_tid, "both fns ran on pool threads");
    CHECK_TRUE(mid_tid == strand_tid, "resume between steps stayed on strand");
    CHECK_TRUE(std::this_thread::get_id() == strand_tid, "final resume on strand");
    co_return;
}

// 用例 3：业务函数抛异常 —— 在池线程抛出，在 strand 上以 eptr 收到，协程不挂死。
// Case 3: fn throws on the pool thread; the awaitable resumes on strand with eptr.
asio::awaitable<void> case_exception_propagation()
{
    std::cout << "[case 3] exception in fn is delivered back as eptr on strand\n";
    auto                     strand_tid = std::this_thread::get_id();
    auto                     peer       = std::make_shared<http::httppeer>();
    std::thread::id          fn_tid{};
    std::function<std::string(std::shared_ptr<http::httppeer> &)> fn =
        [&](std::shared_ptr<http::httppeer> &) -> std::string
    {
        fn_tid = std::this_thread::get_id();
        throw std::runtime_error("boom-pool-thread");
    };

    auto r = co_await http::co_pool_run_step(fn, peer, asio::use_awaitable);

    CHECK_TRUE(static_cast<bool>(r.eptr), "exception captured into eptr");
    CHECK_TRUE(http::exception_text(r.eptr) == "boom-pool-thread", "exception text preserved");
    CHECK_TRUE(fn_tid != strand_tid, "throw happened on a pool thread");
    CHECK_TRUE(std::this_thread::get_id() == strand_tid, "resumed on strand after throw");
    co_return;
}

// 用例 4：池拒单 —— rejected=true 就地完成，协程同样回到 strand，不永久挂起。
// Case 4: pool rejects the task — rejected step completes inline, still resuming on strand.
asio::awaitable<void> case_rejected_step()
{
    std::cout << "[case 4] rejected task completes inline with rejected=true\n";
    auto            strand_tid = std::this_thread::get_id();
    auto            peer       = std::make_shared<http::httppeer>();
    unsigned        before     = http::get_server_app().clientrunpool.getsyncdropped();
    bool            fn_ran     = false;
    std::function<std::string(std::shared_ptr<http::httppeer> &)> fn =
        [&](std::shared_ptr<http::httppeer> &) -> std::string
    {
        fn_ran = true;
        return "should-not-run";
    };

    http::get_server_app().clientrunpool.force_reject_sync_once();
    auto r = co_await http::co_pool_run_step(fn, peer, asio::use_awaitable);
    unsigned after = http::get_server_app().clientrunpool.getsyncdropped();

    CHECK_TRUE(r.rejected, "step marked rejected");
    CHECK_TRUE(!fn_ran, "rejected fn never executed");
    CHECK_TRUE(after == before + 1, "sync dropped counter advanced by 1");
    CHECK_TRUE(std::this_thread::get_id() == strand_tid, "resumed on strand after rejection");
    co_return;
}

// 用例 5：零参 bool / void 入口（mqtt/socket 通路，同一个派单模板）也要正确切线程。
// Case 5: zero-arg bool/void entries (mqtt/socket path) hop threads via the same template.
asio::awaitable<void> case_zeroarg_entries()
{
    std::cout << "[case 5] co_pool_run_bool / co_pool_run_void thread hops\n";
    auto            strand_tid = std::this_thread::get_id();
    std::thread::id bool_tid{}, void_tid{};

    auto rb = co_await http::co_pool_run_bool(
        [&]
        {
            bool_tid = std::this_thread::get_id();
            std::this_thread::sleep_for(1ms);
            return true;
        },
        asio::use_awaitable);
    auto mid_tid = std::this_thread::get_id();
    auto rv      = co_await http::co_pool_run_void(
        [&]
        {
            void_tid = std::this_thread::get_id();
            std::this_thread::sleep_for(1ms);
        },
        asio::use_awaitable);

    CHECK_TRUE(!rb.eptr && !rb.rejected && rb.value, "bool step delivered its value");
    CHECK_TRUE(!rv.eptr && !rv.rejected, "void step completed");
    CHECK_TRUE(bool_tid != strand_tid && void_tid != strand_tid, "both fns ran on pool threads");
    CHECK_TRUE(mid_tid == strand_tid && std::this_thread::get_id() == strand_tid,
               "resumes stayed on strand");
    co_return;
}

// 用例 6：并发 —— N 条协程各 3 步，任务一个不丢，业务全在池线程、续体全在 io 线程，
// 且负载确实分散到多个池工作线程（用 2ms sleep 保证多线程必然被调度到）。
// Case 6: N coroutines x 3 steps: no lost task, all fns off-strand, all resumes on the
// single io thread, and work spreads over multiple pool workers.
asio::awaitable<void> case_concurrent(const unsigned idx,
                                      const unsigned steps,
                                      std::atomic<unsigned> &fn_total,
                                      std::atomic<unsigned> &fn_off_strand,
                                      std::atomic<unsigned> &resume_on_strand,
                                      std::set<std::thread::id> &pool_tids,
                                      std::mutex &pool_mtx,
                                      std::thread::id io_tid)
{
    auto peer = std::make_shared<http::httppeer>();
    for (unsigned s = 0; s < steps; ++s)
    {
        std::function<std::string(std::shared_ptr<http::httppeer> &)> fn =
            [&, s](std::shared_ptr<http::httppeer> &) -> std::string
        {
            auto tid = std::this_thread::get_id();
            ++fn_total;
            if (tid != io_tid)
                ++fn_off_strand;
            {
                std::lock_guard<std::mutex> lk(pool_mtx);
                pool_tids.insert(tid);
            }
            std::this_thread::sleep_for(2ms);
            return "step-" + std::to_string(s);
        };
        auto r = co_await http::co_pool_run_step(fn, peer, asio::use_awaitable);
        if (!r.eptr && !r.rejected && std::this_thread::get_id() == io_tid)
            ++resume_on_strand;
    }
    (void)idx;
    co_return;
}

}// namespace

int main()
{
    try
    {
        unsigned pool_n = http::get_server_app().clientrunpool.getpoolthreadnum();
        std::cout << "clientrunpool worker threads: " << pool_n << "\n";
        if (pool_n < 2)
        {
            std::cout << "[FATAL] pool has fewer than 2 workers, test is meaningless\n";
            return 2;
        }

        run_on_strand(case_single_step);
        run_on_strand(case_two_step_chain);
        run_on_strand(case_exception_propagation);
        run_on_strand(case_rejected_step);
        run_on_strand(case_zeroarg_entries);

        // ---- case 6: concurrent fan-out on a shared single-thread io_context ----
        constexpr unsigned N     = 64;
        constexpr unsigned STEPS = 3;
        std::cout << "[case 6] " << N << " concurrent coroutines x " << STEPS << " steps\n";

        asio::io_context ioc;

        std::atomic<unsigned> fn_total{0}, fn_off{0}, resume_ok{0};
        std::set<std::thread::id> pool_tids;
        std::mutex                pool_mtx;
        std::thread::id           io_tid;

        // 先 spawn 完再启动 io 线程：否则空 io_context 的 run() 会立刻返回。
        // Spawn first, then start the io thread — an empty io_context.run() returns at once.
        for (unsigned i = 0; i < N; ++i)
        {
            auto ex = asio::make_strand(ioc);
            asio::co_spawn(ex,
                           [&, i]
                           {
                               return case_concurrent(i, STEPS, fn_total, fn_off, resume_ok,
                                                      pool_tids, pool_mtx, io_tid);
                           },
                           asio::detached);
        }
        std::thread io_runner([&]
                              {
                                  io_tid = std::this_thread::get_id();
                                  ioc.run();
                              });
        io_runner.join();

        CHECK_TRUE(fn_total.load() == N * STEPS, "every pool step executed exactly once (no lost task)");
        CHECK_TRUE(fn_off.load() == N * STEPS, "every fn executed off the io/strand thread");
        CHECK_TRUE(resume_ok.load() == N * STEPS, "every resume happened on the io thread");
        CHECK_TRUE(pool_tids.size() >= 2, "load spread across multiple pool workers");
        {
            std::lock_guard<std::mutex> lk(g_print_mtx);
            std::cout << "  [INFO ] distinct pool workers observed: " << pool_tids.size() << "\n";
        }

        std::cout << (g_fail == 0 ? "\nALL TESTS PASSED\n" : "\nTESTS FAILED\n");
        return g_fail == 0 ? 0 : 1;
    }
    catch (std::exception &e)
    {
        std::cout << "[FATAL] unexpected exception: " << e.what() << "\n";
        return 2;
    }
}
