#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testrand.h"
namespace http
{
      //@urlpath(null,testrand)
      std::string testrand(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << " Number random number and character random generation 随机数测试 ";
            client << "<p>100,999: " << rand_range(100, 999) << "</p>";
            client << "<p>1-0 A-Z a-z: " << rand_string(8, 0) << "</p>"; //(length,type )  1-0 A-Z a-z
            client << "<p>1-0 A-Z:  " << rand_string(8, 1) << "</p>";    //(length,type )  1-0 A-Z
            client << "<p>A-Z:  " << rand_string(8, 2) << "</p>";        //(length,type )  A-Z
            client << "<p>a-z:  " << rand_string(8, 3) << "</p>";        //(length,type )  a-z
            return "";
      }

}