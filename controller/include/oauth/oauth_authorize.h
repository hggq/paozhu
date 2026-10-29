
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string oauth_authorize(std::shared_ptr<httppeer> peer);
	std::string oauth_authorize_approve(std::shared_ptr<httppeer> peer);
}
