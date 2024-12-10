
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string attachfile_gateway(std::shared_ptr<httppeer> peer);
	std::string attachimg_gateway(std::shared_ptr<httppeer> peer);
	std::string attachfile_delete(std::shared_ptr<httppeer> peer);
	std::string attachimg_delete(std::shared_ptr<httppeer> peer);
	std::string attachfile_upload(std::shared_ptr<httppeer> peer);
	std::string attachfile_uploadpost(std::shared_ptr<httppeer> peer);
	std::string admin_attachselectfiles(std::shared_ptr<httppeer> peer);
}
