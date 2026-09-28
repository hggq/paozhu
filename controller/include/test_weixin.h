
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string test_getopenid(std::shared_ptr<httppeer> peer);
	std::string test_weixinpay(std::shared_ptr<httppeer> peer);
	std::string test_weixin_native(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_xcxnotify(std::shared_ptr<httppeer> peer);
	std::string test_xcxgetphone(std::shared_ptr<httppeer> peer);
	std::string test_weixin_order_new(std::shared_ptr<httppeer> peer);
	std::string test_weixin_order_list(std::shared_ptr<httppeer> peer);
	std::string test_weixin_order_detail(std::shared_ptr<httppeer> peer);
	std::string test_weixin_refund(std::shared_ptr<httppeer> peer);
	std::string test_weixin_refund_query(std::shared_ptr<httppeer> peer);
}
