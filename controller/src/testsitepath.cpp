#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testsitepath.h"
namespace http
{
//@urlpath(null,testsitepath)
std::string testsitepath(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "sitepath: " << client.get_sitepath();

    return "";
}

}//namespace http