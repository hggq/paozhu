
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string admin_login(std::shared_ptr<httppeer> peer);
	std::string admin_loginpost(std::shared_ptr<httppeer> peer);
	std::string admin_logout(std::shared_ptr<httppeer> peer);
	std::string admin_islogin(std::shared_ptr<httppeer> peer);
	std::string admin_isloginjson(std::shared_ptr<httppeer> peer);
	std::string admin_main(std::shared_ptr<httppeer> peer);
	std::string admin_welcome(std::shared_ptr<httppeer> peer);
	std::string admin_siteinfo(std::shared_ptr<httppeer> peer);
	std::string admin_siteinfopost(std::shared_ptr<httppeer> peer);
	std::string admin_footscript(std::shared_ptr<httppeer> peer);
	std::string admin_footscriptpost(std::shared_ptr<httppeer> peer);
	std::string admin_copyright(std::shared_ptr<httppeer> peer);
	std::string admin_copyrightpost(std::shared_ptr<httppeer> peer);
	std::string admin_sitelogo(std::shared_ptr<httppeer> peer);
	std::string admin_sitelogopost(std::shared_ptr<httppeer> peer);
	std::string admin_userinfo(std::shared_ptr<httppeer> peer);
	std::string admin_editpassword(std::shared_ptr<httppeer> peer);
	std::string admin_editpwdpost(std::shared_ptr<httppeer> peer);
}
