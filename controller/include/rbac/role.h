
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string rbac_role_list(std::shared_ptr<httppeer> peer);
	std::string rbac_role_add(std::shared_ptr<httppeer> peer);
	std::string rbac_role_edit(std::shared_ptr<httppeer> peer);
	std::string rbac_role_delete(std::shared_ptr<httppeer> peer);
	std::string rbac_roleperm_list(std::shared_ptr<httppeer> peer);
	std::string rbac_roleperm_bind(std::shared_ptr<httppeer> peer);
	std::string rbac_roleperm_unbind(std::shared_ptr<httppeer> peer);
}
