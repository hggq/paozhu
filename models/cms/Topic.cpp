
#include "topic_mysql.h"
#include "cms/include/topic_base.h"
#include "cms/include/Topic.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Topic::Topic(std::string dbtag):topic_mysql(dbtag){ mod=this; }
			 Topic::Topic():topic_mysql(){ mod=this; }


		} 

	  }
