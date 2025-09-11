#include <string>
#include <iostream>

#include "orm.h"
#include "mysql_conn.h"
#include "mysql_conn_pool.h"

/**
g++ test/test_mysql_orm.cpp -std=c++20 -Imodels -Imodels/cms/include models/cms/Sysuser.cpp vendor/httpserver/src/mysql_conn.cpp vendor/httpserver/src/mysql_conn_pool.cpp  -Ivendor/httpserver/include -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -ldl -Iorm -Iorm/cms -Icommon -Iorm/cms/include -Iorm/include -Imodels/include  vendor/httpserver/src/mysql_connect_mar.cpp vendor/httpserver/src/func.cpp vendor/httpserver/src/datetime.cpp vendor/httpserver/src/clientdatacache.cpp models/cms/Article.cpp -g -fsanitize=address -Wall
**/

int main(int argc, char *argv[])
{
    try
    {
        asio::io_context io_context{0};
        std::string configpath          = "./conf/orm.conf";
 
        std::string orm_log_file = orm::init_orm_conn_pool(io_context, configpath );
        std::cout<<orm_log_file<<std::endl;
        auto users = orm::cms::Sysuser();
 
        users.where("name","admin").fetch_one();
        if (users.getAdminid() > 0)
        {
        std::printf("found: %s\n", users.data.name.c_str());
        }
        std::printf("echo\n");
        orm::init_orm_conn_pool_release();
    }
    catch (std::exception &e)
    {
        std::printf("Exception: %s\n", e.what());
    }
    catch (const char *e)
    {
        std::printf("Exception: %s\n", e);
    }
    std::cout<<"--end--"<<std::endl;
    return 0;
}
