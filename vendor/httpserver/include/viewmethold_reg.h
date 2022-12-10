#ifndef __VIEWMETHOLD_REG_H__
#define __VIEWMETHOLD_REG_H__

#include<string>
#include<map>
#include<functional>
#include "request.h"
#include "viewso_param.h"
namespace http
{
  typedef std::map<std::string,std::function<std::string(const struct view_param&,http::OBJ_VALUE &)>> VIEW_REG;

void _run_view_reg_to();
  VIEW_REG &get_viewmetholdreg();

}
#endif
