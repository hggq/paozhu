
#include "orm/lite/include/fortune_opsql.h"
#include "orm/lite/include/fortune_base.h"
#include "models/lite/include/Fortune.h"

/* Tue, 01 Sep 2026 03:21:31 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace lite{  
			 Fortune::Fortune(std::string dbtag_):fortune_opsql(dbtag_){ mod=this; }
			 Fortune::Fortune():fortune_opsql(){ mod=this; }


		} 

	  }
