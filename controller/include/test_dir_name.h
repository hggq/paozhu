
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_dir_name(std::shared_ptr<httppeer> peer);
}
