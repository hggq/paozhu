
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testcmake(std::shared_ptr<httppeer> peer);
	std::string testcauto(std::shared_ptr<httppeer> peer);
}
