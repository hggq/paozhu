
#include "sitelog_mysql.h"
#include "cms/include/sitelog_base.h"
#include "cms/include/Sitelog.h"

/* Thu, 07 Aug 2025 02:38:20 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Sitelog::Sitelog(std::string dbtag):sitelog_mysql(dbtag){ mod=this; }
			 Sitelog::Sitelog():sitelog_mysql(){ mod=this; }


		} 

	  }
