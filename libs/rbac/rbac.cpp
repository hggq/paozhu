#include "rbac.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include "datetime.h"
#include "orm.h"
#include "orm_query.h"

namespace http
{
namespace rbac
{
namespace
{
bool add_unique(std::vector<unsigned int> &items, unsigned int value)
{
    if (value == 0 || std::find(items.begin(), items.end(), value) != items.end())
    {
        return false;
    }
    items.push_back(value);
    return true;
}

bool tenant_owned(unsigned int owner, unsigned int companyid)
{
    // 只有平台登记给本企业的行才算数；0 是无效归属，不是可复用的全局层
    // Only rows the platform registered for this company count; 0 is an invalid owner,
    // not a reusable global tier.
    return companyid != 0 && owner == companyid;
}

// 把角色授权集窄化到平台分配集内：分配层只减不加，没分配给本企业的 permsid 一律不认。
// 判定链两处（load_role_perms / load_user_grants）必须同口径，否则页面列出的和运行期认的会漂移。
// Narrow role grants to the platform's allocation set; both judging paths share this rule.
bool keep_allocated(const std::vector<unsigned int> &candidates,
                    unsigned int companyid,
                    std::vector<unsigned int> &granted)
{
    granted.clear();
    if (candidates.empty())
    {
        return true;
    }
    std::vector<unsigned int> alloc;
    if (!load_company_perms(companyid, alloc))
    {
        return false;
    }
    if (alloc.empty())
    {
        return true;
    }
    std::vector<unsigned int> sorted = candidates;
    std::sort(sorted.begin(), sorted.end());
    std::sort(alloc.begin(), alloc.end());
    std::set_intersection(sorted.begin(), sorted.end(), alloc.begin(), alloc.end(), std::back_inserter(granted));
    return true;
}
}// namespace

bool role_in_company(unsigned int roleid, unsigned int companyid)
{
    if (roleid == 0 || companyid == 0)
    {
        return false;
    }
    orm::cms::Sysrole role;
    if (role.where("roleid", roleid).whereAnd("userid", companyid).fetch_one() == 0)
    {
        return false;
    }
    return tenant_owned(role.getUserid(), companyid);
}

bool load_company_perms(unsigned int companyid, std::vector<unsigned int> &permsids)
{
    permsids.clear();
    if (companyid == 0)
    {
        return false;
    }
    orm::cms::Syscompanyperms alloc;
    alloc.select("permsid").where("company_id", companyid).fetch();
    if (alloc.iserror)
    {
        return false;
    }
    for (unsigned int i = 0; i < alloc.record.size(); i++)
    {
        add_unique(permsids, alloc.record[i].permsid);
    }
    return true;
}

bool perms_assignable(unsigned int permsid, unsigned int companyid)
{
    if (permsid == 0 || companyid == 0)
    {
        return false;
    }
    // 分配行是这一层的真值：没有它，目录行存在也不给租户分配
    // The allocation row is this layer's truth; without it a catalog row is not assignable.
    orm::cms::Syscompanyperms alloc;
    if (alloc.where("company_id", companyid).whereAnd("permsid", permsid).fetch_one() == 0)
    {
        return false;
    }
    orm::cms::Sysperms perms;
    if (perms.where("permsid", permsid).whereAnd("userid", companyid).fetch_one() == 0)
    {
        return false;
    }
    // 停用的权限项不在可分配集合里，与页面列出的一致；归属仍要求目录行是本企业的
    // A disabled row is not assignable, matching what the pages list; the catalog row must
    // still belong to this tenant.
    return perms.getStatus() == 1 && tenant_owned(perms.getUserid(), companyid);
}

bool load_user_roles(unsigned int adminid, unsigned int companyid, std::vector<unsigned int> &roleids)
{
    roleids.clear();
    // companyid==0 will never survive tenant_owned below, so deny up front and skip the empty
    // queries that are guaranteed to come back with zero rows.
    // 0 租户在下面的 tenant_owned 里注定全灭，这里直接拒，省掉那几条空查询。
    if (adminid == 0 || companyid == 0)
    {
        return false;
    }

    std::vector<unsigned int> candidates;

    orm::cms::Sysuserrole binding;
    binding.where("user_id", adminid).whereAnd("company_id", companyid).fetch();
    for (unsigned int i = 0; i < binding.record.size(); i++)
    {
        add_unique(candidates, binding.record[i].role_id);
    }

    // 兼容历史单角色字段；只补充，不覆盖关联表
    // Keep the legacy single-role column as an additional source, never an override.
    orm::cms::Sysuser user;
    if (user.where("adminid", adminid).whereAnd("companyid", companyid).fetch_one() > 0)
    {
        add_unique(candidates, user.getRoleid());
    }

    if (candidates.empty())
    {
        return true;
    }

    orm::cms::Sysrole role;
    role.whereIn(orm::cms::sysrole_info::cols::roleid, candidates)
        .whereAnd("userid", companyid)
        .whereAnd("status", 1)
        .fetch();
    for (unsigned int i = 0; i < role.record.size(); i++)
    {
        if (tenant_owned(role.record[i].userid, companyid))
        {
            add_unique(roleids, role.record[i].roleid);
        }
    }
    return true;
}

bool load_role_perms(const std::vector<unsigned int> &roleids, unsigned int companyid, std::vector<unsigned int> &permsids)
{
    permsids.clear();
    if (companyid == 0)
    {
        return false;
    }
    if (roleids.empty())
    {
        return true;
    }

    std::vector<unsigned int> candidates;
    orm::cms::Sysroleperms roleperms;
    roleperms.whereIn(orm::cms::sysroleperms_info::cols::roleid, roleids)
        .whereAnd("userid", companyid)
        .fetch();
    for (unsigned int i = 0; i < roleperms.record.size(); i++)
    {
        add_unique(candidates, roleperms.record[i].permsid);
    }
    if (candidates.empty())
    {
        return true;
    }

    // Allocation narrowing: a permsid that the platform has not allocated to this tenant is
    // rejected even if the role references it.
    // 分配层窄化：平台没分配给本企业的 permsid，即使角色里有也不认。
    std::vector<unsigned int> allocated;
    if (!keep_allocated(candidates, companyid, allocated))
    {
        return false;
    }
    if (allocated.empty())
    {
        return true;
    }

    orm::cms::Sysperms perms;
    perms.whereIn(orm::cms::sysperms_info::cols::permsid, allocated)
        .whereAnd("userid", companyid)
        .whereAnd("status", 1)
        .fetch();
    for (unsigned int i = 0; i < perms.record.size(); i++)
    {
        if (tenant_owned(perms.record[i].userid, companyid))
        {
            add_unique(permsids, perms.record[i].permsid);
        }
    }
    return true;
}

bool load_user_grants(unsigned int adminid,
                      unsigned int companyid,
                      std::unordered_map<std::string, std::vector<unsigned int>> &grants)
{
    grants.clear();
    if (adminid == 0 || companyid == 0)
    {
        return false;
    }

    // 1) 用户 → 角色关联表；2) 兼容历史单角色字段
    // 1) user → role bindings; 2) legacy single-role column as an extra source.
    std::vector<unsigned int> candidates;

    orm::cms::Sysuserrole binding;
    binding.where("user_id", adminid).whereAnd("company_id", companyid).fetch();
    if (binding.iserror)
    {
        return false;
    }
    for (unsigned int i = 0; i < binding.record.size(); i++)
    {
        add_unique(candidates, binding.record[i].role_id);
    }

    orm::cms::Sysuser user;
    user.where("adminid", adminid).whereAnd("companyid", companyid).fetch_one();
    if (user.iserror)
    {
        return false;
    }
    // 查不到该 (adminid, companyid) 的账号，或账号已被关闭：一律 fail-closed。
    // 这条同时挡住“用户被改到别的租户后，旧 companyid 上残留的 sysuserrole 行”继续生效。
    // No account for this (adminid, companyid), or the account is disabled: fail closed.
    // This also stops leftover sysuserrole rows under the old companyid from granting access
    // after the user has been moved to another tenant.
    if (user.data.adminid == 0 || user.data.isopen != 1)
    {
        return false;
    }
    add_unique(candidates, user.data.roleid);
    if (candidates.empty())
    {
        return true;
    }

    // 3) 角色必须启用且属于本租户
    // 3) roles must be enabled and belong to this tenant.
    std::vector<unsigned int> roleids;
    orm::cms::Sysrole role;
    role.whereIn(orm::cms::sysrole_info::cols::roleid, candidates)
        .whereAnd("userid", companyid)
        .whereAnd("status", 1)
        .fetch();
    if (role.iserror)
    {
        return false;
    }
    for (unsigned int i = 0; i < role.record.size(); i++)
    {
        if (tenant_owned(role.record[i].userid, companyid))
        {
            add_unique(roleids, role.record[i].roleid);
        }
    }
    if (roleids.empty())
    {
        return true;
    }

    // 4) 角色 → 权限 ID；5) 分配层窄化；6) 权限 → urlpath（启用 + 同租户）
    // 4) role → permission ids; 5) narrow to the platform's allocation set; 6) permission → urlpath
    // (enabled + same tenant).
    std::vector<unsigned int> permsids;
    orm::cms::Sysroleperms roleperms;
    roleperms.whereIn(orm::cms::sysroleperms_info::cols::roleid, roleids)
        .whereAnd("userid", companyid)
        .fetch();
    if (roleperms.iserror)
    {
        return false;
    }
    for (unsigned int i = 0; i < roleperms.record.size(); i++)
    {
        add_unique(permsids, roleperms.record[i].permsid);
    }
    if (permsids.empty())
    {
        return true;
    }

    // 5) Allocation narrowing again at the final perms step: revoking one platform allocation
    //    removes one permsid here and the user immediately loses access, even though the
    //    sysroleperms row still looks intact.
    // 5) 分配层窄化：平台撤销一次分配 = 这里少一条 permsid，sysroleperms 一行不动也判无权。
    std::vector<unsigned int> allocated;
    if (!keep_allocated(permsids, companyid, allocated))
    {
        return false;
    }
    if (allocated.empty())
    {
        return true;
    }

    orm::cms::Sysperms perms;
    perms.whereIn(orm::cms::sysperms_info::cols::permsid, allocated)
        .whereAnd("userid", companyid)
        .whereAnd("status", 1)
        .fetch();
    if (perms.iserror)
    {
        return false;
    }
    for (unsigned int i = 0; i < perms.record.size(); i++)
    {
        if (!tenant_owned(perms.record[i].userid, companyid))
        {
            continue;
        }
        if (perms.record[i].urlpath.empty())
        {
            continue;
        }
        // Multiple permissions for the same urlpath each contribute one maskcode; they are not
        // OR-merged. Runtime check requires an exact match with any one of them.
        // 同一 urlpath 的多条权限各留一条 maskcode，不合并：判定要求路由掩码与其中某条精确相等。
        auto &masks = grants[perms.record[i].urlpath];
        if (std::find(masks.begin(), masks.end(), perms.record[i].maskcode) == masks.end())
        {
            masks.push_back(perms.record[i].maskcode);
        }
    }
    return true;
}

bool user_has_url(unsigned int adminid, unsigned int companyid, const std::string &urlpath)
{
    if (adminid == 0 || companyid == 0 || urlpath.empty())
    {
        return false;
    }

    std::vector<unsigned int> roleids;
    std::vector<unsigned int> permsids;
    if (!load_user_roles(adminid, companyid, roleids) || !load_role_perms(roleids, companyid, permsids))
    {
        return false;
    }
    if (permsids.empty())
    {
        return false;
    }

    orm::cms::Sysperms perms;
    if (perms.whereIn(orm::cms::sysperms_info::cols::permsid, permsids)
            .whereAnd("userid", companyid)
            .whereAnd("urlpath", urlpath)
            .whereAnd("status", 1)
            .fetch_one() == 0)
    {
        return false;
    }
    return tenant_owned(perms.getUserid(), companyid);
}

bool user_has_permscode(unsigned int adminid, unsigned int companyid, unsigned int permscode)
{
    if (adminid == 0 || companyid == 0 || permscode == 0)
    {
        return false;
    }

    std::vector<unsigned int> roleids;
    std::vector<unsigned int> permsids;
    if (!load_user_roles(adminid, companyid, roleids) || !load_role_perms(roleids, companyid, permsids))
    {
        return false;
    }
    if (permsids.empty())
    {
        return false;
    }

    orm::cms::Sysperms perms;
    if (perms.whereIn(orm::cms::sysperms_info::cols::permsid, permsids)
            .whereAnd("userid", companyid)
            .whereAnd("permscode", permscode)
            .whereAnd("status", 1)
            .fetch_one() == 0)
    {
        return false;
    }
    return tenant_owned(perms.getUserid(), companyid);
}

bool bind_user_role(unsigned int userid, unsigned int roleid, unsigned int companyid, unsigned int operid, std::string &error)
{
    error.clear();
    if (userid == 0 || roleid == 0 || companyid == 0)
    {
        error = "userid, roleid and companyid are required";
        return false;
    }

    orm::cms::Sysuser user;
    if (user.where("adminid", userid).whereAnd("companyid", companyid).fetch_one() == 0)
    {
        error = "user not found in this company";
        return false;
    }
    if (!role_in_company(roleid, companyid))
    {
        error = "role not found in this company";
        return false;
    }

    orm::cms::Sysuserrole binding;
    if (binding.where("user_id", userid).whereAnd("role_id", roleid).whereAnd("company_id", companyid).fetch_one() > 0)
    {
        error = "user role binding already exists";
        return false;
    }

    binding.data.user_id    = userid;
    binding.data.role_id    = roleid;
    binding.data.company_id = companyid;
    binding.data.created_at = timeid();
    binding.data.created_by = operid;

    auto result = binding.save();
    if (std::get<0>(result) == 0)
    {
        error = binding.error_msg.empty() ? "save user role binding failed" : binding.error_msg;
        return false;
    }
    return true;
}

bool unbind_user_role(unsigned int userid, unsigned int roleid, unsigned int companyid, std::string &error)
{
    error.clear();
    if (userid == 0 || roleid == 0 || companyid == 0)
    {
        error = "userid, roleid and companyid are required";
        return false;
    }

    orm::cms::Sysuserrole binding;
    binding.where("user_id", userid).whereAnd("role_id", roleid).whereAnd("company_id", companyid);
    int affected = binding.remove();
    if (affected == 0)
    {
        error = "user role binding not found";
        return false;
    }
    return true;
}

replace_role_perms_result replace_role_perms(unsigned int roleid, unsigned int companyid, const std::vector<unsigned int> &permsids, unsigned int operid, std::string &error)
{
    error.clear();
    if (companyid == 0)
    {
        error = "companyid is required";
        return replace_role_perms_result::rejected;
    }
    if (!role_in_company(roleid, companyid))
    {
        error = "role not found in this company";
        return replace_role_perms_result::rejected;
    }

    std::vector<unsigned int> uniqueperms;
    for (unsigned int i = 0; i < permsids.size(); i++)
    {
        add_unique(uniqueperms, permsids[i]);
    }

    // 先全量校验再删：整体替换中途拒绝会留下半套授权
    // Validate the whole set before deleting: a late refusal would leave half a grant set.
    for (unsigned int i = 0; i < uniqueperms.size(); i++)
    {
        if (!perms_assignable(uniqueperms[i], companyid))
        {
            error = "permission not assignable in this company";
            return replace_role_perms_result::rejected;
        }
    }

    // 先把语句全部拼成 SQL 字符串（DELETE 旧集合 + 每条 INSERT），再放进同一个事务执行；
    // 任何一条失败或提交失败都整体回滚，库里保持调用前的旧集合。
    // Build every statement first (DELETE the old set, then one INSERT per permission), then run
    // them on a single transaction connection. Any failed write or commit rolls the whole thing
    // back, so the previous set stays intact.
    orm::cms::Sysroleperms del;
    del.where("roleid", roleid).whereAnd("userid", companyid);
    std::string deletesql = del.commit_remove();

    std::vector<std::string> insertsql;
    insertsql.reserve(uniqueperms.size());
    for (unsigned int i = 0; i < uniqueperms.size(); i++)
    {
        orm::cms::Sysroleperms row;
        row.data.userid  = companyid;
        row.data.roleid  = roleid;
        row.data.permsid = uniqueperms[i];
        row.data.sortid  = (int)i;
        insertsql.push_back(row.commit_insert());
    }

    auto ulink = std::make_unique<orm::db_conn>("cms");
    if (!ulink->begin_commit())
    {
        error = "failed to start a transaction; nothing was written: " + ulink->error_msg;
        return replace_role_perms_result::rolled_back;
    }

    bool write_failed = false;
    std::string dbmsg;
    try
    {
        ulink->edit_query(deletesql);
        if (ulink->iserror)
        {
            write_failed = true;
            dbmsg       = ulink->error_msg;
        }
        for (unsigned int i = 0; !write_failed && i < insertsql.size(); i++)
        {
            ulink->edit_query(insertsql[i]);
            if (ulink->iserror)
            {
                write_failed = true;
                dbmsg       = ulink->error_msg;
            }
        }
    }
    catch (const std::exception &e)
    {
        write_failed = true;
        dbmsg        = e.what();
    }

    if (write_failed)
    {
        ulink->rollback();
        error = "role permission write failed, the whole set was rolled back: " + (dbmsg.empty() ? "unknown error" : dbmsg);
        return replace_role_perms_result::rolled_back;
    }
    if (!ulink->commit())
    {
        std::string commitmsg = ulink->error_msg;
        ulink->rollback();
        error = "commit failed, the whole set was rolled back: " + (commitmsg.empty() ? "unknown error" : commitmsg);
        return replace_role_perms_result::rolled_back;
    }

    // 记录本次操作的执行人，便于后续审计扩展
    // Keep the operator id for future audit extension.
    (void)operid;
    return replace_role_perms_result::success;
}
}// namespace rbac
}// namespace http
