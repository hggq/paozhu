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
//g++ viewsrc/view/superadmin/welcome.cpp
namespace http {

namespace view {
	namespace superadmin{
 		 std::string welcome([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>首页</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">网站管理</h5>\n    <hr>\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <table class=\"table table-bordered\">\n\n          <tbody>\n            <tr>\n              <th>#</th>\n              <td>公司</td>\n              <td>网站名称</td>\n              <th>域名</th>\n              <th>状态</th>\n              <th>管理账号</th>\n              <th>操作</th>\n            </tr>\n            ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"            <tr>\n              <td>";
 echo<<a["sid"].to_string(); 
 			 echo<<"</td>\n              <td>";
 echo<<a["companyname"].to_string(); 
 			 echo<<"</td>\n              <td>";
 echo<<a["sitename"].to_string(); 
 			 echo<<"</td>\n              <td>";
 echo<<a["sitedomain"].to_string(); 
 			 echo<<"</td>\n              <td>";
 if(a["isopen"].to_int()==1){ 
 			 echo<<"启用";
 }else{ 
 			 echo<<"关闭";
 } 
 			 echo<<"</td>\n              <td><a href=\"/superadmin/listuser?sid=";
 echo<<a["sid"].to_string(); 
 			 echo<<"\">管理账号</a></td>\n              <td><a href=\"/superadmin/editsiteinfo?sid=";
 echo<<a["sid"].to_string(); 
 			 echo<<"\">编辑</a> | <a href=\"/superadmin/deletesiteinfo?sid=";
 echo<<a["sid"].to_string(); 
 			 echo<<"\">删除</a></td>\n            </tr>\n            ";
 } 
 			 echo<<"          </tbody>\n\n        </table>\n      </div>\n    </div>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n</body>\n\n</html>";

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
    