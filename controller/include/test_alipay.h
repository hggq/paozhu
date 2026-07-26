
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_alipay_app(std::shared_ptr<httppeer> peer);
	std::string test_alipay_qrcode(std::shared_ptr<httppeer> peer);
	std::string test_alipay_page(std::shared_ptr<httppeer> peer);
	std::string test_alipay_query(std::shared_ptr<httppeer> peer);
	std::string test_alipay_cancel(std::shared_ptr<httppeer> peer);
	std::string test_alipay_refund(std::shared_ptr<httppeer> peer);
	std::string test_alipay_notify(std::shared_ptr<httppeer> peer);
	std::string test_alipay_return(std::shared_ptr<httppeer> peer);
}
