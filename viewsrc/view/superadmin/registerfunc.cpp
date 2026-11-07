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
 		 std::string registerfunc([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>注册函数列表</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">注册函数列表</h5>\n    <div class=\"text-muted mb-2\">\n      当前站点：";
echo<<obj["current_site"].to_string(); 
 			 echo<<"；\n      命名空间内路由：";
echo<<obj["total"].to_string(); 
 			 echo<<" 条；\n      可登记（maskcode 非 0）：";
echo<<obj["candidate_count"].to_string(); 
 			 echo<<" 条；\n      本企业已登记：";
echo<<obj["registered_count"].to_string(); 
 			 echo<<"；\n      掩码漂移：";
echo<<obj["drift_count"].to_string(); 
 			 echo<<"；\n      当前列表：";
echo<<obj["list_total"].to_string(); 
 			 echo<<" 条\n    </div>\n    <div class=\"alert alert-info py-2\">\n      列表默认展示「全局命名空间 + 当前域名命名空间」内全部路由（<code>maskcode=0</code> 的行只读、不可勾选），\n      每页 100 条；点「只看可登记」可只保留带 <code>maskcode</code> 的候选。翻页会重新载入列表，已勾选状态不跨页保留。<br>\n      同 <code>urlpath</code>\n      已存在且 <code>maskcode</code> 一致则跳过，不一致则以路由注册值改写（库里的值漂移了，运行期判定会一直 403）。<br>\n      登记后写入 <code>sysperms</code>：userid=<b>所选企业的 sid</b>（即 <code>sysuser.companyid</code>）、urlpath=路由路径、\n      urlname=路由路径、name=注解 <code>:title(名称)</code> 里的名称（没写 title 时=name=urlpath）、status=1、maskcode=路由掩码；同批再写一条 <code>syscompanyperms</code>（登记即分配，\n      该企业立刻可以把这条权限授给角色）。权限按企业隔离，租户管理员只能分配平台已分配给本企业的项。\n      登记只是生成权限项，是否放行还要到角色页给角色授权。\n    </div>\n    ";
 if(obj["company_count"].to_int()==0){ 
 			 echo<<"    <div class=\"alert alert-warning py-2\">当前账号名下还没有站点，无法按企业登记权限；请先在网站管理里新增站点。</div>\n    ";
 } 
 			 echo<<"\n    <div class=\"row mb-2 g-2 align-items-end\">\n      <div class=\"col-md-4\">\n        <label class=\"form-label mb-1\" for=\"companyid\">登记给企业（sysperms.userid）</label>\n        <select id=\"companyid\" class=\"form-select\">\n          ";
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
 			 echo<<"        </select>\n        <div class=\"form-text\">切换企业会重新载入该企业的登记状态。</div>\n      </div>\n      <div class=\"col-md-4\">\n        <label class=\"form-label mb-1\" for=\"filter\">筛选路由</label>\n        <input id=\"filter\" class=\"form-control\" placeholder=\"输入 urlpath 或站点名，例如：/orders 或 saas.com\">\n        <div class=\"form-text\">仅在前端过滤当前表格，不会修改数据。</div>\n      </div>\n      <div class=\"col-md-4 text-end\">\n        ";
 if(obj["candidate_only"].to_int()==1){ 
 			 echo<<"        <a class=\"btn btn-outline-secondary btn-sm\" href=\"";
 echo<<obj["url_candidate"].to_string(); 
 			 echo<<"\">显示全部路由</a>\n        ";
 }else{ 
 			 echo<<"        <a class=\"btn btn-outline-secondary btn-sm\" href=\"";
 echo<<obj["url_candidate"].to_string(); 
 			 echo<<"\">只看可登记</a>\n        ";
 } 
 			 echo<<"        ";
 if(obj["showall"].to_int()==1){ 
 			 echo<<"        <a class=\"btn btn-outline-secondary btn-sm\" href=\"";
 echo<<obj["url_showall"].to_string(); 
 			 echo<<"\">只看本站</a>\n        ";
 }else{ 
 			 echo<<"        <a class=\"btn btn-outline-secondary btn-sm\" href=\"";
 echo<<obj["url_showall"].to_string(); 
 			 echo<<"\">全部站点（只读）</a>\n        ";
 } 
 			 echo<<"        <button class=\"btn btn-primary btn-sm\" id=\"savebtn\">登记选中项</button>\n      </div>\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <table class=\"table table-sm table-bordered align-middle\" id=\"routeTable\">\n      <thead class=\"table-light\">\n        <tr>\n          <th style=\"width:70px\">\n            <input type=\"checkbox\" id=\"checkall\" class=\"form-check-input me-1\" title=\"全选当前列表\">选择\n            <br><small class=\"text-muted fw-normal\">勾选=登记</small>\n          </th>\n          <th style=\"width:150px\">站点<br><small class=\"text-muted fw-normal\">site</small></th>\n          <th>路由路径<br><small class=\"text-muted fw-normal\">urlpath</small></th>\n          <th style=\"width:150px\">名称<br><small class=\"text-muted fw-normal\">title</small></th>\n          <th style=\"width:100px\">掩码<br><small class=\"text-muted fw-normal\">mask_code</small></th>\n          <th style=\"width:110px\">限流（秒/次）<br><small class=\"text-muted fw-normal\">rate_time / rate_num</small></th>\n          <th style=\"width:90px\">前置校验<br><small class=\"text-muted fw-normal\">pre_kind</small></th>\n          <th style=\"width:90px\">注册类型<br><small class=\"text-muted fw-normal\">reg_kind</small></th>\n          <th style=\"width:130px\">状态<br><small class=\"text-muted fw-normal\">registered / drift</small></th>\n        </tr>\n      </thead>\n      <tbody>\n        ";
 for(auto &a:obj["infos"].as_array()){ 
 			 echo<<"        <tr data-text=\"";
 echo<<a["site"].to_string(); 
 			 echo<<" ";
 echo<<a["urlpath"].to_string(); 
 			 echo<<"\">\n          <td>\n            ";
 if(a["candidate"].to_int()==1 && (a["registered"].to_int()==0 || a["drift"].to_int()==1)){ 
 			 echo<<"            <input class=\"form-check-input rowcheck\" type=\"checkbox\" value=\"";
 echo<<a["urlpath"].to_string(); 
 			 echo<<"\">\n            ";
 } 
 			 echo<<"          </td>\n          <td>\n            ";
 echo<<a["site"].to_string(); 
 			 echo<<"            ";
 if(a["site"].to_string()=="*"){ 
 			 echo<<"<span class=\"badge bg-info text-dark\">全局</span>";
 } 
 			 echo<<"          </td>\n          <td>";
 echo<<a["urlpath"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["title"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["mask_code"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["rate_time"].to_string(); 
 			 echo<<" 秒 / ";
 echo<<a["rate_num"].to_string(); 
 			 echo<<" 次</td>\n          <td>";
 echo<<a["pre_kind"].to_string(); 
 			 echo<<"</td>\n          <td>";
 echo<<a["reg_kind"].to_string(); 
 			 echo<<"</td>\n          <td>\n            ";
 if(a["candidate"].to_int()==0){ 
 			 echo<<"            <span class=\"badge bg-secondary\">mask=0，不参与</span>\n            ";
 }else if(a["drift"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-danger\">掩码漂移</span><br><small class=\"text-muted\">库 ";
 echo<<a["stored_mask"].to_string(); 
 			 echo<<" / 路由 ";
 echo<<a["mask_code"].to_string(); 
 			 echo<<"</small>\n            ";
 }else if(a["registered"].to_int()==1){ 
 			 echo<<"            <span class=\"badge bg-success\">已登记</span>\n            ";
 }else{ 
 			 echo<<"            <span class=\"badge bg-warning text-dark\">可登记</span>\n            ";
 } 
 			 echo<<"          </td>\n        </tr>\n        ";
 } 
 			 echo<<"      </tbody>\n    </table>\n\n    <div class=\"d-flex flex-wrap align-items-center justify-content-between gap-2 mb-2\">\n      <div class=\"text-muted small\">\n        第 <b>";
 echo<<obj["page"].to_string(); 
 			 echo<<"</b> / ";
 echo<<obj["total_pages"].to_string(); 
 			 echo<<" 页，\n        共 <b>";
 echo<<obj["list_total"].to_string(); 
 			 echo<<"</b> 条；\n        本页显示第 ";
 echo<<obj["page_start"].to_string(); 
 			 echo<<"–";
 echo<<obj["page_end"].to_string(); 
 			 echo<<" 条（每页 ";
 echo<<obj["page_size"].to_string(); 
 			 echo<<" 条）\n      </div>\n      <nav class=\"btn-group btn-group-sm\" aria-label=\"分页\">\n        ";
 if(obj["url_prev"].to_string().empty()){ 
 			 echo<<"        <button class=\"btn btn-outline-secondary\" disabled>上一页</button>\n        ";
 }else{ 
 			 echo<<"        <a class=\"btn btn-outline-secondary\" href=\"";
 echo<<obj["url_prev"].to_string(); 
 			 echo<<"\">上一页</a>\n        ";
 } 
 			 echo<<"        ";
 if(obj["url_next"].to_string().empty()){ 
 			 echo<<"        <button class=\"btn btn-outline-secondary\" disabled>下一页</button>\n        ";
 }else{ 
 			 echo<<"        <a class=\"btn btn-outline-secondary\" href=\"";
 echo<<obj["url_next"].to_string(); 
 			 echo<<"\">下一页</a>\n        ";
 } 
 			 echo<<"      </nav>\n    </div>\n\n    <div class=\"text-muted small mb-2\">\n      字段说明：<code>mask_code</code>=路由注册时声明的掩码，非 0 才参与 RBAC（0 的行只展示、不可登记）；\n      <code>title</code>=注解 <code>:title(名称)</code> 里的名称，登记时写进 <code>sysperms.name</code>（空则用 urlpath）；\n      <code>rate_time</code>=限流时间窗口（秒），<code>rate_num</code>=窗口内允许请求次数；\n      <code>pre_kind</code>/<code>reg_kind</code>：0=无，1=sync，2=coro；\n      <code>site</code> 为 <code>*</code> 表示全局命名空间。\n    </div>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    document.getElementById('filter').addEventListener('input', function () {\n      var q = this.value.trim().toLowerCase();\n      document.querySelectorAll('#routeTable tbody tr').forEach(function (tr) {\n        tr.style.display = q === '' || tr.dataset.text.toLowerCase().indexOf(q) >= 0 ? '' : 'none';\n      });\n    });\n    document.getElementById('checkall') && document.getElementById('checkall').addEventListener('change', function () {\n      document.querySelectorAll('.rowcheck').forEach(function (cb) { cb.checked = this.checked; }, this);\n    });\n    document.getElementById('companyid').addEventListener('change', function () {\n      var tail = ";
 if(obj["showall"].to_int()==1){ 
 			 echo<<"'&showall=1'";
 }else{ 
 			 echo<<"''";
 } 
 			 echo<<";\n      var tail2 = ";
 if(obj["candidate_only"].to_int()==1){ 
 			 echo<<"'&candidate=1'";
 }else{ 
 			 echo<<"''";
 } 
 			 echo<<";\n      location.href = '/superadmin/registerfunc?companyid=' + this.value + tail + tail2;\n    });\n    document.getElementById('savebtn').addEventListener('click', function () {\n      var picked = [];\n      document.querySelectorAll('.rowcheck:checked').forEach(function (cb) { picked.push(cb.value); });\n      var box = document.getElementById('result');\n      if (picked.length === 0) {\n        box.innerHTML = '<div class=\"alert alert-warning py-2 mb-0\">请先勾选要登记的 urlpath（只有「可登记」和「掩码漂移」状态的行可勾选）</div>';\n        return;\n      }\n      var fd = new FormData();\n      fd.append('csrf_token', '";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"');\n      fd.append('companyid', '";
echo<<obj["companyid"].to_string(); 
 			 echo<<"');\n      fd.append('urlpaths', picked.join('\\n'));\n      fetch('/superadmin/registerfunc/save', { method: 'POST', body: fd, credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            box.innerHTML = '<div class=\"alert alert-success py-2 mb-0\">' +\n              (j.msg || '登记完成') + '（企业 sid ' + j.companyid + '）：提交 ' + j.total + ' 项，新增 ' + j.inserted +\n              '，改写掩码 ' + j.synced + '，跳过 ' + j.skipped + '（已登记且掩码一致），拒绝 ' + j.rejected +\n              '（不在当前站点候选范围或写入失败）</div>';\n            setTimeout(function () { location.reload(); }, 900);\n          } else {\n            box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">登记失败：' +\n              (j.msg || ('HTTP ' + r.status)) + '</div>';\n          }\n        })\n        .catch(function (e) {\n          box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">请求失败：' + e + '</div>';\n        });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    