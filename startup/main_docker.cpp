#include <string>
#include <iostream>
#include <filesystem>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>
#include "server.h"

namespace fs = std::filesystem;

// Docker 模式: 直接前台运行, PID 1 = 服务进程
// - 不 fork, 让 Docker 管理进程生命周期
// - 捕获 SIGTERM/SIGINT 优雅关闭 (关闭 acceptor → listener 线程退出 → join 完成)
// - 崩溃重启由 compose 的 restart: unless-stopped 负责

// 信号处理器只置标志位：stop() 会取互斥锁、join 线程，在信号上下文里调用属于未定义行为。
// Signal handler only sets a flag: stop() takes mutexes and joins threads, which is
// undefined behaviour inside a signal context.
static volatile sig_atomic_t g_stop_requested = 0;
static std::atomic<bool>    g_run_finished{false};

static void signal_handler(int signo)
{
    (void)signo;
    g_stop_requested = 1;
}

int main(int argc, char *argv[])
{
    std::string argv_str;
    if (argc > 1)
    {
        // server.conf filepath or confpath
        argv_str.append(argv[1]);
        fs::path conf_path = argv_str;
        if (!fs::is_regular_file(conf_path))
        {
            if (argv_str.back() == '/')
            {
                argv_str = argv_str + "server.conf";
            }
            else
            {
                argv_str = argv_str + "/server.conf";
            }
            conf_path = argv_str;
            if (!fs::is_regular_file(conf_path))
            {
                // 容器里这条路径尤其要响：静默 return 0 会被 docker 当成"正常跑完"，
                // restart 策略再把同一个失败重复一遍，日志里一行线索都没有。
                std::cerr << "paozhu: server.conf not found, tried: " << argv[1] << " and "
                          << argv_str << "\n";
                return 1;
            }
        }
    }
    else
    {
        fs::path conf_path = fs::current_path();
        argv_str           = conf_path.string() + "/conf/server.conf";
        conf_path          = argv_str;
        if (!fs::is_regular_file(conf_path))
        {
            argv_str  = "/usr/local/etc/paozhu/server.conf";
            conf_path = argv_str;
            if (!fs::is_regular_file(conf_path))
            {
                // 这里的候选只有两条，比配置加载器（serverconfig::init_path）少一条
                // /etc/paozhu/conf/server.conf：装在那儿的部署走不到加载器就先退出了。
                std::cerr << "paozhu: server.conf not found, tried: " << argv_str
                          << " and /usr/local/etc/paozhu/server.conf; mount the conf dir or pass the path\n";
                return 1;
            }
        }
    }

    // 注册信号: docker stop 发送 SIGTERM, Ctrl+C 发送 SIGINT
    // Windows 无 SIGTERM, 仅注册 SIGINT
#ifdef _WIN32
    std::signal(SIGINT, signal_handler);
#else
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);
#endif

    // 轮询线程负责真正调用 stop()：信号处理器只置位，由普通线程上下文完成
    // 关 acceptor、唤醒 listener、join 这一整套动作。
    // The poller performs the real stop() from a normal thread context.
    std::thread stopper;
    try
    {
        http::httpserver &httpmy = http::get_server_app();
        stopper = std::thread(
            [&httpmy]()
            {
                while (!g_run_finished.load())
                {
                    if (g_stop_requested != 0)
                    {
                        httpmy.stop();
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            });

        httpmy.run(argv_str);
    }
    catch (std::exception &e)
    {
        std::printf("Exception: %s\n", e.what());
    }
    // 无论 run() 正常返回还是异常退出，都要让轮询线程收尾，避免 joinable 线程析构
    g_run_finished.store(true);
    if (stopper.joinable())
    {
        stopper.join();
    }
    return 0;
}
