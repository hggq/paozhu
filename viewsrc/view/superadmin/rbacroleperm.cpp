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
 		 std::string rbacroleperm([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>角色权限</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">角色权限：";
echo<<obj["role"]["name"].to_string(); 
 			 echo<<"</h5>\n    <div class=\"text-muted mb-2\">\n      角色ID <code>roleid</code>：";
echo<<obj["role"]["roleid"].to_string(); 
 			 echo<<"；\n      归属用户ID <code>userid</code>：";
echo<<obj["role"]["userid"].to_string(); 
 			 echo<<"      ";
 if(obj["role"]["userid"].to_int()==0){ 
 			 echo<<"<span class=\"badge bg-warning text-dark\">归属无效（userid=0，待清理）</span>";
 } 
 			 echo<<"    </div>\n    <div class=\"alert alert-info py-2\">\n      勾选该角色可访问的权限项，保存后整体替换原有授权（写入 <code>sysroleperms</code>：\n      userid=角色的 userid、roleid=当前角色、permsid=勾选的权限）。<br>\n      列表仅显示启用中、且归属本角色所在公司的权限项；<code>maskcode</code> 是路由注册时的掩码，\n      角色授权后，持有该角色的用户才能访问对应 urlpath。\n    </div>\n\n    <div class=\"d-flex align-items-center gap-3 mb-2\">\n      <div class=\"form-check\">\n        <input class=\"form-check-input\" type=\"checkbox\" id=\"checkall\">\n        <label class=\"form-check-label\" for=\"checkall\">全选当前列表</label>\n      </div>\n      <button class=\"btn btn-primary btn-sm\" id=\"savebtn\">保存权限</button>\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:60px\">选择<br><small class=\"text-muted fw-normal\">勾选=授权</small></th>\n          <th style=\"width:90px\">权限ID<br><small class=\"text-muted fw-normal\">permsid</small></th>\n          <th>权限路径<br><small class=\"text-muted fw-normal\">urlpath</small></th>\n          <th>权限名称<br><small class=\"text-muted fw-normal\">name</small></th>\n          <th style=\"width:100px\">归属用户ID<br><small class=\"text-muted fw-normal\">userid</small></th>\n          <th style=\"width:100px\">权限码<br><small class=\"text-muted fw-normal\">permscode</small></th>\n          <th style=\"width:100px\">掩码<br><small class=\"text-muted fw-normal\">maskcode</small></th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr>\n          <td>\n            <input class=\"form-check-input rowcheck\" type=\"checkbox\" value=\"";
 echo<<a["permsid"].to_string(); 
 			 echo<<"\"\n              ";
 if(a["checked"].to_int()==1){ 
 			 echo<<"checked";
 } 
 			 echo<<">\n          </td>\n          <td>";
 echo<<a["permsid"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["urlpath"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["name"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 echo<<a["userid"].to_string(); 
 			 echo<<"            ";
 if(a["userid"].to_int()==0){ 
 			 echo<<"<span class=\"badge bg-info text-dark\">全局</span>";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["permscode"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["maskcode"].to_string(); 
 			 echo<<"</td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n    <a href=\"/superadmin/rbac/role\" class=\"btn btn-link\">返回角色列表</a>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    document.getElementById('checkall').addEventListener('change', function () {\n      document.querySelectorAll('.rowcheck').forEach(function (cb) { cb.checked = this.checked; }, this);\n    });\n\n    document.getElementById('savebtn').addEventListener('click', function () {\n      var box = document.getElementById('result');\n      var picked = [];\n      document.querySelectorAll('.rowcheck:checked').forEach(function (cb) { picked.push(cb.value); });\n      var fd = new FormData();\n      fd.append('csrf_token', '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"');\n      fd.append('roleid', '";
echo<<obj["role"]["roleid"].to_string(); 
 			 echo<<"');\n      fd.append('permsids', picked.join(','));\n      fetch('/superadmin/rbac/roleperm/save', { method: 'POST', body: fd, credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            box.innerHTML = '<div class=\"alert alert-success py-2 mb-0\">' +\n              (j.msg || '角色授权已保存') + '：当前共 ' + j.count + ' 项权限（已整体替换）</div>';\n          } else {\n            box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">保存失败：' +\n              (j.msg || ('HTTP ' + r.status)) + '</div>';\n          }\n        })\n        .catch(function (e) {\n          box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">请求失败：' + e + '</div>';\n        });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    