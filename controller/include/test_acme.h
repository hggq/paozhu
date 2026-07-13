
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_acme(std::shared_ptr<httppeer> peer);
}
