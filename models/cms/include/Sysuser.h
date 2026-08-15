#ifndef ORM_CMS_SYSUSER_H
#define ORM_CMS_SYSUSER_H
#include "cms/include/sysuser_opsql.h" 
#include "cms/include/sysuser_base.h"
#include "cms/include/Article.h"
/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Sysuser : public sysuser_opsql<Sysuser,sysuser_base>{
		 public:
		 Sysuser(std::string dbtag_);
		 Sysuser();
		 std::vector<orm::cms::article_info::meta> gettoparticle(int userid);
		};
	 }
 }
#endif
