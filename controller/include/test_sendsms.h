
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testsendsms(std::shared_ptr<httppeer> peer);
	std::string testtencentsms(std::shared_ptr<httppeer> peer);
}
