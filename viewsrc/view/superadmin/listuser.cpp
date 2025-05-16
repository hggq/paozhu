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
 		 std::string listuser([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>首页</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">";
echo<<obj["info"]["sitename"].to_string(); 
 			 echo<<" 网站管理</h5>\n    <hr>\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <a class=\"btn btn-primary\" href=\"/superadmin/adduser?sid=";
echo<<obj["info"]["sid"].to_string(); 
 			 echo<<"\">添加用户</a>\n      </div>\n    </div>      \n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <table class=\"table table-bordered\">\n\n          <tbody>\n            <tr>\n              <th>#</th>\n              <th>昵称</th>\n              <th>登录名</th>\n              <th>状态</th>\n              <th>操作</th>\n            </tr>\n            ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"            <tr>\n              <td>";
 echo<<a["userid"].to_string(); 
 			 echo<<"</td>\n              <td>";
 echo<<a["nickname"].to_string(); 
 			 echo<<"</td>\n              <td>";
 echo<<a["name"].to_string(); 
 			 echo<<"</td>\n              <td>";
 if(a["isopen"].to_int()==1){ 
 			 echo<<"启用";
 }else{ 
 			 echo<<"关闭";
 } 
 			 echo<<"</td>\n              <td><a href=\"/superadmin/edituser?userid=";
 echo<<a["userid"].to_string(); 
 			 echo<<"\">编辑</a> | <a href=\"/superadmin/deleteuser?userid=";
 echo<<a["userid"].to_string(); 
 			 echo<<"\" onclick=\"return confirm('确定删除？');\">删除</a></td>\n            </tr>\n            ";
 } 
 			 echo<<"          </tbody>\n\n        </table>\n      </div>\n    </div>\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n         <a href=\"/superadmin/welcome\" class=\"btn btn-link rounded-pill px-3\">返回</a>\n      </div>\n    </div>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n</body>\n\n</html>";

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
    