#include <algorithm>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "orm.h"
#include "orm_query.h"
#include "rbac/rbacapi.h"
#include "rbac/route_cache.h"
#include "superadmin/rbac/companyperm.h"
#include "superadmin/rbac/rbac_common.h"

namespace http
{

// 企业权限分配：syscompanyperms 的唯一写入口。
// Company permission allocation: the only write path for syscompanyperms.
// 这一层决定租户后台「能看到哪些权限项」；租户管理员只能从已分配的集合里挑，
// 改不动也删不掉这里的行。
// This layer decides which permission items a tenant admin can see in the tenant console;
// tenant admins can only pick from the allocated set and can neither modify nor delete these
// rows.
//@urlpath(superadmin_islogin,superadmin/rbac/companyperm)
std::string superadmin_rbac_companyperm(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    unsigned int agentid = (unsigned int)client.session["superid"].to_int();
    std::vector<unsigned int> sids;
    client.val["companies"].set_array();
    obj_val company;
    orm::cms::Siteinfo sites;
    sites.where("agentid", agentid).asc("sid").fetch();
    for (unsigned int i = 0; i < sites.record.size(); i++)
    {
        company.clear();
        company["sid"]         = (unsigned int)sites.record[i].sid;
        company["sitename"]    = sites.record[i].sitename;
        company["companyname"] = sites.record[i].companyname;
        client.val["companies"].push(company);
        sids.push_back((unsigned int)sites.record[i].sid);
    }

    unsigned int companyid = (unsigned int)client.get["companyid"].to_int();
    if (std::find(sids.begin(), sids.end(), companyid) == sids.end())
    {
        // 非法或缺省都回落到第一个站点；没有站点时保持 0，保存会被拒
        // Invalid or missing selection falls back to the first site; it stays 0 when the account
        // owns no site, and saving is then rejected.
        companyid = sids.empty() ? 0 : sids[0];
    }

    // 一次查回该企业的全部授权行，避免逐行查库
    // Load every allocation of this company in one query instead of querying per row.
    std::unordered_set<unsigned int> allocated;
    if (companyid != 0)
    {
        orm::cms::Syscompanyperms alloc;
        alloc.select("permsid").where("company_id", companyid).fetch();
        if (!alloc.iserror)
        {
            for (unsigned int i = 0; i < alloc.record.size(); ++i)
            {
                allocated.insert(alloc.record[i].permsid);
            }
        }
    }

    // 目录列全部状态：分配层管范围，status 管可用性，两层各自独立。
    // The catalog lists every status: allocation controls scope and status controls availability;
    // the two layers are independent. A disabled row keeps its allocation, but the decision chain
    // will not allow it until it is enabled (it requires status=1).
    orm::cms::Sysperms perms;
    if (companyid != 0)
    {
        perms.where("userid", companyid).asc("parentid", "permsid").fetch();
    }

    unsigned int enabled_count   = 0;
    unsigned int disabled_count  = 0;
    unsigned int allocated_count = 0;
    std::unordered_set<unsigned int> catalog_ids;
    for (unsigned int i = 0; i < perms.record.size(); ++i)
    {
        catalog_ids.insert(perms.record[i].permsid);
    }

    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = 0; i < perms.record.size(); ++i)
    {
        auto const &p = perms.record[i];
        bool enabled  = (p.status == 1);
        bool checked  = (allocated.find(p.permsid) != allocated.end());
        if (enabled)
        {
            ++enabled_count;
        }
        else
        {
            ++disabled_count;
        }
        if (checked)
        {
            ++allocated_count;
        }
        temp["permsid"]   = p.permsid;
        temp["parentid"]  = p.parentid;
        temp["urlpath"]   = p.urlpath;
        temp["name"]      = p.name;
        temp["permscode"] = p.permscode;
        temp["maskcode"]  = p.maskcode;
        temp["status"]    = (unsigned int)p.status;
        temp["checked"]   = checked ? 1 : 0;
        client.val["infos"].push(temp);
    }

    // 分配表里指向已删除目录行的残留：判定链是取交集，多几条不影响访问权，
    // 但说明目录删行时没同步清分配，给个提示便于清理。
    // Orphans are allocation rows pointing at deleted catalog rows. The decision chain intersects
    // both sets, so extra rows do not grant access; they only show that catalog deletion did not
    // clean up allocations, and are reported so they can be removed.
    unsigned int orphan_count = 0;
    for (auto const &permsid : allocated)
    {
        if (catalog_ids.find(permsid) == catalog_ids.end())
        {
            ++orphan_count;
        }
    }

    client.val["companyid"]       = companyid;
    client.val["company_count"]   = (unsigned int)sids.size();
    client.val["catalog_count"]   = (unsigned int)perms.record.size();
    client.val["enabled_count"]   = enabled_count;
    client.val["disabled_count"]  = disabled_count;
    client.val["allocated_count"] = allocated_count;
    client.val["orphan_count"]    = orphan_count;

    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/rbaccompanyperm");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/rbac/companyperm/save)
std::string superadmin_rbac_companyperm_save(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }

