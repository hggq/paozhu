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
 		 std::string rbacrolelist([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>角色管理</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">角色管理</h5>\n    <div class=\"alert alert-info py-2\">\n      对应数据表 <code>sysrole</code>。角色本身只是「一组权限的容器」，需要到「权限」页给角色绑定\n      <code>sysperms</code>，再在用户角色页把角色分配给用户。<br>\n      <code>userid</code> 是该角色归属的公司/租户 id，必须填真实企业 id；没有全局角色，0 会被拒绝。\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <div class=\"card mb-3\">\n      <div class=\"card-header py-2\">新增角色</div>\n      <div class=\"card-body\">\n        <form id=\"addform\" class=\"row g-3\" action=\"/superadmin/rbac/role/addpost\" method=\"post\">\n          <input type=\"hidden\" name=\"csrf_token\" value=\"";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"\">\n\n          <div class=\"col-md-3\">\n            <label class=\"form-label\" for=\"add-name\">\n              角色名称 <code>name</code> <span class=\"text-danger\">*必填</span>\n            </label>\n            <input class=\"form-control form-control-sm\" id=\"add-name\" name=\"name\" placeholder=\"例如：订单管理员\" required\n              minlength=\"2\" maxlength=\"60\">\n            <div class=\"form-text\">2-60 个字符，用于展示和分配，例如「订单管理员」。</div>\n          </div>\n\n          <div class=\"col-md-2\">\n            <label class=\"form-label\" for=\"add-userid\">\n              归属用户ID <code>userid</code> <span class=\"text-danger\">*必填</span>\n            </label>\n            <input class=\"form-control form-control-sm\" id=\"add-userid\" name=\"userid\" type=\"number\" min=\"1\" value=\"\"\n              required>\n            <div class=\"form-text\">填公司 ID（如 1）= 该租户专属角色；不能为 0。</div>\n          </div>\n\n          <div class=\"col-md-2\">\n            <label class=\"form-label\" for=\"add-status\">\n              状态 <code>status</code> <span class=\"text-danger\">*必填</span>\n            </label>\n            <select class=\"form-select form-select-sm\" id=\"add-status\" name=\"status\">\n              <option value=\"1\" selected>启用（1）</option>\n              <option value=\"0\">关闭（0）</option>\n            </select>\n            <div class=\"form-text\">1=启用，可被分配权限与用户；0=关闭，暂不参与授权。</div>\n          </div>\n\n          <div class=\"col-md-2\">\n            <label class=\"form-label\" for=\"add-rolecode\">\n              角色编码 <code>rolecode</code> <span class=\"text-muted\">选填</span>\n            </label>\n            <input class=\"form-control form-control-sm\" id=\"add-rolecode\" name=\"rolecode\" type=\"number\" min=\"0\"\n              placeholder=\"留空=0，例如 1001\">\n            <div class=\"form-text\">业务自定义数字标识，留空按 0 保存。</div>\n          </div>\n\n          <div class=\"col-md-2\">\n            <label class=\"form-label\" for=\"add-sortid\">\n              排序 <code>sortid</code> <span class=\"text-muted\">选填</span>\n            </label>\n            <input class=\"form-control form-control-sm\" id=\"add-sortid\" name=\"sortid\" type=\"number\" value=\"0\">\n            <div class=\"form-text\">数字越小越靠前，留空默认 0。</div>\n          </div>\n\n          <div class=\"col-md-1\">\n            <label class=\"form-label\" for=\"add-rolevalue\">\n              角色值 <code>rolevalue</code>\n            </label>\n            <input class=\"form-control form-control-sm\" id=\"add-rolevalue\" name=\"rolevalue\" type=\"number\" min=\"0\"\n              value=\"0\">\n            <div class=\"form-text\">业务数值，留空默认 0。</div>\n          </div>\n\n          <div class=\"col-md-2\">\n            <label class=\"form-label\" for=\"add-roleid\">\n              角色ID <code>roleid</code> <span class=\"text-muted\">自动</span>\n            </label>\n            <input class=\"form-control form-control-sm\" id=\"add-roleid\" value=\"保存后自动生成\" readonly disabled>\n            <div class=\"form-text\">自增主键，无需填写。</div>\n          </div>\n\n          <div class=\"col-12\">\n            <button class=\"btn btn-primary btn-sm\" type=\"submit\">添加角色</button>\n          </div>\n        </form>\n      </div>\n    </div>\n\n    <table class=\"table table-sm table-bordered align-middle\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:90px\">角色ID<br><small class=\"text-muted fw-normal\">roleid</small></th>\n          <th style=\"width:100px\">归属用户ID<br><small class=\"text-muted fw-normal\">userid</small></th>\n          <th>角色名称<br><small class=\"text-muted fw-normal\">name</small></th>\n          <th style=\"width:90px\">状态<br><small class=\"text-muted fw-normal\">status</small></th>\n          <th style=\"width:110px\">角色编码<br><small class=\"text-muted fw-normal\">rolecode</small></th>\n          <th style=\"width:80px\">排序<br><small class=\"text-muted fw-normal\">sortid</small></th>\n          <th style=\"width:100px\">角色值<br><small class=\"text-muted fw-normal\">rolevalue</small></th>\n          <th style=\"width:210px\">操作</th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr>\n          <td>";
 echo<<a["roleid"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 echo<<a["userid"].to_string(); 
 			 echo<<"            ";
 if(a["userid"].to_int()==0){ 
 			 echo<<"<span class=\"badge bg-info text-dark\">全局</span>";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["name"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 if(a["status"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-success\">启用</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-secondary\">关闭</span>\n            ";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["rolecode"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["sortid"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["rolevalue"].to_string(); 
 			 echo<<"</td>\n          <td>\n            <button type=\"button\" class=\"btn btn-outline-primary btn-sm edit-btn\"\n              data-roleid=\"";
 echo<<a["roleid"].to_string(); 
 			 echo<<"\"\n              data-userid=\"";
 echo<<a["userid"].to_string(); 
 			 echo<<"\"\n              data-name=\"";
 echo<<a["name"].to_string(); 
 			 echo<<"\"\n              data-status=\"";
 echo<<a["status"].to_string(); 
 			 echo<<"\"\n              data-rolecode=\"";
 echo<<a["rolecode"].to_string(); 
 			 echo<<"\"\n              data-sortid=\"";
 echo<<a["sortid"].to_string(); 
 			 echo<<"\"\n              data-rolevalue=\"";
 echo<<a["rolevalue"].to_string(); 
 			 echo<<"\">编辑</button>\n            <a class=\"btn btn-outline-secondary btn-sm\" href=\"/superadmin/rbac/roleperm?roleid=";
 echo<<a["roleid"].to_string(); 
 			 echo<<"\">权限</a>\n            <button type=\"button\" class=\"btn btn-outline-danger btn-sm delete-btn\"\n              data-roleid=\"";
 echo<<a["roleid"].to_string(); 
 			 echo<<"\"\n              data-name=\"";
 echo<<a["name"].to_string(); 
 			 echo<<"\">删除</button>\n          </td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n  </div>\n\n  <div class=\"modal fade\" id=\"editModal\" tabindex=\"-1\" aria-labelledby=\"editModalLabel\" aria-hidden=\"true\">\n    <div class=\"modal-dialog\">\n      <div class=\"modal-content\">\n        <div class=\"modal-header\">\n          <h5 class=\"modal-title\" id=\"editModalLabel\">编辑角色</h5>\n          <button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\" aria-label=\"关闭\"></button>\n        </div>\n        <form id=\"editform\" action=\"/superadmin/rbac/role/editpost\" method=\"post\">\n          <div class=\"modal-body\">\n            <input type=\"hidden\" name=\"csrf_token\" value=\"";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"\">\n            <input type=\"hidden\" name=\"roleid\" id=\"edit-roleid\">\n\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-roleid-view\">\n                角色ID <code>roleid</code> <span class=\"text-muted\">自动生成，不可修改</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-roleid-view\" readonly disabled>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-userid-view\">\n                归属用户ID <code>userid</code> <span class=\"text-muted\">不可修改</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-userid-view\" readonly disabled>\n              <div class=\"form-text\">该角色归属的公司 ID。如需变更归属，请删除后重新创建。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-name\">\n                角色名称 <code>name</code> <span class=\"text-danger\">*必填</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-name\" name=\"name\" placeholder=\"例如：订单管理员\" required minlength=\"2\"\n                maxlength=\"60\">\n              <div class=\"form-text\">2-60 个字符。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-status\">状态 <code>status</code></label>\n              <select class=\"form-select\" id=\"edit-status\" name=\"status\">\n                <option value=\"1\">启用（1）：可被分配权限与用户</option>\n                <option value=\"0\">关闭（0）：暂不参与授权</option>\n              </select>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-rolecode\">\n                角色编码 <code>rolecode</code> <span class=\"text-muted\">选填</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-rolecode\" name=\"rolecode\" type=\"number\" min=\"0\" placeholder=\"留空=0\">\n              <div class=\"form-text\">业务自定义数字标识，留空按 0 保存。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-sortid\">\n                排序 <code>sortid</code> <span class=\"text-muted\">选填</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-sortid\" name=\"sortid\" type=\"number\" placeholder=\"留空=0\">\n              <div class=\"form-text\">数字越小越靠前。</div>\n            </div>\n            <div class=\"mb-3\">\n              <label class=\"form-label\" for=\"edit-rolevalue\">\n                角色值 <code>rolevalue</code> <span class=\"text-muted\">选填</span>\n              </label>\n              <input class=\"form-control\" id=\"edit-rolevalue\" name=\"rolevalue\" type=\"number\" min=\"0\" placeholder=\"留空=0\">\n              <div class=\"form-text\">业务数值，留空按 0 保存。</div>\n            </div>\n          </div>\n          <div class=\"modal-footer\">\n            <button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">取消</button>\n            <button type=\"submit\" class=\"btn btn-primary\">保存</button>\n          </div>\n        </form>\n      </div>\n    </div>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    function showResult(type, text) {\n      document.getElementById('result').innerHTML =\n        '<div class=\"alert alert-' + type + ' py-2 mb-0\">' + text + '</div>';\n    }\n\n    document.getElementById('addform').addEventListener('submit', function (e) {\n      e.preventDefault();\n      fetch(this.action, { method: 'POST', body: new FormData(this), credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            showResult('success', (j.msg || '角色创建成功') + '（roleid=' + j.roleid + '）');\n            setTimeout(function () { location.reload(); }, 800);\n          } else {\n            showResult('danger', '新增失败：' + (j.msg || ('HTTP ' + r.status)) +\n              '（角色名称 2-60 字符，userid 填 0 或公司 ID）');\n          }\n        })\n        .catch(function (e) {\n          showResult('danger', '请求失败：' + e);\n        });\n    });\n\n    var editModal = new bootstrap.Modal(document.getElementById('editModal'));\n\n    document.querySelectorAll('.edit-btn').forEach(function (btn) {\n      btn.addEventListener('click', function () {\n        document.getElementById('edit-roleid').value = this.dataset.roleid;\n        document.getElementById('edit-roleid-view').value = this.dataset.roleid;\n        document.getElementById('edit-userid-view').value = this.dataset.userid;\n        document.getElementById('edit-name').value = this.dataset.name;\n        document.getElementById('edit-status').value = this.dataset.status;\n        document.getElementById('edit-rolecode').value = this.dataset.rolecode;\n        document.getElementById('edit-sortid').value = this.dataset.sortid;\n        document.getElementById('edit-rolevalue').value = this.dataset.rolevalue;\n        editModal.show();\n      });\n    });\n\n    document.getElementById('editform').addEventListener('submit', function (e) {\n      e.preventDefault();\n      fetch(this.action, { method: 'POST', body: new FormData(this), credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            showResult('success', j.msg || '角色保存成功');\n            setTimeout(function () { location.reload(); }, 800);\n          } else {\n            showResult('danger', '保存失败：' + (j.msg || ('HTTP ' + r.status)));\n          }\n        })\n        .catch(function (e) {\n          showResult('danger', '请求失败：' + e);\n        });\n    });\n\n    document.querySelectorAll('.delete-btn').forEach(function (btn) {\n      btn.addEventListener('click', function () {\n        if (!confirm('确定删除角色「' + this.dataset.name + '」？关联的用户绑定与角色授权也会被删除。')) {\n          return;\n        }\n        var fd = new FormData();\n        fd.append('csrf_token', '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"');\n        fd.append('roleid', this.dataset.roleid);\n        fetch('/superadmin/rbac/role/delete', { method: 'POST', body: fd, credentials: 'same-origin' })\n          .then(function (r) { return r.json(); })\n          .then(function (j) {\n            if (j.code === 0) {\n              showResult('success', (j.msg || '角色已删除') +\n                '（解除用户绑定 ' + j.userrole_count + ' 条，删除授权 ' + j.perms_count +\n                ' 条，角色记录 ' + j.role_count + ' 条）');\n              setTimeout(function () { location.reload(); }, 1200);\n            } else {\n              showResult('danger', '删除失败：' + (j.msg || ('HTTP ' + r.status)));\n            }\n          })\n          .catch(function (e) {\n            showResult('danger', '请求失败：' + e);\n          });\n      });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    