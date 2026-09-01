
#include "orm/cms/include/topicset_opsql.h"
#include "orm/cms/include/topicset_base.h"
#include "models/cms/include/Topicset.h"

/* Tue, 01 Sep 2026 03:21:26 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Topicset::Topicset(std::string dbtag_):topicset_opsql(dbtag_){ mod=this; }
			 Topicset::Topicset():topicset_opsql(){ mod=this; }


		} 

	  }
