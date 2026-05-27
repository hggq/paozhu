
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> test_rpcclient(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_rpcclientssl(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_rpcserver(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_rpc_chunkc(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_rpc_chunks(std::shared_ptr<httppeer> peer);
}
