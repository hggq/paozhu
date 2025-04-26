
#include "article_mysql.h"
#include "cms/include/article_base.h"
#include "cms/include/Article.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Article::Article(std::string dbtag):article_mysql(dbtag){ mod=this; }
			 Article::Article():article_mysql(){ mod=this; }


		} 

	  }
