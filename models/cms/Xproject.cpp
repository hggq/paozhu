
#include "cms/include/xproject_opsql.h"
#include "cms/include/xproject_base.h"
#include "cms/include/Xproject.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Xproject::Xproject(std::string dbtag_):xproject_opsql(dbtag_){ mod=this; }
			 Xproject::Xproject():xproject_opsql(){ mod=this; }


		} 

	  }
