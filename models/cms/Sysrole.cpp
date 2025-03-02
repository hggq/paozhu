
#include "sysrole_mysql.h"
#include "cms/include/sysrolebase.h"
#include "cms/include/Sysrole.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Sysrole::Sysrole(std::string dbtag):sysrole_mysql(dbtag){ mod=this; }
			 Sysrole::Sysrole():sysrole_mysql(){ mod=this; }


		} 

	  }
