
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string pxapidepartmentsaddpost(std::shared_ptr<httppeer> peer);
	std::string pxapidepartmentseditpost(std::shared_ptr<httppeer> peer);
	std::string pxapidepartmentslist(std::shared_ptr<httppeer> peer);
	std::string pxapipagesdepartlist(std::shared_ptr<httppeer> peer);
}
