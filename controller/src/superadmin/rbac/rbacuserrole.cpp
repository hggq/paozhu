#include <algorithm>
#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "orm.h"
#include "rbac/rbacapi.h"
#include "rbac/rbac.h"
#include "rbac/route_cache.h"
#include "superadmin/rbac/rbac_common.h"
#include "superadmin/rbac/rbacuserrole.h"

namespace http
{

//@urlpath(superadmin_islogin,superadmin/rbac/userrole)
std::string superadmin_rbac_userrole(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->get_peer();
    unsigned int userid = (unsigned int)client.get["userid"].to_int();

    orm::cms::Sysuser user;
    user.where("adminid", userid).fetch_one();
    if (user.data.adminid == 0)
    {
        client.goto_url("/superadmin/rbac/user", 3, "User not found.");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)user.data.companyid))
    {
        client.goto_url("/superadmin/rbac/user", 3, "User not found.");
        return "";
    }
    unsigned int companyid = user.data.companyid;

    // 可选角色只有用户所属公司这一份，与 bind_user_role 的校验同口径
    // Selectable roles are the user's own company rows, same rule as bind_user_role.
    orm::cms::Sysrole roles;
    roles.where("userid", companyid).asc("userid", "roleid").fetch();

    // 当前已绑定的角色
    // Roles currently bound to this user; used to pre-check the boxes.
    orm::cms::Sysuserrole binding;
    binding.where("user_id", userid).whereAnd("company_id", companyid).fetch();
    std::vector<unsigned int> bound_ids;
    for (unsigned int i = 0; i < binding.record.size(); ++i)
    {
        bound_ids.push_back(binding.record[i].role_id);
    }

    client.val["user"].set_object();
    client.val["user"]["adminid"]   = user.data.adminid;
    client.val["user"]["name"]      = user.data.name;
    client.val["user"]["companyid"] = companyid;

    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < roles.record.size(); ++i)
    {
        auto const &r   = roles.record[i];
        temp["roleid"]  = r.roleid;
        temp["name"]    = r.name;
        temp["userid"]  = r.userid;
        temp["status"]  = (unsigned int)r.status;
        temp["checked"] = std::find(bound_ids.begin(), bound_ids.end(), r.roleid) != bound_ids.end() ? 1 : 0;
        client.val["infos"].push(temp);
    }

    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/rbacuserrole");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/userrole/save)
std::string superadmin_rbac_userrole_save(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }

    unsigned int userid = (unsigned int)client.post["userid"].to_int();
    orm::cms::Sysuser user;
    user.where("adminid", userid).fetch_one();
    if (user.data.adminid == 0)
    {
        superadmin_rbac::json_fail(client, "User not found or already deleted (invalid userid).");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)user.data.companyid))
    {
        superadmin_rbac::json_forbid(client, "User does not belong to this account.");
        return "";
    }
    // companyid 一律以库内用户数据为准，不信任前端提交
    // The tenant always comes from the stored user row; never trust a client-submitted companyid.
    unsigned int companyid = user.data.companyid;

    std::vector<unsigned int> selected = rbacapi::uint_list(client.post["roleids"].to_string());

    orm::cms::Sysuserrole binding;
    binding.where("user_id", userid).whereAnd("company_id", companyid).fetch();
    std::vector<unsigned int> current;
    for (unsigned int i = 0; i < binding.record.size(); ++i)
    {
        current.push_back(binding.record[i].role_id);
    }

    // Bind and unbind are attempted per role; failures are collected into the response while
    // successful operations are kept (no rollback).
    unsigned int added   = 0;
    unsigned int removed = 0;
    unsigned int failed  = 0;
    std::string error;
    std::string aggregated;

    for (auto roleid : selected)
    {
        if (std::find(current.begin(), current.end(), roleid) != current.end())
        {
            continue;
        }
        error.clear();
        if (rbac::bind_user_role(userid, roleid, companyid, 0, error))
        {
            ++added;
        }
        else
        {
            ++failed;
            aggregated.append(std::to_string(roleid)).append(":").append(error.empty() ? "bind failed" : error).append("; ");
        }
    }

    for (auto roleid : current)
    {
        if (std::find(selected.begin(), selected.end(), roleid) != selected.end())
        {
            continue;
        }
        error.clear();
        if (rbac::unbind_user_role(userid, roleid, companyid, error))
        {
            ++removed;
        }
        else
        {
            ++failed;
            aggregated.append(std::to_string(roleid)).append(":").append(error.empty() ? "unbind failed" : error).append("; ");
        }
    }

    // 只影响这一个用户：精确清掉他的快照即可，不必整表失效。
    // Only this user is affected: drop just his snapshot instead of the whole cache.
    if (added > 0 || removed > 0)
    {
        rbac::get_route_cache().invalidate_user(userid, companyid);
    }
    client.val["code"]    = 0;
    client.val["msg"]     = "Role bindings saved successfully.";
    client.val["added"]   = added;
    client.val["removed"] = removed;
    client.val["failed"]  = failed;
    client.val["total"]   = (unsigned int)selected.size();
    if (!aggregated.empty())
    {
        client.val["error"] = aggregated;
    }
    client.out_json();
    return "";
}

}// namespace http
