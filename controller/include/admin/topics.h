
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{        
	std::string admin_addtopic(std::shared_ptr<httppeer> peer);
	std::string admin_edittopic(std::shared_ptr<httppeer> peer);
	std::string admin_martopic(std::shared_ptr<httppeer> peer);
	std::string admin_updatetopicsort(std::shared_ptr<httppeer> peer);
	std::string admin_addtopicpost(std::shared_ptr<httppeer> peer);
	std::string admin_deletetopic(std::shared_ptr<httppeer> peer);
	std::string admin_edittopicpost(std::shared_ptr<httppeer> peer);
	std::string admin_topicfileupload(std::shared_ptr<httppeer> peer);
	std::string admin_topicimgtextupload(std::shared_ptr<httppeer> peer);
	std::string admin_updatetopicview(std::shared_ptr<httppeer> peer);
	std::string admin_updatetopicsideblocksort(std::shared_ptr<httppeer> peer);
	std::string admin_edittopicside(std::shared_ptr<httppeer> peer);
	std::string admin_addtopicsidepick(std::shared_ptr<httppeer> peer);
	std::string admin_addtopicsidetext(std::shared_ptr<httppeer> peer);
	std::string admin_addtopicsideblockpost(std::shared_ptr<httppeer> peer);
	std::string admin_deletetopicsideblock(std::shared_ptr<httppeer> peer);
	std::string admin_edittopicsideblocktext(std::shared_ptr<httppeer> peer);
	std::string admin_edittopicsideblockpick(std::shared_ptr<httppeer> peer);
	std::string admin_edittopicsideblockpost(std::shared_ptr<httppeer> peer);
}
