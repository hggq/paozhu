#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "orm.h"
#include "rbac/route_cache.h"
#include "rbac/route_catalog.h"
#include "superadmin/rbac/registerfunc.h"
#include "superadmin/rbac/rbac_common.h"

namespace http
{

//@urlpath(superadmin_islogin,superadmin/registerfunc)
std::string superadmin_registerfunc(std::shared_ptr<httppeer> peer)
{
    httppeer &client  = peer->get_peer();
    unsigned int slot = rbac::current_slot_for_host(peer->host_index);
    bool showall      = client.get["showall"].to_int() == 1;

    // 权限项按租户登记：sysperms.userid = siteinfo.sid = sysuser.companyid
    // Per-tenant registration: sysperms.userid is siteinfo.sid, which is sysuser.companyid.
    unsigned int agentid = (unsigned int)client.session["superid"].to_int();
    unsigned int siteid  = (unsigned int)client.get_siteid();
    std::vector<unsigned int> sids;
    client.val["companies"].set_array();
    obj_val company;
    orm::cms::Siteinfo sites;
    sites.where("agentid", agentid).asc("sid").fetch();
    for (unsigned int i = 0; i < sites.record.size(); i++)
    {
        // 路由表属于当前 host 的 slot：公司下拉只列当前站点，避免把 A 站点的路由登记给 B 公司。
        // The route table belongs to the current host slot: list only the current site so routes
        // from host A can never be registered for company B.
        if (siteid > 0 && (unsigned int)sites.record[i].sid != siteid && (unsigned int)sites.record[i].userid != siteid)
        {
            continue;
        }
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
        // Invalid or missing selection falls back to the first site; 0 when there is none.
        companyid = sids.empty() ? 0 : sids[0];
    }

    std::vector<rbac::route_entry> routes = showall ? rbac::list_all_routes() : rbac::collect_routes(slot);

    // 一次查回已登记的 urlpath 及其 maskcode，避免逐行查库
    // Load the registered urlpaths and their maskcodes in one query instead of per row. Only
    // mask-bearing routes are queried, matching the rows that can appear in the list.
    std::vector<std::string> paths;
    for (auto const &r : routes)
    {
        if (r.mask_code != 0)
        {
            paths.push_back(r.urlpath);
        }
    }
    std::map<std::string, unsigned int> registered;
    if (companyid != 0 && !paths.empty())
    {
        orm::cms::Sysperms perms;
        perms.select("urlpath,maskcode").where("userid", companyid).whereIn(orm::cms::sysperms_info::cols::urlpath, paths).fetch();
        for (unsigned int i = 0; i < perms.record.size(); ++i)
        {
            registered[perms.record[i].urlpath] = perms.record[i].maskcode;
        }
    }

    // 服务端分页：默认每页 100 条；?page=N 越界时收敛到有效页。
    // Server-side pagination: 100 rows per page; out-of-range ?page=N is clamped.
    const unsigned int page_size = 100;
    int page_request             = client.get["page"].to_int();
    unsigned int page            = page_request < 1 ? 1 : (unsigned int)page_request;

    struct row_view
    {
        const rbac::route_entry *route;
        bool registered;
        unsigned int stored_mask;
        bool drift;
    };

    unsigned int candidate_count  = 0;
    unsigned int registered_count = 0;
    unsigned int drift_count      = 0;
    bool only_candidate           = client.get["candidate"].to_int() == 1;
    std::vector<row_view> rows;
    rows.reserve(routes.size());
    for (auto const &r : routes)
    {
        // 默认列出命名空间内全部路由；candidate=1 只保留可登记项（maskcode 非 0）。
        // List every route in this namespace by default; candidate=1 keeps registrable rows only.
        if (only_candidate && r.mask_code == 0)
        {
            continue;
        }
        auto reg_it = registered.find(r.urlpath);
        bool is_reg = (reg_it != registered.end());
        // 漂移 = 库里登记的 maskcode 和代码注册的掩码不一致；运行期判定会因此一直 403。
        // Drift = the stored maskcode differs from the code-registered mask; the runtime check
        // requires exact equality, so such a route is permanently denied with 403.
        bool is_drift = (r.mask_code != 0 && is_reg && reg_it->second != r.mask_code);
        if (r.mask_code != 0)
        {
            ++candidate_count;
            if (is_reg)
            {
                ++registered_count;
            }
            if (is_drift)
            {
                ++drift_count;
            }
        }
        rows.push_back({&r, is_reg, is_reg ? reg_it->second : 0, is_drift});
    }

    unsigned int list_total  = (unsigned int)rows.size();
    unsigned int total_pages = list_total == 0 ? 1 : (list_total + page_size - 1) / page_size;
    if (page > total_pages)
    {
        page = total_pages;
    }
    unsigned int page_start_idx = (page - 1) * page_size;
    unsigned int page_end_idx   = std::min(page_start_idx + page_size, list_total);

    client.val["infos"].set_array();
    obj_val temp;
    for (unsigned int i = page_start_idx; i < page_end_idx; ++i)
    {
        auto const &row     = rows[i];
        auto const &r       = *row.route;
        temp["site"]        = r.site;
        temp["urlpath"]     = r.urlpath;
        temp["title"]       = r.title;
        temp["mask_code"]   = r.mask_code;
        temp["rate_time"]   = (unsigned int)r.rate_time;
        temp["rate_num"]    = (unsigned int)r.rate_num;
        temp["pre_kind"]    = (unsigned int)r.pre_kind;
        temp["reg_kind"]    = (unsigned int)r.reg_kind;
        temp["candidate"]   = r.mask_code != 0 ? 1 : 0;
        temp["registered"]  = row.registered ? 1 : 0;
        temp["stored_mask"] = row.stored_mask;
        temp["drift"]       = row.drift ? 1 : 0;
        client.val["infos"].push(temp);
    }

    client.val["total"]            = (unsigned int)routes.size();
    client.val["candidate_count"]  = candidate_count;
    client.val["registered_count"] = registered_count;
    client.val["drift_count"]      = drift_count;
    client.val["list_total"]       = list_total;
    client.val["page"]             = page;
    client.val["page_size"]        = page_size;
    client.val["total_pages"]      = total_pages;
    client.val["page_start"]       = list_total == 0 ? 0 : page_start_idx + 1;
    client.val["page_end"]         = page_end_idx;
    client.val["current_site"]     = rbac::site_name_for_slot(slot);
    client.val["showall"]          = showall ? 1 : 0;
    client.val["companyid"]        = companyid;
    client.val["company_count"]    = (unsigned int)sids.size();
    client.val["candidate_only"]   = only_candidate ? 1 : 0;

    // 分页与筛选链接都在服务端拼好，视图只负责输出，避免模板里重复条件分支。
    // Build pagination/filter links here so the view stays a plain echo of values.
    auto build_url = [&](bool all_sites, bool filter_candidate, unsigned int target_page)
    {
        std::string url = "/superadmin/registerfunc?companyid=" + std::to_string(companyid);
        if (all_sites)
        {
            url += "&showall=1";
        }
        if (filter_candidate)
        {
            url += "&candidate=1";
        }
        if (target_page > 1)
        {
            url += "&page=" + std::to_string(target_page);
        }
        return url;
    };
    client.val["url_prev"]      = page > 1 ? build_url(showall, only_candidate, page - 1) : "";
    client.val["url_next"]      = page < total_pages ? build_url(showall, only_candidate, page + 1) : "";
    client.val["url_candidate"] = build_url(showall, !only_candidate, 1);
    client.val["url_showall"]   = build_url(!showall, only_candidate, 1);

    superadmin_rbac::issue_csrf(client);
    peer->view("superadmin/registerfunc");
    return "";
}

//@urlpath(superadmin_isloginjson,superadmin/registerfunc/save)
std::string superadmin_registerfunc_save(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!superadmin_rbac::check_csrf(client))
    {
        superadmin_rbac::csrf_fail(client);
        return "";
    }

