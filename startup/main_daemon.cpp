#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <filesystem>
#include <system_error>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "server.h"
#include <asio/posix/stream_descriptor.hpp>
#include "init_daemon.hpp"
// paozhu-rbac:begin
#include "rbac/route_guard.h"
// paozhu-rbac:end

namespace fs = std::filesystem;

// Supervisor 重启机制：
// init_daemon 双 fork + setsid 后，当前进程是 daemon（即 supervisor）。
// 外层 while 每轮 fork 一个 worker 跑服务。
// worker 任何原因退出（崩溃/OOM）→ supervisor 收到 SIGCHLD → 清僵尸 → 下一轮 fork 新的。
// supervisor 收到 SIGTERM → 设 g_stop → 通过 socketpair 通知 worker 调用 stop()，
// worker 正常退出后 supervisor 回收并退出；10s 未退出才 SIGKILL。
// Supervisor restart loop:
// After init_daemon's double fork + setsid, this process IS the supervisor.
// Each outer-loop iteration forks a fresh worker.
// Any worker death (crash/OOM) → SIGCHLD → reap → fork replacement.
// SIGTERM sets g_stop; supervisor writes one byte to the socketpair, the worker
// calls stop() from its existing io thread, and exits so no orphan holds the port.
static volatile sig_atomic_t g_worker_exited = 0;
static volatile sig_atomic_t g_worker_pid    = 0;
static volatile sig_atomic_t g_stop          = 0;

static void sig_child_handler(int)
{
    // 只回收当前 worker，避免业务里的 system()/popen() 子进程被误判成 worker 退出。
    // 同时保存 errno：信号处理器不应破坏被中断代码的 errno。
    const int saved_errno = errno;
    int stat              = 0;
    for (;;)
    {
        // 全部回收避免僵尸；但只有当前 worker 退出才触发重启。
        pid_t r = waitpid(-1, &stat, WNOHANG);
        if (r > 0)
        {
            if (g_worker_pid <= 0 || r == static_cast<pid_t>(g_worker_pid))
                g_worker_exited = 1;
            continue;
        }
        if (r < 0 && errno == EINTR)
            continue;
        break;
    }
    errno = saved_errno;
}

static void sig_term_handler(int)
{
    g_stop = 1;
}

