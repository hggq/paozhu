
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testhello(std::shared_ptr<httppeer> peer);
	std::string testhellobusy(std::shared_ptr<httppeer> peer);
}
