#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testview.h"
namespace http
{

std::string testloginview(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << " 视图测试 ";
    // client << client.gethosturl();
    // client<<"<p><a href=\""<<client.gethosturl()<<"/showcookie\">show</a></p>";

    peer->view("login/login");
    return "";
}

}// namespace http