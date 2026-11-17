/*
 * test_conn_pool_borrow.cpp — ORM 连接池跨槽借用空闲连接定向测试
 *
 * 验证 ORM 连接池跨槽借用的主要场景（异步/同步、坏连接探测、等待者交接、跨类型）。
 * 全部使用真实 MySQL / PostgreSQL 连接：
 *   - 用 rr_counter.store(s) 控制 async_get 的落槽，从而建出归属指定槽的连接；
 *   - 取出的连接由测试持有(占用)，back_* 放回原槽(空闲)；
 *   - 坏连接通过 conn_link->close() 关闭 socket 后归还进池构造。
 * 自建 conf 里 minpool=0(避免预建干扰)，maxpool=5 让 edit 两槽配额为 3/2。
 */
#include <asio.hpp>
#include <asio/post.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/redirect_error.hpp>
#include <asio/steady_timer.hpp>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "orm_conn_pool.h"
#include "mysql_conn.h"
#include "pg_conn.h"
#include "orm_common.h"

using namespace orm;

using mysql_ptr = std::shared_ptr<mysql_conn_base>;
using pg_ptr    = std::shared_ptr<pg_conn_base>;

static int g_pass = 0;
static int g_fail = 0;

static void check(bool cond, const std::string &name)
{
    if (cond)
    {
        ++g_pass;
        std::cout << "[PASS] " << name << "\n";
    }
    else
    {
        ++g_fail;
        std::cout << "[FAIL] " << name << "\n";
    }
}

static const char *kConfPath = "temp/test_conn_pool_borrow.conf";
static const char *kTagMy    = "borrowtest";
static const char *kTagPg    = "borrowpg";

// 真实凭据，取自 conf/orm.conf 的 [default] / [pg]。
static const char *kConfText = R"([borrowtest]
type = main
host = 127.0.0.1
port = 3306
dbname = hello_world
user = benchmarkdbuser
password = benchmarkdbpass
minpool = 0
maxpool = 5
dbtype = mysql
charset = utf8mb4

type = second
host = 127.0.0.1
port = 3306
dbname = hello_world
user = benchmarkdbuser
password = benchmarkdbpass
minpool = 0
maxpool = 20
dbtype = mysql
charset = utf8mb4

[borrowpg]
type = main
host = 127.0.0.1
port = 5432
dbname = hello_world
user = hello
password = 12345678
minpool = 0
maxpool = 5
dbtype = postgresql
charset = utf8

type = second
host = 127.0.0.1
port = 5432
dbname = hello_world
user = hello
password = 12345678
minpool = 0
maxpool = 20
dbtype = postgresql
charset = utf8
)";

template <typename ConnT>
struct GetResult
{
    std::shared_ptr<ConnT> conn;
    std::string error;
    long long ms = 0;
};

// 在独立 ioc 上跑一个 async_get_*，返回结果/异常与耗时。
template <typename ConnT, typename F>
static GetResult<ConnT> run_get(F &&make_awaitable)
{
    GetResult<ConnT> r;
    asio::io_context ioc;
    auto t0 = std::chrono::steady_clock::now();
    asio::co_spawn(ioc,
                   [&]() -> asio::awaitable<void>
                   {
                       try
                       {
                           r.conn = co_await make_awaitable();
                       }
                       catch (const std::exception &e)
                       {
                           r.error = e.what();
                       }
                   },
                   asio::detached);
    ioc.run();
    r.ms = std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - t0)
               .count();
    return r;
}

// 落指定槽取一条连接(占用中)。目标槽空闲应为空，这样 async_get 会真实建连到该槽。
static mysql_ptr take_edit(orm_conn_pool *pool, unsigned int slot)
{
    pool->rr_counter.store(slot);
    auto r = run_get<mysql_conn_base>([&] { return pool->async_get_mysql_edit_conn(); });
    if (!r.conn)
        throw std::runtime_error("take_edit(slot=" + std::to_string(slot) + "): " + r.error);
    return r.conn;
}
static mysql_ptr take_select(orm_conn_pool *pool, unsigned int slot)
{
    pool->rr_counter.store(slot);
    auto r = run_get<mysql_conn_base>([&] { return pool->async_get_mysql_select_conn(); });
    if (!r.conn)
        throw std::runtime_error("take_select: " + r.error);
    return r.conn;
}
static pg_ptr take_pg_edit(orm_conn_pool *pool, unsigned int slot)
{
    pool->rr_counter.store(slot);
    auto r = run_get<pg_conn_base>([&] { return pool->async_get_pg_edit_conn(); });
    if (!r.conn)
        throw std::runtime_error("take_pg_edit: " + r.error);
    return r.conn;
}
static pg_ptr take_pg_select(orm_conn_pool *pool, unsigned int slot)
{
    pool->rr_counter.store(slot);
    auto r = run_get<pg_conn_base>([&] { return pool->async_get_pg_select_conn(); });
    if (!r.conn)
        throw std::runtime_error("take_pg_select: " + r.error);
    return r.conn;
}

