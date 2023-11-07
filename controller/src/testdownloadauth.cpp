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
    if (client.get["token"].to_int() != 123456)
    {
        client << " token is wrong ";
        return "fail";
    }
    return "";
}

}// namespace http