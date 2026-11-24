// ORM 二进制接口探针：用法与 HttpArena frameworks/paozhu/src/controller/src/db.cpp 一致。
// 覆盖 async_query_prepared_binary（流式回调）与 async_query_prepared_all（整取 + decode_row），
// 验证冷/暖两轮结果与类型一致（冷轮文本列经归一化也必须给出 int64_t，抓"数值全 0"回归）、
// string_view 指向结果集缓冲区、SQL 错误时结果集清空且连接回池仍可用。
#include <asio.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "orm_query.h"

static int g_pass = 0;
static int g_fail = 0;

static void expect(bool ok, const std::string &name, const std::string &detail = "")
{
    if (ok)
    {
        ++g_pass;
        std::printf("PASS %s\n", name.c_str());
    }
    else
    {
        ++g_fail;
        std::printf("FAIL %s %s\n", name.c_str(), detail.c_str());
    }
}

// ---- 与 arena db.cpp 的 variant_int / variant_text 保持一致 ----
static long long variant_int(const orm::col_value_variant &v)
{
    if (auto p = std::get_if<int64_t>(&v))
        return *p;
    if (auto p = std::get_if<uint64_t>(&v))
        return static_cast<long long>(*p);
    if (auto p = std::get_if<double>(&v))
        return static_cast<long long>(*p);
    return 0;
}
static std::string_view variant_text(const orm::col_value_variant &v)
{
    if (auto p = std::get_if<std::string_view>(&v))
        return *p;
    return {};
}

struct item_rec
{
    long long id = 0, price = 0, quantity = 0, active = 0, rating_score = 0, rating_count = 0;
    std::string name, category, tags;
    bool operator==(const item_rec &o) const
    {
        return id == o.id && price == o.price && quantity == o.quantity && active == o.active &&
               rating_score == o.rating_score && rating_count == o.rating_count && name == o.name &&
               category == o.category && tags == o.tags;
    }
};

static const char *ITEMS_SQL =
    "SELECT id,name,category,price,quantity,active,tags,rating_score,rating_count "
    "FROM _ormbc_item WHERE price BETWEEN $1 AND $2 LIMIT $3";
static const char *ITEMS_WANT[9] = {"id",        "name",     "category", "price",   "quantity",
                                    "active",    "tags",     "rating_score", "rating_count"};

// 数值列（含 bool）的 variant 下标必须是 int64/uint64，不能是 string_view——
// 冷轮文本归一化失效时这里会先炸。
static bool numeric_index_ok(const orm::col_value_variant &v)
{
    return v.index() == orm::VI_INT || v.index() == orm::VI_UINT;
}

