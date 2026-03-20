
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_getopenid(std::shared_ptr<httppeer> peer);
	std::string test_weixinpay(std::shared_ptr<httppeer> peer);
	std::string test_xcxnotify(std::shared_ptr<httppeer> peer);
	std::string test_xcxgetphone(std::shared_ptr<httppeer> peer);
}
