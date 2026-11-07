#include "orm.h"

#include <map>
#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "md5.h"
#include "rbac/rbacapi.h"
#include "rbac/route_cache.h"
#include "rbac/sysuser.h"

namespace http
{
namespace
{
// 本企业里本页可管理的账户：companyid 等于会话租户，且 isadmin=0
// Accounts this page may manage: companyid equals the session tenant and isadmin=0.
bool subadmin_in_company(unsigned int adminid, unsigned int companyid)
{
    orm::cms::Sysuser user;
    return user.where("adminid", adminid).whereAnd("companyid", companyid).whereAnd("isadmin", 0).fetch_one() > 0;
}
}// namespace

//@urlpath(admin_islogin,admin/rbac/sysuser)
std::string rbac_sysuser_page(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // Page endpoint: tenant admins only; unauthorized callers get a plain 403 body, not JSON.
    if (!rbacapi::company_admin(client))
    {
        client.status(403);
        client << "Only tenant administrators (sysuser.isadmin=1) may manage this company's accounts.";
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);
    unsigned int operid         = rbacapi::adminid(client);

    // Hard cap of 200 accounts; this page has no pagination.
    orm::cms::Sysuser users;
    users.where("companyid", user_companyid).asc("adminid").limit(0, 200).fetch();

    // 一次查回本企业的绑定行按账户计数，避免逐行查库
    // One pass over this company's bindings, counted per account, instead of a query per row.
    std::map<unsigned int, unsigned int> role_count;
    orm::cms::Sysuserrole binding;
    binding.select("user_id").where("company_id", user_companyid).fetch();
    for (unsigned int i = 0; i < binding.record.size(); i++)
    {
        role_count[binding.record[i].user_id]++;
    }

    unsigned int subadmin_count = 0;
    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < users.record.size(); i++)
    {
        auto const &u = users.record[i];
        if (u.isadmin == 0)
        {
            ++subadmin_count;
        }
        temp.clear();
        temp["adminid"]      = u.adminid;
        temp["name"]         = html_encode(u.name);
        temp["nickname"]     = html_encode(u.nickname);
        temp["isopen"]       = (unsigned int)u.isopen;
        temp["isadmin"]      = (unsigned int)u.isadmin;
        temp["self"]         = u.adminid == operid ? 1 : 0;
        temp["role_count"]   = role_count[u.adminid];
        temp["created_date"] = u.created_at == 0 ? "" : get_date("%Y-%m-%d %H:%M", u.created_at);
        client.val["infos"].push(temp);
    }

    client.val["companyid"]      = user_companyid;
    client.val["user_total"]     = (unsigned int)users.record.size();
    client.val["subadmin_count"] = subadmin_count;
    rbacapi::issue_csrf(client);
    peer->view("admin/rbacsysuserlist");
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/sysuser/add)
std::string rbac_sysuser_add(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    if (!rbacapi::check_csrf(client))
    {
        rbacapi::csrf_fail(client);
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    // 与登录查询同口径（admin/loginpost 只 trim 用户名）：值走 exec_* 绑定参数，不预转义 ——
    // 控制器转义一次、ORM 加引号再一次，MySQL 默认模式下反斜杠会把翻倍出来的引号吞掉半个。
    // Same spelling as the login lookup: values go to the database bound as parameters (exec_*),
    // never pre-escaped — the ORM quotes them again, and the backslash then eats half a doubled quote.
    std::string name     = str_trim(client.post["name"].to_trim());
    std::string password = client.post["password"].to_string();
    if (name.size() < 4 || name.size() > 26)
    {
        rbacapi::fail(client, "login name must be 4-26 characters");
        return "";
    }
    if (password.size() < 6 || password.size() > 20)
    {
        rbacapi::fail(client, "password must be 6-20 characters");
        return "";
    }

    // 登录只按 name 查、不带 companyid，所以登录名必须全局唯一
    // Login looks the account up by name alone, so the name must be globally unique.
    orm::cms::Sysuser exists;
    exists.where("name", name).exec_one();
    if (exists.data.adminid != 0)
    {
        rbacapi::fail(client, "login name already exists");
        return "";
    }

    orm::cms::Sysuser user;
    user.data.name       = name;
    user.data.password   = md5(password);
    user.data.nickname   = str_trim(client.post["nickname"].to_string());
    user.data.isopen     = client.post["isopen"].to_int() == 1 ? 1 : 0;
    user.data.isadmin    = 0;
    user.data.companyid  = user_companyid;
    user.data.created_at = timeid();
    auto result          = user.exec_insert();
    if (std::get<0>(result) == 0)
    {
        rbacapi::fail(client, user.error_msg.empty() ? "save user failed" : user.error_msg);
        return "";
    }

    client.val["code"]    = 0;
    client.val["msg"]     = "ok";
    client.val["adminid"] = std::get<1>(result);
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/sysuser/delete)
std::string rbac_sysuser_delete(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    if (!rbacapi::check_csrf(client))
    {
        rbacapi::csrf_fail(client);
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    unsigned int userid = (unsigned int)client.post["userid"].to_int();
    if (userid == 0 || !subadmin_in_company(userid, user_companyid))
    {
        rbacapi::fail(client, "account not found in this company, or it is the tenant super admin");
        return "";
    }

    orm::cms::Sysuserrole binding;
    binding.where("user_id", userid).whereAnd("company_id", user_companyid).remove();

    orm::cms::Sysuser user;
    user.where("adminid", userid).whereAnd("companyid", user_companyid).whereAnd("isadmin", 0).remove();

    // 账户连同它的角色绑定一起消失，只失效它自己的授权快照
    // The account and its role bindings disappear, so drop exactly its snapshot.
    rbac::get_route_cache().invalidate_user(userid, user_companyid);

    client.val["code"]   = 0;
    client.val["msg"]    = "ok";
    client.val["userid"] = userid;
    client.out_json();
    return "";
}
}// namespace http
