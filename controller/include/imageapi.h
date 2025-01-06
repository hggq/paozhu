
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string imageapi_gateway(std::shared_ptr<httppeer> peer);
	std::string imageapi_upload(std::shared_ptr<httppeer> peer);
}
