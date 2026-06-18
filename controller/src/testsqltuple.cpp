#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "testsqltuple.h"


namespace orm::cust
{
    struct LocaluserStruct : orm::Base<LocaluserStruct> 
    {
        unsigned int adminid;
        std::string name;
        std::string nickname;
        ORM_NAMES(adminid, name, nickname);
    };

}

namespace http
{
//@urlpath(null,mtuple)
std::string testsqltuple(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  testsqltuple ";

    try
    {
           std::vector<orm::cust::LocaluserStruct> loaduser;
           auto users = orm::cms::Sysuser();
           std::string sqlstring="SELECT adminid,name,nickname FROM ";
           sqlstring.append(orm::cms::Sysuser::org_tablename);
           sqlstring.append(" where 1 limit 1");

           users.query(sqlstring,loaduser);

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
                client << users.error_msg;
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