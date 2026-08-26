
#include "lite/include/world_opsql.h"
#include "lite/include/world_base.h"
#include "lite/include/World.h"

/* Wed, 26 Aug 2026 13:26:06 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace lite{  
			 World::World(std::string dbtag_):world_opsql(dbtag_){ mod=this; }
			 World::World():world_opsql(){ mod=this; }


		} 

	  }
