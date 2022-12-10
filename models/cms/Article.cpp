
#include "mysqlmodel.hpp"
#include "cms/include/articlebase.h"
#include "cms/include/Article.h"
#include "cms/include/User.h"
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
 *If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

namespace orm
{
	namespace cms
	{

		Article::Article(std::string dbtag) : mysqlclientDB(dbtag) {}
		Article::Article() : mysqlclientDB() {}

		std::string Article::get_username(int userid)
		{

			auto User = orm::cms::User();

			User.where("userid", userid).limit(1).fetch();

			return User.getName();
		}
		std::vector<articlebase::meta> get_usertop10article(int userid)
		{

			std::vector<articlebase::meta> a;

			return a;
		}
	}

}
