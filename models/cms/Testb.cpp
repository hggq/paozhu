
#include "mysqlmodel.hpp" 
#include "cms/include/testbbase.h"
#include "cms/include/Testb.h"

/* 如果此文件存在不会自动覆盖，没有则会自动生成。
*If this file exists, it will not be overwritten automatically. If not, it will be generated automatically. */

	 
 namespace orm{
	 namespace cms{  

			 Testb::Testb(std::string dbtag):mysqlclientDB(dbtag){}
			 Testb::Testb():mysqlclientDB(){}


		} 

	  }
