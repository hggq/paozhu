/*
 * test_mysql_stress.cpp — MySQL 压力测试
 * Sync × Dirty/Prepared × INSERT/SELECT/UPDATE/DELETE
 * 输出: docx/mysql_stress.csv + docx/mysql_stress_report.md
 */
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>
#include <cstdio>
#include <mutex>
#include <asio.hpp>
#include "orm_conn_pool.h"
#include "mysql_conn.h"
#include "request.h"

using clk = std::chrono::high_resolution_clock;

// 出错时 exec_dml/exec_dml_prepared 在三个后端都返回 (unsigned int)-1，
// 所以 "> 0" 会把错误当成成功；fetch_directly 出错与 0 行都返回 0，
// 只能靠"期望恰好 1 行"来判定读到了数据。
struct ErrSink {
    std::mutex mu;
    std::string first;
    void set(std::string msg) {
        std::lock_guard<std::mutex> l(mu);
        if (first.empty()) first = std::move(msg);
    }
    void reset() { std::lock_guard<std::mutex> l(mu); first.clear(); }
    std::string get() { std::lock_guard<std::mutex> l(mu); return first; }
};
static ErrSink g_err;

template <typename Conn>
static inline bool ok_dml(unsigned int r, const Conn &c, const char *ctx) {
    if (r == static_cast<unsigned int>(-1)) { g_err.set(std::string(ctx) + ": " + c->error_msg); return false; }
    if (r == 0) { g_err.set(std::string(ctx) + ": 0 rows affected"); return false; }
    return true;
}

static inline bool ok_row(unsigned int n, const char *ctx) {
    if (n != 1) { g_err.set(std::string(ctx) + ": " + std::to_string(n) + " rows (0 = error or no match)"); return false; }
    return true;
}

struct BenchStat {
    std::string name;
    int         iters = 0;
    double      total_ms = 0;
    double      avg_us = 0, p50_us = 0, p99_us = 0, throughput = 0;
    int         errors = 0;
    std::vector<double> samples;
    std::string first_error;
    void finalize() {
        if (samples.empty()) return;
        std::sort(samples.begin(), samples.end());
        avg_us  = total_ms * 1000.0 / iters;
        p50_us  = samples[samples.size() * 50 / 100];
        p99_us  = samples[samples.size() * 99 / 100];
        throughput = iters / (total_ms / 1000.0);
    }
};

template <typename Fn>
static BenchStat bench(const std::string &name, int iters, Fn &&fn) {
    BenchStat s{name, iters, {}, {}, {}, {}, {}, 0, {}, {}};
    s.samples.reserve(iters);
    g_err.reset();
    auto t0 = clk::now();
    for (int i = 0; i < iters; ++i) {
        auto a = clk::now();
        bool ok = fn(i);
        auto b = clk::now();
        s.samples.push_back(std::chrono::duration<double, std::micro>(b - a).count());
        if (!ok) s.errors++;
    }
    s.total_ms = std::chrono::duration<double, std::milli>(clk::now() - t0).count();
    s.finalize();
    s.first_error = g_err.get();
    return s;
}

static void print_stat(const BenchStat &s) {
    std::printf("  %-38s iter=%6d  total=%8.1fms  avg=%8.0fus  p50=%7.0fus  p99=%7.0fus  %9.1f/s  err=%d\n",
        s.name.c_str(), s.iters, s.total_ms, s.avg_us, s.p50_us, s.p99_us, s.throughput, s.errors);
    if (!s.first_error.empty())
        std::printf("      |_ %s\n", s.first_error.c_str());
}

static std::string csv_q(const std::string &v) {
    std::string o = "\"";
    for (char ch : v) { if (ch == '"') o += "\"\""; else o += ch; }
    return o + "\"";
}

