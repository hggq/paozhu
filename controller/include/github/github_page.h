
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string github_list(std::shared_ptr<httppeer> peer);
}
