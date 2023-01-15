
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string testurlencoded(std::shared_ptr<httppeer> peer);
	std::string testformmultipart(std::shared_ptr<httppeer> peer);
	std::string testformjsonpost(std::shared_ptr<httppeer> peer);
	std::string testformxmlpost(std::shared_ptr<httppeer> peer);
}
