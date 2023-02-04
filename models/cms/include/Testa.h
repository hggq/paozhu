#ifndef ORM_CMS_TESTA_H
#define ORM_CMS_TESTA_H
#include "mysqlmodel.hpp" 
#include "cms/include/testabase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	 namespace cms { 
		class Testa : public mysqlclientDB<Testa,testabase>{
		 public:
		 Testa(std::string dbtag);
		 Testa();
		};
	 }
 };
#endif
