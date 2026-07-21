
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_pzjpg(std::shared_ptr<httppeer> peer);
	std::string test_outjpg(std::shared_ptr<httppeer> peer);
}
