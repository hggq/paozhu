#include <chrono>
#include <thread>
#include <vector>
#include <list>
#include "httppeer.h"
#include "teststr2int.h"
#include "func.h"
namespace http
{
//@urlpath(null,teststr2int)
std::string teststr2int(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    std::string a    = "-123456789";
    client << "<p>str2int origin|" << a << "|</p>";
    client << "<p>str2int result|" << str2int(a) << "|</p>";
    a = "123456789";
    client << "<p>str2int not result|" << str2int(a) << "|</p>";
    client << "<p>str2int data(),size() result|" << str2int(a.data(), a.size()) << "|</p>";
    client << "<p>char2hex result|" << char2hex((const unsigned char *)a.data(), a.size()) << "|</p>";
    client << "<p>char2hex 4p result|" << char2hex((const unsigned char *)a.data(), a.size(), 4) << "|</p>";
    return "";
}

}// namespace http