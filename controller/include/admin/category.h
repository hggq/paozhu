
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string admin_marcatalogue(std::shared_ptr<httppeer> peer);
	std::string admin_addcataloguepost(std::shared_ptr<httppeer> peer);
	std::string admin_editcataloguepost(std::shared_ptr<httppeer> peer);
	std::string admin_deletecatalogue(std::shared_ptr<httppeer> peer);
}
