#ifndef ORM_CMS_XPROJECT_H
#define ORM_CMS_XPROJECT_H
#include "xproject_mysql.h" 
#include "cms/include/xprojectbase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Xproject : public xproject_mysql<Xproject,xprojectbase>{
		 public:
		 Xproject(std::string dbtag);
		 Xproject();
		};
	 }
 }
#endif
