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
	namespace admin{
 		 std::string footscript([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>网站信息</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n<body>\n\n  <div class=\"container-fluid\">\n    <h5 class=\"card-title mt-2\">底部Javascript代码</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n        <form id=\"form\" action=\"/admin/footscriptpost\" method=\"post\">\n          <div class=\"row mb-3\">\n            <div class=\"ml-2\">（需要用&lt;script&gt;闭合js代码）</div>\n          </div>\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-12\">\n              <textarea class=\"form-control\" name=\"footscript\" id=\"footscript\" rows=\"20\"\n                name=\"footscript\">";
echo << obj["info"]["footscript"].to_string(); 
 			 echo<<"</textarea>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-10 offset-sm-2\">\n              <button type=\"submit\" class=\"btn btn-primary\">确认</button>\n              <input type=\"hidden\" id=\"sid\" name=\"sid\" value=\"";
echo<<obj["info"]["sid"].to_string(); 
 			 echo<<"\" />\n            </div>\n          </div>\n\n        </form>\n      </div>\n    </div>\n\n  </div>\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n</body>\n\n</html>";

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
    