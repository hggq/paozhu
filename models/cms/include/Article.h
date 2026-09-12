#ifndef ORM_CMS_ARTICLE_H
#define ORM_CMS_ARTICLE_H
#include "orm/cms/include/article_opsql.h" 
#include "orm/cms/include/article_base.h"

/* Sat, 12 Sep 2026 07:38:47 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

 namespace orm {
	namespace cms { 
		class Article : public article_opsql<Article,article_base>{
		 public:
		 Article(std::string dbtag_);
		 Article();
		};
	 }
 }
#endif
