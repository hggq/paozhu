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
//g++ viewsrc/view/admin/main.cpp
namespace http {

namespace view {
	namespace admin{
 		 std::string main([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::OBJ_VALUE &obj)
			{
 
                     std::ostringstream echo;

        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>Paozhu admin</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n  <style>\n\n  </style>\n</head>\n\n<body>\n\n  <header class=\"navbar sticky-top flex-md-nowrap p-0 navbar-dark bg-dark\">\n    <a class=\"navbar-brand col-md-3 col-lg-2 me-0 px-3 fs-6 d-flex justify-content-center\" href=\" #\">后台管理</a>\n    <button class=\"navbar-toggler position-absolute d-md-none collapsed\" type=\"button\" data-bs-toggle=\"collapse\"\n      data-bs-target=\"#sidebarMenu\" aria-controls=\"sidebarMenu\" aria-expanded=\"false\" aria-label=\"Toggle navigation\">\n      <span class=\"navbar-toggler-icon\"></span>\n    </button>\n\n    <div class=\"dropdown\" id=\"menbarpop\">\n      <button class=\"btn dropdown-toggle dropdown-menu-dark\" id=\"bd-versions\" data-bs-toggle=\"dropdown\"\n        aria-expanded=\"false\" data-bs-display=\"static\">\n        <span style=\"color:aliceblue\">管理员 <i class=\"bi bi-box-arrow-right\"\n            style=\"font-size: 1rem; color: aliceblue;\"></i><span>\n      </button>\n      <ul class=\"dropdown-menu dropdown-menu-end\" aria-labelledby=\"bd-versions\" data-bs-popper=\"static\">\n\n        <li>\n          <a class=\"dropdown-item\" href=\"/admin/userinfo\">\n            账号信息\n          </a>\n        </li>\n        <li>\n          <a class=\"dropdown-item\" href=\"/admin/editpassword\">修改密码</a>\n        </li>\n\n        <li>\n          <hr class=\"dropdown-divider\">\n        </li>\n        <li><a class=\"dropdown-item\" id=\"barlogout\" href=\"/admin/logout\">退出</a></li>\n      </ul>\n    </div>\n  </header>\n\n  <div class=\"container-fluid\">\n    <div class=\"row\">\n      <nav id=\"sidebarMenu\" class=\"col-md-3 col-lg-2 d-md-block bg-body-tertiary sidebar collapse\">\n        <div class=\"position-sticky pt-3 sidebar-sticky\">\n          <ul class=\"bsa-menu\" data-bsa-toggle=\"sidebar\" data-accordion=\"true\" data-animation-speed=\"150\">\n            <li>\n              <a href=\"/admin/welcome\">\n                <i class=\"bi bi-house\"></i>首页\n              </a>\n            </li>\n            <li>\n              <a href=\"/admin/siteinfo\"><i class=\"bi bi-gear\"></i>网站设置</a>\n            </li>\n            <li>\n              <a href=\"javascript:\" class=\"has-children open\">\n                <i class=\"bi bi-pencil-square\"></i>栏目管理\n              </a>\n              <ul>\n                <li>\n                  <a href=\"/admin/addtopic\">添加栏目</a>\n                </li>\n                <li>\n                  <a href=\"/admin/martopic\">栏目列表</a>\n                </li>\n              </ul>\n            </li>\n\n            <li>\n              <a href=\"javascript:\" class=\"has-children open\">\n                <i class=\"bi bi-pencil-square\"></i>文章管理\n              </a>\n              <ul>\n                <li>\n                  <a href=\"/admin/addarticle\">添加文章</a>\n                </li>\n                <li>\n                  <a href=\"/admin/listarticle\">文章列表</a>\n                </li>\n              </ul>\n            </li>\n          </ul>\n        </div>\n      </nav>\n\n      <main class=\"col-md-9 ms-sm-auto col-lg-10\" style=\"padding-right:0px;padding-left:0px;\">\n        <ul class=\"nav nav-tabs\" id=\"myNavbarTab\" role=\"tablist\">\n          <li class=\"nav-item\" role=\"presentation\">\n            <button class=\"nav-link active\" id=\"adminwelcome\" data-bs-toggle=\"tab\" data-bs-target=\"#adminwelcome-pane\"\n              type=\"button\" role=\"tab\" aria-controls=\"adminwelcome-pane\" aria-selected=\"true\">首页 </button>\n          </li>\n\n        </ul>\n        <div class=\"tab-content\" id=\"myNavbarContent\">\n          <div class=\"tab-pane fade show active\" id=\"adminwelcome-pane\" role=\"tabpanel\" aria-labelledby=\"adminwelcome\"\n            tabindex=\"0\">\n            <iframe width=\"100%\" height=\"100%\" frameborder=\"0\" src=\"/admin/welcome\"></iframe>\n          </div>\n\n        </div>\n\n      </main>\n    </div>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n\n  <script src=\"/js/jquery.min.js\"></script>\n  <script src=\"/js/navtabs.js\" type=\"text/javascript\"></script>\n  <script type=\"text/javascript\">\n\n    jQuery(function ($) {\n\n      TABSNAV.init();\n      jQuery(\".has-children\").click(function (e) {\n        if (jQuery(this).hasClass(\"open\")) {\n          jQuery(this).removeClass(\"open\");\n        } else {\n          jQuery(this).addClass(\"open\");\n        }\n        return false;\n      });\n      $('#myNavbarTab button[data-bs-toggle=\"tab\"]').on('shown.bs.tab', function (e) {\n\n        TABSNAV.refreshvport();\n      });\n    });\n\n\n\n  </script>\n</body>\n\n</html>";

                  return echo.str();
             }

         }
        }
       }
    