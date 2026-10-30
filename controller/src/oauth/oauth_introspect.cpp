// OAuth 2.1 / RFC 7662 令牌自省端点：POST /oauth/introspect。
// 必须过客户端认证，而且只放保密客户端进来——公开客户端没有口令，
// authenticate_client() 对"只带 client_id 的提交"是返回 true 的，
// 少了这道闸，任何注册过的 client_id 都能拿这个端点当"某个令牌活着吗、是谁的"的查询处。
//
// 只回 200 {"active":false}，从不回 403/404：查无此令牌、令牌属于别的客户端、
// 签名不对、已过期，这四种在调用方看来必须是同一格答案，
// 否则这个端点就成了枚举器（跨 aud 的判定在 store 的 introspect_token 里）。
// token_type_hint 是调用方可控的字符串，所以它不参与路由：令牌形状自己可判
// （opaque 令牌恒 43 字符且不含 '.'，JWT 恰含两枚 '.'），见 oauth_store.h 的 looks_like_jwt()。

#include <string>

#include "cost_define.h"
#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_config.h"
#include "oauth/oauth_store.h"
#include "oauth_introspect.h"

namespace http
{
namespace
{
void introspect_error(httppeer &client, unsigned int status, const std::string &error, const std::string &detail)
{
    client.status(status);
    client.val["error"] = error;
    if (!detail.empty())
    {
        client.val["error_description"] = detail;
    }
    client.out_json();
}

void introspect_denied(httppeer &client)
{
    client.status(401);
    client.set_header("WWW-Authenticate", "Basic realm=\"oauth\"");
    client.val["error"] = "invalid_client";
    client.out_json();
}

// 认证判定整套在 store 里，和 token / revoke 两个端点共用；这里只把请求里的字符串交出去。
bool peer_auth_client(httppeer &client, oauth::client_record &rec, std::string &out_id)
{
    std::string auth_header;
    auto hit = client.header.find("authorization");
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
}// namespace

//@urlpath(null,oauth/introspect)
std::string oauth_introspect(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.set_header("Cache-Control", "no-store");
    client.set_header("X-Frame-Options", "DENY");

    if (!client.is_ssl())
    {
        introspect_error(client, 400, "invalid_request", "introspection endpoint requires HTTPS");
        return "";
    }
    oauth::client_record rec;
    std::string authed_id;
    if (!peer_auth_client(client, rec, authed_id))
    {
        introspect_denied(client);
        return "";
    }
    if (!rec.confidential)
    {
        // 与认证失败同一句话：这里多说一个字，就等于告诉调用方"你的 client_id 注册过、
        // 只是类型不对"，而类型是注册方自己填的，猜中与否不该由这个端点透露。
        introspect_denied(client);
        return "";
    }

    std::string token = client.post["token"].to_string();
    if (token.empty())
    {
        introspect_error(client, 400, "invalid_request", "token is required");
        return "";
    }
    if (token.size() > CONST_OAUTH_TOKEN_MAX_LEN)
    {
        // 先钳长度再验签/查库：这一格的答案与"查无此令牌"同形，所以它不泄露"你猜的长度对不对"。
        client.status(200);
        client.val["active"] = false;
        client.out_json();
        return "";
    }

    oauth::oauth_settings   settings = oauth::oauth_settings_cached();
    oauth::signing_material keys     = oauth::signing_material_cached();
    if (!keys.ready)
    {
        // 公钥读不出来是"判不了"，不是"令牌不活"：回 500，别让运维把它当成客户端问题，
        // 也别让调用方拿一个 active:false 去撤一个其实有效的令牌。
        DEBUG_LOG("[oauth] introspect: public key not loaded (conf/oauth.conf)");
        introspect_error(client, 500, "server_error", "");
        return "";
    }
    if (settings.issuer.empty())
    {
        DEBUG_LOG("[oauth] introspect: issuer empty in conf/oauth.conf");
        introspect_error(client, 500, "server_error", "");
        return "";
    }

    oauth::introspect_record ir;
    if (!oauth::introspect_token(authed_id, keys.pubkey_pem, settings.issuer, CONST_OAUTH_JWT_LEEWAY, token, ir))
    {
        DEBUG_LOG("[oauth] introspect: cannot judge token (store returned false)");
        introspect_error(client, 500, "server_error", "");
        return "";
    }

    client.status(200);
    client.val["active"] = ir.active;
    if (ir.active)
    {
        // RFC 7662 §2.2：active:true 时可以带上元数据；缺的那几项直接不写键，
        // 不写 null 也不编值——opaque 那一支没有 iss/aud/jti/iat（oauth_refresh 没有签发时刻列）。
        // username 刻意不公布：它把"令牌活着"扩成"令牌活着、而且这个人在这个系统里叫什么"。
        client.val["token_type"] = ir.token_type;
        client.val["client_id"]  = ir.client_id;
        if (!ir.iss.empty())
        {
            client.val["iss"] = ir.iss;
        }
        if (!ir.sub.empty())
        {
            client.val["sub"] = ir.sub;
        }
        if (!ir.aud.empty())
        {
            client.val["aud"] = ir.aud;
        }
        if (!ir.scope.empty())
        {
            client.val["scope"] = ir.scope;
        }
        if (!ir.jti.empty())
        {
            client.val["jti"] = ir.jti;
        }
        if (ir.exp != 0)
        {
            client.val["exp"] = ir.exp;
        }
        if (ir.iat != 0)
        {
            client.val["iat"] = ir.iat;
        }
    }
    client.out_json();
    return "";
}
}// namespace http
