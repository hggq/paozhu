
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testaddclienttaskpre(std::shared_ptr<httppeer> peer);
	std::string testaddclienttask(std::shared_ptr<httppeer> peer);
	std::string testexecuteclienttask(std::shared_ptr<httppeer> peer);
}
