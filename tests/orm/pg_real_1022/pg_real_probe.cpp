// 真实 PG 连接层操作方式探针：预编译 SELECT / binary / 参数类型 / 事务 /
// 断线重连 / ping / 错误恢复 / 异步 / 多线程预编译，以及中止事务 ping、
// 事务中断连的读失败、warm 轮撞唯一约束不重发。
#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <variant>

#include "orm_conn_pool.h"
#include "pg_conn.h"
#include "request.h"

using conn_t = std::shared_ptr<orm::pg_conn_base>;

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

    bool operator()(int cc, char **, std::function<orm::col_value_variant(int)> get)
    {
        if (rows == 0)
        {
            for (int i = 0; i < cc; ++i)
            {
                auto v = get(i);
                types.push_back(static_cast<int>(v.index()));
                if (auto p = std::get_if<int64_t>(&v))
                    texts.push_back(std::to_string(*p));
                else if (auto u = std::get_if<uint64_t>(&v))
                    texts.push_back(std::to_string(*u));
                else if (auto d = std::get_if<double>(&v))
                    texts.push_back(std::to_string(*d));
                else if (auto s = std::get_if<std::string_view>(&v))
                    texts.emplace_back(*s);
                else
                    texts.push_back("<null>");
            }
        }
        ++rows;
        return true;
    }
};

