#ifndef ORM_CMS_TESTB_H
#define ORM_CMS_TESTB_H
#include "testb_mysql.h" 
#include "cms/include/testb_base.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Testb : public testb_mysql<Testb,testb_base>{
		 public:
		 Testb(std::string dbtag);
		 Testb();
		};
	 }
 }
#endif
