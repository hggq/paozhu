
#include "testa_mysql.h"
#include "cms/include/testa_base.h"
#include "cms/include/Testa.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Testa::Testa(std::string dbtag):testa_mysql(dbtag){ mod=this; }
			 Testa::Testa():testa_mysql(){ mod=this; }


		} 

	  }
