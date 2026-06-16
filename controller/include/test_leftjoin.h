
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> test_leftjoinlimit(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_leftjoinfull(std::shared_ptr<httppeer> peer);
}
