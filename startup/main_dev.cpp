#include <string>
#include <iostream>
#include "server.h"
#include "serverconfig.h"
// paozhu-rbac:begin
#include "rbac/route_guard.h"
// paozhu-rbac:end

int main(int argc, char *argv[])
{
    try
    {
        http::httpserver &httpmy = http::get_server_app();
        std::string argv_str;
        if (argc > 1)
        {
            //server.conf filepath or confpath
            argv_str.append(argv[1]);
        }
        // paozhu-rbac:begin
        // 只装钩子，不查库：钩子未装时带 maskcode 的路由按 403 拒绝，装了以后
        // 权限表不可读也在第一个请求上 fail-closed，不需要在这里拦启动。
        http::rbac::install_route_guard();
        // paozhu-rbac:end
        httpmy.run(argv_str);
        // run() 找不到 server.conf 只打一行日志就返回，退出码照旧是 0——
        // "根本没起来"和"跑完收工"在 shell 里看起来一模一样，所以在这里补一个非零出口。
        if (http::getserversysconfig().configfile.empty())
        {
            std::cerr << "paozhu: server.conf not found; pass its full path or a conf directory as argv[1]\n";
            return 1;
        }
    }
    catch (std::exception &e)
    {
        std::printf("Exception: %s\n", e.what());
        return 1;
    }
    return 0;
}
