
#include "sysperms_mysql.h"
#include "cms/include/sysperms_base.h"
#include "cms/include/Sysperms.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Sysperms::Sysperms(std::string dbtag):sysperms_mysql(dbtag){ mod=this; }
			 Sysperms::Sysperms():sysperms_mysql(){ mod=this; }


		} 

	  }
