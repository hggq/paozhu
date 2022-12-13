#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testmodel.h"
namespace http
{

      std::string testmodel(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << "testmodel ";
            auto users = orm::cms::User();
           auto artlists=users.gettoparticle(1);

            for(int i=0;i<artlists.size();i++)
            {     
                  client << "<p> title "<<artlists[i].title<<"</p>";      
            }
            return "";
      }

}