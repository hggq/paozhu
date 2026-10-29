
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string github_sync(std::shared_ptr<httppeer> peer);
	std::string github_sync_arm(std::shared_ptr<httppeer> peer);
}
