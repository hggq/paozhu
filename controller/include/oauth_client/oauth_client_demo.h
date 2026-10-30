
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string oauth_client_page(std::shared_ptr<httppeer> peer);
	std::string oauth_client_cc(std::shared_ptr<httppeer> peer);
	std::string oauth_client_build_auth(std::shared_ptr<httppeer> peer);
	std::string oauth_client_exchange(std::shared_ptr<httppeer> peer);
	std::string oauth_client_refresh(std::shared_ptr<httppeer> peer);
	std::string oauth_client_introspect(std::shared_ptr<httppeer> peer);
	std::string oauth_client_revoke(std::shared_ptr<httppeer> peer);
}
