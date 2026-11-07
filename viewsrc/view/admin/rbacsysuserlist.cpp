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
 		 std::string rbacsysuserlist([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>添加管理员</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">添加管理员</h5>\n    <div class=\"text-muted mb-2\">\n      本企业（companyid）：";
echo<<obj["companyid"].to_string(); 
 			 echo<<"；\n      账户数：";
echo<<obj["user_total"].to_string(); 
 			 echo<<"；\n      本页可管理的子管理员：";
echo<<obj["subadmin_count"].to_string(); 
 			 echo<<"    </div>\n\n    <div class=\"alert alert-info py-2\">\n      这里建的账户写进 <code>sysuser</code>：<code>companyid</code>=本企业、<code>isadmin</code>=0（不是租户超级管理员）、\n      密码 MD5 后存 <code>password</code>。登录名全站唯一（登录只按登录名查），4–26 字符；密码 6–20 字符。<br>\n      建好只是「能登录」，能看到哪些页面还要到「用户角色」页给它绑角色，角色权限来自「角色权限」页。<br>\n      添加和删除只有 <code>isadmin=1</code> 的租户管理员能操作；租户超级管理员自己和本公司的平台侧账户不在本页管理范围，\n      删除只会删 <code>isadmin=0</code> 的本企业账户，并一起清掉它的角色绑定。\n    </div>\n\n    <div class=\"row g-2 mb-3\">\n      <div class=\"col-md-3\">\n        <label class=\"form-label mb-1\" for=\"name\">登录名</label>\n        <input id=\"name\" class=\"form-control\" maxlength=\"26\" placeholder=\"4-26 字符，全站唯一\">\n      </div>\n      <div class=\"col-md-2\">\n        <label class=\"form-label mb-1\" for=\"password\">密码</label>\n        <input id=\"password\" class=\"form-control\" type=\"password\" maxlength=\"20\" placeholder=\"6-20 字符\">\n      </div>\n      <div class=\"col-md-3\">\n        <label class=\"form-label mb-1\" for=\"nickname\">显示名称</label>\n        <input id=\"nickname\" class=\"form-control\" maxlength=\"60\" placeholder=\"例如：内容编辑小王\">\n      </div>\n      <div class=\"col-md-2\">\n        <label class=\"form-label mb-1\" for=\"isopen\">状态</label>\n        <select id=\"isopen\" class=\"form-select\">\n          <option value=\"1\">启用</option>\n          <option value=\"0\">停用</option>\n        </select>\n      </div>\n      <div class=\"col-md-2 d-flex align-items-end gap-2\">\n        <button class=\"btn btn-primary btn-sm\" id=\"addbtn\">添加管理员</button>\n        <button class=\"btn btn-outline-secondary btn-sm\" id=\"resetbtn\">清空</button>\n      </div>\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:90px\">adminid</th>\n          <th>登录名 / 显示名称</th>\n          <th style=\"width:130px\">身份</th>\n          <th style=\"width:90px\">状态</th>\n          <th style=\"width:110px\">绑定角色数</th>\n          <th style=\"width:160px\">创建时间</th>\n          <th style=\"width:130px\">操作</th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr>\n          <td>";
 echo<<a["adminid"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 echo<<a["name"].to_string(); 
 			 echo<<"            <div class=\"text-muted small\">";
 echo<<a["nickname"].to_string(); 
 			 echo<<"</div>\n          </td>\n          <td>\n            ";
 if(a["isadmin"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-dark\">租户超级管理员</span>\n            ";
 }else if(a["self"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-secondary\">本人</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-info text-dark\">子管理员</span>\n            ";
 } 
 			 echo<<"          </td>\n          <td>\n            ";
 if(a["isopen"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-success\">启用</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-secondary\">停用</span>\n            ";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["role_count"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["created_date"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 if(a["isadmin"].to_int()==0){ 
 			 echo<<"            <button class=\"btn btn-outline-danger btn-sm delbtn\" data-adminid=\"";
 echo<<a["adminid"].to_string(); 
 			 echo<<"\"\n              data-name=\"";
 echo<<a["name"].to_string(); 
 			 echo<<"\">删除</button>\n            ";
 } 
 			 echo<<"            <a class=\"btn btn-outline-secondary btn-sm\" href=\"/admin/rbac/userrole?userid=";
 echo<<a["adminid"].to_string(); 
 			 echo<<"\">配角色</a>\n          </td>\n        </tr>\n        ";
 } 
 			 echo<<"        ";
 if(obj["user_total"].to_int()==0){ 
 			 echo<<"        <tr>\n          <td colspan=\"7\" class=\"text-center text-muted\">本企业还没有账户。</td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n    <div class=\"text-muted small mb-2\">\n      「租户超级管理员」是登录时写进会话 <code>isadmin</code> 的那一个账户，本页只读不删；\n      停用改的是 <code>sysuser.isopen</code>：登录要求这一列为 1，所以停用后不能重新登录；本页这类租户管理页每次请求都回查该列，\n      挂权限掩码的接口在装载授权时同样要求这一列为 1，所以已登录的会话也会立刻失效。超管侧「用户」页的启停按钮还会即时清掉该账户的授权快照（本页无启停，只在添加时选状态）。\n      唯一残留：未挂权限掩码的旧后台页面只看会话是否登录，那类页面在旧会话登出或过期前仍可用，要彻底封住请删除账户或解除角色绑定。\n    </div>\n\n    <a href=\"/admin/rbac/userrole\" class=\"btn btn-link\">用户角色</a>\n    <a href=\"/admin/rbac/role\" class=\"btn btn-link\">角色管理</a>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    var CSRF = '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"';\n    function show(msg, ok) {\n      document.getElementById('result').innerHTML =\n        '<div class=\"alert alert-' + (ok ? 'success' : 'danger') + ' py-2 mb-0\">' + msg + '</div>';\n    }\n    function post(url, data) {\n      var fd = new FormData();\n      fd.append('csrf_token', CSRF);\n      for (var k in data) { fd.append(k, data[k]); }\n      return fetch(url, { method: 'POST', body: fd, credentials: 'same-origin' })\n        .then(function (r) { return r.json(); });\n    }\n    document.getElementById('addbtn').addEventListener('click', function () {\n      var body = {\n        name: document.getElementById('name').value.trim(),\n        password: document.getElementById('password').value,\n        nickname: document.getElementById('nickname').value.trim(),\n        isopen: document.getElementById('isopen').value\n      };\n      if (body.name === '') { show('登录名不能为空', false); return; }\n      if (body.password === '') { show('密码不能为空', false); return; }\n      post('/admin/rbac/sysuser/add', body).then(function (j) {\n        if (j.code === 0) {\n          show('已添加管理员（adminid ' + (j.adminid || '?') + '），记得去「用户角色」页给它绑角色。', true);\n          setTimeout(function () { location.reload(); }, 900);\n        } else { show(j.msg || '添加失败', false); }\n      });\n    });\n    document.getElementById('resetbtn').addEventListener('click', function () {\n      document.getElementById('name').value = '';\n      document.getElementById('password').value = '';\n      document.getElementById('nickname').value = '';\n      document.getElementById('isopen').value = '1';\n    });\n    document.querySelectorAll('.delbtn').forEach(function (btn) {\n      btn.addEventListener('click', function () {\n        if (!confirm('删除账户 ' + btn.dataset.name + '？它的角色绑定会一起清除，下一次请求即失去全部权限。')) { return; }\n        post('/admin/rbac/sysuser/delete', { userid: btn.dataset.adminid }).then(function (j) {\n          if (j.code === 0) { show('已删除账户 ' + btn.dataset.name, true); setTimeout(function () { location.reload(); }, 900); }\n          else { show(j.msg || '删除失败', false); }\n        });\n      });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    