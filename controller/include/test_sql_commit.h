
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_sql_commit(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_co_sql_commit(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_co_sql_orm(std::shared_ptr<httppeer> peer);
}
