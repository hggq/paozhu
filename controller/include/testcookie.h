#pragma once
#include <string>
#include <thread>
#include "httppeer.h"
namespace http
{

      std::string testaddcookie(std::shared_ptr<httppeer> peer);
      std::string testshowcookie(std::shared_ptr<httppeer> peer);

}