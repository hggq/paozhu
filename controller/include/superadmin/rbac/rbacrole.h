
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string superadmin_rbac_role_list(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_role_addpost(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_role_editpost(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_role_delete(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_roleperm(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_roleperm_save(std::shared_ptr<httppeer> peer);
}
