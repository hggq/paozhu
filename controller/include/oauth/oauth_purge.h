
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string oauth_purge(std::shared_ptr<httppeer> peer);
	std::string oauth_purge_arm(std::shared_ptr<httppeer> peer);
}
