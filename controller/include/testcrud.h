#pragma once
#include <string>
#include <thread>
#include "httppeer.h"

namespace http
{

   std::string articlelogin(std::shared_ptr<httppeer> peer);
   std::string articleloginpost(std::shared_ptr<httppeer> peer);
   std::string articlelist(std::shared_ptr<httppeer> peer);
   std::string articleshow(std::shared_ptr<httppeer> peer);

   std::string articleedit(std::shared_ptr<httppeer> peer);


   std::string articleeditpost(std::shared_ptr<httppeer> peer);


   std::string articleadd(std::shared_ptr<httppeer> peer);

   std::string articleaddpost(std::shared_ptr<httppeer> peer);

   std::string articledelete(std::shared_ptr<httppeer> peer);
 
}