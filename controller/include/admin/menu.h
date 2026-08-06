
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string admin_menu(std::shared_ptr<httppeer> peer);
	std::string admin_vuetest(std::shared_ptr<httppeer> peer);
	std::string admin_dashboard(std::shared_ptr<httppeer> peer);
	std::string admin_jstimetest(std::shared_ptr<httppeer> peer);
	std::string admin_testelementplus(std::shared_ptr<httppeer> peer);
	std::string admin_testgetjson(std::shared_ptr<httppeer> peer);
}
