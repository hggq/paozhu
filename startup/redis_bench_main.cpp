/*
 * pzredis 性能 benchmark（纯 direct connection，无 httpserver 依赖）
 *
 * 用法: cd build && ./redis_bench [-n count] [-m mode] [-h host] [-p port]
 *
 * 编译命令（paozhu 根目录下执行）:
 *   clang++ -std=c++20 -O2 -DENABLE_REDIS \
 *     -I../vendor/httpserver/include \
 *     -I../vendor/pzredis/include \
 *     -I.. \
 *     ../startup/redis_bench_main.cpp \
 *     ../startup/bench_stub.cpp \
 *     ../vendor/pzredis/src/*.cpp \
 *     ../vendor/httpserver/src/request.cpp \
 *     ../vendor/httpserver/src/parse_ini.cpp \
 *     -o redis_bench -lpthread -lssl -lcrypto
 *
 * 注: httpserver 只用到 request.cpp（obj_val 实现）+ parse_ini.cpp（INI 解析）
 *     + bench_stub.cpp（str2uint64_strict stub）——这三个是 pzredis 的硬依赖。
 *     -DPZREDIS_STANDALONE 未来可去掉这层依赖（pzredis 自身改造）。
 */

#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#include "redis_conn.h"

using namespace std::chrono;

void print_stats(const char *label, const std::vector<double> &latencies)
{
    if (latencies.empty()) return;
    std::vector<double> sorted = latencies;
    std::sort(sorted.begin(), sorted.end());
    double sum = 0;
    for (auto v : sorted) sum += v;
    double avg = sum / sorted.size();
    size_t p95 = std::min(sorted.size() - 1, static_cast<size_t>(sorted.size() * 0.95));
    size_t p99 = std::min(sorted.size() - 1, static_cast<size_t>(sorted.size() * 0.99));

    printf("%-30s count=%zu  avg=%.1fus  p50=%.1fus  p95=%.1fus  p99=%.1fus  max=%.1fus\n",
           label, sorted.size(), avg, sorted[sorted.size() / 2],
           sorted[p95], sorted[p99], sorted.back());
}

void run_direct_bench(pz::redis::conn_config_t &cfg, int n)
{
    asio::io_context ioc;
    pz::redis::redis_conn_base conn(ioc);
    cfg.timeout_sec = 30;

    if (!conn.connect(cfg))
    {
        printf("ERROR: direct connect failed: %s\n", conn.last_error_msg().c_str());
        return;
    }
    printf("[1] Direct sync connection (single connection)\n");

    // SET
    std::vector<double> set_lat;
    auto t1 = steady_clock::now();
    for (int i = 0; i < n; ++i)
    {
        auto st = steady_clock::now();
        conn.command({"SET", "bench_direct_" + std::to_string(i), "value_1234567890"});
        auto en = steady_clock::now();
        set_lat.push_back(duration<double, std::micro>(en - st).count());
    }
    auto t2 = steady_clock::now();
    long long set_ms = duration_cast<milliseconds>(t2 - t1).count();
    printf("    SET: %lld ms  %.0f ops/sec\n", set_ms, n * 1000.0 / set_ms);
    print_stats("    SET latency", set_lat);

    // GET
    std::vector<double> get_lat;
    auto t3 = steady_clock::now();
    for (int i = 0; i < n; ++i)
    {
        auto st = steady_clock::now();
        conn.command({"GET", "bench_direct_" + std::to_string(i)});
        auto en = steady_clock::now();
        get_lat.push_back(duration<double, std::micro>(en - st).count());
    }
    auto t4 = steady_clock::now();
    long long get_ms = duration_cast<milliseconds>(t4 - t3).count();
    printf("    GET: %lld ms  %.0f ops/sec\n", get_ms, n * 1000.0 / get_ms);
    print_stats("    GET latency", get_lat);

    conn.close();
}

