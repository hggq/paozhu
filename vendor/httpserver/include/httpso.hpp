#ifndef HTTP_CONTROL_HPP
#define HTTP_CONTROL_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include "pz_dll.h"
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
#if defined(ENABLE_VIEWSO) || defined(ENABLE_MODELSO)
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

server_loaclvar &get_server_global_var()
{
    return clientapi::get().server_global_var();
}
#ifndef _CONTROL_DESTROY
void _destroy()
{
}
#endif

PZ_DLL_ALIAS(http::clientapi::setclientapi, _setclientapi)
}// namespace http
#endif// ENABLE_VIEWSO || ENABLE_MODELSO
#define _SHOW(A) PZ_DLL_ALIAS(http::A, A)
#define _SHOWS(A, B) PZ_DLL_ALIAS(http::A, B)
#endif