
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testrestfulpath(std::shared_ptr<httppeer> peer);
	std::string testrestfulprofilepath(std::shared_ptr<httppeer> peer);
}
