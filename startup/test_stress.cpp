/**
 * test_stress.cpp — TLS 1.3 项目 HTTPS 压力测试工具
 * test_stress.cpp — TLS 1.3 Project HTTPS Stress Testing Tool
 *
 * 用法 Usage:
 *   test_stress <num_threads> <loop_count> <ip:port> <hostname> [tls_version] [timeout_sec]
 *
 * 参数 Argument:
 *   num_threads  - 并发线程数
 *   loop_count   - 每个线程的请求次数 (总请求 = num_threads * loop_count)
 *   ip:port      - 目标地址, 如 127.0.0.1:443
 *   hostname     - 目标主机名, 支持 http:// 或 https:// 前缀
 *   tls_version  - 可选, 0=TLS1.3, 1=TLS1.2, 其他=auto (默认 0)
 *   timeout_sec  - 可选, 读写超时秒数 (默认 10)
 *
 * 环境变量 environment variable:
 *   QVERBOSE=1   - 开启逐包调试输出 (仅调试用, 压测时勿开)
 *
 * 单独编译 Compile separately:
 *   c++ -std=c++20 -O3 -o test_stress startup/test_stress.cpp -lssl -lcrypto -lpthread
 *
 * 示例 Example:
 *   ./test_stress 10 100 127.0.0.1:443 https://www.example.com
 *   ./test_stress 50 200 127.0.0.1:8443 www.example.com 1
 *   ./test_stress 20 500 127.0.0.1:80 http://www.example.com 2 5
 */

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <mutex>
#include <cmath>

// ========================== 全局统计 ==========================
static std::atomic<int64_t> g_success(0);       // 成功请求数
static std::atomic<int64_t> g_fail(0);          // 失败请求数
static std::atomic<int64_t> g_total_bytes(0);   // 读取的总字节数 (含 header)
static std::atomic<int64_t> g_connect_fail(0);  // TCP 连接失败
static std::atomic<int64_t> g_handshake_fail(0);// TLS 握手失败
static std::atomic<int64_t> g_write_fail(0);    // 写请求失败
static std::atomic<int64_t> g_read_fail(0);     // 读响应失败

// 延迟采样 (每线程本地收集, 结束后合并)
struct LatencyRecord
{
    std::vector<double> connect_ms;   // TCP 连接耗时
    std::vector<double> handshake_ms; // TLS 握手耗时
    std::vector<double> total_ms;     // 完整请求耗时 (连接→读完响应)
};

static std::mutex g_latency_mutex;
static std::vector<LatencyRecord> g_all_latencies;

// 配置
static unsigned char g_tls_type  = 0;  // 0=TLS1.3, 1=TLS1.2, 2=auto
static bool g_use_https          = true;
static bool g_verbose            = false;
static bool g_no_verify          = true;  // 默认跳过证书验证 (压测场景多为自签证书)
static int  g_timeout_sec        = 10;

// ========================== 工具函数 ==========================
static void set_bio_timeout(BIO *bio, int seconds)
{
    int fd = -1;
    BIO_get_fd(bio, &fd);
    if (fd != -1)
    {
        struct timeval tv;
        tv.tv_sec  = seconds;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }
}

static double now_ms()
{
    auto tp = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration<double, std::milli>(tp).count();
}

