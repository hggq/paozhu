#ifndef ORM_CMS_XTALK_H
#define ORM_CMS_XTALK_H
#include "xtalk_mysql.h" 
#include "cms/include/xtalkbase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Xtalk : public xtalk_mysql<Xtalk,xtalkbase>{
		 public:
		 Xtalk(std::string dbtag);
		 Xtalk();
		};
	 }
 }
#endif
