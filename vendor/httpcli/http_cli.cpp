/*
 *  @author 黄自权 huangziquan
 *  @date 2022-04-15
 *  @dest 命令行管理mvc文件
 */
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <filesystem>
#include "utility.hpp"
#include "templateparse.hpp"
#include "controllerparse.hpp"

#include "tmplatefun.hpp"
#include "ccontrollfun.hpp"
#include "modelfun.hpp"

#include "tmplatefunfile.hpp"
#include "templateparsefile.hpp"
#include "struct_reflectfile.hpp"

namespace fs = std::filesystem;
/*
g++ http_cli.cpp -std=c++20 -I../httpserver/include -I../../common \
 ../httpserver/src/mysql_conn.cpp \
 ../httpserver/src/mysql_conn_pool.cpp \
 -lssl -lcrypto -ldl -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lz \
 ../httpserver/src/clientdatacache.cpp -g -fsanitize=address
*/ 
int main(int argc, char *argv[])
{
    std::string commandstr;
    if (argc == 1)
    {
        std::cout << " \033[1m\033[31m model ｜ view | viewtocpp | control \033[0m "
                     " \n Welcome to use \033[4m\033[31mcli\033[0m to manage "
                     "your MVC files。\n";
    }
    if (argc > 1)
    {
        commandstr = argv[1];
    }
    // std::string commandstr{argv[1]};
    while (1)
    {
        if (commandstr == "file")
        {
            viewfilecli();
        }
        if (commandstr == "view")
        {
            viewcli();
        }
        if (commandstr == "control")
        {
            controlcli();
        }
        if (commandstr == "model")
        {
            modelcli();
        }
        if (commandstr == "json")
        {
            jsoncli();
        }
        commandstr.clear();
        std::cout << "(m)model (v)view (f)viewtocpp or (c)control , (j)son ,x or q "
                     "to exit[input\033[1m\033[32m m|v|f|c|j|\033[0m]:";
        std::cin >> commandstr;

        if (commandstr.size() == 1 &&
            (commandstr[0] == 'x' || commandstr[0] == 'q'))
        {
            break;
        }
        if (commandstr.size() == 1 &&
            (commandstr[0] == 'v' || commandstr[0] == 'V'))
        {

            viewcli();
        }
        if (commandstr.size() == 1 &&
            (commandstr[0] == 'f' || commandstr[0] == 'F'))
        {

            viewfilecli();
        }
        if (commandstr.size() == 1 &&
            (commandstr[0] == 'c' || commandstr[0] == 'C'))
        {

            controlcli();
        }
        if (commandstr.size() == 1 &&
            (commandstr[0] == 'm' || commandstr[0] == 'M'))
        {

            modelcli();
        }
        if (commandstr.size() == 1 &&
            (commandstr[0] == 'j' || commandstr[0] == 'J'))
        {

            jsoncli();
        }
        if (commandstr == "view")
        {
            viewfilecli();
        }
        if (commandstr == "control")
        {
            controlcli();
        }
        if (commandstr == "model")
        {
            modelcli();
        }
        if (commandstr == "file")
        {
            viewcli();
        }
        if (commandstr == "json")
        {
            jsoncli();
        }
        commandstr.clear();
    }

    return 0;
}