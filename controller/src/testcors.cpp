#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testcors.h"
namespace http
{
//@urlpath(null,api/user/message)
std::string testcors(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // CORS 全部由框架处理：普通请求的 Allow-Origin 在解析期按站点 cors_domain 白名单判定，
    // OPTIONS 预检由 httpserver::send_cors_domain() 直接应答、不会进到控制器。
    // 原 set_header("Access-Control-*") 与 OPTIONS 分支已删除。
    // main content output
    {
        client.val["code"] = 0;
        client.val["data"] = (int)rand_range(0, 99);
    }
    client.out_json();
    return "";
}

//@urlpath(null,api/user/info)
std::string testcorssimple(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // 同 testcors：CORS 由框架处理，原 cors_domain("*") 与 OPTIONS 分支已删除
    // main content output
    // client << " testcors 🧨 Paozhu c++ web framework ";

    client.val["code"]           = 0;
    client.val["data"]["id"]     = 1;
    client.val["data"]["name"]   = "Admins";
    client.val["data"]["avatar"] = "";
    client.val["data"]["roles"].set_array();
    client.val["data"]["roles"].push("admin");

    client.out_json();
    return "";
}

//@urlpath(null,api/user/vary)
std::string testcorsvary(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // add_vary 是追加+去重：解析期命中白名单时框架已经写过 Vary: Origin，
    // 这里再补业务字段不应把 Origin 冲掉；重复字段名（含大小写不同的写法）不再追加一次。
    client.add_vary("Accept-Language");
    client.add_vary("origin");
    client.add_vary("Accept-Language, X-Test");

    client.val["code"] = 0;
    client.out_json();
    return "";
}

}// namespace http