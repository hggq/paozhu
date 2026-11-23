
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> arena_echo(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> arena_spec(std::shared_ptr<httppeer> peer);
}
