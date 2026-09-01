#ifndef ORM_CMS_SYSUSER_H
#define ORM_CMS_SYSUSER_H
#include "orm/cms/include/sysuser_opsql.h" 
#include "orm/cms/include/sysuser_base.h"
#include "models/cms/include/Article.h"
/* Tue, 01 Sep 2026 03:21:26 GMT */
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
