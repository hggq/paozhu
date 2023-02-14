#include <chrono>
#include <thread>
#include "qrcode.h"
#include "httppeer.h"
#include "testsessionid.h"
namespace http
{
      //@urlpath(null,testsetsession)
      std::string testsetsession(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << "hello world!  add cookie ";
            client << client.get_hosturl();

            int semid = client.get["aaa"].to_int();
            if (semid != 0)
            {
                client.session["aaa"] = semid;  
                client << "<br />set session 2222 ";
            }
            else
            {
               client.session["aaa"] = 1111;   
            }
            
            client.save_session();
            client << "<p><a href=\"" << client.get_hosturl() << "/testsetsession?aaa=2222\">2222 session</a></p>";    
            client.set_cookie("ccbb", "123456abc", 7200 * 24);
            client.set_cookie("same", "samesite", 7200 * 24, "/", client.host, false, true, "Lax");
            client.set_header("server", "cpphttp");
            client << "<p><a href=\"" << client.get_hosturl() << "/testshowsession\">show</a></p>";
            client << "<p><a href=\"" << client.get_hosturl() << "/testshowsession?sessionid=" << client.get_session_id() << "\">session(" << client.get_session_id() << ")</a></p>";
            return "";
      }
      //@urlpath(null,testshowsession)
      std::string testshowsession(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << "hello world!  show cookie<br/>";
            client << "<p><a href=\"" << client.get_hosturl() << "/testsetsession\">add cookie</a></p>";
            client << "<p>session aaa:|" << client.session["aaa"].to_string() << "|</p>";
            client << "<p>session_id:|" << client.get_session_id()<< "|</p>";

            client << "<p>cookie same:" << client.cookie["same"] << "</p>";
            client << "<p>cookie ccbb:" << client.cookie["ccbb"] << "</p>";

            std::string semid = client.get["sessionid"].as_string();
            if (!semid.empty())
            {
                  client.set_session_id(semid);
                  client << "<p>new session_id:|" << client.get_session_id()<< "|</p>";
                  client << "<p>new session aaa:|" << client.session["aaa"] << "|</p>";
                  client << "<p>cookie same:" << client.cookie["same"] << "</p>";
                  client << "<p>cookie ccbb:" << client.cookie["ccbb"] << "</p>";
            }
            client.view("cookie/show");
            return "";
      }

}