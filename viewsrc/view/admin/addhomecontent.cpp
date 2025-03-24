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
//g++ viewsrc/view/admin/addhomecontent.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string addhomecontent([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>显示栏目</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n</head>\n\n<body>\n\n  <div class=\"container\">\n    <form class=\"g-3 mt-1\" id=\"form\" method=\"post\">\n      <h3>显示栏目</h3>\n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"title\" class=\"form-label\">模块名称</label>\n          <input type=\"text\" class=\"form-control\" id=\"title\" name=\"title\" value=\"";
echo<<obj["info"]["title"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"title\" class=\"form-label\">显示条目数</label>\n          <input type=\"text\" class=\"form-control\" id=\"rownum\" name=\"rownum\" value=\"";
echo<<obj["info"]["rownum"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"title\" class=\"form-label\">选择的栏目</label>\n          <input type=\"text\" class=\"form-control\" id=\"topicid_content\" name=\"content\" value=\"";
echo<<obj["info"]["content"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n      <div class=\"row\">\n        ";
 for(auto &a:obj["list"].as_array()){ 
 			 echo<<"          <div class=\"row g-3 mt-1\">\n                <div class=\"col-md-2\">\n                  ";
echo<<a["title"].to_string(); 
 			 echo<<"              </div>\n              <div class=\"col-md-2\">\n                <input class=\"form-check-input topicidlist\" type=\"checkbox\" role=\"switch\" name=\"topicid[]\" id=\"topicid_";
echo<<a["topicid"].to_string(); 
 			 echo<<"\" onclick=\"checktopicinsert(this)\" value=\"";
echo<<a["topicid"].to_string(); 
 			 echo<<"\">\n              </div>\n              \n          </div>\n           ";
 } 
 			 echo<<" \n \n      </div>  \n       \n      <div class=\"row\" style=\"margin-top: 20px;\">\n        <div class=\"col-md-6\">\n        <br />\n        <input type=\"hidden\" name=\"hbid\" value=\"";
echo<<obj["info"]["hbid"].to_string(); 
 			 echo<<"\">\n        <input type=\"hidden\" name=\"viewtype\" value=\"3\">\n        <button type=\"submit\" class=\"btn btn-primary\">添加</button>\n        <a href=\"/admin/marhome\" class=\"btn btn-link rounded-pill px-3\">返回</a>\n        </div>\n      </div>\n    </form>\n\n  </div>\n  \n \n \n<div class=\"modal fade\" id=\"myfiletobigModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"staticBackdropLabel\" aria-hidden=\"true\">\n  <div class=\"modal-dialog\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"staticBackdropLabel\">提示</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n         文件太大了，请传4M以下的文件！\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">关闭</button>\n         \n      </div>\n    </div>\n  </div>\n</div> \n\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n\n     function gotourl()\n    {\n        window.location.href='/admin/marhome';\n    }\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n      var data = $(this).serialize();\n\n      var obj=this;  \n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/addhomeblockpost\",\n        method: 'post',\n        contentType:\"application/x-www-form-urlencoded\",\n        data: data,\n        dataType: \"json\"\n        }).done(function(data) {\n          if(data.code==0)\n          {\n             setTimeout(\"gotourl()\",500 );  \n          }\n        });\n    });\n    \n    function checktopicinsert(obj)\n    {\n \n      var topicidstr=$(\"#topicid_content\").val();\n      if(jQuery(obj).is(\":checked\"))\n        {\n            if(topicidstr.length>0)\n            {\n              topicidstr=topicidstr+\",\"+jQuery(obj).val();\n            }else{\n              topicidstr=jQuery(obj).val();\n            }\n            \n        }else{\n          var artArray = topicidstr.split(\",\");   \n          for (var i = 0; i < artArray.length; i++) {\n            if (artArray[i] == jQuery(obj).val()) {\n              artArray.splice(i, 1);  //删除第i项\n            }\n          }\n          topicidstr = artArray.join(\",\");\n        }\n        $(\"#topicid_content\").val(topicidstr);\n    }\n    \n    \n \n\n    $(document).ready(function(e) {\n         \n\n \n    });\n  </script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    