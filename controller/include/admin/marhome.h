
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string admin_marhome(std::shared_ptr<httppeer> peer);
	std::string admin_edithomeblockpic(std::shared_ptr<httppeer> peer);
	std::string admin_updatehomeblocksort(std::shared_ptr<httppeer> peer);
	std::string admin_addhomepic(std::shared_ptr<httppeer> peer);
	std::string admin_addhometext(std::shared_ptr<httppeer> peer);
	std::string admin_addhometopic(std::shared_ptr<httppeer> peer);
	std::string admin_addhomecontent(std::shared_ptr<httppeer> peer);
	std::string admin_edithomeblockcontent(std::shared_ptr<httppeer> peer);
	std::string admin_edithomeblocktopic(std::shared_ptr<httppeer> peer);
	std::string admin_edithomeblockpost(std::shared_ptr<httppeer> peer);
	std::string admin_addhomeblockpost(std::shared_ptr<httppeer> peer);
	std::string admin_addhomehot(std::shared_ptr<httppeer> peer);
	std::string admin_edithomeblockhot(std::shared_ptr<httppeer> peer);
	std::string admin_edithomeblocktext(std::shared_ptr<httppeer> peer);
	std::string admin_deletehomeblock(std::shared_ptr<httppeer> peer);
	std::string admin_homedesign(std::shared_ptr<httppeer> peer);
	std::string admin_edithomeblockmulit(std::shared_ptr<httppeer> peer);
}
