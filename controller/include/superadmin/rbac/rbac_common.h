#pragma once

#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "orm.h"
#include "session_id.h"

namespace http
{
namespace superadmin_rbac
{

constexpr const char *kCsrfSessionKey = "superadmin_rbac_csrf";

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
        token = get_rand_32char();
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

inline void json_fail(httppeer &client, const std::string &msg)
{
    client.status(400);
    client.val["code"] = 1;
    client.val["msg"]  = msg;
    client.out_json();
}

inline void json_ok(httppeer &client, const std::string &msg)
{
    client.val["code"] = 0;
    client.val["msg"]  = msg;
    client.out_json();
}

// 403：目标不属于当前超管账号，与 400 的“参数不合法”区分开。
// 403: the target does not belong to this superadmin account; distinct from a 400 bad request.
inline void json_forbid(httppeer &client, const std::string &msg)
{
    client.status(403);
    client.val["code"] = 1;
    client.val["msg"]  = msg;
    client.out_json();
}

// 当前超管账号名下的站点 sid（role/user/perms 的 userid 字段都是它）。
// Site ids owned by this superadmin account (the userid field of role/user/perms rows).
inline std::vector<unsigned int> agent_sids(httppeer &client)
{
    std::vector<unsigned int> sids;
    orm::cms::Siteinfo sites;
    sites.select("sid").where("agentid", (unsigned int)client.session["superid"].to_int()).asc("sid").fetch();
    if (sites.iserror)
    {
        return sids;
    }
    for (unsigned int i = 0; i < sites.record.size(); ++i)
    {
        sids.push_back((unsigned int)sites.record[i].sid);
    }
    return sids;
}

// 目标企业必须归属于当前超管账号；0、外账号站点一律拒绝。
// The target company must belong to this superadmin account; 0 and foreign sites are rejected.
inline bool company_owned(httppeer &client, unsigned int companyid)
{
    if (companyid == 0)
    {
        return false;
    }
    orm::cms::Siteinfo site;
    site.select("sid").where("agentid", (unsigned int)client.session["superid"].to_int()).whereAnd("sid", companyid).fetch_one();
    return !site.iserror && site.data.sid != 0;
}

// Unified form-token failure message: tell the caller exactly what went wrong and how to fix it,
// instead of a bare code=1.
// 表单令牌失效统一提示：告诉用户具体原因与处理方式，而不是只返回 code=1。
inline void csrf_fail(httppeer &client)
{
    json_fail(client, "Form token expired; refresh the page and submit again.");
}

// Split a newline-joined urlpath list (the form posts all checked rows in a single hidden field).
// 按换行拆分 urlpath 列表（表单用 hidden 字段汇总勾选项）。
inline std::vector<std::string> split_lines(const std::string &text)
{
    std::vector<std::string> out;
    std::string item;
    for (char c : text)
    {
        if (c == '\n' || c == '\r')
        {
            std::string one = str_trim(item);
            if (!one.empty())
            {
                out.push_back(one);
            }
            item.clear();
        }
        else
        {
            item.push_back(c);
        }
    }
    std::string one = str_trim(item);
    if (!one.empty())
    {
        out.push_back(one);
    }
    return out;
}

}// namespace superadmin_rbac
}// namespace http
