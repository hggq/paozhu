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
//g++ viewsrc/view/admin/copyright.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string copyright([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>网站信息</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n  <script type=\"text/javascript\" charset=\"utf-8\" src=\"/ueditor/ueditor.config.js\"></script>\n  <script type=\"text/javascript\" charset=\"utf-8\" src=\"/ueditor/ueditor.admin.all.js\"></script>\n  <script type=\"text/javascript\" charset=\"utf-8\" src=\"/ueditor/lang/zh-cn/zh-cn.js\"></script>\n  \n  <style type=\"text/css\">\n   \n\n    .edui-editor {\n      width: 1082px !important;\n      min-height: 300px;\n      height: auto;\n    }\n\n    #edui1_iframeholder {\n      min-height: 200px;\n    }\n\n    body {\n      background-color: #FFFFFF;\n    }\n  </style>\n</head>\n<body>\n\n  <div class=\"container-fluid\">\n    <h5 class=\"card-title mt-2\">版权信息</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n        <form id=\"form\" action=\"/admin/copyrightpost\" method=\"post\">\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-12\">\n              <textarea name=\"copyright\" id=\"copyright\" rows=\"20\"\n                name=\"copyright\">";
echo << obj["info"]["copyright"].to_string(); 
 			 echo<<"</textarea>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-10 offset-sm-2\">\n              <button type=\"submit\" class=\"btn btn-primary\">确认</button>\n              <input type=\"hidden\" id=\"sid\" name=\"sid\" value=\"";
echo<<obj["info"]["sid"].to_string(); 
 			 echo<<"\" />\n            </div>\n          </div>\n\n        </form>\n      </div>\n    </div>\n\n  </div>\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <!-- <script src=\"/nicedit/nicEdit.js\" type=\"text/javascript\"></script> -->\n  <script lang=\"javascript\">\n    var ue = UE.getEditor('copyright',{initialFrameWidth:1080});\n  </script>  \n</body>\n\n</html>";

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
    