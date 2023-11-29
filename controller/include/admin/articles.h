
#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"

namespace http
{

            
	std::string admin_addarticle(std::shared_ptr<httppeer> peer);
	std::string admin_addarticlepost(std::shared_ptr<httppeer> peer);
	std::string admin_editarticle(std::shared_ptr<httppeer> peer);
	std::string admin_editarticlepost(std::shared_ptr<httppeer> peer);
	std::string admin_deletearticle(std::shared_ptr<httppeer> peer);
	std::string admin_gettoparticle(std::shared_ptr<httppeer> peer);
	std::string admin_updatearticlesort(std::shared_ptr<httppeer> peer);
	std::string admin_listarticle(std::shared_ptr<httppeer> peer);
}
