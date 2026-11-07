#include <string>

#include "func.h"
#include "httppeer.h"
#include "rbac/check.h"
#include "rbac/rbacapi.h"
#include "rbac/rbac.h"

namespace http
{
//@urlpath(admin_isloginjson,admin/rbac/check)
std::string rbac_check(std::shared_ptr<httppeer> peer)
{
    httppeer &client            = peer->get_peer();
    // Tenant admins only; superadmin routes use a separate login guard.
    if (!rbacapi::company_admin(client))
    {
        rbacapi::forbid(client, "tenant admin only");
        return "";
    }
    unsigned int user_companyid = rbacapi::companyid(client);
    unsigned int current_admin  = rbacapi::adminid(client);

    // Either urlpath or permscode is required; urlpath wins when both are supplied.
    std::string urlpath = str_trim(client.get["urlpath"].to_string());
    unsigned int permscode = (unsigned int)client.get["permscode"].to_int();

    if (urlpath.empty() && permscode == 0)
    {
        rbacapi::fail(client, "urlpath or permscode is required");
        return "";
    }

    bool allowed = false;
    if (!urlpath.empty())
    {
        allowed = rbac::user_has_url(current_admin, user_companyid, urlpath);
    }
    else
    {
        allowed = rbac::user_has_permscode(current_admin, user_companyid, permscode);
    }

    client.val["code"]    = 0;
    client.val["msg"]     = "ok";
    client.val["allowed"] = allowed;
    client.out_json();
    return "";
}
}// namespace http
