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
        std::vector<orm::cust::LocalusersqlStruct> loaduser;

        auto  ulink= std::make_unique<orm::db_conn>("cms");
        std::string sqlstring="SELECT adminid,name,nickname FROM ";
        sqlstring.append("sysuser");
        sqlstring.append(" where 1 limit 1");

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

}// namespace http