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
 		 std::string rbacuserlist([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>用户管理</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <div class=\"d-flex justify-content-between align-items-center mt-2 mb-2\">\n      <h5 class=\"mb-0\">用户管理</h5>\n      <a class=\"btn btn-primary btn-sm\" href=\"/superadmin/rbac/user/add\">新增用户</a>\n    </div>\n\n    <div class=\"alert alert-info py-2\">\n      对应数据表 <code>sysuser</code>。<code>isadmin=1</code> 表示租户管理员，可在租户后台管理本公司的角色与授权；\n      <code>isadmin=0</code> 为普通用户，只能被分配权限。用户可持有的角色在「角色」页维护。\n    </div>\n\n    <div class=\"card mb-3\">\n      <div class=\"card-body py-2\">\n        <form class=\"row g-2 align-items-end\" method=\"get\" action=\"/superadmin/rbac/user\">\n          <div class=\"col-md-4\">\n            <label class=\"form-label mb-1\" for=\"query-name\">登录名 <code>name</code></label>\n            <input class=\"form-control form-control-sm\" id=\"query-name\" name=\"name\" placeholder=\"输入登录名关键词，支持模糊匹配\"\n              value=\"";
echo<<obj["query_name"].to_string(); 
 			 echo<<"\">\n          </div>\n          <div class=\"col-md-3\">\n            <label class=\"form-label mb-1\" for=\"query-companyid\">公司ID <code>companyid</code></label>\n            <input class=\"form-control form-control-sm\" id=\"query-companyid\" name=\"companyid\" type=\"number\" min=\"0\"\n              placeholder=\"留空=全部公司\" value=\"";
echo<<obj["query_companyid"].to_string(); 
 			 echo<<"\">\n          </div>\n          <div class=\"col-md-5\">\n            <button class=\"btn btn-outline-secondary btn-sm\" type=\"submit\">筛选</button>\n            <a class=\"btn btn-link btn-sm\" href=\"/superadmin/rbac/user\">重置</a>\n            <span class=\"text-muted ms-2\">登录名模糊匹配；公司ID精确匹配，留空不限制。</span>\n          </div>\n        </form>\n      </div>\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:90px\">用户ID<br><small class=\"text-muted fw-normal\">adminid</small></th>\n          <th>登录名<br><small class=\"text-muted fw-normal\">name</small></th>\n          <th>昵称<br><small class=\"text-muted fw-normal\">nickname</small></th>\n          <th style=\"width:100px\">公司ID<br><small class=\"text-muted fw-normal\">companyid</small></th>\n          <th style=\"width:90px\">管理员<br><small class=\"text-muted fw-normal\">isadmin</small></th>\n          <th style=\"width:80px\">状态<br><small class=\"text-muted fw-normal\">isopen</small></th>\n          <th style=\"width:110px\">到期日期<br><small class=\"text-muted fw-normal\">enddate</small></th>\n          <th style=\"width:190px\">操作</th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr>\n          <td>";
 echo<<a["adminid"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["name"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["nickname"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["companyid"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 if(a["isadmin"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-primary\">是（1）</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-light text-dark\">否（0）</span>\n            ";
 } 
 			 echo<<"          </td>\n          <td>\n            ";
 if(a["isopen"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-success\">启用（1）</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-secondary\">关闭（0）</span>\n            ";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["enddate"].to_string(); 
 			 echo<<"</td>\n          <td>\n            <a class=\"btn btn-outline-primary btn-sm\" href=\"/superadmin/rbac/user/edit?userid=";
 echo<<a["adminid"].to_string(); 
 			 echo<<"\">编辑</a>\n            <a class=\"btn btn-outline-secondary btn-sm\" href=\"/superadmin/rbac/userrole?userid=";
 echo<<a["adminid"].to_string(); 
 			 echo<<"\">角色</a>\n            <button type=\"button\" class=\"btn btn-outline-warning btn-sm toggle-btn\"\n              data-userid=\"";
 echo<<a["adminid"].to_string(); 
 			 echo<<"\"\n              data-name=\"";
 echo<<a["name"].to_string(); 
 			 echo<<"\"\n              data-isopen=\"";
 echo<<a["isopen"].to_string(); 
 			 echo<<"\">启停</button>\n          </td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n    <nav>\n      <ul class=\"pagination pagination-sm mb-3\">\n        ";
 for(long long p = obj["bar_min"].to_int(); p <= obj["bar_max"].to_int(); ++p){ 
 			 echo<<"        <li class=\"page-item ";
 if(p==obj["page"].to_int()){ 
 			 echo<<"active";
 } 
 			 echo<<"\">\n          <a class=\"page-link\"\n            href=\"/superadmin/rbac/user?page=";
 echo<<p; 
 			 echo<<"&name=";
 echo<<obj["query_name"].to_string(); 
 			 echo<<"&companyid=";
 echo<<obj["query_companyid"].to_string(); 
 			 echo<<"\">";
 echo<<p; 
 			 echo<<"</a>\n        </li>\n        ";
 } 
 			 echo<<"      </ul>\n      <div class=\"text-muted\">共 ";
 echo<<obj["total_page"].to_string(); 
 			 echo<<" 页，每页 20 条</div>\n    </nav>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    function showResult(type, text) {\n      document.getElementById('result').innerHTML =\n        '<div class=\"alert alert-' + type + ' py-2 mb-0\">' + text + '</div>';\n    }\n\n    document.querySelectorAll('.toggle-btn').forEach(function (btn) {\n      btn.addEventListener('click', function () {\n        var nextOpen = this.dataset.isopen === '1' ? '0' : '1';\n        var label = nextOpen === '1' ? '启用' : '关闭';\n        if (!confirm('确定' + label + '用户「' + this.dataset.name + '」？')) {\n          return;\n        }\n        var fd = new FormData();\n        fd.append('csrf_token', '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"');\n        fd.append('userid', this.dataset.userid);\n        fd.append('isopen', nextOpen);\n        fetch('/superadmin/rbac/user/toggle', { method: 'POST', body: fd, credentials: 'same-origin' })\n          .then(function (r) { return r.json(); })\n          .then(function (j) {\n            if (j.code === 0) {\n              showResult('success', '用户「' + this.dataset.name + '」已' + label +\n                '（isopen=' + j.isopen + '）');\n              setTimeout(function () { location.reload(); }, 800);\n            } else {\n              showResult('danger', '操作失败：' + (j.msg || ('HTTP ' + r.status)));\n            }\n          }.bind(this))\n          .catch(function (e) {\n            showResult('danger', '请求失败：' + e);\n          });\n      });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    