#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testcookie.h"
namespace http
{

std::string testaddcookie(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  add cookie ";
    client << client.get_hosturl();
    client << "<p><a href=\"" << client.get_hosturl() << "/showcookie\">show</a></p>";

    // client<<client.remote_ip;
    // client<<client.remote_port;

    client.session["aaa"] = 1111;
    client.save_session();

    client.set_cookie("ccbb", "123456abc", 7200 * 24);
    client.set_cookie("same", "samesite", 7200 * 24, "/", client.host, false, true, "Lax");
    client.set_header("server", "cpphttp");
    return "";
}
std::string testshowcookie(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "hello world!  show cookie<br/>";
    client << "<p><a href=\"" << client.get_hosturl() << "/addcookie\">add cookie</a></p>";
    client << "<p>session:|" << client.session["aaa"] << "|</p>";
    client << client.client_ip;
    client << "<br/>";
    client << client.client_port;

    client << "<p>cookie same:" << client.cookie["same"] << "</p>";
    client << "<p>cookie ccbb:" << client.cookie["ccbb"] << "</p>";
    client.view("cookie/show");
    return "";
}

}// namespace http