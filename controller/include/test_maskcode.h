
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_mask_pre(std::shared_ptr<httppeer> peer);
	std::string test_maskcode(std::shared_ptr<httppeer> peer);
	std::string test_maskcodehas(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_comaskprehas(std::shared_ptr<httppeer> peer);
}
