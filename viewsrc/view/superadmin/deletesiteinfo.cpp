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
	namespace superadmin{
 		 std::string deletesiteinfo([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n \n  <title>网站信息</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">删除网站</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n        <p>公司名称：";
echo << obj["info"]["companyname"].to_string(); 
 			 echo<<"</p>\n        <p>网站名称：";
echo << obj["info"]["sitename"].to_string(); 
 			 echo<<"</p>\n      </div>\n    </div>\n    <div class=\"row\">\n      <div class=\"col-12\">\n            你确定要删除网站吗？数据将永久删除。\n      </div>\n    </div>\n    <div class=\"row\">\n      <div class=\"col-12\">\n        <form id=\"form\" action=\"/superadmin/deletesiteinfopost\" method=\"post\">\n          <div class=\"row mb-3\">\n            <label for=\"username\" class=\"col-sm-2 col-form-label text-sm-end\">超级管理员用户名</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" name=\"username\" id=\"username\" placeholder=\"用户名\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"password\" class=\"col-sm-2 col-form-label text-sm-end\">超级管理员密码</label>\n            <div class=\"col-sm-10\">\n              <input type=\"password\" class=\"form-control\" name=\"password\" id=\"password\" placeholder=\"密码\">\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-10 offset-sm-2\">\n              <button type=\"submit\" class=\"btn btn-primary\">确定删除</button>\n              <input type=\"hidden\" id=\"sid\" name=\"sid\" value=\"";
echo<<obj["info"]["sid"].to_string(); 
 			 echo<<"\" />\n                <a href=\"/superadmin/welcome\" class=\"btn btn-link rounded-pill px-3\">返回</a>\n            </div>\n          </div>\n\n        </form>\n      </div>\n    </div>\n  </div>\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n \n</body>\n\n</html>";

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
    