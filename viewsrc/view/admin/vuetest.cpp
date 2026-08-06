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
//This file create by paozhu Thu, 06 Aug 2026 03:34:08 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string vuetest([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<div class=\"page-card\">\n    <div class=\"page-title\">📦 Vue 演示 <span style=\"font-size:14px;font-weight:400;color:#8a9aaa;\">自定义添加菜单</span></div>\n    <div class=\"page-desc\">您可以在下方添加任意菜单项，指向后台预置的接口。</div>\n    <div id=\"vue-app\" class=\"vue-demo-container\">\n        <h3>自定义菜单添加器</h3>\n        <div class=\"add-menu-form\">\n            <div>\n                <label>菜单标题：</label>\n                <input v-model=\"menuTitle\" placeholder=\"如：数据分析\" />\n            </div>\n            <div style=\"margin-top:8px;\">\n                <label>链接地址：</label>\n                <input v-model=\"menuHref\" placeholder=\"如：/data-analysis\" style=\"width:300px;\" />\n            </div>\n            <div style=\"margin-top:12px;\">\n                <button @click=\"addCustomMenu\">➕ 添加到菜单</button>\n                <span style=\"margin-left:16px;color:#5a6a82;\">（页面会以 HTML 模式加载）</span>\n            </div>\n        </div>\n        <hr style=\"margin:16px 0;\" />\n        <div>\n            <strong>当前菜单列表：</strong>\n            <ul class=\"menu-list\">\n                <li v-for=\"item in menuItems\" :key=\"item.href\">\n                    {{ item.text }} ({{ item.href }})\n                </li>\n            </ul>\n        </div>\n        <p><small>提示：添加后左侧菜单会立即更新，点击新菜单项即可打开对应页面。</small></p>\n    </div>\n    <script data-vue>\n        {\n            setup() {\n                const menuTitle = Vue.ref('');\n                const menuHref = Vue.ref('');\n                const menuItems = Vue.computed(() => {\n                    const items = [];\n                    MENU_CONFIG.forEach(group => {\n                        group.items.forEach(item => {\n                            items.push({ text: item.text, href: item.href });\n                        });\n                    });\n                    return items;\n                });\n\n                const addCustomMenu = () => {\n                    const title = menuTitle.value.trim();\n                    const href = menuHref.value.trim();\n                    if (!title || !href) {\n                        alert('请填写完整的标题和链接地址');\n                        return;\n                    }\n                    const newItem = {\n                        text: title,\n                        href: href,\n                        icon: '📌',\n                        html: true\n                    };\n                    if (MENU_CONFIG.length > 0) {\n                        MENU_CONFIG[0].items.push(newItem);\n                    } else {\n                        MENU_CONFIG.push({ group: '自定义', items: [newItem] });\n                    }\n                    if (window.renderMenu) window.renderMenu();\n                    menuTitle.value = '';\n                    menuHref.value = '';\n                    alert('✅ 菜单已添加：' + title);\n                };\n\n                return { menuTitle, menuHref, menuItems, addCustomMenu };\n            }\n        }\n    </script>\n    <div style=\"margin-top:16px;font-size:14px;color:#5a6a82;\">\n        <p>📌 您可以通过此页面动态添加任意菜单项，无需提前加载配置。</p>\n        <p>🗑️ 关闭 Tab 时，Vue 应用会自动卸载。</p>\n    </div>\n</div>";

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
    