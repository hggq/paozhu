#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "test_sql_query.h"
#include "mysql_query.h"

namespace orm::cust
{
    struct LocalusersqlStruct : orm::Base<LocalusersqlStruct> 
    {
        unsigned int adminid;
        std::string name;
        std::string nickname;
        ORM_NAMES(adminid, name, nickname);
    };

}

namespace http
{
//@urlpath(null,sqlquery)
std::string testsqlquery(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  alone use sqlquery ";

    try
    {
        //返回结果结构体，如果只有一条可以使用 orm::cust::LocalusersqlStruct loaduser 单独一个
        // Return the result struct. If there is only one, you can use orm::cust::LocalusersqlStruct loaduser alone
        std::vector<orm::cust::LocalusersqlStruct> loaduser;
        //独立使用数据库 db conn  Use database db conn independently
        //cms为orm.conf配置文件标签 cms is the tag for the orm.conf configuration file
        auto  ulink= std::make_unique<orm::db_conn>("cms"); 
        //select 字段要和返回结果结构体字段对得上
        //The "select" field must correspond to the field of the returned result struct
        std::string sqlstring="SELECT adminid,name,nickname FROM ";
        sqlstring.append("sysuser");
        sqlstring.append(" where 1 limit 1");

        //同步模式 synchronous mode
        ulink->query(sqlstring,loaduser);

        if(loaduser.size() >0)
        {
            client << "sql:"<<sqlstring<<"<hr>";
            if (loaduser.size() > 0)
            {
                for (size_t i = 0; i < loaduser.size(); i++)
                {
                    client << "adminid:";
                    client << loaduser[i].adminid ;
                    client << ", name:";
                    client << loaduser[i].name ;
                    client << ", nickname:";
                    client << loaduser[i].nickname ;
                    client << "<br />";
                        
                }
            }
        }
        else
        {
            client << "<p>--------------</p>";
            client << ulink->error_msg;
        }
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    return "";
}

//@urlpath(null,co_sqlquery)
asio::awaitable<std::string> test_co_sqlquery(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  alone use co_sqlquery ";

    try
    {
        std::vector<orm::cust::LocalusersqlStruct> loaduser;
        //独立使用数据库 db conn  Use database db conn independently
        auto  ulink= std::make_unique<orm::db_conn>("cms");
        std::string sqlstring="SELECT adminid,name,nickname FROM ";
        sqlstring.append("sysuser");
        sqlstring.append(" where 1 limit 1");

        //协程模式 Coroutine Pattern
        co_await ulink->async_query(sqlstring,loaduser);

        if(loaduser.size() >0)
        {
            client << "sql:"<<sqlstring<<"<hr>";
            if (loaduser.size() > 0)
            {
                for (size_t i = 0; i < loaduser.size(); i++)
                {
                    client << "adminid:";
                    client << loaduser[i].adminid ;
                    client << ", name:";
                    client << loaduser[i].name ;
                    client << ", nickname:";
                    client << loaduser[i].nickname ;
                    client << "<br />";
                        
                }
            }
        }
        else
        {
            client << "<p>--------------</p>";
            client << ulink->error_msg;
        }
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        co_return "";
    }
    co_return "";
}

}// namespace http