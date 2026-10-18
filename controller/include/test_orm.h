
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> testormhello(std::shared_ptr<httppeer> peer);
}
