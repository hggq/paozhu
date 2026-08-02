
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_httppeersend(std::shared_ptr<httppeer> peer);
	std::string test_httppeersend_text(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_httppeersend_async(std::shared_ptr<httppeer> peer);
}
