#ifndef ORM_CMS_LOGININFO_H
#define ORM_CMS_LOGININFO_H
#include "logininfo_mysql.h" 
#include "cms/include/logininfobase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Logininfo : public logininfo_mysql<Logininfo,logininfobase>{
		 public:
		 Logininfo(std::string dbtag);
		 Logininfo();
		};
	 }
 }
#endif