    // 目标企业必须是当前账号名下的站点
    // The target company must be a site owned by this account.
    unsigned int companyid = (unsigned int)client.post["companyid"].to_int();
    orm::cms::Siteinfo site;
    site.where("agentid", (unsigned int)client.session["superid"].to_int()).whereAnd("sid", companyid).fetch_one();
    if (site.data.sid == 0)
    {
        superadmin_rbac::json_fail(client, "Select a company first: companyid is invalid or does not belong to this account.");
        return "";
    }

    std::vector<unsigned int> wanted;
    for (unsigned int permsid : rbacapi::uint_list(client.post["permsids"].to_string()))
    {
        if (permsid != 0 && std::find(wanted.begin(), wanted.end(), permsid) == wanted.end())
        {
            wanted.push_back(permsid);
        }
    }

    // 先全量校验再删：整体替换中途拒绝会留下半套分配
    // Validate everything before deleting: rejecting halfway through a full replace would leave
    // half a set behind.
    std::unordered_set<unsigned int> exists_ids;
    if (!wanted.empty())
    {
        orm::cms::Sysperms catalog;
        catalog.select("permsid").where("userid", companyid).whereIn(orm::cms::sysperms_info::cols::permsid, wanted).fetch();
        if (catalog.iserror)
        {
            superadmin_rbac::json_fail(client, "Validation failed: could not read the permission catalog (" + catalog.error_msg + ").");
            return "";
        }
        for (unsigned int i = 0; i < catalog.record.size(); ++i)
        {
            exists_ids.insert(catalog.record[i].permsid);
        }
        for (unsigned int i = 0; i < wanted.size(); ++i)
        {
            if (exists_ids.find(wanted[i]) == exists_ids.end())
            {
                superadmin_rbac::json_fail(client, "Permission does not exist or does not belong to this company; nothing was saved (permsid=" + std::to_string(wanted[i]) + ").");
                return "";
            }
        }
    }

    unsigned int superid = (unsigned int)client.session["superid"].to_int();
    unsigned int now     = (unsigned int)timeid();

    // 整体替换必须在一条事务里：删掉旧分配后若某条 INSERT 失败，判定链会按半套集合放行或拒绝
    // The full replace needs one transaction: a failed INSERT after the DELETE leaves the
    // decision chain reading half a set.
    auto ulink = std::make_unique<orm::db_conn>("cms");
    if (!ulink->begin_commit())
    {
        superadmin_rbac::json_fail(client, "Failed to start a transaction; nothing was saved (" + ulink->error_msg + ").");
        return "";
    }

    unsigned int saved = 0;
    std::string err;
    {
        orm::cms::Syscompanyperms del;
        del.lock_conn(ulink->mysql_edit_conn);
        del.where("company_id", companyid).remove();
        del.unlock_conn();
        // 两个旗标各缺一角，都得看：catch 只写 error_msg 不置 iserror，驱动侧反过来只置旗标不写文案。
        // err 是下面循环的"继续"信号，失败时它必须非空，否则新旧分配会 commit 成并集还报成功。
        // Both flags are needed: the catch sets error_msg without iserror, the driver the other way round.
        // An empty err means "keep going", so a blank failure would commit old and new as one union.
        if (del.iserror || !del.error_msg.empty())
        {
            err = del.error_msg.empty() ? "delete of the old allocations did not take effect" : del.error_msg;
        }
    }
    for (unsigned int i = 0; err.empty() && i < wanted.size(); ++i)
    {
        orm::cms::Syscompanyperms row;
        row.lock_conn(ulink->mysql_edit_conn);
        row.data.company_id = companyid;
        row.data.permsid    = wanted[i];
        row.data.created_at = now;
        row.data.created_by = superid;
        auto result         = row.insert();
        row.unlock_conn();
        if (std::get<0>(result) == 0 || row.iserror)
        {
            err = row.error_msg.empty() ? "insert allocation failed (permsid=" + std::to_string(wanted[i]) + ")" : row.error_msg;
            break;
        }
        ++saved;
    }
    if (!err.empty())
    {
        ulink->rollback();
        superadmin_rbac::json_fail(client, "Allocation not saved, the whole set was rolled back: " + err);
        return "";
    }
    if (!ulink->commit())
    {
        ulink->rollback();
        superadmin_rbac::json_fail(client, "Failed to commit the allocation set, the whole set was rolled back: " + ulink->error_msg);
        return "";
    }

    // 分配层直接参与判定链：撤销一条分配 = 该企业所有角色立刻少一条权限，整表作废。
    // Allocation feeds the decision chain directly: revoking one allocation immediately removes
    // that permission from every role of the company, so the whole cache is dropped.
    rbac::get_route_cache().invalidate_all();
    client.val["code"]  = 0;
    client.val["msg"]   = "Company permission allocations saved successfully.";
    client.val["count"] = saved;
    client.out_json();
    return "";
}

}// namespace http
