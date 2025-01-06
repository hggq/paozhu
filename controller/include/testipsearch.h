
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testipsearch(std::shared_ptr<httppeer> peer);
	std::string testipsearchget(std::shared_ptr<httppeer> peer);
}
