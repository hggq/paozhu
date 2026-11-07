#include <chrono>
#include <thread>
#include "httppeer.h"
#include "test_co_handle.h"

namespace http
{
//@urlpath(null,testcohandle)
asio::awaitable<std::string> test_co_handle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " hello world! this is a test co_handle function. ";
    co_return "";
}
}//namespace http
