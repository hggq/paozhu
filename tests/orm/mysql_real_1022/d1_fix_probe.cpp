// D1/D3 落码后的行为判据（J-1..J-3、J-5）：文本大行必须显式报错、
// 结果集必须排空到终结包、连接不关闭也要能继续用、回池后下一借用者不受污染。
// 判据只认预登记的固定尺寸表，不看运行期数据量。
#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <future>
#include <map>
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
static std::string g_arm = "startup";

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
    std::fflush(stdout);
}

struct row_capture
{
    int cols = -1;
    std::vector<std::vector<std::string>> vals;
    std::vector<std::vector<bool>> nulls;

    bool operator()(int cc, char **, std::function<std::tuple<unsigned char *, size_t>(int)> get)
    {
        if (cols < 0)
            cols = cc;
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

static asio::awaitable<long long> aquery_int(conn_t &c, const std::string &sql)
{
    long long n = -1;
    co_await c->async_fetch_directly(
        sql,
        [&n](int, char **, auto get) -> bool
        {
            auto [p, l] = get(0);
            if (p)
                n = std::stoll(std::string(reinterpret_cast<char *>(p), l));
            return true;
        });
    co_return n;
}

// 载荷 = lenenc 前缀 + 数据；单列 N 的前缀在 N < 2^24 时是 4 字节，之后是 9 字节。
static size_t payload_of(size_t n)
{
    return n + (n < (1u << 24) ? 4u : 9u);
}

static const unsigned int PKT_MAX = 0xFFFFFFu;

// 预登记的判据表：载荷 < 0xFFFFFF 必须完整，>= 0xFFFFFF 必须显式错误
static std::string want_of(size_t n)
{
    return payload_of(n) < PKT_MAX ? "intact" : "explicit_error";
}

static std::string verdict_of(unsigned int rows, row_capture &rc, size_t n, unsigned int code,
                              conn_t &c)
{
    if (rows == 1 && rc.vals.size() == 1 && !(rc.nulls.size() && rc.nulls[0][0]) &&
        rc.vals[0][0].size() == n)
        return "intact";
    if (rows == 0 && code == 20 && c->error_msg.find("over 16MB") != std::string::npos)
        return "explicit_error";
    if (rows == 0 && code == 0)
        return "silent_empty";
    return "silent_corrupt";
}

static size_t avail_bytes(conn_t &c)
{
    asio::error_code aec;
    return (c->conn_link && c->conn_link->socket) ? c->conn_link->socket->available(aec) : 0;
}

// ---------- 同步大行臂 ----------
static void sync_big_row_arm(pool_t &pool, size_t n)
{
    std::string tag = std::to_string(n);
    g_arm = "sync_text_row_" + tag;
    auto c = pool->get_mysql_select_conn();
    long long pre = query_int(c, "SELECT 1");
    row_capture rc;
    unsigned int rows = c->fetch_directly("SELECT REPEAT('a', " + tag + ")", std::ref(rc));
    std::string v = verdict_of(rows, rc, n, c->error_code, c);
    reading("sync_row_" + tag,
            "verdict=" + v + " want=" + want_of(n) + " rows=" + std::to_string(rows) +
                " captured=" + std::to_string(rc.vals.size()) +
                " len=" + (rc.vals.empty() ? "0" : std::to_string(rc.vals[0][0].size())) +
                " payload=" + std::to_string(payload_of(n)) +
                " code=" + std::to_string(c->error_code) +
                " avail=" + std::to_string(avail_bytes(c)) +
                " isclose=" + std::to_string((int)c->isclose) +
                " err=" + c->error_msg.substr(0, 60));
    expect(v == want_of(n), "J1_sync_text_row_" + tag + "_no_silent_corruption",
           "verdict=" + v + " want=" + want_of(n));

    // J-2：不 hard_close，同一条连接接着读 ⇒ 结果集必须已排空到终结包
    long long s42 = query_int(c, "SELECT 42");
    reading("sync_row_" + tag + "_sentinel42",
            std::to_string(s42) + " avail=" + std::to_string(avail_bytes(c)) +
                " err=" + c->error_msg.substr(0, 60));
    expect(s42 == 42, "J2_sync_text_row_" + tag + "_stream_realigned", "sentinel=" + std::to_string(s42));

    // J-2b：原样回池再借出 ⇒ 池里不得留脏连接
    pool->back_mysql_select_conn(std::move(c));
    auto c2 = pool->get_mysql_select_conn();
    long long s43 = query_int(c2, "SELECT 43");
    reading("sync_row_" + tag + "_pool_reissue43",
            std::to_string(s43) + " isclose=" + std::to_string((int)c2->isclose));
    expect(s43 == 43, "J2_sync_text_row_" + tag + "_pool_reissue_clean", "sentinel=" + std::to_string(s43));
    c2->hard_close();
    pool->back_mysql_select_conn(std::move(c2));
}

// ---------- 异步大行臂 ----------
static asio::awaitable<void> async_big_row_arm(pool_t pool, size_t n)
{
    std::string tag = std::to_string(n);
    g_arm = "async_text_row_" + tag;
    conn_t c = co_await pool->async_get_mysql_select_conn();
    if (!c)
    {
        expect(false, "async_borrow_" + tag);
        co_return;
    }
    long long pre = co_await aquery_int(c, "SELECT 1");
    row_capture rc;
    unsigned int rows = co_await c->async_fetch_directly("SELECT REPEAT('a', " + tag + ")",
                                                         std::ref(rc));
    std::string v = verdict_of(rows, rc, n, c->error_code, c);
    reading("async_row_" + tag,
            "verdict=" + v + " want=" + want_of(n) + " rows=" + std::to_string(rows) +
                " captured=" + std::to_string(rc.vals.size()) +
                " len=" + (rc.vals.empty() ? "0" : std::to_string(rc.vals[0][0].size())) +
                " pre=" + std::to_string(pre) +
                " code=" + std::to_string(c->error_code) +
                " avail=" + std::to_string(avail_bytes(c)) +
                " err=" + c->error_msg.substr(0, 60));
    expect(v == want_of(n), "J1_async_text_row_" + tag + "_no_silent_corruption",
           "verdict=" + v + " want=" + want_of(n));

    long long s42 = co_await aquery_int(c, "SELECT 42");
    reading("async_row_" + tag + "_sentinel42",
            std::to_string(s42) + " avail=" + std::to_string(avail_bytes(c)));
    expect(s42 == 42, "J2_async_text_row_" + tag + "_stream_realigned", "sentinel=" + std::to_string(s42));

    pool->back_mysql_select_conn(std::move(c));
    conn_t c2 = co_await pool->async_get_mysql_select_conn();
    long long s43 = co_await aquery_int(c2, "SELECT 43");
    expect(s43 == 43, "J2_async_text_row_" + tag + "_pool_reissue_clean", "sentinel=" + std::to_string(s43));
    c2->hard_close();
    pool->back_mysql_select_conn(std::move(c2));
    co_return;
}

int main()
{
    setvbuf(stdout, nullptr, _IONBF, 0);

    // 看门狗：大行没排空就是无期限阻塞，这里换成有期限的红
    std::atomic<bool> done{false};
    std::thread wd(
        [&done]()
        {
            for (int i = 0; i < 900; ++i)
            {
                if (done.load())
                    return;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            std::printf("WATCHDOG hung arm=%s\n", g_arm.c_str());
            std::fflush(stdout);
            std::_Exit(99);
        });

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
    reading("env_max_packet", std::to_string(query_int(maint_sel, "SELECT @@max_allowed_packet")));
    reading("env_autocommit", std::to_string(query_int(maint_sel, "SELECT @@session.autocommit")));
    maint->exec_dml("DROP TABLE IF EXISTS _mysql_probe_d1");
    maint->exec_dml("CREATE TABLE _mysql_probe_d1 (id INT PRIMARY KEY, txt LONGTEXT) ENGINE=InnoDB");

    // ---------- 列定义包上界实测（D1_ONLY_ALIAS=1 单跑）：给"COLUMNS 相位不会有满块包"取证 ----------
    const bool only_alias = (std::getenv("D1_ONLY_ALIAS") != nullptr);
    if (only_alias)
    {
        std::printf("READ arms=alias_only\n");
        // 客户端 send 上限：sql.length()+1 >= 0xFFFFFF 即拒发（write_sql :2521），
        // 所以最大可发送语句 = 0xFFFFFF-2 = 16777213 字符。列名回显长度决定
        // Column Definition 包能否达到满块（0xFFFFFF），必须实测而非推断。
        for (size_t alen : {size_t(64), size_t(65535), size_t(1000000), size_t(16000000),
                            size_t(16777213), size_t(16777214)})
        {
            g_arm  = "alias_" + std::to_string(alen);
            auto c = pool->get_mysql_select_conn();
            size_t name_len = 0;
            size_t vlen     = 0;
            unsigned int rows = c->fetch_directly(
                "SELECT 1 AS `" + std::string(alen, 'a') + "`",
                [&name_len, &vlen](int, char **names, auto get) -> bool
                {
                    if (names && name_len == 0)
                        name_len = std::string(names[0]).size();
                    auto [p, l] = get(0);
                    if (p)
                        vlen = l;
                    return true;
                });
            size_t after = avail_bytes(c);
            long long s  = query_int(c, "SELECT 42");
            reading("alias_" + std::to_string(alen),
                    "rows=" + std::to_string(rows) + " name_len=" + std::to_string(name_len) +
                        " vlen=" + std::to_string(vlen) + " code=" + std::to_string(c->error_code) +
                        " avail=" + std::to_string(after) + " sentinel=" + std::to_string(s) +
                        " isclose=" + std::to_string(c->isclose ? 1 : 0) +
                        " err=" + c->error_msg.substr(0, 70));
            // 判据一：两种结局都不许残留字节（发得出=读干净；拒发=没发过字节）
            expect(after == 0, "ALIAS1_stream_intact_" + std::to_string(alen),
                   "avail=" + std::to_string(after));
            // 判据二：读成功必须回显整名列（实测服务端把列名回显截到 256 字节）且交付 1 行；拒发必须显式 code=20
            size_t want_name = (alen < 256 ? alen : 256);
            bool ok_read   = (rows == 1 && name_len == want_name && vlen == 1 && c->error_code == 0);
            bool ok_refused =
                (rows == 0 && c->error_code == 20 &&
                 c->error_msg.find("SQL too large") != std::string::npos);
            expect(ok_read || ok_refused, "ALIAS2_verdict_" + std::to_string(alen),
                   "read=" + std::to_string(ok_read) + " refused=" + std::to_string(ok_refused));
            // 判据三：连接状态与结论一致（拒发那条按 write_sql 的设计置 isclose）
            expect(ok_read ? (s == 42 && !c->isclose) : (c->isclose || s == 42),
                   "ALIAS3_conn_state_" + std::to_string(alen),
                   "sentinel=" + std::to_string(s) + " isclose=" + std::to_string(c->isclose ? 1 : 0));
            pool->back_mysql_select_conn(std::move(c));
        }
    }

    // ---------- J-1/J-2 同步文本：固定尺寸表 ----------
    // D1_SKIP_SYNC=1 时只跑异步族（突变臂在同步族第一个挂死点就 _Exit，异步判据需要单独一趟才有牙）
    const bool skip_sync = (std::getenv("D1_SKIP_SYNC") != nullptr) || only_alias;
    if (skip_sync && !only_alias)
        std::printf("READ arms=async_only\n");
    if (!skip_sync)
    {
    for (size_t n : {size_t(16000000), size_t(16777206), size_t(16777210), size_t(16777211),
                     size_t(16777212), size_t(16777215), size_t(16777216), size_t(17000000)})
    {
        sync_big_row_arm(pool, n);
    }

    // ---------- J-1 边界代价格：两列合计恰好 0xFFFFFF ----------
    {
        g_arm = "sync_text_two_col_exact_fit";
        auto c = pool->get_mysql_select_conn();
        query_int(c, "SELECT 1");
        row_capture rc;
        unsigned int rows = c->fetch_directly(
            "SELECT REPEAT('a', 8388603), REPEAT('b', 8388604)", std::ref(rc));
        bool explicit_err = (rows == 0 && c->error_code == 20);
        reading("sync_two_col_exact_fit",
                "rows=" + std::to_string(rows) + " captured=" + std::to_string(rc.vals.size()) +
                    " code=" + std::to_string(c->error_code) +
                    " avail=" + std::to_string(avail_bytes(c)) +
                    " err=" + c->error_msg.substr(0, 60));
        expect(explicit_err, "J1b_two_col_payload_exact_0xFFFFFF_is_explicit_error");
        long long s45 = query_int(c, "SELECT 45");
        expect(s45 == 45, "J2_two_col_exact_fit_stream_realigned", "sentinel=" + std::to_string(s45));
        pool->back_mysql_select_conn(std::move(c));
    }

    // ---------- J-3 大行之前的行必须照常交付，且不重投 ----------
    {
        g_arm = "sync_text_rows_before_big";
        auto c = pool->get_mysql_select_conn();
        query_int(c, "SELECT 1");
        row_capture rc;
        unsigned int rows = c->fetch_directly(
            "SELECT REPEAT('a', 10) UNION ALL SELECT REPEAT('a', 17000000) UNION ALL SELECT REPEAT('a', 11)",
            std::ref(rc));
        size_t first_len = rc.vals.empty() ? 0 : rc.vals[0][0].size();
        reading("J3_rows_before_big",
                "rows=" + std::to_string(rows) + " captured=" + std::to_string(rc.vals.size()) +
                    " first_len=" + std::to_string(first_len) +
                    " code=" + std::to_string(c->error_code) +
                    " avail=" + std::to_string(avail_bytes(c)) +
                    " err=" + c->error_msg.substr(0, 60));
        expect(rows == 1 && first_len == 10 && c->error_code == 20,
               "J3_rows_delivered_before_error",
               "rows=" + std::to_string(rows) + " first_len=" + std::to_string(first_len));
        long long s46 = query_int(c, "SELECT 46");
        expect(s46 == 46, "J3_stream_realigned_after_partial_result", "sentinel=" + std::to_string(s46));
        pool->back_mysql_select_conn(std::move(c));
    }

    // ---------- J-5 D3：预编译超限参数必须置 error_code=20，且连接继续可用 ----------
    {
        g_arm = "prepared_17mb_param_code";
        std::string bigparam(17000000, 'a');
        std::vector<http::obj_val> p = {http::obj_val(1), http::obj_val(bigparam)};
        unsigned int n = maint->exec_dml_prepared("INSERT INTO _mysql_probe_d1 (id, txt) VALUES (?, ?)", p);
        reading("J5_prepared_17mb_param_code",
                std::to_string(n) + "/" + std::to_string(maint->error_code));
        reading("J5_prepared_17mb_param_error", maint->error_msg.substr(0, 80));
        expect(n == static_cast<unsigned int>(-1) && maint->error_code == 20 &&
                   maint->error_msg.find("payload too large") != std::string::npos &&
                   !maint->is_closed(),
               "J5_prepared_over_limit_sets_error_code_keep_conn");
        expect(query_int(maint, "SELECT 47") == 47, "J5_prepared_over_limit_recover");
    }

    // ---------- 存量零影响（J-4 轻版）：普通读写各若干轮 ----------
    {
        g_arm = "normal_traffic";
        auto c = pool->get_mysql_select_conn();
        unsigned int okrows = 0;
        for (int i = 0; i < 200; ++i)
        {
            row_capture rc;
            okrows += c->fetch_directly("SELECT REPEAT('a', " + std::to_string(1000 + i) + ")", std::ref(rc));
        }
        long long s48 = query_int(c, "SELECT 48");
        reading("J4_normal_rows", std::to_string(okrows) + " sentinel=" + std::to_string(s48) +
                                       " code=" + std::to_string(c->error_code));
        expect(okrows == 200 && s48 == 48 && c->error_code == 0, "J4_normal_traffic_untouched");
        pool->back_mysql_select_conn(std::move(c));
    }
    } // !skip_sync

    // ---------- 异步文本大行（§十 第 1 条补的臂） ----------
    if (!only_alias)
    {
    for (size_t n : {size_t(16777210), size_t(16777216), size_t(17000000)})
    {
        auto fut = asio::co_spawn(ioc, async_big_row_arm(pool, n), asio::use_future);
        fut.get();
    }

    // ---------- 异步预编译超限参数 ----------
    {
        g_arm = "async_prepared_17mb_param";
        auto fut = asio::co_spawn(
            ioc,
            [pool]() -> asio::awaitable<void>
            {
                conn_t c = co_await pool->async_get_mysql_edit_conn();
                std::string bigparam(17000000, 'a');
                std::vector<http::obj_val> p = {http::obj_val(2), http::obj_val(bigparam)};
                unsigned int n = co_await c->async_exec_dml_prepared(
                    "INSERT INTO _mysql_probe_d1 (id, txt) VALUES (?, ?)", p);
                reading("J5_async_prepared_code", std::to_string(n) + "/" + std::to_string(c->error_code));
                expect(n == static_cast<unsigned int>(-1) && c->error_code == 20,
                       "J5_async_prepared_over_limit_sets_error_code");
                long long s49 = co_await aquery_int(c, "SELECT 49");
                expect(s49 == 49, "J5_async_prepared_over_limit_recover", "sentinel=" + std::to_string(s49));
                pool->back_mysql_edit_conn(std::move(c));
                co_return;
            },
            asio::use_future);
        fut.get();
    }
    } // !only_alias

    maint->exec_dml("DROP TABLE IF EXISTS _mysql_probe_d1");
    pool->back_mysql_edit_conn(std::move(maint));
    pool->back_mysql_select_conn(std::move(maint_sel));

    pool.reset();
    ioc.stop();
    io_thread.join();
    orm::init_orm_conn_pool_release();

    done.store(true);
    wd.join();

    std::printf("SUMMARY pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
