
#include "cms/include/testb_opsql.h"
#include "cms/include/testb_base.h"
#include "cms/include/Testb.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Testb::Testb(std::string dbtag_):testb_opsql(dbtag_){ mod=this; }
			 Testb::Testb():testb_opsql(){ mod=this; }


		} 

	  }
