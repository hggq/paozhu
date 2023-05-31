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
//g++ viewsrc/view/login/login.cpp
namespace http {

namespace view {
	namespace login{
 		 std::string login(const struct view_param &vinfo,http::OBJ_VALUE &obj)
			{
 
                     if(sizeof(vinfo)||obj.is_array())
                     {

                     }
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <meta name=\"description\" content=\"\">\n    <meta name=\"author\" content=\"Mark Otto, Jacob Thornton, and Bootstrap contributors\">\n    <meta name=\"generator\" content=\"Hugo 0.101.0\">\n    <title>Content Management System</title>\n\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body class=\"text-center\">\n    ";
  			try{ echo<<http::view::home::header(vinfo,obj); }catch (std::exception& e)   {          echo<< "--- include_sub home/header error ---";  }
 			 echo<<"    <div class=\"container text-center\">\n      <div class=\"row\">\n        <div class=\"col-3\"></div>  \n        <div class=\"col-6\">\n          <h2 id=\"horizontal-form\">CMS Admin </h2>\n          <form action=\"/cms/loginpost\" method=\"post\">\n            <div class=\"row mb-3\">\n              <label for=\"username\" class=\"col-sm-2 col-form-label\">Username</label>\n              <div class=\"col-sm-10\">\n                <input type=\"text\" class=\"form-control\" id=\"username\" name=\"username\" value=\"admin\">\n              </div>\n            </div>\n            <div class=\"row mb-3\">\n              <label for=\"password\" class=\"col-sm-2 col-form-label\">Password</label>\n              <div class=\"col-sm-10\">\n                <input type=\"password\" class=\"form-control\" id=\"password\" name=\"password\" value=\"123456\">\n              </div>\n            </div>\n\n            <button type=\"submit\" class=\"btn btn-primary\">Sign in</button>\n          </form>\n      \n        </div>\n \n      </div>\n    </div>\n\n  </body>\n  </html>";

                  return echo.str();
             }

         }
        }
       }
    