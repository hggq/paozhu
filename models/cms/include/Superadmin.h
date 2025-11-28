#ifndef ORM_CMS_SUPERADMIN_H
#define ORM_CMS_SUPERADMIN_H
#include "cms/include/superadmin_mysql.h" 
#include "cms/include/superadmin_base.h"

/* Fri, 28 Nov 2025 12:11:33 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Superadmin : public superadmin_mysql<Superadmin,superadmin_base>{
		 public:
		 Superadmin(std::string dbtag);
		 Superadmin();
		};
	 }
 }
#endif
