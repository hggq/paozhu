
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string oauth_token(std::shared_ptr<httppeer> peer);
}
