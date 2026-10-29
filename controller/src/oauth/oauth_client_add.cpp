// 客户端注册端点：POST /oauth/client/add，挂在后台 JSON 登录过滤器后面，不是协议端点。
// 本授权服务器只服务自家后台，所以 client_id 和回调白名单由注册方指定，
// 口令由服务端生成、只在这次响应里出现一次，库里只留 hex(SHA-256(口令))。

#include <string>

#include "cost_define.h"
#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_store.h"
#include "oauth_client_add.h"

namespace http
{
namespace
{
void add_client_fail(httppeer &client, const std::string &msg)
{
    client.status(400);
    client.val["code"] = 0;
    client.val["msg"]  = msg;
    client.out_json();
}
} // namespace

//@urlpath(admin_isloginjson,oauth/client/add)
std::string oauth_client_add(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.set_header("Cache-Control", "no-store");
    client.set_header("X-Frame-Options", "DENY");

    // 明文口会把这次唯一一次出现的 client_secret 直接回显出去，与 authorize/token/revoke/introspect 同闸。
    if (!client.is_ssl())
    {
        add_client_fail(client, "client registration requires HTTPS");
        return "";
    }

    std::string client_id      = str_trim(client.post["client_id"].to_string());
    std::string name           = str_trim(client.post["name"].to_string());
    std::string redirect_uris  = str_trim(client.post["redirect_uris"].to_string());
    // 两列策略串：空格分隔的 scope 白名单、逗号分隔的 grant 白名单，空串都表示不设上限
    std::string allowed_scopes = str_trim(client.post["allowed_scopes"].to_string());
    std::string grant_types    = str_trim(client.post["grant_types"].to_string());
    if (client_id.empty())
    {
        add_client_fail(client, "client_id is required");
        return "";
    }
    if (client_id.size() > CONST_OAUTH_CLIENT_ID_MAX_LEN)
    {
        add_client_fail(client, "client_id too long");
        return "";
    }
    if (name.size() > CONST_OAUTH_CLIENT_NAME_MAX_LEN)
    {
        add_client_fail(client, "name too long");
        return "";
    }
    // 超长在入口拒，不靠库的静默截断：截短的白名单少几项，表现成"合法 scope 换不到令牌"
    if (allowed_scopes.size() > CONST_OAUTH_CLIENT_SCOPES_MAX_LEN)
    {
        add_client_fail(client, "allowed_scopes too long");
        return "";
    }
    if (grant_types.size() > CONST_OAUTH_CLIENT_GRANTS_MAX_LEN)
    {
        add_client_fail(client, "grant_types too long");
        return "";
    }
    if (redirect_uris.empty())
    {
        add_client_fail(client, "redirect_uris is required");
        return "";
    }
    // 空项存进去永远不会被 match_redirect_uri 命中，白名单里留着它只是噪音
    for (const std::string &one : mb_split(",", redirect_uris))
    {
        if (str_trim(one).empty())
        {
            add_client_fail(client, "redirect_uris has an empty item");
            return "";
        }
    }
    // scheme 闸门：明文只放行环回字面量，其余必须 https。
    // 这一句管的是"注册方自己给出的地址"，不是请求方可控的输入。
    for (const std::string &one : mb_split(",", redirect_uris))
    {
        if (!oauth::redirect_uri_scheme_ok(str_trim(one)))
        {
            add_client_fail(client, "redirect_uri must be https:// (http:// only for 127.0.0.1 / [::1])");
            return "";
        }
    }
    // 只有显式写 is_confidential=0 才是公开客户端；字段缺失按保密客户端注册
    bool confidential = client.post["is_confidential"].to_string() != "0";
    // 同一个极性：只有显式写 isopen=0 才是停用，字段缺失按启用注册
    bool isopen = client.post["isopen"].to_string() != "0";

    std::string secret;
    if (!oauth::add_client(client_id, name, redirect_uris, confidential,
                           allowed_scopes, grant_types, isopen, secret))
    {
        // 长度和空值上面都查过了，走到这里只剩两种可能：
        // client_id 已被占用，或者语句本身没写进去（库连不上、唯一键冲突之外的报错）
        client.status(400);
        client.val["code"] = 0;
        client.val["msg"]  = "register failed: client_id exists or db error";
        client.out_json();
        return "";
    }
    client.val["code"]            = 1;
    client.val["client_id"]       = client_id;
    client.val["is_confidential"] = confidential ? 1 : 0;
    // 明文口令只有这一次机会；公开客户端这里是空串
    client.val["client_secret"] = secret;
    client.out_json();
    return "";
}
} // namespace http
