#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "testmysql.h"

namespace http
{

std::string testmysqlconnect(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  testmysqlconnect ";
    client << client.get_hosturl();
    client << "<p><a href=\"" << client.get_hosturl() << "/showcookie\">show</a></p>";
    auto users = orm::cms::Sysuser();

    users.where("name", "admin").fetch_one();
    try
    {
        client << "<p>sql result</p>";
        // view orm create sql
        client << "<p>sql:" << users.sqlstring << "</p>";
        if (users.getAdminid() > 0)
        {
            // save session,other page get  int userid= client.session["userid"].to_int();
            client.session["aaa"] = users.getAdminid();
            client.save_session();
            client << "<p>found:" << users.data.name << "</p>";
            return "";
        }
        else
        {
            return "";
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