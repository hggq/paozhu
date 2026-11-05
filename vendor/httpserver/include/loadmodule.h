#ifndef __FRAME_LOADMODULE_H__
#define __FRAME_LOADMODULE_H__

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "pz_dll.h"
#include "request.h"
#include "httppeer.h"
namespace http
{
typedef std::function<void()> callback_t;
typedef std::function<void(std::string)> echo_callback_t;
typedef std::function<void(std::string &)> echo_callbackand_t;
typedef std::function<bool(std::list<std::string> &, size_t)> mysql_callbacksql_rollback;
typedef std::function<std::function<std::string(obj_val &)>(std::string)> modulemethod_callback_t;
typedef std::function<std::function<std::string(std::shared_ptr<httppeer>)>(std::string)> module_method_call;

#ifdef ENABLE_VIEWSO
typedef std::function<std::string(obj_val &)> method_callback_t;
typedef std::map<std::size_t, method_callback_t> so_cachepath_t;

std::string httpempty(obj_val &a);
method_callback_t viewmodulecreate(std::string module, std::string name);
method_callback_t loadview(std::string modulemethod);
void viewmoduleclear(std::string module, std::string method);
so_cachepath_t &get_sharedpathchache();
#endif

#ifdef ENABLE_MODELSO
typedef std::function<std::string(std::shared_ptr<httppeer>)> www_method_call;

www_method_call controlmodulecreate(std::string module, std::string name, size_t tt);
www_method_call loadcontrol(const std::string &modulemethod, const std::string &name);
void controlmoduleclear(std::string module, std::string method);
std::map<std::size_t, www_method_call> &get_controlpathchache();
std::map<std::size_t, std::vector<std::string>> &get_sharedmethodchache();
#endif
}// namespace http
#endif
