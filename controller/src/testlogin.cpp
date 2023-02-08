#include <chrono>
#include <thread>
#include "qrcode.h"
#include "httppeer.h"
#include "testlogin.h"
namespace http
{

      std::string testlogin(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            // client << " 必须登录 ";
            // client << client.gethosturl();
            // client<<"<p><a href=\""<<client.gethosturl()<<"/showcookie\">show</a></p>";

            if(client.session["aaa"].as_int()==0)
            {
                  client << " 必须登录 ";
                  client << client.get_hosturl();
                  client<<"<p><a href=\""<<client.get_hosturl()<<"/showlogin\">show</a></p>";
                  return "showlogin";
            }
            return "ok";
      }
      std::string testshowlogin(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            // client << " 必须登录 ";
            // client << client.gethosturl();
            // client<<"<p><a href=\""<<client.gethosturl()<<"/showcookie\">show</a></p>";
 
                  client << " 点击登录 ";
                  client << client.get_hosturl();
                  client<<"<p><a href=\""<<client.get_hosturl()<<"/loginpost?login=1\">show</a></p>";
                  client<<"<p><a href=\""<<client.get_hosturl()<<"/loginpost?login=0\">clear</a></p>";      
            return "";
      }
      std::string testloginpost(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            // client << " 必须登录 ";
            // client << client.gethosturl();
            // client<<"<p><a href=\""<<client.gethosturl()<<"/showcookie\">show</a></p>";
            if(client.get["login"].to_string()=="1")
            {
                   client.session["aaa"]=1111;
                  client.save_session();
                  peer->goto_url("/showcookie",3,"我要走了！");
            }
            else{
                   client.clear_session();
                   client << " 必须登录 ";
                  peer->goto_url("/showlogin",3,"必须登录！");
            }
 
      
            return "";
      }
}