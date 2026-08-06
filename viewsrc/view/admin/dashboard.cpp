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
//This file create by paozhu Thu, 06 Aug 2026 06:17:02 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string dashboard([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<div class=\"page-card\" id=\"vue-app\">\n    <div class=\"page-title\">📊 仪表盘</div>\n    <p><a href=\"/admin/jstimetest\">登录</a></p>\n    <p>当前时间：2025-01-15 14:30:25</p>\n    <div>\n        <p>Vue 组件也能用：{{ time }}</p>\n        <p>当前用户：{{ username }}</p>\n    </div>\n</div>\n<script data-vue>\n{\n    setup() {\n        const timeRef = Vue.ref(new Date().toLocaleString());\n        const timer = setInterval(() => {\n            timeRef.value = new Date().toLocaleString();\n        }, 1000);\n\n        const userStore = window.useUserStore();\n        const username = Vue.computed(() => userStore.state.name);\n\n        // 组件卸载时清理定时器\n        Vue.onUnmounted(() => clearInterval(timer));\n        return { time: timeRef,username };\n    }\n}\n</script>";

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
    