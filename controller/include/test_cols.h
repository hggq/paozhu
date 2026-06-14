
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_cols(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_cols_co(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_leftjoin(std::shared_ptr<httppeer> peer);
}
