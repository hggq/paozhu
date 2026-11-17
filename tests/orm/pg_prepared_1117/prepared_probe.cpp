#include <cstdio>
#include <chrono>
#include <string>
#include <vector>
#include <asio.hpp>
#include "orm_query.h"

struct world_row
{
    unsigned int id = 0, rn = 0;
    void set_val(const std::string &n, const unsigned char *p, std::size_t l, unsigned char)
    {
        std::string v(reinterpret_cast<const char *>(p), l);
        if (n == "id")
            id = (unsigned int)std::stoul(v);
        else if (n == "randomnumber")
            rn = (unsigned int)std::stoul(v);
    }
};
struct fortune_row
{
    unsigned int id = 0;
    std::string message;
    void set_val(const std::string &n, const unsigned char *p, std::size_t l, unsigned char)
    {
        std::string v(reinterpret_cast<const char *>(p), l);
        if (n == "id")
            id = (unsigned int)std::stoul(v);
        else if (n == "message")
            message = v;
    }
};

static int fails = 0;
#define CHECK(cond, msg)                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (cond)                                                                                                      \
            std::printf("PASS %s\n", msg);                                                                             \
        else                                                                                                           \
        {                                                                                                              \
            std::printf("FAIL %s\n", msg);                                                                             \
            fails++;                                                                                                   \
        }                                                                                                              \
    } while (0)

int main(int argc, char **argv)
{
    std::string conf = argc > 1 ? argv[1] : "conf/orm.conf";
    orm::init_orm_conn_pool(conf, 1, 2);
    {
        orm::db_conn link("pg");
        const std::string text_sql = "SELECT id,randomnumber FROM world WHERE id BETWEEN 1 AND 10 ORDER BY id";
        const std::string prep_sql = "SELECT id,randomnumber FROM world WHERE id BETWEEN ? AND ? ORDER BY id";
        std::vector<http::obj_val> params;
        params.emplace_back(1);
        params.emplace_back(10);

        std::vector<world_row> a, b;
        unsigned int n1 = link.query(text_sql, a);
        unsigned int n2 = link.query_prepared(prep_sql, params, b);
        CHECK(n1 == 10 && n2 == 10, "sync row counts 10/10");
        bool same = a.size() == b.size();
        for (size_t i = 0; same && i < a.size(); i++)
            same = a[i].id == b[i].id && a[i].rn == b[i].rn;
        CHECK(same, "sync text vs prepared rows identical");
        CHECK(link.error_msg.empty(), "sync no error");

        std::vector<world_row> b2;
        unsigned int n3 = link.query_prepared(prep_sql, params, b2);
        CHECK(n3 == 10 && b2.size() == b.size(), "sync warm named-statement reuse");

        std::vector<fortune_row> f1, f2;
        unsigned int m1 = link.query("SELECT id,message FROM fortune ORDER BY id", f1);
        unsigned int m2 = link.query_prepared("SELECT id,message FROM fortune ORDER BY id", {}, f2);
        bool fsame = m1 == m2 && f1.size() == f2.size();
        for (size_t i = 0; fsame && i < f1.size(); i++)
            fsame = f1[i].id == f2[i].id && f1[i].message == f2[i].message;
        CHECK(fsame, "sync empty-params fortunes identical");

        std::vector<world_row> bad;
        std::vector<http::obj_val> wrong;
        wrong.emplace_back(1);
        unsigned int n4 = link.query_prepared(prep_sql, wrong, bad);
        CHECK(n4 == 0 && !link.error_msg.empty(), "sync param-count mismatch refused");
        std::printf("  mismatch error_msg: %s\n", link.error_msg.c_str());
    }
    {
        asio::io_context ioc;
        asio::co_spawn(
            ioc,
            [&]() -> asio::awaitable<void> {
                orm::db_conn link("pg");
                const std::string text_sql = "SELECT id,randomnumber FROM world WHERE id BETWEEN 3 AND 7 ORDER BY id";
                const std::string prep_sql = "SELECT id,randomnumber FROM world WHERE id BETWEEN ? AND ? ORDER BY id";
                std::vector<http::obj_val> params;
                params.emplace_back(3);
                params.emplace_back(7);
                std::vector<world_row> a, b;
                unsigned int n1 = co_await link.async_query(text_sql, a);
                unsigned int n2 = co_await link.async_query_prepared(prep_sql, params, b);
                CHECK(n1 == 5 && n2 == 5, "async row counts 5/5");
                bool same = a.size() == b.size();
                for (size_t i = 0; same && i < a.size(); i++)
                    same = a[i].id == b[i].id && a[i].rn == b[i].rn;
                CHECK(same, "async text vs prepared rows identical");
                std::vector<world_row> b2;
                unsigned int n3 = co_await link.async_query_prepared(prep_sql, params, b2);
                CHECK(n3 == 5, "async warm named-statement reuse");
                co_return;
            },
            asio::detached);
        ioc.run();
    }
    {
        orm::db_conn link("pg");
        const std::string prep_sql = "SELECT id,randomnumber FROM world WHERE id = ?";
        std::vector<http::obj_val> params;
        params.emplace_back(500);
        std::vector<world_row> rows;
        link.query_prepared(prep_sql, params, rows); // warm the named statement
        const int N = 2000;
        auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < N; i++)
        {
            link.query("SELECT id,randomnumber FROM world WHERE id = 500", rows);
        }
        auto t1 = std::chrono::steady_clock::now();
        for (int i = 0; i < N; i++)
        {
            link.query_prepared(prep_sql, params, rows);
        }
        auto t2 = std::chrono::steady_clock::now();
        double text_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double prep_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        std::printf("A/B sync %d iters: text %.1f ms (%.3f ms/q) | prepared %.1f ms (%.3f ms/q) | speedup %.2fx\n",
                    N, text_ms, text_ms / N, prep_ms, prep_ms / N, text_ms / prep_ms);
    }
    {
        asio::io_context ioc;
        asio::co_spawn(
            ioc,
            [&]() -> asio::awaitable<void> {
                orm::db_conn link("pg");
                const std::string prep_sql = "SELECT id,randomnumber FROM world WHERE id = ?";
                std::vector<http::obj_val> params;
                params.emplace_back(500);
                std::vector<world_row> rows;
                co_await link.async_query_prepared(prep_sql, params, rows);
                const int N = 2000;
                auto t0 = std::chrono::steady_clock::now();
                for (int i = 0; i < N; i++)
                {
                    co_await link.async_query("SELECT id,randomnumber FROM world WHERE id = 500", rows);
                }
                auto t1 = std::chrono::steady_clock::now();
                for (int i = 0; i < N; i++)
                {
                    co_await link.async_query_prepared(prep_sql, params, rows);
                }
                auto t2 = std::chrono::steady_clock::now();
                double text_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
                double prep_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
                std::printf("A/B async %d iters: text %.1f ms (%.3f ms/q) | prepared %.1f ms (%.3f ms/q) | speedup %.2fx\n",
                            N, text_ms, text_ms / N, prep_ms, prep_ms / N, text_ms / prep_ms);
                co_return;
            },
            asio::detached);
        ioc.run();
    }
    orm::init_orm_conn_pool_release();
    std::printf(fails ? "PROBE FAIL %d\n" : "PROBE ALL PASS\n", fails);
    return fails ? 1 : 0;
}