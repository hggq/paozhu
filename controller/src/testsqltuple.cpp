#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "testsqltuple.h"

namespace http
{
   //@urlpath(null,mtuple)
   std::string testsqltuple(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      client << "hello world!  testsqltuple ";
 
      try
      {
         auto users = orm::cms::User();

         auto [key_names,lists]=users.query("SELECT * FROM `user` ");
         client << "<p>--------------</p>";
         client << users.error_msg;
         if(key_names.size()>0)
         {
            for(int i=0;i<lists.size();i++)
            {
               client <<key_names[1]<<":"<< lists[i][1];
               client <<key_names[2]<<":"<< lists[i][2];
            }
         }

         client << "<p>--------------</p>";

         users.clear();
         auto [colnames,vallists]=users.where("name","admin").limit(1).fetchRow();

         client << "<p>--------------</p>";
         client << users.error_msg;
         if(colnames.size()>0)
         {
            for(int i=0;i<vallists.size();i++)
            {
               client <<colnames[1]<<":"<< vallists[i][1];
               client <<colnames[2]<<":"<< vallists[i][2];
            }
         }
         client << "<p>--------------</p>";

      }
      catch (std::exception &e)
      {
         client << "<p>" << e.what() << "</p>";
         return "";
      }
      return "";
   }
  
}