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
//This file create by paozhu Thu, 06 Aug 2026 08:39:04 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string elementplus([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<div class=\"page-card\">\n    <div class=\"page-title\">📊 仪表盘 <span style=\"font-size:14px;font-weight:400;color:#8a9aaa;\">Element Plus 示例</span></div>\n    <div id=\"vue-app\">\n        <!-- 按钮 -->\n        <el-button type=\"primary\" @click=\"handleClick\">主要按钮</el-button>\n        <el-button @click=\"handleClick\">默认按钮</el-button>\n        <el-button type=\"success\" @click=\"handleClick\">成功按钮</el-button>\n\n        <!-- 输入框 -->\n        <div style=\"margin: 16px 0;\">\n            <el-input v-model=\"inputText\" placeholder=\"请输入内容\" style=\"width: 300px;\"></el-input>\n            <div style=\"margin-left: 12px; display: inline-block;\">输入内容：{{ inputText }}</div>\n        </div>\n\n        <!-- 表格 -->\n        <el-table :data=\"tableData\" style=\"width: 100%; margin-top: 16px;\" border>\n            <el-table-column prop=\"date\" label=\"日期\" min-width=\"180\"></el-table-column>\n            <el-table-column prop=\"name\" label=\"姓名\" min-width=\"180\"></el-table-column>\n            <el-table-column prop=\"address\" label=\"地址\" min-width=\"180\"></el-table-column>\n        </el-table>\n\n        <!-- 弹窗演示 -->\n        <el-button type=\"danger\" @click=\"dialogVisible = true\">打开弹窗</el-button>\n        <el-dialog v-model=\"dialogVisible\" title=\"提示\" width=\"30%\">\n            <span>这是一个 Element Plus 弹窗</span>\n            <template #footer>\n                <span class=\"dialog-footer\">\n                    <el-button @click=\"dialogVisible = false\">取消</el-button>\n                    <el-button type=\"primary\" @click=\"dialogVisible = false\">确认</el-button>\n                </span>\n            </template>\n        </el-dialog>\n    </div>\n</div>\n\n<script data-vue>\n{\n    setup() {\n        const inputText = Vue.ref('');\n        const dialogVisible = Vue.ref(false);\n        const tableData = Vue.ref([]);\n        const loading = Vue.ref(true);\n\n        const handleClick = () => {\n            alert('按钮点击！');\n        };\n\n        // 从接口获取数据\n        const fetchData = async () => {\n            loading.value = true;\n            try {\n                const res = await fetch('/admin/testgetjson');\n                if (!res.ok) throw new Error(`HTTP ${res.status}`);\n                const data = await res.json();\n                // 假设返回的 JSON 格式是 [{ date, name, address }, ...]\n                // 如果字段名不同，请在此映射\n                tableData.value = data;\n            } catch (e) {\n                console.error('加载表格数据失败:', e);\n                tableData.value = [];\n            } finally {\n                loading.value = false;\n            }\n        };\n\n        // 组件挂载时自动请求\n        Vue.onMounted(fetchData);\n\n        // 调试：监听输入变化\n        Vue.watch(inputText, (newVal) => {\n            console.log('输入内容:', newVal);\n        });\n\n        return {\n            inputText,\n            dialogVisible,\n            tableData,\n            loading,\n            handleClick\n        };\n    }\n}\n</script>";

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
    