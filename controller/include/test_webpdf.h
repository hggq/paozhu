
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_webpdf(std::shared_ptr<httppeer> peer);
	std::string test_otfpdf(std::shared_ptr<httppeer> peer);
}
