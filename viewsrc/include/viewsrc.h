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

namespace admin{ 

	std::string addarticle(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string addhomecontent(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string addhomehot(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string addhomepic(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string addhometext(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string addhometopic(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string addproduct(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string addtopic(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string copyright(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string editarticle(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string edithomecontent(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string edithomehot(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string edithomepic(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string edithometext(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string edithometopic(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string editpassword(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string editproduct(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string edittopic(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string fileupload(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string footscript(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string homedesign(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string listarticle(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string listproduct(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string login(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string main(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string marbrand(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string marcategory(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string marhome(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string marproductattach(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string martopic(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string siteinfo(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string sitelogo(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string userinfo(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string welcome(const struct view_param &vinfo,http::OBJ_VALUE &obj);
}

namespace cms{ 

	std::string add(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string edit(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string list(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string show(const struct view_param &vinfo,http::OBJ_VALUE &obj);
}

namespace home{ 

	std::string header(const struct view_param &vinfo,http::OBJ_VALUE &obj);
}

namespace login{ 

	std::string login(const struct view_param &vinfo,http::OBJ_VALUE &obj);
}

namespace superadmin{ 

	std::string addsiteinfo(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string adduser(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string deletesiteinfo(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string editpassword(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string editsiteinfo(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string edituser(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string listuser(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string login(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string main(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string siteinfo(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string userinfo(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string userlist(const struct view_param &vinfo,http::OBJ_VALUE &obj);
	std::string welcome(const struct view_param &vinfo,http::OBJ_VALUE &obj);
}

namespace techempower{ 

	std::string fortunes(const struct view_param &vinfo,http::OBJ_VALUE &obj);
}


}

}
#endif