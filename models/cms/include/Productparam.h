#ifndef ORM_CMS_PRODUCTPARAM_H
#define ORM_CMS_PRODUCTPARAM_H
#include "productparam_mysql.h" 
#include "cms/include/productparambase.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Productparam : public productparam_mysql<Productparam,productparambase>{
		 public:
		 Productparam(std::string dbtag);
		 Productparam();
		};
	 }
 }
#endif