// ========================== 单次请求 ==========================
static void request_once(const std::string &ip_port, const std::string &hostname)
{
    LatencyRecord lat;
    double t_start = now_ms();

    // ---- TCP 连接 ----
    BIO *bio = BIO_new_connect(ip_port.c_str());
    if (!bio || BIO_do_connect(bio) <= 0)
    {
        if (g_verbose && bio)
        {
            std::cerr << "[FAIL] TCP connect to " << ip_port << " failed\n";
            ERR_print_errors_fp(stderr);
        }
        g_connect_fail.fetch_add(1, std::memory_order_relaxed);
        g_fail.fetch_add(1, std::memory_order_relaxed);
        if (bio) BIO_free(bio);
        return;
    }
    double t_connected = now_ms();
    lat.connect_ms.push_back(t_connected - t_start);

    set_bio_timeout(bio, g_timeout_sec);

    SSL *ssl    = nullptr;
    SSL_CTX *ctx = nullptr;

    if (g_use_https)
    {
        // ---- TLS 握手 ----
        double t_hs_start = now_ms();
        ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx)
        {
            if (g_verbose) ERR_print_errors_fp(stderr);
            g_handshake_fail.fetch_add(1, std::memory_order_relaxed);
            g_fail.fetch_add(1, std::memory_order_relaxed);
            BIO_free(bio);
            return;
        }

        // 跳过证书验证 (压测工具默认行为, 适配自签证书)
        if (g_no_verify)
            SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

        if (g_tls_type == 0)
        {
            SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
            SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
        }
        else if (g_tls_type == 1)
        {
            SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
            SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);
        }
        else
        {
            SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
            SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
        }

        ssl = SSL_new(ctx);
        SSL_set_connect_state(ssl);
        SSL_set_tlsext_host_name(ssl, hostname.c_str());
        SSL_set_bio(ssl, bio, bio);

        if (SSL_do_handshake(ssl) != 1)
        {
            if (g_verbose) ERR_print_errors_fp(stderr);
            g_handshake_fail.fetch_add(1, std::memory_order_relaxed);
            g_fail.fetch_add(1, std::memory_order_relaxed);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            return;
        }
        double t_hs_done = now_ms();
        lat.handshake_ms.push_back(t_hs_done - t_hs_start);
    }

    // ---- 发送 HTTP 请求 ----
    std::string req = "GET / HTTP/1.1\r\nHost: " + hostname +
                      "\r\nUser-Agent: tls13-stress-test/1.0\r\n"
                      "Connection: close\r\nAccept: */*\r\n\r\n";

    int write_ret;
    if (g_use_https)
        write_ret = SSL_write(ssl, req.c_str(), static_cast<int>(req.size()));
    else
        write_ret = BIO_write(bio, req.c_str(), static_cast<int>(req.size()));

    if (write_ret <= 0)
    {
        if (g_verbose) ERR_print_errors_fp(stderr);
        g_write_fail.fetch_add(1, std::memory_order_relaxed);
        g_fail.fetch_add(1, std::memory_order_relaxed);
        if (g_use_https) { SSL_free(ssl); SSL_CTX_free(ctx); }
        else { BIO_free(bio); }
        return;
    }

    // ---- 读取响应 (解析 Content-Length 或依赖连接关闭) ----
    char buf[8192];
    int n;
    std::string full_response;
    int content_length  = -1;
    size_t header_end   = std::string::npos;
    bool header_found   = false;
    size_t body_read    = 0;
    size_t header_len   = 0;
    int64_t total_read  = 0;

    while (true)
    {
        if (g_use_https)
            n = SSL_read(ssl, buf, sizeof(buf) - 1);
        else
            n = BIO_read(bio, buf, sizeof(buf) - 1);

        if (n <= 0)
        {
            if (n < 0 && g_verbose)
                ERR_print_errors_fp(stderr);
            break;
        }

        buf[n] = '\0';
        total_read += n;

        if (!header_found)
        {
            full_response.append(buf, n);
            header_end = full_response.find("\r\n\r\n");
            if (header_end != std::string::npos)
            {
                header_found = true;
                std::string headers = full_response.substr(0, header_end);

                // 解析 Content-Length
                size_t pos = headers.find("Content-Length:");
                if (pos == std::string::npos)
                    pos = headers.find("content-length:");
                if (pos != std::string::npos)
                {
                    size_t start = pos + 15;
                    while (start < headers.size() && headers[start] == ' ') start++;
                    size_t end = headers.find("\r\n", start);
                    if (end == std::string::npos) end = headers.size();
                    try { content_length = std::stoi(headers.substr(start, end - start)); }
                    catch (...) { content_length = -1; }
                }

                header_len = header_end + 4;
                if (full_response.size() > header_len)
                    body_read = full_response.size() - header_len;

                if (content_length >= 0 && body_read >= static_cast<size_t>(content_length))
                    break;
            }
        }
        else
        {
            body_read += n;
            if (content_length >= 0 && body_read >= static_cast<size_t>(content_length))
                break;
        }
    }

    double t_end = now_ms();
    lat.total_ms.push_back(t_end - t_start);

    // ---- 统计 ----
    g_total_bytes.fetch_add(total_read, std::memory_order_relaxed);

    if (header_found &&
        (content_length == 0 ||
         (content_length > 0 && body_read >= static_cast<size_t>(content_length))))
    {
        g_success.fetch_add(1, std::memory_order_relaxed);
    }
    else if (header_found)
    {
        // 头部解析成功但未读完全部 body (如 chunked 或无 Content-Length)
        // 只要读到了数据就算成功
        g_success.fetch_add(1, std::memory_order_relaxed);
    }
    else
    {
        g_read_fail.fetch_add(1, std::memory_order_relaxed);
        g_fail.fetch_add(1, std::memory_order_relaxed);
    }

    // ---- 清理 ----
    if (g_use_https)
    {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
    else
    {
        BIO_free(bio);
    }

    // 记录延迟 (加锁合并)
    {
        std::lock_guard<std::mutex> lk(g_latency_mutex);
        g_all_latencies.push_back(std::move(lat));
    }
}

