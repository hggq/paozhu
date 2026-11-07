
#include "orm/cms/include/sysuserrole_opsql.h"
#include "orm/cms/include/sysuserrole_base.h"
#include "models/cms/include/Sysuserrole.h"

/* Thu, 05 Nov 2026 12:27:44 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Sysuserrole::Sysuserrole(std::string dbtag_):sysuserrole_opsql(dbtag_){ mod=this; }
			 Sysuserrole::Sysuserrole():sysuserrole_opsql(){ mod=this; }


		} 

	  }
