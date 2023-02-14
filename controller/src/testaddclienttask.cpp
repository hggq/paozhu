#include "orm.h"
#include <chrono>
#include <thread>
#include "func.h"
#include "httppeer.h"
#include "testaddclienttask.h"

namespace http
{
   //@urlpath(null,testnotaddclienttaskpre)
   std::string testaddclienttaskpre(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      client << " testaddclienttaskpre <br/> ";
      return "ok";
   }
   // testaddclienttask must has pre method
   //@urlpath(testaddclienttaskpre,testaddclienttask)
   std::string testaddclienttask(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      client << " Interval task test <br/> ";
      client << client.get_hosturl();
      peer->add_timeloop_task("executeclienttask", 5); //(regfunc,second)
      return "T";
   }
   //@urlpath(null,executeclienttask)
   std::string testexecuteclienttask(std::shared_ptr<httppeer> peer)
   {
      httppeer &client = peer->getpeer();
      auto users = orm::cms::User();

      try
      {
         users.where("name", "admin").limit(1).fetch();
         if (users.getUserid() > 0)
         {
            // not output
            users.update_col("level", 1);
            peer->add_timeloop_count();
            if (peer->get_timeloop_count() > 5)
            {
               peer->clear_timeloop_task();
            }
            return "";
         }
         else
         {
            return "";
         }
      }
      catch (std::exception &e)
      {
         return "";
      }
      return "";
   }

}