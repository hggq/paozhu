
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string superadmin_rbac_user_list(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_user_add(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_user_addpost(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_user_edit(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_user_editpost(std::shared_ptr<httppeer> peer);
	std::string superadmin_rbac_user_toggle(std::shared_ptr<httppeer> peer);
}
