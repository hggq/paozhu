// OAuth 客户端完整测试 / demo 页面。
// 请求流程（client_credentials → authorize → approve → code exchange → refresh rotation → replay → introspect → revoke）
// 全部用 C++ httpclient 串成一组自包含的端点。依赖方默认填本地 paozhu oauth 服务器
// （https://www.hggq.com），client_id / client_secret / scope 走表单输入，方便换别的 OAuth 2.1 服务器直接复用。
/*
 *  更换相应的ouath2地址
 *    ep.authorize_url  = "https://www.hggq.com/oauth/authorize";
 *    ep.token_url      = "https://www.hggq.com/oauth/token";
 *    ep.introspect_url = "https://www.hggq.com/oauth/introspect";
 *    ep.revoke_url     = "https://www.hggq.com/oauth/revoke";
 */
#include <string>

#include "func.h"
#include "httppeer.h"
#include "oauth_client/oauth_client.h"
#include "oauth_client_demo.h"
#include "session_id.h"

namespace http
{
namespace
{

// 端点默认值：paozhu 自搭的 OAuth 服务器（本项目 controller/src/oauth/）。
// 想切 GitHub / Google / 别的，在表单里改 URL 就行；
// GitHub 特别注意：token 端点不认 Basic，必须把 client_id/secret 放 POST body——
// 在 OAuth Server Endpoints 那排下面有个 "client auth" 开关可以切。
oauth_client::oauth_endpoints default_endpoints()
{
    oauth_client::oauth_endpoints ep;
    ep.authorize_url     = "https://www.hggq.com/oauth/authorize";
    ep.token_url         = "https://www.hggq.com/oauth/token";
    ep.introspect_url    = "https://www.hggq.com/oauth/introspect";
    ep.revoke_url        = "https://www.hggq.com/oauth/revoke";
    ep.timeout_seconds   = 10;
    ep.prefer_basic_auth = true;
    return ep;
}

// paozhu OAuth server 测试客户端。
// 想测 GitHub OAuth App 把下面两行换成 Ov23li98gp47ujnemKCA / 32b1581c8a6ff1c70fe1ca76fd75fabb4c4b369e
// 同时把 authorize_url/token_url 改成 github.com/login/oauth/{authorize,access_token}、prefer_basic_auth=false。
constexpr const char *DEFAULT_CLIENT_ID     = "test_client";
constexpr const char *DEFAULT_CLIENT_SECRET = "test-secret-abc123-xyz789";

// 把 token_answer / introspect_answer 编成通用 JSON 输出。
void emit_token_json(httppeer &client, const oauth_client::token_answer &t, const char *grant_label)
{
    client.val["grant"]              = grant_label;
    client.val["ok"]                 = t.ok ? 1 : 0;
    client.val["http_status"]        = t.http_status;
    client.val["access_token"]       = t.access_token;
    client.val["refresh_token"]      = t.refresh_token;
    client.val["scope"]              = t.scope;
    client.val["token_type"]         = t.token_type;
    client.val["access_expires_in"]  = t.access_expires_in;
    client.val["refresh_expires_in"] = t.refresh_expires_in;
    client.val["error"]              = t.error;
}

void emit_introspect_json(httppeer &client, const oauth_client::introspect_answer &r)
{
    client.val["ok"]          = r.ok ? 1 : 0;
    client.val["active"]      = r.active ? 1 : 0;
    client.val["http_status"] = r.http_status;
    client.val["error"]       = r.error;
    if (!r.active)
    {
        client.out_json();
        return;
    }
    client.val["token_type"] = r.token_type;
    client.val["client_id"]  = r.client_id;
    client.val["iss"]        = r.iss;
    client.val["sub"]        = r.sub;
    client.val["aud"]        = r.aud;
    client.val["scope"]      = r.scope;
    client.val["jti"]        = r.jti;
    client.val["exp"]        = r.exp;
    client.val["iat"]        = r.iat;
}

void emit_revoke_json(httppeer &client, oauth_client::revoke_result r, unsigned int status)
{
    const char *label = "unknown";
    switch (r)
    {
    case oauth_client::revoke_result::ok: label = "ok"; break;
    case oauth_client::revoke_result::auth_failed: label = "auth_failed"; break;
    case oauth_client::revoke_result::transport: label = "transport"; break;
    }
    client.val["result"]      = label;
    client.val["http_status"] = status;
}
}// namespace

// ---------------------------------------------------------------------------
// GET /oauth_client/page
// 渲染一个表单，用户填 client_id / client_secret / endpoints / scope，然后点各步按钮。
// 这里也提供一组默认值（paozhu 自搭的 test_client），直接能跑。
// ---------------------------------------------------------------------------
//@urlpath(null,oauth_client/page)
std::string oauth_client_page(std::shared_ptr<httppeer> peer)
{
    httppeer &client             = peer->get_peer();
    auto ep                      = default_endpoints();
    client.val["authorize_url"]  = ep.authorize_url;
    client.val["token_url"]      = ep.token_url;
    client.val["introspect_url"] = ep.introspect_url;
    client.val["revoke_url"]     = ep.revoke_url;
    client.val["client_id"]      = DEFAULT_CLIENT_ID;
    client.val["client_secret"]  = DEFAULT_CLIENT_SECRET;
    client.val["scope"]          = "repo user";
    client.view("oauth_client/page");
    return "";
}

// ---------------------------------------------------------------------------
// POST /oauth_client/client_credentials
// 走 client_credentials grant，只需要 client_id + secret + token_url + scope。
// ---------------------------------------------------------------------------
//@urlpath(null,oauth_client/client_credentials)
std::string oauth_client_cc(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto ep          = default_endpoints();
    // 表单可以覆盖 endpoints（debug 不同服务器时用）
    if (client.post.isset("token_url"))
        ep.token_url = client.post["token_url"].to_string();
    if (client.post.isset("introspect_url"))
        ep.introspect_url = client.post["introspect_url"].to_string();

    std::string client_id     = client.post["client_id"].to_string();
    std::string client_secret = client.post["client_secret"].to_string();
    std::string scope         = client.post["scope"].to_string();

    oauth_client::token_answer out;
    oauth_client::client_credentials(ep, client_id, client_secret, scope, out);
    client.status(out.ok ? 200 : (out.http_status ? out.http_status : 500));
    emit_token_json(client, out, "client_credentials");
    client.out_json();
    return "";
}

// ---------------------------------------------------------------------------
// POST /oauth_client/build_authorize
// 生成 authorize URL + state + code_challenge + code_verifier（后两者放 session 里）。
// 用户拿到 URL 之后手动点（浏览器走完同意流程），再用拿到的 code 调下一步。
// ---------------------------------------------------------------------------
//@urlpath(null,oauth_client/build_authorize)
std::string oauth_client_build_auth(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto ep          = default_endpoints();
    if (client.post.isset("authorize_url"))
        ep.authorize_url = client.post["authorize_url"].to_string();

    std::string client_id    = client.post["client_id"].to_string();
    std::string redirect_uri = client.post["redirect_uri"].to_string();
    std::string scope        = client.post["scope"].to_string();

    std::string verifier  = oauth_client::make_code_verifier();
    std::string challenge = oauth_client::pkce_challenge(verifier);
    std::string state     = get_rand_32char();

    std::string url;
    bool ok = oauth_client::build_authorize_url(ep, client_id, redirect_uri, scope, state, challenge, url);

    if (ok)
    {
        // verifier + state 只放 session，不回显给用户
        client.session["oc_verifier"] = verifier;
        client.session["oc_state"]    = state;
        client.save_session();
    }

    client.val["ok"]             = ok ? 1 : 0;
    client.val["authorize_url"]  = ep.authorize_url;
    client.val["url"]            = url;
    client.val["state"]          = state;
    client.val["code_challenge"] = challenge;
    client.val["code_verifier"]  = verifier;// 方便 demo 复制粘贴，实际生产别回显
    client.val["error"]          = ok ? "" : "missing required fields (client_id/redirect_uri/scope)";
    client.status(200);
    client.out_json();
    return "";
}

// ---------------------------------------------------------------------------
// POST /oauth_client/exchange_code
// 把 authorize 回调拿到的 code 换成 access + refresh。code_verifier 从上一步的 session 里取。
// ---------------------------------------------------------------------------
//@urlpath(null,oauth_client/exchange_code)
std::string oauth_client_exchange(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto ep          = default_endpoints();
    if (client.post.isset("token_url"))
        ep.token_url = client.post["token_url"].to_string();

    std::string client_id     = client.post["client_id"].to_string();
    std::string client_secret = client.post["client_secret"].to_string();
    std::string code          = client.post["code"].to_string();
    std::string redirect_uri  = client.post["redirect_uri"].to_string();
    std::string verifier      = client.post["code_verifier"].to_string();// 生产应该从 session 取

    oauth_client::token_answer out;
    oauth_client::exchange_code(ep, client_id, client_secret, code, redirect_uri, verifier, out);
    client.status(out.ok ? 200 : (out.http_status ? out.http_status : 500));
    emit_token_json(client, out, "authorization_code");
    client.out_json();
    return "";
}

// ---------------------------------------------------------------------------
// POST /oauth_client/refresh
// ---------------------------------------------------------------------------
//@urlpath(null,oauth_client/refresh)
std::string oauth_client_refresh(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto ep          = default_endpoints();
    if (client.post.isset("token_url"))
        ep.token_url = client.post["token_url"].to_string();

    std::string client_id     = client.post["client_id"].to_string();
    std::string client_secret = client.post["client_secret"].to_string();
    std::string refresh_token = client.post["refresh_token"].to_string();

    oauth_client::token_answer out;
    oauth_client::refresh_access(ep, client_id, client_secret, refresh_token, out);
    client.status(out.ok ? 200 : (out.http_status ? out.http_status : 500));
    emit_token_json(client, out, "refresh_token");
    client.out_json();
    return "";
}

// ---------------------------------------------------------------------------
// POST /oauth_client/introspect
// ---------------------------------------------------------------------------
//@urlpath(null,oauth_client/introspect)
std::string oauth_client_introspect(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto ep          = default_endpoints();
    if (client.post.isset("introspect_url"))
        ep.introspect_url = client.post["introspect_url"].to_string();

    std::string client_id       = client.post["client_id"].to_string();
    std::string client_secret   = client.post["client_secret"].to_string();
    std::string token           = client.post["token"].to_string();
    std::string token_type_hint = client.post["token_type_hint"].to_string();

    oauth_client::introspect_answer out;
    oauth_client::do_introspect(ep, client_id, client_secret, token, token_type_hint, out);
    client.status(out.ok ? 200 : 500);
    emit_introspect_json(client, out);
    client.out_json();
    return "";
}

// ---------------------------------------------------------------------------
// POST /oauth_client/revoke
// ---------------------------------------------------------------------------
//@urlpath(null,oauth_client/revoke)
std::string oauth_client_revoke(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    auto ep          = default_endpoints();
    if (client.post.isset("revoke_url"))
        ep.revoke_url = client.post["revoke_url"].to_string();

    std::string client_id       = client.post["client_id"].to_string();
    std::string client_secret   = client.post["client_secret"].to_string();
    std::string token           = client.post["token"].to_string();
    std::string token_type_hint = client.post["token_type_hint"].to_string();

    unsigned int status = 0;
    auto r              = oauth_client::do_revoke(ep, client_id, client_secret, token, token_type_hint, status);
    client.status(200);
    emit_revoke_json(client, r, status);
    client.out_json();
    return "";
}

}// namespace http
