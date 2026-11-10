
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> test_ioc_affinity(std::shared_ptr<httppeer> peer);
}
