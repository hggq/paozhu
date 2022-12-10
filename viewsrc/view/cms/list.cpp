#include<iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <map> 
#include <vector>
#include <ctime>
#include <array>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include "request.h"
#include "datetime.h"
#include "cookie.h"
#include "urlcode.h"
#include "loadviewso.h"
#include "viewso_param.h"
#include "http_so_common_api.h"
#include "viewsrc.h"
//g++ viewsrc/view/cms/list.cpp
namespace http {

namespace view {
	namespace cms{
 		 std::string list(const struct view_param &vinfo,http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <meta name=\"description\" content=\"\">\n    <meta name=\"author\" content=\"Mark Otto, Jacob Thornton, and Bootstrap contributors\">\n    <meta name=\"generator\" content=\"Hugo 0.101.0\">\n    <title>Content Management System</title>\n\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body class=\"text-center\">\n    ";
  			try{ echo<<http::view::home::header(vinfo,obj); }catch (std::exception& e)   {          echo<< "homeheader too long";  }
 			 echo<<"     <h3>内容列表 <a href=\"/cms/add\">添加内容</a></h3>\n     <p>";
 echo<<obj["msg"].as_string(); 
 			 echo<<"</p>\n    <div class=\"list-group w-auto\">\n      ";

      for(auto &a:obj["list"].as_array()){
      
 			 echo<<"      <div class=\"list-group-item list-group-item-action d-flex gap-3 py-3\" aria-current=\"true\">\n        <img src=\"#\" alt=\"twbs\" class=\"rounded-circle flex-shrink-0\" width=\"32\" height=\"32\">\n        <div class=\"d-flex gap-2 w-100 justify-content-between\">\n          <div>\n            <h6 class=\"mb-0\"><a href=\"/cms/show?id=";
 echo<<a.second["aid"].to_string(); 
 			 echo<<"\">";
 echo<<a.second["title"].as_string(); 
 			 echo<<"</a></h6>\n            <p class=\"mb-0 opacity-75\">";
 echo<<a.second["summary"].as_string(); 
 			 echo<<"</p>\n          </div>\n          <small class=\"opacity-50 text-nowrap\">";
 echo<<a.second["createtime"].as_string(); 
 			 echo<<"</small>\n\n          <small class=\"opacity-50 text-nowrap\"><a href=\"/cms/edit?id=";
 echo<<a.second["aid"].to_string(); 
 			 echo<<"\">编辑</a></small>\n          <small class=\"opacity-50 text-nowrap\"><a href=\"/cms/delete?id=";
 echo<<a.second["aid"].to_string(); 
 			 echo<<"\">删除</a></small>\n        </div>\n      </div>\n      ";

      }
      
 			 echo<<"  \n       \n    </div>\n\n  </body>\n  </html>";

                  return echo.str();
             }

         }
        }
       }
    