#include "httppeer.h"
#include "testhello.h"
namespace http
{
//@urlpath(null,hello)
std::string testhello(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << " Hello world! 🧨 Paozhu c++ web framework ";

    return "";
}

//@urlpath(null,hellobusy)
// this on stress test
std::string testhellobusy(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << " Hello world! 🧨 Paozhu c++ web framework ";
    std::this_thread::sleep_for(std::chrono::seconds(5));// ab -n 60 -c 30 http://127.0.0.1/hellobusy
    return "";
}
}// namespace http