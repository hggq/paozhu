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
#include "dbconver.hpp"
#include "dbtable.hpp"
#include "dbexport.hpp"

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
                     "your MVC files。\n"
                     "Usage:\n"
                     "  paozhu_cli view          update modified view templates to cpp (viewsrc/)\n"
                     "  paozhu_cli view force    force regenerate all view templates to cpp\n"
                     "  paozhu_cli json          update json reflect files\n"
                     "  paozhu_cli orm <dbtag>   update ORM files, dbtag e.g. cms (see conf/orm.conf)\n"
                     "  paozhu_cli dbconver <dbtag1> <dbtag2> [force]   migrate database\n"
                     "  paozhu_cli dbtable <dbtag> <filename.sql> [-target=mysql|pg]  export table structures\n"
                     "  paozhu_cli dbexport <dbtag> <filename.sql> [-target=mysql|pg]  export schema + data\n"
                     "Example:\n"
                     "  paozhu_cli orm pg                               generate ORM from PostgreSQL [pg]\n"
                     "  paozhu_cli orm default                          generate ORM from MySQL [default]\n"
                     "  paozhu_cli dbconver cms pg                      migrate MySQL [cms] to PG [pg]\n"
                     "  paozhu_cli dbconver cms pg force                migrate, overwrite existing\n"
                     "  paozhu_cli dbtable cms ./schema.sql             export MySQL [cms] to SQL file\n"
                     "  paozhu_cli dbtable cms ./pg.sql -target=pg      export MySQL as PG DDL\n"
                     "  paozhu_cli dbtable pg ./mysql.sql -target=mysql export PG as MySQL DDL\n"
                     "  paozhu_cli dbexport cms ./dump.sql              export MySQL schema+data\n"
                     "  paozhu_cli dbexport pg ./mysql.sql -target=mysql export PG as MySQL DDL+data\n"
                     "Input f create view, from view directory to viewsrc c++ cpp file, next input a create all modify html to c++ cpp\n"
                     "Input m create ORM file, from database to ORM c++ cpp file, next select db front number to create.\n"
                     "Input j create json file, scan libs directory *.json, annotation [//@reflect json to_json from_json] struct to json.cpp\n";
    }
    if (argc > 1)
    {
        commandstr = argv[1];
    }
    if (commandstr == "view")
    {
        if (argc > 2 && std::string(argv[2]) == "force")
        {
            return viewfilecli(false, true);
        }
        return viewfilecli(true);
    }
    if (commandstr == "json")
    {
        return jsoncli();
    }

    if (commandstr == "orm")
    {
        if (argc < 3)
        {
            std::cout << "Usage: paozhu_cli orm <dbtag>  (dbtag: section name in conf/orm.conf, e.g. cms)" << std::endl;
            return 1;
        }
        return modelcli(argv[2]);
    }

    if (commandstr == "dbconver")
    {
        if (argc < 4)
        {
            std::cout << "Usage: paozhu_cli dbconver <dbtag1> <dbtag2> [force]" << std::endl;
            std::cout << "  dbtag1: source database tag (from conf/orm.conf)" << std::endl;
            std::cout << "  dbtag2: target database tag (from conf/orm.conf)" << std::endl;
            std::cout << "  force:  drop and recreate if table already exists (optional)" << std::endl;
            return 1;
        }
        return dbconver::dbconvercli(argv[2], argv[3], (argc >= 5 ? argv[4] : ""));
    }

    if (commandstr == "dbtable")
    {
        if (argc < 4)
        {
            std::cout << "Usage: paozhu_cli dbtable <dbtag> <filename.sql> [-target=mysql|postgresql]" << std::endl;
            std::cout << "  dbtag:    database tag (from conf/orm.conf)" << std::endl;
            std::cout << "  filename: output SQL file path" << std::endl;
            std::cout << "  -target:  target database type for DDL conversion (optional)" << std::endl;
            return 1;
        }

        std::string target_arg;
        for (int i = 4; i < argc; i++)
        {
            std::string arg = argv[i];
            if (arg.find("-target=") == 0)
            {
                target_arg = arg.substr(8);
            }
        }

        return dbtable::dbtablecli(argv[2], argv[3], target_arg);
    }

    if (commandstr == "dbexport")
    {
        if (argc < 4)
        {
            std::cout << "Usage: paozhu_cli dbexport <dbtag> <filename.sql> [-target=mysql|postgresql]" << std::endl;
            std::cout << "  dbtag:    database tag (from conf/orm.conf)" << std::endl;
            std::cout << "  filename: output SQL file path" << std::endl;
            std::cout << "  -target:  target database type for DDL conversion (optional)" << std::endl;
            return 1;
        }

        std::string target_arg;
        for (int i = 4; i < argc; i++)
        {
            std::string arg = argv[i];
            if (arg.find("-target=") == 0)
            {
                target_arg = arg.substr(8);
            }
        }

        return dbexport::dbexportcli(argv[2], argv[3], target_arg);
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
