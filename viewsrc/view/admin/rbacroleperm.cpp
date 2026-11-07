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
	namespace admin{
 		 std::string rbacroleperm([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>角色权限</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">角色权限</h5>\n    <div class=\"text-muted mb-2\">\n      本企业（companyid）：";
echo<<obj["companyid"].to_string(); 
 			 echo<<"；\n      可分配权限项：";
echo<<obj["assignable_total"].to_string(); 
 			 echo<<"；\n      当前角色已勾选：";
echo<<obj["granted_count"].to_string(); 
 			 echo<<"    </div>\n\n    ";
 if(obj["role_ok"].to_int()==0){ 
 			 echo<<"    <div class=\"alert alert-warning py-2\">\n      本企业还没有角色，请先到「角色管理」新建角色。\n      <a class=\"btn btn-link btn-sm\" href=\"/admin/rbac/role\">角色管理</a>\n    </div>\n    ";
 }else if(obj["assignable_total"].to_int()==0){ 
 			 echo<<"    <div class=\"alert alert-warning py-2\">\n      平台还没有把任何权限登记给本企业，这里没有任何可勾选项。请联系平台管理员。\n    </div>\n    ";
 }else{ 
 			 echo<<"    <div class=\"alert alert-info py-2\">\n      列表来自平台分配给本企业的权限项（<code>syscompanyperms</code>），且对应的目录行\n      （<code>sysperms</code>，<code>userid</code> 等于本企业）必须启用中——\n      只能在里面拆开重组，勾不到平台没分配给本公司的权限。<br>\n      保存是整组替换：以本次勾选为准，未勾的会取消。改完即对该角色绑定的所有账户生效。\n    </div>\n    ";
 } 
 			 echo<<"\n    <div class=\"row mb-2 g-2 align-items-end\">\n      <div class=\"col-md-5\">\n        <label class=\"form-label mb-1\" for=\"rolesel\">选择角色</label>\n        <select id=\"rolesel\" class=\"form-select\">\n          ";
 for(auto &r:obj["roles"].as_array()){ 
 			 echo<<"          <option value=\"";
 echo<<r["roleid"].to_string(); 
 			 echo<<"\"\n            ";
 if(r["roleid"].to_int()==obj["roleid"].to_int()){ 
 			 echo<<"selected";
 } 
 			 echo<<">\n            ";
 echo<<r["name"].to_string(); 
 			 echo<<"（roleid ";
 echo<<r["roleid"].to_string(); 
 			 echo<<"";
 if(r["status"].to_int()!=1){ 
 			 echo<<" · 已停用";
 } 
 			 echo<<"）\n          </option>\n          ";
 } 
 			 echo<<"        </select>\n        <div class=\"form-text\">切换角色会重新载入本页并显示该角色已勾选的权限。</div>\n      </div>\n      <div class=\"col-md-7 text-end\">\n        ";
 if(obj["role_ok"].to_int()==1){ 
 			 echo<<"        <button class=\"btn btn-outline-secondary btn-sm\" id=\"checkall\">全选</button>\n        <button class=\"btn btn-outline-secondary btn-sm\" id=\"uncheckall\">全不选</button>\n        <button class=\"btn btn-primary btn-sm\" id=\"savebtn\">保存勾选</button>\n        ";
 } 
 			 echo<<"      </div>\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:70px\">\n            <input type=\"checkbox\" id=\"headcheck\" class=\"form-check-input me-1\" title=\"勾选表头复选框切换本页全部\">选择\n          </th>\n          <th style=\"width:90px\">permsid</th>\n          <th style=\"width:80px\">上级</th>\n          <th>路由路径<br><small class=\"text-muted fw-normal\">urlpath</small></th>\n          <th>名称</th>\n          <th style=\"width:100px\">掩码<br><small class=\"text-muted fw-normal\">maskcode</small></th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr>\n          <td>\n            <input class=\"form-check-input rowcheck\" type=\"checkbox\"\n              value=\"";
 echo<<a["permsid"].to_string(); 
 			 echo<<"\"\n              ";
 if(a["checked"].to_int()==1){ 
 			 echo<<"checked";
 } 
 			 echo<<">\n          </td>\n          <td>";
 echo<<a["permsid"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["parentid"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["urlpath"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["name"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["maskcode"].to_string(); 
 			 echo<<"</td>\n        </tr>\n        ";
 } 
 			 echo<<"        ";
 if(obj["assignable_total"].to_int()==0){ 
 			 echo<<"        <tr>\n          <td colspan=\"6\" class=\"text-center text-muted\">没有可分配的权限项。</td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n    <div class=\"text-muted small mb-2\">\n      掩码为 <code>0</code> 的行不参与 RBAC 判定（路由不查权限）；掩码非 0 的行要求授权掩码与路由掩码精确相等，\n      否则该路由会一直 403。\n    </div>\n\n    <a href=\"/admin/rbac/role\" class=\"btn btn-link\">角色管理</a>\n    <a href=\"/admin/rbac/userrole\" class=\"btn btn-link\">用户角色</a>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    var CSRF = '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"';\n    var ROLEID = '";
echo<<obj["roleid"].to_string(); 
 			 echo<<"';\n    function show(msg, ok) {\n      document.getElementById('result').innerHTML =\n        '<div class=\"alert alert-' + (ok ? 'success' : 'danger') + ' py-2 mb-0\">' + msg + '</div>';\n    }\n    document.getElementById('rolesel').addEventListener('change', function () {\n      location.href = '/admin/rbac/roleperm?roleid=' + encodeURIComponent(this.value);\n    });\n    function setAll(checked) {\n      document.querySelectorAll('.rowcheck').forEach(function (cb) { cb.checked = checked; });\n    }\n    document.getElementById('checkall') && document.getElementById('checkall').addEventListener('click', function () { setAll(true); });\n    document.getElementById('uncheckall') && document.getElementById('uncheckall').addEventListener('click', function () { setAll(false); });\n    document.getElementById('headcheck') && document.getElementById('headcheck').addEventListener('change', function () { setAll(this.checked); });\n    document.getElementById('savebtn') && document.getElementById('savebtn').addEventListener('click', function () {\n      var picked = [];\n      document.querySelectorAll('.rowcheck:checked').forEach(function (cb) { picked.push(cb.value); });\n      if (picked.length === 0 && !confirm('当前一个都没勾：保存会清空该角色的全部权限，确定？')) { return; }\n      var fd = new FormData();\n      fd.append('csrf_token', CSRF);\n      fd.append('roleid', ROLEID);\n      fd.append('permsids', picked.join(','));\n      fetch('/admin/rbac/roleperm/bind', { method: 'POST', body: fd, credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            show('已保存：该角色现在有 ' + picked.length + ' 项权限。', true);\n            setTimeout(function () { location.reload(); }, 700);\n          } else {\n            show('保存失败：' + (j.msg || '未知错误'), false);\n          }\n        })\n        .catch(function (e) { show('请求失败：' + e, false); });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    