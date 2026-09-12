
#include "orm/pg/include/fk_parent_opsql.h"
#include "orm/pg/include/fk_parent_base.h"
#include "models/pg/include/FkParent.h"

/* Sat, 12 Sep 2026 07:38:42 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace pg{  
			 FkParent::FkParent(std::string dbtag_):fk_parent_opsql(dbtag_){ mod=this; }
			 FkParent::FkParent():fk_parent_opsql(){ mod=this; }


		} 

	  }
