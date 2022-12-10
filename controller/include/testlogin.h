#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

      std::string testlogin(std::shared_ptr<httppeer> peer);

      std::string testshowlogin(std::shared_ptr<httppeer> peer);

      std::string testloginpost(std::shared_ptr<httppeer> peer);

}