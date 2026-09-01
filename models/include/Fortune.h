#ifndef ORM_DEFAULT_FORTUNE_H
#define ORM_DEFAULT_FORTUNE_H
#include "orm/include/fortune_opsql.h" 
#include "orm/include/fortune_base.h"

/* Tue, 01 Sep 2026 03:21:20 GMT */
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
