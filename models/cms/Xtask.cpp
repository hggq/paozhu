
#include "cms/include/xtask_mysql.h"
#include "cms/include/xtask_base.h"
#include "cms/include/Xtask.h"

/* Sat, 27 Jun 2026 11:17:55 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Xtask::Xtask(std::string dbtag_):xtask_mysql(dbtag_){ mod=this; }
			 Xtask::Xtask():xtask_mysql(){ mod=this; }


		} 

	  }
