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
//g++ viewsrc/view/admin/welcome.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string welcome(const struct view_param &vinfo,http::OBJ_VALUE &obj)
			{
 
                     if(sizeof(vinfo)||obj.is_array())
                     {

                     }
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>首页</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">网站信息</h5>\n    <hr>\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <table class=\"table table-bordered\">\n\n          <tbody>\n            <tr>\n              <th scope=\"row\">一周内文章数</th>\n              <td>";
echo << obj["lastweektotal"].to_string(); 
 			 echo<<"</td>\n              <th scope=\"row\">文章总数</th>\n              <td>";
echo << obj["total"].to_string(); 
 			 echo<<"</td>\n            </tr>\n            <tr>\n              <th scope=\"row\">Paozhu版本</th>\n              <td colspan=\"2\">1.3.3</td>\n              <td>20230426</td>\n            </tr>\n          </tbody>\n\n        </table>\n      </div>\n    </div>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    