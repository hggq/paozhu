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
 		 std::string edit([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <meta name=\"description\" content=\"\">\n    <meta name=\"author\" content=\"Mark Otto, Jacob Thornton, and Bootstrap contributors\">\n    <meta name=\"generator\" content=\"Hugo 0.101.0\">\n    <title>Content Management System</title>\n\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body class=\"text-center\">\n    ";
  			try{ echo<<http::view::home::header(vinfo,obj); }catch (std::exception& e)   {          echo<< "--- include_sub home/header error ---";  }
 			 echo<<"     <h3><a href=\"/cms/list\">返回列表 Return content list</a> 内容编辑 Content edit</h3>\n     \n \n\n    <div class=\"container text-center\">\n      <div class=\"row\">\n        <div class=\"col-3\"></div>  \n        <div class=\"col-6\">\n          <h2 id=\"horizontal-form\">";
 echo<<obj["title"].as_string(); 
 			 echo<<"</h2>\n          <form action=\"/cms/editpost?id=";
 echo<<obj["aid"].to_string(); 
 			 echo<<"\" method=\"post\">\n\n    <div class=\"mb-3\">\n      <label for=\"title\" class=\"form-label\">内容标题 Title</label>\n      <input type=\"text\" class=\"form-control\" id=\"title\" name=\"title\" placeholder=\"标题 title\" value=\"";
 echo<<obj["title"].to_string(); 
 			 echo<<"\">\n    </div>\n    <div class=\"mb-3\">\n      <label for=\"content\" class=\"form-label\">内容 Content</label>\n      <textarea class=\"form-control\" id=\"content\" name=\"content\"  rows=\"20\">";
 echo<<obj["content"].to_string(); 
 			 echo<<"</textarea>\n    </div>\n    <input type=\"hidden\" id=\"aid\" name=\"aid\"  value=\"";
 echo<<obj["aid"].to_string(); 
 			 echo<<"\">\n    <button type=\"submit\" class=\"btn btn-primary\">提交(Submit)</button>\n  </form>\n\n</div>\n\n</div>\n</div>\n\n  </body>\n  </html>";

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
    