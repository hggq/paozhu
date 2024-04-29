
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string testpzcache(std::shared_ptr<httppeer> peer);
	std::string testshowcache(std::shared_ptr<httppeer> peer);
}
