
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testmodelfromjson(std::shared_ptr<httppeer> peer);
}
