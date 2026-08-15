#ifndef ORM_CMS_BRAND_H
#define ORM_CMS_BRAND_H
#include "cms/include/brand_opsql.h" 
#include "cms/include/brand_base.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Brand : public brand_opsql<Brand,brand_base>{
		 public:
		 Brand(std::string dbtag_);
		 Brand();
		};
	 }
 }
#endif
