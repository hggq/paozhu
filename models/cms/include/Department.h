#ifndef ORM_CMS_DEPARTMENT_H
#define ORM_CMS_DEPARTMENT_H
#include "cms/include/department_mysql.h" 
#include "cms/include/department_base.h"

/* Fri, 28 Nov 2025 12:11:33 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Department : public department_mysql<Department,department_base>{
		 public:
		 Department(std::string dbtag);
		 Department();
		};
	 }
 }
#endif
