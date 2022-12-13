#ifndef ORM_CMS_USER_H
#define ORM_CMS_USER_H
#include "mysqlmodel.hpp" 
#include "cms/include/userbase.h"
#include "cms/include/Article.h"
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	 namespace cms { 
		class User : public mysqlclientDB<User,userbase>{
		 public:
		 User(std::string dbtag);
		 User();
		 User &get();
		 std::vector<orm::cms::articlebase::meta> gettoparticle(int userid);
		};
	 }
 };
#endif
