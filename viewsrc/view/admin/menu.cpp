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
//This file create by paozhu Thu, 06 Aug 2026 08:55:28 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string menu([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!DOCTYPE html>\n<html lang=\"zh-CN\">\n<head>\n    <meta charset=\"UTF-8\" />\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n    <title>系统后台</title>\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n    <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n    <script src=\"/js/vue.global.js\"></script>\n    <link href=\"/css/menu.css\" rel=\"stylesheet\">\n    <link rel=\"stylesheet\" href=\"/css/element.css\" />\n    <script src=\"/js/element.min.js\"></script>\n    <script src=\"/js/jquery.min.js\"></script>\n</head>\n<body>\n\n    <header class=\"app-header\">\n        <button class=\"sidebar-toggle\" id=\"btnSidebarToggle\" type=\"button\" aria-label=\"Toggle navigation\" aria-controls=\"sidebar\">\n            <i class=\"bi bi-list\"></i>\n        </button>\n        <div class=\"logo\"><i>📦</i> Admin</div>\n        <span class=\"header-title\">· 自定义菜单</span>\n        <div class=\"header-right\">\n            <span>管理员</span>\n            <button class=\"btn-logout\" id=\"btnLogout\">退出</button>\n        </div>\n    </header>\n\n    <div class=\"app-body\">\n        <aside class=\"app-sidebar\" id=\"sidebar\">\n            <ul class=\"sidebar-menu\" id=\"menuContainer\"></ul>\n        </aside>\n        <main class=\"app-main\">\n            <div class=\"tab-bar\" id=\"tabBar\"></div>\n            <div class=\"tab-content-wrapper\" id=\"tabContentWrapper\"></div>\n        </main>\n    </div>\n\n    <div class=\"toast\" id=\"toast\"></div>\n    <div class=\"sidebar-backdrop\" id=\"sidebarBackdrop\"></div>\n\n    <script>\n        // ================================================================\n        //  菜单配置：由后台动态输出（C++ 视图可在此注入真实菜单数据）\n        // ================================================================\n        const MENU_CONFIG = [{\n            group: '公共',\n            icon: 'bi-grid',\n            items: [\n                { text: '仪表盘', href: '/admin/dashboard', icon: '📊', html: true},\n                { text: 'Vue 演示', href: '/admin/vuetest', icon: '📦', html: true},\n                { text: 'Element 演示', href: '/admin/testelementplus', icon: '🐷', html: true},\n            ]\n        }];\n</script>\n    <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n    <script src=\"/js/store.js\"></script>\n    <script src=\"/js/menu.js\"></script>\n    \n</body>\n</html>";

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
    