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
//g++ viewsrc/view/cms/add.cpp
namespace http {

namespace view {
	namespace cms{
 		 std::string add(const struct view_param &vinfo,http::OBJ_VALUE &obj)
			{
 
                     if(sizeof(vinfo)||obj.is_array())
                     {

                     }
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <meta name=\"description\" content=\"\">\n    <meta name=\"author\" content=\"Mark Otto, Jacob Thornton, and Bootstrap contributors\">\n    <meta name=\"generator\" content=\"Hugo 0.101.0\">\n    <title>Content Management System</title>\n\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body class=\"text-center\">\n    ";
  			try{ echo<<http::view::home::header(vinfo,obj); }catch (std::exception& e)   {          echo<< "--- include_sub home/header error ---";  }
 			 echo<<"     <h3><a href=\"/cms/list\">返回列表 Return content list</a> </h3>\n     \n \n\n    <div class=\"container text-center\">\n      <div class=\"row\">\n        <div class=\"col-3\"></div>  \n        <div class=\"col-6\">\n          <h2 id=\"horizontal-form\">添加新内容 Add new content</h2>\n          <form action=\"/cms/addpost\" method=\"post\">\n\n    <div class=\"mb-3\">\n      <label for=\"title\" class=\"form-label\">内容标题 Title</label>\n      <input type=\"text\" class=\"form-control\" id=\"title\" name=\"title\" placeholder=\"标题 title\" value=\"\">\n    </div>\n    <div class=\"mb-3\">\n      <label for=\"content\" class=\"form-label\">内容 Content</label>\n      <textarea class=\"form-control\" id=\"content\" name=\"content\"  rows=\"20\"></textarea>\n    </div>\n    <input type=\"hidden\" id=\"aid\" name=\"aid\"  value=\"0\">\n    <button type=\"submit\" class=\"btn btn-primary\">提交(Submit)</button>\n  </form>\n\n</div>\n\n</div>\n</div>\n\n  </body>\n  </html>";

                  return echo.str();
             }

         }
        }
       }
    