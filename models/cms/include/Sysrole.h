#ifndef ORM_CMS_SYSROLE_H
#define ORM_CMS_SYSROLE_H
#include "cms/include/sysrole_opsql.h" 
#include "cms/include/sysrole_base.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Sysrole : public sysrole_opsql<Sysrole,sysrole_base>{
		 public:
		 Sysrole(std::string dbtag_);
		 Sysrole();
		};
	 }
 }
#endif
