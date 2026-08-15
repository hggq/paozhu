
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string article_pg_list(std::shared_ptr<httppeer> peer);
	std::string article_pg_show(std::shared_ptr<httppeer> peer);
}
