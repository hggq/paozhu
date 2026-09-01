#ifndef ORM_LITE_WORLD_H
#define ORM_LITE_WORLD_H
#include "orm/lite/include/world_opsql.h" 
#include "orm/lite/include/world_base.h"

/* Tue, 01 Sep 2026 03:21:31 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace lite { 
		class World : public world_opsql<World,world_base>{
		 public:
		 World(std::string dbtag_);
		 World();
		};
	 }
 }
#endif
