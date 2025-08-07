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
//This file create by paozhu Thu, 07 Aug 2025 02:38:24 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string martopicside([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>栏目边栏模块</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>  \n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">栏目边栏模块</h5>\n    <hr>\n    <div class=\"row p-3\">\n      <div class=\"col-10\">\n        <table class=\"table table-bordered\">\n          <thead>\n            <tr>\n              <th scope=\"col\">#</th>\n              <th scope=\"col\">名称</th>\n              <th scope=\"col\">类型</th>\n              <th scope=\"col\">排序</th>\n              <th scope=\"col\">编辑</th>\n            </tr>\n          </thead>\n          <tbody>\n            ";
 for(auto &a:obj["list"].as_array()){ 
 			 echo<<"              <tr id=\"topicid_";
 echo<<a["id"].to_string(); 
 			 echo<<"\">\n                <td>\n                  ";
 echo<<a["id"].to_string(); 
 			 echo<<"                </td>\n                <td>\n                  ";
 echo<<a["title"].as_string(); 
 			 echo<<"                </td>\n                <td>\n                  ";
 
                      switch(a["viewtype"].to_int())
                      {
                            case 0:
                                 echo<<"图文内容";
                                 break;
                            case 1:
                                 echo<<"栏目内容";
                                 break;             
                      }
                  
 			 echo<<"                </td>\n                <td>\n                  <input type=\"text\" class=\"form-inline topicsort col-md-3\" id=\"title_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" name=\"title_";
 echo<<a["id"].to_string(); 
 			 echo<<"\" value=\"";
 echo<<a["sortid"].to_string(); 
 			 echo<<"\" onchange=\"updatehomeblocksort(";
 echo<<a["id"].to_string(); 
 			 echo<<",this.value)\">\n                </td>\n                </td>\n                <td>\n\n                  <a href=\"/admin/edittopicsideblock";
 
                  switch(a["viewtype"].to_int())
                  {
                        case 0:
                             echo<<"text";
                             break;
                        case 1:
                             echo<<"pick";
                             break;
           
                  }
              
 			 echo<<"?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\">编辑</a>\n                  \n                  ｜\n                  <a href=\"/admin/deletetopicsideblock?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\" onclick=\"return deletehomeblock(";
 echo<<a["id"].to_string(); 
 			 echo<<")\">删除</a>\n                </td>\n              </tr>\n              ";
 } 
 			 echo<<"          </tbody>\n\n        </table>\n      </div>\n    </div>\n\n\n    <div class=\"row p-3\">\n      <div class=\"col-md-6\">\n             <div class=\"row p-3\">\n              <div class=\"col-md-12\">\n                <label for=\"viewtype\" class=\"form-label\">模块类型</label> \n              </div>\n            </div> \n            <div class=\"row p-3\">\n              <div class=\"col-6\">\n                <select id=\"viewtype\" class=\"form-select\" name=\"viewtype\">\n                  <option  value=\"0\">图文内容</option>\n                  <option  value=\"1\">栏目内容</option>\n                </select>  \n              </div>\n              <div class=\"col-6\">\n\n                  <a href=\"/admin/addtopicsideblock\" class=\"btn btn-primary\" onclick=\"return addblcok();\">添加模块内容</a>\n        \n                \n              </div>\n            </div> \n            <div class=\"row p-3\">\n              <div class=\"col-md-4\">\n                <a href=\"/admin/edittopic?id=";
 echo<<obj["topicid"].to_string(); 
 			 echo<<"\">返回</a>\n              </div>\n            </div> \n      </div>\n      <div class=\"col-md-6\">\n        <img src=\"/images/topicsidedemo.png\">\n      </div>\n    </div>  \n\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  \n  <script lang=\"javascript\">\n      function addblcok()\n    {\n      var blid=jQuery('#viewtype').val();\n      switch(blid)\n      {\n        case \"0\":\n          window.location.href=\"/admin/addtopicsidetext?topicid=";
 echo<<obj["topicid"].to_string(); 
 			 echo<<"\";\n          break;\n        case \"1\":\n          window.location.href=\"/admin/addtopicsidepick?topicid=";
 echo<<obj["topicid"].to_string(); 
 			 echo<<"\";\n          break;         \n        default:\n          window.location.href=\"/admin/addtopicsidetext?topicid=";
 echo<<obj["topicid"].to_string(); 
 			 echo<<"\";\n      }   \n      return false;\n    }\n\n    function deletehomeblock(id)\n    {\n      if(confirm('是否删除?'))\n      {          \n          $.getJSON(\"/admin/deletetopicsideblock?id=\"+id,function(result){\n                if(result.code==0)\n                {\n                \n                     $(\"#topicid_\"+id).remove();\n                }\n                \n          });\n\n      }\n      return false;\n    }\n      function updatehomeblocksort(id,sortid)\n    {\n       $.getJSON(\"/admin/updatetopicsideblocksort?id=\"+id+\"&sortid=\"+sortid,function(result){\n                               \n       });\n    }\n\n  </script>\n</body>\n\n</html>";

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
    