
#include "sysroleperms_mysql.h"
#include "cms/include/sysroleperms_base.h"
#include "cms/include/Sysroleperms.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Sysroleperms::Sysroleperms(std::string dbtag):sysroleperms_mysql(dbtag){ mod=this; }
			 Sysroleperms::Sysroleperms():sysroleperms_mysql(){ mod=this; }


		} 

	  }
