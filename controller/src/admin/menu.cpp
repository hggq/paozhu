#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "func.h"
#include "menu.h"
#include "md5.h"
#include <filesystem>
namespace http
{
namespace fs = std::filesystem;
//@urlpath(null,admin/menu)
std::string admin_menu(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("admin/menu");
    return "";
}

//@urlpath(null,admin/vuetest)
std::string admin_vuetest(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("admin/vuetest");
    return "";
}
 
//@urlpath(null,admin/dashboard)
std::string admin_dashboard(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("admin/dashboard");
    return "";
}

//@urlpath(null,admin/jstimetest)
std::string admin_jstimetest(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("admin/jstimetest");
    return "";
}

//@urlpath(null,admin/testelementplus)
std::string admin_testelementplus(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.view("admin/elementplus");
    return "";
}


//@urlpath(null,admin/testgetjson)
std::string admin_testgetjson(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    client.output=R"([{ "date": "2025-01-01", "name": "张三", "address": "上海市" },  { "date": "2025-01-02", "name": "李四", "address": "北京市" }, { "date": "2025-01-03", "name": "王五", "address": "深圳市" }])";
    client.type("application/json");    
    return "";
}

}//namespace http
