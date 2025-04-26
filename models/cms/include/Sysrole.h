#ifndef ORM_CMS_SYSROLE_H
#define ORM_CMS_SYSROLE_H
#include "sysrole_mysql.h" 
#include "cms/include/sysrole_base.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Sysrole : public sysrole_mysql<Sysrole,sysrole_base>{
		 public:
		 Sysrole(std::string dbtag);
		 Sysrole();
		};
	 }
 }
#endif
