
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> techempowerplaintext(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> techempowerjson(std::shared_ptr<httppeer> peer);
	std::string techempowerdb(std::shared_ptr<httppeer> peer);
	std::string techempowerqueries(std::shared_ptr<httppeer> peer);
	std::string techempowerfortunes(std::shared_ptr<httppeer> peer);
	std::string techempowerupdates(std::shared_ptr<httppeer> peer);
	std::string techempowercached_queries(std::shared_ptr<httppeer> peer);
	std::string techempowercached_db(std::shared_ptr<httppeer> peer);
}
