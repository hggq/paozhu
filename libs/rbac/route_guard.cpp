#include "route_guard.h"

#include "httppeer.h"
#include "route_cache.h"
#include "router.h"

namespace http
{
namespace rbac
{

unsigned int check_masked_route(httppeer &client, const std::string &urlpath, unsigned int mask_code)
{
    // 会话键的语义只在这一处：adminid = sysuser.adminid，userid = sysuser.companyid。
    // The session key semantics live only here: adminid = sysuser.adminid, userid = sysuser.companyid.
    // 没登录和没权限要分开回，两者的处置动作不同（一个去登录，一个找管理员授权）。
    // Missing login and missing permission must return different statuses: the follow-up actions
    // differ (one goes to login, the other asks an admin for a grant).
    // 判定顺序：先 adminid（401），再 companyid（403），最后才进缓存判定。
    // Order: adminid first (401), then companyid (403), and only then the cache decision.
    unsigned int adminid   = static_cast<unsigned int>(client.session["adminid"].to_int());
    unsigned int companyid = static_cast<unsigned int>(client.session["userid"].to_int());
    if (adminid == 0)
    {
        return 401;
    }
    // userid==0 是登录态，但不是有效租户；0 不是可复用的全局层，回 403 让它去找管理员
    // userid==0 means authenticated but not a valid tenant; 0 is not a reusable global layer,
    // so return 403 and let the user ask an admin for a grant.
    if (companyid == 0)
    {
        return 403;
    }

    // 装载、TTL、代次失效、fail-closed 全在 route_cache 里；这里只做状态码适配。
    // Loading, TTL, generation invalidation and fail-closed all live in route_cache; this layer
    // only maps the result to a status code.
    // allow()==false covers no grant, maskcode drift and database load failure (fail-closed), and
    // is reported as 403 here after the 401/403 checks above.
    return get_route_cache().allow(adminid, companyid, urlpath, mask_code) ? 0 : 403;
}

void install_route_guard()
{
    set_route_auth_hook([](httppeer &client, const std::string &urlpath, unsigned int mask_code)
                        { return check_masked_route(client, urlpath, mask_code); });
}

}// namespace rbac
}// namespace http