void run_concurrent_bench(pz::redis::conn_config_t &cfg, int n, int threads)
{
    // 多线程多连接并发 command（模拟多 client 同时发）
    printf("[2] Concurrent: %d threads × %d ops/thread\n", threads, n);

    std::atomic<long long> total_set_ops{0};
    std::atomic<long long> total_get_ops{0};
    std::atomic<long long> total_set_ns{0};
    std::atomic<long long> total_get_ns{0};

    std::vector<std::thread> workers;
    auto t0 = steady_clock::now();

    for (int t = 0; t < threads; ++t)
    {
        workers.emplace_back([&cfg, n, t, &total_set_ops, &total_get_ops,
                              &total_set_ns, &total_get_ns] {
            asio::io_context ioc;
            pz::redis::redis_conn_base conn(ioc);
            auto my_cfg = cfg;
            my_cfg.timeout_sec = 30;
            if (!conn.connect(my_cfg)) return;

            for (int i = 0; i < n; ++i)
            {
                auto st = steady_clock::now();
                conn.command({"SET", "bench_conc_" + std::to_string(t) + "_" + std::to_string(i),
                              "value_1234567890"});
                auto en = steady_clock::now();
                total_set_ns += duration_cast<nanoseconds>(en - st).count();
                total_set_ops++;
            }
            for (int i = 0; i < n; ++i)
            {
                auto st = steady_clock::now();
                conn.command({"GET", "bench_conc_" + std::to_string(t) + "_" + std::to_string(i)});
                auto en = steady_clock::now();
                total_get_ns += duration_cast<nanoseconds>(en - st).count();
                total_get_ops++;
            }
            conn.close();
        });
    }
    for (auto &w : workers) w.join();
    auto tend = steady_clock::now();
    long long wall_ms = duration_cast<milliseconds>(tend - t0).count();

    double set_ops_per_sec = total_set_ops * 1000.0 / wall_ms;
    double get_ops_per_sec = total_get_ops * 1000.0 / wall_ms;
    double set_avg_us = total_set_ops > 0 ? total_set_ns / 1000.0 / total_set_ops : 0;
    double get_avg_us = total_get_ops > 0 ? total_get_ns / 1000.0 / total_get_ops : 0;

    printf("    SET wall: %lld ms  %.0f ops/sec  avg=%.1fus\n",
           wall_ms, set_ops_per_sec, set_avg_us);
    printf("    GET wall: %lld ms  %.0f ops/sec  avg=%.1fus\n",
           wall_ms, get_ops_per_sec, get_avg_us);
}

int main(int argc, char *argv[])
{
    int n = 1000;
    int threads = 8;
    std::string mode = "all";
    std::string host = "127.0.0.1";
    unsigned short port = 6379;
    std::string username, password;
    unsigned int dbindex = 0;

    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        if (a == "-n" && i + 1 < argc) n = std::atoi(argv[++i]);
        else if (a == "-t" && i + 1 < argc) threads = std::atoi(argv[++i]);
        else if (a == "-m" && i + 1 < argc) mode = argv[++i];
        else if (a == "-h" && i + 1 < argc) host = argv[++i];
        else if (a == "-p" && i + 1 < argc) port = (unsigned short)std::atoi(argv[++i]);
        else if (a == "-u" && i + 1 < argc) username = argv[++i];
        else if (a == "-w" && i + 1 < argc) password = argv[++i];
        else if (a == "-d" && i + 1 < argc) dbindex = (unsigned int)std::atoi(argv[++i]);
        else if (a == "--help" || a == "-?")
        {
            printf("Usage: %s [-n count] [-t threads] [-m mode] [-h host] [-p port] [-u user] [-w pwd] [-d db]\n"
                   "  mode=direct|concurrent|all (default: all)\n",
                   argv[0]);
            return 0;
        }
    }

    pz::redis::conn_config_t cfg;
    cfg.host = host;
    cfg.port = port;
    cfg.username = username;
    cfg.password = password;
    cfg.dbindex = dbindex;

    printf("=== pzredis Benchmark: n=%d  threads=%d  mode=%s  %s:%u  db=%u ===\n\n",
           n, threads, mode.c_str(), host.c_str(), port, dbindex);

    if (mode == "direct" || mode == "all")
    {
        run_direct_bench(cfg, n);
    }
    if (mode == "concurrent" || mode == "all")
    {
        printf("\n");
        run_concurrent_bench(cfg, n, threads);
    }

    printf("\nDone.\n");
    return 0;
}
