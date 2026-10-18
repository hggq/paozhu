#include "orm.h"
#include "httppeer.h"
#include <cstddef>
#include "test_orm.h"
namespace http
{
//@urlpath(null,testormhello)
asio::awaitable<std::string> testormhello(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto users       = orm::cms::Sysuser();
    try
    {
        unsigned int n = co_await users.eqName("admin").async_exec_fetch_one();
        if (n > 0)
        {
            peer->output = "<p>found:";
            peer->output.append(users.data.name);
            peer->output.append("</p>");
        }
        else
        {
            peer->output = "<p>Not found</p>";
        }
    }
    catch (std::exception &e)
    {
        client << "<p>" << e.what() << "</p>";
    }
    co_return "";
}
}// namespace http
