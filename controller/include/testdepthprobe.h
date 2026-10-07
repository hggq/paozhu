
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testdpshadowshort(std::shared_ptr<httppeer> peer);
	std::string testdpshadowexact(std::shared_ptr<httppeer> peer);
	std::string testdpprobedepth6(std::shared_ptr<httppeer> peer);
	std::string testdpprobedepth7(std::shared_ptr<httppeer> peer);
	std::string testdpuserinfo2(std::shared_ptr<httppeer> peer);
}