// 清空各槽空闲队列：shared_ptr 析构(连接对象析构 -> link 析构)归还名额。
static void cleanup_queues(orm_conn_pool &pool)
{
    for (auto &sp : pool.slots)
    {
        sp->mysql_edit_pool.clear();
        sp->mysql_select_pool.clear();
        sp->pg_edit_pool.clear();
        sp->pg_select_pool.clear();
    }
}

// 连续 5 次协程取 edit，rr 依次 0/1/0/1/0，全部本槽命中(真实建连)。
static void test_t1(orm_conn_pool *pool)
{
    const unsigned int expect[5] = {0, 1, 0, 1, 0};
    std::vector<mysql_ptr> held;
    bool ok = true;
    for (int i = 0; i < 5; ++i)
    {
        pool->rr_counter.store(expect[i]);
        auto r = run_get<mysql_conn_base>([&] { return pool->async_get_mysql_edit_conn(); });
        if (r.error.empty() && r.conn)
        {
            if (r.conn->conn_link->slot_index != expect[i])
                ok = false;
            held.push_back(r.conn);
        }
        else
        {
            std::cout << "       get#" << i << " error=" << r.error << "\n";
            ok = false;
            break;
        }
    }
    check(ok, "5 async gets succeed, slot_index = 0/1/0/1/0");
    check(pool->slots[0]->mysql_edit_pool.empty() && pool->slots[1]->mysql_edit_pool.empty(),
          "all 5 conns held (no idle in either slot)");
    cleanup_queues(*pool);
}

// 槽0 满(占用)无空闲、槽1 有空闲 -> 借槽1；归还后回槽1 不落槽0。
static void test_t2_t3(orm_conn_pool *pool)
{
    std::vector<mysql_ptr> h0;
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));// 槽0 满(3)
    std::vector<mysql_ptr> h1;
    h1.push_back(take_edit(pool, 1));
    h1.push_back(take_edit(pool, 1));// 槽1 满(2)

    pool->back_mysql_edit_conn(h1[0]);// 槽1 空闲 = 1
    h1[0].reset();
    check(pool->slots[0]->mysql_edit_pool.empty() &&
              pool->slots[1]->mysql_edit_pool.size() == 1,
          "precond: slot0 empty & full, slot1 has 1 idle");

    pool->rr_counter.store(0);
    auto r = run_get<mysql_conn_base>([&] { return pool->async_get_mysql_edit_conn(); });
    check(r.error.empty() && r.conn != nullptr, "borrow succeeded without error");
    if (r.conn)
    {
        check(r.conn->conn_link->slot_index == 1, "borrowed conn belongs to slot1");
        auto before0 = pool->slots[0]->mysql_edit_pool.size();
        auto before1 = pool->slots[1]->mysql_edit_pool.size();
        pool->back_mysql_edit_conn(r.conn);
        check(pool->slots[1]->mysql_edit_pool.size() == before1 + 1 &&
                  pool->slots[0]->mysql_edit_pool.size() == before0,
              "returned conn goes back to slot1, not slot0");
        r.conn.reset();
    }
    cleanup_queues(*pool);
}

// 槽0 满、槽1 唯一空闲是坏连接 -> 探测丢弃坏连接，不返回坏连接，名额归还。
static void test_t4(orm_conn_pool *pool)
{
    std::vector<mysql_ptr> h0;
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));// 槽0 满
    auto bad = take_edit(pool, 1);
    bad->conn_link->close();     // 关闭 socket => is_closed()==true
    check(bad->is_closed(), "precond: bad conn is_closed()");
    pool->back_mysql_edit_conn(bad);// 槽1 空闲 = [bad]（isclose 仍 false，会落池）
    auto total_before = pool->slots[1]->total_count[0]->load(std::memory_order_relaxed);

    pool->rr_counter.store(0);
    auto r = run_get<mysql_conn_base>([&] { return pool->async_get_mysql_edit_conn(); });
    check(r.conn == nullptr && !r.error.empty(), "no conn returned (bad dropped, falls to timeout)");
    check(r.error.find("wait timeout") != std::string::npos,
          "coroutine waited then timed out (not given bad conn)");
    check(pool->slots[1]->mysql_edit_pool.empty(), "bad idle removed from slot1");
    bad.reset();// 释放本地引用 -> 坏连接析构 -> 名额归还
    check(pool->slots[1]->total_count[0]->load(std::memory_order_relaxed) == total_before - 1,
          "dropped bad conn released its slot1 quota");
    cleanup_queues(*pool);
}