    std::vector<std::string> requested = superadmin_rbac::split_lines(client.post["urlpaths"].to_string());
    if (requested.empty())
    {
        superadmin_rbac::json_fail(client, "Select at least one registerable urlpath (only candidate routes that are not yet registered can be selected).");
        return "";
    }

    // 登记目标必须是当前账号名下的站点，userid 直接取它的 sid
    // The target must be a site owned by this account; userid is that site's sid.
    unsigned int companyid = (unsigned int)client.post["companyid"].to_int();
    unsigned int superid   = (unsigned int)client.session["superid"].to_int();
    orm::cms::Siteinfo site;
    site.where("agentid", superid).whereAnd("sid", companyid).fetch_one();
    if (site.data.sid == 0)
    {
        superadmin_rbac::json_fail(client, "Select a company first: companyid is invalid or does not belong to this account.");
        return "";
    }
    // 提交的 companyid 必须与当前 host 的站点一致，否则这条请求就是在拿别的 slot 的路由表登记。
    // The submitted companyid must match the site served by this host; otherwise the request
    // would register another slot's route table.
    unsigned int siteid = (unsigned int)client.get_siteid();
    if (siteid > 0 && (unsigned int)site.data.sid != siteid && (unsigned int)site.data.userid != siteid)
    {
        superadmin_rbac::json_fail(client, "Select the company of the site you are currently managing.");
        return "";
    }

