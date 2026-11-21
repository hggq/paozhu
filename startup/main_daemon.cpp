#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <sys/wait.h>
#include "server.h"
#include "init_daemon.hpp"
// paozhu-rbac:begin
#include "rbac/route_guard.h"
// paozhu-rbac:end

namespace fs = std::filesystem;

// Supervisor 重启机制：
// init_daemon 双 fork + setsid 后，当前进程是 daemon（即 supervisor）。
// 外层 while 每轮 fork 一个 worker 跑服务。
// worker 任何原因退出（崩溃/OOM）→ supervisor 收到 SIGCHLD → 清僵尸 → 下一轮 fork 新的。
// supervisor 收到 SIGTERM → 设 g_stop，不再 fork，等最后一个 worker 退出后自己也退。
// Supervisor restart loop:
// After init_daemon's double fork + setsid, this process IS the supervisor.
// Each outer-loop iteration forks a fresh worker.
// Any worker death (crash/OOM) → SIGCHLD → reap → fork replacement.
// SIGTERM sets g_stop; supervisor stops forking and exits after the
// last worker is gone. This keeps killall working.
static volatile sig_atomic_t g_worker_exited = 0;
static volatile sig_atomic_t g_stop = 0;

static void sig_child_handler(int)
{
    // 非阻塞收僵尸，设标志让主循环知道可以 fork 下一个 worker 了。
    // Non-blocking reap; flag tells main loop to fork a new worker.
    int stat;
    while (waitpid(-1, &stat, WNOHANG) > 0) {}
    g_worker_exited = 1;
}

static void sig_term_handler(int)
{
    g_stop = 1;
}

int main(int argc, char *argv[])
{
  std::string argv_str;
  if (argc > 1)
  {
    // server.conf filepath or confpath
    argv_str.append(argv[1]);
    fs::path conf_path = argv_str;
    if (fs::is_regular_file(conf_path))
    {

    }
    else
    {
       if(argv_str.back()=='/')
       {
        argv_str=argv_str+"server.conf";
       }
       else
       {
        argv_str=argv_str+"/server.conf";
       }
       conf_path = argv_str;
       if (fs::is_regular_file(conf_path))
       {

       }
       else
       {
          // 启动期找不到配置是一件必须让人看见的事
          std::cerr << "paozhu: server.conf not found, tried: " << argv[1] << " and "
                    << argv_str << "\n";
          return 1;
       }

    }
  }
  else
  {

    fs::path conf_path = fs::current_path();
    argv_str=conf_path.string()+"/conf/server.conf";
    conf_path=argv_str;
    if (fs::is_regular_file(conf_path))
    {

    }
    else
    {
      argv_str="/usr/local/etc/paozhu/server.conf";
      conf_path=argv_str;
      if (fs::is_regular_file(conf_path))
      {

      }
      else
      {
        std::cerr << "paozhu: server.conf not found, tried: " << argv_str
                  << " and /usr/local/etc/paozhu/server.conf; pass the server.conf path as argv[1]\n";
        return 1;
      }

    }
  }
  init_daemon();

  signal(SIGCHLD, sig_child_handler);
  signal(SIGTERM, sig_term_handler);

  while (!g_stop)
  {
    g_worker_exited = 0;

    pid_t pid = fork();
    if (pid < 0)
    {
      perror("fork error:");
      exit(1);
    }

    if (pid == 0)
    {
      // ── worker 子进程 ────────────────────────────────────────
      // 恢复默认信号处理，避免继承 supervisor 的 handler
      // Restore default signals so child doesn't inherit supervisor handlers
      signal(SIGCHLD, SIG_DFL);
      signal(SIGTERM, SIG_DFL);

      try
      {
        http::httpserver &httpmy = http::get_server_app();
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

    // ── supervisor 父进程：等 worker 退出或收到 SIGTERM ────────
    // Parent: wait for worker exit or SIGTERM
    while (!g_worker_exited && !g_stop)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    // ────────────────────────────────────────────────────────
  }

  return 0;
}
