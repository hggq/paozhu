#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testmodel.h"
namespace http
{

std::string testmodel(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "testmodel ";
    auto users = orm::cms::Sysuser();
    users.where("name", "admin").fetch_one();

    client << "userid: " << users.getAdminid();
    auto artlists = users.gettoparticle(1);

    for (size_t i = 0; i < artlists.size(); i++)
    {
        client << "<p> title " << artlists[i].title << "</p>";
    }
    return "";
}
std::string testmodelsmartptr(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "testmodel ";
    auto users = std::make_shared<orm::cms::Sysuser>();

    users->where("name", "admin").fetch_one();

    client << "userid: " << users->getAdminid();
    auto artlists = users->gettoparticle(1);

    for (unsigned int i = 0; i < artlists.size(); i++)
    {
        client << "<p> title " << artlists[i].title << "</p>";
    }
    return "";
}
}// namespace http