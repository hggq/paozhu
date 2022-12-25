#ifndef __HTTP_REG_VIEW_METHOD_HPP
#define __HTTP_REG_VIEW_METHOD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include<string>
#include<map>
#include<functional>
#include "request.h"
#include "viewso_param.h"
#include "viewmethold_reg.h"
#include "viewsrc.h"

namespace http
{
  void _initview_method_regto(VIEW_REG  &_viewmetholdreg)
  {
            	 //create time: Sun, 25 Dec 2022 01:10:24 GMT

	_viewmetholdreg.emplace("cms/add",http::view::cms::add);
	_viewmetholdreg.emplace("cms/edit",http::view::cms::edit);
	_viewmetholdreg.emplace("cms/list",http::view::cms::list);
	_viewmetholdreg.emplace("cms/show",http::view::cms::show);
	_viewmetholdreg.emplace("home/header",http::view::home::header);
	_viewmetholdreg.emplace("login/login",http::view::login::login);

	} 
}
#endif