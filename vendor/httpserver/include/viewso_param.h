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
        view_param(OBJ_VALUE &a,OBJ_VALUE &b,Cookie &c,OBJ_VALUE &d):get(a),post(b),cookie(c),session(d)
        {

        }
        OBJ_VALUE &get;
        OBJ_VALUE &post;
        Cookie &cookie;
        OBJ_VALUE &session;
        std::string viewsopath;
    };
}
#endif
