
#include "cms/include/topicset_mysql.h"
#include "cms/include/topicset_base.h"
#include "cms/include/Topicset.h"

/* Fri, 28 Nov 2025 12:11:33 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Topicset::Topicset(std::string dbtag):topicset_mysql(dbtag){ mod=this; }
			 Topicset::Topicset():topicset_mysql(){ mod=this; }


		} 

	  }
