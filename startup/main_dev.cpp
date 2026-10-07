#include <string>
#include <iostream>
#include "server.h"
#include "serverconfig.h"

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
