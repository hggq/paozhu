
#include "orm/include/fk_child_opsql.h"
#include "orm/include/fk_child_base.h"
#include "models/include/FkChild.h"

/* Sat, 12 Sep 2026 07:42:43 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
 
			 FkChild::FkChild(std::string dbtag_):fk_child_opsql(dbtag_){ mod=this; }
			 FkChild::FkChild():fk_child_opsql(){ mod=this; }


	  }
