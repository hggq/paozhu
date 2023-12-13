#ifndef HTTP_CONTROL_HPP
#define HTTP_CONTROL_HPP
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll/alias.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>
#include <string_view>
#include <memory>
#include <map>
#include <thread>
#include <vector>
#include <ctime>
#include <mutex>
#include <array>
#include <condition_variable>
#include <sys/stat.h>
#include "mysql.h"
#include <cstdlib>
#include "request.h"
#include "md5.h"
#include "httppeer.h"
#include "http_so_common_api.h"

namespace http
{
unsigned int get_controlversion()
{
    return 0x01000000;
}

std::unique_ptr<MYSQL, decltype(&mysql_close)> get_mysqlselectexecute(size_t dbhash)
{
    return clientapi::get().api_mysqlselect(dbhash);
}
std::unique_ptr<MYSQL, decltype(&mysql_close)> get_mysqleditexecute(size_t dbhash)
{
    return clientapi::get().api_mysqledit(dbhash);
}
bool back_mysql_connect(size_t dbhash, std::unique_ptr<MYSQL, decltype(&mysql_close)> conn)
{
    return clientapi::get().api_mysql_back_conn(dbhash, std::move(conn));
}
server_loaclvar &get_server_global_var()
{
    return clientapi::get().server_global_var();
}
#ifndef _CONTROL_DESTROY
void _destroy()
{
}
#endif

BOOST_DLL_ALIAS(http::clientapi::setclientapi, _setclientapi)
#define _SHOW(A) BOOST_DLL_ALIAS(http::A, A)
#define _SHOWS(A, B) BOOST_DLL_ALIAS(http::A, B)
}// namespace http
#endif