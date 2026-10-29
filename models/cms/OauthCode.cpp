
#include "orm/cms/include/oauth_code_opsql.h"
#include "orm/cms/include/oauth_code_base.h"
#include "models/cms/include/OauthCode.h"

/* Wed, 28 Oct 2026 04:26:39 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 OauthCode::OauthCode(std::string dbtag_):oauth_code_opsql(dbtag_){ mod=this; }
			 OauthCode::OauthCode():oauth_code_opsql(){ mod=this; }


		} 

	  }
