#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "testmodelfromjson.h"

namespace http
{
//@urlpath(null,mfromjson)
std::string testmodelfromjson(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  testmodelfromjson ";
    client << client.get_hosturl();

    try
    {
        auto users          = orm::cms::Sysuser();
        std::string jsonstr = R"({"adminid":3333,"name":"myname hzq","password":"\u4e2d\u56fd\ud83c\udde8\ud83c\uddf3\ud83d\udc68\u200d\ud83d\udc69\u200d\ud83d\udc67\u200d\ud83d\udc67\u4eba\u6c11","isopen":1,"level":11})";
        users.from_json(jsonstr);
        client << "<p>adminid:" << users.data.adminid << "</p>";
        client << "<p>name:" << users.data.name << "</p>";
        client << "<p>password:" << users.data.password << "</p>";
        client << "<p>isopen:" << std::to_string(users.data.isopen) << "</p>";
        client << "<p>level:" << std::to_string(users.data.level) << "</p>";

        jsonstr = R"([{"adminid":6666,"name":"myname \" hzq","password":"aabb\u4e2d\u56fd\ud83c\udde8\ud83c\uddf3\ud83d\udc68\u200d\ud83d\udc69\u200d\ud83d\udc67\u200d\ud83d\udc67\u4eba\u6c11","isopen":11,"level":111},{"adminid":4444,"name":"myname' hzq","password":"\u4e2d\u56fd\ud83c\udde8\ud83c\uddf3\ud83d\udc68\u200d\ud83d\udc69\u200d\ud83d\udc67\u200d\ud83d\udc67\u4eba\u6c11\ud83e\udea2\"","isopen":2,"level":22}] )";
        users.from_json(jsonstr);
        client << "<p>adminid:" << users.data.adminid << "</p>";
        client << "<p>name:" << users.data.name << "</p>";
        client << "<p>password:" << users.data.password << "</p>";
        client << "<p>isopen:" << std::to_string(users.data.isopen) << "</p>";
        client << "<p>level:" << std::to_string(users.data.level) << "</p>";

        for (unsigned int i = 0; i < users.record.size(); i++)
        {
            client << "<p>adminid:" << users.record[i].adminid << "</p>";
            client << "<p>name:" << users.record[i].name << "</p>";
            client << "<p>password:" << users.record[i].password << "</p>";
            client << "<p>isopen:" << std::to_string(users.record[i].isopen) << "</p>";
            client << "<p>level:" << std::to_string(users.record[i].level) << "</p>";
        }

        client << "<p>data_tojson:" << users.data_tojson() << "</p>";
        client << "<p>to_json:" << users.to_json() << "</p>";
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    return "";
}

}// namespace http