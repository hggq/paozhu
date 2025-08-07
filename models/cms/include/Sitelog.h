#ifndef ORM_CMS_SITELOG_H
#define ORM_CMS_SITELOG_H
#include "sitelog_mysql.h" 
#include "cms/include/sitelog_base.h"

/* Thu, 07 Aug 2025 02:38:20 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Sitelog : public sitelog_mysql<Sitelog,sitelog_base>{
		 public:
		 Sitelog(std::string dbtag);
		 Sitelog();
		};
	 }
 }
#endif
