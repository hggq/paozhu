
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_pzpng(std::shared_ptr<httppeer> peer);
	std::string test_outpng(std::shared_ptr<httppeer> peer);
}
