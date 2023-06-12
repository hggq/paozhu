#include <chrono>
#include <thread>
#include "httppeer.h"
#include "devcors.h"
namespace http
{
//@urlpath(null,api/dev/hostcors)
std::string api_dev_hostcors(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client.cors_domain("*");
    if (client.method == HEAD_METHOD::OPTIONS)
    {
        client.cors_method();
        return "exit";
    }
    return "";
}

}// namespace http