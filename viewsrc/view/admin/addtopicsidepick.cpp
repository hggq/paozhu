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
 		 std::string addtopicsidepick([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>栏目边栏栏目提取显示</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n\n  \n  <style type=\"text/css\">\n   \n\n    .edui-editor {\n      width: 781px !important;\n      min-height: 300px;\n      height: auto;\n    }\n\n    #edui1_iframeholder {\n      min-height: 200px;\n    }\n\n    body {\n      background-color: #FFFFFF;\n    }\n  </style>\n</head>\n\n<body>\n\n  <div class=\"container\">\n    <form class=\"g-3 mt-1\" id=\"form\" method=\"post\">\n      <h3>栏目边栏栏目提取显示</h3>\n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"title\" class=\"form-label\">模块名称</label>\n          <input type=\"text\" class=\"form-control\" id=\"title\" name=\"title\" value=\"";
echo<<obj["info"]["title"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"rownum\" class=\"form-label\">行数</label>\n          <input type=\"text\" class=\"form-control\" id=\"rownum\" name=\"rownum\" value=\"";
echo<<obj["info"]["rownum"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"picktopicid\" class=\"form-label\">选择提取栏目名</label>\n          <select name=\"picktopicid\" class=\"form-control\" id=\"picktopicid\">\n            <option value=\"0\">选择提取栏目名</option>\n            ";
 for(auto &a:obj["list"].as_array()){ 
 			 echo<<"            <option value=\"";
 echo<<a["id"].to_string(); 
 			 echo<<"\" ";
 if(a["parentid"].to_int()>0){ 
 			 echo<<"style=\"padding-left:";
 echo<<(a["level"].to_int()*15+15); 
 			 echo<<"px\"";
 } 
 			 echo<<">";
 echo<<a["value"].as_string(); 
 			 echo<<"</option>\n            ";
 } 
 			 echo<<"  \n          </select>\n        </div>\n      </div>  \n\n       \n      <div class=\"row\" style=\"margin-top: 20px;\">\n        <div class=\"col-md-6\">\n        <br />\n        <input type=\"hidden\" name=\"topicsetid\" value=\"";
echo<<obj["info"]["topicsetid"].to_string(); 
 			 echo<<"\">\n        <input type=\"hidden\" name=\"topicid\" value=\"";
 echo<<obj["topicid"].to_string(); 
 			 echo<<"\">\n        <input type=\"hidden\" name=\"viewtype\" value=\"1\">\n        <button type=\"submit\" class=\"btn btn-primary\">添加</button>\n        <a href=\"/admin/edittopicside?topicid=";
 echo<<obj["topicid"].to_string(); 
 			 echo<<"\" class=\"btn btn-link rounded-pill px-3\">返回</a>  \n        </div>\n      </div>\n    </form>\n\n  </div>\n  \n \n \n<div class=\"modal fade\" id=\"myfiletobigModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"staticBackdropLabel\" aria-hidden=\"true\">\n  <div class=\"modal-dialog\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"staticBackdropLabel\">提示</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n         文件太大了，请传4M以下的文件！\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">关闭</button>\n         \n      </div>\n    </div>\n  </div>\n</div> \n\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n\n     function gotourl()\n    {\n        window.location.href='/admin/edittopicside?topicid=";
 echo<<obj["topicid"].to_string(); 
 			 echo<<"';\n    }\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n      var data = $(this).serialize();\n\n      var obj=this;  \n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/addtopicsideblockpost\",\n        method: 'post',\n        contentType:\"application/x-www-form-urlencoded\",\n        data: data,\n        dataType: \"json\"\n        }).done(function(data) {\n          if(data.code==0)\n          {\n             setTimeout(\"gotourl()\",500 );  \n          }\n        });\n    });\n    \n \n    \n \n\n    $(document).ready(function(e) {\n         \n\n \n    });\n  </script>\n</body>\n\n</html>";

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
    