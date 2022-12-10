#include <chrono>
#include <thread>
#include "httppeer.h"
#include "testhello.h"
namespace http
{

      std::string testhello(std::shared_ptr<httppeer> peer)
      {
            httppeer &client = peer->getpeer();
            client << " Hello world! 🧨 Paozhu c++ web framework ";
            
            return "";
      }

}