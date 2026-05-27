
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> test_socket_client(std::shared_ptr<httppeer> peer);
}
