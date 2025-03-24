#ifndef BOOST_DLL_HTTPSOCOMMON_API_HPP
#define BOOST_DLL_HTTPSOCOMMON_API_HPP

#ifdef ENABLE_BOOST
#include <functional>
#include <string>
#include <string_view>
#include <boost/function.hpp>
#include <memory>
#include <list>
#include "request.h"
#include "server_localvar.h"
#include "viewso_param.h"
#include "mysql.h"
#include "httppeer.h"

using namespace http;
namespace http
{

class BOOST_SYMBOL_VISIBLE clientapi
{
  public:
    clientapi() {}
    typedef boost::function<std::string(std::string)> http_callback_t;// header("");getheader setcookie() cookie
    typedef boost::function<void(std::string)> echo_callback_t;       // header("");getheader setcookie() cookie
    typedef boost::function<void(std::string &)> echo_callbackand_t;
    typedef boost::function<int(const char *, int)> webscoket_callback_t;// websocket send

    typedef boost::function<boost::function<std::string(struct view_param, http::obj_val &)>(std::string)> viewmethod_callback_t;
    typedef boost::function<boost::function<std::string(std::shared_ptr<http::httppeer> peer)>(const std::string &, const std::string &)> modulemethod_callback_t;

    typedef boost::function<std::unique_ptr<MYSQL, decltype(&mysql_close)>(size_t)> mysql_callbackand_t;
    typedef boost::function<bool(size_t, std::unique_ptr<MYSQL, decltype(&mysql_close)>)> mysql_back_conn_call;

    typedef std::map<std::string, std::map<std::string, std::string>> map_value_call;

    viewmethod_callback_t api_loadview;

    modulemethod_callback_t api_loadcontrol;

    mysql_callbackand_t api_mysqlselect;
    mysql_callbackand_t api_mysqledit;

    mysql_callbackand_t api_mysqlcommit;
    mysql_back_conn_call api_mysql_back_conn;

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
#endif
#endif