
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string testcors(std::shared_ptr<httppeer> peer);
	std::string testcorssimple(std::shared_ptr<httppeer> peer);
}
