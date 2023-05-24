#ifndef __SERVER_LOCAL_VAR_H__
#define __SERVER_LOCAL_VAR_H__

#include <iostream>
#include <string>
#include <vector>
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
    bool http2_enable        = true;
    bool debug_enable        = false;
    bool deamon_enable       = true;
    bool show_visit_info     = false;
    bool is_alias_uploadpath = false;
    bool is_alias_urlpath    = false;

    bool static_file_compress_cache = false;
    unsigned char session_type      = 0;
    std::string reboot_password;
    std::string www_path;
    std::string config_path;
    std::map<std::string, std::map<std::string, std::string>> map_value;
    std::map<std::string, std::string> map_kv;
};
server_loaclvar &get_server_global_var();
unsigned int get_server_siteinfo_index(const std::string &);
std::vector<server_loaclvar> &get_server_siteinfo_var();
} //namespace http
#endif
