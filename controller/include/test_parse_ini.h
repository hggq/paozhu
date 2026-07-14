
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_parse_ini(std::shared_ptr<httppeer> peer);
	std::string test_parse_fix(std::shared_ptr<httppeer> peer);
	std::string test_fix_server_conf(std::shared_ptr<httppeer> peer);
	std::string test_fix_orm_conf(std::shared_ptr<httppeer> peer);
}
