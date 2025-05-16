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
 		 std::string addsiteinfo([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n \n  <title>网站信息</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">添加网站</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n        <form id=\"form\" action=\"/superadmin/addsiteinfopost\" method=\"post\">\n          <div class=\"row mb-3\">\n            <label for=\"companyname\" class=\"col-sm-2 col-form-label text-sm-end\">公司名称</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"companyname\" name=\"companyname\" value=\"";
echo << obj["info"]["companyname"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"linkname\" class=\"col-sm-2 col-form-label text-sm-end\">联系人</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"linkname\" name=\"linkname\" value=\"";
echo << obj["info"]["linkname"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"linkmobile\" class=\"col-sm-2 col-form-label text-sm-end\">电话</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"linkmobile\" name=\"linkmobile\" value=\"";
echo << obj["info"]["linkmobile"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"sitename\" class=\"col-sm-2 col-form-label text-sm-end\">网站名称</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"sitename\" name=\"sitename\" value=\"";
echo << obj["info"]["sitename"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"sitedomain\" class=\"col-sm-2 col-form-label text-sm-end\">网站域名</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"sitedomain\" name=\"sitedomain\" value=\"";
echo << obj["info"]["sitedomain"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label for=\"sitepath\" class=\"col-sm-2 col-form-label text-sm-end\">网站路径</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"sitepath\" name=\"sitepath\" placeholder=\"必须是字母和数字\" value=\"";
echo << obj["info"]["sitepath"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"expectdate\" class=\"col-sm-2 col-form-label text-sm-end\">到期日期</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"expectdate\" name=\"expectdate\" placeholder=\"yyyy-mm-dd\" value=\"";
echo << obj["info"]["expectdate"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"isopen\" class=\"col-sm-2 col-form-label text-sm-end\">启用状态</label>\n            <div class=\"col-sm-10\">\n                <div class=\"form-check\">\n                  <input class=\"form-check-input\" type=\"checkbox\" value=\"1\" name=\"isopen\" id=\"isopen\" ";
 if(obj["info"]["isopen"].to_int()==1){ 
 			 echo<<"checked=\"checked\"";
 } 
 			 echo<<">\n                </div>                \n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-10 offset-sm-2\">\n              <button type=\"submit\" class=\"btn btn-primary\">添加</button>\n              <input type=\"hidden\" id=\"sid\" name=\"sid\" value=\"";
echo<<obj["info"]["sid"].to_string(); 
 			 echo<<"\" />\n            </div>\n          </div>\n\n        </form>\n      </div>\n    </div>\n\n  </div>\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n \n</body>\n\n</html>";

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
    