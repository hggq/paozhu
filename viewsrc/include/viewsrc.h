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


}

}
#endif