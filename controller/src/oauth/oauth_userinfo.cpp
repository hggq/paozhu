// 用户信息端点：GET /oauth/userinfo，凭证是 Bearer access_token（前置过滤器 oauth_bearerauth）。
// 主体从过滤器注入的 session["adminid"] 拿，不在这里解析令牌里的 sub：
// sub 是不是人由 gty 那一格说，判定只留在过滤器一处（签发端改形状时这里不用跟着改）。
// sysuser 里有 password/textword/mobile/email，fetch_one() 不带 select() 会把整行拉进 data，
// 所以列必须写全。

#include <string>

#include "httppeer.h"
#include "oauth_userinfo.h"
#include "orm.h"

namespace http
{
//@urlpath(oauth_bearerauth,oauth/userinfo)
std::string oauth_userinfo(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.set_header("Cache-Control", "no-store");
    client.set_header("X-Frame-Options", "DENY");

    // 机器令牌（client_credentials 签的那一种）没有授权人：报在主体上，不去查库猜号段。
    if (client.session["oauth_gty"].to_string() != "user")
    {
        client.status(400);
        client.val["error"] = "invalid_subject";
        client.out_json();
        return "";
    }
    long long adminid = client.session["adminid"].to_int();
    if (adminid <= 0)
    {
        client.status(400);
        client.val["error"] = "invalid_subject";
        client.out_json();
        return "";
    }

    auto model = orm::cms::Sysuser();
    model.select("adminid,name,nickname,realname,avatar,companyid");
    model.where("adminid", adminid);
    if (model.fetch_one() == 0)
    {
        // 令牌本身没问题，是它代表的用户查不到：报在主体上，不报 invalid_token
        client.status(400);
        client.val["error"] = "invalid_subject";
        client.out_json();
        return "";
    }

    // 键名用列原文，不借 OIDC 的标准声明名（preferred_username / picture）：
    // 这个 AS 不发 id_token，scopes_supported 是空数组；长得像 OIDC 响应会诱导
    // 客户端按 OIDC 来配它。sub 是令牌里那个主体，companyid 是租户那一列——同一行数据
    // 前置过滤器已经写进本次请求的 session["userid"] 了，业务端点不用自己再查一遍。
    client.val["sub"]       = std::to_string(model.data.adminid);
    client.val["name"]      = model.data.name;
    client.val["nickname"]  = model.data.nickname;
    client.val["realname"]  = model.data.realname;
    client.val["avatar"]    = model.data.avatar;
    client.val["companyid"] = model.data.companyid;
    client.out_json();
    return "";
}
}// namespace http