int main(int argc, char **argv)
{
    const std::string conf = argc > 1 ? argv[1] : "conf/orm.conf";
    auto ioc               = std::make_shared<asio::io_context>();
    orm::init_orm_conn_pool(ioc, conf);
    if (!orm::get_orm_conn_pool_obj()["pg"])
    {
        std::printf("SETUP_FAIL no [pg] pool (%s)\n", conf.c_str());
        return 2;
    }

    // ---- 建表与种子数据（同步路径，与 arena 库列型一致）----
    {
        orm::db_conn link("pg");
        unsigned int a0 = link.exec_edit_query("DROP TABLE IF EXISTS _ormbc_item", {});
        unsigned int a1 = link.exec_edit_query(
            "CREATE TABLE _ormbc_item (id INT PRIMARY KEY, name TEXT, category TEXT, price INT, "
            "quantity INT, active BOOLEAN, tags TEXT, rating_score INT, rating_count INT)",
            {});
        expect(a0 == 0 && a1 == 0 && !link.iserror && link.error_msg.empty(), "setup_item_ddl",
               "drop=" + std::to_string(a0) + " create=" + std::to_string(a1) + " err=" + link.error_msg);
        link.exec_edit_query(
            "INSERT INTO _ormbc_item VALUES "
            "(1,'alpha','cat1',100,1,true,'{\"a\":1}',5,50),"
            "(2,'beta','cat2',200,2,false,'{\"b\":2}',4,40),"
            "(3,'gamma','cat1',300,3,true,'{\"c\":3}',3,30)",
            {});
        expect(!link.iserror && link.error_msg.empty(), "setup_item_insert", link.error_msg);
        link.exec_edit_query("CREATE TABLE _ormbc_fortune (id INT PRIMARY KEY, message TEXT)", {});
        link.exec_edit_query(
            "INSERT INTO _ormbc_fortune VALUES "
            "(1,'z fortune'),(2,'A fortune'),(3,'another fortune'),(4,'M fortune'),(5,'Another fortune')",
            {});
        expect(!link.iserror && link.error_msg.empty(), "setup_fortune_insert", link.error_msg);
    }

    std::vector<item_rec> streamed, collected_cold, collected_warm;
    bool stream_numeric_ok  = true;
    bool collect_numeric_ok = true;

    // ---- 异步：流式 binary + 整取 collect，冷/暖各一轮 ----
    asio::co_spawn(
        *ioc,
        [&]() -> asio::awaitable<void>
        {
            std::vector<http::obj_val> params;
            params.emplace_back(100);
            params.emplace_back(300);
            params.emplace_back(5);

            // 0) 诊断：select 连接实际落在哪个库（DDL 走 edit 连接，两者必须同库）
            {
                orm::db_conn link("pg");
                orm::pg_result_set rs;
                co_await link.async_query_prepared_all("SELECT current_database(), current_user", {}, rs);
                std::vector<orm::col_value_variant> values;
                std::vector<std::string> backing;
                std::string db, usr;
                if (rs.decode_row(0, values, backing) && values.size() >= 2)
                {
                    db  = std::string(variant_text(values[0]));
                    usr = std::string(variant_text(values[1]));
                }
                expect(link.error_msg.empty() && db == "hello_world", "diag_select_conn_db",
                       "db=" + db + " user=" + usr + " err=" + link.error_msg);
            }
            // 1) 流式 binary（arena /async-db 改造前的用法）
            {
                orm::db_conn link("pg");
                std::vector<int> cols;
                bool colerr = false;
                co_await link.async_query_prepared_binary(
                    ITEMS_SQL, params,
                    [&](int cc, char **nm, auto get) -> bool
                    {
                        if (cols.empty())
                        {
                            cols.assign(9, -1);
                            for (int i = 0; i < cc; ++i)
                                for (int w = 0; w < 9; ++w)
                                    if (nm[i] && std::string_view(nm[i]) == ITEMS_WANT[w])
                                        cols[w] = i;
                            for (int w = 0; w < 9; ++w)
                                if (cols[w] < 0)
                                    colerr = true;
                        }
                        if (colerr)
                            return false;
                        item_rec r;
                        if (!numeric_index_ok(get(cols[0])) || !numeric_index_ok(get(cols[3])) ||
                            !numeric_index_ok(get(cols[5])) || !numeric_index_ok(get(cols[7])))
                            stream_numeric_ok = false;
                        r.id           = variant_int(get(cols[0]));
                        r.name         = std::string(variant_text(get(cols[1])));
                        r.category     = std::string(variant_text(get(cols[2])));
                        r.price        = variant_int(get(cols[3]));
                        r.quantity     = variant_int(get(cols[4]));
                        r.active       = variant_int(get(cols[5]));
                        r.tags         = std::string(variant_text(get(cols[6])));
                        r.rating_score = variant_int(get(cols[7]));
                        r.rating_count = variant_int(get(cols[8]));
                        streamed.push_back(std::move(r));
                        return true;
                    });
                expect(streamed.size() == 3 && link.error_msg.empty(), "stream_binary_rows3",
                       "n=" + std::to_string(streamed.size()) + " err=" + link.error_msg);
                expect(stream_numeric_ok, "stream_binary_numeric_types");
                expect(!colerr, "stream_binary_colnames");
            }

            // 2) 整取 collect：首轮（可能冷）
            auto collect_items = [&](std::vector<item_rec> &into,
                                     bool check_numeric) -> asio::awaitable<bool>
            {
                orm::db_conn link("pg");
                orm::pg_result_set rs;
                unsigned int n = co_await link.async_query_prepared_all(ITEMS_SQL, params, rs);
                if (n != 3 || rs.row_count() != 3 || !link.error_msg.empty())
                {
                    into.clear();
                    co_return false;
                }
                expect(n == 3, "collect_rows", "n=" + std::to_string(n));
                expect(rs.row_count() == n && !link.pg_select_conn, "collect_conn_released_before_decode");
                int pos[9];
                for (int w = 0; w < 9; ++w)
                {
                    pos[w] = -1;
                    for (size_t i = 0; i < rs.col_names.size(); ++i)
                        if (rs.col_names[i] == ITEMS_WANT[w])
                            pos[w] = (int)i;
                    if (pos[w] < 0)
                        co_return false;
                }
                std::vector<orm::col_value_variant> values;
                std::vector<std::string> backing;
                for (size_t i = 0; i < rs.row_count(); ++i)
                {
                    values.clear();
                    backing.clear();
                    if (!rs.decode_row(i, values, backing) || values.size() < 9)
                        co_return false;
                    if (check_numeric &&
                        !(numeric_index_ok(values[pos[0]]) && numeric_index_ok(values[pos[3]]) &&
                          numeric_index_ok(values[pos[5]]) && numeric_index_ok(values[pos[7]])))
                        collect_numeric_ok = false;
                    item_rec r;
                    r.id           = variant_int(values[pos[0]]);
                    r.name         = std::string(variant_text(values[pos[1]]));
                    r.category     = std::string(variant_text(values[pos[2]]));
                    r.price        = variant_int(values[pos[3]]);
                    r.quantity     = variant_int(values[pos[4]]);
                    r.active       = variant_int(values[pos[5]]);
                    r.tags         = std::string(variant_text(values[pos[6]]));
                    r.rating_score = variant_int(values[pos[7]]);
                    r.rating_count = variant_int(values[pos[8]]);
                    into.push_back(std::move(r));
                }
                co_return true;
            };

            bool cold_ok = co_await collect_items(collected_cold, true);
            expect(cold_ok && collected_cold.size() == 3, "collect_cold_rows3");
            expect(collect_numeric_ok, "collect_numeric_types");
            bool warm_ok = co_await collect_items(collected_warm, true);
            expect(warm_ok && collected_warm.size() == 3, "collect_warm_rows3");
            expect(streamed == collected_cold, "collect_cold_matches_stream");
            expect(collected_cold == collected_warm, "collect_warm_matches_cold");
            if (cold_ok && !collected_cold.empty())
            {
                const item_rec &r0 = collected_cold[0];
                expect(r0.id == 1 && r0.price == 100 && r0.quantity == 1 && r0.active == 1 &&
                           r0.rating_score == 5 && r0.rating_count == 50 && r0.name == "alpha" &&
                           r0.category == "cat1" && r0.tags == "{\"a\":1}",
                       "collect_values_exact", "id=" + std::to_string(r0.id) +
                                                   " price=" + std::to_string(r0.price));
                const item_rec &r1 = collected_cold[1];
                expect(r1.id == 2 && r1.active == 0, "collect_bool_false_zero");
            }

            // 3) fortunes 用法：整取 -> string_view 排序 -> 渲染
            {
                orm::db_conn link("pg");
                orm::pg_result_set rs;
                unsigned int n = co_await link.async_query_prepared_all(
                    "SELECT id,message FROM _ormbc_fortune ORDER BY id", {}, rs);
                expect(n == 5 && rs.row_count() == 5 && link.error_msg.empty() && !link.pg_select_conn,
                       "collect_fortune_rows5", "n=" + std::to_string(n) + " err=" + link.error_msg);
                struct view
                {
                    long long id;
                    std::string_view message;
                };
                std::vector<view> rows;
                std::vector<orm::col_value_variant> values;
                std::vector<std::string> backing;
                bool vok = true;
                for (size_t i = 0; i < rs.row_count(); ++i)
                {
                    values.clear();
                    backing.clear();
                    if (!rs.decode_row(i, values, backing) || values.size() < 2)
                    {
                        vok = false;
                        break;
                    }
                    rows.push_back(view{variant_int(values[0]), variant_text(values[1])});
                }
                expect(vok && rows.size() == 5, "collect_fortune_decode");
                rows.push_back(view{0, "Additional fortune added at request time."});
                std::sort(rows.begin(), rows.end(),
                          [](const view &a, const view &b) { return a.message < b.message; });
                // 字节序：A<space... "A fortune"(2) < "Additional..."(0) < "Another fortune"(5)
                // < "M fortune"(4) < "another fortune"(3) < "z fortune"(1)
                std::vector<long long> ids;
                for (auto &r : rows)
                    ids.push_back(r.id);
                std::string got;
                for (auto id : ids)
                    got += std::to_string(id) + ",";
                expect(ids == std::vector<long long>({2, 0, 5, 4, 3, 1}), "collect_fortune_sort_order",
                       "got=" + got);
            }

            // 4) SQL 错误：返回 0、结果集清空、错误可读；随后新对象查询成功（连接已回池可用）
            {
                orm::db_conn link("pg");
                orm::pg_result_set rs;
                unsigned int n =
                    co_await link.async_query_prepared_all("SELECT nope FROM _ormbc_item", {}, rs);
                expect(n == 0 && !link.error_msg.empty(), "collect_sql_error", link.error_msg);
                expect(rs.row_count() == 0, "collect_error_rs_cleared");
                expect(!link.pg_select_conn, "collect_error_conn_released");
            }
            {
                orm::db_conn link("pg");
                orm::pg_result_set rs;
                unsigned int n = co_await link.async_query_prepared_all(
                    "SELECT id,price FROM _ormbc_item WHERE id = $1",
                    std::vector<http::obj_val>{http::obj_val(2)}, rs);
                expect(n == 1 && rs.row_count() == 1 && link.error_msg.empty() && !link.pg_select_conn,
                       "collect_after_error_ok");
            }
            // 5) 参数个数不符：PG bind 阶段报错，同样不得当成空结果
            {
                orm::db_conn link("pg");
                orm::pg_result_set rs;
                unsigned int n = co_await link.async_query_prepared_all(
                    ITEMS_SQL, std::vector<http::obj_val>{http::obj_val(100)}, rs);
                expect(n == 0 && !link.error_msg.empty(), "collect_param_mismatch_error", link.error_msg);
                expect(rs.row_count() == 0, "collect_param_mismatch_rs_cleared");
                expect(!link.pg_select_conn, "collect_param_mismatch_conn_released");
            }
            // 6) 冷/暖错误都与 Execute 同包关闭 portal；随后必须能在同一条物理连接上继续查询。
            {
                orm::db_conn link("pg");
                link.lock_conn(true);
                orm::pg_result_set rs;
                unsigned int n = co_await link.async_query_prepared_all(
                    "SELECT nope FROM _ormbc_item -- collect cold pipeline error", {}, rs);
                auto *conn_after_cold_error = link.pg_select_conn.get();
                expect(n == 0 && link.iserror && !link.error_msg.empty() && conn_after_cold_error &&
                           !conn_after_cold_error->is_closed(),
                       "collect_cold_pipeline_error", link.error_msg);
                link.iserror = false;
                link.error_msg.clear();
                n = co_await link.async_query_prepared_all("SELECT 1 -- collect after cold pipeline error", {}, rs);
                expect(n == 1 && rs.row_count() == 1 && link.error_msg.empty() &&
                           link.pg_select_conn.get() == conn_after_cold_error,
                       "collect_cold_error_same_conn_reuse");

                const std::string warm_sql =
                    "SELECT id FROM _ormbc_item WHERE id = $1 -- collect warm pipeline";
                n = co_await link.async_query_prepared_all(warm_sql, {http::obj_val(1)}, rs);
                expect(n == 1 && link.error_msg.empty(), "collect_warm_pipeline_prime");
                n = co_await link.async_query_prepared_all(warm_sql, {http::obj_val("not-an-int")}, rs);
                auto *conn_after_warm_error = link.pg_select_conn.get();
                expect(n == 0 && link.iserror && !link.error_msg.empty() && conn_after_warm_error &&
                           !conn_after_warm_error->is_closed(),
                       "collect_warm_pipeline_error", link.error_msg);
                link.iserror = false;
                link.error_msg.clear();
                n = co_await link.async_query_prepared_all(warm_sql, {http::obj_val(2)}, rs);
                expect(n == 1 && rs.row_count() == 1 && link.error_msg.empty() &&
                           link.pg_select_conn.get() == conn_after_warm_error,
                       "collect_warm_error_same_conn_reuse");
                link.lock_conn(false);
            }
            co_return;
        },
        asio::detached);
    ioc->run();

    // ---- 清理 ----
    if (argc < 3)// 传第二个参数保留现场，便于 psql 验证 DDL 是否真的落库
    {
        orm::db_conn link("pg");
        link.exec_edit_query("DROP TABLE IF EXISTS _ormbc_item", {});
        link.exec_edit_query("DROP TABLE IF EXISTS _ormbc_fortune", {});
    }
    orm::init_orm_conn_pool_release();
    std::printf(g_fail ? "PROBE FAIL %d\n" : "PROBE ALL PASS\n", g_fail);
    return g_fail ? 1 : 0;
}
