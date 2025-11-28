#ifndef ORM_CMS_SYSROLEPERMS_H
#define ORM_CMS_SYSROLEPERMS_H
#include "cms/include/sysroleperms_mysql.h" 
#include "cms/include/sysroleperms_base.h"

/* Fri, 28 Nov 2025 12:11:33 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Sysroleperms : public sysroleperms_mysql<Sysroleperms,sysroleperms_base>{
		 public:
		 Sysroleperms(std::string dbtag);
		 Sysroleperms();
		};
	 }
 }
#endif
