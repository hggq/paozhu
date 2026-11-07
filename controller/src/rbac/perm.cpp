#include "orm.h"

#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "rbac/rbacapi.h"
#include "rbac/rbac.h"
#include "rbac/perm.h"

namespace http
{
//@urlpath(admin_isloginjson,admin/rbac/perm/list)
std::string rbac_perm_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    orm::cms::Sysperms perms;
    // 天花板：租户能分配的是平台分配给本企业的权限项（syscompanyperms），目录行还要启用中
    // Ceiling: rows the platform allocated to this company, still enabled in the catalog.
    std::vector<unsigned int> alloc;
    rbac::load_company_perms(user_companyid, alloc);
    // A failed lookup also leaves alloc empty, so the response degrades to an empty list
    // (fail-closed) instead of exposing the full catalog.
    if (!alloc.empty())
    {
        perms.whereIn(orm::cms::sysperms_info::cols::permsid, alloc)
            .whereAnd("userid", user_companyid)
            .whereAnd("status", 1)
            .asc("parentid", "permsid")
            .fetch();
    }

    client.val["code"] = 0;
    client.val["msg"]  = "ok";
    client.val["data"].set_array();
    obj_val item;
    for (unsigned int i = 0; i < perms.record.size(); i++)
    {
        item.clear();
        item["permsid"]   = perms.record[i].permsid;
        item["parentid"]  = perms.record[i].parentid;
        item["userid"]    = perms.record[i].userid;
        item["urlpath"]   = perms.record[i].urlpath;
        item["urlname"]   = perms.record[i].urlname;
        item["name"]      = perms.record[i].name;
        item["status"]    = perms.record[i].status;
        item["permscode"] = perms.record[i].permscode;
        item["maskcode"]  = perms.record[i].maskcode;
        item["readonly"]  = perms.record[i].readonly;
        client.val["data"].push(item);
    }
    client.out_json();
    return "";
}
}// namespace http
