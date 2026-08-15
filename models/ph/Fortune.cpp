
#include "ph/include/fortune_opsql.h"
#include "ph/include/fortune_base.h"
#include "ph/include/Fortune.h"

/* Sat, 15 Aug 2026 06:07:42 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace ph{  
			 Fortune::Fortune(std::string dbtag_):fortune_opsql(dbtag_){ mod=this; }
			 Fortune::Fortune():fortune_opsql(){ mod=this; }


		} 

	  }
