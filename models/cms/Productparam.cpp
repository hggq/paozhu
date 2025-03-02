
#include "productparam_mysql.h"
#include "cms/include/productparambase.h"
#include "cms/include/Productparam.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Productparam::Productparam(std::string dbtag):productparam_mysql(dbtag){ mod=this; }
			 Productparam::Productparam():productparam_mysql(){ mod=this; }


		} 

	  }