// ========================== 线程运行 ==========================
static void thread_run(int num_threads, int loop_count,
                       const std::string &ip_port, const std::string &hostname)
{
    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < loop_count; ++j)
            {
                request_once(ip_port, hostname);
            }
        });
    }

    for (auto &t : threads)
        t.join();

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // ========================== 汇总报告 ==========================
    int64_t total_req  = g_success.load() + g_fail.load();
    int64_t success    = g_success.load();
    int64_t fail       = g_fail.load();
    double  elapsed_s  = elapsed.count();
    double  rps        = (elapsed_s > 0) ? total_req / elapsed_s : 0;
    double  success_rps = (elapsed_s > 0) ? success / elapsed_s : 0;

    // 合并所有延迟数据
    std::vector<double> all_connect, all_handshake, all_total;
    for (auto &rec : g_all_latencies)
    {
        all_connect.insert(all_connect.end(), rec.connect_ms.begin(), rec.connect_ms.end());
        all_handshake.insert(all_handshake.end(), rec.handshake_ms.begin(), rec.handshake_ms.end());
        all_total.insert(all_total.end(), rec.total_ms.begin(), rec.total_ms.end());
    }

    auto percentile = [](std::vector<double> &v, double p) -> double {
        if (v.empty()) return 0.0;
        std::sort(v.begin(), v.end());
        size_t idx = static_cast<size_t>(p * v.size());
        if (idx >= v.size()) idx = v.size() - 1;
        return v[idx];
    };

    double total_bytes_mb = g_total_bytes.load() / (1024.0 * 1024.0);

    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              TLS Stress Test Report                    ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Target:        " << std::left << std::setw(40) << (hostname.empty() ? ip_port : hostname) << " ║\n";
    std::cout << "║  TLS Version:   " << std::left << std::setw(40)
              << (g_tls_type == 0 ? "TLS 1.3" : g_tls_type == 1 ? "TLS 1.2" : "Auto (1.2~1.3)")
              << " ║\n";
    std::cout << "║  Mode:          " << std::left << std::setw(40) << (g_use_https ? "HTTPS" : "HTTP") << " ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Threads:       " << std::left << std::setw(40) << num_threads << " ║\n";
    std::cout << "║  Req/thread:    " << std::left << std::setw(40) << loop_count << " ║\n";
    std::cout << "║  Total requests:" << std::left << std::setw(39) << total_req << " ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Success:       " << std::left << std::setw(40) << success << " ║\n";
    std::cout << "║  Failed:        " << std::left << std::setw(40) << fail << " ║\n";
    std::cout << "║    - Connect:   " << std::left << std::setw(40) << g_connect_fail.load() << " ║\n";
    std::cout << "║    - Handshake: " << std::left << std::setw(40) << g_handshake_fail.load() << " ║\n";
    std::cout << "║    - Write:     " << std::left << std::setw(40) << g_write_fail.load() << " ║\n";
    std::cout << "║    - Read:      " << std::left << std::setw(40) << g_read_fail.load() << " ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "║  Total time:    " << std::left << std::setw(37) << (std::to_string(elapsed_s) + " s") << " ║\n";
    std::cout << "║  RPS (total):   " << std::left << std::setw(40) << std::to_string(static_cast<int>(rps)) << " ║\n";
    std::cout << "║  RPS (success): " << std::left << std::setw(40) << std::to_string(static_cast<int>(success_rps)) << " ║\n";
    std::cout << "║  Throughput:    " << std::left << std::setw(37) << (std::to_string(total_bytes_mb).substr(0, 8) + " MB") << " ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";

    if (!all_total.empty())
    {
        std::cout << "║  Latency (ms):  min / avg / p50 / p95 / p99 / max     ║\n";
        std::cout << "║  ──────────────────────────────────────────────────── ║\n";

        auto fmt_lat = [&](const std::string &label, std::vector<double> &v) {
            if (v.empty()) { std::cout << "║  " << std::left << std::setw(14) << label << "  N/A\n"; return; }
            std::sort(v.begin(), v.end());
            double mn  = v.front();
            double mx  = v.back();
            double avg = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
            double p50 = percentile(v, 0.50);
            double p95 = percentile(v, 0.95);
            double p99 = percentile(v, 0.99);
            std::cout << "║  " << std::left << std::setw(14) << label << " "
                      << std::right << std::fixed << std::setprecision(1)
                      << std::setw(6) << mn << " / "
                      << std::setw(6) << avg << " / "
                      << std::setw(6) << p50 << " / "
                      << std::setw(6) << p95 << " / "
                      << std::setw(6) << p99 << " / "
                      << std::setw(6) << mx << "\n";
        };

        fmt_lat("Connect", all_connect);
        fmt_lat("Handshake", all_handshake);
        fmt_lat("Total", all_total);
    }

    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;
}

