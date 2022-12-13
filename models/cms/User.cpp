
#include "mysqlmodel.hpp" 
#include "cms/include/userbase.h"
#include "cms/include/User.h"
#include "cms/include/articlebase.h"
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  

			 User::User(std::string dbtag):mysqlclientDB(dbtag){}
			 User::User():mysqlclientDB(){}
			 User &User::get(){ return *this; }

			std::vector<orm::cms::articlebase::meta> User::gettoparticle(int userid)
			{
				auto art = orm::cms::Article();
				userid=this->data.userid;
				art.where("userid",userid).order(" aid desc ").limit(5).fetch();
				//art.where("userid",userid).whereOr("userid",0).order(" aid desc ").limit(5).fetch();
				std::cout<<art.sqlstring;
				return art.record;	

			}
		} 

	  }
