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
//This file create by paozhu Thu, 06 Aug 2026 04:58:15 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string jstimetest([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<div class=\"page-card\">\n    <div class=\"page-title\">📊 仪表盘</div>\n    <p><a href=\"/admin/login\">登录</a></p>\n    <p>当前时间：<span id=\"timeDisplay\">加载中...</span></p>\n</div>\n\n<script>\n    (function() {\n        const el = document.getElementById('timeDisplay');\n        let timer = null;\n\n        function update() {\n            el.textContent = new Date().toLocaleString();\n        }\n\n        update();\n        timer = setInterval(update, 1000);\n\n        // 将清理函数挂载到当前容器（页面脚本执行时，容器为 document.currentScript.parentNode 或通过 parent 获取）\n        const container = document.currentScript.parentNode;\n        container.__cleanup = function() {\n            clearInterval(timer);\n            console.log('定时器已清理');\n        };\n    })();\n</script>";

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
    