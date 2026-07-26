
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_wxpay_native(std::shared_ptr<httppeer> peer);
	std::string test_wxpay_jsapi(std::shared_ptr<httppeer> peer);
	std::string test_wxpay_app(std::shared_ptr<httppeer> peer);
	std::string test_wxpay_h5(std::shared_ptr<httppeer> peer);
	std::string test_wxpay_query(std::shared_ptr<httppeer> peer);
	std::string test_wxpay_close(std::shared_ptr<httppeer> peer);
	std::string test_wxpay_refund(std::shared_ptr<httppeer> peer);
	std::string test_wxpay_notify(std::shared_ptr<httppeer> peer);
	std::string test_wxpay_download_cert(std::shared_ptr<httppeer> peer);
}
