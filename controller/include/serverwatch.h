
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string paozhu_metrics(std::shared_ptr<httppeer> peer);
	std::string frametasks_timeloop(std::shared_ptr<httppeer> peer);
}
