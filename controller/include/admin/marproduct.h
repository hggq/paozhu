
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string admin_addproduct(std::shared_ptr<httppeer> peer);
	std::string admin_addproductpost(std::shared_ptr<httppeer> peer);
	std::string admin_editproductpost(std::shared_ptr<httppeer> peer);
	std::string admin_getcategorytopproduct(std::shared_ptr<httppeer> peer);
	std::string admin_editproduct(std::shared_ptr<httppeer> peer);
	std::string admin_listproduct(std::shared_ptr<httppeer> peer);
	std::string admin_marproductattach(std::shared_ptr<httppeer> peer);
	std::string admin_deleteproduct(std::shared_ptr<httppeer> peer);
	std::string admin_deleteproductajax(std::shared_ptr<httppeer> peer);
	std::string admin_updateproducthome(std::shared_ptr<httppeer> peer);
	std::string admin_updateproductstore(std::shared_ptr<httppeer> peer);
	std::string admin_updateproductview(std::shared_ptr<httppeer> peer);
	std::string admin_updateproductsort(std::shared_ptr<httppeer> peer);
}
