#ifndef PZ_HTTPSOCOMMON_API_HPP
#define PZ_HTTPSOCOMMON_API_HPP

#if defined(ENABLE_VIEWSO) || defined(ENABLE_MODELSO)
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include "pz_dll.h"
#include "request.h"
#include "server_localvar.h"
#include "viewso_param.h"
#include "httppeer.h"

using namespace http;
namespace http
{

class PZ_SYMBOL_VISIBLE clientapi
{
  public:
    clientapi() {}
    typedef std::function<std::string(std::string)> http_callback_t;// header("");getheader setcookie() cookie
    typedef std::function<void(std::string)> echo_callback_t;       // header("");getheader setcookie() cookie
    typedef std::function<void(std::string &)> echo_callbackand_t;
    typedef std::function<int(const char *, int)> webscoket_callback_t;// websocket send

    typedef std::function<std::function<std::string(struct view_param, http::obj_val &)>(std::string)> viewmethod_callback_t;
    typedef std::function<std::function<std::string(std::shared_ptr<http::httppeer> peer)>(const std::string &, const std::string &)> modulemethod_callback_t;

    typedef std::map<std::string, std::map<std::string, std::string>> map_value_call;

    viewmethod_callback_t api_loadview;

    modulemethod_callback_t api_loadcontrol;

    map_value_call map_value;
    std::function<http::server_loaclvar &()> server_global_var;

    static clientapi *instance()
    {
        if (singleton == NULL)
            singleton = new clientapi();
        return singleton;
    }
    static clientapi &get()
    {
        if (singleton == NULL)
            singleton = new clientapi();
        return *singleton;
    }
    static clientapi *singleton;
    static clientapi *setclientapi(clientapi *p)
    {
        singleton = p;
        return singleton;
    }
};

}// namespace http
#endif// ENABLE_VIEWSO || ENABLE_MODELSO
#endif
