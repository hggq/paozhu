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
//g++ viewsrc/view/cms/show.cpp
namespace http {

namespace view {
	namespace cms{
 		 std::string show(const struct view_param &vinfo,http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <meta name=\"description\" content=\"\">\n    <meta name=\"author\" content=\"Mark Otto, Jacob Thornton, and Bootstrap contributors\">\n    <meta name=\"generator\" content=\"Hugo 0.101.0\">\n    <title>Content Management System</title>\n\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body class=\"text-center\">\n    ";
  			try{ echo<<http::view::home::header(vinfo,obj); }catch (std::exception& e)   {          echo<< "homeheader too long";  }
 			 echo<<"     <h3><a href=\"/cms/list\">列表</a> 内容介绍</h3>\n     \n     <div class=\"container\">\n      <h1 class=\"mt-5\">";
 echo<<obj["title"].as_string(); 
 			 echo<<"</h1>\n      <div>";
 echo<<obj["content"].as_string(); 
 			 echo<<"</div>\n\n    </div>\n\n  </body>\n  </html>";

                  return echo.str();
             }

         }
        }
       }
    