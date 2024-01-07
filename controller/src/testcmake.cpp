#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testcmake.h"
namespace http
{

//@urlpath(null,ccmake)
std::string testcmake(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " Hello world! 🧨 Paozhu c++ web framework ";

    return "";
}
//@urlpath(null,ccauto)
std::string testcauto(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " Hello world! 🧨 Paozhu c++ web framework ";
    return "";
}
}// namespace http
