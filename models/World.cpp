
#include "orm/include/world_opsql.h"
#include "orm/include/world_base.h"
#include "models/include/World.h"

/* Tue, 01 Sep 2026 03:21:20 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
 
			 World::World(std::string dbtag_):world_opsql(dbtag_){ mod=this; }
			 World::World():world_opsql(){ mod=this; }


	  }
