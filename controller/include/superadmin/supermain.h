
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string superadmin_login(std::shared_ptr<httppeer> peer);
	std::string superadmin_loginpost(std::shared_ptr<httppeer> peer);
	std::string superadmin_logout(std::shared_ptr<httppeer> peer);
	std::string superadmin_islogin(std::shared_ptr<httppeer> peer);
	std::string superadmin_isloginjson(std::shared_ptr<httppeer> peer);
	std::string superadmin_main(std::shared_ptr<httppeer> peer);
	std::string superadmin_editsiteinfo(std::shared_ptr<httppeer> peer);
	std::string superadmin_editsiteinfopost(std::shared_ptr<httppeer> peer);
	std::string superadmin_deletesiteinfo(std::shared_ptr<httppeer> peer);
	std::string superadmin_deletesiteinfopost(std::shared_ptr<httppeer> peer);
	std::string superadmin_addsiteinfo(std::shared_ptr<httppeer> peer);
	std::string superadmin_addsiteinfopost(std::shared_ptr<httppeer> peer);
	std::string superadmin_welcome(std::shared_ptr<httppeer> peer);
	std::string superadmin_listuser(std::shared_ptr<httppeer> peer);
	std::string superadmin_edituser(std::shared_ptr<httppeer> peer);
	std::string superadmin_adduser(std::shared_ptr<httppeer> peer);
	std::string superadmin_adduserpost(std::shared_ptr<httppeer> peer);
	std::string superadmin_edituserpost(std::shared_ptr<httppeer> peer);
	std::string superadmin_deleteuser(std::shared_ptr<httppeer> peer);
	std::string superadmin_userinfo(std::shared_ptr<httppeer> peer);
	std::string superadmin_editpassword(std::shared_ptr<httppeer> peer);
	std::string superadmin_editpwdpost(std::shared_ptr<httppeer> peer);
}
