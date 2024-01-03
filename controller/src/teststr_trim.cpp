#include <chrono>
#include <thread>
#include <vector>
#include <list>
#include "httppeer.h"
#include "teststr_trim.h"
#include "func.h"
namespace http
{
//@urlpath(null,teststr_trim)
std::string teststrtrim(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    std::string a    = " 0123456789abcdefghij ";
    client << "<p>strim|" << a << "|</p>";
    client << "<p>str_trim result|" << str_trim(a) << "|</p>";
    client << "<p>str_trim_view result|" << str_trim_view(a) << "|</p>";
    return "";
}

}// namespace http