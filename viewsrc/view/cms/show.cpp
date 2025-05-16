#include<iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <map> 
#include <vector>
#include <ctime>
#include <array>
#include <sys/stat.h>
#include <cstdlib>
#include "request.h"
#include "datetime.h"
#include "cookie.h"
#include "urlcode.h"
#include "loadviewso.h"
#include "viewso_param.h"
#include "http_so_common_api.h"
#include "viewsrc.h"
//This file create by paozhu Fri, 16 May 2025 06:02:27 GMT
namespace http {

namespace view {
	namespace cms{
 		 std::string show([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <meta name=\"description\" content=\"\">\n    <meta name=\"author\" content=\"Mark Otto, Jacob Thornton, and Bootstrap contributors\">\n    <meta name=\"generator\" content=\"Hugo 0.101.0\">\n    <title>Content Management System</title>\n\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body class=\"text-center\">\n    ";
  			try{ echo<<http::view::home::header(vinfo,obj); }catch (std::exception& e)   {          echo<< "--- include_sub home/header error ---";  }
 			 echo<<"     <h3><a href=\"/cms/list\">列表 Return content list</a> 内容介绍 Content show</h3>\n     \n     <div class=\"container\">\n      <h1 class=\"mt-5\">";
 echo<<obj["title"].as_string(); 
 			 echo<<"</h1>\n      <div>";
 echo<<obj["content"].as_string(); 
 			 echo<<"</div>\n\n    </div>\n\n  </body>\n  </html>";

                    }
                    catch(const char *e)
                    {
                        echo << e;
                        return echo.str();
                    }
                  return echo.str();
             }

         }
        }
       }
    