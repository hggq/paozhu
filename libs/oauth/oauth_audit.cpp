#include "oauth_audit.h"

#include <string>

#include "datetime.h"
#include "server.h"

namespace http
{
namespace oauth
{

void audit_log(const char *func, unsigned int line, const std::string &msg)
{
    std::string item;
    item.reserve(48 + msg.size());
    item = "[oauth] ";
    item += func;
    item.push_back(':');
    item += std::to_string(line);
    item.push_back(' ');
    item += msg;
    // 这条汇逐行原样落盘，flush 不给前缀，所以时刻自己带
    item += " ts=";
    item += std::to_string((long long)timeid());
    item.push_back('\n');
    get_server_app().add_error_lists(item);
}

}// namespace oauth
}// namespace http
