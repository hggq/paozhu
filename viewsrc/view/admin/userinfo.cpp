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
//g++ viewsrc/view/admin/userinfo.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string userinfo([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>账号信息</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">账号信息</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n   \n          <div class=\"row mb-3\">\n            <label for=\"sitename\" class=\"col-sm-2 col-form-label text-sm-end\">账户名称</label>\n            <div class=\"col-sm-10\">\n               ";
echo << obj["info"]["name"].to_string(); 
 			 echo<<"            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"domain\" class=\"col-sm-2 col-form-label text-sm-end\">等级</label>\n            <div class=\"col-sm-10\">\n               ";
echo << obj["info"]["level"].to_string(); 
 			 echo<<"            </div>\n          </div>\n\n      </div>\n    </div>\n    \n    <h5 class=\"card-title mt-2\">登录记录</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n   \n           <table class=\"table\">\n              <thead>\n                <tr>\n                  <th scope=\"col\">#</th>\n                  <th scope=\"col\">日期</th>\n                  <th scope=\"col\">地区</th>\n                  <th scope=\"col\">IP</th>\n                </tr>\n              </thead>\n              <tbody>\n              \n                <tr>\n                  <th scope=\"row\">1</th>\n                  <td>2023-05-23</td>\n                  <td>cn</td>\n                  <td>127.0.0.1</td>\n                </tr>\n\n              </tbody>\n            </table>\n\n      </div>\n    </div>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    