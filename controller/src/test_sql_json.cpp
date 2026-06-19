#include "Fortune.h"
#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "test_sql_json.h"
 
namespace http
{
//@urlpath(null,testsqljson)
std::string testsqljson(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  alone use sqlquery ";

    try
    {
        auto fortune_m = orm::Fortune();
        fortune_m.where("id",1).limit(1).fetch_one();

        std::string jsonstr= fortune_m.data_tojson();
        client << "<p>id:" << fortune_m.data.id<<", message:"<< fortune_m.data.message << "</p>";
        client << "<p>" << jsonstr << "</p>";
        jsonstr = R"({"id":55,"message":"uuuu🚀马到成功uuuu"})";
        fortune_m.from_json(jsonstr);
        client << "<p>" << jsonstr << "</p>";
        client << "<p>id:" << fortune_m.data.id<<", message:"<< fortune_m.data.message << "</p>";
        jsonstr = R"([{"id":999999,"message":"gggggg🚀ggg"},{"id":66,"message":"mmmm马到成功mmmmmmm"}])";
        client << "<p>" << jsonstr << "</p>";
        fortune_m.from_json(jsonstr);
        client << "<p>" << fortune_m.to_json() << "</p>";
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
        return "";
    }
    return "";
}

 

}// namespace http