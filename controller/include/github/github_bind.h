
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string github_bind(std::shared_ptr<httppeer> peer);
}
