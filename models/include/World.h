#ifndef ORM_DEFAULT_WORLD_H
#define ORM_DEFAULT_WORLD_H
#include "orm/include/world_opsql.h" 
#include "orm/include/world_base.h"

/* Sat, 12 Sep 2026 07:42:43 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
		class World : public world_opsql<World,world_base>{
		 public:
		 World(std::string dbtag_);
		 World();
		};
}
#endif
