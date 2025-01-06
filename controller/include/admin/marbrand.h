
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string admin_marbrand(std::shared_ptr<httppeer> peer);
	std::string admin_brandaddpost(std::shared_ptr<httppeer> peer);
	std::string admin_deletebrand(std::shared_ptr<httppeer> peer);
	std::string admin_updatebrandsort(std::shared_ptr<httppeer> peer);
}
