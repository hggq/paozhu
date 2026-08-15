#ifndef ORM_CMS_HOMEBLOCK_H
#define ORM_CMS_HOMEBLOCK_H
#include "cms/include/homeblock_opsql.h" 
#include "cms/include/homeblock_base.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Homeblock : public homeblock_opsql<Homeblock,homeblock_base>{
		 public:
		 Homeblock(std::string dbtag_);
		 Homeblock();
		};
	 }
 }
#endif
