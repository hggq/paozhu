
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
 namespace saas { 
	asio::awaitable<std::string> test_co_handle(std::shared_ptr<httppeer> peer);
 }
}
