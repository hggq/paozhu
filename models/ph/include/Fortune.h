#ifndef ORM_PH_FORTUNE_H
#define ORM_PH_FORTUNE_H
#include "ph/include/fortune_opsql.h" 
#include "ph/include/fortune_base.h"

/* Sat, 15 Aug 2026 06:07:42 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace ph { 
		class Fortune : public fortune_opsql<Fortune,fortune_base>{
		 public:
		 Fortune(std::string dbtag_);
		 Fortune();
		};
	 }
 }
#endif
