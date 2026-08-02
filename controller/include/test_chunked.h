
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_chunked(std::shared_ptr<httppeer> peer);
	std::string test_chunkedfile(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_chunked_async(std::shared_ptr<httppeer> peer);
}
