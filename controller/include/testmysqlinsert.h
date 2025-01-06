
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testmysqlinsert(std::shared_ptr<httppeer> peer);
	std::string testmysqlpagebar(std::shared_ptr<httppeer> peer);
}
