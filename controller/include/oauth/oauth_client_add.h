
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string oauth_client_add(std::shared_ptr<httppeer> peer);
}
