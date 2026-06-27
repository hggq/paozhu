
#include "cms/include/brand_mysql.h"
#include "cms/include/brand_base.h"
#include "cms/include/Brand.h"

/* Sat, 27 Jun 2026 11:17:55 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Brand::Brand(std::string dbtag_):brand_mysql(dbtag_){ mod=this; }
			 Brand::Brand():brand_mysql(){ mod=this; }


		} 

	  }
