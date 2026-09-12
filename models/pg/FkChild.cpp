
#include "orm/pg/include/fk_child_opsql.h"
#include "orm/pg/include/fk_child_base.h"
#include "models/pg/include/FkChild.h"

/* Sat, 12 Sep 2026 07:38:42 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace pg{  
			 FkChild::FkChild(std::string dbtag_):fk_child_opsql(dbtag_){ mod=this; }
			 FkChild::FkChild():fk_child_opsql(){ mod=this; }


		} 

	  }
