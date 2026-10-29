
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string github_quickbind(std::shared_ptr<httppeer> peer);
}
