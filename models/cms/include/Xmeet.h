#ifndef ORM_CMS_XMEET_H
#define ORM_CMS_XMEET_H
#include "xmeet_mysql.h" 
#include "cms/include/xmeetbase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Xmeet : public xmeet_mysql<Xmeet,xmeetbase>{
		 public:
		 Xmeet(std::string dbtag);
		 Xmeet();
		};
	 }
 }
#endif
