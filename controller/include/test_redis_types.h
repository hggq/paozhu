
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> test_redis_types(std::shared_ptr<httppeer> peer);
	std::string test_redis_types_sync(std::shared_ptr<httppeer> peer);
}
