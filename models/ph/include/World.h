#ifndef ORM_PH_WORLD_H
#define ORM_PH_WORLD_H
#include "ph/include/world_opsql.h" 
#include "ph/include/world_base.h"

/* Sat, 15 Aug 2026 06:07:42 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace ph { 
		class World : public world_opsql<World,world_base>{
		 public:
		 World(std::string dbtag_);
		 World();
		};
	 }
 }
#endif
