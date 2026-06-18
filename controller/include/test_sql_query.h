
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testsqlquery(std::shared_ptr<httppeer> peer);
}
