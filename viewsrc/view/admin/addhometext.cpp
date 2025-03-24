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
//g++ viewsrc/view/admin/addhometext.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string addhometext([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>显示栏目</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link rel=\"stylesheet\" href=\"/libs/tree/select-tree.css\">\n \n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/libs/tree/select-tree.js\"></script>\n  <script type=\"text/javascript\" charset=\"utf-8\" src=\"/ueditor/ueditor.config.js\"></script>\n  <script type=\"text/javascript\" charset=\"utf-8\" src=\"/ueditor/ueditor.all.js\"> </script>\n  <script type=\"text/javascript\" charset=\"utf-8\" src=\"/ueditor/lang/zh-cn/zh-cn.js\"></script>\n  \n  <style type=\"text/css\">\n   \n\n    .edui-editor {\n      width: 781px !important;\n      min-height: 300px;\n      height: auto;\n    }\n\n    #edui1_iframeholder {\n      min-height: 200px;\n    }\n\n    body {\n      background-color: #FFFFFF;\n    }\n  </style>\n</head>\n\n<body>\n\n  <div class=\"container\">\n    <form class=\"g-3 mt-1\" id=\"form\" method=\"post\">\n      <h3>显示栏目</h3>\n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"title\" class=\"form-label\">模块名称</label>\n          <input type=\"text\" class=\"form-control\" id=\"title\" name=\"title\" value=\"";
echo<<obj["info"]["title"].to_string(); 
 			 echo<<"\">\n        </div>\n      </div>  \n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <label for=\"title\" class=\"form-label\">图文内容</label>\n        </div>\n      </div>  \n      <div class=\"row\">\n        <div class=\"col-md-6\">\n          <textarea id=\"editor\" style=\"width:830px\" rows=\"10\" name=\"content\">";
echo << obj["info"]["content"].to_string(); 
 			 echo<<"</textarea>\n        </div>\n      </div>  \n \n       \n      <div class=\"row\" style=\"margin-top: 20px;\">\n        <div class=\"col-md-6\">\n        <br />\n        <input type=\"hidden\" name=\"hbid\" value=\"";
echo<<obj["info"]["hbid"].to_string(); 
 			 echo<<"\">\n        <input type=\"hidden\" name=\"viewtype\" value=\"0\">\n        <button type=\"submit\" class=\"btn btn-primary\">添加</button>\n        <a href=\"/admin/marhome\" class=\"btn btn-link rounded-pill px-3\">返回</a>  \n        </div>\n      </div>\n    </form>\n\n  </div>\n  \n \n \n<div class=\"modal fade\" id=\"myfiletobigModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"staticBackdropLabel\" aria-hidden=\"true\">\n  <div class=\"modal-dialog\">\n    <div class=\"modal-content\">\n      <div class=\"modal-header\">\n        <h1 class=\"modal-title fs-5\" id=\"staticBackdropLabel\">提示</h1>\n        <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n      </div>\n      <div class=\"modal-body\">\n         文件太大了，请传4M以下的文件！\n      </div>\n      <div class=\"modal-footer\">\n        <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">关闭</button>\n         \n      </div>\n    </div>\n  </div>\n</div> \n\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n    var ue = UE.getEditor('editor');\n     function gotourl()\n    {\n        window.location.href='/admin/marhome';\n    }\n    $(\"#form\").on(\"submit\", function (event) {\n      event.preventDefault();\n      var data = $(this).serialize();\n\n      var obj=this;  \n      //得到序列化数据\n      $.ajax({\n        url: \"/admin/addhomeblockpost\",\n        method: 'post',\n        contentType:\"application/x-www-form-urlencoded\",\n        data: data,\n        dataType: \"json\"\n        }).done(function(data) {\n          if(data.code==0)\n          {\n             setTimeout(\"gotourl()\",500 );  \n          }\n        });\n    });\n    \n \n    \n \n\n    $(document).ready(function(e) {\n         \n\n \n    });\n  </script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    