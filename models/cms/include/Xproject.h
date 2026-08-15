#ifndef ORM_CMS_XPROJECT_H
#define ORM_CMS_XPROJECT_H
#include "cms/include/xproject_opsql.h" 
#include "cms/include/xproject_base.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Xproject : public xproject_opsql<Xproject,xproject_base>{
		 public:
		 Xproject(std::string dbtag_);
		 Xproject();
		};
	 }
 }
#endif
