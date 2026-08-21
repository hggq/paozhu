
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testsendmaildo(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_co_sendmaildo(std::shared_ptr<httppeer> peer);
}
