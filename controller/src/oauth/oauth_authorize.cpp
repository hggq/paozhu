// OAuth 2.1 授权端点。
// GET  /oauth/authorize           校验授权请求，出 scope 同意页
// POST /oauth/authorize/approve   用户点了同意之后发授权码，302 回回调地址
// 校验链的顺序：HTTPS → PKCE → client_id → redirect_uri → 登录态 → 同意页。

#include <string>

#include "cost_define.h"
#include "datetime.h"
#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_store.h"
#include "oauth_authorize.h"
#include "session_id.h"
#include "urlcode.h"

namespace http
{
namespace
{
// PKCE 挑战值的形状闸门：32 字节摘要的无填充 base64url 恒为 43 字符、字母表只有 [A-Za-z0-9-_]。
// 挡在落库之前——库里这一行会被换码时原样带出来比，形状不定的值等于把验算交给请求方。
bool code_challenge_shape_ok(const std::string &challenge)
{
    if (challenge.size() != 43)
    {
        return false;
    }
    for (char c : challenge)
    {
        unsigned char u = (unsigned char)c;
        bool          ok = (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') || (u >= '0' && u <= '9') ||
                  u == '-' || u == '_';
        if (!ok)
        {
            return false;
        }
    }
    return true;
}

// 一次授权请求要带的参数。GET 从 query 取、approve 从表单取，取完走同一条校验链。
struct authorize_request
{
    std::string response_type;
    std::string client_id;
    std::string redirect_uri;
    std::string scope;
    std::string state;
    std::string code_challenge;
    std::string code_challenge_method;
};

enum class authorize_fail
{
    none = 0,
    not_https,
    response_type,
    pkce,
    unknown_client,
    redirect_uri,
    scope,
    not_logged_in
};

authorize_request read_authorize_get(httppeer &client)
{
    authorize_request req;
    req.response_type          = client.get["response_type"].to_string();
    req.client_id              = client.get["client_id"].to_string();
    req.redirect_uri           = client.get["redirect_uri"].to_string();
    req.scope                  = client.get["scope"].to_string();
    req.state                  = client.get["state"].to_string();
    req.code_challenge         = client.get["code_challenge"].to_string();
    req.code_challenge_method  = client.get["code_challenge_method"].to_string();
    return req;
}

authorize_request read_authorize_post(httppeer &client)
{
    authorize_request req;
    req.response_type         = client.post["response_type"].to_string();
    req.client_id             = client.post["client_id"].to_string();
    req.redirect_uri          = client.post["redirect_uri"].to_string();
    req.scope                 = client.post["scope"].to_string();
    req.state                 = client.post["state"].to_string();
    req.code_challenge        = client.post["code_challenge"].to_string();
    req.code_challenge_method = client.post["code_challenge_method"].to_string();
    return req;
}

// 校验链。失败时 out_fail 说明停在哪一步，成功后 rec/matched 是客户端行与白名单里命中的那条回调。
// matched 用**白名单里的那一条**，不是请求参数原文：落库的就是它，换码时再逐字节比回来。
authorize_fail check_authorize(httppeer &client,
                               const authorize_request &req,
                               oauth::client_record &rec,
                               std::string &matched)
{
    if (!client.is_ssl())
    {
        return authorize_fail::not_https;
    }
    // 2.1 删了 implicit，只认 code
    if (req.response_type != "code")
    {
        return authorize_fail::response_type;
    }
    if (req.code_challenge.empty() || req.code_challenge_method != "S256" ||
        !code_challenge_shape_ok(req.code_challenge))
    {
        return authorize_fail::pkce;
    }
    if (req.client_id.empty() || !oauth::load_client(req.client_id, rec) || !rec.found || !rec.isopen)
    {
        return authorize_fail::unknown_client;
    }
    if (!oauth::match_redirect_uri(rec.redirect_uris, req.redirect_uri, matched))
    {
        return authorize_fail::redirect_uri;
    }
    // 申请的每一项都要落在客户端的 scope 白名单里（空白名单＝不设上限）。放在登录检查之前：
    // 一个注定换不到令牌的请求不该先把人送到登录页。
    if (!oauth::scope_within(rec.allowed_scopes, req.scope))
    {
        return authorize_fail::scope;
    }
    // 授权人是 sysuser.adminid；session["userid"] 存的是企业 id，不能当人用
    if (client.session["adminid"].to_int() <= 0)
    {
        return authorize_fail::not_logged_in;
    }
    return authorize_fail::none;
}

// 校验阶段的错误一律不回跳（RFC 6749 §4.1.2.1：client 未认证前不得向它的回调地址发东西），
// 只在本站回一个带状态码的 JSON。请求带来的值一个都不回显。
void authorize_error(httppeer &client, authorize_fail fail)
{
    const char *error  = "invalid_request";
    const char *detail = "invalid request";
    switch (fail)
    {
    case authorize_fail::not_https:
        detail = "authorization endpoint requires HTTPS";
        break;
    case authorize_fail::response_type:
        error = "unsupported_response_type";
        detail = "only response_type=code is supported";
        break;
    case authorize_fail::pkce:
        detail = "code_challenge with code_challenge_method=S256 is required";
        break;
    case authorize_fail::unknown_client:
        error  = "invalid_client";
        detail = "unknown client_id";
        break;
    case authorize_fail::redirect_uri:
        detail = "redirect_uri does not match the registered value";
        break;
    case authorize_fail::scope:
        error  = "invalid_scope";
        detail = "requested scope is not allowed for this client";
        break;
    default:
        break;
    }
    client.status(400);
    client.val["error"] = error;
    client.val["error_description"] = detail;
    client.out_json();
}
} // namespace

//@urlpath(null,oauth/authorize)
std::string oauth_authorize(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    // 这一条链上任何一步的响应都不能被缓存：里面是登录态和授权决定
    client.set_header("X-Frame-Options", "DENY");
    client.set_header("Content-Security-Policy", "frame-ancestors 'none'");
    client.set_header("Cache-Control", "no-store");

    authorize_request req = read_authorize_get(client);
    oauth::client_record rec;
    std::string          matched;
    authorize_fail       fail = check_authorize(client, req, rec, matched);
    if (fail == authorize_fail::not_logged_in)
    {
        // 后台没登录就跳登录页。登录成功后落点是 /admin/main，本端点不会被自动回访，
        // 授权请求要由客户端重发一次（admin 登录没有 redirect 参数）
        client.status(302);
        client.set_header("Location", "/admin/login");
        return "";
    }
    if (fail != authorize_fail::none)
    {
        authorize_error(client, fail);
        return "";
    }

    // 同意页：客户端展示名来自库，其余是校验过的请求值。模板里 as_string() 原样输出，
    // 所以五个自由文本在塞进 val 之前用 html_escape() 转义；
    // code_challenge 由 code_challenge_shape_ok() 钳在 43 个 base64url 字符（那个字母表里没有
    // 任何一个要转义的字符），method 与 submit 是本端点自己写的常量。
    client.val["client_name"]            = html_escape(rec.name);
    client.val["client_id"]              = html_escape(rec.client_id);
    client.val["redirect_uri"]           = html_escape(matched);
    client.val["scope"]                  = html_escape(req.scope);
    client.val["state"]                  = html_escape(req.state);
    client.val["code_challenge"]         = req.code_challenge;
    client.val["code_challenge_method"]  = "S256";
    client.val["submit"]                 = "/oauth/authorize/approve";

    // 同意表单要带一张一次性票据：GET 签发进 session，approve 比对通过才发码。
    // 票据必须跨请求存活，所以这一对路由是全仓唯一调 save_session() 的 OAuth 端点，
    // 写进去的只有这张票据，没有任何身份信息。随机源失败会抛，路由层转成 500——
    // 宁可不渲染同意页，也不发一张比对不了的页面。
    std::string csrf_token       = get_rand_32char();
    client.session["oauth_csrf"] = csrf_token;
    client.save_session();
    client.val["csrf_token"]     = csrf_token;

    client.view("oauth/consent");
    return "";
}

//@urlpath(null,oauth/authorize/approve)
std::string oauth_authorize_approve(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.set_header("X-Frame-Options", "DENY");
    client.set_header("Content-Security-Policy", "frame-ancestors 'none'");
    client.set_header("Cache-Control", "no-store");

    // 表单里的每一个值都当没验过：整条链在这里重跑一遍，不用同意页传上来的状态
    authorize_request req = read_authorize_post(client);
    oauth::client_record rec;
    std::string          matched;
    authorize_fail       fail = check_authorize(client, req, rec, matched);
    if (fail == authorize_fail::not_logged_in)
    {
        client.status(302);
        client.set_header("Location", "/admin/login");
        return "";
    }
    if (fail != authorize_fail::none)
    {
        authorize_error(client, fail);
        return "";
    }

    // CSRF 票据：session 里没有（没经过本端点的同意页）或与表单不一致，就是伪造的提交。
    // 放在整条授权链之后、发码之前：登录态那一步的跳转要先照旧，伪造请求也换不到更细的报错。
    // 比对失败不动 session 里那份——否则一个跨站 POST 就能把用户正开着的同意页作废。
    {
        std::string session_token = client.session["oauth_csrf"].to_string();
        std::string post_token    = client.post["csrf_token"].to_string();
        if (session_token.empty() || post_token != session_token)
        {
            client.status(400);
            client.val["error"]             = "invalid_request";
            client.val["error_description"] = "consent ticket missing or expired";
            client.out_json();
            return "";
        }
        // 一次一用：比对通过立刻作废，同一份表单重复提交会被这一步拒掉
        client.session["oauth_csrf"] = "";
        client.save_session();
    }

    long long user_id = client.session["adminid"].to_int();
    std::string code = oauth::issue_code(rec.client_id,
                                         user_id,
                                         matched,
                                         req.scope,
                                         req.code_challenge,
                                         (long long)(timeid() + CONST_OAUTH_CODE_TTL));
    if (code.empty())
    {
        // 随机源失败或 INSERT 无效：不降级、不发半个码
        client.status(500);
        client.val["error"] = "server_error";
        client.out_json();
        return "";
    }

    // 真 302，不是 goto_url()（那是 <meta refresh> 的 HTML 页）
    std::string location = matched;
    location += (matched.find('?') == std::string::npos ? "?" : "&");
    location += "code=" + url_encode(code.data(), (int)code.size());
    if (!req.state.empty())
    {
        // state 是请求方任意可控的串，拼进 Location 前必须编码
        location += "&state=" + url_encode(req.state.data(), (int)req.state.size());
    }
    client.status(302);
    client.set_header("Location", location);
    return "";
}
} // namespace http
