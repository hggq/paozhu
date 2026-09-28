
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string testwxpaycert_islogin(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testwxpaycert_islogin_co(std::shared_ptr<httppeer> peer);
	std::string testwxpay(std::shared_ptr<httppeer> peer);
	std::string testwxpayv3jsapi(std::shared_ptr<httppeer> peer);
	std::string testwxpaydownloadcert(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testwxpaydownloadcert_co(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testwxpayv3native_co(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testwxpayv3jsapi_co(std::shared_ptr<httppeer> peer);
	std::string testwxpayv3_order_new(std::shared_ptr<httppeer> peer);
	std::string testwxpayv3_order_list(std::shared_ptr<httppeer> peer);
	std::string testwxpayv3_order_detail(std::shared_ptr<httppeer> peer);
	std::string testwxpayv3_refund(std::shared_ptr<httppeer> peer);
	std::string testwxpayv3_refund_query(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testwxpayv3_order_new_co(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testwxpayv3_order_query_co(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testwxpayv3_order_status(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> testwxpayv3_refund_co(std::shared_ptr<httppeer> peer);
	asio::awaitable<std::string> test_wxpay_v3_notify(std::shared_ptr<httppeer> peer);
}
