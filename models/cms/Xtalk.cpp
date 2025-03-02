
#include "xtalk_mysql.h"
#include "cms/include/xtalkbase.h"
#include "cms/include/Xtalk.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Xtalk::Xtalk(std::string dbtag):xtalk_mysql(dbtag){ mod=this; }
			 Xtalk::Xtalk():xtalk_mysql(){ mod=this; }


		} 

	  }
