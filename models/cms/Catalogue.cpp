
#include "catalogue_mysql.h"
#include "cms/include/catalogue_base.h"
#include "cms/include/Catalogue.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Catalogue::Catalogue(std::string dbtag):catalogue_mysql(dbtag){ mod=this; }
			 Catalogue::Catalogue():catalogue_mysql(){ mod=this; }


		} 

	  }
