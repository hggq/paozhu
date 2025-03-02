
#include "product_mysql.h"
#include "cms/include/productbase.h"
#include "cms/include/Product.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Product::Product(std::string dbtag):product_mysql(dbtag){ mod=this; }
			 Product::Product():product_mysql(){ mod=this; }


		} 

	  }
