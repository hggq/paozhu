#ifndef __FRAME_LOADVEWSO_H__
#define __FRAME_LOADVEWSO_H__

#include <iostream>
#include <string>
#include <list>
#include <functional>
#include <mysqlx/xdevapi.h>
#include <boost/dll/alias.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>
#include "request.h"
#include "cookie.h"
#include "func.h"

namespace http
{
   
    typedef boost::function<boost::function<std::string( struct view_param ,OBJ_VALUE &)>(std::string)> view_modulemethod_callback_t;
    typedef boost::function<std::string( struct view_param ,OBJ_VALUE &)> view_method_callback_t;
    typedef std::map<std::size_t, view_method_callback_t> view_so_cachepath_t;

    view_method_callback_t viewsomodulecreate(std::string module, std::string name);
    view_method_callback_t loadviewso(std::string modulemethod);
 
    
}
#endif
