#ifndef ORM_LITE_FORTUNE_H
#define ORM_LITE_FORTUNE_H
#include "lite/include/fortune_opsql.h" 
#include "lite/include/fortune_base.h"

/* Wed, 26 Aug 2026 13:26:06 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace lite { 
		class Fortune : public fortune_opsql<Fortune,fortune_base>{
		 public:
		 Fortune(std::string dbtag_);
		 Fortune();
		};
	 }
 }
#endif
