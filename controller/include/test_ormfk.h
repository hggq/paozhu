
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_ormfk_one(std::shared_ptr<httppeer> peer);
	std::string test_ormfk_many(std::shared_ptr<httppeer> peer);
	std::string test_ormfk_join(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_ormfk_co(std::shared_ptr<httppeer> peer);
}
