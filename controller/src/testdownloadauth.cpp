#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testdownloadauth.h"
namespace http
{
//@urlpath(null,downloadfileauth)
std::string downloadfileauthmethod(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    //if (client.cookie["token"].to_string() ==md5(client.cookie["uid"].to_string()+"FWWdR@AB(755y2qwjjj)"))
    if (client.get["token"].to_int() != 123456)
    {
        client << " token is wrong ";
        return "fail";
    }
    return "";
}

}// namespace http