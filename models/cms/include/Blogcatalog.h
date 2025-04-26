#ifndef ORM_CMS_BLOGCATALOG_H
#define ORM_CMS_BLOGCATALOG_H
#include "blogcatalog_mysql.h" 
#include "cms/include/blogcatalog_base.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Blogcatalog : public blogcatalog_mysql<Blogcatalog,blogcatalog_base>{
		 public:
		 Blogcatalog(std::string dbtag);
		 Blogcatalog();
		};
	 }
 }
#endif
