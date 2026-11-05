#ifndef __FRAME_LOADVEWSO_H__
#define __FRAME_LOADVEWSO_H__

#include <functional>
#include <map>
#include <string>
#include "pz_dll.h"
#include "request.h"
#include "cookie.h"
#include "func.h"

#ifdef ENABLE_VIEWSO
namespace http
{

typedef std::function<std::function<std::string(struct view_param, obj_val &)>(std::string)> view_modulemethod_callback_t;
typedef std::function<std::string(struct view_param, obj_val &)> view_method_callback_t;
typedef std::map<std::size_t, view_method_callback_t> view_so_cachepath_t;

view_method_callback_t viewsomodulecreate(std::string module, std::string name);
view_method_callback_t loadviewso(std::string modulemethod);

}// namespace http
#endif
#endif
