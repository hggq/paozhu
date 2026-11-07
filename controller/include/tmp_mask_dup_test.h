
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string tmp_mask_dup_a(std::shared_ptr<httppeer> peer);
	std::string tmp_mask_dup_b(std::shared_ptr<httppeer> peer);
	std::string tmp_mask_dup_c(std::shared_ptr<httppeer> peer);
}
