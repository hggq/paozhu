
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string testhttpclient_cowait_php(std::shared_ptr<httppeer> peer);
	std::string testhttpclient_cowait_body(std::shared_ptr<httppeer> peer);
	std::string testhttpclient_cowait_post(std::shared_ptr<httppeer> peer);
	std::string testhttpclient_cowait_urls(std::shared_ptr<httppeer> peer);
	std::string testhttpclient_cowait_spawn(std::shared_ptr<httppeer> peer);
	std::string testhttpclient_get_range(std::shared_ptr<httppeer> peer);
}
