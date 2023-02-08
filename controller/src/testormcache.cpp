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

            articles.where("isopen=1").where(" aid=", aid).limit(1).use_cache().fetch();
            client << "<br />title:" << articles.getTitle();
            if(articles.isuse_cache())
            {
                  articles.save_cache(60); //cache 60 second
                  client << "<p>  this visit to cache </p>";
            }      
            return "";
      }

      //@urlpath(null,testormcacheb)
      std::string testormcacheb(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << " test orm cache B style ";
            auto articles = orm::cms::Article();
            int aid = 5;

            articles.where("isopen=1").where(" aid=", aid).limit(1).use_cache(60).fetch();
            client << "<br />title:" << articles.getTitle();
            if(articles.isuse_cache(true))
            {
                  //is has cache data
                  client << "<p>  this visit to cache </p>";
            }      
            return "";
      }

       //@urlpath(null,testormcachec)
      std::string testormcachec(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << " test orm cache fetchRow style ";
            auto articles = orm::cms::Article();
            int aid = 5;

            auto [colnames,colnamemaps,vallists]=articles.where("isopen=1").where(" aid=", aid).limit(1).use_cache(60).fetch_row();

            client << "<br />Record size:" << std::to_string(vallists.size());  
            client << "<br />Colname size:" << std::to_string(colnames.size());  
            
            if(vallists.size()>0)
            {
                if(vallists[0].size()>4)
                {
                  client << "<br />title:" << vallists[0][4];  
                }
                
            }
            for(std::size_t i=0;i<vallists.size();i++)
            {
               for(std::size_t j=0;j<colnames.size();j++)
               {
                  if(j<vallists[i].size())
                  {
                        client <<"<p>"<<colnames[j]<<":"<< vallists[i][j]<<"</p>";
                  }
                  if(j>3)
                  {
                        break;
                  }
               }   
               
            }
            
            if(articles.isuse_cache(true))
            {
                  //is has cache data
                  client << "<p>  this visit to cache </p>";
            }      
            return "";
      }
}
