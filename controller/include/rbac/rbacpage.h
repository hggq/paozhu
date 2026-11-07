
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string rbac_page_role(std::shared_ptr<httppeer> peer);
	std::string rbac_page_roleperm(std::shared_ptr<httppeer> peer);
	std::string rbac_page_userrole(std::shared_ptr<httppeer> peer);
}
