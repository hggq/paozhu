#ifndef __SERVER_LOCAL_VAR_H__
#define __SERVER_LOCAL_VAR_H__

#include <iostream>
#include <string>
#include <map>
namespace http
{
   // 方便 so 调用  target so file call server var
   struct server_loaclvar
   {
      std::string server_path;
      std::string temp_path;
      std::string control_so_path;
      std::string view_so_path;
      std::string log_path;
      bool http2_enable = true;
      bool debug_enable = false;
      bool deamon_enable = true;
      bool show_visit_info = false;
      std::string www_path;
      std::string config_path;
      std::map<std::string, std::map<std::string, std::string>> map_value;
   };
   server_loaclvar &get_server_global_var();
}
#endif
