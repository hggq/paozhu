
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_deepseek_api(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_deepseek_async(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_deepseek_sse(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_deepseek_chunk(std::shared_ptr<httppeer> peer);
}
