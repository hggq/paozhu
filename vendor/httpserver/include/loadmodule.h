#ifndef __FRAME_LOADMODULE_H__
#define __FRAME_LOADMODULE_H__

#ifdef ENABLE_BOOST
#include <iostream>
#include <string>
#include <list>
#include <functional>
#include <boost/dll/alias.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>
#include "request.h"
#include "httppeer.h"
namespace http
{
typedef boost::function<void()> callback_t;
typedef boost::function<void(std::string)> echo_callback_t;
typedef boost::function<void(std::string &)> echo_callbackand_t;
typedef boost::function<bool(std::list<std::string> &, size_t)> mysql_callbacksql_rollback;
typedef boost::function<std::string(obj_val &)> method_callback_t;
typedef boost::function<std::string(std::shared_ptr<httppeer>)> www_method_call;
typedef boost::function<boost::function<std::string(obj_val &)>(std::string)> modulemethod_callback_t;
typedef boost::function<boost::function<std::string(std::shared_ptr<httppeer>)>(std::string)> module_method_call;

typedef boost::function<std::string(obj_val &)> method_callback_t;
typedef std::map<std::size_t, method_callback_t> so_cachepath_t;

std::string httpempty(obj_val &a);
method_callback_t viewmodulecreate(std::string module, std::string name);
method_callback_t loadview(std::string modulemethod);
www_method_call controlmodulecreate(std::string module, std::string name, size_t tt);
www_method_call loadcontrol(const std::string &modulemethod, const std::string &name);
void viewmoduleclear(std::string module, std::string method);
void controlmoduleclear(std::string module, std::string method);
so_cachepath_t &get_sharedpathchache();
std::map<std::size_t, www_method_call> &get_controlpathchache();
std::map<std::size_t, std::vector<std::string>> &get_sharedmethodchache();
}// namespace http
#endif
#endif
