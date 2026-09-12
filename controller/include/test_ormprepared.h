
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_ormprepared_dsl(std::shared_ptr<httppeer> peer);
	std::string test_ormprepared_exec(std::shared_ptr<httppeer> peer);
	std::string test_ormprepared_write(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_ormprepared_co(std::shared_ptr<httppeer> peer);
}
