
#include "brand_mysql.h"
#include "cms/include/brandbase.h"
#include "cms/include/Brand.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Brand::Brand(std::string dbtag):brand_mysql(dbtag){ mod=this; }
			 Brand::Brand():brand_mysql(){ mod=this; }


		} 

	  }
