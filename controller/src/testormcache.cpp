#include "orm.h"
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testormcache.h"
namespace http
{

      //@urlpath(null,testormcache)
      std::string testormcache(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << " test orm cache ";
            auto articles = orm::cms::Article();
            int aid = 5;

            articles.where("isopen=1").where(" aid=", aid).limit(1).get_cache().fetch();
            client << "title:" << articles.getTitle();
            if(articles.getAid()>0)
            {
                  articles.save_cache(60); //cache 60 second
            }      
            return "";
      }

}