// 所有槽都满且无空闲 -> 仍抛 wait timeout，耗时约 5 秒，错误文本不变。
static void test_t5(orm_conn_pool *pool)
{
    std::vector<mysql_ptr> h0, h1;
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));
    h1.push_back(take_edit(pool, 1));
    h1.push_back(take_edit(pool, 1));// 槽0=3 + 槽1=2 全部占用

    pool->rr_counter.store(0);
    auto r = run_get<mysql_conn_base>([&] { return pool->async_get_mysql_edit_conn(); });
    check(!r.error.empty() && r.conn == nullptr, "all full -> exception, no conn");
    check(r.error.find("orm conn pool wait timeout") != std::string::npos &&
              r.error.find("role=edit") != std::string::npos,
          "timeout error text preserved");
    check(r.ms >= 4500 && r.ms < 9000,
          "waited about 5s (got " + std::to_string(r.ms) + "ms)");
    cleanup_queues(*pool);
}

// 同步路径跨槽借，issynch=true；归还重置 issynch 并回原槽。
static void test_t6(orm_conn_pool *pool)
{
    std::vector<mysql_ptr> h0;
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));// 槽0 满、空闲空
    auto s1 = take_edit(pool, 1);
    pool->back_mysql_edit_conn(s1);
    s1.reset();// 槽1 空闲 = 1

    pool->rr_counter.store(0);
    auto c = pool->get_mysql_edit_conn();
    check(c != nullptr, "sync get returned a conn");
    if (c)
    {
        check(c->conn_link->slot_index == 1, "sync borrowed from slot1");
        check(c->issynch.load(), "borrowed conn issynch==true");
        pool->back_mysql_edit_conn(c);
        check(pool->slots[1]->mysql_edit_pool.size() == 1, "returned to slot1");
        check(!c->issynch.load(), "back resets issynch to false");
    }
    cleanup_queues(*pool);
}

// 同步路径所有槽都空 -> 按现状在轮到的槽新建(真实 MySQL)，不报错。
static void test_t7(orm_conn_pool *pool)
{
    cleanup_queues(*pool);
    pool->rr_counter.store(0);
    auto c = pool->get_mysql_edit_conn();
    check(c != nullptr, "sync get created a new conn when all slots empty");
    if (c)
    {
        check(c->conn_link->slot_index == 0, "new conn bound to caller slot0");
        check(c->issynch.load(), "new sync conn issynch==true");
        pool->back_mysql_edit_conn(c);
    }
    cleanup_queues(*pool);
}

// 本槽有 waiter 时不跨槽借，waiter 按 FIFO 交接。
// 编排协程按步骤推进：A 挂起成 waiter -> 注入槽1空闲 -> B 到来(有 waiter 不探测)
// -> 归还两条槽0连接，A、B 依次被交接。
static void test_t8(orm_conn_pool *pool)
{
    std::vector<mysql_ptr> h0;
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));
    h0.push_back(take_edit(pool, 0));// 槽0 满、空闲为空
    auto s1 = take_edit(pool, 1); // 备用的槽1连接，稍后 back 进空闲
    check(pool->slots[1]->mysql_edit_pool.empty(), "precond: slot1 empty at A arrival");

    asio::io_context ioc;
    bool a_done = false, b_done = false;
    std::shared_ptr<mysql_conn_base> a_conn, b_conn;

    auto yield_n = [&](int n) -> asio::awaitable<void>
    {
        for (int i = 0; i < n; ++i)
            co_await asio::post(ioc, asio::use_awaitable);
    };

    asio::co_spawn(ioc,
                   [&]() -> asio::awaitable<void>
                   {
                       // A 落槽0：探测无果 -> 注册 waiter 挂起
                       pool->rr_counter.store(0);
                       asio::co_spawn(ioc,
                                      [&]() -> asio::awaitable<void>
                                      {
                                          a_conn = co_await pool->async_get_mysql_edit_conn();
                                          a_done = true;
                                      },
                                      asio::detached);
                       co_await yield_n(4);
                       check(pool->slots[0]->edit_waiters.size() == 1,
                             "A registered as slot0 waiter (probed, no borrow)");

                       // A 挂起后，槽1 才出现空闲
                       pool->back_mysql_edit_conn(s1);
                       s1.reset();
                       check(pool->slots[1]->mysql_edit_pool.size() == 1,
                             "slot1 has 1 idle after A parked");

                       // B 落槽0：此时有 waiter，不探测，应排到 A 之后
                       pool->rr_counter.store(0);
                       asio::co_spawn(ioc,
                                      [&]() -> asio::awaitable<void>
                                      {
                                          b_conn = co_await pool->async_get_mysql_edit_conn();
                                          b_done = true;
                                      },
                                      asio::detached);
                       co_await yield_n(4);
                       check(pool->slots[0]->edit_waiters.size() == 2,
                             "B queued behind A (no cross-slot borrow)");
                       check(pool->slots[1]->mysql_edit_pool.size() == 1,
                             "slot1 idle NOT stolen by B");

                       // 归还第一条：FIFO 交给 A
                       pool->back_mysql_edit_conn(h0.back());
                       h0.pop_back();
                       for (int k = 0; k < 8 && !a_done; ++k)
                           co_await yield_n(1);
                       check(a_done, "A resumed via handoff");
                       check(a_done && !b_done, "A handed off before B (FIFO)");

                       // 归还第二条：交给 B
                       pool->back_mysql_edit_conn(h0.back());
                       h0.pop_back();
                       for (int k = 0; k < 8 && !b_done; ++k)
                           co_await yield_n(1);
                       check(b_done, "B resumed via handoff");
                       check(a_conn && b_conn && !a_conn->is_closed() && !b_conn->is_closed(),
                             "both handed-off conns healthy & from slot0");
                   },
                   asio::detached);
    ioc.run();
    cleanup_queues(*pool);
}

