#include <chrono>
#include <thread>
#include "orm.h"
//if open compile command -DCREATE_SO=on so model
#if defined CREATE_SO
#include "httpso.hpp"
#endif 
#include "httppeer.h"

/*
--compiler--begin--
--compiler--end--
*/

namespace http
{

      std::string viewso(std::shared_ptr<httppeer> peer)
      {
            //run paozhu_cli create this file to so, then browser visit so file nanme and methold.
            //e.g hhttps://www.xxx.com/testviewso/viewso
            httppeer &client = peer->getpeer();
            client << "hello world!  so视图测试 ";
            client << client.gethosturl();
            client << "<p><a href=\"" << client.gethosturl() << "/showcookie\">show</a></p>";
            auto users = orm::cms::User();

            users.where("name", "admin").limit(1).fetch();
            try
            {
                  client << "<p>sql result</p>";
                  // view orm create sql
                  client << "<p>sql:" << users.sqlstring << "</p>";
                  if (users.getUserid() > 0)
                  {
                        // save session,other page get  int userid= client.session["userid"].to_int();
                        client.session["aaa"] = users.getUserid();
                        client.save_session();
                        client << "<p>found:" << users.data.name << "</p>";
                        //return "";
                  }
                  else
                  {
                       // return "";
                  }
            }
            catch (std::exception &e)
            {
                  client << "<p>" << e.what() << "</p>";
                  return "";
            }

            peer->view("login/login");
            return "";
      }
      #if defined CREATE_SO
      _SHOW(viewso)
      #endif 
      
}