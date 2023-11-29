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
            	 //create time: Wed, 29 Nov 2023 09:25:06 GMT

	_viewmetholdreg.emplace("admin/addarticle",http::view::admin::addarticle);
	_viewmetholdreg.emplace("admin/addtopic",http::view::admin::addtopic);
	_viewmetholdreg.emplace("admin/editarticle",http::view::admin::editarticle);
	_viewmetholdreg.emplace("admin/editpassword",http::view::admin::editpassword);
	_viewmetholdreg.emplace("admin/edittopic",http::view::admin::edittopic);
	_viewmetholdreg.emplace("admin/listarticle",http::view::admin::listarticle);
	_viewmetholdreg.emplace("admin/main",http::view::admin::main);
	_viewmetholdreg.emplace("admin/martopic",http::view::admin::martopic);
	_viewmetholdreg.emplace("admin/siteinfo",http::view::admin::siteinfo);
	_viewmetholdreg.emplace("admin/userinfo",http::view::admin::userinfo);
	_viewmetholdreg.emplace("admin/welcome",http::view::admin::welcome);
	_viewmetholdreg.emplace("cms/add",http::view::cms::add);
	_viewmetholdreg.emplace("cms/edit",http::view::cms::edit);
	_viewmetholdreg.emplace("cms/list",http::view::cms::list);
	_viewmetholdreg.emplace("cms/show",http::view::cms::show);
	_viewmetholdreg.emplace("home/header",http::view::home::header);
	_viewmetholdreg.emplace("login/login",http::view::login::login);

	} 
}
#endif