
#include "cms/include/sysrole_mysql.h"
#include "cms/include/sysrole_base.h"
#include "cms/include/Sysrole.h"

/* Sat, 27 Jun 2026 11:17:55 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Sysrole::Sysrole(std::string dbtag_):sysrole_mysql(dbtag_){ mod=this; }
			 Sysrole::Sysrole():sysrole_mysql(){ mod=this; }


		} 

	  }
