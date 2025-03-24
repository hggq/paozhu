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
//g++ viewsrc/view/superadmin/editpassword.cpp
namespace http {

namespace view {
	namespace superadmin{
 		 std::string editpassword([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n \n  <title>Edit password</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">修改密码</h5>\n    <hr>\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <form class=\"row g-3\" id=\"form\" action=\"/superadmin/editpwdpost\" method=\"post\">\n          <div class=\"col-12\">\n            <label for=\"oldpassword\" class=\"form-label\">旧密码</label>\n            <input type=\"password\" class=\"form-control\" name=\"oldpassword\" id=\"oldpassword\" placeholder=\"旧密码\">\n          </div>\n          <div class=\"col-md-6\">\n            <label for=\"newpassword\" class=\"form-label\">新密码</label>\n            <input type=\"password\" class=\"form-control\" name=\"newpassword\" id=\"newpassword\" placeholder=\"新密码，请保持密码8位数以上\">\n          </div>\n          <div class=\"col-md-6\">\n            <label for=\"newpasswordconfirm\" class=\"form-label\">确认密码</label>\n            <input type=\"password\" class=\"form-control\" name=\"newpasswordconfirm\" id=\"newpasswordconfirm\" placeholder=\"确认新密码\">\n          </div>\n          <div class=\"col-12\">\n            <button type=\"submit\" class=\"btn btn-primary\">确认</button>\n          </div>\n        </form>\n        </div>\n    </div>\n\n  </div>\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script> \n  <script lang=\"javascript\">\n      $(\"#form\").on(\"submit\", function (event) {\n      \n      if(jQuery(\"#newpassword\").val()!=jQuery(\"#newpasswordconfirm\").val())  \n      {\n           alert(\"前后密码不相同！\");\n           return;\n      }\n      \n      event.preventDefault();\n\n      var data = $(this).serialize();\n\n      var obj = this;\n\n      //得到序列化数据\n      $.ajax({\n        url: \"/superadmin/editpwdpost\",\n        method: 'post',\n        data: data,\n        success: function (res) {\n             \n                alert(res.msg);\n             \n        }\n      })\n    });\n\n  </script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    