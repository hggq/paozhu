
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string testpinyin(std::shared_ptr<httppeer> peer);
	std::string testpinyin_loaded(std::shared_ptr<httppeer> peer);
}
