/*
 * test_mysql_stress_mt.cpp — MySQL 多线程并发连接池压力测试
 * 线程数 × 每线程迭代数, 每轮从连接池 get/back
 * 输出: docx/mysql_stress_mt.csv + docx/mysql_stress_mt_report.md
 */
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>
#include <cstdio>
#include <set>
#include <map>
#include <mutex>
#include <asio.hpp>
#include "orm_conn_pool.h"
#include "mysql_conn.h"
#include "request.h"

using clk = std::chrono::high_resolution_clock;

// 出错时 exec_dml 返回 (unsigned int)-1，"> 0" 会把错误当成成功；
// fetch_directly 出错与 0 行都返回 0，只能靠"期望恰好 1 行"判定读到了数据。
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

struct ThreadStat {
    int64_t ops = 0;
    int64_t errors = 0;
    std::vector<double> samples;
};

struct BenchStat {
    std::string name;
    int threads = 0;
    int64_t total_ops = 0;
    int64_t total_errors = 0;
    double total_ms = 0;
    double avg_us = 0, p50_us = 0, p99_us = 0;
    double throughput = 0;
    std::string first_error;
};

template <typename Fn>
static BenchStat concurrent_bench(const std::string &name, int threads, int iters_per_thread, Fn &&fn) {
    BenchStat s{name, threads, 0, 0, 0, 0, 0, 0, 0, {}};
    std::vector<ThreadStat> tstats(threads);
    std::atomic<int> start_flag{0};
    std::vector<std::thread> workers;

    g_err.reset();
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&, t]() {
            while (start_flag.load() == 0) std::this_thread::yield();
            for (int i = 0; i < iters_per_thread; ++i) {
                auto a = clk::now();
                bool ok = fn(t, i);
                auto b = clk::now();
                tstats[t].samples.push_back(
                    std::chrono::duration<double, std::micro>(b - a).count());
                tstats[t].ops++;
                if (!ok) tstats[t].errors++;
            }
        });
    }

    auto run_start = clk::now();
    start_flag.store(1);
    for (auto &w : workers) w.join();
    auto run_end = clk::now();
    s.total_ms = std::chrono::duration<double, std::milli>(run_end - run_start).count();

    std::vector<double> all;
    for (auto &ts : tstats) {
        s.total_ops    += ts.ops;
        s.total_errors += ts.errors;
        all.insert(all.end(), ts.samples.begin(), ts.samples.end());
    }
    std::sort(all.begin(), all.end());
    if (!all.empty()) {
        s.avg_us = s.total_ms * 1000.0 / s.total_ops;
        s.p50_us = all[all.size() * 50 / 100];
        s.p99_us = all[all.size() * 99 / 100];
    }
    s.throughput = s.total_ops / (s.total_ms / 1000.0);
    s.first_error = g_err.get();
    return s;
}

static void print_stat(const BenchStat &s) {
    std::printf("  %-34s T=%2d  ops=%7lld  total=%7.0fms  avg=%6.0fus  p50=%6.0fus  p99=%6.0fus  %9.1f/s  err=%lld\n",
        s.name.c_str(), s.threads, (long long)s.total_ops, s.total_ms,
        s.avg_us, s.p50_us, s.p99_us, s.throughput, (long long)s.total_errors);
    if (!s.first_error.empty())
        std::printf("      |_ %s\n", s.first_error.c_str());
}

static std::string csv_q(const std::string &v) {
    std::string o = "\"";
    for (char ch : v) { if (ch == '"') o += "\"\""; else o += ch; }
    return o + "\"";
}

static void write_outputs(const std::vector<BenchStat> &stats, const std::string &tag) {
    std::string stem = "docx/" + tag + "_stress_mt";
    std::ofstream csv(stem + ".csv");
    csv << "name,threads,ops,total_ms,avg_us,p50_us,p99_us,throughput,errors,first_error\n";
    for (auto &s : stats)
        csv << s.name << "," << s.threads << "," << s.total_ops << "," << s.total_ms << ","
            << s.avg_us << "," << s.p50_us << "," << s.p99_us << ","
            << s.throughput << "," << s.total_errors << "," << csv_q(s.first_error) << "\n";

    std::ofstream md(stem + "_report.md");
    md << "# " << tag << " Multi-Thread Stress Test Report (Sync Connection Pool)\n\n";
    md << "_口径: 每个 op 都从连接池 get 再 back；池在无空闲连接时直接新建（不理会 max_pool），"
          "所以 T 对应真实并发连接数。_\n\n";
    md << "_注意: `avg_us` = total_ms/ops，即吞吐的倒数，不是平均延迟（并发下二者差异很大，请看 p50/p99）；"
          "每个 op 的计时包含连接池 get/back，与 sync 压测（复用同一条连接、只测执行）口径不同，两组数字不可直接比较。_\n\n";
    std::set<int> thread_counts;
    std::set<std::string> op_names;
    for (auto &s : stats) { thread_counts.insert(s.threads); op_names.insert(s.name); }

    md << "## Throughput (ops/s) by Thread Count\n\n| Op / Threads | ";
    for (int t : thread_counts) md << "T" << t << " | ";
    md << "\n|---|";
    for (size_t i = 0; i < thread_counts.size(); ++i) md << "---|";
    md << "\n";
    std::map<std::pair<std::string, int>, BenchStat> lookup;
    for (auto &s : stats) lookup[{s.name, s.threads}] = s;
    for (auto &op : op_names) {
        md << "| " << op << " |";
        for (int t : thread_counts) {
            auto it = lookup.find({op, t});
            md << (it != lookup.end() ? " " + std::to_string((int)it->second.throughput) + " |" : " - |");
        }
        md << "\n";
    }
    md << "\n## Detailed\n\n";
    md << "| Name | Threads | Ops | Total(ms) | Avg(us) | P50(us) | P99(us) | Throughput | Errors |\n";
    md << "|------|---------|-----|-----------|---------|---------|---------|------------|--------|\n";
    for (auto &s : stats)
        md << "| " << s.name << " | " << s.threads << " | " << s.total_ops
           << " | " << (int)s.total_ms << " | " << (int)s.avg_us
           << " | " << (int)s.p50_us << " | " << (int)s.p99_us
           << " | " << (int)s.throughput << " | " << s.total_errors << " |\n";

    bool any_err = false;
    for (auto &s : stats) if (!s.first_error.empty()) any_err = true;
    if (any_err) {
        md << "\n## Failures\n\n";
        for (auto &s : stats)
            if (!s.first_error.empty())
                md << "- `" << s.name << "` T" << s.threads << " (" << s.total_errors << "/" << s.total_ops
                   << "): " << s.first_error << "\n";
    }
    std::printf("\n[OK] %s.csv + %s_report.md written\n", stem.c_str(), stem.c_str());
}

