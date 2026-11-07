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
 		 std::string rbacuserform([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"zh-CN\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <title>用户表单</title>\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n</head>\n\n<body>\n  <div class=\"container-fluid p-3\">\n    <h5 class=\"mt-2\">\n      ";
 if(obj["action"].to_string()=="add"){ 
 			 echo<<"新增用户";
 }else{ 
 			 echo<<"编辑用户";
 } 
 			 echo<<"      <small class=\"text-muted fs-6\">数据表 sysuser</small>\n    </h5>\n    <div class=\"alert alert-info py-2\">\n      带 <span class=\"text-danger\">*</span> 为必填；标注「自动」的字段由系统生成，无需填写；标注「留空」的字段可保持默认值。\n    </div>\n\n    <div id=\"result\" class=\"mb-2\"></div>\n\n    <form id=\"form\" action=\"/superadmin/rbac/user/";
echo<<obj["action"].to_string(); 
 			 echo<<"post\" method=\"post\">\n      <input type=\"hidden\" name=\"csrf_token\" value=\"";
echo<<obj["csrf_token"].to_string(); 
 			 echo<<"\">\n      ";
 if(obj["action"].to_string()=="edit"){ 
 			 echo<<"      <input type=\"hidden\" name=\"userid\" value=\"";
echo<<obj["info"]["adminid"].to_string(); 
 			 echo<<"\">\n      ";
 } 
 			 echo<<"\n      <div class=\"card mb-3\">\n        <div class=\"card-header py-2\">账号信息</div>\n        <div class=\"card-body\">\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-adminid\">\n              用户ID <code>adminid</code> <span class=\"text-muted\">自动</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input class=\"form-control\" id=\"f-adminid\" readonly disabled\n                value=\"";
 if(obj["action"].to_string()=="add"){ 
 			 echo<<"保存后自动生成";
 }else{ 
 			 echo<<"";
echo<<obj["info"]["adminid"].to_string(); 
 			 echo<<"";
 } 
 			 echo<<"\">\n              <div class=\"form-text\">自增主键，不可修改；用户角色绑定按该 ID 关联。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-name\">\n              登录名 <code>name</code> <span class=\"text-danger\">*必填</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-name\" name=\"name\" required minlength=\"4\" maxlength=\"26\"\n                placeholder=\"例如：order_admin\" value=\"";
echo<<obj["info"]["name"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">4-26 个字符，登录使用，全局唯一，例如「order_admin」。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-password\">\n              密码 <code>password</code>\n              ";
 if(obj["action"].to_string()=="add"){ 
 			 echo<<"<span class=\"text-danger\">*必填</span>";
 }else{ 
 			 echo<<"<span class=\"text-muted\">留空不改</span>";
 } 
 			 echo<<"            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"password\" class=\"form-control\" id=\"f-password\" name=\"password\" minlength=\"6\" maxlength=\"20\"\n                placeholder=\"";
 if(obj["action"].to_string()=="add"){ 
 			 echo<<"6-20 位，例如 Admin@123";
 }else{ 
 			 echo<<"留空表示不修改密码";
 } 
 			 echo<<"\"\n                ";
 if(obj["action"].to_string()=="add"){ 
 			 echo<<"required";
 } 
 			 echo<<">\n              <div class=\"form-text\">\n                6-20 位；保存时自动 MD5 写入 <code>password</code>，并同步明文到 <code>textword</code>（兼容旧逻辑）。\n                ";
 if(obj["action"].to_string()=="edit"){ 
 			 echo<<"编辑时留空即保留原密码。";
 } 
 			 echo<<"              </div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-isopen\">\n              启用状态 <code>isopen</code> <span class=\"text-danger\">*必填</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <select class=\"form-select\" id=\"f-isopen\" name=\"isopen\">\n                <option value=\"1\" ";
 if(obj["action"].to_string()=="add" || obj["info"]["isopen"].to_int()==1){ 
 			 echo<<"selected";
 } 
 			 echo<<">启用（1）：允许登录</option>\n                <option value=\"0\" ";
 if(obj["action"].to_string()!="add" && obj["info"]["isopen"].to_int()!=1){ 
 			 echo<<"selected";
 } 
 			 echo<<">关闭（0）：禁止登录</option>\n              </select>\n              <div class=\"form-text\">1=启用，可登录；0=关闭，账号保留但不能登录。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-isadmin\">\n              租户管理员 <code>isadmin</code> <span class=\"text-danger\">*必填</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <select class=\"form-select\" id=\"f-isadmin\" name=\"isadmin\">\n                <option value=\"0\" ";
 if(obj["info"]["isadmin"].to_int()==0){ 
 			 echo<<"selected";
 } 
 			 echo<<">否（0）：普通用户，仅被分配权限</option>\n                <option value=\"1\" ";
 if(obj["info"]["isadmin"].to_int()==1){ 
 			 echo<<"selected";
 } 
 			 echo<<">是（1）：租户管理员，可管理本公司角色与授权</option>\n              </select>\n              <div class=\"form-text\">\n                1=可在租户后台维护本公司的 <code>sysrole</code> / <code>sysroleperms</code>；\n                无论取值如何，都不能修改全局 <code>sysperms</code> 与 <code>sysuserrole</code>。\n              </div>\n            </div>\n          </div>\n        </div>\n      </div>\n\n      <div class=\"card mb-3\">\n        <div class=\"card-header py-2\">组织与归属</div>\n        <div class=\"card-body\">\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-companyid\">\n              公司ID <code>companyid</code> <span class=\"text-danger\">*必填</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"number\" class=\"form-control\" id=\"f-companyid\" name=\"companyid\" min=\"0\"\n                placeholder=\"例如：1；0 表示平台级\" value=\"";
echo<<obj["info"]["companyid"].to_int(); 
 			 echo<<"\">\n              <div class=\"form-text\">所属企业/租户 ID，决定该用户能看到哪些本公司角色；0 表示平台级。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-level\">\n              等级 <code>level</code> <span class=\"text-muted\">留空=0</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"number\" class=\"form-control\" id=\"f-level\" name=\"level\" placeholder=\"0\"\n                value=\"";
echo<<obj["info"]["level"].to_int(); 
 			 echo<<"\">\n              <div class=\"form-text\">业务自定义等级，整数，默认 0。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-roleid\">\n              旧版角色ID <code>roleid</code> <span class=\"text-muted\">留空=0</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"number\" class=\"form-control\" id=\"f-roleid\" name=\"roleid\" min=\"0\" placeholder=\"0\"\n                value=\"";
echo<<obj["info"]["roleid"].to_int(); 
 			 echo<<"\">\n              <div class=\"form-text\">\n                sysuser 的单角色兼容列，0=未设置。多角色授权请使用用户列表的「角色」按钮维护\n                <code>sysuserrole</code>。\n              </div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-dpid\">\n              部门ID <code>dpid</code> <span class=\"text-muted\">留空=0</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"number\" class=\"form-control\" id=\"f-dpid\" name=\"dpid\" min=\"0\" placeholder=\"0\"\n                value=\"";
echo<<obj["info"]["dpid"].to_int(); 
 			 echo<<"\">\n              <div class=\"form-text\">所属部门编号，0=未设置。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-jobid\">\n              岗位ID <code>jobid</code> <span class=\"text-muted\">留空=0</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"number\" class=\"form-control\" id=\"f-jobid\" name=\"jobid\" min=\"0\" placeholder=\"0\"\n                value=\"";
echo<<obj["info"]["jobid"].to_int(); 
 			 echo<<"\">\n              <div class=\"form-text\">所属岗位编号，0=未设置。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-postid\">\n              职位ID <code>postid</code> <span class=\"text-muted\">留空=0</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"number\" class=\"form-control\" id=\"f-postid\" name=\"postid\" min=\"0\" placeholder=\"0\"\n                value=\"";
echo<<obj["info"]["postid"].to_int(); 
 			 echo<<"\">\n              <div class=\"form-text\">所属职位编号，0=未设置。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-expectdate\">\n              到期日期 <code>enddate</code> <span class=\"text-muted\">留空=今天</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"date\" class=\"form-control\" id=\"f-expectdate\" name=\"expectdate\"\n                value=\"";
echo<<obj["info"]["expectdate"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">账号有效期截止日；留空或格式不正确时自动取当天，可在用户列表查看。</div>\n            </div>\n          </div>\n        </div>\n      </div>\n\n      <div class=\"card mb-3\">\n        <div class=\"card-header py-2\">联系方式与资料</div>\n        <div class=\"card-body\">\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-nickname\">昵称 <code>nickname</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-nickname\" name=\"nickname\" maxlength=\"60\"\n                placeholder=\"例如：订单管理员\" value=\"";
echo<<obj["info"]["nickname"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">界面显示名，可留空。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-realname\">真实姓名 <code>realname</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-realname\" name=\"realname\" maxlength=\"40\"\n                placeholder=\"例如：张三\" value=\"";
echo<<obj["info"]["realname"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">可留空。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-gender\">性别 <code>gender</code></label>\n            <div class=\"col-sm-6\">\n              <select class=\"form-select\" id=\"f-gender\" name=\"gender\">\n                <option value=\"0\" ";
 if(obj["info"]["gender"].to_int()==0){ 
 			 echo<<"selected";
 } 
 			 echo<<">未知（0）</option>\n                <option value=\"1\" ";
 if(obj["info"]["gender"].to_int()==1){ 
 			 echo<<"selected";
 } 
 			 echo<<">男（1）</option>\n                <option value=\"2\" ";
 if(obj["info"]["gender"].to_int()==2){ 
 			 echo<<"selected";
 } 
 			 echo<<">女（2）</option>\n              </select>\n              <div class=\"form-text\">0=未知，1=男，2=女。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-mobile\">手机号 <code>mobile</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-mobile\" name=\"mobile\" maxlength=\"40\"\n                placeholder=\"例如：13800138000\" value=\"";
echo<<obj["info"]["mobile"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">可留空。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-email\">电子邮箱 <code>email</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-email\" name=\"email\" maxlength=\"60\"\n                placeholder=\"例如：order@example.com\" value=\"";
echo<<obj["info"]["email"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">可留空。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-avatar\">头像 <code>avatar</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-avatar\" name=\"avatar\" maxlength=\"256\"\n                placeholder=\"头像图片 URL，例如 /assets/images/avatar.png\"\n                value=\"";
echo<<obj["info"]["avatar"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">填写图片地址，可留空。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-wxuuid\">微信UUID <code>wxuuid</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-wxuuid\" name=\"wxuuid\" maxlength=\"40\"\n                placeholder=\"微信绑定的 uuid，可留空\" value=\"";
echo<<obj["info"]["wxuuid"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">用于微信侧绑定，可留空。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-expname\">紧急联系人 <code>expname</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-expname\" name=\"expname\" maxlength=\"60\"\n                placeholder=\"例如：李四\" value=\"";
echo<<obj["info"]["expname"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">可留空。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-expmobile\">紧急联系电话 <code>expmobile</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-expmobile\" name=\"expmobile\" maxlength=\"60\"\n                placeholder=\"例如：13900139000\" value=\"";
echo<<obj["info"]["expmobile"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">可留空。</div>\n            </div>\n          </div>\n\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-expaddress\">紧急联系地址 <code>expaddress</code></label>\n            <div class=\"col-sm-6\">\n              <input type=\"text\" class=\"form-control\" id=\"f-expaddress\" name=\"expaddress\" maxlength=\"160\"\n                placeholder=\"例如：上海市浦东新区示例路 1 号\" value=\"";
echo<<obj["info"]["expaddress"].to_string(); 
 			 echo<<"\">\n              <div class=\"form-text\">可留空。</div>\n            </div>\n          </div>\n        </div>\n      </div>\n\n      <div class=\"card mb-3\">\n        <div class=\"card-header py-2\">其他</div>\n        <div class=\"card-body\">\n          <div class=\"row mb-3\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\" for=\"f-qrtemp\">\n              二维码模板 <code>qrtemp</code> <span class=\"text-muted\">留空=0</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input type=\"number\" class=\"form-control\" id=\"f-qrtemp\" name=\"qrtemp\" min=\"0\" placeholder=\"0\"\n                value=\"";
echo<<obj["info"]["qrtemp"].to_int(); 
 			 echo<<"\">\n              <div class=\"form-text\">二维码模板编号，0=默认模板。</div>\n            </div>\n          </div>\n\n          <div class=\"row\">\n            <label class=\"col-sm-3 col-form-label text-sm-end\">\n              创建时间 <code>created_at</code> <span class=\"text-muted\">自动</span>\n            </label>\n            <div class=\"col-sm-6\">\n              <input class=\"form-control\" readonly disabled value=\"保存时由系统写入，不可修改\">\n              <div class=\"form-text\">新增时自动记录当前时间。</div>\n            </div>\n          </div>\n        </div>\n      </div>\n\n      <div class=\"row mb-3\">\n        <div class=\"col-sm-6 offset-sm-3\">\n          <button type=\"submit\" class=\"btn btn-primary\">\n            ";
 if(obj["action"].to_string()=="add"){ 
 			 echo<<"添加";
 }else{ 
 			 echo<<"保存";
 } 
 			 echo<<"          </button>\n          <a href=\"/superadmin/rbac/user\" class=\"btn btn-link\">返回</a>\n        </div>\n      </div>\n    </form>\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n  <script>\n    document.getElementById('form').addEventListener('submit', function (e) {\n      e.preventDefault();\n      var box = document.getElementById('result');\n      var name = document.getElementById('f-name').value;\n      fetch(this.action, { method: 'POST', body: new FormData(this), credentials: 'same-origin' })\n        .then(function (r) { return r.json(); })\n        .then(function (j) {\n          if (j.code === 0) {\n            box.innerHTML = '<div class=\"alert alert-success py-2 mb-0\">' +\n              (j.msg || '保存成功') + '（登录名：' + name + '）</div>';\n            setTimeout(function () { location.href = '/superadmin/rbac/user'; }, 800);\n          } else {\n            box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">保存失败：' +\n              (j.msg || ('HTTP ' + r.status)) + '</div>';\n          }\n        })\n        .catch(function (e) {\n          box.innerHTML = '<div class=\"alert alert-danger py-2 mb-0\">请求失败：' + e + '</div>';\n        });\n    });\n  </script>\n</body>\n\n</html>\n";

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
    