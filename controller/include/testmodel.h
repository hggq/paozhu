#pragma once
#include <chrono>
#include <thread>
#include "httppeer.h"
#include "orm.h"
namespace http
{

      std::string testmodel(std::shared_ptr<httppeer> peer);
}