static long long scalar_ll(conn_t &c, const std::string &sql)
{
    long long v = -1;
    c->fetch_directly(sql,
                      [&v](int, char **, auto get) -> bool
                      {
                          auto [p, l] = get(0);
                          if (p)
                              v = std::stoll(std::string(reinterpret_cast<char *>(p), l));
                          return true;
                      });
    return v;
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

static long long count_where(conn_t &c, const std::string &cond)
{
    long long n = -1;
    c->fetch_directly("SELECT count(*) FROM _pgprobe_real WHERE " + cond,
                      [&n](int, char **, auto get) -> bool
                      {
                          auto [p, l] = get(0);
                          if (p)
                              n = std::stoll(std::string(reinterpret_cast<char *>(p), l));
                          return true;
                      });
    return n;
}

static long long backend_pid(conn_t &c)
{
    long long pid = -1;
    c->fetch_directly("SELECT pg_backend_pid()",
                      [&pid](int, char **, auto get) -> bool
                      {
                          auto [p, l] = get(0);
                          if (p)
                              pid = std::stoll(std::string(reinterpret_cast<char *>(p), l));
                          return true;
                      });
    return pid;
}

static bool is_ssl_session(conn_t &c)
{
    bool ssl = false;
    c->fetch_directly("SELECT ssl FROM pg_stat_ssl WHERE pid = pg_backend_pid()",
                      [&ssl](int, char **, auto get) -> bool
                      {
                          auto [p, l] = get(0);
                          ssl          = p && l == 1 && *p == 't';
                          return true;
                      });
    return ssl;
}

int main(int argc, char **argv)
{
    std::string conf = argc > 1 ? argv[1] : "conf/orm.conf";
    asio::io_context ioc;
    orm::init_orm_conn_pool(ioc, conf);
    auto pool = orm::get_orm_conn_pool_obj()["pg"];
    if (!pool)
    {
        std::printf("SETUP_FAIL no [pg] pool (%s)\n", conf.c_str());
        return 2;
    }

    auto edit = pool->get_pg_edit_conn();
    if (!edit || edit->is_closed())
    {
        std::printf("SETUP_FAIL cannot connect to pg\n");
        orm::init_orm_conn_pool_release();
        return 2;
    }
    reading("ssl_session", is_ssl_session(edit) ? "1" : "0");

    edit->clear_error();
    edit->exec_dml("DROP TABLE IF EXISTS _pgprobe_real");
    edit->exec_dml("CREATE TABLE _pgprobe_real (id SERIAL PRIMARY KEY, i INT, s TEXT, b BOOLEAN, d DOUBLE PRECISION, n TEXT)");
    expect(edit->error_msg.empty(), "setup_ddl", edit->error_msg);
    pool->back_pg_edit_conn(std::move(edit));

    // ---- 文本路径 + collect 接口 ----
    {
        auto c = pool->get_pg_edit_conn();
        expect(c->exec_dml("INSERT INTO _pgprobe_real (i, s) VALUES (1, 'a')") == 1, "text_insert");
        pool->back_pg_edit_conn(std::move(c));

        auto sel = pool->get_pg_select_conn();
        row_capture cap;
        unsigned int rows = sel->fetch_directly("SELECT i, s FROM _pgprobe_real WHERE i = 1", std::ref(cap));
        expect(rows == 1 && cap.vals.size() == 1 && cap.vals[0].size() == 2 &&
                   cap.vals[0][0] == "1" && cap.vals[0][1] == "a",
               "text_select", "rows=" + std::to_string(rows));

        std::vector<orm::field_info_t> fields;
        std::vector<orm::pg_row_data_t> rows_out;
        unsigned int affected = 0;
        unsigned int rc       = sel->execute_and_fetch("SELECT i, s FROM _pgprobe_real WHERE i = 1", fields, rows_out, affected);
        expect(rc == 0 && fields.size() == 2 && rows_out.size() == 1 &&
                   !rows_out[0].is_null.empty() && rows_out[0].values[1] == "a",
               "execute_and_fetch", "rc=" + std::to_string(rc) + " fields=" + std::to_string(fields.size()) +
                                        " rows=" + std::to_string(rows_out.size()));
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- 预编译 SELECT ----
    {
        auto sel = pool->get_pg_select_conn();
        row_capture cap;
        std::vector<http::obj_val> params = {http::obj_val(1)};
        unsigned int rows = sel->fetch_prepared("SELECT i, s FROM _pgprobe_real WHERE i = $1", params, std::ref(cap));
        expect(rows == 1 && cap.vals.size() == 1 && cap.vals[0][1] == "a", "prepared_select",
               "rows=" + std::to_string(rows) + " err=" + sel->error_msg);
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- 预编译参数类型：int / 引号分号 / bool / double / NULL / UTF-8 / 空串 ----
    {
        auto c = pool->get_pg_edit_conn();
        std::vector<http::obj_val> params = {http::obj_val(42), http::obj_val("quote'd; drop"),
                                             http::obj_val(true), http::obj_val(3.5), http::obj_val(nullptr)};
        unsigned int n = c->exec_dml_prepared(
            "INSERT INTO _pgprobe_real (i, s, b, d, n) VALUES ($1, $2, $3, $4, $5)", params);
        expect(n == 1, "prepared_insert_typed", "n=" + std::to_string(n) + " err=" + c->error_msg);

        std::string u8 = "中文'\"\\line";
        std::vector<http::obj_val> p2 = {http::obj_val(43), http::obj_val(u8)};
        c->exec_dml_prepared("INSERT INTO _pgprobe_real (i, s) VALUES ($1, $2)", p2);
        std::vector<http::obj_val> p3 = {http::obj_val(44), http::obj_val("")};
        c->exec_dml_prepared("INSERT INTO _pgprobe_real (i, s) VALUES ($1, $2)", p3);
        pool->back_pg_edit_conn(std::move(c));

        auto sel = pool->get_pg_select_conn();
        row_capture cap;
        std::vector<http::obj_val> q = {http::obj_val("quote'd; drop")};
        unsigned int rows = sel->fetch_prepared("SELECT i, s, b, d, n FROM _pgprobe_real WHERE s = $1", q, std::ref(cap));
        bool ok_row = rows == 1 && cap.vals.size() == 1 && cap.vals[0][0] == "42" &&
                      cap.vals[0][1] == "quote'd; drop" && cap.vals[0][2] == "t" &&
                      cap.vals[0][3].rfind("3.5", 0) == 0 && cap.nulls[0][4];
        expect(ok_row, "prepared_typed_readback", "rows=" + std::to_string(rows));

        row_capture cap2;
        std::vector<http::obj_val> q2 = {http::obj_val(u8)};
        sel->fetch_prepared("SELECT s FROM _pgprobe_real WHERE s = $1", q2, std::ref(cap2));
        expect(cap2.vals.size() == 1 && cap2.vals[0][0] == u8, "prepared_utf8_roundtrip");

        row_capture cap3;
        std::vector<http::obj_val> q3 = {http::obj_val(44)};
        sel->fetch_prepared("SELECT s, n FROM _pgprobe_real WHERE i = $1", q3, std::ref(cap3));
        expect(cap3.vals.size() == 1 && !cap3.nulls[0][0] && cap3.vals[0][0].empty() && cap3.nulls[0][1],
               "empty_string_vs_null");
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- 预编译结果元数据缓存 ----
    {
        auto sel = pool->get_pg_select_conn();
        sel->prepared_reset_stats();
        const std::string sql = "SELECT i FROM _pgprobe_real WHERE i = $1 -- cache probe";
        for (int k = 0; k < 5; ++k)
        {
            row_capture cap;
            std::vector<http::obj_val> params = {http::obj_val(42)};
            sel->fetch_prepared(sql, params, std::ref(cap));
        }
        reading("prepared_misses", std::to_string(sel->prepared_misses()));
        reading("prepared_hits", std::to_string(sel->prepared_hits()));
        expect(sel->prepared_misses() >= 1 && sel->prepared_hits() >= 4, "prepared_meta_cache");
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- binary 协议：冷包要 text、热包才按白名单要 binary ----
    {
        auto sel = pool->get_pg_select_conn();
        const std::string bsql = "SELECT i, s, b, d, n FROM _pgprobe_real WHERE i = $1 -- binary probe";
        std::vector<http::obj_val> params = {http::obj_val(42)};
        bin_capture cold;
        unsigned int rows = sel->fetch_prepared_binary(bsql, params, std::ref(cold));
        bin_capture warm;
        unsigned int rows2 = sel->fetch_prepared_binary(bsql, params, std::ref(warm));
        auto join_types  = [](const bin_capture &c)
        {
            std::string s;
            for (size_t k = 0; k < c.types.size(); ++k)
                s += (k ? "," : "") + std::to_string(c.types[k]);
            return s.empty() ? std::string("-1") : s;
        };
        reading("binary_types_cold", join_types(cold));
        reading("binary_types_warm", join_types(warm));
        reading("binary_warm_i_hex", warm.texts.empty() ? std::string() : hex_of(warm.texts[0]));
        reading("binary_warm_b_hex", warm.texts.size() > 2 ? hex_of(warm.texts[2]) : std::string());
        bool ok_cold = rows == 1 && cold.types.size() == 5 && cold.texts[0] == "42" &&
                       cold.texts[1] == "quote'd; drop" && cold.texts[4] == "<null>" &&
                       cold.types[4] == orm::VI_MONO;
        bool ok_warm = rows2 == 1 && warm.types.size() == 5 && warm.texts[0] == "42" &&
                       warm.texts[1] == "quote'd; drop" && warm.texts[4] == "<null>" &&
                       warm.types[0] == orm::VI_INT && warm.types[1] == orm::VI_SV &&
                       warm.types[2] == orm::VI_INT && warm.types[3] == orm::VI_DBL &&
                       warm.types[4] == orm::VI_MONO;
        expect(ok_cold && ok_warm, "prepared_binary_select",
               "rows=" + std::to_string(rows) + "/" + std::to_string(rows2) + " err=" + sel->error_msg);
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- binary 解码：date / time / timestamp(tz) / numeric / 整数 / 浮点 / bool ----
    {
        auto sel = pool->get_pg_select_conn();
        const std::string dsql =
            "SELECT DATE '2024-02-29', TIMESTAMP '2024-02-29 12:34:56.789', "
            "TIMESTAMPTZ '2024-02-29 12:34:56+00', TIME '12:34:56.789', TIMETZ '12:34:56.789+08', "
            "NUMERIC '12345.6789', BIGINT '42', SMALLINT '-7', REAL '1.5', "
            "DOUBLE PRECISION '2.25', BOOLEAN 'true' -- type probe";
        bin_capture cold;
        unsigned int cr = sel->fetch_prepared_binary(dsql, {}, std::ref(cold));
        bin_capture warm;
        unsigned int wr = sel->fetch_prepared_binary(dsql, {}, std::ref(warm));
        std::string tz;
        {
            row_capture zc;
            sel->fetch_directly("SHOW TimeZone", std::ref(zc));
            if (!zc.vals.empty() && !zc.vals[0].empty())
                tz = zc.vals[0][0];
        }
        reading("server_timezone", tz);
        reading("type_text_cold", cold.texts.empty() ? std::string() : cold.texts[2]);
        reading("type_text_warm", warm.texts.empty() ? std::string() : warm.texts[2]);
        reading("timetz_cold", cold.texts.size() > 4 ? cold.texts[4] : std::string());
        reading("timetz_warm", warm.texts.size() > 4 ? warm.texts[4] : std::string());
        reading("type_warm_date_hex", warm.texts.empty() ? std::string() : hex_of(warm.texts[0]));
        reading("type_warm_ts_hex", warm.texts.size() > 1 ? hex_of(warm.texts[1]) : std::string());
        reading("type_warm_num_hex", warm.texts.size() > 5 ? hex_of(warm.texts[5]) : std::string());
        bool ok_cold = cr == 1 && cold.texts.size() == 11 && cold.texts[0] == "2024-02-29" &&
                       cold.texts[1] == "2024-02-29 12:34:56.789" && cold.texts[5] == "12345.6789" &&
                       cold.texts[6] == "42" && cold.texts[7] == "-7" && cold.texts[10] == "t";
        bool ok_warm = wr == 1 && warm.texts.size() == 11 && warm.texts[0] == "2024-02-29" &&
                       warm.texts[1] == "2024-02-29 12:34:56.789000" &&
                       warm.texts[3] == "12:34:56.789000" && warm.texts[5] == "12345.6789" &&
                       warm.texts[6] == "42" && warm.texts[7] == "-7" &&
                       warm.texts[8] == "1.500000" && warm.texts[9] == "2.250000" && warm.texts[10] == "1" &&
                       warm.types[6] == orm::VI_INT && warm.types[7] == orm::VI_INT &&
                       warm.types[8] == orm::VI_DBL && warm.types[9] == orm::VI_DBL &&
                       warm.types[10] == orm::VI_INT;
        expect(ok_cold && ok_warm, "prepared_binary_types",
               "cold=" + std::to_string(cr) + " warm=" + std::to_string(wr) + " err=" + sel->error_msg);
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- 预编译 0 行 ----
    {
        auto sel = pool->get_pg_select_conn();
        row_capture zc;
        std::vector<http::obj_val> zp = {http::obj_val(-999999)};
        unsigned int zr  = sel->fetch_prepared("SELECT i FROM _pgprobe_real WHERE i = $1", zp, std::ref(zc));
        bin_capture zb;
        unsigned int zbr = sel->fetch_prepared_binary("SELECT i FROM _pgprobe_real WHERE i = $1", zp, std::ref(zb));
        expect(zr == 0 && zc.vals.empty() && zbr == 0 && zb.rows == 0, "prepared_zero_rows",
               "text=" + std::to_string(zr) + " bin=" + std::to_string(zbr));
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- warm 轮撞唯一约束：不许冷重发 ----
    {
        auto c = pool->get_pg_edit_conn();
        c->clear_error();
        c->exec_dml("DROP TABLE IF EXISTS _pgprobe_dup");
        c->exec_dml("DROP SEQUENCE IF EXISTS _pgprobe_dup_seq");
        c->exec_dml("CREATE SEQUENCE _pgprobe_dup_seq START 1");
        c->exec_dml("CREATE TABLE _pgprobe_dup (id INT PRIMARY KEY, s TEXT)");
        expect(c->error_msg.empty(), "dup_setup", c->error_msg);

        // nextval 不随语句回滚：冷重发一次就多前进一格，把"重发了几次"变成可读数
        const std::string dup_sql =
            "INSERT INTO _pgprobe_dup (id, s) VALUES (1, nextval('_pgprobe_dup_seq')::text) -- 23505 probe";
        std::vector<http::obj_val> no_params;
        unsigned int first = c->exec_dml_prepared(dup_sql, no_params);
        expect(first == 1, "dup_insert_first", "n=" + std::to_string(first) + " err=" + c->error_msg);

        long long seq_before = scalar_ll(c, "SELECT last_value FROM _pgprobe_dup_seq");
        c->prepared_reset_stats();
        unsigned int second = c->exec_dml_prepared(dup_sql, no_params);
        std::string serr    = c->error_msg;
        uint64_t smiss      = c->prepared_misses();
        uint64_t shit       = c->prepared_hits();
        long long seq_after = scalar_ll(c, "SELECT last_value FROM _pgprobe_dup_seq");
        reading("warm_23505_error", serr);
        reading("warm_23505_seq_before", std::to_string(seq_before));
        reading("warm_23505_seq_after", std::to_string(seq_after));
        reading("warm_23505_misses", std::to_string(smiss));
        reading("warm_23505_hits", std::to_string(shit));
        expect(second == static_cast<unsigned int>(-1) && !serr.empty(), "warm_23505_rejected", serr);
        expect(seq_before > 0 && seq_after == seq_before + 1, "warm_23505_no_cold_resend",
               "before=" + std::to_string(seq_before) + " after=" + std::to_string(seq_after));
        expect(smiss == 0 && shit >= 1, "warm_23505_stays_warm",
               "hits=" + std::to_string(shit) + " misses=" + std::to_string(smiss));

        c->exec_dml("DROP TABLE _pgprobe_dup");
        c->exec_dml("DROP SEQUENCE _pgprobe_dup_seq");
        pool->back_pg_edit_conn(std::move(c));
    }

    // ---- 大行：1MB 文本跨 DataRow 报文组装 ----
    {
        std::string big(1024 * 1024, 'x');
        big.replace(0, 4, "HEAD");
        big.replace(big.size() - 4, 4, "TAIL");
        auto c = pool->get_pg_edit_conn();
        std::vector<http::obj_val> ip = {http::obj_val(300), http::obj_val(big)};
        unsigned int ins = c->exec_dml_prepared("INSERT INTO _pgprobe_real (i, s) VALUES ($1, $2)", ip);
        pool->back_pg_edit_conn(std::move(c));

        auto sel = pool->get_pg_select_conn();
        row_capture lc;
        std::vector<http::obj_val> qp = {http::obj_val(300)};
        unsigned int lrows = sel->fetch_prepared("SELECT s FROM _pgprobe_real WHERE i = $1", qp, std::ref(lc));
        bin_capture lb;
        unsigned int brows = sel->fetch_prepared_binary("SELECT s FROM _pgprobe_real WHERE i = $1", qp, std::ref(lb));
        bool ok_text = lrows == 1 && lc.vals.size() == 1 && lc.vals[0][0] == big;
        bool ok_bin  = brows == 1 && lb.texts.size() == 1 && lb.texts[0] == big;
        reading("large_row_bytes", std::to_string(ok_text ? lc.vals[0][0].size() : 0));
        expect(ins == 1 && ok_text && ok_bin, "large_row_roundtrip",
               "ins=" + std::to_string(ins) + " text=" + std::to_string(ok_text) + " bin=" + std::to_string(ok_bin));
        pool->back_pg_select_conn(std::move(sel));

        c = pool->get_pg_edit_conn();
        c->exec_dml("DELETE FROM _pgprobe_real WHERE i = 300");
        pool->back_pg_edit_conn(std::move(c));
    }

    // ---- 事务：COMMIT / ROLLBACK ----
    {
        auto c = pool->get_pg_edit_conn();
        c->exec_dml("BEGIN");
        expect(c->in_transaction(), "begin_sets_flag", c->error_msg);
        expect(c->exec_dml("INSERT INTO _pgprobe_real (i, s) VALUES (100, 'commit')") == 1, "txn_insert");
        c->exec_dml("COMMIT");
        expect(!c->in_transaction(), "commit_clears_flag", c->error_msg);
        pool->back_pg_edit_conn(std::move(c));

        c = pool->get_pg_edit_conn();
        c->exec_dml("BEGIN");
        c->exec_dml("INSERT INTO _pgprobe_real (i, s) VALUES (101, 'rollback')");
        c->exec_dml("ROLLBACK");
        expect(!c->in_transaction(), "rollback_clears_flag", c->error_msg);
        pool->back_pg_edit_conn(std::move(c));

        auto sel = pool->get_pg_select_conn();
        expect(count_where(sel, "i = 100") == 1, "commit_visible");
        expect(count_where(sel, "i = 101") == 0, "rollback_invisible");
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- 未结束事务归还连接池 ----
    {
        auto c1 = pool->get_pg_edit_conn();
        c1->exec_dml("BEGIN");
        c1->exec_dml("INSERT INTO _pgprobe_real (i, s) VALUES (102, 'leaked')");
        pool->back_pg_edit_conn(std::move(c1));// 故意不提交/不回滚

        auto c2 = pool->get_pg_edit_conn();
        reading("pool_return_keeps_txn", c2->in_transaction() ? "1" : "0");
        if (c2->in_transaction())
        {
            c2->exec_dml("ROLLBACK");
        }
        c2->clear_error();
        expect(c2->exec_dml("INSERT INTO _pgprobe_real (i, s) VALUES (103, 'after')") == 1, "conn_usable_after_leak");
        pool->back_pg_edit_conn(std::move(c2));

        auto sel = pool->get_pg_select_conn();
        expect(count_where(sel, "i = 102") == 0, "leaked_txn_not_committed");
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- ping ----
    {
        auto c = pool->get_pg_edit_conn();
        expect(c->ping(), "ping_live", c->error_msg);
        pool->back_pg_edit_conn(std::move(c));
    }

    // ---- 中止事务上的 ping：必须报错，且不留残帧 ----
    {
        auto c = pool->get_pg_edit_conn();
        c->exec_dml("BEGIN");
        // 让事务进入 aborted 态：随后的 SELECT 1 会收到 25P02 ErrorResponse
        c->exec_dml("SELECT * FROM no_such_table_ping");
        expect(c->in_transaction(), "ping_aborted_arm", c->error_msg);

        bool alive       = c->ping();
        std::string perr = c->error_msg;
        reading("ping_aborted_error", perr);
        reading("ping_aborted_code", std::to_string(c->error_code));
        expect(!alive && !perr.empty() && c->error_code == 21, "ping_aborted_txn_false", perr);

        // ping 的 ErrorResponse 后面还有一条 ReadyForQuery；没读掉的话这里会先吃到它
        c->exec_dml("ROLLBACK");
        expect(!c->in_transaction(), "ping_aborted_rollback_ok", c->error_msg);
        expect(c->ping(), "ping_after_rollback", c->error_msg);
        pool->back_pg_edit_conn(std::move(c));
    }

    // ---- 错误恢复：SQL 错误后连接仍可用 ----
    {
        auto c = pool->get_pg_edit_conn();
        unsigned int bad = c->exec_dml("THIS IS NOT SQL");
        expect(bad == static_cast<unsigned int>(-1) && !c->error_msg.empty(), "bad_sql_reports_error");
        reading("bad_sql_error", c->error_msg);
        expect(c->exec_dml("INSERT INTO _pgprobe_real (i, s) VALUES (110, 'recovered')") == 1, "usable_after_bad_dml");

        row_capture cap;
        unsigned int rows = c->fetch_directly("SELECT * FROM no_such_table_xyz", std::ref(cap));
        expect(rows == 0 && !c->error_msg.empty(), "bad_select_reports_error");
        expect(c->fetch_directly("SELECT 1", cap) == 1, "usable_after_bad_select");
        pool->back_pg_edit_conn(std::move(c));
    }

    // ---- 事务中杀后端：拒绝重连重发 ----
    {
        auto victim = pool->get_pg_edit_conn();
        long long pid1 = backend_pid(victim);
        victim->exec_dml("BEGIN");
        victim->exec_dml("INSERT INTO _pgprobe_real (i, s) VALUES (200, 'txn_pre_kill')");
        expect(victim->in_transaction(), "kill_arm_txn_open");

        auto other = pool->get_pg_edit_conn();
        row_capture kcap;
        other->fetch_directly("SELECT pg_terminate_backend(" + std::to_string(pid1) + ")", std::ref(kcap));
        expect(kcap.vals.size() == 1, "kill_backend_issued");

        unsigned int r = victim->exec_dml("INSERT INTO _pgprobe_real (i, s) VALUES (201, 'must_not_commit')");
        reading("txn_kill_result", std::to_string(r));
        reading("txn_kill_in_transaction", victim->in_transaction() ? "1" : "0");
        reading("txn_kill_is_closed", victim->is_closed() ? "1" : "0");
        reading("txn_kill_error", victim->error_msg);
        expect(r == static_cast<unsigned int>(-1), "txn_kill_dml_fails");
        expect(count_where(other, "i IN (200, 201)") == 0, "txn_kill_no_silent_commit");
        pool->back_pg_edit_conn(std::move(victim));
        pool->back_pg_edit_conn(std::move(other));

        auto c = pool->get_pg_edit_conn();
        expect(c->ping(), "pool_healthy_after_kill");
        pool->back_pg_edit_conn(std::move(c));
    }

    // ---- 事务中杀后端：SELECT 读路径同样不许重连重发 ----
    {
        auto killer = pool->get_pg_edit_conn();
        // 两条受害者先取到手，避免断连的旧连接经池的 FIFO 被下一段拿走
        auto v1 = pool->get_pg_edit_conn();
        auto v2 = pool->get_pg_edit_conn();
        std::vector<http::obj_val> pp = {http::obj_val(42)};

        // (a) 文本路径：重连重发会连到新会话并返回 1 行
        long long pid1 = backend_pid(v1);
        v1->exec_dml("BEGIN");
        expect(v1->in_transaction(), "txn_kill_read_direct_arm", v1->error_msg);
        row_capture k1;
        killer->fetch_directly("SELECT pg_terminate_backend(" + std::to_string(pid1) + ")", std::ref(k1));
        row_capture r1;
        unsigned int n1 = v1->fetch_directly("SELECT 1", std::ref(r1));
        std::string e1  = v1->error_msg;
        reading("txn_kill_read_direct_rows", std::to_string(n1));
        reading("txn_kill_read_direct_error", e1);
        reading("txn_kill_read_direct_is_closed", v1->is_closed() ? "1" : "0");
        reading("txn_kill_read_direct_isclose", v1->isclose ? "1" : "0");
        // 读失败必须把自己标记为不可用，否则池会把死连接再交出去
        expect(n1 == 0 && !e1.empty() && v1->isclose, "txn_kill_read_direct_fails", e1);
        pool->back_pg_edit_conn(std::move(v1));

        // (b) 预编译路径：warm 命中后被断连，冷重发同样会凭空返回 1 行
        long long pid2 = backend_pid(v2);
        row_capture warmup;
        v2->fetch_prepared("SELECT s FROM _pgprobe_real WHERE i = $1", pp, std::ref(warmup));
        v2->exec_dml("BEGIN");
        expect(v2->in_transaction(), "txn_kill_read_prepared_arm", v2->error_msg);
        row_capture k2;
        killer->fetch_directly("SELECT pg_terminate_backend(" + std::to_string(pid2) + ")", std::ref(k2));
        row_capture r2;
        unsigned int n2 = v2->fetch_prepared("SELECT s FROM _pgprobe_real WHERE i = $1", pp, std::ref(r2));
        std::string e2  = v2->error_msg;
        reading("txn_kill_read_prepared_rows", std::to_string(n2));
        reading("txn_kill_read_prepared_error", e2);
        reading("txn_kill_read_prepared_isclose", v2->isclose ? "1" : "0");
        expect(n2 == 0 && !e2.empty() && v2->isclose, "txn_kill_read_prepared_fails", e2);
        pool->back_pg_edit_conn(std::move(v2));

        expect(killer->ping(), "pool_healthy_after_txn_read_kill", killer->error_msg);
        pool->back_pg_edit_conn(std::move(killer));
    }

    // ---- 自动提交语义下杀后端：读失败必须报错并判死，不重连重发 ----
    {
        auto victim = pool->get_pg_edit_conn();
        long long pid1 = backend_pid(victim);
        auto other = pool->get_pg_edit_conn();
        row_capture kcap;
        other->fetch_directly("SELECT pg_terminate_backend(" + std::to_string(pid1) + ")", std::ref(kcap));
        pool->back_pg_edit_conn(std::move(other));

        // 写已上 wire、回执丢失 ⇒ 结果不可知：报错并把连接判死
        row_capture cap;
        unsigned int rows = victim->fetch_directly("SELECT pg_backend_pid()", std::ref(cap));
        std::string e1    = victim->error_msg;
        reading("autocommit_pid_before", std::to_string(pid1));
        reading("autocommit_rows_after_kill", std::to_string(rows));
        reading("autocommit_error_code", std::to_string(victim->error_code));
        reading("autocommit_error", e1);
        reading("autocommit_isclose", victim->isclose ? "1" : "0");
        expect(rows == 0 && !e1.empty() && victim->isclose, "autocommit_read_fails", e1);

        // 同一对象不得凭空复活
        unsigned int again = victim->fetch_directly("SELECT 1", std::ref(cap));
        reading("autocommit_second_rows", std::to_string(again));
        expect(again == 0 && victim->isclose, "autocommit_no_reconnect_same_object");

        // 交回池必须被丢弃：池里不得出现判死的连接
        pool->back_pg_edit_conn(std::move(victim));
        bool any_dead = false;
        for (auto &c : pool->pg_edit_pool)
            if (c->isclose || c->is_closed())
                any_dead = true;
        reading("autocommit_dead_conn_pooled", any_dead ? "1" : "0");
        expect(!any_dead, "pool_drops_dead_conn");

        // 池里剩下的连接都可用；prepared 在新会话上重新 Parse
        auto fresh    = pool->get_pg_edit_conn();
        row_capture fcap;
        unsigned int frows = fresh->fetch_directly("SELECT pg_backend_pid()", std::ref(fcap));
        long long pid2 = (frows == 1 && fcap.vals.size() == 1) ? std::stoll(fcap.vals[0][0]) : -1;
        reading("autocommit_pid_after", std::to_string(pid2));
        expect(frows == 1 && pid2 > 0 && pid2 != pid1, "pool_healthy_after_read_kill", fresh->error_msg);

        row_capture pcap;
        std::vector<http::obj_val> pp = {http::obj_val(42)};
        unsigned int prows = fresh->fetch_prepared("SELECT s FROM _pgprobe_real WHERE i = $1", pp, std::ref(pcap));
        expect(prows == 1 && pcap.vals.size() == 1 && pcap.vals[0][0] == "quote'd; drop",
               "prepared_on_fresh_conn", "rows=" + std::to_string(prows) + " err=" + fresh->error_msg);
        pool->back_pg_edit_conn(std::move(fresh));
    }

    // ---- 异步入口 ----
    {
        auto aedit = pool->get_pg_edit_conn();
        auto asel  = pool->get_pg_select_conn();
        struct async_out
        {
            unsigned int ins = 0, upd = 0, del = 0, sel_rows = 0, txt_rows = 0, bin_rows = 0, bin_rows2 = 0;
            std::string s;
            std::string bin_types2, bin_i_hex2;
        } out;

        asio::co_spawn(
            ioc,
            [&]() -> asio::awaitable<void>
            {
                out.ins = co_await aedit->async_exec_dml(
                    "INSERT INTO _pgprobe_real (i, s) VALUES (300, 'async')");
                std::vector<http::obj_val> up = {http::obj_val("async2"), http::obj_val(300)};
                out.upd = co_await aedit->async_exec_dml_prepared(
                    "UPDATE _pgprobe_real SET s = $1 WHERE i = $2", up);
                row_capture cap;
                std::vector<http::obj_val> q = {http::obj_val(300)};
                out.sel_rows = co_await asel->async_fetch_prepared(
                    "SELECT s FROM _pgprobe_real WHERE i = $1", q, std::ref(cap));
                out.s = cap.vals.empty() ? "" : cap.vals[0][0];
                row_capture tcap;
                out.txt_rows = co_await asel->async_fetch_directly("SELECT 1", std::ref(tcap));
                bin_capture bcap;
                std::vector<http::obj_val> bq = {http::obj_val(42)};
                out.bin_rows = co_await asel->async_fetch_prepared_binary(
                    "SELECT i FROM _pgprobe_real WHERE i = $1", bq, std::ref(bcap));
                bin_capture bwarm;
                out.bin_rows2 = co_await asel->async_fetch_prepared_binary(
                    "SELECT i FROM _pgprobe_real WHERE i = $1", bq, std::ref(bwarm));
                out.bin_types2 = bwarm.types.empty() ? std::string() : std::to_string(bwarm.types[0]);
                out.bin_i_hex2 = bwarm.texts.empty() ? std::string() : hex_of(bwarm.texts[0]);
                out.del = co_await aedit->async_exec_dml(
                    "DELETE FROM _pgprobe_real WHERE i = 300");
                co_return;
            },
            asio::detached);
        ioc.run();

        expect(out.ins == 1 && out.upd == 1 && out.del == 1, "async_dml",
               "ins=" + std::to_string(out.ins) + " upd=" + std::to_string(out.upd) +
                   " del=" + std::to_string(out.del));
        expect(out.sel_rows == 1 && out.s == "async2", "async_fetch_prepared", out.s);
        expect(out.txt_rows == 1, "async_fetch_directly");
        expect(out.bin_rows == 1, "async_fetch_prepared_binary");
        reading("async_binary_warm_type", out.bin_types2);
        reading("async_binary_warm_hex", out.bin_i_hex2);
        expect(out.bin_rows2 == 1 && out.bin_types2 == "1" && out.bin_i_hex2 == "3432",
               "async_binary_warm", "hex=" + out.bin_i_hex2 + " type=" + out.bin_types2);
        pool->back_pg_edit_conn(std::move(aedit));
        pool->back_pg_select_conn(std::move(asel));
    }

    // ---- 冷轮在 Describe 之前失败（Bind 参数转换错）：槽位不得带空列信息进热轮 ----
    {
        auto sel = pool->get_pg_select_conn();
        std::vector<http::obj_val> bad  = {http::obj_val("abc")};
        std::vector<http::obj_val> good = {http::obj_val(42)};

        // 文本路径：第一次冷包 Parse 成功、Bind 失败 ⇒ 槽位只有语句名；
        // 第二次同 SQL 用合法参数必须取回那一行（旧实现在热轮按空列信息静默丢行，返回 0 行且无错误）。
        const std::string tsql = "SELECT i FROM _pgprobe_real WHERE i = $1 -- bind_err probe";
        row_capture tc1;
        unsigned int tr1  = sel->fetch_prepared(tsql, bad, std::ref(tc1));
        std::string terr1 = sel->error_msg;
        row_capture tc2;
        unsigned int tr2  = sel->fetch_prepared(tsql, good, std::ref(tc2));
        std::string terr2 = sel->error_msg;
        sel->prepared_reset_stats();
        row_capture tc3;
        unsigned int tr3 = sel->fetch_prepared(tsql, good, std::ref(tc3));
        reading("bind_err_text_first_rows", std::to_string(tr1));
        reading("bind_err_text_first_error", terr1);
        reading("bind_err_text_second_rows", std::to_string(tr2));
        reading("bind_err_text_second_error", terr2);
        reading("bind_err_text_third_rows", std::to_string(tr3));
        reading("bind_err_text_hits", std::to_string(sel->prepared_hits()));
        expect(tr1 == 0 && !terr1.empty(), "bind_err_text_reports", terr1);
        expect(tr2 == 1 && tc2.vals.size() == 1 && tc2.vals[0][0] == "42" && terr2.empty(),
               "bind_err_text_then_valid", "rows=" + std::to_string(tr2) + " err=" + terr2);
        expect(tr3 == 1 && tc3.vals.size() == 1 && tc3.vals[0][0] == "42" && sel->prepared_hits() >= 1,
               "bind_err_text_warm_after", "rows=" + std::to_string(tr3) +
                                               " hits=" + std::to_string(sel->prepared_hits()));

        // binary 路径同源孪生
        const std::string bsql = "SELECT i FROM _pgprobe_real WHERE i = $1 -- bind_err bin probe";
        bin_capture bc1;
        unsigned int br1  = sel->fetch_prepared_binary(bsql, bad, std::ref(bc1));
        std::string berr1 = sel->error_msg;
        bin_capture bc2;
        unsigned int br2  = sel->fetch_prepared_binary(bsql, good, std::ref(bc2));
        std::string berr2 = sel->error_msg;
        sel->prepared_reset_stats();
        bin_capture bc3;
        unsigned int br3 = sel->fetch_prepared_binary(bsql, good, std::ref(bc3));
        reading("bind_err_bin_first_rows", std::to_string(br1));
        reading("bind_err_bin_first_error", berr1);
        reading("bind_err_bin_second_rows", std::to_string(br2));
        reading("bind_err_bin_second_error", berr2);
        reading("bind_err_bin_third_rows", std::to_string(br3));
        reading("bind_err_bin_hits", std::to_string(sel->prepared_hits()));
        expect(br1 == 0 && !berr1.empty(), "bind_err_bin_reports", berr1);
        expect(br2 == 1 && bc2.rows == 1 && bc2.texts.size() == 1 && bc2.texts[0] == "42" && berr2.empty(),
               "bind_err_bin_then_valid", "rows=" + std::to_string(br2) + " err=" + berr2);
        expect(br3 == 1 && bc3.rows == 1 && sel->prepared_hits() >= 1, "bind_err_bin_warm_after",
               "rows=" + std::to_string(br3) + " hits=" + std::to_string(sel->prepared_hits()));
        pool->back_pg_select_conn(std::move(sel));
    }

    // ---- 异步孪生：同一条自愈路径 ----
    {
        auto asel = pool->get_pg_select_conn();
        struct bind_err_async_out
        {
            unsigned int t1 = 0, t2 = 0, b1 = 0, b2 = 0;
            std::string e1, e2, be1, be2, tval, bval;
        } ao;
        ioc.restart();// 上一个 run() 无活可干返回后 ioc 处于 stopped，不重启则 co_spawn 的启动帧不会被派发
        asio::co_spawn(
            ioc,
            [&]() -> asio::awaitable<void>
            {
                std::vector<http::obj_val> bad  = {http::obj_val("abc")};
                std::vector<http::obj_val> good = {http::obj_val(42)};
                row_capture c1;
                ao.t1 = co_await asel->async_fetch_prepared(
                    "SELECT i FROM _pgprobe_real WHERE i = $1 -- bind_err async probe", bad, std::ref(c1));
                ao.e1 = asel->error_msg;
                row_capture c2;
                ao.t2   = co_await asel->async_fetch_prepared(
                    "SELECT i FROM _pgprobe_real WHERE i = $1 -- bind_err async probe", good, std::ref(c2));
                ao.e2   = asel->error_msg;
                ao.tval = c2.vals.empty() ? std::string() : c2.vals[0][0];
                bin_capture d1;
                ao.b1 = co_await asel->async_fetch_prepared_binary(
                    "SELECT i FROM _pgprobe_real WHERE i = $1 -- bind_err async bin probe", bad, std::ref(d1));
                ao.be1 = asel->error_msg;
                bin_capture d2;
                ao.b2   = co_await asel->async_fetch_prepared_binary(
                    "SELECT i FROM _pgprobe_real WHERE i = $1 -- bind_err async bin probe", good, std::ref(d2));
                ao.be2  = asel->error_msg;
                ao.bval = d2.texts.empty() ? std::string() : d2.texts[0];
                co_return;
            },
            asio::detached);
        ioc.run();
        reading("bind_err_async_text", std::to_string(ao.t1) + "/" + std::to_string(ao.t2) + " v=" + ao.tval);
        reading("bind_err_async_text_error", ao.e1);
        reading("bind_err_async_bin", std::to_string(ao.b1) + "/" + std::to_string(ao.b2) + " v=" + ao.bval);
        reading("bind_err_async_bin_error", ao.be1);
        expect(ao.t1 == 0 && !ao.e1.empty() && ao.t2 == 1 && ao.tval == "42" && ao.e2.empty(),
               "bind_err_async_text_selfheal");
        expect(ao.b1 == 0 && !ao.be1.empty() && ao.b2 == 1 && ao.bval == "42" && ao.be2.empty(),
               "bind_err_async_bin_selfheal");
        pool->back_pg_select_conn(std::move(asel));
    }

    // ---- 多线程预编译 ----
    {
        constexpr int kThreads = 4;
        constexpr int kIters   = 30;
        std::atomic<int> errors{0};
        std::vector<std::thread> ts;
        for (int t = 0; t < kThreads; ++t)
        {
            ts.emplace_back([&, t]()
            {
                for (int k = 0; k < kIters; ++k)
                {
                    int val = 1000 + t * 100 + k;
                    auto c  = pool->get_pg_edit_conn();
                    std::vector<http::obj_val> params = {http::obj_val(val), http::obj_val("mt_" + std::to_string(val))};
                    unsigned int n = c->exec_dml_prepared(
                        "INSERT INTO _pgprobe_real (i, s) VALUES ($1, $2)", params);
                    if (n != 1)
                        ++errors;
                    pool->back_pg_edit_conn(std::move(c));
                }
            });
        }
        for (auto &th : ts)
            th.join();
        expect(errors.load() == 0, "mt_prepared_insert", "errors=" + std::to_string(errors.load()));

        auto sel = pool->get_pg_select_conn();
        expect(count_where(sel, "i >= 1000 AND i < 2000") == kThreads * kIters, "mt_prepared_count");
        pool->back_pg_select_conn(std::move(sel));
    }

    {
        auto c = pool->get_pg_edit_conn();
        c->exec_dml("DROP TABLE _pgprobe_real");
        pool->back_pg_edit_conn(std::move(c));
    }

    orm::init_orm_conn_pool_release();
    std::printf("SUMMARY PASS=%d FAIL=%d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
