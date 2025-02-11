#ifndef ORM_CMS_HOMEBLOCK_H
#define ORM_CMS_HOMEBLOCK_H
#include "mysqlmodel.hpp" 
#include "cms/include/homeblockbase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	 namespace cms { 
		class Homeblock : public mysqlclientDB<Homeblock,homeblockbase>{
		 public:
		 Homeblock(std::string dbtag);
		 Homeblock();
		};
	 }
 };
#endif
