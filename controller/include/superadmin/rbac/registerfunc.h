
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string superadmin_registerfunc(std::shared_ptr<httppeer> peer);
	std::string superadmin_registerfunc_save(std::shared_ptr<httppeer> peer);
}
