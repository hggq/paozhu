
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testhttpclient_get_body(std::shared_ptr<httppeer> peer);
	std::string testhttpclient_get_timebody(std::shared_ptr<httppeer> peer);
	std::string testhttpclient_get_file(std::shared_ptr<httppeer> peer);
}
