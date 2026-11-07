#pragma once

#include <exception>
#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "orm.h"
#include "session_id.h"

namespace http
{
namespace rbacapi
{
inline constexpr const char *kCsrfSessionKey = "admin_rbac_csrf";

// 统一的 JSON 失败响应
// Unified JSON failure response.
inline void fail(httppeer &client, const std::string &msg)
{
    client.status(400);
    client.val["code"] = 1;
    client.val["msg"]  = msg;
    client.out_json();
}

// 登录时写入的 userid 就是 sysuser.companyid
// The userid written at login is sysuser.companyid.
inline unsigned int companyid(httppeer &client)
{
    return (unsigned int)client.session["userid"].to_int();
}

inline unsigned int adminid(httppeer &client)
{
    return (unsigned int)client.session["adminid"].to_int();
}

// 会话里的租户必须是非 0 的真实企业
// The session's tenant id must be a real company, not the 0 sentinel.
inline bool has_tenant(httppeer &client)
{
    return companyid(client) != 0;
}

// 租户管理员门禁：仅 sysuser.isadmin=1 的租户管理员可进入
// Tenant admin gate: only a tenant admin with sysuser.isadmin=1 may pass.
inline bool company_admin(httppeer &client)
{
    unsigned int aid = adminid(client);
    unsigned int cid = companyid(client);
    if (aid == 0 || cid == 0 || client.session["isadmin"].to_int() != 1)
    {
        return false;
    }
    // 会话里的 isadmin/companyid 可能已经过期（账号被停用，或被改到别的租户），
    // 这里回查 sysuser：只有当前 (adminid, companyid) 仍是启用中的本租户管理员才放行。
    // The session's isadmin/companyid may be stale (account disabled or moved to another tenant),
    // so re-check sysuser: allow only when this (adminid, companyid) is still an enabled admin
    // of this tenant.
    orm::cms::Sysuser user;
    user.select("adminid,isadmin,isopen").where("adminid", aid).whereAnd("companyid", cid).fetch_one();
    return user.data.adminid != 0 && user.data.isadmin == 1 && user.data.isopen == 1;
}

// 统一的 403 拒绝响应
// Unified 403 forbidden response.
inline void forbid(httppeer &client, const std::string &msg)
{
    client.status(403);
    client.val["code"] = 1;
    client.val["msg"]  = msg;
    client.out_json();
}

// 逗号分隔的无符号整型参数
// Comma separated unsigned integer parameter.
inline std::vector<unsigned int> uint_list(const std::string &text)
{
    std::vector<unsigned int> out;
    for (const std::string &one : mb_split(",", text))
    {
        std::string item = str_trim(one);
        if (!item.empty())
        {
            try
            {
                out.push_back((unsigned int)std::stoul(item));
            }
            catch (const std::exception &)
            {
                // 非法数字直接忽略，由调用方按空集合处理
                // Ignore invalid numbers; callers treat an empty list as no permissions.
            }
        }
    }
    return out;
}

// 页面表单签发；写接口首行校验。
// 令牌与会话绑定，会话里已有则复用：同一会话的多个页面、多个标签页共用一枚令牌，
// 每次 GET 都重新签发会让先打开的页面里的令牌先失效，提交必报失效。
// The token is session-scoped and issued once; a per-GET rotation would invalidate the
// token already rendered in any other page or tab of the same session.
inline void issue_csrf(httppeer &client)
{
    std::string token = client.session[kCsrfSessionKey].to_string();
    if (token.size() != 32)
    {
        token                           = get_rand_32char();
        client.session[kCsrfSessionKey] = token;
        client.save_session();
    }
    client.val["csrf_token"] = token;
}

inline bool check_csrf(httppeer &client)
{
    std::string session_token = client.session[kCsrfSessionKey].to_string();
    std::string post_token    = client.post["csrf_token"].to_string();
    return !session_token.empty() && session_token == post_token;
}

inline void csrf_fail(httppeer &client)
{
    fail(client, "Form token expired; refresh the page and submit again.");
}
}// namespace rbacapi
}// namespace http
