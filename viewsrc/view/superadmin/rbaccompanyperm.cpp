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
 		 std::string rbaccompanyperm([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>企业权限分配</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">企业权限分配</h5>\n    <div class=\"alert alert-info py-2\">\n      列表来自 <code>sysperms</code>（权限目录，<code>userid</code>=企业ID），勾选结果写入\n      <code>syscompanyperms</code>（企业权限分配表）。<br>\n      <strong>这一层只有平台能改</strong>：租户后台的可分配权限来自且仅来自这里勾选过的行，\n      租户管理员看不到也删不掉未分配给它的权限项。<br>\n      撤销一条分配后，该企业所有角色立刻失去对应权限，<code>sysroleperms</code> 的行不用动。\n      一项都不勾并保存＝撤销本企业全部分配（与角色权限页的「整体替换」同语义）。<br>\n      停用的权限项（<code>status=0</code>）可以保留分配，但在启用之前不会给任何用户访问权。\n    </div>\n\n    <div class=\"row g-2 mb-3\">\n      <div class=\"col-md-5\">\n        <label class=\"form-label mb-1\" for=\"company\">企业</label>\n        <select id=\"company\" class=\"form-select form-select-sm\">\n          ";
 for(auto &c:obj["companies"].as_array()){ 
 			 echo<<"          <option value=\"";
 echo<<c["sid"].to_string(); 
 			 echo<<"\" ";
 if(c["sid"].to_int()==obj["companyid"].to_int()){ 
 			 echo<<"selected";
 } 
 			 echo<<">sid ";
 echo<<c["sid"].to_string(); 
 			 echo<<" · ";
 echo<<c["sitename"].to_string(); 
 			 echo<<"</option>\n          ";
 } 
 			 echo<<"        </select>\n        <div class=\"form-text\">切换企业会重新载入该企业的分配状态。</div>\n      </div>\n      <div class=\"col-md-7 d-flex align-items-end flex-wrap gap-2\">\n        <div class=\"form-check\">\n          <input class=\"form-check-input\" type=\"checkbox\" id=\"checkall\">\n          <label class=\"form-check-label\" for=\"checkall\">全选当前列表</label>\n        </div>\n        <button class=\"btn btn-primary btn-sm\" id=\"savebtn\">保存分配</button>\n        <input id=\"filter\" class=\"form-control form-control-sm\" style=\"max-width:260px\"\n          placeholder=\"前端过滤：urlpath 或名称\">\n      </div>\n    </div>\n\n    <div class=\"mb-2\">\n      <span class=\"badge bg-secondary\">目录 ";
echo<<obj["catalog_count"].to_string(); 
 			 echo<<" 项</span>\n      <span class=\"badge bg-success\">启用 ";
echo<<obj["enabled_count"].to_string(); 
 			 echo<<" 项</span>\n      <span class=\"badge bg-warning text-dark\">停用 ";
echo<<obj["disabled_count"].to_string(); 
 			 echo<<" 项</span>\n      <span class=\"badge bg-primary\">已分配 ";
echo<<obj["allocated_count"].to_string(); 
 			 echo<<" 项</span>\n      ";
 if(obj["orphan_count"].to_int()>0){ 
 			 echo<<"<span class=\"badge bg-danger\">分配残留 ";
echo<<obj["orphan_count"].to_string(); 
 			 echo<<" 条</span>";
 } 
 			 echo<<"    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\" id=\"permtable\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:60px\">选择<br><small class=\"text-muted fw-normal\">勾选=分配给该企业</small></th>\n          <th style=\"width:90px\">权限ID<br><small class=\"text-muted fw-normal\">permsid</small></th>\n          <th>权限路径<br><small class=\"text-muted fw-normal\">urlpath</small></th>\n          <th>权限名称<br><small class=\"text-muted fw-normal\">name</small></th>\n          <th style=\"width:80px\">状态<br><small class=\"text-muted fw-normal\">status</small></th>\n          <th style=\"width:100px\">权限码<br><small class=\"text-muted fw-normal\">permscode</small></th>\n          <th style=\"width:100px\">掩码<br><small class=\"text-muted fw-normal\">maskcode</small></th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr>\n          <td>\n            <input class=\"form-check-input rowcheck\" type=\"checkbox\" value=\"";
 echo<<a["permsid"].to_string(); 
 			 echo<<"\"\n              ";
 if(a["checked"].to_int()==1){ 
 			 echo<<"checked";
 } 
 			 echo<<">\n          </td>\n          <td>";
 echo<<a["permsid"].to_string(); 
 			 echo<<"</td>\n          <td class=\"pathcell\">";
 echo<<a["urlpath"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["name"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 if(a["status"].to_int()==1){ 
 			 echo<<"<span class=\"badge bg-success\">启用</span>";
 }else{ 
 			 echo<<"<span class=\"badge bg-warning text-dark\">停用</span>";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["permscode"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["maskcode"].to_string(); 
 			 echo<<"</td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    document.getElementById('company').addEventListener('change', function () {\n      location.href = '/superadmin/rbac/companyperm?companyid=' + this.value;\n    });\n\n    document.getElementById('checkall').addEventListener('change', function () {\n      document.querySelectorAll('#permtable tbody tr').forEach(function (tr) {\n        var cb = tr.querySelector('.rowcheck');\n        if (cb && tr.style.display !== 'none') { cb.checked = document.getElementById('checkall').checked; }\n      });\n    });\n\n    document.getElementById('filter').addEventListener('input', function () {\n      var kw = this.value.trim().toLowerCase();\n      document.querySelectorAll('#permtable tbody tr').forEach(function (tr) {\n        var cell = tr.querySelector('.pathcell');\n        var text = (cell ? cell.textContent : tr.textContent).toLowerCase();\n        tr.style.display = (kw === '' || text.indexOf(kw) >= 0) ? '' : 'none';\n      });\n    });\n\n    document.getElementById('savebtn').addEventListener('click', function () {\n      var box = document.getElementById('result');\n      var picked = [];\n      document.querySelectorAll('.rowcheck:checked').forEach(function (cb) { picked.push(cb.value); });\n      var fd = new FormData();\n      fd.append('csrf_token', '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"');\n      fd.append('companyid', '";
echo<<obj["companyid"].to_string(); 
 			 echo<<"');\n      fd.append('permsids', picked.join(','));\n      fetch('/superadmin/rbac/companyperm/save', { method: 'POST', body: fd, credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            box.innerHTML = '<div class=\"alert alert-success py-2 mb-0\">' +\n              (j.msg || '企业权限分配已保存') + '：当前共 ' + j.count + ' 项（已整体替换）</div>';\n          } else {\n            box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">保存失败：' +\n              (j.msg || '请求未成功') + '</div>';\n          }\n        })\n        .catch(function (e) {\n          box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">请求失败：' + e + '</div>';\n        });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    