// 等待 worker 自行退出并回收；10s 内未退出则 SIGKILL，避免留下孤儿。
// 停机通知由 socketpair 负责，这里不再发 SIGTERM。
static void stop_worker(pid_t pid)
{
    if (pid <= 0)
        return;

    // 先非阻塞确认一次，SIGCHLD handler 被阻塞时由这里亲自回收。
    int stat = 0;
    pid_t r  = waitpid(pid, &stat, WNOHANG);
    if (r == pid)
    {
        g_worker_exited = 1;
        return;
    }
    if (r < 0 && errno != EINTR)
        return;// ECHILD：已经回收

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    for (;;)
    {
        r = waitpid(pid, &stat, WNOHANG);
        if (r == pid)
        {
            g_worker_exited = 1;
            return;
        }
        if (r < 0)
        {
            if (errno == EINTR)
                continue;
            return;// ECHILD：已被 handler 回收
        }
        if (std::chrono::steady_clock::now() >= deadline)
        {
            if (kill(pid, SIGKILL) == 0)
            {
                while (waitpid(pid, &stat, 0) < 0 && errno == EINTR)
                {
                }
                g_worker_exited = 1;
            }
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// 快速失败判定与退避上限：存活不足 10s 视为崩溃循环，最长退避 30s。
// Fast-failure window and backoff cap.
constexpr auto WORKER_STABLE_UPTIME = std::chrono::seconds(10);
constexpr int WORKER_BACKOFF_MAX_MS = 30000;

// 可被 SIGTERM 打断的退避等待；false = 收到 g_stop。
// Backoff wait interruptible by SIGTERM; false = g_stop received.
static bool worker_backoff(int fail_count)
{
    if (fail_count <= 0)
        return !g_stop;

    int delay_ms = 1000;
    for (int i = 1; i < fail_count && delay_ms < WORKER_BACKOFF_MAX_MS; ++i)
        delay_ms *= 2;
    if (delay_ms > WORKER_BACKOFF_MAX_MS)
        delay_ms = WORKER_BACKOFF_MAX_MS;

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay_ms);
    while (!g_stop && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return !g_stop;
}

static bool is_conf_file(const fs::path &p)
{
    std::error_code ec;
    return fs::is_regular_file(p, ec) && !ec;
}

static bool try_conf_path(const fs::path &candidate, std::string &out, std::vector<std::string> &tried)
{
    tried.push_back(candidate.string());
    if (!is_conf_file(candidate))
        return false;
    out = candidate.string();
    return true;
}

// 与 serverconfig::init_path 的口径对齐：目录先找 <dir>/conf/server.conf，再找 <dir>/server.conf。
static bool resolve_server_conf(int argc, char *argv[], std::string &out, std::vector<std::string> &tried)
{
    if (argc > 1)
    {
        const std::string arg = (argv[1] != nullptr) ? argv[1] : "";
        if (arg.empty())
        {
            tried.push_back("<empty argv[1]>");
            return false;
        }
        const fs::path p(arg);
        if (try_conf_path(p, out, tried))
            return true;

        std::error_code ec;
        if (fs::is_directory(p, ec) && !ec)
        {
            if (try_conf_path(p / "conf" / "server.conf", out, tried))
                return true;
        }
        return try_conf_path(p / "server.conf", out, tried);
    }

    std::error_code ec;
    const fs::path cwd = fs::current_path(ec);
    if (!ec && !cwd.empty() && try_conf_path(cwd / "conf" / "server.conf", out, tried))
        return true;
    if (try_conf_path(fs::path("/usr/local/etc/paozhu/server.conf"), out, tried))
        return true;
    return try_conf_path(fs::path("/etc/paozhu/conf/server.conf"), out, tried);
}

int main(int argc, char *argv[])
{
    std::string argv_str;
    std::vector<std::string> tried;
    if (!resolve_server_conf(argc, argv, argv_str, tried))
    {
        std::cerr << "paozhu: server.conf not found; tried:\n";
        for (const auto &p : tried)
            std::cerr << "  " << p << "\n";
        std::cerr << "pass the server.conf path or a conf directory as argv[1]\n";
        return 1;
    }

    init_daemon();

    signal(SIGCHLD, sig_child_handler);
    signal(SIGTERM, sig_term_handler);
    signal(SIGINT, sig_term_handler);
    signal(SIGHUP, sig_term_handler);

    int fail_count = 0;
    while (!g_stop)
    {
        int stop_sv[2] = {-1, -1};
        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, stop_sv) != 0)
        {
            perror("socketpair error:");
            exit(1);
        }

        // fork 与登记 g_worker_pid 之间屏蔽 SIGCHLD，避免信号窗口里误回收/误置标志。
        sigset_t block_chld, old_mask;
        sigemptyset(&block_chld);
        sigaddset(&block_chld, SIGCHLD);
        sigprocmask(SIG_BLOCK, &block_chld, &old_mask);

        g_worker_exited = 0;
        pid_t pid       = fork();
        if (pid < 0)
        {
            ::close(stop_sv[0]);
            ::close(stop_sv[1]);
            sigprocmask(SIG_SETMASK, &old_mask, nullptr);
            perror("fork error:");
            exit(1);
        }

        if (pid == 0)
        {
            // ── worker 子进程 ────────────────────────────────────────
            // 恢复默认信号处理；停止由 supervisor 通过 socketpair 通知，SIGTERM 等仍作硬杀兜底。
            signal(SIGCHLD, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            signal(SIGHUP, SIG_DFL);
            sigprocmask(SIG_SETMASK, &old_mask, nullptr);

            ::close(stop_sv[0]);
            const int fd_flags = ::fcntl(stop_sv[1], F_GETFL, 0);
            if (fd_flags >= 0)
                ::fcntl(stop_sv[1], F_SETFL, fd_flags | O_NONBLOCK);

            try
            {
                http::httpserver &httpmy = http::get_server_app();

                // 监听 supervisor 的停机通知：回调跑在框架已有的 io 线程上，不额外开线程。
                auto stop_reader = std::make_shared<asio::posix::stream_descriptor>(httpmy.get_ctx(), stop_sv[1]);
                auto stop_byte   = std::make_shared<unsigned char>(0);
                stop_reader->async_read_some(
                    asio::buffer(stop_byte.get(), 1),
                    [&httpmy, stop_reader, stop_byte](const asio::error_code &ec, std::size_t)
                    {
                        if (!ec)
                            httpmy.stop();
                    });

                // paozhu-rbac:begin
                // 只装钩子，不查库：带 maskcode 的路由在第一个请求上就会 fail-closed，不必拦启动。
                http::rbac::install_route_guard();
                // paozhu-rbac:end
                httpmy.run(argv_str);
            }
            catch (std::exception &e)
            {
                std::printf("Exception: %s\n", e.what());
                exit(1);
            }
            exit(0);
            // ────────────────────────────────────────────────────────
        }

        ::close(stop_sv[1]);
        g_worker_pid = pid;
        sigprocmask(SIG_SETMASK, &old_mask, nullptr);

        // ── supervisor 父进程：等 worker 退出或收到 SIGTERM ────────
        // 记录 worker 启动时间，用于判定快速失败。
        const auto worker_started = std::chrono::steady_clock::now();

        while (!g_worker_exited && !g_stop)
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (g_stop)
        {
            if (!g_worker_exited)
            {
                const char stop_byte = 1;
                while (::write(stop_sv[0], &stop_byte, 1) < 0 && errno == EINTR)
                {
                }
                // 屏蔽 SIGCHLD 后自己回收，避免 stop_worker 与 handler 抢同一个 pid。
                sigemptyset(&block_chld);
                sigaddset(&block_chld, SIGCHLD);
                sigprocmask(SIG_BLOCK, &block_chld, &old_mask);
                stop_worker(pid);
                sigprocmask(SIG_SETMASK, &old_mask, nullptr);
            }
            ::close(stop_sv[0]);
            break;
        }

        ::close(stop_sv[0]);
        g_worker_pid = 0;

        // 快速失败累计退避；稳定运行超过阈值后清零。
        if (std::chrono::steady_clock::now() - worker_started < WORKER_STABLE_UPTIME)
        {
            if (fail_count < 6)
                ++fail_count;
        }
        else
        {
            fail_count = 0;
        }

        if (!worker_backoff(fail_count))
            break;
        // ────────────────────────────────────────────────────────
    }

    return 0;
}
