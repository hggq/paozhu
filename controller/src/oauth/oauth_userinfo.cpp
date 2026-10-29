// 用户信息端点：GET /oauth/userinfo，凭证是 Bearer access_token（前置过滤器 oauth_bearerauth）。
// token 里的 sub 就是 sysuser.adminid（授权时从 session["adminid"] 取），按它查一行、只回白名单列。
// sysuser 里有 password/textword/mobile/email，fetch_one() 不带 select() 会把整行拉进 data，
// 所以列必须写全；sub 不是数字（client_credentials 那种）查不到行，落到 invalid_subject。

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

    long long adminid = client.session["oauth_sub"].to_int();

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
} // namespace http
