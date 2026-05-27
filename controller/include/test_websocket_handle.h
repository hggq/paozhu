
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> test_websocket_client(std::shared_ptr<httppeer> peer);
}
