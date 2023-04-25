#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testcors.h"
namespace http
{
//@urlpath(null,pxapi/user/infos)
std::string testcors(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    /*
        Access-Control-Allow-Origin: https://foo.bar.org
        Access-Control-Allow-Methods: POST, GET, OPTIONS, DELETE
        Access-Control-Max-Age: 86400
    */
    // *  or domain.com
    client.set_header("Access-Control-Allow-Origin", "*");
    client.set_header("Access-Control-Allow-Headers", "*");
    client.set_header("Access-Control-Expose-Headers", "*");
    client.set_header("Access-Control-Max-Age", "1800");

    // http OPTIONS request response
    if (client.method > 2)
    {
        client.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        return "";
    }
    // main content output
    client << " testcors 🧨 Paozhu c++ web framework ";

    return "";
}

//@urlpath(null,pxapi/user/info)
std::string testcorssimple(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    /*
        Access-Control-Allow-Origin: https://foo.bar.org
        Access-Control-Allow-Methods: POST, GET, OPTIONS, DELETE
        Access-Control-Max-Age: 86400
    */
    // *  or domain.com
    // client.set_header("Access-Control-Allow-Origin", "*");
    // client.set_header("Access-Control-Allow-Headers", "*");
    // client.set_header("Access-Control-Expose-Headers", "*");
    // client.set_header("Access-Control-Max-Age", "1800");
    client.cors_domain("*");
    // http OPTIONS request response
    if (client.method == 3)
    {
        // client.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        client.cors_method();
        return "";
    }
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

} // namespace http