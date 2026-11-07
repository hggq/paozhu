#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "orm.h"
#include "rbac/rbacapi.h"
#include "rbac/rbac.h"
#include "rbac/route_cache.h"
#include "superadmin/rbac/rbac_common.h"
#include "superadmin/rbac/rbacrole.h"

namespace http
{

namespace
{

long long rolecode_from_post(httppeer &client)
{
    // An empty or non-numeric rolecode is normalized to 0 instead of failing the request.
    std::string text = str_trim(client.post["rolecode"].to_string());
    if (text.empty())
    {
        return 0;
    }
    try
    {
        return std::stoll(text);
    }
    catch (const std::exception &)
    {
        return 0;
    }
}

}// namespace

//@urlpath(superadmin_islogin,superadmin/rbac/role)
std::string superadmin_rbac_role_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    std::vector<unsigned int> sids = superadmin_rbac::agent_sids(client);
    orm::cms::Sysrole roles;
    if (!sids.empty())
    {
        roles.whereIn("userid", sids).asc("userid", "roleid").fetch();
    }

    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < roles.record.size(); ++i)
    {
        auto const &r     = roles.record[i];
        temp["roleid"]    = r.roleid;
        temp["userid"]    = r.userid;
        temp["name"]      = r.name;
        temp["status"]    = (unsigned int)r.status;
        temp["rolecode"]  = (long long)r.rolecode;
        temp["sortid"]    = r.sortid;
        temp["rolevalue"] = r.rolevalue;
        client.val["infos"].push(temp);
    }
    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/rbacrolelist");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/role/addpost)
std::string superadmin_rbac_role_addpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    std::string name = str_trim(client.post["name"].to_string());
    if (name.size() < 2 || name.size() > 60)
    {
        superadmin_rbac::json_fail(client, "Role name (name) must be 2-60 characters.");
        return "";
    }

    orm::cms::Sysrole role;
    // Fail closed: a role must belong to a real company owned by this account.
    unsigned int target_userid = (unsigned int)client.post["userid"].to_int();
    if (target_userid == 0)
    {
        superadmin_rbac::json_fail(client, "Select the company that owns this role: userid must be a real company id and cannot be 0.");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, target_userid))
    {
        superadmin_rbac::json_forbid(client, "The selected company does not belong to this account.");
        return "";
    }
    role.data.userid    = target_userid;
    role.data.name      = name;
    role.data.status    = client.post["status"].to_int() == 1 ? 1 : 0;
    role.data.rolecode  = rolecode_from_post(client);
    role.data.sortid    = client.post["sortid"].to_int();
    role.data.rolevalue = (unsigned int)client.post["rolevalue"].to_int();
    auto result         = role.save();
    if (std::get<0>(result) == 0)
    {
        superadmin_rbac::json_fail(client, role.error_msg.empty() ? "Failed to save role: the database write did not take effect." : role.error_msg);
        return "";
    }
    client.val["code"]   = 0;
    client.val["msg"]    = "Role created successfully.";
    client.val["roleid"] = std::get<1>(result);
    client.out_json();
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/role/editpost)
std::string superadmin_rbac_role_editpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    unsigned int roleid = (unsigned int)client.post["roleid"].to_int();
    orm::cms::Sysrole role;
    role.where("roleid", roleid).fetch_one();
    if (role.data.roleid == 0)
    {
        superadmin_rbac::json_fail(client, "Role not found or already deleted (invalid roleid).");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)role.data.userid))
    {
        superadmin_rbac::json_forbid(client, "Role does not belong to this account.");
        return "";
    }
    std::string name = str_trim(client.post["name"].to_string());
    if (name.size() < 2 || name.size() > 60)
    {
        superadmin_rbac::json_fail(client, "Role name (name) must be 2-60 characters.");
        return "";
    }

    orm::cms::Sysrole update;
    update.data.name      = name;
    update.data.status    = client.post["status"].to_int() == 1 ? 1 : 0;
    update.data.rolecode  = rolecode_from_post(client);
    update.data.sortid    = client.post["sortid"].to_int();
    update.data.rolevalue = (unsigned int)client.post["rolevalue"].to_int();
    update.where("roleid", roleid);
    if (update.update("name,status,rolecode,sortid,rolevalue") == 0)
    {
        superadmin_rbac::json_fail(client, "Save failed: no field values changed, or the database write failed.");
        return "";
    }
    // 角色 status 决定其授权是否生效，改完立刻让运行期缓存重载。
    // A role's status decides whether its grants are effective; invalidate the runtime snapshots
    // immediately after the update.
    rbac::get_route_cache().invalidate_all();
    superadmin_rbac::json_ok(client, "Role saved successfully.");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/role/delete)
