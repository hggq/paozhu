#include "httppeer.h"
#include "testrestfulpath.h"
namespace http
{
//@urlpath(null,user/info/:userid)
std::string testrestfulpath(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << " Hello world! 🧨 Paozhu c++ web framework ";
    client << "<br />user/info/:userid<br /> userid:";
    client << client.get["userid"].to_string();
    return "";
}
//@urlpath(null,user/profile/:userid/:pathid)
std::string testrestfulprofilepath(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << " Hello world! 🧨 Paozhu c++ web framework ";
    client << "<br />user/profile/:userid/:pathid<br /> userid:";
    client << client.get["userid"].to_string();
    client << "<br /> pathid:";
    client << client.get["pathid"].to_string();
    return "";
}
}// namespace http