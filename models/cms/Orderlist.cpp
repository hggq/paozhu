
#include "orm/cms/include/orderlist_opsql.h"
#include "orm/cms/include/orderlist_base.h"
#include "models/cms/include/Orderlist.h"

/* Sun, 20 Sep 2026 12:34:54 GMT */
/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  
			 Orderlist::Orderlist(std::string dbtag_):orderlist_opsql(dbtag_){ mod=this; }
			 Orderlist::Orderlist():orderlist_opsql(){ mod=this; }


		} 

	  }
