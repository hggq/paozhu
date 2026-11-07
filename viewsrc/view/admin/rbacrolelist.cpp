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
 		 std::string rbacrolelist([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>角色管理</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">角色管理</h5>\n    <div class=\"text-muted mb-2\">\n      本企业（companyid）：";
echo<<obj["companyid"].to_string(); 
 			 echo<<"；\n      角色数：";
echo<<obj["role_count"].to_string(); 
 			 echo<<"；\n      可分配权限项：";
echo<<obj["assignable_total"].to_string(); 
 			 echo<<"    </div>\n\n    ";
 if(obj["assignable_total"].to_int()==0){ 
 			 echo<<"    <div class=\"alert alert-warning py-2\">\n      平台还没有把任何权限登记给本企业，角色建出来也分配不了权限。请联系平台管理员在\n      「注册函数列表」页按本企业登记路由权限，并在「企业权限分配」页把权限分配给本企业。\n    </div>\n    ";
 } else{ 
 			 echo<<"    <div class=\"alert alert-info py-2\">\n      角色是「一组权限的容器」，只能从平台登记给本企业的权限项里挑。<br>\n      建好角色后到「角色权限」页勾选权限，再到「用户角色」页把角色分配给本企业账户。\n    </div>\n    ";
 } 
 			 echo<<"\n    <div class=\"row g-2 mb-3\">\n      <div class=\"col-md-3\">\n        <label class=\"form-label mb-1\" for=\"name\">角色名称</label>\n        <input id=\"name\" class=\"form-control\" maxlength=\"60\" placeholder=\"例如：内容编辑\">\n      </div>\n      <div class=\"col-md-2\">\n        <label class=\"form-label mb-1\" for=\"sortid\">排序</label>\n        <input id=\"sortid\" class=\"form-control\" type=\"number\" value=\"0\">\n      </div>\n      <div class=\"col-md-2\">\n        <label class=\"form-label mb-1\" for=\"status\">状态</label>\n        <select id=\"status\" class=\"form-select\">\n          <option value=\"1\">启用</option>\n          <option value=\"0\">停用</option>\n        </select>\n      </div>\n      <div class=\"col-md-2\">\n        <label class=\"form-label mb-1\" for=\"rolecode\">角色码</label>\n        <input id=\"rolecode\" class=\"form-control\" type=\"number\" value=\"0\">\n      </div>\n      <div class=\"col-md-3 d-flex align-items-end gap-2\">\n        <button class=\"btn btn-primary btn-sm\" id=\"addbtn\">新建角色</button>\n        <button class=\"btn btn-outline-secondary btn-sm\" id=\"resetbtn\">清空</button>\n      </div>\n    </div>\n    <input type=\"hidden\" id=\"roleid\" value=\"0\">\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:90px\">roleid</th>\n          <th>名称</th>\n          <th style=\"width:90px\">状态</th>\n          <th style=\"width:110px\">权限数</th>\n          <th style=\"width:110px\">绑定账户数</th>\n          <th style=\"width:230px\">操作</th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr>\n          <td>";
 echo<<a["roleid"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 echo<<a["name"].to_string(); 
 			 echo<<"            <div class=\"text-muted small\">排序 ";
 echo<<a["sortid"].to_string(); 
 			 echo<<" · 角色码 ";
 echo<<a["rolecode"].to_string(); 
 			 echo<<"</div>\n          </td>\n          <td>\n            ";
 if(a["status"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-success\">启用</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-secondary\">停用</span>\n            ";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["perm_count"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["user_count"].to_string(); 
 			 echo<<"</td>\n          <td>\n            <a class=\"btn btn-outline-primary btn-sm\" href=\"/admin/rbac/roleperm?roleid=";
 echo<<a["roleid"].to_string(); 
 			 echo<<"\">配权限</a>\n            <button class=\"btn btn-outline-secondary btn-sm editbtn\"\n              data-roleid=\"";
 echo<<a["roleid"].to_string(); 
 			 echo<<"\"\n              data-name=\"";
 echo<<a["name"].to_string(); 
 			 echo<<"\"\n              data-status=\"";
 echo<<a["status"].to_string(); 
 			 echo<<"\"\n              data-sortid=\"";
 echo<<a["sortid"].to_string(); 
 			 echo<<"\"\n              data-rolecode=\"";
 echo<<a["rolecode"].to_string(); 
 			 echo<<"\">编辑</button>\n            <button class=\"btn btn-outline-danger btn-sm delbtn\" data-roleid=\"";
 echo<<a["roleid"].to_string(); 
 			 echo<<"\">删除</button>\n          </td>\n        </tr>\n        ";
 } 
 			 echo<<"        ";
 if(obj["role_count"].to_int()==0){ 
 			 echo<<"        <tr>\n          <td colspan=\"6\" class=\"text-center text-muted\">还没有角色，先在上方新建一个。</td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n    <div class=\"text-muted small mb-2\">\n      删除角色会一起清掉它的权限配置和本企业账户的绑定；被绑定的账户下一次请求即失去这些权限。\n    </div>\n\n    <a href=\"/admin/rbac/userrole\" class=\"btn btn-link\">用户角色</a>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    var CSRF = '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"';\n    function show(msg, ok) {\n      document.getElementById('result').innerHTML =\n        '<div class=\"alert alert-' + (ok ? 'success' : 'danger') + ' py-2 mb-0\">' + msg + '</div>';\n    }\n    function post(url, data) {\n      var fd = new FormData();\n      fd.append('csrf_token', CSRF);\n      for (var k in data) { fd.append(k, data[k]); }\n      return fetch(url, { method: 'POST', body: fd, credentials: 'same-origin' })\n        .then(function (r) { return r.json(); });\n    }\n    document.getElementById('addbtn').addEventListener('click', function () {\n      var roleid = document.getElementById('roleid').value;\n      var name = document.getElementById('name').value.trim();\n      if (name === '') { show('角色名称不能为空', false); return; }\n      var body = {\n        name: name,\n        status: document.getElementById('status').value,\n        sortid: document.getElementById('sortid').value,\n        rolecode: document.getElementById('rolecode').value\n      };\n      var url = '/admin/rbac/role/add';\n      if (roleid !== '0') { body.roleid = roleid; url = '/admin/rbac/role/edit'; }\n      post(url, body).then(function (j) {\n        if (j.code === 0) { show((roleid === '0' ? '已新建角色 ' : '已保存角色 ') + (j.roleid || roleid), true); setTimeout(function () { location.reload(); }, 700); }\n        else { show(j.msg || '保存失败', false); }\n      });\n    });\n    document.getElementById('resetbtn').addEventListener('click', function () {\n      document.getElementById('roleid').value = '0';\n      document.getElementById('name').value = '';\n      document.getElementById('status').value = '1';\n      document.getElementById('sortid').value = '0';\n      document.getElementById('rolecode').value = '0';\n    });\n    document.querySelectorAll('.editbtn').forEach(function (btn) {\n      btn.addEventListener('click', function () {\n        document.getElementById('roleid').value = btn.dataset.roleid;\n        document.getElementById('name').value = btn.dataset.name;\n        document.getElementById('status').value = btn.dataset.status;\n        document.getElementById('sortid').value = btn.dataset.sortid;\n        document.getElementById('rolecode').value = btn.dataset.rolecode;\n        show('正在编辑角色 ' + btn.dataset.roleid + '，改完点「新建角色」按钮保存（按钮此时是保存）。', true);\n      });\n    });\n    document.querySelectorAll('.delbtn').forEach(function (btn) {\n      btn.addEventListener('click', function () {\n        if (!confirm('删除角色 ' + btn.dataset.roleid + '？它的权限配置和账户绑定会一起清除。')) { return; }\n        post('/admin/rbac/role/delete', { roleid: btn.dataset.roleid }).then(function (j) {\n          if (j.code === 0) { show('已删除角色 ' + btn.dataset.roleid, true); setTimeout(function () { location.reload(); }, 700); }\n          else { show(j.msg || '删除失败', false); }\n        });\n      });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    