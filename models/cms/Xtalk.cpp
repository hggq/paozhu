
#include "cms/include/xtalk_opsql.h"
#include "cms/include/xtalk_base.h"
#include "cms/include/Xtalk.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Xtalk::Xtalk(std::string dbtag_):xtalk_opsql(dbtag_){ mod=this; }
			 Xtalk::Xtalk():xtalk_opsql(){ mod=this; }


		} 

	  }
