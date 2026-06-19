
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testsqljson(std::shared_ptr<httppeer> peer);
}
