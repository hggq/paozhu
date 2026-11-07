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
//This file create by paozhu
namespace http {

namespace view {
	namespace superadmin{
 		 std::string rbacuserrole([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>用户角色</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">用户角色：";
echo<<obj["user"]["name"].to_string(); 
 			 echo<<"</h5>\n    <div class=\"text-muted mb-2\">\n      用户ID <code>adminid</code>：";
echo<<obj["user"]["adminid"].to_string(); 
 			 echo<<"；\n      公司ID <code>companyid</code>：";
echo<<obj["user"]["companyid"].to_string(); 
 			 echo<<"    </div>\n    <div class=\"alert alert-info py-2\">\n      角色绑定写入 <code>sysuserrole</code>（user_id=用户 adminid、role_id=角色 roleid、company_id=用户 companyid）。<br>\n      可选角色 = 该用户所属公司的角色（<code>userid=companyid</code>），不含其他公司或归属为 0 的角色；\n      保存时只处理发生变化的绑定：新增未勾选项、解除已取消勾选项。只有超级管理员可以修改本表。\n    </div>\n\n    <div class=\"row mb-2 g-2 align-items-end\">\n      <div class=\"col-md-4\">\n        <label class=\"form-label mb-1\" for=\"filter\">筛选角色</label>\n        <input id=\"filter\" class=\"form-control form-control-sm\" placeholder=\"输入角色名称或角色ID，例如：订单 或 3\">\n        <div class=\"form-text\">仅在前端过滤当前表格，不会修改数据。</div>\n      </div>\n      <div class=\"col-md-8 text-end\">\n        <button class=\"btn btn-primary btn-sm\" id=\"savebtn\">保存角色</button>\n      </div>\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\" id=\"roleTable\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:70px\">\n            <input type=\"checkbox\" id=\"checkall\" class=\"form-check-input me-1\" title=\"全选当前列表\">选择\n            <br><small class=\"text-muted fw-normal\">勾选=分配</small>\n          </th>\n          <th style=\"width:90px\">角色ID<br><small class=\"text-muted fw-normal\">roleid</small></th>\n          <th>角色名称<br><small class=\"text-muted fw-normal\">name</small></th>\n          <th style=\"width:110px\">归属用户ID<br><small class=\"text-muted fw-normal\">userid</small></th>\n          <th style=\"width:90px\">状态<br><small class=\"text-muted fw-normal\">status</small></th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr data-text=\"";
 echo<<a["name"].to_string(); 
 			 echo<<" ";
 echo<<a["roleid"].to_string(); 
 			 echo<<"\">\n          <td>\n            <input class=\"form-check-input rowcheck\" type=\"checkbox\" value=\"";
 echo<<a["roleid"].to_string(); 
 			 echo<<"\"\n              ";
 if(a["checked"].to_int()==1){ 
 			 echo<<"checked";
 } 
 			 echo<<">\n          </td>\n          <td>";
 echo<<a["roleid"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["name"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 echo<<a["userid"].to_string(); 
 			 echo<<"            ";
 if(a["userid"].to_int()==0){ 
 			 echo<<"<span class=\"badge bg-info text-dark\">全局</span>";
 } 
 			 echo<<"          </td>\n          <td>\n            ";
 if(a["status"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-success\">启用</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-secondary\">关闭</span>\n            ";
 } 
 			 echo<<"          </td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n    <a href=\"/superadmin/rbac/user\" class=\"btn btn-link\">返回用户列表</a>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    document.getElementById('filter').addEventListener('input', function () {\n      var q = this.value.trim().toLowerCase();\n      document.querySelectorAll('#roleTable tbody tr').forEach(function (tr) {\n        tr.style.display = q === '' || tr.dataset.text.toLowerCase().indexOf(q) >= 0 ? '' : 'none';\n      });\n    });\n\n    document.getElementById('checkall').addEventListener('change', function () {\n      document.querySelectorAll('.rowcheck').forEach(function (cb) { cb.checked = this.checked; }, this);\n    });\n\n    document.getElementById('savebtn').addEventListener('click', function () {\n      var box = document.getElementById('result');\n      var picked = [];\n      document.querySelectorAll('.rowcheck:checked').forEach(function (cb) { picked.push(cb.value); });\n      var fd = new FormData();\n      fd.append('csrf_token', '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"');\n      fd.append('userid', '";
echo<<obj["user"]["adminid"].to_string(); 
 			 echo<<"');\n      fd.append('roleids', picked.join(','));\n      fetch('/superadmin/rbac/userrole/save', { method: 'POST', body: fd, credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            var html = '<div class=\"alert alert-success py-2 mb-0\">' +\n              (j.msg || '角色绑定已保存') + '：勾选 ' + j.total + ' 项，新增 ' + j.added +\n              '，解除 ' + j.removed + '，失败 ' + j.failed;\n            if (j.error) {\n              html += '<br>失败明细：' + j.error;\n            }\n            html += '</div>';\n            box.innerHTML = html;\n            setTimeout(function () { location.reload(); }, 1000);\n          } else {\n            box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">保存失败：' +\n              (j.msg || ('HTTP ' + r.status)) + '</div>';\n          }\n        })\n        .catch(function (e) {\n          box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">请求失败：' + e + '</div>';\n        });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    