
#include "cms/include/article_opsql.h"
#include "cms/include/article_base.h"
#include "cms/include/Article.h"

/* Fri, 14 Aug 2026 13:00:48 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Article::Article(std::string dbtag_):article_opsql(dbtag_){ mod=this; }
			 Article::Article():article_opsql(){ mod=this; }


		} 

	  }