std::string superadmin_rbac_role_delete(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    unsigned int roleid = (unsigned int)client.post["roleid"].to_int();
    orm::cms::Sysrole role;
    role.where("roleid", roleid).fetch_one();
    if (role.data.roleid == 0)
    {
        superadmin_rbac::json_fail(client, "Role not found or already deleted (invalid roleid).");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)role.data.userid))
    {
        superadmin_rbac::json_forbid(client, "Role does not belong to this account.");
        return "";
    }
    unsigned int companyid = role.data.userid;

    // Delete order: bindings, grants, then the role itself. There is no transaction, so a partial
    // failure can leave the role partially cleaned up.
    orm::cms::Sysuserrole userrole;
    userrole.where("role_id", roleid).whereAnd("company_id", companyid);
    int userrole_count = userrole.remove();

    orm::cms::Sysroleperms roleperms;
    roleperms.where("roleid", roleid).whereAnd("userid", companyid);
    int perms_count = roleperms.remove();

    orm::cms::Sysrole remove_role;
    remove_role.where("roleid", roleid);
    int role_count = remove_role.remove();

    // 删角色同时清掉 sysuserrole / sysroleperms，受影响用户的快照全部作废。
    // Deleting a role also removes its sysuserrole and sysroleperms rows, so every affected
    // user's snapshot is dropped.
    rbac::get_route_cache().invalidate_all();
    client.val["code"]           = 0;
    client.val["msg"]            = "Role deleted successfully.";
    client.val["userrole_count"] = userrole_count;
    client.val["perms_count"]    = perms_count;
    client.val["role_count"]     = role_count;
    client.out_json();
    return "";
}

//@urlpath(superadmin_islogin,superadmin/rbac/roleperm)
std::string superadmin_rbac_roleperm(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->get_peer();
    unsigned int roleid = (unsigned int)client.get["roleid"].to_int();
    orm::cms::Sysrole role;
    role.where("roleid", roleid).fetch_one();
    if (role.data.roleid == 0)
    {
        client.goto_url("/superadmin/rbac/role", 3, "Role not found.");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)role.data.userid))
    {
        client.goto_url("/superadmin/rbac/role", 3, "Role not found.");
        return "";
    }
    unsigned int companyid = role.data.userid;

    // 可分配集合 = 平台已分配给该企业（syscompanyperms）且目录行启用中；
    // 与 replace_role_perms 的校验同口径，页面列出的行写入必不被拒。
    // Assignable set = allocated to this company (syscompanyperms) and still enabled in the
    // catalog. The same rule is enforced by replace_role_perms, so everything listed here is
    // guaranteed to be accepted on save.
    orm::cms::Sysperms perms;
    std::vector<unsigned int> alloc;
    rbac::load_company_perms(companyid, alloc);
    if (!alloc.empty())
    {
        perms.whereIn(orm::cms::sysperms_info::cols::permsid, alloc)
            .whereAnd("userid", companyid)
            .whereAnd("status", 1)
            .asc("parentid", "permsid")
            .fetch();
    }

    orm::cms::Sysroleperms granted;
    granted.select("permsid").where("roleid", roleid).whereAnd("userid", companyid).fetch();
    std::vector<unsigned int> granted_ids;
    for (unsigned int i = 0; i < granted.record.size(); ++i)
    {
        granted_ids.push_back(granted.record[i].permsid);
    }

    client.val["role"].set_object();
    client.val["role"]["roleid"] = role.data.roleid;
    client.val["role"]["userid"] = companyid;
    client.val["role"]["name"]   = role.data.name;

    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < perms.record.size(); ++i)
    {
        auto const &p     = perms.record[i];
        temp["permsid"]   = p.permsid;
        temp["userid"]    = p.userid;
        temp["parentid"]  = p.parentid;
        temp["urlpath"]   = p.urlpath;
        temp["name"]      = p.name;
        temp["permscode"] = p.permscode;
        temp["maskcode"]  = p.maskcode;
        temp["checked"]   = 0;
        for (unsigned int k = 0; k < granted_ids.size(); ++k)
        {
            if (granted_ids[k] == p.permsid)
            {
                temp["checked"] = 1;
                break;
            }
        }
        client.val["infos"].push(temp);
    }
    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/rbacroleperm");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/roleperm/save)
std::string superadmin_rbac_roleperm_save(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    unsigned int roleid = (unsigned int)client.post["roleid"].to_int();
    orm::cms::Sysrole role;
    role.where("roleid", roleid).fetch_one();
    if (role.data.roleid == 0)
    {
        superadmin_rbac::json_fail(client, "Role not found or already deleted (invalid roleid).");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)role.data.userid))
    {
        superadmin_rbac::json_forbid(client, "Role does not belong to this account.");
        return "";
    }

    std::vector<unsigned int> permsids = rbacapi::uint_list(client.post["permsids"].to_string());
    std::string error;
    auto replace_result = rbac::replace_role_perms(roleid, role.data.userid, permsids, 0, error);
    if (replace_result == rbac::replace_role_perms_result::rejected)
    {
        superadmin_rbac::json_fail(client, error.empty() ? "Role permissions were not saved: the permission set was rejected." : error);
        return "";
    }
    if (replace_result == rbac::replace_role_perms_result::rolled_back)
    {
        superadmin_rbac::json_fail(client, error.empty() ? "Role permissions were not saved; the whole set was rolled back." : error);
        return "";
    }
    // 角色与权限的绑定变了，所有用户的 urlpath 集合都可能变，整表失效。
    // Role-permission bindings changed, so every holder's urlpath set may change; drop the
    // whole cache.
    rbac::get_route_cache().invalidate_all();
    client.val["code"]  = 0;
    client.val["msg"]   = "Role permissions saved successfully.";
    client.val["count"] = (unsigned int)permsids.size();
    client.out_json();
    return "";
}

}// namespace http
