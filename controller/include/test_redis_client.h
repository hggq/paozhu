
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_redis_parse(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_redis_pool(std::shared_ptr<httppeer> peer);
	std::string test_redis_sync(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_redis_pubsub(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_redis_hardfix(std::shared_ptr<httppeer> peer);
}
