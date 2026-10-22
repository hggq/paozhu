// 真实 MySQL 连接层操作方式探针：文本/预编译/binary/类型/事务/池/
// 断线重连/故障注入/异步/多线程。
#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <variant>

#include "orm_conn_pool.h"
#include "mysql_conn.h"
#include "request.h"

using conn_t = std::shared_ptr<orm::mysql_conn_base>;
using pool_t = std::shared_ptr<orm::orm_conn_pool>;

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

static void reading(const std::string &name, const std::string &v)
{
    std::printf("READ %s=%s\n", name.c_str(), v.c_str());
}

static std::string hex_of(const std::string &s)
{
    static const char *hx = "0123456789abcdef";
    std::string r;
    for (unsigned char ch : s)
    {
        r.push_back(hx[ch >> 4]);
        r.push_back(hx[ch & 15]);
    }
    return r;
}

struct row_capture
{
    int cols = -1;
    std::vector<std::string> names;
    std::vector<std::vector<std::string>> vals;
    std::vector<std::vector<bool>> nulls;

    bool operator()(int cc, char **nm, std::function<std::tuple<unsigned char *, size_t>(int)> get)
    {
        if (cols < 0)
        {
            cols = cc;
            for (int i = 0; i < cc; ++i)
                names.emplace_back(nm[i] ? nm[i] : "");
        }
        std::vector<std::string> r;
        std::vector<bool> n;
        for (int i = 0; i < cc; ++i)
        {
            auto [p, l] = get(i);
            n.push_back(p == nullptr);
            r.emplace_back(p ? std::string(reinterpret_cast<char *>(p), l) : std::string());
        }
        vals.push_back(std::move(r));
        nulls.push_back(std::move(n));
        return true;
    }
};

struct bin_capture
{
    int rows = 0;
    std::vector<int> types;
    std::vector<std::string> texts;
    std::vector<bool> nulls;

    bool operator()(int cc, char **, std::function<orm::col_value_variant(int)> get)
    {
        if (rows == 0)
        {
            for (int i = 0; i < cc; ++i)
            {
                auto v = get(i);
                types.push_back(static_cast<int>(v.index()));
                if (auto p = std::get_if<int64_t>(&v))
                {
                    texts.push_back(std::to_string(*p));
                    nulls.push_back(false);
                }
                else if (auto u = std::get_if<uint64_t>(&v))
                {
                    texts.push_back(std::to_string(*u));
                    nulls.push_back(false);
                }
                else if (auto d = std::get_if<double>(&v))
                {
                    texts.push_back(std::to_string(*d));
                    nulls.push_back(false);
                }
                else if (auto s = std::get_if<std::string_view>(&v))
                {
                    texts.emplace_back(*s);
                    nulls.push_back(false);
                }
                else
                {
                    texts.push_back("<null>");
                    nulls.push_back(true);
                }
            }
        }
        ++rows;
        return true;
    }
};

static long long query_int(conn_t &c, const std::string &sql)
{
    long long n = -1;
    c->fetch_directly(sql,
                      [&n](int, char **, auto get) -> bool
                      {
                          auto [p, l] = get(0);
                          if (p)
                              n = std::stoll(std::string(reinterpret_cast<char *>(p), l));
                          return true;
                      });
    return n;
}

static std::string query_text(conn_t &c, const std::string &sql)
{
    std::string s;
    c->fetch_directly(sql,
                      [&s](int, char **, auto get) -> bool
                      {
                          auto [p, l] = get(0);
                          if (p)
                              s.assign(reinterpret_cast<char *>(p), l);
                          return true;
                      });
    return s;
}

static long long conn_id(conn_t &c)
{
    return query_int(c, "SELECT CONNECTION_ID()");
}

static long long count_rows(conn_t &c, const std::string &cond)
{
    return query_int(c, "SELECT count(*) FROM _mysql_probe_real WHERE " + cond);
}

static void kill_connection(pool_t pool, long long id)
{
    auto k = pool->get_mysql_edit_conn();
    k->exec_dml("KILL CONNECTION " + std::to_string(id));
    pool->back_mysql_edit_conn(std::move(k));
}

