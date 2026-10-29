#ifndef ORM_CMS_OAUTHREFRESH_H
#define ORM_CMS_OAUTHREFRESH_H
#include "orm/cms/include/oauth_refresh_opsql.h" 
#include "orm/cms/include/oauth_refresh_base.h"

/* Wed, 28 Oct 2026 04:26:39 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class OauthRefresh : public oauth_refresh_opsql<OauthRefresh,oauth_refresh_base>{
		 public:
		 OauthRefresh(std::string dbtag_);
		 OauthRefresh();
		};
	 }
 }
#endif
