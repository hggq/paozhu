#include <chrono>
#include <thread>
#include <vector>
#include <list>
#include "httppeer.h"
#include "teststr_join.h"
#include "func.h"
namespace http
{
//@urlpath(null,teststr_join)
std::string teststrjoin(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::vector<std::string> a;
    a.push_back("aaaa");
    a.push_back("bbbb");
    a.push_back("cccc");
    client << "<p>std::vector<std::string>:" << str_join(a, ',') << "</p>";

    std::list<std::string> b;
    b.push_back("aaaa");
    b.push_back("bbbb");
    b.push_back("cccc");
    client << "<p>std::list<std::string>:" << str_join(b, ',') << "</p>";

    client << "<p>std::list<std::string>:" << str_join(b) << "</p>";
    return "";
}

}// namespace http