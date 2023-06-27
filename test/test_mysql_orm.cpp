#include <string>
#include <iostream>
#include "orm.h"
#include "serverconfig.h"
#include "mysqlconfig.h"
#include "mysqlpool.h"
#include "mysqlproxyfun.h"
// g++ test/test_mysql_orm.cpp -std=c++20 -Imodels -Imodels/cms/include models/cms/User.cpp
// vendor/httpserver/src/mysqlconfig.cpp vendor/httpserver/src/mysqlpool.cpp vendor/httpserver/src/mysqlproxyfun.cpp
// -std=c++20 -Ivendor/httpserver/include -lmysqlcppconn8 -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib
// -lssl -lcrypto -ldl  vendor/httpserver/src/serverconfig.cpp -I/usr/local/mysql/include -I./orm -L/usr/local/mysql/lib
// -lmysqlclient vendor/httpserver/src/datetime.cpp models/cms/Article.cpp vendor/httpserver/src/server_localvar.cpp
// -Wl,-rpath,/usr/local/mysql/lib/

//  g++ test/test_mysql_orm.cpp -std=c++20 -Imodels -Imodels/cms/include models/cms/User.cpp
//  vendor/httpserver/src/mysqlconfig.cpp vendor/httpserver/src/mysqlpool.cpp vendor/httpserver/src/mysqlproxyfun.cpp
//  -std=c++20 -Ivendor/httpserver/include -lmysqlcppconn8 -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib
//  -lssl -lcrypto -ldl  vendor/httpserver/src/serverconfig.cpp

int main(int argc, char *argv[])
{

    try
    {
        // Run paozhu_cli creates orm file from mysql
        http::serverconfig &sysconfigpath = http::getserversysconfig();
        sysconfigpath.configpath          = "./conf/";
        http::mysqlconfig_init_link();
        auto users = orm::cms::User();
        users.where("name", "admin").limit(1).fetch();

        std::cout << users.sqlstring << std::endl;
        std::cout << users.data.name << std::endl;
    }
    catch (std::exception &e)
    {
        std::printf("Exception: %s\n", e.what());
    }
}