static void write_outputs(const std::vector<BenchStat> &stats, const std::string &tag) {
    std::string stem = "docx/" + tag + "_stress";
    std::ofstream csv(stem + ".csv");
    csv << "name,iterations,total_ms,avg_us,p50_us,p99_us,throughput,errors,first_error\n";
    for (auto &s : stats)
        csv << s.name << "," << s.iters << "," << s.total_ms << "," << s.avg_us << ","
            << s.p50_us << "," << s.p99_us << "," << s.throughput << "," << s.errors << ","
            << csv_q(s.first_error) << "\n";
    std::ofstream md(stem + "_report.md");
    md << "# " << tag << " Stress Test Report (Sync)\n\n";
    md << "_口径: 每个测项复用同一条连接，计时只覆盖语句执行本身，不含连接池 get/back。"
          "`errors` = 命中错误哨兵或影响 0 行的迭代数；SELECT 以恰好返回 1 行为成功。_\n\n";
    md << "| Name | Iter | Total(ms) | Avg(us) | P50(us) | P99(us) | Throughput | Errors |\n";
    md << "|------|------|-----------|---------|---------|---------|------------|--------|\n";
    for (auto &s : stats)
        md << "| " << s.name << " | " << s.iters << " | " << (int)s.total_ms
           << " | " << (int)s.avg_us << " | " << (int)s.p50_us << " | " << (int)s.p99_us
           << " | " << (int)s.throughput << " | " << s.errors << " |\n";

    bool any_err = false;
    for (auto &s : stats) if (!s.first_error.empty()) any_err = true;
    if (any_err) {
        md << "\n## Failures\n\n";
        for (auto &s : stats)
            if (!s.first_error.empty())
                md << "- `" << s.name << "` (" << s.errors << "/" << s.iters << "): " << s.first_error << "\n";
    }
    std::printf("\n[OK] %s.csv + %s_report.md written\n", stem.c_str(), stem.c_str());
}

