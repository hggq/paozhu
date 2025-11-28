
#include "cms/include/blogcatalog_mysql.h"
#include "cms/include/blogcatalog_base.h"
#include "cms/include/Blogcatalog.h"

/* Fri, 28 Nov 2025 12:11:33 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Blogcatalog::Blogcatalog(std::string dbtag):blogcatalog_mysql(dbtag){ mod=this; }
			 Blogcatalog::Blogcatalog():blogcatalog_mysql(){ mod=this; }


		} 

	  }
