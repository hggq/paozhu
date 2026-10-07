
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_redis_bench_sync(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_redis_bench_async(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_redis_bench_async_concurrent(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_redis_bench_pool(std::shared_ptr<httppeer> peer);
	std::string test_redis_bench_direct(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_ioc_probe(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_async_profile(std::shared_ptr<httppeer> peer);
}