int main(int argc, char **argv) {
    int iters = (argc > 1) ? std::atoi(argv[1]) : 3000;
    // argv[2] = conf 标签（默认 cms = MySQL 8 实例；本机 MariaDB 用 mb），argv[3] = 产物名前缀
    const std::string conf_tag = (argc > 2) ? argv[2] : "cms";
    const std::string label    = (argc > 3) ? argv[3] : "mysql";
    std::printf("=== MySQL-protocol Stress Test — %d iterations per bench (conf tag [%s]) ===\n\n",
        iters, conf_tag.c_str());

    asio::io_context ioc;
    orm::init_orm_conn_pool(ioc, "conf/orm.conf");
    auto pool = orm::get_orm_conn_pool_obj()[conf_tag];
    if (pool == nullptr)
    {
        std::printf("[FAIL] conf 标签 [%s] 不存在\n", conf_tag.c_str());
        return 1;
    }

    {   // 服务端版本：同一 tag 可能指向不同实例，报告必须自带出处
        auto c = pool->get_mysql_edit_conn();
        if (c == nullptr)
        {
            std::printf("[FAIL] [%s] 取不到 mysql 编辑连接（库/账号/服务不可用？）\n", conf_tag.c_str());
            return 1;
        }
        std::string ver;
        c->fetch_directly("SELECT VERSION()", [&ver](int, char **, auto get_data) mutable -> bool {
            auto [ptr, len] = get_data(0);
            if (ptr) ver.assign(reinterpret_cast<const char *>(ptr), len);
            return true;
        });
        std::printf("[server] %s\n\n", ver.c_str());
        pool->back_mysql_edit_conn(std::move(c));
    }

    // DDL
    {
        auto c = pool->get_mysql_edit_conn();
        c->exec_dml("DROP TABLE IF EXISTS _stress_mysql");
        c->exec_dml("CREATE TABLE _stress_mysql (id INT PRIMARY KEY AUTO_INCREMENT, val1 INT, val2 VARCHAR(128), INDEX idx_val1 (val1))");
        pool->back_mysql_edit_conn(std::move(c));
    }

    std::vector<BenchStat> stats;

    // 单连接复用
    auto edit = pool->get_mysql_edit_conn();
    auto sel  = pool->get_mysql_select_conn();

    stats.push_back(bench("mysql_SYNC_DIRTY_INSERT", iters, [&](int i) {
        return ok_dml(edit->exec_dml("INSERT INTO _stress_mysql (val1, val2) VALUES (" +
            std::to_string(i) + ", 'sync_" + std::to_string(i) + "')"), edit, "DIRTY_INSERT");
    }));
    stats.push_back(bench("mysql_SYNC_DIRTY_SELECT", iters, [&](int i) {
        return ok_row(sel->fetch_directly(
            "SELECT val1 FROM _stress_mysql WHERE id = " + std::to_string(i + 1),
            [](int, char**, auto) { return true; }), "DIRTY_SELECT");
    }));
    stats.push_back(bench("mysql_SYNC_DIRTY_UPDATE", iters, [&](int i) {
        return ok_dml(edit->exec_dml("UPDATE _stress_mysql SET val1 = " + std::to_string(i * 2) +
            " WHERE id = " + std::to_string(i + 1)), edit, "DIRTY_UPDATE");
    }));
    stats.push_back(bench("mysql_SYNC_DIRTY_DELETE", iters, [&](int i) {
        return ok_dml(edit->exec_dml("DELETE FROM _stress_mysql WHERE id = " +
            std::to_string(i + 1)), edit, "DIRTY_DELETE");
    }));

    // Prepared
    // 显式写入 id: 上一条 DELETE 不会重置 AUTO_INCREMENT，若让 id 继续前移，
    // 下面 UPDATE/DELETE 的 WHERE id = i+1 会一行都命中不了，测的就不是目标通路。
    edit->exec_dml("DELETE FROM _stress_mysql");
    for (int i = 0; i < iters; ++i)
        edit->exec_dml("INSERT INTO _stress_mysql (id, val1, val2) VALUES (" +
            std::to_string(i + 1) + ", " + std::to_string(i) + ", 'prep_" + std::to_string(i) + "')");

    stats.push_back(bench("mysql_SYNC_PREPARED_INSERT", iters, [&](int i) {
        // 数值列必须传数值类型的 obj_val。MySQL 二进制协议按 prepare 时服务端报的列类型决定
        // 值段编码，数值分支读 obj_val 的 union 成员；std::string 构造的 obj_val 不碰 union，
        // 于是绑进未初始化值。
        std::vector<http::obj_val> params = {http::obj_val(i + 100000), "sync_prep_" + std::to_string(i)};
        return ok_dml(edit->exec_dml_prepared("INSERT INTO _stress_mysql (val1, val2) VALUES (?, ?)", params),
            edit, "PREPARED_INSERT");
    }));
    // affected==1 只能证明语句执行了，证明不了参数真的落库，回读一条做闭环。
    {
        long long probe_val = iters - 1 + 100000;
        long long got = -1;
        sel->fetch_directly("SELECT val1 FROM _stress_mysql WHERE val2 = 'sync_prep_" +
            std::to_string(iters - 1) + "'",
            [&got](int, char **, auto get_data) mutable -> bool {
                auto [ptr, len] = get_data(0);
                if (ptr) got = std::stoll(std::string(reinterpret_cast<const char *>(ptr), len));
                return true;
            });
        if (got != probe_val) {
            g_err.set("PREPARED_INSERT readback: val1=" + std::to_string(got) +
                      " expected " + std::to_string(probe_val));
            stats.back().errors++;
        }
    }
    stats.push_back(bench("mysql_SYNC_PREPARED_UPDATE", iters, [&](int i) {
        std::vector<http::obj_val> params = {http::obj_val(i * 3), http::obj_val(i + 1)};
        return ok_dml(edit->exec_dml_prepared("UPDATE _stress_mysql SET val1 = ? WHERE id = ?", params),
            edit, "PREPARED_UPDATE");
    }));
    stats.push_back(bench("mysql_SYNC_PREPARED_DELETE", iters, [&](int i) {
        std::vector<http::obj_val> params = {http::obj_val(i + 1)};
        return ok_dml(edit->exec_dml_prepared("DELETE FROM _stress_mysql WHERE id = ?", params),
            edit, "PREPARED_DELETE");
    }));

    pool->back_mysql_edit_conn(std::move(edit));
    pool->back_mysql_select_conn(std::move(sel));

    // cleanup
    {
        auto c = pool->get_mysql_edit_conn();
        c->exec_dml("DROP TABLE _stress_mysql");
        pool->back_mysql_edit_conn(std::move(c));
    }

    std::printf("\n=== MySQL Stress Result (%d iters) ===\n", iters);
    long long total_errors = 0;
    for (auto &s : stats) { print_stat(s); total_errors += s.errors; }

    write_outputs(stats, label);
    orm::init_orm_conn_pool_release();
    std::printf("[TOTAL] errors=%lld / %lld ops\n", total_errors, (long long)stats.size() * iters);
    return total_errors == 0 ? 0 : 1;
}
