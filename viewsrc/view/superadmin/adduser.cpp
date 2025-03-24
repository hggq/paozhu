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
//g++ viewsrc/view/superadmin/adduser.cpp
namespace http {

namespace view {
	namespace superadmin{
 		 std::string adduser([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n \n  <title>网站信息</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">";
echo<<obj["info"]["sitename"].to_string(); 
 			 echo<<" 添加用户</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n        <form id=\"form\" action=\"/superadmin/adduserpost\" method=\"post\">\n          <div class=\"row mb-3\">\n            <label for=\"nickname\" class=\"col-sm-2 col-form-label text-sm-end\">昵称</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"nickname\" name=\"nickname\" value=\"";
echo << obj["info"]["nickname"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"username\" class=\"col-sm-2 col-form-label text-sm-end\">登录名</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"username\" name=\"username\" value=\"";
echo << obj["info"]["username"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"password\" class=\"col-sm-2 col-form-label text-sm-end\">密码</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"password\" name=\"password\" value=\"";
echo << obj["info"]["password"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label for=\"mobile\" class=\"col-sm-2 col-form-label text-sm-end\">手机号</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"mobile\" name=\"mobile\" value=\"";
echo << obj["info"]["mobile"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label for=\"email\" class=\"col-sm-2 col-form-label text-sm-end\">电子邮箱</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"email\" name=\"email\" value=\"";
echo << obj["info"]["email"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label for=\"expectdate\" class=\"col-sm-2 col-form-label text-sm-end\">到期日期</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"expectdate\" name=\"expectdate\" placeholder=\"yyyy-mm-dd\" value=\"";
echo << obj["info"]["expectdate"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"isopen\" class=\"col-sm-2 col-form-label text-sm-end\">启用状态</label>\n            <div class=\"col-sm-10\">\n                <div class=\"form-check\">\n                  <input class=\"form-check-input\" type=\"checkbox\" value=\"1\" name=\"isopen\" id=\"isopen\" ";
 if(obj["info"]["isopen"].to_int()==1){ 
 			 echo<<"checked=\"checked\"";
 } 
 			 echo<<">\n                </div>                \n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-10 offset-sm-2\">\n              <button type=\"submit\" class=\"btn btn-primary\">添加</button>\n              <input type=\"hidden\" id=\"sid\" name=\"sid\" value=\"";
echo<<obj["info"]["sid"].to_string(); 
 			 echo<<"\" />\n                <a href=\"/superadmin/welcome\" class=\"btn btn-link rounded-pill px-3\">返回</a>\n            </div>\n          </div>\n\n        </form>\n      </div>\n    </div>\n\n  </div>\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n \n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    