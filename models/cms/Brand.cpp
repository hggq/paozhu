
#include "cms/include/brand_opsql.h"
#include "cms/include/brand_base.h"
#include "cms/include/Brand.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Brand::Brand(std::string dbtag_):brand_opsql(dbtag_){ mod=this; }
			 Brand::Brand():brand_opsql(){ mod=this; }


		} 

	  }
