#include <algorithm>
#include <string>

#include "func.h"
#include "httppeer.h"
#include "orm.h"
#include "rbac/route_cache.h"
#include "superadmin/rbac/rbac_common.h"
#include "superadmin/rbac/rbacperm.h"

namespace http
{

//@urlpath(superadmin_islogin,superadmin/rbac/perm)
std::string superadmin_rbac_perm_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client    = peer->get_peer();
    unsigned int userid = (unsigned int)client.get["userid"].to_int();
    orm::cms::Sysperms perms;
    std::vector<unsigned int> sids = superadmin_rbac::agent_sids(client);
    if (userid > 0 && std::find(sids.begin(), sids.end(), userid) == sids.end())
    {
        // 外账号企业的过滤条件不返回任何行。
        // A filter on a foreign company returns no rows.
        sids.clear();
    }
    if (!sids.empty())
    {
        if (userid > 0)
        {
            perms.where("userid", userid);
        }
        else
        {
            perms.whereIn("userid", sids);
        }
    }
    else
    {
        // 无站点或外账号过滤：返回空列表，不能退化为列出全部目录行。
        // No owned site or a foreign filter: return nothing instead of listing the whole catalog.
        perms.where("permsid", 0);
    }
    perms.asc("parentid", "permsid").fetch();

    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < perms.record.size(); ++i)
    {
        auto const &p        = perms.record[i];
        temp["permsid"]      = p.permsid;
        temp["parentid"]     = p.parentid;
        temp["userid"]       = p.userid;
        temp["urlpath"]      = p.urlpath;
        temp["urlname"]      = p.urlname;
        temp["name"]         = p.name;
        temp["status"]       = (unsigned int)p.status;
        temp["permscode"]    = p.permscode;
        temp["maskcode"]     = p.maskcode;
        temp["readonly"]     = (unsigned int)p.readonly;
        temp["created_time"] = get_date("%Y-%m-%d %X", p.created_time);
        client.val["infos"].push(temp);
    }
    client.val["query_userid"] = userid;
    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/rbacpermplist");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/perm/editpost)
std::string superadmin_rbac_perm_editpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    unsigned int permsid = (unsigned int)client.post["permsid"].to_int();
    orm::cms::Sysperms perm;
    perm.where("permsid", permsid).fetch_one();
    if (perm.data.permsid == 0)
    {
        superadmin_rbac::json_fail(client, "Permission record not found or already deleted (invalid permsid).");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)perm.data.userid))
    {
        superadmin_rbac::json_forbid(client, "Permission record does not belong to this account.");
        return "";
    }

    // Empty name/urlname fall back to the stored urlpath.
    std::string name    = str_trim(client.post["name"].to_string());
    std::string urlname = str_trim(client.post["urlname"].to_string());
    if (name.empty())
    {
        name = perm.data.urlpath;
    }
    if (urlname.empty())
    {
        urlname = perm.data.urlpath;
    }

    orm::cms::Sysperms update;
    update.data.parentid     = (unsigned int)client.post["parentid"].to_int();
    update.data.urlname      = urlname;
    update.data.name         = name;
    update.data.status       = client.post["status"].to_int() == 1 ? 1 : 0;
    update.data.permscode    = (unsigned int)client.post["permscode"].to_int();
    update.data.readonly     = client.post["readonly"].to_int() == 1 ? 1 : 0;
    update.data.updated_time = timeid();
    update.data.updated_user = 0;
    update.where("permsid", permsid);
    if (update.update("parentid,urlname,name,status,permscode,readonly,updated_time,updated_user") == 0)
    {
        superadmin_rbac::json_fail(client, "Save failed: no field values changed, or the database write failed.");
        return "";
    }
    // 权限目录的 status / permscode 变了，所有用户的快照都可能受影响。
    // Catalog status and permscode changes can affect every user's snapshot.
    rbac::get_route_cache().invalidate_all();
    superadmin_rbac::json_ok(client, "Permission saved successfully.");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/perm/toggle)
std::string superadmin_rbac_perm_toggle(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }
    unsigned int permsid = (unsigned int)client.post["permsid"].to_int();
    int status           = client.post["status"].to_int() == 1 ? 1 : 0;
    orm::cms::Sysperms perm;
    perm.where("permsid", permsid).fetch_one();
    if (perm.data.permsid == 0)
    {
        superadmin_rbac::json_fail(client, "Permission record not found or already deleted (invalid permsid).");
        return "";
    }
    if (!superadmin_rbac::company_owned(client, (unsigned int)perm.data.userid))
    {
        superadmin_rbac::json_forbid(client, "Permission record does not belong to this account.");
        return "";
    }
    orm::cms::Sysperms update;
    update.data.status       = (char)status;
    update.data.updated_time = timeid();
    update.where("permsid", permsid);
    update.update("status,updated_time");
    // 启用/停用一条权限会直接改变谁能访问对应 urlpath。
    // Enabling or disabling one permission directly changes who may reach that urlpath.
    rbac::get_route_cache().invalidate_all();
    client.val["code"]   = 0;
    client.val["msg"]    = "Permission status updated.";
    client.val["status"] = status;
    client.out_json();
    return "";
}

}// namespace http
