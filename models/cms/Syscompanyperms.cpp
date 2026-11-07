
#include "orm/cms/include/syscompanyperms_opsql.h"
#include "orm/cms/include/syscompanyperms_base.h"
#include "models/cms/include/Syscompanyperms.h"

/* Sat, 07 Nov 2026 04:02:55 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Syscompanyperms::Syscompanyperms(std::string dbtag_):syscompanyperms_opsql(dbtag_){ mod=this; }
			 Syscompanyperms::Syscompanyperms():syscompanyperms_opsql(){ mod=this; }


		} 

	  }
