#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testpzcache.h"
#include "pzcache.h"
namespace http
{

      //@urlpath(null,testcache)
      std::string testpzcache(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << " test pzcache ";
            pzcache<std::string> &temp_cache = pzcache<std::string>::conn();

            //one hashid
            std::string namestring = "testname";
            std::size_t cache_hashid = std::hash<std::string>{}(namestring);

            //two hashid 
            std::size_t num_hashid = 2244;

            if (temp_cache.check(cache_hashid) > -1)
            {
                  client << " data has cache <a href=\"/testshowcache\">show</a>";
            }
            else
            {
                  std::string cache_data = "This cache content!";
                  temp_cache.save(cache_hashid, cache_data, 30);

                  client << "<p> Visit this pzcache cache data1 </p>";
                  client << "<p><a href=\"/testshowcache\">show</a></p>";
            }

            if (temp_cache.check(num_hashid) > -1)
            {
                  client << " data has cache <a href=\"/testshowcache\">show</a>";
            }
            else
            {
                  std::string cache_data = "This cache hashid is number!";
                  temp_cache.save(num_hashid, cache_data,40);

                  client << "<p> Visit this pzcache cache data2 </p>";
                  client << "<p><a href=\"/testshowcache\">show</a></p>";
            }

            return "";
      }
      //@urlpath(null,testshowcache)
      std::string testshowcache(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << " test pzcache show";
            pzcache<std::string> &temp_cache = pzcache<std::string>::conn();

            std::string namestring = "testname";
            std::size_t cache_hashid = std::hash<std::string>{}(namestring);

            std::string cache_data = temp_cache.get(cache_hashid);

            client << " data1: <p>" << cache_data << "</p>";

            std::size_t num_hashid = 2244;

            cache_data = temp_cache.get(num_hashid);
            client << " data2: <p>" << cache_data << "</p>";

            client << "<p><a href=\"/testcache\">return</a></p>";
            return "";
      }
}
