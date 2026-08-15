#ifndef ORM_CMS_SITEINFO_H
#define ORM_CMS_SITEINFO_H
#include "cms/include/siteinfo_opsql.h" 
#include "cms/include/siteinfo_base.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Siteinfo : public siteinfo_opsql<Siteinfo,siteinfo_base>{
		 public:
		 Siteinfo(std::string dbtag_);
		 Siteinfo();
		};
	 }
 }
#endif
