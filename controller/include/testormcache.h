
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string testormcache(std::shared_ptr<httppeer> peer);
	std::string testormcacheb(std::shared_ptr<httppeer> peer);
	std::string testormcachec(std::shared_ptr<httppeer> peer);
}
