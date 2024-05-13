#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testsiteid.h"
namespace http
{
//@urlpath(null,testsiteid)
std::string testsiteid(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "siteid: " << client.get_siteid();

    return "";
}

}//namespace http