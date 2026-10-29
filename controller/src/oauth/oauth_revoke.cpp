// OAuth 2.1 / RFC 7009 撤销端点：POST /oauth/revoke。
// 必须过客户端认证——否则任何拿到 refresh token 的人都能撤销它，撤销变成免费的拒绝服务入口。
// 未找到 / 已撤销也回 200 空体（防枚举）；只有认证失败才是 401。

#include <string>

#include "cost_define.h"
#include "datetime.h"
#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_store.h"
#include "oauth_revoke.h"

namespace http
{
namespace
{
void revoke_error(httppeer &client, unsigned int status, const std::string &error, const std::string &detail)
{
    client.status(status);
    client.val["error"] = error;
    if (!detail.empty())
    {
        client.val["error_description"] = detail;
    }
    client.out_json();
}

// 认证判定整套在 store 里，和 token 端点共用；这里只负责把请求里的字符串交出去。
bool peer_auth_client(httppeer &client, oauth::client_record &rec, std::string &out_id)
{
    std::string auth_header;
    auto        hit = client.header.find("authorization");
    if (hit != client.header.end())
    {
        auth_header = hit->second;
    }
    return oauth::authenticate_client(auth_header,
                                      client.post["client_id"].to_string(),
                                      client.post["client_secret"].to_string(),
                                      rec,
                                      out_id);
}
} // namespace

//@urlpath(null,oauth/revoke)
std::string oauth_revoke(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.set_header("Cache-Control", "no-store");
    client.set_header("X-Frame-Options", "DENY");

    if (!client.is_ssl())
    {
        revoke_error(client, 400, "invalid_request", "revocation endpoint requires HTTPS");
        return "";
    }
    oauth::client_record rec;
    std::string          authed_id;
    if (!peer_auth_client(client, rec, authed_id))
    {
        client.status(401);
        client.set_header("WWW-Authenticate", "Basic realm=\"oauth\"");
        client.val["error"] = "invalid_client";
        client.out_json();
        return "";
    }
    std::string token = client.post["token"].to_string();
    if (token.empty())
    {
        revoke_error(client, 400, "invalid_request", "token is required");
        return "";
    }

    // token_type_hint 只是提示，两种表都查过去（RFC 7009 §2.1）。
    // 撤 refresh：WHERE 里必须带 client_id，否则 A 客户端能撤 B 客户端的同值 token。
    if (oauth::revoke_refresh_by_token(token, authed_id))
    {
        client.status(200);
        return "";
    }
    // 撤还没消费的授权码：库里那一行的"消费"就是撤销，同一条 CAS 语句。
    // 查不到、已经用过、或者属于别的客户端，都落到下面的 200 空体。
    oauth::code_record cr;
    if (oauth::find_code(token, cr) && cr.found && cr.client_id == authed_id && !cr.used)
    {
        oauth::consume_code(token);
    }
    // access_token 是自包含 JWT，没有表可撤——只能等它自己过期（900 秒窗口）。
    // 按 RFC 7009 的口径，这种情况也回 200，不告诉调用方"这个撤不掉"。
    client.status(200);
    return "";
}
} // namespace http
