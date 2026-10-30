// OAuth 2.1 token 端点：POST /oauth/token，按 grant_type 分发三个分支。
// 闸门顺序：客户端认证 → 令牌查表 → PKCE → CAS 消费 → 签发。
// 三张表的读写全部走 libs/oauth/oauth_store.h，控制器不碰 orm。

#include <string>

#include "cost_define.h"
#include "datetime.h"
#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_config.h"
#include "oauth/oauth_store.h"
#include "oauth_token.h"

namespace http
{
namespace
{
// 错误响应。框架默认 200，out_json() 不改状态码，所以每一条都必须显式 status()。
// detail 是给调用方的一句话，只从固定文案里取，不回显请求带来的值。
void token_error(httppeer &client, unsigned int status, const std::string &error, const std::string &detail)
{
    client.status(status);
    client.val["error"] = error;
    if (!detail.empty())
    {
        client.val["error_description"] = detail;
    }
    client.out_json();
}

// 从请求里取出认证需要的三个字符串。判定整套在 store 里（token 与 revoke 共用，
// 两边各写一遍迟早漂开——漂开的那一处就是一个能用弱形状过的端点）。
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

// 认证失败的固定形状：401 + WWW-Authenticate（RFC 6749 §2.3.1 的客户端认证失败）。
void client_auth_failed(httppeer &client)
{
    client.status(401);
    client.set_header("WWW-Authenticate", "Basic realm=\"oauth\"");
    client.val["error"] = "invalid_client";
    client.out_json();
}

// 这个客户端登记时是否开了这门 grant（空串＝三种全开）。不开就是 unauthorized_client，
// 与"这个客户端根本不许用这种 grant"同义，不是参数错误。
bool grant_allowed(httppeer &client, const oauth::client_record &rec, const std::string &grant)
{
    if (oauth::grant_within(rec.grant_types, grant))
    {
        return true;
    }
    token_error(client, 400, "unauthorized_client", "grant_type not allowed for this client");
    return false;
}

// 取常驻的 conf 读数与私钥正文。任一步拿不到回 false——签发端点据此报 500，
// 不降级到对称算法，也不签半个 token 出去。
bool signing_ready(oauth::oauth_settings &settings, std::string &privkey_pem)
{
    settings = oauth::oauth_settings_cached();
    if (settings.issuer.empty())
    {
        return false;
    }
    oauth::signing_material keys = oauth::signing_material_cached();
    if (!keys.ready)
    {
        return false;
    }
    privkey_pem = keys.privkey_pem;
    return true;
}

// 签一枚 access_token。sub 用 JSON 字符串（RFC 7519 §4.1.2 的 sub 是 StringOrURI），
// aud 是 client_id 字符串——签发与 Bearer 校验两头用的是同一个形状。
std::string make_access_token(const oauth::oauth_settings &settings,
                              const std::string &privkey_pem,
                              const std::string &aud,
                              const std::string &sub,
                              const std::string &scope)
{
    std::string jti = oauth::random_token();
    if (jti.empty())
    {
        return "";
    }
    long long now = (long long)timeid();
    obj_val payload;
    payload.set_object();
    payload["iss"]   = settings.issuer;
    payload["sub"]   = sub;
    payload["aud"]   = aud;
    payload["scope"] = scope;
    payload["exp"]   = now + (long long)CONST_OAUTH_ACCESS_TTL;
    payload["iat"]   = now;
    payload["jti"]   = jti;
    return oauth::sign_access_token(privkey_pem, payload.to_json(), settings.kid);
}
}// namespace

//@urlpath(null,oauth/token)
std::string oauth_token(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // token 与 refresh 都是不能缓存的响应（RFC 6749 §5.3）
    client.set_header("Cache-Control", "no-store");
    client.set_header("Pragma", "no-cache");
    client.set_header("X-Frame-Options", "DENY");

    if (!client.is_ssl())
    {
        token_error(client, 400, "invalid_request", "token endpoint requires HTTPS");
        return "";
    }
    std::string grant_type = client.post["grant_type"].to_string();

    // ------------------------------------------------------------------
    // grant_type=authorization_code
    // ------------------------------------------------------------------
    if (grant_type == "authorization_code")
    {
        oauth::client_record rec;
        std::string authed_id;
        if (!peer_auth_client(client, rec, authed_id))
        {
            client_auth_failed(client);
            return "";
        }
        if (!grant_allowed(client, rec, "authorization_code"))
        {
            return "";
        }
        std::string code = client.post["code"].to_string();
        oauth::code_record cr;
        if (code.empty() || !oauth::find_code(code, cr) || !cr.found)
        {
            token_error(client, 400, "invalid_grant", "unknown authorization code");
            return "";
        }
        // used 已经是 1：这一条就是重放 ⇒ 认定码泄露，把这一族 refresh 整族撤掉。
        // family_id 是 0 时什么都不撤：那次换码要么还没回填、要么回填失败，
        // 拿 0 当族号去撤会命中所有未回填的族根。对外仍只回 invalid_grant，不撤成 200。
        if (cr.used)
        {
            if (cr.family_id != 0)
            {
                oauth::revoke_refresh_family(cr.family_id);
            }
            token_error(client, 400, "invalid_grant", "authorization code already used");
            return "";
        }
        if (cr.expires_at <= (long long)timeid())
        {
            token_error(client, 400, "invalid_grant", "authorization code expired");
            return "";
        }
        if (cr.client_id != authed_id)
        {
            token_error(client, 400, "invalid_grant", "authorization code issued to another client");
            return "";
        }
        // RFC 6749 §4.1.3：换码必须带回与授权时逐字节一致的回调地址。授权侧 match_redirect_uri
        // 必过，写进码行的那一格恒为非空，所以缺失与不匹配同形拒绝，不留"不带参数就放行"的分支。
        std::string submitted_redirect = client.post["redirect_uri"].to_string();
        if (submitted_redirect.empty() || submitted_redirect != cr.redirect_uri)
        {
            token_error(client, 400, "invalid_grant", "redirect_uri mismatch");
            return "";
        }
        // PKCE 验算放在消费之前：验不过就不该把这条码烧掉
        if (!oauth::pkce_s256_ok(client.post["code_verifier"].to_string(), cr.code_challenge))
        {
            token_error(client, 400, "invalid_grant", "PKCE verification failed");
            return "";
        }
        // CAS：UPDATE ... WHERE code_hash=? AND used=0，抢不到说明并发重放。
        // 这一支不撤族，且不是漏写：走到这里说明上面那次 find_code 读到的是 used=0，
        // 而族根只在 used=1 之后才绑得上（bind_code_family 的 WHERE 带 used=1），
        // 所以同一次读数里 cr.family_id 必然是 0——真正撤到族的是上面那条重放分支。
        if (!oauth::consume_code(code))
        {
            token_error(client, 400, "invalid_grant", "authorization code already used");
            return "";
        }

        oauth::oauth_settings settings;
        std::string privkey_pem;
        if (!signing_ready(settings, privkey_pem))
        {
            token_error(client, 500, "server_error", "signing key not available");
            return "";
        }
        std::string access =
            make_access_token(settings, privkey_pem, authed_id, std::to_string(cr.user_id), cr.scope);
        if (access.empty())
        {
            token_error(client, 500, "server_error", "token signing failed");
            return "";
        }
        // 本次授权的第一行 refresh：family_id 传 0，由 store 回填成它自己的 id
        unsigned int refresh_row_id = 0;
        std::string refresh =
            oauth::issue_refresh(0, authed_id, cr.user_id, cr.scope, (long long)(timeid() + settings.refresh_ttl), refresh_row_id);
        if (refresh.empty())
        {
            token_error(client, 500, "server_error", "refresh token issuing failed");
            return "";
        }
        // 把这一族的族根 id 绑回码行：这是"码被重放 ⇒ 撤族"唯一的连线。
        // 绑不上只说明下一次重放撤不到族，不值得把已经签出来的令牌作废掉，所以不判返回值。
        oauth::bind_code_family(code, refresh_row_id);
        client.status(200);
        client.val["access_token"]  = access;
        client.val["token_type"]    = "Bearer";
        client.val["expires_in"]    = (long long)CONST_OAUTH_ACCESS_TTL;
        client.val["refresh_token"] = refresh;
        client.val["scope"]         = cr.scope;
        client.out_json();
        return "";
    }

    // ------------------------------------------------------------------
    // grant_type=refresh_token：rotation + 重放检测
    // ------------------------------------------------------------------
    if (grant_type == "refresh_token")
    {
        oauth::client_record rec;
        std::string authed_id;
        if (!peer_auth_client(client, rec, authed_id))
        {
            client_auth_failed(client);
            return "";
        }
        if (!grant_allowed(client, rec, "refresh_token"))
        {
            return "";
        }
        std::string rt = client.post["refresh_token"].to_string();
        oauth::refresh_record rr;
        if (rt.empty() || !oauth::find_refresh(rt, rr) || !rr.found)
        {
            token_error(client, 400, "invalid_grant", "unknown refresh token");
            return "";
        }
        if (rr.client_id != authed_id)
        {
            token_error(client, 400, "invalid_grant", "refresh token issued to another client");
            return "";
        }
        // revoked 判在过期判之前：轮换出去的旧行即使已经过期，它那一族的后续行还是活的，
        // 这条重放信号不能因为"旧行到期了"就丢掉
        unsigned int family = rr.family_id != 0 ? rr.family_id : rr.row_id;
        if (rr.revoked)
        {
            oauth::revoke_refresh_family(family);
            token_error(client, 400, "invalid_grant", "refresh token revoked");
            return "";
        }
        if (rr.expires_at <= (long long)timeid())
        {
            token_error(client, 400, "invalid_grant", "refresh token expired");
            return "";
        }
        // 先取签发材料，再烧旧行：私钥读不出来时这一支应该原样退回，
        // 而不是把用户的 refresh 消费掉之后回一个 500。
        // refresh 的有效期也从这里拿（conf/oauth.conf 的 refresh_ttl）。
        oauth::oauth_settings settings;
        std::string privkey_pem;
        if (!signing_ready(settings, privkey_pem))
        {
            token_error(client, 500, "server_error", "signing key not available");
            return "";
        }
        // 旧行的 CAS 抢不到 = 这一次拿的是同一行的第二次请求，同样是重放信号
        if (!oauth::revoke_refresh_row(rr.row_id))
        {
            oauth::revoke_refresh_family(family);
            token_error(client, 400, "invalid_grant", "refresh token replayed");
            return "";
        }
        // 新 refresh 的 scope 恒等于原 scope，轮换不许缩小它——
        // 请求里带的 scope 这一分支一律忽略
        std::string new_refresh;
        {
            unsigned int ignore_row_id = 0;
            new_refresh                = oauth::issue_refresh(family, authed_id, rr.user_id, rr.scope, (long long)(timeid() + settings.refresh_ttl), ignore_row_id);
        }
        if (new_refresh.empty())
        {
            token_error(client, 500, "server_error", "refresh token issuing failed");
            return "";
        }
        std::string access =
            make_access_token(settings, privkey_pem, authed_id, std::to_string(rr.user_id), rr.scope);
        if (access.empty())
        {
            token_error(client, 500, "server_error", "token signing failed");
            return "";
        }
        client.status(200);
        client.val["access_token"]  = access;
        client.val["token_type"]    = "Bearer";
        client.val["expires_in"]    = (long long)CONST_OAUTH_ACCESS_TTL;
        client.val["refresh_token"] = new_refresh;
        client.val["scope"]         = rr.scope;
        client.out_json();
        return "";
    }

    // ------------------------------------------------------------------
    // grant_type=client_credentials：没有授权人，也不发 refresh
    // ------------------------------------------------------------------
    if (grant_type == "client_credentials")
    {
        oauth::client_record rec;
        std::string authed_id;
        if (!peer_auth_client(client, rec, authed_id))
        {
            client_auth_failed(client);
            return "";
        }
        if (!grant_allowed(client, rec, "client_credentials"))
        {
            return "";
        }
        if (!rec.confidential)
        {
            token_error(client, 400, "unauthorized_client", "public clients cannot use client_credentials");
            return "";
        }
        // 申请的每一项都要落在客户端的 scope 白名单里（空白名单＝不设上限）。
        // 没带 scope 就签一枚空 scope 的令牌，不替客户端补默认值。
        std::string scope = client.post["scope"].to_string();
        if (!oauth::scope_within(rec.allowed_scopes, scope))
        {
            token_error(client, 400, "invalid_scope", "requested scope is not allowed for this client");
            return "";
        }
        oauth::oauth_settings settings;
        std::string privkey_pem;
        if (!signing_ready(settings, privkey_pem))
        {
            token_error(client, 500, "server_error", "signing key not available");
            return "";
        }
        std::string access = make_access_token(settings, privkey_pem, authed_id, authed_id, scope);
        if (access.empty())
        {
            token_error(client, 500, "server_error", "token signing failed");
            return "";
        }
        client.status(200);
        client.val["access_token"] = access;
        client.val["token_type"]   = "Bearer";
        client.val["expires_in"]   = (long long)CONST_OAUTH_ACCESS_TTL;
        client.val["scope"]        = scope;
        client.out_json();
        return "";
    }

    // password 与 implicit 两个 grant 在 2.1 里已经删掉，这里不存在对应分支
    token_error(client, 400, "unsupported_grant_type", "grant_type must be authorization_code, refresh_token or client_credentials");
    return "";
}
}// namespace http
