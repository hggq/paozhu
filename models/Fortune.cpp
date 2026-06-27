
#include "fortune_mysql.h"
#include "fortune_base.h"
#include "Fortune.h"

/* Sat, 27 Jun 2026 11:17:52 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
 
			 Fortune::Fortune(std::string dbtag_):fortune_mysql(dbtag_){ mod=this; }
			 Fortune::Fortune():fortune_mysql(){ mod=this; }


	  }
