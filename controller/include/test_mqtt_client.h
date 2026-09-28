
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	asio::awaitable<std::string> test_mqtt_client(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_mqtt_tick_push(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_mqtt_tick_push_co(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> start_mqtt_loop(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> stop_mqtt_loop(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> list_mqtt_loop(std::shared_ptr<httppeer> peer);
}