    unsigned int slot = rbac::current_slot_for_host(peer->host_index);
    std::map<std::string, rbac::route_entry> candidate;
    for (auto const &r : rbac::collect_routes(slot))
    {
        if (r.mask_code != 0)
        {
            candidate[r.urlpath] = r;
        }
    }

    unsigned int inserted     = 0;
    unsigned int synced       = 0;
    unsigned int skipped      = 0;
    unsigned int rejected     = 0;
    unsigned int alloc_failed = 0;
    for (auto const &path : requested)
    {
        auto it = candidate.find(path);
        if (it == candidate.end())
        {
            ++rejected;
            continue;
        }

        orm::cms::Sysperms exists;
        exists.where("userid", companyid).whereAnd("urlpath", path).fetch_one();
        if (exists.data.permsid != 0)
        {
            // 已登记但库里记的 maskcode 和代码里注册的掩码不一致：以注册表为准改写。
            // 运行期判定要求授权掩码与路由掩码精确相等，漂移不修就一直是 403。
            // Already registered but the stored maskcode differs from the code-registered mask:
            // rewrite it from the registration catalog. The runtime check requires exact equality,
            // so unrepaired drift is denied with 403 forever.
            if (exists.data.maskcode != it->second.mask_code)
            {
                orm::cms::Sysperms sync;
                sync.data.maskcode     = it->second.mask_code;
                sync.data.updated_time = timeid();
                sync.where("permsid", exists.data.permsid);
                if (sync.update("maskcode,updated_time") > 0)
                {
                    ++synced;
                }
                else
                {
                    ++rejected;
                }
                continue;
            }
            ++skipped;
            continue;
        }

        orm::cms::Sysperms perms;
        perms.data.userid   = companyid;
        perms.data.parentid = 0;
        perms.data.urlpath  = path;
        perms.data.urlname  = path;
        // 注解 :title(名称) 写了就用它当权限项名称，没写仍回落成 urlpath
        // Use the annotation's :title as the permission name when present; otherwise fall back to
        // the urlpath.
        perms.data.name            = it->second.title.empty() ? path : it->second.title;
        perms.data.status          = 1;
        perms.data.permscode       = 0;
        perms.data.maskcode        = it->second.mask_code;
        perms.data.readonly        = 0;
        perms.data.created_time    = timeid();
        perms.data.updated_time    = perms.data.created_time;
        perms.data.created_user    = 0;
        perms.data.updated_user    = 0;
        auto [effect_num, last_id] = perms.save();
        if (effect_num == 0)
        {
            ++rejected;
        }
        else
        {
            ++inserted;
            // 登记即分配：新目录行立刻进入该企业的可分配集合，
            // 否则企业权限页第一次打开时历史行是勾上的、新登记的行永远不勾，两条路看不出差别。
            // Register means allocate: the new catalog row immediately joins this company's
            // assignable set. Otherwise, on the first visit to the Company Permissions page
            // historical rows are checked while newly registered rows never are, hiding the
            // difference between the two paths.
            orm::cms::Syscompanyperms alloc;
            alloc.data.company_id = companyid;
            alloc.data.permsid    = last_id;
            alloc.data.created_at = timeid();
            alloc.data.created_by = superid;
            if (std::get<0>(alloc.insert()) == 0)
            {
                ++alloc_failed;
            }
        }
    }

    // 新登记的 urlpath 进入权限目录，已有用户可能立刻被授予它；maskcode 改写会改变
    // 掩码精确相等判定。两种变化都可能让缓存里的快照判错，所以都要整表作废。
    // A newly registered urlpath enters the catalog and may be granted to existing users
    // immediately; rewriting a maskcode changes the exact-equality check. Either change can make
    // cached snapshots decide wrong, so both invalidate the whole cache.
    if (inserted > 0 || synced > 0)
    {
        rbac::get_route_cache().invalidate_all();
    }
    client.val["code"]      = 0;
    client.val["msg"]       = alloc_failed > 0 ? "Registration completed, but " + std::to_string(alloc_failed) + " entries were not added to the company's assignable set (fix them on the Company Permissions page)." : "Registration completed successfully.";
    client.val["companyid"] = companyid;
    client.val["inserted"]  = inserted;
    client.val["synced"]    = synced;
    client.val["skipped"]   = skipped;
    client.val["rejected"]  = rejected;
    client.val["total"]     = (unsigned int)requested.size();
    client.out_json();
    return "";
}

}// namespace http
