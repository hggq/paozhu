
#include "xtask_mysql.h"
#include "cms/include/xtask_base.h"
#include "cms/include/Xtask.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Xtask::Xtask(std::string dbtag):xtask_mysql(dbtag){ mod=this; }
			 Xtask::Xtask():xtask_mysql(){ mod=this; }


		} 

	  }
