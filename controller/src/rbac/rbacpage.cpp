#include "orm.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "rbac/rbacapi.h"
#include "rbac/rbac.h"
#include "rbac/rbacpage.h"

namespace http
{
//@urlpath(admin_islogin,admin/rbac/role)
std::string rbac_page_role(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        client.status(403);
        client << "Only tenant administrators (sysuser.isadmin=1) may access permission management.";
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    orm::cms::Sysrole role;
    role.where("userid", user_companyid).asc("sortid").fetch();

    // 一次查回本公司的授权行与绑定行，按角色分组计数，避免逐行查库
    // One pass over this company's grants and bindings, counted per role, instead of a query per row.
    std::map<unsigned int, unsigned int> perm_count;
    orm::cms::Sysroleperms roleperms;
    roleperms.select("roleid").where("userid", user_companyid).fetch();
    for (unsigned int i = 0; i < roleperms.record.size(); i++)
    {
        perm_count[roleperms.record[i].roleid]++;
    }
    std::map<unsigned int, unsigned int> user_count;
    orm::cms::Sysuserrole binding;
    binding.select("role_id").where("company_id", user_companyid).fetch();
    for (unsigned int i = 0; i < binding.record.size(); i++)
    {
        user_count[binding.record[i].role_id]++;
    }

    unsigned int assignable_total = 0;
    // 可分配集口径 = 平台分配给本企业（syscompanyperms）且目录行启用；与 roleperm 页、写入校验同源
    // Assignable set = allocated to this company and still enabled; same rule as the roleperm page.
    std::vector<unsigned int> alloc;
    rbac::load_company_perms(user_companyid, alloc);
    if (!alloc.empty())
    {
        orm::cms::Sysperms assignable;
        assignable.select("permsid")
            .whereIn(orm::cms::sysperms_info::cols::permsid, alloc)
            .whereAnd("userid", user_companyid)
            .whereAnd("status", 1)
            .fetch();
        assignable_total = (unsigned int)assignable.record.size();
    }

    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < role.record.size(); i++)
    {
        temp.clear();
        temp["roleid"]     = role.record[i].roleid;
        temp["name"]       = role.record[i].name;
        temp["status"]     = (unsigned int)role.record[i].status;
        temp["rolecode"]   = role.record[i].rolecode;
        temp["sortid"]     = role.record[i].sortid;
        temp["perm_count"] = perm_count[role.record[i].roleid];
        temp["user_count"] = user_count[role.record[i].roleid];
        client.val["infos"].push(temp);
    }

    client.val["companyid"]        = user_companyid;
    client.val["role_count"]       = (unsigned int)role.record.size();
    client.val["assignable_total"] = assignable_total;
    rbacapi::issue_csrf(client);
    peer->view("admin/rbacrolelist");
    return "";
}

//@urlpath(admin_islogin,admin/rbac/roleperm)
std::string rbac_page_roleperm(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        client.status(403);
        client << "Only tenant administrators (sysuser.isadmin=1) may access permission management.";
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    orm::cms::Sysrole role;
    role.where("userid", user_companyid).asc("sortid").fetch();

    unsigned int roleid = (unsigned int)client.get["roleid"].to_int();
    bool role_ok        = false;
    for (unsigned int i = 0; i < role.record.size(); i++)
    {
        if (role.record[i].roleid == roleid)
        {
            role_ok = true;
            break;
        }
    }
    if (!role_ok)
    {
        // 缺省或非法 roleid 回落到第一个角色；没有角色时保持 0，页面显示空状态
        // Missing or foreign roleid falls back to the first role; 0 when the company has none.
        roleid  = role.record.empty() ? 0 : role.record[0].roleid;
        role_ok = roleid != 0;
    }

    // 可分配集 = 平台分配给本企业且启用的权限项；未分配的条目租户看不到也勾不到
    // Assignable set = rows the platform allocated to this company, enabled.
    orm::cms::Sysperms perms;
    std::vector<unsigned int> alloc;
    rbac::load_company_perms(user_companyid, alloc);
    if (!alloc.empty())
    {
        perms.whereIn(orm::cms::sysperms_info::cols::permsid, alloc)
            .whereAnd("userid", user_companyid)
            .whereAnd("status", 1)
            .asc("parentid", "permsid")
            .fetch();
    }

    std::vector<unsigned int> granted_ids;
    if (roleid != 0)
    {
        orm::cms::Sysroleperms granted;
        granted.select("permsid").where("roleid", roleid).whereAnd("userid", user_companyid).fetch();
        for (unsigned int i = 0; i < granted.record.size(); i++)
        {
            granted_ids.push_back(granted.record[i].permsid);
        }
    }

    client.val["roles"].set_array();
    obj_val onerole;
    for (unsigned int i = 0; i < role.record.size(); i++)
    {
        onerole.clear();
        onerole["roleid"] = role.record[i].roleid;
        onerole["name"]   = role.record[i].name;
        onerole["status"] = (unsigned int)role.record[i].status;
        client.val["roles"].push(onerole);
    }

    unsigned int granted_count = 0;
    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < perms.record.size(); i++)
    {
        auto const &p = perms.record[i];
        bool picked   = std::find(granted_ids.begin(), granted_ids.end(), p.permsid) != granted_ids.end();
        if (picked)
        {
            ++granted_count;
        }
        temp.clear();
        temp["permsid"]   = p.permsid;
        temp["parentid"]  = p.parentid;
        temp["urlpath"]   = p.urlpath;
        temp["name"]      = p.name;
        temp["permscode"] = p.permscode;
        temp["maskcode"]  = p.maskcode;
        temp["checked"]   = picked ? 1 : 0;
        client.val["infos"].push(temp);
    }

    client.val["companyid"]        = user_companyid;
    client.val["roleid"]           = roleid;
    client.val["role_ok"]          = role_ok ? 1 : 0;
    client.val["granted_count"]    = granted_count;
    client.val["assignable_total"] = (unsigned int)perms.record.size();
    rbacapi::issue_csrf(client);
    peer->view("admin/rbacroleperm");
    return "";
}