int main(int argc, char **argv) {
    int iters_per_thread = (argc > 1) ? std::atoi(argv[1]) : 2000;
    // argv[2] = conf 标签（默认 cms = MySQL 8 实例；本机 MariaDB 用 mb），argv[3] = 产物名前缀
    const std::string conf_tag = (argc > 2) ? argv[2] : "cms";
    const std::string label    = (argc > 3) ? argv[3] : "mysql";
    std::vector<int> thread_counts = {1, 2, 4, 8, 16};
    std::printf("=== MySQL-protocol Multi-Thread Stress Test — %d iters/thread, conf tag [%s], threads:",
        iters_per_thread, conf_tag.c_str());
    for (int t : thread_counts) std::printf(" %d", t);
    std::printf("\n\n");

    asio::io_context ioc;
    orm::init_orm_conn_pool(ioc, "conf/orm.conf");
    auto pool = orm::get_orm_conn_pool_obj()[conf_tag];
    if (pool == nullptr)
    {
        std::printf("[FAIL] conf 标签 [%s] 不存在\n", conf_tag.c_str());
        return 1;
    }

    {
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
        c->exec_dml("DROP TABLE IF EXISTS _stress_mysql");
        c->exec_dml("CREATE TABLE _stress_mysql (id INT PRIMARY KEY AUTO_INCREMENT, val1 INT UNIQUE, val2 VARCHAR(128), UNIQUE KEY uk_val1 (val1))");
        pool->back_mysql_edit_conn(std::move(c));
    }

    std::vector<BenchStat> stats;

    for (int T : thread_counts) {
        std::printf("[T=%d] ", T);

        stats.push_back(concurrent_bench("mysql_INSERT_DIRTY", T, iters_per_thread, [&](int t, int i) {
            auto c = pool->get_mysql_edit_conn();
            bool ok = ok_dml(c->exec_dml("INSERT INTO _stress_mysql (val1, val2) VALUES (" +
                std::to_string(t * 1000000 + i) + ", 't" + std::to_string(t) + "_i" + std::to_string(i) + "')"),
                c, "INSERT");
            pool->back_mysql_edit_conn(std::move(c));
            return ok;
        }));
        print_stat(stats.back());

        stats.push_back(concurrent_bench("mysql_SELECT_DIRTY", T, iters_per_thread, [&](int t, int i) {
            auto c = pool->get_mysql_select_conn();
            bool ok = ok_row(c->fetch_directly(
                "SELECT val1 FROM _stress_mysql WHERE val1 = " + std::to_string(t * 1000000 + i),
                [](int, char**, auto) { return true; }), "SELECT");
            pool->back_mysql_select_conn(std::move(c));
            return ok;
        }));
        print_stat(stats.back());

        stats.push_back(concurrent_bench("mysql_UPDATE_DIRTY", T, iters_per_thread, [&](int t, int i) {
            auto c = pool->get_mysql_edit_conn();
            bool ok = ok_dml(c->exec_dml("UPDATE _stress_mysql SET val2 = \'upd\' WHERE val1 = " +
                std::to_string(t * 1000000 + i)), c, "UPDATE");
            pool->back_mysql_edit_conn(std::move(c));
            return ok;
        }));
        print_stat(stats.back());

        stats.push_back(concurrent_bench("mysql_DELETE_DIRTY", T, iters_per_thread, [&](int t, int i) {
            auto c = pool->get_mysql_edit_conn();
            bool ok = ok_dml(c->exec_dml("DELETE FROM _stress_mysql WHERE val1 = " +
                std::to_string(t * 1000000 + i)), c, "DELETE");
            pool->back_mysql_edit_conn(std::move(c));
            return ok;
        }));
        print_stat(stats.back());

        std::printf("\n");
    }

    {
        auto c = pool->get_mysql_edit_conn();
        c->exec_dml("DROP TABLE _stress_mysql");
        pool->back_mysql_edit_conn(std::move(c));
    }

    long long total_errors = 0, total_ops = 0;
    for (auto &s : stats) { total_errors += s.total_errors; total_ops += s.total_ops; }
    write_outputs(stats, label);
    orm::init_orm_conn_pool_release();
    std::printf("[TOTAL] errors=%lld / %lld ops\n", total_errors, total_ops);
    return total_errors == 0 ? 0 : 1;
}