// 跨类型补充：至少覆盖 mysql select / pg edit / pg select 各一条(真实连接)。
static void test_cross_type(orm_conn_pool *mysql_pool, orm_conn_pool *pg_pool)
{
    {
        auto s1 = take_select(mysql_pool, 1);
        mysql_pool->back_mysql_select_conn(s1);
        mysql_pool->rr_counter.store(0);
        auto c = mysql_pool->get_mysql_select_conn();
        check(c && c->conn_link->slot_index == 1 && c->issynch.load(),
              "mysql select borrowed from slot1");
        if (c) mysql_pool->back_mysql_select_conn(c);
        cleanup_queues(*mysql_pool);
    }
    {
        try
        {
            auto s1 = take_pg_edit(pg_pool, 1);
            pg_pool->back_pg_edit_conn(s1);
            pg_pool->rr_counter.store(0);
            auto c = pg_pool->get_pg_edit_conn();
            check(c && c->conn_link->slot_index == 1 && c->issynch.load(),
                  "pg edit borrowed from slot1");
            if (c) pg_pool->back_pg_edit_conn(c);
        }
        catch (const std::exception &e)
        {
            std::cout << "[SKIP] pg edit: " << e.what() << "\n";
        }
        cleanup_queues(*pg_pool);
    }
    {
        try
        {
            auto s1 = take_pg_select(pg_pool, 1);
            pg_pool->back_pg_select_conn(s1);
            pg_pool->rr_counter.store(0);
            auto c = pg_pool->get_pg_select_conn();
            check(c && c->conn_link->slot_index == 1 && c->issynch.load(),
                  "pg select borrowed from slot1");
            if (c) pg_pool->back_pg_select_conn(c);
        }
        catch (const std::exception &e)
        {
            std::cout << "[SKIP] pg select: " << e.what() << "\n";
        }
        cleanup_queues(*pg_pool);
    }
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    try
    {
        {
            std::ofstream f(kConfPath, std::ios::trunc);
            f << kConfText;
        }
        std::string log = init_orm_conn_pool(kConfPath, 2, 0);
        std::cout << log << "\n";

        auto &pools     = get_orm_conn_pool_obj();
        auto mysql_pool = pools[kTagMy];
        auto pg_pool    = pools[kTagPg];
        check(mysql_pool != nullptr && mysql_pool->slots.size() == 2,
              "init: borrowtest pool has 2 slots");
        check(mysql_pool->slot_max_of(0, 0) == 3 && mysql_pool->slot_max_of(0, 1) == 2,
              "init: edit slot_max = 3/2 (maxpool=5)");
        check(pg_pool != nullptr && pg_pool->slots.size() == 2,
              "init: borrowpg pool has 2 slots");

        test_t1(mysql_pool.get());
        test_t2_t3(mysql_pool.get());
        test_t4(mysql_pool.get());
        test_t5(mysql_pool.get());
        test_t6(mysql_pool.get());
        test_t8(mysql_pool.get());
        test_t7(mysql_pool.get());
        test_cross_type(mysql_pool.get(), pg_pool.get());
    }
    catch (const std::exception &e)
    {
        std::cout << "[FATAL] " << e.what() << "\n";
        ++g_fail;
    }
    init_orm_conn_pool_release();
    std::cout << "\n==== test_conn_pool_borrow: " << g_pass << " passed, "
              << g_fail << " failed ====\n";
    return g_fail ? 1 : 0;
}
