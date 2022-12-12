#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

      std::string testjson(std::shared_ptr<httppeer> peer);
}