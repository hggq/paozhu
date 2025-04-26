
#include "xproject_mysql.h"
#include "cms/include/xproject_base.h"
#include "cms/include/Xproject.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Xproject::Xproject(std::string dbtag):xproject_mysql(dbtag){ mod=this; }
			 Xproject::Xproject():xproject_mysql(){ mod=this; }


		} 

	  }
