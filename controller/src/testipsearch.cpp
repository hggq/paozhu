#include <chrono>
#include <thread>
#include "httppeer.h"
#include "ip_data_obj.h"
#include "testqrcode.h"
namespace http
{
//@urlpath(null,testipsearch)
std::string testipsearch(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "<p>search ip</p>";
    client << "<form action=\"/ipsearchget\"  method=\"get\">";
    client << "<p><input type=\"text\" id=\"queryword\" name=\"queryword\" value=\"";
    client << client.client_ip;
    client << "\" placeholder=\"input ip\" /><button type=\"submit\">search</button></p>";
    client << "</form>";
    return "";
}
//@urlpath(null,ipsearchget)
std::string testipsearchget(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->getpeer();
    client << "<p>search ip result:";
    ip_data_obj &ipobj=get_ip_data_obj();
    client << ipobj.search(client.get["queryword"].to_string(),true);
    client << "</p>";
    client << "<p>";
    client << ipobj.search(client.get["queryword"].to_string());
    client << "</p>";
    return "";
}
}// namespace http