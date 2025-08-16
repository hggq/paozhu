#include <chrono>
#include <thread>
#include <vector>
#include <list>
#include "httppeer.h"
#include "test_reverse.h"
#include "func.h"
namespace http
{
//@urlpath(null,testmb_reverse)
std::string testmb_reverse(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::string a    = "中👨‍👩‍👧‍👧a国b人🧝‍♀️c";
    client << "<p>Original characters|" << a << "|</p>";
    client << "<p>Reverse characters|" << mb_reverse(a) << "|</p>";

    return "";
}

}// namespace http