
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string oauth_introspect(std::shared_ptr<httppeer> peer);
}
