
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string rbac_check(std::shared_ptr<httppeer> peer);
}
