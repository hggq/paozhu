
#include "cms/include/sysrole_opsql.h"
#include "cms/include/sysrole_base.h"
#include "cms/include/Sysrole.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Sysrole::Sysrole(std::string dbtag_):sysrole_opsql(dbtag_){ mod=this; }
			 Sysrole::Sysrole():sysrole_opsql(){ mod=this; }


		} 

	  }