// ========================== 参数解析 & main ==========================
static void print_usage(const char *prog)
{
    std::cerr << "Usage: " << prog
              << " <num_threads> <loop_count> <ip:port> <hostname> [tls_version] [timeout_sec]\n"
              << "\n"
              << "  num_threads  - 并发线程数\n"
              << "  loop_count   - 每线程请求次数\n"
              << "  ip:port      - 目标地址 (如 127.0.0.1:443)\n"
              << "  hostname     - 主机名 (支持 http:// 或 https:// 前缀)\n"
              << "  tls_version  - 可选: 0=TLS1.3(默认), 1=TLS1.2, 2=Auto\n"
              << "  timeout_sec  - 可选: 读写超时秒数 (默认 10)\n"
              << "  --verify     - 启用证书验证 (默认跳过)\n"
              << "\n"
              << "示例:\n"
              << "  " << prog << " 10 100 127.0.0.1:443 https://www.example.com\n"
              << "  " << prog << " 50 200 127.0.0.1:8443 www.example.com 1\n"
              << "  " << prog << " 20 500 127.0.0.1:80 http://www.example.com 2 5\n"
              << "\n"
              << "环境变量: QVERBOSE=1 开启逐包调试输出\n";
}

int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        print_usage(argv[0]);
        return 1;
    }

    int num_threads = std::stoi(argv[1]);
    int loop_count  = std::stoi(argv[2]);
    std::string ip_port  = argv[3];
    std::string hostname = argv[4];

    g_verbose = (std::getenv("QVERBOSE") != nullptr);

    // 解析 http:// 或 https:// 前缀
    if (hostname.rfind("https://", 0) == 0)
    {
        g_use_https = true;
        hostname = hostname.substr(8);
    }
    else if (hostname.rfind("http://", 0) == 0)
    {
        g_use_https = false;
        hostname = hostname.substr(7);
    }
    else
    {
        // 无前缀时根据端口判断
        size_t colon = ip_port.find_last_of(':');
        if (colon != std::string::npos)
        {
            int port = std::stoi(ip_port.substr(colon + 1));
            if (port == 80) g_use_https = false;
            else g_use_https = true;
        }
        else
        {
            g_use_https = true;
        }
    }

    // 解析可选参数
    for (int i = 5; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--verify")
        {
            g_no_verify = false;
        }
        else if (g_tls_type == 0 && i == 5 && arg != "--verify")
        {
            int tmp = std::stoi(arg);
            if (tmp < 0 || tmp > 2)
            {
                std::cerr << "Invalid tls_version, use 0 (TLS1.3), 1 (TLS1.2), or 2 (Auto)\n";
                return 1;
            }
            g_tls_type = static_cast<unsigned char>(tmp);
        }
        else if (arg != "--verify")
        {
            // 非 --verify 的参数视为 timeout
            g_timeout_sec = std::stoi(arg);
            if (g_timeout_sec <= 0)
            {
                std::cerr << "timeout_sec must be positive\n";
                return 1;
            }
        }
    }

    if (num_threads <= 0 || loop_count <= 0)
    {
        std::cerr << "Threads and loop_count must be positive integers.\n";
        return 1;
    }

    // OpenSSL 初始化
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    int64_t total_expected = static_cast<int64_t>(num_threads) * loop_count;
    std::cout << "TLS Stress Test starting...\n"
              << "  Threads: " << num_threads
              << ", Req/thread: " << loop_count
              << ", Total: " << total_expected
              << ", Target: " << hostname
              << " (" << ip_port << ")"
              << ", TLS: " << (g_tls_type == 0 ? "1.3" : g_tls_type == 1 ? "1.2" : "Auto")
              << ", Mode: " << (g_use_https ? "HTTPS" : "HTTP")
              << ", Timeout: " << g_timeout_sec << "s"
              << std::endl;

    thread_run(num_threads, loop_count, ip_port, hostname);

    return 0;
}
