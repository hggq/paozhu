
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testormcache(std::shared_ptr<httppeer> peer);
	std::string testormcacheb(std::shared_ptr<httppeer> peer);
	std::string testormcachec(std::shared_ptr<httppeer> peer);
	std::string testormcache_d(std::shared_ptr<httppeer> peer);
	std::string testormcache_d_invalidate(std::shared_ptr<httppeer> peer);
	std::string testormcache_e(std::shared_ptr<httppeer> peer);
}
