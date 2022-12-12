#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testhello.h"
namespace http
{

      std::string testjson(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client.val["msg"]="insert 3 row";
            client.val["status"]="ok,insert";
            client.val["num"]=1234;
            client.out_json();
            return "";
      }

}