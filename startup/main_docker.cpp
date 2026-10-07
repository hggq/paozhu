#include <string>
#include <iostream>
#include <filesystem>
#include <csignal>
#include "server.h"

namespace fs = std::filesystem;

// Docker 模式: 直接前台运行, PID 1 = 服务进程
// - 不 fork, 让 Docker 管理进程生命周期
// - 捕获 SIGTERM/SIGINT 优雅关闭 (关闭 acceptor → listener 线程退出 → join 完成)
// - 崩溃重启由 compose 的 restart: unless-stopped 负责

static void signal_handler(int signo)
{
    // 收到停止信号时调用 server.stop(), 关闭 acceptor 唤醒阻塞的 accept()
    http::httpserver &srv = http::get_server_app();
    srv.stop();
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

    try
    {
        http::httpserver &httpmy = http::get_server_app();
        httpmy.run(argv_str);
    }
    catch (std::exception &e)
    {
        std::printf("Exception: %s\n", e.what());
    }
    return 0;
}
