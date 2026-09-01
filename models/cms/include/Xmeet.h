#ifndef ORM_CMS_XMEET_H
#define ORM_CMS_XMEET_H
#include "orm/cms/include/xmeet_opsql.h" 
#include "orm/cms/include/xmeet_base.h"

/* Tue, 01 Sep 2026 03:21:26 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Xmeet : public xmeet_opsql<Xmeet,xmeet_base>{
		 public:
		 Xmeet(std::string dbtag_);
		 Xmeet();
		};
	 }
 }
#endif
