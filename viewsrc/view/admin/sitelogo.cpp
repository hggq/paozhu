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
//g++ viewsrc/view/admin/sitelogo.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string sitelogo([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>网站信息</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <script src=\"/js/jquery.min.js\"></script>\n</head>\n<body>\n\n  <div class=\"container-fluid\">\n    <h5 class=\"card-title mt-2\">网站LOGO</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n        <form id=\"form\" action=\"/admin/sitelogopost\" method=\"post\">\n          <div class=\"row mb-3\">\n            <div class=\"ml-2\">（格式为png、svg或jpg）</div>\n          </div>\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-10 offset-sm-2 ml-2\"><div id=\"progress-bar-result\">\n              ";
std::string templogourl=obj["info"]["sitelogo"].to_string();  if(templogourl.size()>0){ 
 			 echo<<"              <a href=\"";
echo << obj["info"]["sitelogo"].to_string(); 
 			 echo<<"?";
echo << timeid(); 
 			 echo<<"\" target=\"_blank\"><img src=\"";
echo << obj["info"]["sitelogo"].to_string(); 
 			 echo<<"?";
echo << timeid(); 
 			 echo<<"\"></a>\n              ";
} 
 			 echo<<"             </div></div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"sitelogo\" class=\"col-sm-2 col-form-label text-sm-end\">选择图片</label>\n            <div class=\"col-sm-6\">\n              <div class=\"input-group mb-3\">\n              <input type=\"text\" class=\"form-control\" id=\"sitelogo\" name=\"sitelogo\" value=\"";
echo << obj["info"]["sitelogo"].to_string(); 
 			 echo<<"\" placeholder=\"网站logo\">\n              <input class=\"form-control\" type=\"file\" style=\"width:100px\" id=\"relate-sitelogo\" accept=\".png,.jpg,.jpeg\">\n              </div>\n            </div>\n            <div class=\"col-sm-4\">\n              <div class=\"progress\" id=\"progress-bar-show\" style=\"display:none\" role=\"progressbar\" aria-valuenow=\"0\" aria-valuemin=\"0\" aria-valuemax=\"100\">\n                  <div class=\"progress-bar\" id=\"progress-bar\" style=\"width: 0%\">0</div>\n                </div>\n              \n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <div class=\"ml-2 col-sm-10 offset-sm-2\">\n            <div class=\"form-check\">\n              <input class=\"form-check-input\" type=\"checkbox\" value=\"1\" id=\"uplogotoimages\" name=\"uplogotoimages\">\n              <label class=\"form-check-label\" for=\"uplogotoimages\">\n                LOGO更新到 /images/logo.jpg 、 /images/logo.svg 或 /images/logo.png 位置\n              </label>\n            </div>   \n          </div>         \n          </div>\n\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-10 offset-sm-2\">\n              <button type=\"submit\" class=\"btn btn-primary\">确认</button>\n              <input type=\"hidden\" id=\"sid\" name=\"sid\" value=\"";
echo<<obj["info"]["sid"].to_string(); 
 			 echo<<"\" />\n            </div>\n          </div>\n\n        </form>\n      </div>\n    </div>\n\n  </div>\n  \n  <div class=\"modal fade\" id=\"myfiletobigModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"staticBackdropLabel\" aria-hidden=\"true\">\n    <div class=\"modal-dialog\">\n      <div class=\"modal-content\">\n        <div class=\"modal-header\">\n          <h1 class=\"modal-title fs-5\" id=\"staticBackdropLabel\">提示</h1>\n          <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n        </div>\n        <div class=\"modal-body\">\n           文件太大了，请传4M以下的文件！\n        </div>\n        <div class=\"modal-footer\">\n          <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">关闭</button>\n           \n        </div>\n      </div>\n    </div>\n  </div>\n\n  <div class=\"modal fade\" id=\"myfilesuccessModal\" data-bs-backdrop=\"static\" data-bs-keyboard=\"false\" tabindex=\"-1\" aria-labelledby=\"myfilesuccess\" aria-hidden=\"true\">\n    <div class=\"modal-dialog\">\n      <div class=\"modal-content\">\n        <div class=\"modal-header\">\n          <h1 class=\"modal-title fs-5\" id=\"myfilesuccess\">提示</h1>\n          <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"Close\"></button>\n        </div>\n        <div class=\"modal-body\">\n           上传成功！\n        </div>\n        <div class=\"modal-footer\">\n          <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">确定</button>\n           \n        </div>\n      </div>\n    </div>\n  </div>\n  \n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script lang=\"javascript\">\n\n$(document).ready(function(e) {\n  \n        $(\"#relate-sitelogo\").change(function(){\n            var fd=new FormData();\n            if($(this).get(0).files[0].size>4596640){\n                $('#myfiletobigModal').modal('show');\n                return false;\n            }\n            fd.append(\"upfile\",$(this).get(0).files[0]);\n            $(\"#progress-bar-show\").show();\n            $(\"#progress-bar-result\").hide();\n            $.ajax({\n                url:\"/imageapi/upload?action=uploadimage\",\n                type:\"POST\",\n                processData: false,\n                contentType:false,\n                data: fd,\n                dataType: \"json\",\n                xhr: function() {  // custom xhr\n                    myXhr = $.ajaxSettings.xhr();\n                    if(myXhr.upload){ // check if upload property exists\n                        myXhr.upload.addEventListener('progress',function(evt){\n                            var percentComplete = Math.round(evt.loaded*100 / evt.total);\n                            $( \"#progress-bar\").css(\"width\",percentComplete*2);\n                            $( \"#progress-bar\").html(percentComplete+'%');\n                        }, false); // for handling the progress of the upload\n                    }\n                    return myXhr;\n                },\n                success:function(data){\n                    if(data.state==\"SUCCESS\"){\n                        $(\"#progress-bar-show\").hide();\n                        $(\"#progress-bar-result\").show();\n                        $(\"#sitelogo\").val(data.url);\n\t\t\t\t\t\t$(\"#progress-bar-result\").html(\"<a href='\"+data.url+\"' target=_blank><img src=\\\"\"+data.url+\"\\\"/></a>\");\n                       $('#myfilesuccessModal').modal('show');\n                    }\n                     \n                }\n            });\n        });\n    });\t  \n  </script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    