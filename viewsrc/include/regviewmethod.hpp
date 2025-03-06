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
            	 //create time: Wed, 05 Mar 2025 12:22:47 GMT

	_viewmetholdreg.emplace("admin/addarticle",http::view::admin::addarticle);
	_viewmetholdreg.emplace("admin/addhomecontent",http::view::admin::addhomecontent);
	_viewmetholdreg.emplace("admin/addhomehot",http::view::admin::addhomehot);
	_viewmetholdreg.emplace("admin/addhomepic",http::view::admin::addhomepic);
	_viewmetholdreg.emplace("admin/addhometext",http::view::admin::addhometext);
	_viewmetholdreg.emplace("admin/addhometopic",http::view::admin::addhometopic);
	_viewmetholdreg.emplace("admin/addproduct",http::view::admin::addproduct);
	_viewmetholdreg.emplace("admin/addtopic",http::view::admin::addtopic);
	_viewmetholdreg.emplace("admin/copyright",http::view::admin::copyright);
	_viewmetholdreg.emplace("admin/editarticle",http::view::admin::editarticle);
	_viewmetholdreg.emplace("admin/edithomeblockmulit",http::view::admin::edithomeblockmulit);
	_viewmetholdreg.emplace("admin/edithomecontent",http::view::admin::edithomecontent);
	_viewmetholdreg.emplace("admin/edithomehot",http::view::admin::edithomehot);
	_viewmetholdreg.emplace("admin/edithomepic",http::view::admin::edithomepic);
	_viewmetholdreg.emplace("admin/edithometext",http::view::admin::edithometext);
	_viewmetholdreg.emplace("admin/edithometopic",http::view::admin::edithometopic);
	_viewmetholdreg.emplace("admin/editpassword",http::view::admin::editpassword);
	_viewmetholdreg.emplace("admin/editproduct",http::view::admin::editproduct);
	_viewmetholdreg.emplace("admin/edittopic",http::view::admin::edittopic);
	_viewmetholdreg.emplace("admin/fileupload",http::view::admin::fileupload);
	_viewmetholdreg.emplace("admin/footscript",http::view::admin::footscript);
	_viewmetholdreg.emplace("admin/homedesign",http::view::admin::homedesign);
	_viewmetholdreg.emplace("admin/listarticle",http::view::admin::listarticle);
	_viewmetholdreg.emplace("admin/listproduct",http::view::admin::listproduct);
	_viewmetholdreg.emplace("admin/login",http::view::admin::login);
	_viewmetholdreg.emplace("admin/main",http::view::admin::main);
	_viewmetholdreg.emplace("admin/marbrand",http::view::admin::marbrand);
	_viewmetholdreg.emplace("admin/marcategory",http::view::admin::marcategory);
	_viewmetholdreg.emplace("admin/marhome",http::view::admin::marhome);
	_viewmetholdreg.emplace("admin/marproductattach",http::view::admin::marproductattach);
	_viewmetholdreg.emplace("admin/martopic",http::view::admin::martopic);
	_viewmetholdreg.emplace("admin/siteinfo",http::view::admin::siteinfo);
	_viewmetholdreg.emplace("admin/sitelogo",http::view::admin::sitelogo);
	_viewmetholdreg.emplace("admin/userinfo",http::view::admin::userinfo);
	_viewmetholdreg.emplace("admin/welcome",http::view::admin::welcome);
	_viewmetholdreg.emplace("cms/add",http::view::cms::add);
	_viewmetholdreg.emplace("cms/edit",http::view::cms::edit);
	_viewmetholdreg.emplace("cms/list",http::view::cms::list);
	_viewmetholdreg.emplace("cms/show",http::view::cms::show);
	_viewmetholdreg.emplace("home/header",http::view::home::header);
	_viewmetholdreg.emplace("login/login",http::view::login::login);
	_viewmetholdreg.emplace("superadmin/addsiteinfo",http::view::superadmin::addsiteinfo);
	_viewmetholdreg.emplace("superadmin/adduser",http::view::superadmin::adduser);
	_viewmetholdreg.emplace("superadmin/deletesiteinfo",http::view::superadmin::deletesiteinfo);
	_viewmetholdreg.emplace("superadmin/editpassword",http::view::superadmin::editpassword);
	_viewmetholdreg.emplace("superadmin/editsiteinfo",http::view::superadmin::editsiteinfo);
	_viewmetholdreg.emplace("superadmin/edituser",http::view::superadmin::edituser);
	_viewmetholdreg.emplace("superadmin/listuser",http::view::superadmin::listuser);
	_viewmetholdreg.emplace("superadmin/login",http::view::superadmin::login);
	_viewmetholdreg.emplace("superadmin/main",http::view::superadmin::main);
	_viewmetholdreg.emplace("superadmin/siteinfo",http::view::superadmin::siteinfo);
	_viewmetholdreg.emplace("superadmin/userinfo",http::view::superadmin::userinfo);
	_viewmetholdreg.emplace("superadmin/userlist",http::view::superadmin::userlist);
	_viewmetholdreg.emplace("superadmin/welcome",http::view::superadmin::welcome);
	_viewmetholdreg.emplace("techempower/fortunes",http::view::techempower::fortunes);

	} 
}
#endif