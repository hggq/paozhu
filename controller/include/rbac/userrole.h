
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string rbac_userrole_list(std::shared_ptr<httppeer> peer);
	std::string rbac_userrole_save(std::shared_ptr<httppeer> peer);
}
