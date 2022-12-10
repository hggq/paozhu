#ifndef PROJECT_FUNC_H
#define PROJECT_FUNC_H

#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <memory>
#include <string_view>
#include "httppeer.h"

namespace http
{   
    extern std::map<std::string,bool> _block_ip_tables;
    extern std::map<std::string,bool> _block_host_tables;
    bool check_blockip(const std::string &);
    bool hook_host_http1(std::shared_ptr<httppeer> peer);
    bool hook_host_http2(std::shared_ptr<httppeer> peer);
}
#endif