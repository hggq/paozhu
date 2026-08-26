
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string article_sqlite_list(std::shared_ptr<httppeer> peer);
	std::string article_sqlite_show(std::shared_ptr<httppeer> peer);
}
