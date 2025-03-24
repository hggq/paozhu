#ifndef __FRAME_VIEW_SO_PARAM_H__
#define __FRAME_VIEW_SO_PARAM_H__

#include <iostream>
#include <string>
#include "request.h"
#include "cookie.h"
#include "func.h"
namespace http
{
    struct view_param
    {
        view_param(obj_val &a,obj_val &b,http::cookie &c,obj_val &d):get(a),post(b),cookie(c),session(d)
        {

        }
        obj_val &get;
        obj_val &post;
        http::cookie &cookie;
        obj_val &session;
        std::string viewsopath;
    };
}
#endif
