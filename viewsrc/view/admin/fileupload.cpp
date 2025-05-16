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
	namespace admin{
 		 std::string fileupload([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>上传文件</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n</header>\n<body>\n<div class=\"card\">\n  <div class=\"card-body\">\n    <form action=\"/attachfile/uploadpost\" method=\"post\" enctype=\"multipart/form-data\">\n    <div class=\"mb-3\">\n     \n        <div class=\"mb-3\">\n        <label for=\"upfile\" class=\"form-label\">选择上传文件(Choose File):</label>\n        <input type=\"file\" name=\"upfile\" id=\"upfile\"> (jpg,png,js,html,pdf,zip,tar.gz,apk)\n      </div>\n    \n      <button type=\"submit\"  class=\"btn btn-primary\">提交(Upload)</button>\n    </form>\n  </div>\n</div>\n\n\n</body>\n</html>";

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
    