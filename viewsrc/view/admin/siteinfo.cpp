#include<iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <map> 
#include <vector>
#include <ctime>
#include <array>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include "request.h"
#include "datetime.h"
#include "cookie.h"
#include "urlcode.h"
#include "loadviewso.h"
#include "viewso_param.h"
#include "http_so_common_api.h"
#include "viewsrc.h"
//g++ viewsrc/view/admin/siteinfo.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string siteinfo([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n \n  <title>网站信息</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n\n</head>\n\n<body>\n\n  <div class=\"container-fluid\">\n\n    <h5 class=\"card-title mt-2\">网站设置</h5>\n    <hr>\n    <div class=\"row\">\n      <div class=\"col-12\">\n        <form id=\"form\" action=\"/admin/siteinfopost\" method=\"post\">\n          <div class=\"row mb-3\">\n            <label for=\"sitename\" class=\"col-sm-2 col-form-label text-sm-end\">网站名称</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"sitename\" name=\"sitename\" value=\"";
echo << obj["info"]["sitename"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"sitedomain\" class=\"col-sm-2 col-form-label text-sm-end\">网站域名</label>\n            <div class=\"col-sm-10\">\n              <input type=\"text\" class=\"form-control\" id=\"sitedomain\" name=\"sitedomain\" value=\"";
echo << obj["info"]["sitedomain"].to_string(); 
 			 echo<<"\">\n            </div>\n          </div>\n\n\n          <div class=\"row mb-3\">\n            <label for=\"metakeys\" class=\"col-sm-2 col-form-label text-sm-end\">META关键词</label>\n            <div class=\"col-sm-10\">\n              <textarea class=\"form-control\" name=\"metakeys\" id=\"metakeys\" rows=\"3\" name=\"keywords\"\n                placeholder=\"多个关键词用英文逗号分隔\">";
echo << obj["info"]["metakeys"].to_string(); 
 			 echo<<"</textarea>\n            </div>\n          </div>\n          <div class=\"row mb-3\">\n            <label for=\"metadesc\" class=\"col-sm-2 col-form-label text-sm-end\">META描述</label>\n            <div class=\"col-sm-10\">\n              <textarea class=\"form-control\" name=\"metadesc\" id=\"metadesc\" rows=\"3\"\n                name=\"descript\">";
echo << obj["info"]["metadesc"].to_string(); 
 			 echo<<"</textarea>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label for=\"copyright\" class=\"col-sm-2 col-form-label text-sm-end\">版权信息</label>\n            <div class=\"col-sm-10\">\n              <textarea class=\"form-control\" name=\"copyright\" id=\"copyright\" rows=\"3\"\n                name=\"copyright\">";
echo << obj["info"]["copyright"].to_string(); 
 			 echo<<"</textarea>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <div class=\"col-sm-10 offset-sm-2\">\n              <button type=\"submit\" class=\"btn btn-primary\">确认</button>\n              <input type=\"hidden\" id=\"sid\" name=\"sid\" value=\"";
echo<<obj["info"]["sid"].to_string(); 
 			 echo<<"\" />\n            </div>\n          </div>\n\n        </form>\n      </div>\n    </div>\n\n  </div>\n  \n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    