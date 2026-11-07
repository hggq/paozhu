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
 		 std::string rbacuserrole([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>用户角色</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">用户角色</h5>\n    <div class=\"text-muted mb-2\">\n      本企业（companyid）：";
echo<<obj["companyid"].to_string(); 
 			 echo<<"；\n      账户数：";
echo<<obj["user_total"].to_string(); 
 			 echo<<"；\n      当前账户已绑角色：";
echo<<obj["bound_count"].to_string(); 
 			 echo<<"    </div>\n\n    ";
 if(obj["user_ok"].to_int()==0){ 
 			 echo<<"    <div class=\"alert alert-warning py-2\">没有可分配的账户。</div>\n    ";
 }else if(obj["roles"].as_array().size()==0){ 
 			 echo<<"    <div class=\"alert alert-warning py-2\">\n      本企业还没有角色，请先到「角色管理」新建并配好权限。\n      <a class=\"btn btn-link btn-sm\" href=\"/admin/rbac/role\">角色管理</a>\n    </div>\n    ";
 }else{ 
 			 echo<<"    <div class=\"alert alert-info py-2\">\n      只能给本企业的账户绑本企业的角色；保存是整组替换，以本次勾选为准。<br>\n      停用（status=0）的角色绑上也不产生权限，运行期判定只认启用角色。<br>\n      平台侧的「单角色」（<code>sysuser.roleid</code>）不在本页管理，仍然生效，见下方提示。\n    </div>\n    ";
 } 
 			 echo<<"\n    <div class=\"row mb-2 g-2 align-items-end\">\n      <div class=\"col-md-5\">\n        <label class=\"form-label mb-1\" for=\"usersel\">选择账户</label>\n        <select id=\"usersel\" class=\"form-select\">\n          ";
 for(auto &u:obj["users"].as_array()){ 
 			 echo<<"          <option value=\"";
 echo<<u["adminid"].to_string(); 
 			 echo<<"\"\n            ";
 if(u["adminid"].to_int()==obj["userid"].to_int()){ 
 			 echo<<"selected";
 } 
 			 echo<<">\n            ";
 echo<<u["name"].to_string(); 
 			 echo<<"（adminid ";
 echo<<u["adminid"].to_string(); 
 			 echo<<"";
 if(u["isadmin"].to_int()==1){ 
 			 echo<<" · 管理员";
 } 
 			 echo<<"";
 if(u["isopen"].to_int()!=1){ 
 			 echo<<" · 已禁用";
 } 
 			 echo<<"）\n          </option>\n          ";
 } 
 			 echo<<"        </select>\n        <div class=\"form-text\">切换账户会重新载入本页并显示该账户已绑的角色。账户本身请到「账户管理」维护。</div>\n      </div>\n      <div class=\"col-md-7 text-end\">\n        ";
 if(obj["user_ok"].to_int()==1){ 
 			 echo<<"        <button class=\"btn btn-primary btn-sm\" id=\"savebtn\">保存绑定</button>\n        ";
 } 
 			 echo<<"      </div>\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    ";
 if(obj["legacy_roleid"].to_int()!=0){ 
 			 echo<<"    <div class=\"alert alert-warning py-2\">\n      该账户还有平台侧单角色 roleid=";
echo<<obj["legacy_roleid"].to_string(); 
 			 echo<<"，它的权限会和本页勾选的角色叠加生效；\n      要取消请到平台侧「用户管理」修改，本页不改 <code>sysuser.roleid</code>。\n    </div>\n    ";
 } 
 			 echo<<"\n    <table class=\"table table-sm table-bordered align-middle\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:70px\">\n            <input type=\"checkbox\" id=\"headcheck\" class=\"form-check-input me-1\" title=\"勾选表头复选框切换本页全部\">选择\n          </th>\n          <th style=\"width:90px\">roleid</th>\n          <th>角色名称</th>\n          <th style=\"width:100px\">状态</th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &r:obj["roles"].as_array()){ 
 			 echo<<"        <tr>\n          <td>\n            <input class=\"form-check-input rowcheck\" type=\"checkbox\"\n              value=\"";
 echo<<r["roleid"].to_string(); 
 			 echo<<"\"\n              ";
 if(r["checked"].to_int()==1){ 
 			 echo<<"checked";
 } 
 			 echo<<">\n          </td>\n          <td>";
 echo<<r["roleid"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<r["name"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 if(r["status"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-success\">启用</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-secondary\">停用</span>\n            ";
 } 
 			 echo<<"          </td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n    <div class=\"text-muted small mb-2\">\n      改动后该账户下一次请求即按新权限判定（只失效它自己的缓存快照）。\n    </div>\n\n    <a href=\"/admin/rbac/role\" class=\"btn btn-link\">角色管理</a>\n    <a href=\"/admin/rbac/roleperm\" class=\"btn btn-link\">角色权限</a>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    var CSRF = '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"';\n    var USERID = '";
echo<<obj["userid"].to_string(); 
 			 echo<<"';\n    function show(msg, ok) {\n      document.getElementById('result').innerHTML =\n        '<div class=\"alert alert-' + (ok ? 'success' : 'danger') + ' py-2 mb-0\">' + msg + '</div>';\n    }\n    document.getElementById('usersel').addEventListener('change', function () {\n      location.href = '/admin/rbac/userrole?userid=' + encodeURIComponent(this.value);\n    });\n    document.getElementById('headcheck') && document.getElementById('headcheck').addEventListener('change', function () {\n      document.querySelectorAll('.rowcheck').forEach(function (cb) { cb.checked = this.checked; }, this);\n    });\n    document.getElementById('savebtn') && document.getElementById('savebtn').addEventListener('click', function () {\n      var picked = [];\n      document.querySelectorAll('.rowcheck:checked').forEach(function (cb) { picked.push(cb.value); });\n      if (picked.length === 0 && !confirm('当前一个都没勾：保存会解绑该账户的全部角色，确定？')) { return; }\n      var fd = new FormData();\n      fd.append('csrf_token', CSRF);\n      fd.append('userid', USERID);\n      fd.append('roleids', picked.join(','));\n      fetch('/admin/rbac/userrole/save', { method: 'POST', body: fd, credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            show('已保存：该账户现在绑定 ' + picked.length + ' 个角色。', true);\n            setTimeout(function () { location.reload(); }, 700);\n          } else {\n            show('保存失败：' + (j.msg || '未知错误'), false);\n          }\n        })\n        .catch(function (e) { show('请求失败：' + e, false); });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    