// 断线后一直重试到某次成功（写失败触发重连），或出现拒绝重连；返回尝试次数，-1 表示都没成功。
static int exec_until_done(conn_t &c, const std::string &sql, int max_attempts,
                           std::vector<unsigned int> &codes, std::string &refused_msg)
{
    codes.clear();
    for (int i = 0; i < max_attempts; ++i)
    {
        unsigned int r = c->exec_dml(sql);
        codes.push_back(r);
        if (c->error_msg.find("reconnect refused") != std::string::npos)
        {
            refused_msg = c->error_msg;
            return i + 1;
        }
        if (r != static_cast<unsigned int>(-1))
            return i + 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    return -1;
}

static asio::awaitable<void> async_arm(pool_t pool)
{
    conn_t ac = co_await pool->async_get_mysql_edit_conn();
    if (!ac)
    {
        std::printf("FAIL async_borrow\n");
        ++g_fail;
        co_return;
    }

    std::vector<http::obj_val> p1 = {http::obj_val(5001), http::obj_val(std::string("async_row"))};
    unsigned int r1              = co_await ac->async_exec_dml_prepared(
        "INSERT INTO _mysql_probe_real (i_val, s_val) VALUES (?, ?)", p1);
    expect(r1 == 1, "async_exec_dml_prepared_insert", "r=" + std::to_string(r1) + " err=" + ac->error_msg);

    row_capture rc;
    std::vector<http::obj_val> p2 = {http::obj_val(5001)};
    unsigned int n1               = co_await ac->async_fetch_prepared(
        "SELECT s_val FROM _mysql_probe_real WHERE i_val = ?", p2, std::ref(rc));
    expect(n1 == 1 && rc.vals.size() == 1 && rc.vals[0][0] == "async_row", "async_fetch_prepared");

    row_capture rc2;
    unsigned int n2 = co_await ac->async_fetch_prepared(
        "SELECT s_val FROM _mysql_probe_real WHERE i_val = ?", p2, std::ref(rc2));
    expect(n2 == 1 && rc2.vals.size() == 1 && rc2.vals[0][0] == "async_row", "async_fetch_prepared_warm");

    bin_capture bc;
    unsigned int n3 = co_await ac->async_fetch_prepared_binary(
        "SELECT i_val, s_val FROM _mysql_probe_real WHERE i_val = ?", p2, std::ref(bc));
    expect(n3 == 1 && bc.rows == 1 && bc.texts.size() == 2 &&
               bc.texts[0] == "5001" && bc.texts[1] == "async_row" && bc.types[1] == 4,
           "async_fetch_prepared_binary");

    long long got = -1;
    unsigned int n4 = co_await ac->async_fetch_directly(
        "SELECT count(*) FROM _mysql_probe_real WHERE i_val = 5001",
        [&got](int, char **, auto get) -> bool
        {
            auto [p, l] = get(0);
            if (p)
                got = std::stoll(std::string(reinterpret_cast<char *>(p), l));
            return true;
        });
    expect(n4 == 1 && got == 1, "async_fetch_directly", "n=" + std::to_string(n4) + " got=" + std::to_string(got));

    std::vector<http::obj_val> p3 = {http::obj_val(std::string("async_upd")), http::obj_val(5001)};
    unsigned int r2 = co_await ac->async_exec_dml(
        "UPDATE _mysql_probe_real SET s_val = 'async_dml' WHERE i_val = 5001");
    expect(r2 == 1, "async_exec_dml_update", "r=" + std::to_string(r2));
    unsigned int r3 = co_await ac->async_exec_dml_prepared(
        "UPDATE _mysql_probe_real SET s_val = ? WHERE i_val = ?", p3);
    expect(r3 == 1, "async_exec_dml_prepared_update", "r=" + std::to_string(r3));

    conn_t sc = co_await pool->async_get_mysql_select_conn();
    long long sgot = -1;
    std::string sid;
    unsigned int ns = co_await sc->async_fetch_directly(
        "SELECT count(*) FROM _mysql_probe_real WHERE i_val = 5001",
        [&sgot](int, char **, auto get) -> bool
        {
            auto [p, l] = get(0);
            if (p)
                sgot = std::stoll(std::string(reinterpret_cast<char *>(p), l));
            return true;
        });
    sid = conn_id(sc);
    expect(ns == 1 && sgot == 1, "async_select_pool_fetch",
           "n=" + std::to_string(ns) + " got=" + std::to_string(sgot) + " conn=" + sid);
    pool->back_mysql_select_conn(std::move(sc));

    pool->back_mysql_edit_conn(std::move(ac));
    co_return;
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    asio::io_context ioc;
    orm::init_orm_conn_pool(ioc, "conf/orm.conf");
    pool_t pool = orm::get_orm_conn_pool_obj()["cms"];
    if (!pool)
    {
        std::printf("FAIL pool_cms\n");
        return 1;
    }
    std::thread io_thread(
        [&ioc]()
        {
            auto guard = asio::make_work_guard(ioc);
            ioc.run();
        });

    auto maint = pool->get_mysql_edit_conn();
    auto maint_sel = pool->get_mysql_select_conn();
    if (!maint || !maint_sel)
    {
        std::printf("FAIL borrow_maint\n");
        return 1;
    }

    reading("server_version", query_text(*&maint, "SELECT VERSION()"));
    reading("connection_id", std::to_string(conn_id(maint)));
    reading("max_allowed_packet", query_text(maint, "SELECT @@max_allowed_packet"));
    reading("transaction_isolation", query_text(maint, "SELECT @@transaction_isolation"));
    reading("autocommit", query_text(maint, "SELECT @@autocommit"));
    reading("sql_mode", query_text(maint, "SELECT @@sql_mode"));
    long long max_packet = std::stoll(query_text(maint, "SELECT @@max_allowed_packet"));
    reading("prepared_stmt_count_begin", query_text(maint, "SHOW GLOBAL STATUS LIKE 'Prepared_stmt_count'"));
    std::string prepared_stmt_begin;
    maint->fetch_directly("SHOW GLOBAL STATUS LIKE 'Prepared_stmt_count'",
                          [&prepared_stmt_begin](int cc, char **, auto get) -> bool
                          {
                              if (cc >= 2)
                              {
                                  auto [p, l] = get(1);
                                  if (p)
                                      prepared_stmt_begin.assign(reinterpret_cast<char *>(p), l);
                              }
                              return true;
                          });
    reading("prepared_stmt_count_value_begin", prepared_stmt_begin);

    maint->exec_dml("DROP TABLE IF EXISTS _mysql_probe_real");
    maint->exec_dml("DROP TABLE IF EXISTS _mysql_probe_alter");
    maint->exec_dml("DROP TABLE IF EXISTS _mysql_probe_txn");
    unsigned int ddl = maint->exec_dml(
        "CREATE TABLE _mysql_probe_real ("
        " id INT PRIMARY KEY AUTO_INCREMENT,"
        " i_val INT, b_val BIGINT, ub_val BIGINT UNSIGNED,"
        " f_val FLOAT, d_val DOUBLE, dec_val DECIMAL(30,10),"
        " s_val VARCHAR(255), txt_val TEXT, bin_val VARBINARY(64),"
        " dt_val DATETIME(6), tm_val TIME(6), ts_val TIMESTAMP(6) NULL,"
        " date_val DATE, yr YEAR, bit_val BIT(8),"
        " en_val ENUM('a','b','c'), set_val SET('x','y','z'), js_val JSON,"
        " flag TINYINT, nul_val VARCHAR(64) NULL"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
    expect(ddl == 0, "create_table", "r=" + std::to_string(ddl) + " err=" + maint->error_msg);
    maint->exec_dml(
        "CREATE TABLE _mysql_probe_alter (id INT PRIMARY KEY, v INT) ENGINE=InnoDB");
    maint->exec_dml(
        "CREATE TABLE _mysql_probe_txn (id INT PRIMARY KEY, v INT) ENGINE=InnoDB");

    // ---------- 文本协议 ----------
    unsigned int ins1 = maint->exec_dml(
        "INSERT INTO _mysql_probe_real (id, i_val, s_val, nul_val) VALUES (1, 11, 'hello', NULL)");
    expect(ins1 == 1, "text_insert", "r=" + std::to_string(ins1) + " err=" + maint->error_msg);
    reading("last_insert_id_after_explicit_id", std::to_string(maint->last_insert_id()));

    unsigned int ins2 = maint->exec_dml(
        "INSERT INTO _mysql_probe_real (id, i_val, s_val, nul_val) VALUES (2, 22, '', NULL)");
    expect(ins2 == 1, "text_insert_empty_string", "r=" + std::to_string(ins2));

    unsigned int ins3 = maint->exec_dml(
        "INSERT INTO _mysql_probe_real (id, i_val, s_val, nul_val) VALUES (3, 33, '中文😀', 'notnull')");
    expect(ins3 == 1, "text_insert_utf8", "r=" + std::to_string(ins3));

    {
        row_capture rc;
        unsigned int n = maint_sel->fetch_directly(
            "SELECT s_val, nul_val FROM _mysql_probe_real WHERE id IN (1,2) ORDER BY id", std::ref(rc));
        bool empty_vs_null = rc.vals.size() == 2 &&
                             rc.nulls[0][1] && rc.vals[0][0] == "hello" &&
                             !rc.nulls[1][0] && rc.vals[1][0].empty() &&
                             rc.nulls[1][1];
        expect(n == 2 && empty_vs_null, "text_reading_null_vs_empty",
               "rows=" + std::to_string(n) + " captured=" + std::to_string(rc.vals.size()));
        reading("text_empty_is_null_flag",
                std::to_string(rc.nulls.size() > 1 && rc.nulls[1].size() > 1 ? rc.nulls[1][0] : 9));
        reading("text_null_flag",
                std::to_string(rc.nulls.size() > 1 && rc.nulls[1].size() > 1 ? rc.nulls[1][1] : 9));
        reading("text_captured_sizes",
                "vals=" + std::to_string(rc.vals.size()) + " nulls=" + std::to_string(rc.nulls.size()));
    }
    {
        reading("step", "text_utf8_before_fetch");
        row_capture rc;
        unsigned int n = maint_sel->fetch_directly(
            "SELECT s_val FROM _mysql_probe_real WHERE id = 3", std::ref(rc));
        reading("step", "text_utf8_after_fetch");
        reading("text_utf8_rows", std::to_string(n));
        reading("text_utf8_captured", std::to_string(rc.vals.size()));
        expect(n == 1 && rc.vals.size() == 1 && rc.vals[0].size() == 1 &&
                   rc.vals[0][0] == "中文😀",
               "text_utf8_roundtrip");
        reading("step", "text_utf8_after_expect");
    }

    unsigned int bad = maint->exec_dml("INSERT INTO _mysql_probe_real (no_such_col) VALUES (1)");
    expect(bad == static_cast<unsigned int>(-1) && !maint->error_msg.empty(), "text_bad_sql_sets_error");
    reading("bad_sql_error", maint->error_msg.substr(0, 80));
    expect(query_int(maint, "SELECT 1") == 1, "text_recover_after_bad_sql");
    expect(maint->ping(), "ping_ok");

    // ---------- 预编译 SELECT（文本化） ----------
    {
        row_capture cold;
        std::vector<http::obj_val> p = {http::obj_val(1)};
        unsigned int n = maint->fetch_prepared(
            "SELECT i_val, s_val, nul_val FROM _mysql_probe_real WHERE id = ?", p, std::ref(cold));
        bool ok = n == 1 && cold.vals.size() == 1 &&
                  cold.vals[0][0] == "11" && cold.vals[0][1] == "hello" && cold.nulls[0][2];
        expect(ok, "prepared_select_cold");

        row_capture warm;
        unsigned int n2 = maint->fetch_prepared(
            "SELECT i_val, s_val, nul_val FROM _mysql_probe_real WHERE id = ?", p, std::ref(warm));
        bool ok2 = n2 == 1 && warm.vals == cold.vals && warm.nulls == cold.nulls;
        expect(ok2, "prepared_select_warm_identical");
    }
    {
        row_capture rc;
        std::vector<http::obj_val> p = {http::obj_val(1)};
        unsigned int n = maint->fetch_prepared(
            "SELECT id AS pid, s_val AS ps FROM _mysql_probe_real WHERE id = ?", p, std::ref(rc));
        auto orgs = maint->prepared_col_org_names();
        expect(n == 1 && rc.names.size() == 2 && rc.names[0] == "pid" && rc.names[1] == "ps" &&
                   orgs.size() == 2 && orgs[0] == "id" && orgs[1] == "s_val",
               "prepared_alias_and_org_names",
               "names=" + (rc.names.size() > 1 ? rc.names[0] + "/" + rc.names[1] : "") +
                   " orgs=" + (orgs.size() > 1 ? orgs[0] + "/" + orgs[1] : ""));
    }
    {
        row_capture rc;
        std::vector<http::obj_val> p = {http::obj_val(999999)};
        unsigned int n = maint->fetch_prepared(
            "SELECT i_val FROM _mysql_probe_real WHERE id = ?", p, std::ref(rc));
        expect(n == 0 && rc.vals.empty() && maint->error_msg.empty(), "prepared_zero_rows");
    }
    {
        row_capture rc;
        unsigned int n = maint->fetch_prepared(
            "SELECT REPEAT('a', 1000000)", {}, std::ref(rc));
        expect(n == 1 && rc.vals.size() == 1 && rc.vals[0][0].size() == 1000000,
               "prepared_1mb_row", "len=" + std::to_string(rc.vals.size() ? rc.vals[0][0].size() : 0));
    }
    if (max_packet > 17000000)
    {
        row_capture rc;
        unsigned int n = maint->fetch_prepared(
            "SELECT REPEAT('a', 17000000)", {}, std::ref(rc));
        bool err_ok = n == 0 && maint->error_code == 20 &&
                      maint->error_msg.find("over 16MB") != std::string::npos;
        expect(err_ok, "prepared_over_16mb_explicit_error",
               "n=" + std::to_string(n) + " code=" + std::to_string(maint->error_code) +
                   " err=" + maint->error_msg);
        expect(query_int(maint, "SELECT 1") == 1, "prepared_over_16mb_stream_realigned");
    }
    else
    {
        reading("prepared_over_16mb_skipped", std::to_string(max_packet));
    }
    {
        row_capture rc;
        unsigned int n = maint->fetch_prepared("SELECT ?", {}, std::ref(rc));
        expect(n == 0 && maint->error_code == 20 &&
                   maint->error_msg.find("placeholder") != std::string::npos,
               "prepared_param_count_mismatch");
        expect(query_int(maint, "SELECT 1") == 1, "prepared_param_mismatch_recover");
    }
    {
        std::vector<http::obj_val> p = {http::obj_val(6001), http::obj_val(std::string("prep_id")),
                                        http::obj_val(nullptr)};
        unsigned int n = maint->exec_dml_prepared(
            "INSERT INTO _mysql_probe_real (i_val, s_val, nul_val) VALUES (?, ?, ?)", p);
        unsigned int lid = maint->last_insert_id();
        expect(n == 1 && lid > 0, "prepared_insert_last_id",
               "n=" + std::to_string(n) + " id=" + std::to_string(lid) + " err=" + maint->error_msg);
        row_capture rc;
        std::vector<http::obj_val> p2 = {http::obj_val(static_cast<long long>(lid))};
        unsigned int n2 = maint->fetch_prepared(
            "SELECT i_val, nul_val FROM _mysql_probe_real WHERE id = ?", p2, std::ref(rc));
        expect(n2 == 1 && rc.vals.size() == 1 && rc.vals[0][0] == "6001" && rc.nulls[0][1],
               "prepared_null_param_roundtrip");
        // 这条自增行占用了 id=4；类型矩阵稍后要显式插 id=4，先清掉避免主键冲突
        maint->exec_dml("DELETE FROM _mysql_probe_real WHERE s_val = 'prep_id'");
    }
    {
        std::vector<http::obj_val> p = {http::obj_val(1), http::obj_val(99)};
        unsigned int n = maint->exec_dml_prepared(
            "INSERT INTO _mysql_probe_real (id, i_val) VALUES (?, ?)", p);
        expect(n == static_cast<unsigned int>(-1) && maint->error_code == 1062 &&
                   maint->error_msg.find("Duplicate") != std::string::npos,
               "prepared_duplicate_key_error",
               "n=" + std::to_string(n) + " code=" + std::to_string(maint->error_code) + " err=" + maint->error_msg);
        expect(query_int(maint, "SELECT 1") == 1, "prepared_duplicate_key_recover");
    }
    {
        std::string bigparam(17000000, 'a');
        std::vector<http::obj_val> p = {http::obj_val(bigparam)};
        unsigned int n = maint->exec_dml_prepared(
            "INSERT INTO _mysql_probe_real (txt_val) VALUES (?)", p);
        reading("prepared_17mb_param_code", std::to_string(n) + "/" + std::to_string(maint->error_code));
        reading("prepared_17mb_param_error", maint->error_msg.substr(0, 80));
        expect(n == static_cast<unsigned int>(-1) &&
                   maint->error_msg.find("payload too large") != std::string::npos &&
                   !maint->is_closed(),
               "prepared_17mb_param_refused_keep_conn");
        expect(query_int(maint, "SELECT 1") == 1, "prepared_17mb_param_recover");
    }

    // ---------- 类型矩阵：文本路径 vs 预编译路径 ----------
    maint->exec_dml(
        "INSERT INTO _mysql_probe_real (id, i_val, b_val, ub_val, f_val, d_val, dec_val,"
        " s_val, txt_val, bin_val, dt_val, tm_val, ts_val, date_val, yr, bit_val,"
        " en_val, set_val, js_val, flag, nul_val) VALUES ("
        "4, -2147483648, -9223372036854775807, 18446744073709551615, 1.5, 0.30000000000000004,"
        " 12345.6789000000, 'types', 'text body', X'00FF41',"
        " '2024-02-29 12:34:56.789000', '-12:34:56.789000', '2024-02-29 12:34:56.789000',"
        " '2024-02-29', 2024, b'10100101', 'b', 'x,z', '{\"k\": 1}', 1, NULL)");
    reading("types_insert_error", maint->error_msg);

    row_capture text_row;
    maint_sel->fetch_directly(
        "SELECT i_val, b_val, ub_val, f_val, d_val, dec_val, s_val, txt_val, bin_val,"
        " dt_val, tm_val, ts_val, date_val, yr, bit_val, en_val, set_val, js_val, flag, nul_val"
        " FROM _mysql_probe_real WHERE id = 4",
        std::ref(text_row));

    row_capture prep_row;
    {
        std::vector<http::obj_val> p = {http::obj_val(4)};
        maint->fetch_prepared(
            "SELECT i_val, b_val, ub_val, f_val, d_val, dec_val, s_val, txt_val, bin_val,"
            " dt_val, tm_val, ts_val, date_val, yr, bit_val, en_val, set_val, js_val, flag, nul_val"
            " FROM _mysql_probe_real WHERE id = ?",
            p, std::ref(prep_row));
    }

    static const char *type_cols[] = {
        "i_val", "b_val", "ub_val", "f_val", "d_val", "dec_val", "s_val", "txt_val",
        "bin_val", "dt_val", "tm_val", "ts_val", "date_val", "yr", "bit_val", "en_val",
        "set_val", "js_val", "flag", "nul_val"};
    bool have_rows = text_row.vals.size() == 1 && prep_row.vals.size() == 1;
    expect(have_rows, "types_both_paths_one_row",
           "text_rows=" + std::to_string(text_row.vals.size()) +
               " prep_rows=" + std::to_string(prep_row.vals.size()));
    if (have_rows)
    {
        for (size_t ci = 0; ci < sizeof(type_cols) / sizeof(type_cols[0]); ++ci)
        {
            const std::string &a = text_row.vals[0][ci];
            const std::string &b = prep_row.vals[0][ci];
            bool same            = (a == b) && (text_row.nulls[0][ci] == prep_row.nulls[0][ci]);
            expect(same, std::string("type_") + type_cols[ci] + "_matches_text",
                   "text=" + hex_of(a) + " prep=" + hex_of(b));
            reading(std::string("type_") + type_cols[ci] + "_text", a);
        }
        expect(text_row.vals[0][0] == "-2147483648", "type_int_min");
        expect(text_row.vals[0][1] == "-9223372036854775807", "type_bigint");
        expect(text_row.vals[0][2] == "18446744073709551615", "type_ubigint_max");
        expect(text_row.vals[0][5] == "12345.6789000000", "type_decimal");
        expect(text_row.vals[0][9] == "2024-02-29 12:34:56.789000", "type_datetime6");
        expect(text_row.vals[0][10] == "-12:34:56.789000", "type_time6_negative");
        expect(text_row.vals[0][11] == "2024-02-29 12:34:56.789000", "type_timestamp6");
        expect(text_row.vals[0][12] == "2024-02-29", "type_date");
        expect(text_row.vals[0][13] == "2024", "type_year");
        expect(text_row.vals[0][15] == "b", "type_enum");
        expect(text_row.vals[0][16] == "x,z", "type_set");
        expect(text_row.nulls[0][19], "type_null_flag");
    }

    // ---------- binary variant ----------
    bin_capture binc;
    {
        std::vector<http::obj_val> p = {http::obj_val(4)};
        unsigned int n = maint->fetch_prepared_binary(
            "SELECT i_val, b_val, ub_val, f_val, d_val, dec_val, s_val, dt_val, tm_val,"
            " date_val, ts_val, bit_val, nul_val"
            " FROM _mysql_probe_real WHERE id = ?",
            p, std::ref(binc));
        expect(n == 1 && binc.rows == 1, "binary_cold_rows", "n=" + std::to_string(n));
        bool types_ok = binc.types.size() == 13 &&
                        binc.types[0] == 1 && binc.types[1] == 1 && binc.types[2] == 2 &&
                        binc.types[3] == 3 && binc.types[4] == 3 && binc.types[5] == 4 &&
                        binc.types[6] == 4 && binc.types[7] == 4 && binc.types[8] == 4 &&
                        binc.types[9] == 4 && binc.types[10] == 4 && binc.types[11] == 4 &&
                        binc.types[12] == 0;
        expect(types_ok, "binary_variant_types");
        if (binc.texts.size() == 13)
        {
            expect(binc.texts[0] == "-2147483648" && binc.texts[1] == "-9223372036854775807" &&
                       binc.texts[2] == "18446744073709551615",
                   "binary_variant_integers");
            expect(binc.texts[5] == "12345.6789000000", "binary_variant_decimal_sv");
            expect(binc.texts[7] == "2024-02-29 12:34:56.789000" &&
                       binc.texts[8] == "-12:34:56.789000" &&
                       binc.texts[9] == "2024-02-29" &&
                       binc.texts[10] == "2024-02-29 12:34:56.789000",
                   "binary_variant_time_family");
            bool bit_same = binc.texts[11] == text_row.vals[0][14];
            expect(bit_same, "binary_variant_bit_matches_text",
                   "bin=" + hex_of(binc.texts[11]) + " text=" + hex_of(text_row.vals[0][14]));
            expect(binc.types[12] == 0, "binary_variant_null_monostate");
        }
    }
    {
        bin_capture warm;
        std::vector<http::obj_val> p = {http::obj_val(4)};
        unsigned int n = maint->fetch_prepared_binary(
            "SELECT i_val, b_val, ub_val, f_val, d_val, dec_val, s_val, dt_val, tm_val,"
            " date_val, ts_val, bit_val, nul_val"
            " FROM _mysql_probe_real WHERE id = ?",
            p, std::ref(warm));
        expect(n == 1 && warm.types == binc.types && warm.texts == binc.texts,
               "binary_warm_identical");
    }

    // ---------- ALTER 后元数据校验 ----------
    {
        maint->exec_dml("INSERT INTO _mysql_probe_alter (id, v) VALUES (1, 5)");
        std::vector<http::obj_val> p = {http::obj_val(1)};
        row_capture cold, warm, after;
        unsigned int n1 = maint->fetch_prepared("SELECT v FROM _mysql_probe_alter WHERE id = ?", p, std::ref(cold));
        unsigned int n2 = maint->fetch_prepared("SELECT v FROM _mysql_probe_alter WHERE id = ?", p, std::ref(warm));
        maint->exec_dml("ALTER TABLE _mysql_probe_alter MODIFY v VARCHAR(32)");
        maint->exec_dml("UPDATE _mysql_probe_alter SET v = 'abc' WHERE id = 1");
        unsigned int n3 = maint->fetch_prepared("SELECT v FROM _mysql_probe_alter WHERE id = ?", p, std::ref(after));
        bool ok = n1 == 1 && n2 == 1 && n3 == 1 &&
                  cold.vals[0][0] == "5" && warm.vals[0][0] == "5" && after.vals[0][0] == "abc";
        expect(ok, "prepared_metadata_refresh_after_alter",
               "cold=" + (cold.vals.size() ? cold.vals[0][0] : "?") +
                   " warm=" + (warm.vals.size() ? warm.vals[0][0] : "?") +
                   " after=" + (after.vals.size() ? after.vals[0][0] : "?"));
        reading("alter_after_value", after.vals.size() ? after.vals[0][0] : "<none>");
    }

    // ---------- 事务 ----------
    unsigned int beg = maint->exec_dml("BEGIN");
    bool txn_on     = maint->in_transaction();
    expect(beg == 0 && txn_on, "txn_begin_sets_flag",
           "r=" + std::to_string(beg) + " flag=" + std::to_string((int)txn_on));
    maint->exec_dml("INSERT INTO _mysql_probe_txn (id, v) VALUES (100, 1)");
    long long unseen = query_int(maint_sel, "SELECT count(*) FROM _mysql_probe_txn WHERE id = 100");
    expect(unseen == 0, "txn_uncommitted_invisible", "count=" + std::to_string(unseen));
    maint->exec_dml("COMMIT");
    bool txn_off = !maint->in_transaction();
    long long seen = query_int(maint_sel, "SELECT count(*) FROM _mysql_probe_txn WHERE id = 100");
    expect(txn_off && seen == 1, "txn_commit_visible",
           "flag_off=" + std::to_string((int)txn_off) + " count=" + std::to_string(seen));

    maint->exec_dml("BEGIN");
    std::vector<http::obj_val> tp = {http::obj_val(101), http::obj_val(2)};
    maint->exec_dml_prepared("INSERT INTO _mysql_probe_txn (id, v) VALUES (?, ?)", tp);
    maint->exec_dml("ROLLBACK");
    long long rolled = query_int(maint_sel, "SELECT count(*) FROM _mysql_probe_txn WHERE id = 101");
    expect(!maint->in_transaction() && rolled == 0, "txn_prepared_insert_rollback",
           "count=" + std::to_string(rolled));

    // 池归还悬挂事务：先把空闲连接全部借空，再归还有事务的那条，
    // 保证下一个借用者一定拿到它（FIFO 池里否则可能拿到别的连接）
    {
        std::vector<conn_t> held;
        for (int i = 0; i < 10; ++i)
        {
            auto c = pool->get_mysql_edit_conn();
            if (!c)
                break;
            held.push_back(std::move(c));
        }
        conn_t borrowed = std::move(held.back());
        borrowed->exec_dml("BEGIN");
        borrowed->exec_dml("INSERT INTO _mysql_probe_txn (id, v) VALUES (102, 3)");
        bool flag_at_return = borrowed->in_transaction();
        pool->back_mysql_edit_conn(std::move(borrowed));
        auto next = pool->get_mysql_edit_conn();
        bool next_sees_txn = next->in_transaction();
        next->exec_dml("ROLLBACK");
        long long n = query_int(maint_sel, "SELECT count(*) FROM _mysql_probe_txn WHERE id = 102");
        pool->back_mysql_edit_conn(std::move(next));
        for (auto &c : held)
        {
            if (c)
                pool->back_mysql_edit_conn(std::move(c));
        }
        reading("pool_return_keeps_txn", std::to_string((int)next_sees_txn));
        expect(flag_at_return && next_sees_txn && n == 0, "pool_return_open_txn_reading");
    }

    // ---------- 故障注入：autocommit 杀后端后重连 ----------
    {
        long long old_id = conn_id(maint);
        maint->exec_dml("INSERT INTO _mysql_probe_real (id, i_val) VALUES (9001, 9001)");
        kill_connection(pool, old_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        std::vector<unsigned int> codes;
        std::string refused;
        int tries = exec_until_done(maint,
                                    "INSERT INTO _mysql_probe_real (id, i_val) VALUES (9002, 9002)",
                                    8, codes, refused);
        long long new_id = conn_id(maint);
        std::string codes_txt;
        for (unsigned int c : codes)
            codes_txt += (c == static_cast<unsigned int>(-1) ? "-1" : std::to_string(c)) + ",";
        reading("kill_autocommit_tries", std::to_string(tries));
        reading("kill_autocommit_codes", codes_txt);
        reading("kill_autocommit_conn_id", std::to_string(old_id) + "->" + std::to_string(new_id));
        expect(tries > 0 && new_id > 0 && new_id != old_id, "kill_autocommit_reconnected",
               "err=" + maint->error_msg);
        long long cnt = query_int(maint_sel, "SELECT count(*) FROM _mysql_probe_real WHERE id IN (9001,9002)");
        expect(cnt == 2, "kill_autocommit_rows", "count=" + std::to_string(cnt));

        std::vector<http::obj_val> pp = {http::obj_val(9100), http::obj_val(9100)};
        unsigned int pr = maint->exec_dml_prepared(
            "INSERT INTO _mysql_probe_real (id, i_val) VALUES (?, ?)", pp);
        expect(pr == 1, "prepared_after_reconnect", "r=" + std::to_string(pr) + " err=" + maint->error_msg);
    }

    // ---------- 故障注入：事务在途杀后端必须拒绝重连 ----------
    {
        auto tconn = pool->get_mysql_edit_conn();
        long long tid = conn_id(tconn);
        tconn->exec_dml("BEGIN");
        tconn->exec_dml("INSERT INTO _mysql_probe_txn (id, v) VALUES (103, 4)");
        kill_connection(pool, tid);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        std::vector<unsigned int> codes;
        std::string refused;
        int tries = exec_until_done(tconn,
                                    "INSERT INTO _mysql_probe_txn (id, v) VALUES (104, 5)",
                                    8, codes, refused);
        std::string codes_txt;
        for (unsigned int c : codes)
            codes_txt += (c == static_cast<unsigned int>(-1) ? "-1" : std::to_string(c)) + ",";
        reading("kill_txn_tries", std::to_string(tries));
        reading("kill_txn_codes", codes_txt);
        reading("kill_txn_refused_msg", refused.substr(0, 120));
        reading("kill_txn_flag", std::to_string((int)tconn->in_transaction()));
        reading("kill_txn_isclosed", std::to_string((int)tconn->is_closed()));
        bool refused_ok = !refused.empty() && tconn->is_closed();
        expect(refused_ok, "kill_in_txn_refuses_reconnect");
        long long cnt = query_int(maint_sel, "SELECT count(*) FROM _mysql_probe_txn WHERE id IN (103,104)");
        expect(cnt == 0, "kill_in_txn_not_committed", "count=" + std::to_string(cnt));
        pool->back_mysql_edit_conn(std::move(tconn));
    }

    // ---------- 会话级 autocommit=0 + 杀后端（门禁只认显式 BEGIN） ----------
    {
        auto c = pool->get_mysql_edit_conn();
        long long cid0 = conn_id(c);
        c->exec_dml("SET autocommit=0");
        unsigned int a1 = c->exec_dml("INSERT INTO _mysql_probe_txn (id, v) VALUES (201, 1)");
        bool flag_after_set = c->in_transaction();
        kill_connection(pool, cid0);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        std::vector<unsigned int> codes;
        std::string refused;
        int tries = exec_until_done(c, "INSERT INTO _mysql_probe_txn (id, v) VALUES (202, 2)", 8, codes, refused);
        long long cid1 = conn_id(c);
        std::string codes_txt;
        for (unsigned int cc : codes)
            codes_txt += (cc == static_cast<unsigned int>(-1) ? "-1" : std::to_string(cc)) + ",";
        long long r201 = query_int(maint_sel, "SELECT count(*) FROM _mysql_probe_txn WHERE id = 201");
        long long r202 = query_int(maint_sel, "SELECT count(*) FROM _mysql_probe_txn WHERE id = 202");
        reading("autocommit0_insert", std::to_string(a1));
        reading("autocommit0_txn_flag", std::to_string((int)flag_after_set));
        reading("autocommit0_tries", std::to_string(tries));
        reading("autocommit0_codes", codes_txt);
        reading("autocommit0_conn_id", std::to_string(cid0) + "->" + std::to_string(cid1));
        reading("autocommit0_row201", std::to_string(r201));
        reading("autocommit0_row202", std::to_string(r202));
        expect(tries > 0 && cid1 > 0 && cid1 != cid0, "autocommit0_kill_reconnects");
        reading("autocommit0_gate_missed",
                std::to_string((int)(!flag_after_set && r201 == 0 && r202 == 1)));
        pool->back_mysql_edit_conn(std::move(c));
    }

    // ---------- 读侧掉线：不置 isclose、下次写侧重连 ----------
    {
        auto c = pool->get_mysql_select_conn();
        long long cid0 = conn_id(c);
        kill_connection(pool, cid0);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        long long got = -2;
        unsigned int n = c->fetch_directly(
            "SELECT 1", [&got](int, char **, auto get) -> bool
            {
                auto [p, l] = get(0);
                if (p)
                    got = std::stoll(std::string(reinterpret_cast<char *>(p), l));
                return true;
            });
        reading("read_fail_rows", std::to_string(n));
        reading("read_fail_value", std::to_string(got));
        reading("read_fail_error", c->error_msg.substr(0, 80));
        reading("read_fail_code", std::to_string(c->error_code));
        reading("read_fail_isclosed", std::to_string((int)c->is_closed()));
        long long nid = conn_id(c);
        reading("read_fail_next_conn", std::to_string(cid0) + "->" + std::to_string(nid));
        expect(nid > 0 && nid != cid0, "read_fail_next_write_reconnects");
        pool->back_mysql_select_conn(std::move(c));
    }

    // ---------- 异步 ----------
    {
        auto fut = asio::co_spawn(ioc, async_arm(pool), asio::use_future);
        fut.get();
    }

    // ---------- 多线程预编译 ----------
    {
        std::vector<std::thread> ths;
        std::atomic<int> mt_err{0};
        for (int t = 0; t < 4; ++t)
        {
            ths.emplace_back(
                [&, t]()
                {
                    auto c = pool->get_mysql_edit_conn();
                    if (!c)
                    {
                        mt_err++;
                        return;
                    }
                    for (int i = 0; i < 30; ++i)
                    {
                        std::vector<http::obj_val> p = {
                            http::obj_val(7000 + t * 100 + i),
                            http::obj_val(std::string("mt_") + std::to_string(t) + "_" + std::to_string(i))};
                        unsigned int r = c->exec_dml_prepared(
                            "INSERT INTO _mysql_probe_real (i_val, s_val) VALUES (?, ?)", p);
                        if (r != 1)
                            mt_err++;
                    }
                    pool->back_mysql_edit_conn(std::move(c));
                });
        }
        for (auto &th : ths)
            th.join();
        long long cnt = query_int(maint_sel,
                                  "SELECT count(*) FROM _mysql_probe_real WHERE i_val >= 7000 AND i_val < 7400");
        expect(mt_err.load() == 0 && cnt == 120, "mt_prepared_120_inserts",
               "err=" + std::to_string(mt_err.load()) + " count=" + std::to_string(cnt));
    }

    // ---------- stmt 缓存容量/淘汰 ----------
    {
        auto c = pool->get_mysql_edit_conn();
        int bad = 0;
        for (int i = 0; i < 130; ++i)
        {
            row_capture rc;
            unsigned int n = c->fetch_prepared("SELECT " + std::to_string(i), {}, std::ref(rc));
            if (n != 1 || rc.vals.size() != 1 || rc.vals[0][0] != std::to_string(i))
                bad++;
        }
        row_capture first;
        unsigned int n = c->fetch_prepared("SELECT 0", {}, std::ref(first));
        bool ok = bad == 0 && n == 1 && first.vals.size() == 1 && first.vals[0][0] == "0";
        expect(ok, "stmt_cache_130_distinct", "bad=" + std::to_string(bad));
        std::string stmt_cnt;
        c->fetch_directly("SHOW GLOBAL STATUS LIKE 'Prepared_stmt_count'",
                          [&stmt_cnt](int cc, char **, auto get) -> bool
                          {
                              if (cc >= 2)
                              {
                                  auto [p, l] = get(1);
                                  if (p)
                                      stmt_cnt.assign(reinterpret_cast<char *>(p), l);
                              }
                              return true;
                          });
        reading("prepared_stmt_count_after_130", stmt_cnt);
        pool->back_mysql_edit_conn(std::move(c));
    }

    reading("prepared_col_org_names_after_zero_rows",
            std::to_string(maint->prepared_col_org_names().size()));

    // ---------- 存储过程多结果集：未协商 MULTI_RESULTS，应在首包报错且流保持对齐 ----------
    {
        maint->exec_dml("DROP PROCEDURE IF EXISTS _mysql_probe_proc");
        unsigned int d = maint->exec_dml(
            "CREATE PROCEDURE _mysql_probe_proc() BEGIN SELECT 1 AS a; SELECT 2 AS b; END");
        reading("proc_create_result", std::to_string(d) + " err=" + maint->error_msg.substr(0, 80));
        row_capture rc;
        unsigned int n = maint->fetch_directly("CALL _mysql_probe_proc()", std::ref(rc));
        reading("proc_call_rows", std::to_string(n));
        reading("proc_call_error_code", std::to_string(maint->error_code));
        reading("proc_call_error", maint->error_msg.substr(0, 120));
        reading("proc_call_captured", std::to_string(rc.vals.size()));
        expect(query_int(maint, "SELECT 1") == 1, "proc_call_stream_realigned");
        maint->exec_dml("DROP PROCEDURE IF EXISTS _mysql_probe_proc");
    }

    // ---------- 文本路径 17MB 单行（服务器按 0xFFFFFF 分片） ----------
    {
        std::atomic<bool> done{false};
        std::thread wd(
            [&done]()
            {
                for (int i = 0; i < 600; ++i)
                {
                    if (done.load())
                        return;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                std::printf("WATCHDOG text_over_16mb_hung\n");
                std::fflush(stdout);
                std::_Exit(99);
            });
        // 1 列文本行 payload = lenenc 前缀(4B/9B) + N；>=0xFFFFFF 会被拆成满块续包。
        // 16,777,210 及以下单包；16,777,211 起分段；16,777,216 起前缀变 0xFE。
        for (size_t sz : {size_t(16000000), size_t(16777206), size_t(16777210), size_t(16777211),
                          size_t(16777212), size_t(16777216), size_t(17000000)})
        {
            auto c = pool->get_mysql_select_conn();
            long long pre = query_int(c, "SELECT 1");
            reading("text_row_size_" + std::to_string(sz) + "_pre",
                    std::to_string(pre) + " conn=" + std::to_string(conn_id(c)) +
                        " isclose=" + std::to_string((int)c->isclose));
            unsigned int q0 = c->query_num;
            row_capture rc;
            unsigned int n = c->fetch_directly("SELECT REPEAT('a', " + std::to_string(sz) + ")", std::ref(rc));
            asio::error_code aec;
            size_t avail = (c->conn_link && c->conn_link->socket) ? c->conn_link->socket->available(aec) : 0;
            reading("text_row_size_" + std::to_string(sz) + "_result",
                    "rows=" + std::to_string(n) + " captured=" + std::to_string(rc.vals.size()) +
                        " len=" + (rc.vals.size() ? std::to_string(rc.vals[0][0].size()) : "0") +
                        " null0=" + (rc.nulls.size() ? std::to_string((int)rc.nulls[0][0]) : "9") +
                        " code=" + std::to_string(c->error_code) +
                        " q=" + std::to_string(c->query_num - q0) +
                        " avail=" + std::to_string(avail) +
                        " isclose=" + std::to_string((int)c->isclose) +
                        " err=" + c->error_msg.substr(0, 60));
            std::string verdict;
            if (n == 1 && rc.vals.size() == 1 && !(rc.nulls.size() && rc.nulls[0][0]) &&
                rc.vals[0][0].size() == sz)
                verdict = "intact";
            else if (n == 0 && c->error_code == 20)
                verdict = "explicit_error";
            else if (n == 0 && c->error_code == 0)
                verdict = "silent_empty";
            else
                verdict = "silent_corrupt";
            reading("text_row_size_" + std::to_string(sz) + "_verdict", verdict);
            expect(verdict == "intact" || verdict == "explicit_error",
                   "text_row_size_" + std::to_string(sz) + "_no_silent_corruption",
                   "verdict=" + verdict + " rows=" + std::to_string(n) +
                       " captured=" + std::to_string(rc.vals.size()));
            // 非完整返回的连接可能残留续包/终结包，不再复用
            c->hard_close();
            pool->back_mysql_select_conn(std::move(c));
        }

        // 对照臂：同一条 17MB 行，预编译两条路径都应显式报错并把结果集排空
        {
            auto c = pool->get_mysql_select_conn();
            query_int(c, "SELECT 1");
            row_capture prc;
            unsigned int pn = c->fetch_prepared("SELECT REPEAT('a', 17000000)",
                                                std::vector<http::obj_val>{}, std::ref(prc));
            reading("prep_row_size_17000000_result",
                    "rows=" + std::to_string(pn) + " captured=" + std::to_string(prc.vals.size()) +
                        " code=" + std::to_string(c->error_code) +
                        " isclose=" + std::to_string((int)c->isclose) +
                        " err=" + c->error_msg.substr(0, 100));
            expect(pn == 0 && c->error_code == 20 &&
                       c->error_msg.find("over 16MB") != std::string::npos,
                   "prep_row_over_16mb_explicit");
            long long after = query_int(c, "SELECT 43");
            reading("prep_row_size_17000000_after",
                    "sentinel=" + std::to_string(after) + " err=" + c->error_msg.substr(0, 60));
            expect(after == 43, "prep_row_over_16mb_drained_aligned");
            c->hard_close();
            pool->back_mysql_select_conn(std::move(c));
        }
        {
            auto c = pool->get_mysql_select_conn();
            query_int(c, "SELECT 1");
            bin_capture pbc;
            unsigned int pn = c->fetch_prepared_binary("SELECT REPEAT('a', 17000000)",
                                                       std::vector<http::obj_val>{}, std::ref(pbc));
            reading("prep_bin_row_size_17000000_result",
                    "rows=" + std::to_string(pn) + " captured=" + std::to_string(pbc.rows) +
                        " code=" + std::to_string(c->error_code) +
                        " isclose=" + std::to_string((int)c->isclose) +
                        " err=" + c->error_msg.substr(0, 100));
            expect(pn == 0 && c->error_code == 20 &&
                       c->error_msg.find("over 16MB") != std::string::npos,
                   "prep_bin_row_over_16mb_explicit");
            long long after = query_int(c, "SELECT 44");
            reading("prep_bin_row_size_17000000_after",
                    "sentinel=" + std::to_string(after) + " err=" + c->error_msg.substr(0, 60));
            expect(after == 44, "prep_bin_row_over_16mb_drained_aligned");
            c->hard_close();
            pool->back_mysql_select_conn(std::move(c));
        }
        {
            // 0xFD 分片区间：文本路径静默给 NULL，预编译路径应显式报错
            auto c = pool->get_mysql_select_conn();
            query_int(c, "SELECT 1");
            row_capture prc;
            unsigned int pn = c->fetch_prepared("SELECT REPEAT('a', 16777212)",
                                                std::vector<http::obj_val>{}, std::ref(prc));
            reading("prep_row_size_16777212_result",
                    "rows=" + std::to_string(pn) + " captured=" + std::to_string(prc.vals.size()) +
                        " code=" + std::to_string(c->error_code) +
                        " err=" + c->error_msg.substr(0, 100));
            expect(pn == 0 && c->error_code == 20, "prep_row_16777212_explicit");
            long long after = query_int(c, "SELECT 45");
            expect(after == 45, "prep_row_16777212_drained_aligned");
            c->hard_close();
            pool->back_mysql_select_conn(std::move(c));
        }

        // 池污染复现：不关闭脏连接直接归还（isclose=0，池会收下），下一借用者读残余字节
        std::vector<conn_t> held;
        for (int i = 0; i < 32; ++i)
        {
            auto x = pool->get_mysql_select_conn();
            if (!x)
                break;
            held.push_back(std::move(x));
        }
        conn_t dirty;
        if (!held.empty())
        {
            dirty = std::move(held.back());
            held.pop_back();
        }
        else
        {
            dirty = pool->get_mysql_select_conn();
        }
        long long dirty_id = conn_id(dirty);
        void *dirty_raw      = dirty.get();
        row_capture drc;
        unsigned int dn = dirty->fetch_directly("SELECT REPEAT('a', 17000000)", std::ref(drc));
        asio::error_code daec;
        size_t davail = (dirty->conn_link && dirty->conn_link->socket)
                            ? dirty->conn_link->socket->available(daec)
                            : 0;
        pool->back_mysql_select_conn(std::move(dirty));// 未关闭 ⇒ 池收下
        auto next = pool->get_mysql_select_conn();
        asio::error_code naec;
        size_t navail = (next->conn_link && next->conn_link->socket)
                            ? next->conn_link->socket->available(naec)
                            : 0;
        reading("pool_poison_dirty_fetch",
                "rows=" + std::to_string(dn) + " captured=" + std::to_string(drc.vals.size()) +
                    " avail=" + std::to_string(davail) + " conn=" + std::to_string(dirty_id));
        reading("pool_poison_reborrow",
                "same=" + std::to_string((int)(next.get() == dirty_raw)) +
                    " avail=" + std::to_string(navail) +
                    " isclose=" + std::to_string((int)next->isclose) +
                    " is_closed=" + std::to_string((int)next->is_closed()));
        // 下一借用者的查询是否会卡死：给 5s 期限，超时就关连接把它撬开
        std::atomic<bool> qdone{false};
        long long sentinel = -1;
        std::thread qt([&] { sentinel = query_int(next, "SELECT 42"); qdone.store(true); });
        for (int i = 0; i < 50 && !qdone.load(); ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        bool blocked = !qdone.load();
        if (blocked)
            next->hard_close();
        qt.join();
        reading("pool_poison_sentinel",
                "blocked_5s=" + std::to_string((int)blocked) + " sentinel=" + std::to_string(sentinel) +
                    " code=" + std::to_string(next->error_code) +
                    " isclose=" + std::to_string((int)next->isclose) +
                    " err=" + next->error_msg.substr(0, 80));
        expect(!(davail > 0 && navail > 0 && next.get() == dirty_raw), "pool_poison_not_reproduced",
               "avail=" + std::to_string(davail) + " reborrow_avail=" + std::to_string(navail) +
                   " same_conn=" + std::to_string((int)(next.get() == dirty_raw)));
        next->hard_close();
        pool->back_mysql_select_conn(std::move(next));
        for (auto &h : held)
            pool->back_mysql_select_conn(std::move(h));

        done.store(true);
        wd.join();
        expect(query_int(maint_sel, "SELECT 1") == 1, "text_over_16mb_other_conn_ok");
    }

    // ---------- 文本路径 17MB 出站 SQL（单包上限，本地拒绝并关闭连接） ----------
    {
        auto c = pool->get_mysql_edit_conn();
        std::string big = "INSERT INTO _mysql_probe_real (txt_val) VALUES ('" +
                          std::string(17000000, 'a') + "')";
        unsigned int n = c->exec_dml(big);
        reading("text_over_16mb_out_code", std::to_string(n) + "/" + std::to_string(c->error_code));
        reading("text_over_16mb_out_error", c->error_msg.substr(0, 100));
        reading("text_over_16mb_out_isclosed", std::to_string((int)c->is_closed()));
        expect(n == static_cast<unsigned int>(-1) && c->error_code == 20 &&
                   c->error_msg.find("too large") != std::string::npos,
               "text_over_16mb_out_refused");
        pool->back_mysql_edit_conn(std::move(c));// isclose=true ⇒ 池丢弃
        auto c2 = pool->get_mysql_edit_conn();
        expect(c2 && query_int(c2, "SELECT 1") == 1, "text_over_16mb_out_new_conn_ok");
        pool->back_mysql_edit_conn(std::move(c2));
    }

    // cleanup
    maint->exec_dml("DROP TABLE IF EXISTS _mysql_probe_real");
    maint->exec_dml("DROP TABLE IF EXISTS _mysql_probe_alter");
    maint->exec_dml("DROP TABLE IF EXISTS _mysql_probe_txn");
    pool->back_mysql_edit_conn(std::move(maint));
    pool->back_mysql_select_conn(std::move(maint_sel));

    ioc.stop();
    io_thread.join();
    orm::init_orm_conn_pool_release();

    std::printf("SUMMARY PASS=%d FAIL=%d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
