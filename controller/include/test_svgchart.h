
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_svgchart(std::shared_ptr<httppeer> peer);
	std::string test_svgstats(std::shared_ptr<httppeer> peer);
}
