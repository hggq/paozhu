#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "testormclient.h"

namespace http
{
//@urlpath(null,testormclient)
std::string testormclient(std::shared_ptr<httppeer> peer)
{
  httppeer &client = peer->getpeer();
  client << "hello world!  testormclient ";
  client << client.get_hosturl();

  try
  {
    auto dbclient = std::make_shared<orm::get_orm_client>("cms");

    auto [colname, results] = dbclient->query("show databases;");
    client << "<p>show databases;</p>";
    if (results.empty())
    {
      client << "<p>empty</p>";
    }
    else
    {
      for (unsigned int i = 0; i < results.size(); i++)
      {
        if (results[i].size() > 0)
        {
          client << "<p>" << (i + 1) << ". " << results[i][0] << "</p>";
        }
      }
    }
  }
  catch (std::exception &e)
  {
    client << "<p>" << e.what() << "</p>";
    return "";
  }
  return "";
}

} // namespace http