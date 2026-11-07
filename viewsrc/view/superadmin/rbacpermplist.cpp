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
 		 std::string rbacpermplist([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>权限管理</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">权限管理</h5>\n    <div class=\"alert alert-info py-2\">\n      对应数据表 <code>sysperms</code>，权限项由「注册函数列表」按路由自动登记。\n      <code>urlpath</code> 与 <code>maskcode</code> 来自路由注册结果，本页不可修改；\n      如需变更请到注册函数页重新登记。只有超级管理员可以编辑本表。<br>\n      本页只管权限目录；某企业能否用到其中一条，还要在「企业权限分配」页分配给它。\n    </div>\n\n    <div class=\"card mb-3\">\n      <div class=\"card-body py-2\">\n        <form class=\"row g-2 align-items-end\" method=\"get\" action=\"/superadmin/rbac/perm\">\n          <div class=\"col-md-4\">\n            <label class=\"form-label mb-1\" for=\"query-userid\">归属用户ID <code>userid</code></label>\n            <input class=\"form-control form-control-sm\" id=\"query-userid\" name=\"userid\" type=\"number\" min=\"0\"\n              placeholder=\"留空或 0=全部；填公司 ID 只看该公司权限\"\n              value=\"";
echo<<obj["query_userid"].to_string(); 
 			 echo<<"\">\n            <div class=\"form-text\">留空或 0 表示不过滤（列出全部），大于 0 表示只看该公司专属权限。</div>\n          </div>\n          <div class=\"col-md-5\">\n            <button class=\"btn btn-outline-secondary btn-sm\" type=\"submit\">筛选</button>\n            <a class=\"btn btn-link btn-sm\" href=\"/superadmin/rbac/perm\">重置</a>\n          </div>\n        </form>\n      </div>\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:80px\">权限ID<br><small class=\"text-muted fw-normal\">permsid</small></th>\n          <th style=\"width:90px\">归属用户ID<br><small class=\"text-muted fw-normal\">userid</small></th>\n          <th style=\"width:90px\">父级ID<br><small class=\"text-muted fw-normal\">parentid</small></th>\n          <th>权限路径<br><small class=\"text-muted fw-normal\">urlpath</small></th>\n          <th>路径别名<br><small class=\"text-muted fw-normal\">urlname</small></th>\n          <th>权限名称<br><small class=\"text-muted fw-normal\">name</small></th>\n          <th style=\"width:80px\">状态<br><small class=\"text-muted fw-normal\">status</small></th>\n          <th style=\"width:90px\">权限码<br><small class=\"text-muted fw-normal\">permscode</small></th>\n          <th style=\"width:90px\">掩码<br><small class=\"text-muted fw-normal\">maskcode</small></th>\n          <th style=\"width:70px\">只读<br><small class=\"text-muted fw-normal\">readonly</small></th>\n          <th style=\"width:150px\">创建时间<br><small class=\"text-muted fw-normal\">created_time</small></th>\n          <th style=\"width:150px\">操作</th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr>\n          <td>";
 echo<<a["permsid"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 echo<<a["userid"].to_string(); 
 			 echo<<"            ";
 if(a["userid"].to_int()==0){ 
 			 echo<<"<span class=\"badge bg-info text-dark\">全局</span>";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["parentid"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["urlpath"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["urlname"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["name"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 if(a["status"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-success\">启用</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-secondary\">关闭</span>\n            ";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["permscode"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["maskcode"].to_string(); 
 			 echo<<"</td>\n          <td>";
 if(a["readonly"].to_int()==1){ 
 			 echo<<"是（1）";
 }else{ 
 			 echo<<"否（0）";
 } 
 			 echo<<"</td>\n          <td>";
 echo<<a["created_time"].to_string(); 
 			 echo<<"</td>\n          <td>\n            <button type=\"button\" class=\"btn btn-outline-primary btn-sm edit-btn\"\n              data-permsid=\"";
 echo<<a["permsid"].to_string(); 
 			 echo<<"\"\n              data-userid=\"";
 echo<<a["userid"].to_string(); 
 			 echo<<"\"\n              data-urlpath=\"";
 echo<<a["urlpath"].to_string(); 
 			 echo<<"\"\n              data-urlname=\"";
 echo<<a["urlname"].to_string(); 
 			 echo<<"\"\n              data-name=\"";
 echo<<a["name"].to_string(); 
 			 echo<<"\"\n              data-parentid=\"";
 echo<<a["parentid"].to_string(); 
 			 echo<<"\"\n              data-status=\"";
 echo<<a["status"].to_string(); 
 			 echo<<"\"\n              data-permscode=\"";
 echo<<a["permscode"].to_string(); 
 			 echo<<"\"\n              data-maskcode=\"";
 echo<<a["maskcode"].to_string(); 
 			 echo<<"\"\n              data-readonly=\"";
 echo<<a["readonly"].to_string(); 
 			 echo<<"\">编辑</button>\n            <button type=\"button\" class=\"btn btn-outline-warning btn-sm toggle-btn\"\n              data-permsid=\"";
 echo<<a["permsid"].to_string(); 
 			 echo<<"\"\n              data-name=\"";
 echo<<a["name"].to_string(); 
 			 echo<<"\"\n              data-status=\"";
 echo<<a["status"].to_string(); 
 			 echo<<"\">启停</button>\n          </td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n  </div>\n\n  <div class=\"modal fade\" id=\"editModal\" tabindex=\"-1\" aria-labelledby=\"editModalLabel\" aria-hidden=\"true\">\n    <div class=\"modal-dialog\">\n      <div class=\"modal-content\">\n        <div class=\"modal-header\">\n          <h5 class=\"modal-title\" id=\"editModalLabel\">编辑权限</h5>\n          <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"关闭\"></button>\n        </div>\n        <form id=\"editform\" action=\"/superadmin/rbac/perm/editpost\" method=\"post\">\n          <div class=\"modal-body\">\n            <input type=\"hidden\" name=\"csrf_token\" value=\"";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"\">\n            <input type=\"hidden\" name=\"permsid\" id=\"edit-permsid\">\n\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-permsid-view\">\n                权限ID <code>permsid</code> <span class=\"text-muted\">自动生成，不可修改</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-permsid-view\" readonly disabled>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-userid-view\">\n                归属用户ID <code>userid</code> <span class=\"text-muted\">不可修改</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-userid-view\" readonly disabled>\n              <div class=\"form-text\">该权限项归属的公司 ID；0 是历史遗留的无效归属，待清理。如需变更请重新登记。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-urlpath-view\">\n                权限路径 <code>urlpath</code> <span class=\"text-muted\">只读</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-urlpath-view\" readonly disabled>\n              <div class=\"form-text\">来自路由注册键，本页不可修改。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-maskcode-view\">\n                掩码 <code>maskcode</code> <span class=\"text-muted\">只读</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-maskcode-view\" readonly disabled>\n              <div class=\"form-text\">与路由注册的 mask_code 对应，0 表示不参与 RBAC 分流。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-parentid\">\n                父级权限ID <code>parentid</code> <span class=\"text-muted\">留空=0（顶级）</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-parentid\" name=\"parentid\" type=\"number\" min=\"0\" placeholder=\"0\">\n              <div class=\"form-text\">用于权限树分组，0 表示顶级节点。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-urlname\">\n                路径别名 <code>urlname</code> <span class=\"text-muted\">留空=urlpath</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-urlname\" name=\"urlname\" maxlength=\"60\"\n                placeholder=\"例如：订单列表-管理员\">\n              <div class=\"form-text\">展示用别名，留空则自动使用 urlpath。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-name\">\n                权限名称 <code>name</code> <span class=\"text-muted\">留空=urlpath</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-name\" name=\"name\" maxlength=\"60\" placeholder=\"例如：订单管理\">\n              <div class=\"form-text\">给管理员看的名称，留空则自动使用 urlpath。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-status\">状态 <code>status</code></label>\n              <select class=\"form-select\" id=\"edit-status\" name=\"status\">\n                <option value=\"1\">启用（1）：可被角色勾选授权</option>\n                <option value=\"0\">关闭（0）：不参与授权</option>\n              </select>\n              <div class=\"form-text\">关闭后不会出现在角色权限勾选列表。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-permscode\">\n                权限码 <code>permscode</code> <span class=\"text-muted\">留空=0</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-permscode\" name=\"permscode\" type=\"number\" min=\"0\" placeholder=\"0\">\n              <div class=\"form-text\">业务自定义数字，0=未设置。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-readonly\">只读 <code>readonly</code></label>\n              <select class=\"form-select\" id=\"edit-readonly\" name=\"readonly\">\n                <option value=\"0\">否（0）：允许编辑</option>\n                <option value=\"1\">是（1）：内置只读，不建议修改</option>\n              </select>\n              <div class=\"form-text\">标记内置权限，防止误改。</div>\n            </div>\n          </div>\n          <div class=\"modal-footer\">\n            <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">取消</button>\n            <button type=\"submit\" class=\"btn btn-primary\">保存</button>\n          </div>\n        </form>\n      </div>\n    </div>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    function showResult(type, text) {\n      document.getElementById('result').innerHTML =\n        '<div class=\"alert alert-' + type + ' py-2 mb-0\">' + text + '</div>';\n    }\n\n    var editModal = new bootstrap.Modal(document.getElementById('editModal'));\n\n    document.querySelectorAll('.edit-btn').forEach(function (btn) {\n      btn.addEventListener('click', function () {\n        document.getElementById('edit-permsid').value = this.dataset.permsid;\n        document.getElementById('edit-permsid-view').value = this.dataset.permsid;\n        document.getElementById('edit-userid-view').value = this.dataset.userid;\n        document.getElementById('edit-urlpath-view').value = this.dataset.urlpath;\n        document.getElementById('edit-maskcode-view').value = this.dataset.maskcode;\n        document.getElementById('edit-parentid').value = this.dataset.parentid;\n        document.getElementById('edit-urlname').value = this.dataset.urlname;\n        document.getElementById('edit-name').value = this.dataset.name;\n        document.getElementById('edit-status').value = this.dataset.status;\n        document.getElementById('edit-permscode').value = this.dataset.permscode;\n        document.getElementById('edit-readonly').value = this.dataset.readonly;\n        editModal.show();\n      });\n    });\n\n    document.getElementById('editform').addEventListener('submit', function (e) {\n      e.preventDefault();\n      fetch(this.action, { method: 'POST', body: new FormData(this), credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            showResult('success', (j.msg || '权限保存成功') + '，下次带 maskcode 的请求立即生效');\n            setTimeout(function () { location.reload(); }, 800);\n          } else {\n            showResult('danger', '保存失败：' + (j.msg || ('HTTP ' + r.status)));\n          }\n        })\n        .catch(function (e) {\n          showResult('danger', '请求失败：' + e);\n        });\n    });\n\n    document.querySelectorAll('.toggle-btn').forEach(function (btn) {\n      btn.addEventListener('click', function () {\n        var nextStatus = this.dataset.status === '1' ? '0' : '1';\n        var label = nextStatus === '1' ? '启用' : '关闭';\n        if (!confirm('确定' + label + '权限「' + this.dataset.name + '」？')) {\n          return;\n        }\n        var fd = new FormData();\n        fd.append('csrf_token', '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"');\n        fd.append('permsid', this.dataset.permsid);\n        fd.append('status', nextStatus);\n        fetch('/superadmin/rbac/perm/toggle', { method: 'POST', body: fd, credentials: 'same-origin' })\n          .then(function (r) { return r.json(); })\n          .then(function (j) {\n            if (j.code === 0) {\n              showResult('success', '权限「' + this.dataset.name + '」已' + label +\n                '（status=' + j.status + '）');\n              setTimeout(function () { location.reload(); }, 800);\n            } else {\n              showResult('danger', '操作失败：' + (j.msg || ('HTTP ' + r.status)));\n            }\n          }.bind(this))\n          .catch(function (e) {\n            showResult('danger', '请求失败：' + e);\n          });\n      });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    