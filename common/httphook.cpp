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
     std::map<std::string,bool> _block_ip_tables;
     std::map<std::string,bool> _block_host_tables;
    bool check_blockip(const std::string &client_ip)
    {
        return false;
    }
    bool hook_host_http1(std::shared_ptr<httppeer> peer)
    {
        return false;
    }
    bool hook_host_http2(std::shared_ptr<httppeer> peer)
    {

        return false;
    }
}
