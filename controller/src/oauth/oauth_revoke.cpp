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
#include "debug_log.h"

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
    // access_token 是自包含 JWT，库里没有对应行，只能记进撤销名单（见 blacklist_access_jti）。
    // 形状判定用 looks_like_jwt，不看 token_type_hint——那是调用方可控的字符串；
    // 本站发的 opaque 令牌是 43 个 base64url 字符、不含 '.'，两种形状互斥且穷尽。
    // 名单写不进去（本构建没编 Redis、段名不存在、连接没建成）时这枚令牌照旧活到过期：
    // 响应形状不变（RFC 7009 不回"撤不掉"），但把这件事记进日志，免得运维以为撤销生效了。
    if (oauth::looks_like_jwt(token))
    {
        oauth::signing_material keys = oauth::signing_material_cached();
        std::string             payload_json;
        std::string             why;
        // 只用公钥验签，私钥那一格是 signing_material 的形态要求（两个文件一起读，任一读不出
        // 都不算就绪）；jwks 与 introspect 也是同一个形状。私钥不可读会让撤销静默无效，
        // 那条日志就是这件事唯一的可见处。
        if (keys.ready &&
            oauth::verify_access_token(keys.pubkey_pem, token, CONST_OAUTH_JWT_LEEWAY, payload_json, why))
        {
            obj_val claims;
            try
            {
                claims.from_json(payload_json);
            }
            catch (const std::exception &)
            {
                claims.clear();
            }
            if (claims.get_type() == obj_type::OBJECT)
            {
                std::string jti = claims["jti"].to_string();
                // 只撤属于这个已认证客户端的令牌，和上面撤 refresh 那条 WHERE client_id 同口径：
                // 少了它，A 客户端能拿 B 的 jti 去占名单。
                // exp 是数字 claim，只能按 64 位整数读（json_util::get_string 读不到带引号以外的
                // 值，int 版又是 32 位），名单键的 TTL 就是它剩下的有效期。
                if (!jti.empty() && claims["aud"].to_string() == authed_id &&
                    !oauth::blacklist_access_jti(jti, claims["exp"].to_int()))
                {
                    LOG_ERROR << "[oauth] revoke: access token not recorded in the revocation list" << LOG_END;
                }
            }
        }
    }
    client.status(200);
    return "";
}
} // namespace http
