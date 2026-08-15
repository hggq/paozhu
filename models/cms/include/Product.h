#ifndef ORM_CMS_PRODUCT_H
#define ORM_CMS_PRODUCT_H
#include "cms/include/product_opsql.h" 
#include "cms/include/product_base.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Product : public product_opsql<Product,product_base>{
		 public:
		 Product(std::string dbtag_);
		 Product();
		};
	 }
 }
#endif
