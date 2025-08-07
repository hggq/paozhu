#ifndef __HTTP_VIEWSRC_ALL_METHOD_H
#define __HTTP_VIEWSRC_ALL_METHOD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include<string>
#include<map>
#include<functional>
#include "request.h"
#include "viewso_param.h"

namespace http { 
namespace view { 

namespace  admin{ 

	std::string addarticle(const struct view_param &vinfo,http::obj_val &obj);
	std::string addhomecontent(const struct view_param &vinfo,http::obj_val &obj);
	std::string addhomehot(const struct view_param &vinfo,http::obj_val &obj);
	std::string addhomepic(const struct view_param &vinfo,http::obj_val &obj);
	std::string addhometext(const struct view_param &vinfo,http::obj_val &obj);
	std::string addhometopic(const struct view_param &vinfo,http::obj_val &obj);
	std::string addproduct(const struct view_param &vinfo,http::obj_val &obj);
	std::string addtopic(const struct view_param &vinfo,http::obj_val &obj);
	std::string addtopicsidepick(const struct view_param &vinfo,http::obj_val &obj);
	std::string addtopicsidetext(const struct view_param &vinfo,http::obj_val &obj);
	std::string copyright(const struct view_param &vinfo,http::obj_val &obj);
	std::string editarticle(const struct view_param &vinfo,http::obj_val &obj);
	std::string edithomeblockmulit(const struct view_param &vinfo,http::obj_val &obj);
	std::string edithomecontent(const struct view_param &vinfo,http::obj_val &obj);
	std::string edithomehot(const struct view_param &vinfo,http::obj_val &obj);
	std::string edithomepic(const struct view_param &vinfo,http::obj_val &obj);
	std::string edithometext(const struct view_param &vinfo,http::obj_val &obj);
	std::string edithometopic(const struct view_param &vinfo,http::obj_val &obj);
	std::string editpassword(const struct view_param &vinfo,http::obj_val &obj);
	std::string editproduct(const struct view_param &vinfo,http::obj_val &obj);
	std::string edittopic(const struct view_param &vinfo,http::obj_val &obj);
	std::string edittopicsidepick(const struct view_param &vinfo,http::obj_val &obj);
	std::string edittopicsidetext(const struct view_param &vinfo,http::obj_val &obj);
	std::string fileupload(const struct view_param &vinfo,http::obj_val &obj);
	std::string footscript(const struct view_param &vinfo,http::obj_val &obj);
	std::string homedesign(const struct view_param &vinfo,http::obj_val &obj);
	std::string listarticle(const struct view_param &vinfo,http::obj_val &obj);
	std::string listproduct(const struct view_param &vinfo,http::obj_val &obj);
	std::string login(const struct view_param &vinfo,http::obj_val &obj);
	std::string main(const struct view_param &vinfo,http::obj_val &obj);
	std::string marbrand(const struct view_param &vinfo,http::obj_val &obj);
	std::string marcategory(const struct view_param &vinfo,http::obj_val &obj);
	std::string marhome(const struct view_param &vinfo,http::obj_val &obj);
	std::string marproductattach(const struct view_param &vinfo,http::obj_val &obj);
	std::string martopic(const struct view_param &vinfo,http::obj_val &obj);
	std::string martopicside(const struct view_param &vinfo,http::obj_val &obj);
	std::string siteinfo(const struct view_param &vinfo,http::obj_val &obj);
	std::string sitelogo(const struct view_param &vinfo,http::obj_val &obj);
	std::string userinfo(const struct view_param &vinfo,http::obj_val &obj);
	std::string welcome(const struct view_param &vinfo,http::obj_val &obj);
}

namespace  cms{ 

	std::string add(const struct view_param &vinfo,http::obj_val &obj);
	std::string edit(const struct view_param &vinfo,http::obj_val &obj);
	std::string list(const struct view_param &vinfo,http::obj_val &obj);
	std::string show(const struct view_param &vinfo,http::obj_val &obj);
}

namespace  home{ 

	std::string header(const struct view_param &vinfo,http::obj_val &obj);
}

namespace  login{ 

	std::string login(const struct view_param &vinfo,http::obj_val &obj);
}

namespace  superadmin{ 

	std::string addsiteinfo(const struct view_param &vinfo,http::obj_val &obj);
	std::string adduser(const struct view_param &vinfo,http::obj_val &obj);
	std::string deletesiteinfo(const struct view_param &vinfo,http::obj_val &obj);
	std::string editpassword(const struct view_param &vinfo,http::obj_val &obj);
	std::string editsiteinfo(const struct view_param &vinfo,http::obj_val &obj);
	std::string edituser(const struct view_param &vinfo,http::obj_val &obj);
	std::string listuser(const struct view_param &vinfo,http::obj_val &obj);
	std::string login(const struct view_param &vinfo,http::obj_val &obj);
	std::string main(const struct view_param &vinfo,http::obj_val &obj);
	std::string siteinfo(const struct view_param &vinfo,http::obj_val &obj);
	std::string userinfo(const struct view_param &vinfo,http::obj_val &obj);
	std::string userlist(const struct view_param &vinfo,http::obj_val &obj);
	std::string welcome(const struct view_param &vinfo,http::obj_val &obj);
}

namespace  techempower{ 

	std::string fortunes(const struct view_param &vinfo,http::obj_val &obj);
}


}

}
#endif