
#include "siteinfo_mysql.h"
#include "cms/include/siteinfo_base.h"
#include "cms/include/Siteinfo.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Siteinfo::Siteinfo(std::string dbtag):siteinfo_mysql(dbtag){ mod=this; }
			 Siteinfo::Siteinfo():siteinfo_mysql(){ mod=this; }


		} 

	  }
