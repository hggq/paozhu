
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string rbac_sysuser_page(std::shared_ptr<httppeer> peer);
	std::string rbac_sysuser_add(std::shared_ptr<httppeer> peer);
	std::string rbac_sysuser_delete(std::shared_ptr<httppeer> peer);
}
