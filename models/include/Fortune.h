#ifndef ORM_DEFAULT_FORTUNE_H
#define ORM_DEFAULT_FORTUNE_H
#include "fortune_opsql.h" 
#include "fortune_base.h"

/* Fri, 14 Aug 2026 13:00:43 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
		class Fortune : public fortune_opsql<Fortune,fortune_base>{
		 public:
		 Fortune(std::string dbtag_);
		 Fortune();
		};
}
#endif
