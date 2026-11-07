#include "orm.h"

#include <algorithm>
#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "rbac/rbacapi.h"
#include "rbac/rbac.h"
#include "rbac/route_cache.h"
#include "rbac/userrole.h"

namespace http
{
namespace
{
// 本企业账户：adminid 存在且 companyid 等于当前会话的租户
// Accounts of this company: adminid exists and its companyid equals the session tenant.
bool user_in_company(unsigned int adminid, unsigned int companyid)
{
    orm::cms::Sysuser user;
    return user.where("adminid", adminid).whereAnd("companyid", companyid).fetch_one() > 0;
}

// 本企业角色集合（严格 userid == companyid，不含全局角色）
// Company role ids, strictly userid == companyid; global roles are not included.
std::vector<unsigned int> company_roleids(unsigned int companyid)
{
    orm::cms::Sysrole role;
    role.select("roleid").where("userid", companyid).asc("sortid").fetch();
    std::vector<unsigned int> out;
    for (unsigned int i = 0; i < role.record.size(); i++)
    {
        out.push_back(role.record[i].roleid);
    }
    return out;
}

// 关联表里已绑定的角色
// Role ids already bound in the relation table.
std::vector<unsigned int> bound_roleids(unsigned int adminid, unsigned int companyid)
{
    orm::cms::Sysuserrole binding;
    binding.select("role_id").where("user_id", adminid).whereAnd("company_id", companyid).fetch();
    std::vector<unsigned int> out;
    for (unsigned int i = 0; i < binding.record.size(); i++)
    {
        out.push_back(binding.record[i].role_id);
    }
    return out;
}
}// namespace

//@urlpath(admin_isloginjson,admin/rbac/userrole/list)
std::string rbac_userrole_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);

    unsigned int target = (unsigned int)client.get["userid"].to_int();
    orm::cms::Sysuser user;
    if (target == 0 || user.where("adminid", target).whereAnd("companyid", user_companyid).fetch_one() == 0)
    {
        rbacapi::fail(client, "user not found in this company");
        return "";
    }

    std::vector<unsigned int> bound = bound_roleids(target, user_companyid);

    client.val["code"] = 0;
    client.val["msg"]  = "ok";
    client.val["data"].set_array();
    for (unsigned int i = 0; i < bound.size(); i++)
    {
        client.val["data"].push(bound[i]);
    }
    // sysuser.roleid 是平台侧单角色，不由本页管理，原样回报
    // sysuser.roleid is the platform-side single role; this page reports it but never writes it.
    client.val["legacy_roleid"] = user.data.roleid;
    client.out_json();
    return "";
}

//@urlpath(admin_isloginjson,admin/rbac/userrole/save)
std::string rbac_userrole_save(std::shared_ptr<httppeer> peer)
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
    unsigned int operid         = rbacapi::adminid(client);

    unsigned int target = (unsigned int)client.post["userid"].to_int();
    if (target == 0 || !user_in_company(target, user_companyid))
    {
        rbacapi::fail(client, "user not found in this company");
        return "";
    }

    std::vector<unsigned int> wanted = rbacapi::uint_list(client.post["roleids"].to_string());
    std::vector<unsigned int> owned  = company_roleids(user_companyid);
    for (unsigned int i = 0; i < wanted.size(); i++)
    {
        // 严格本公司：userid=0 是无效归属，bind_user_role 内部同样拒绝
        // Strictly this company: userid=0 is an invalid owner, bind_user_role refuses it too.
        if (std::find(owned.begin(), owned.end(), wanted[i]) == owned.end())
        {
            rbacapi::fail(client, "role not found in this company");
            return "";
        }
    }

    // Unbind removed roles first, then bind new ones. There is no transaction, so a mid-loop
    // failure leaves the writes already applied in place.
    std::vector<unsigned int> current = bound_roleids(target, user_companyid);
    std::string error;
    for (unsigned int i = 0; i < current.size(); i++)
    {
        if (std::find(wanted.begin(), wanted.end(), current[i]) == wanted.end())
        {
            if (!rbac::unbind_user_role(target, current[i], user_companyid, error))
            {
                rbacapi::fail(client, error.empty() ? "unbind user role failed" : error);
                return "";
            }
        }
    }
    for (unsigned int i = 0; i < wanted.size(); i++)
    {
        if (std::find(current.begin(), current.end(), wanted[i]) == current.end())
        {
            if (!rbac::bind_user_role(target, wanted[i], user_companyid, operid, error))
            {
                rbacapi::fail(client, error.empty() ? "bind user role failed" : error);
                return "";
            }
        }
    }

    // 只动了这一个账户的授权，精确失效它自己
    // Only this account's grants changed, so invalidate exactly its snapshot.
    rbac::get_route_cache().invalidate_user(target, user_companyid);

    client.val["code"]   = 0;
    client.val["msg"]    = "ok";
    client.val["userid"] = target;
    client.out_json();
    return "";
}
}// namespace http