//@urlpath(admin_islogin,admin/rbac/userrole)
std::string rbac_page_userrole(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        client.status(403);
        client << "Only tenant administrators (sysuser.isadmin=1) may access permission management.";
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    // user_total counts all accounts of the company, while the selector below loads at most 200.
    orm::cms::Sysuser users;
    orm::cms::Sysuser counter;
    counter.where("companyid", user_companyid);
    client.val["user_total"] = counter.count();
    users.where("companyid", user_companyid).asc("adminid").limit(0, 200).fetch();

    orm::cms::Sysrole role;
    role.where("userid", user_companyid).asc("sortid").fetch();

    unsigned int userid = (unsigned int)client.get["userid"].to_int();
    bool found          = false;
    for (unsigned int i = 0; i < users.record.size(); i++)
    {
        if (users.record[i].adminid == userid)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        // 缺省或外公司 userid 回落到第一个账户
        // Missing or foreign userid falls back to the first account.
        userid = users.record.empty() ? 0 : users.record[0].adminid;
        found  = userid != 0;
    }

    std::vector<unsigned int> bound_ids;
    unsigned int legacy_roleid = 0;
    if (userid != 0)
    {
        orm::cms::Sysuser one;
        one.where("adminid", userid).whereAnd("companyid", user_companyid).fetch_one();
        legacy_roleid = one.data.roleid;

        orm::cms::Sysuserrole binding;
        binding.select("role_id").where("user_id", userid).whereAnd("company_id", user_companyid).fetch();
        for (unsigned int i = 0; i < binding.record.size(); i++)
        {
            bound_ids.push_back(binding.record[i].role_id);
        }
    }

    client.val["users"].set_array();
    obj_val oneuser;
    for (unsigned int i = 0; i < users.record.size(); i++)
    {
        oneuser.clear();
        oneuser["adminid"]    = users.record[i].adminid;
        oneuser["name"]       = users.record[i].name;
        oneuser["nickname"]   = users.record[i].nickname;
        oneuser["isopen"]     = (unsigned int)users.record[i].isopen;
        oneuser["isadmin"]    = (unsigned int)users.record[i].isadmin;
        oneuser["role_count"] = users.record[i].adminid == userid ? (unsigned int)bound_ids.size() : 0;
        client.val["users"].push(oneuser);
    }

    unsigned int bound_count = 0;
    client.val["roles"].set_array();
    obj_val onerole;
    for (unsigned int i = 0; i < role.record.size(); i++)
    {
        bool picked = std::find(bound_ids.begin(), bound_ids.end(), role.record[i].roleid) != bound_ids.end();
        if (picked)
        {
            ++bound_count;
        }
        onerole.clear();
        onerole["roleid"]  = role.record[i].roleid;
        onerole["name"]    = role.record[i].name;
        onerole["status"]  = (unsigned int)role.record[i].status;
        onerole["checked"] = picked ? 1 : 0;
        client.val["roles"].push(onerole);
    }

    client.val["companyid"]     = user_companyid;
    client.val["userid"]        = userid;
    client.val["user_ok"]       = found ? 1 : 0;
    client.val["bound_count"]   = bound_count;
    client.val["legacy_roleid"] = legacy_roleid;
    rbacapi::issue_csrf(client);
    peer->view("admin/rbacuserrole");
    return "";
}
}// namespace http
