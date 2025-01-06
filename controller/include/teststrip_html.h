
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string teststrip_html(std::shared_ptr<httppeer> peer);
}
