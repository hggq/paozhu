
#include "orm/include/orm_probe_opsql.h"
#include "orm/include/orm_probe_base.h"
#include "models/include/OrmProbe.h"

/* Sat, 12 Sep 2026 07:42:43 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
 
			 OrmProbe::OrmProbe(std::string dbtag_):orm_probe_opsql(dbtag_){ mod=this; }
			 OrmProbe::OrmProbe():orm_probe_opsql(){ mod=this; }


	  }
