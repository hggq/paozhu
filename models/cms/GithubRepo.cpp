
#include "orm/cms/include/github_repo_opsql.h"
#include "orm/cms/include/github_repo_base.h"
#include "models/cms/include/GithubRepo.h"

/* Thu, 29 Oct 2026 11:07:24 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 GithubRepo::GithubRepo(std::string dbtag_):github_repo_opsql(dbtag_){ mod=this; }
			 GithubRepo::GithubRepo():github_repo_opsql(){ mod=this; }


		} 

	  }
