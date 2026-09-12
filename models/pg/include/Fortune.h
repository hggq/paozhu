#ifndef ORM_PG_FORTUNE_H
#define ORM_PG_FORTUNE_H
#include "orm/pg/include/fortune_opsql.h" 
#include "orm/pg/include/fortune_base.h"

/* Sat, 12 Sep 2026 07:38:42 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace pg { 
		class Fortune : public fortune_opsql<Fortune,fortune_base>{
		 public:
		 Fortune(std::string dbtag_);
		 Fortune();
		};
	 }
 }
#endif
