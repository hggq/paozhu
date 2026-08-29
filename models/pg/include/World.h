#ifndef ORM_PG_WORLD_H
#define ORM_PG_WORLD_H
#include "pg/include/world_opsql.h" 
#include "pg/include/world_base.h"

/* Sat, 29 Aug 2026 05:49:37 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace pg { 
		class World : public world_opsql<World,world_base>{
		 public:
		 World(std::string dbtag_);
		 World();
		};
	 }
 }
#endif
