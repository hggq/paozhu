#ifndef ORM_CMS_SYSUSER_H
#define ORM_CMS_SYSUSER_H
#include "cms/include/sysuser_mysql.h" 
#include "cms/include/sysuser_base.h"
#include "cms/include/Article.h"
/* Fri, 28 Nov 2025 12:11:33 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Sysuser : public sysuser_mysql<Sysuser,sysuser_base>{
		 public:
		 Sysuser(std::string dbtag);
		 Sysuser();
		 std::vector<orm::cms::article_base::meta> gettoparticle(int userid);
		};
	 }
 }
#endif
