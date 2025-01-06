
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testsetsession(std::shared_ptr<httppeer> peer);
	std::string testshowsession(std::shared_ptr<httppeer> peer);
}
