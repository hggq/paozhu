
#include "orm/cms/include/sysuser_opsql.h"
#include "orm/cms/include/sysuser_base.h"
#include "models/cms/include/Sysuser.h"
#include "models/cms/include/Article.h"
/* Tue, 01 Sep 2026 03:21:26 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Sysuser::Sysuser(std::string dbtag_):sysuser_opsql(dbtag_){ mod=this; }
			 Sysuser::Sysuser():sysuser_opsql(){ mod=this; }

			 std::vector<orm::cms::article_info::meta> Sysuser::gettoparticle(int userid)
			 {
				 auto art = orm::cms::Article();
				 userid=this->data.companyid;
				 art.where("userid",userid).order(" aid desc ").limit(5).fetch();
				 //art.where("userid",userid).whereOr("userid",0).order(" aid desc ").limit(5).fetch();
				 std::cout<<art.sqlstring;
				 return art.record;	
 
			 }
		} 

	  }
