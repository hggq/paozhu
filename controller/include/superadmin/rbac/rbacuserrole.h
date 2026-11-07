
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string superadmin_rbac_userrole(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_userrole_save(std::shared_ptr<httppeer> peer);
}
