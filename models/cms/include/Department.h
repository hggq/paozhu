#ifndef ORM_CMS_DEPARTMENT_H
#define ORM_CMS_DEPARTMENT_H
#include "cms/include/department_opsql.h" 
#include "cms/include/department_base.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Department : public department_opsql<Department,department_base>{
		 public:
		 Department(std::string dbtag_);
		 Department();
		};
	 }
 }
#endif
