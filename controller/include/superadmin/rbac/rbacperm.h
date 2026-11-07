
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string superadmin_rbac_perm_list(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_perm_editpost(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_perm_toggle(std::shared_ptr<httppeer> peer);
}
