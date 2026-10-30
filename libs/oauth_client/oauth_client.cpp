// OAuth 2.x / 2.1 通用客户端库的实现。
// 核心模式从 libs/github/github_oauth.cpp 直接复用（httpclient 封装、PKCE、JSON 解析），
// 抽出来做一份纯通用版本——这里是客户端，github 里那份是特定于 GitHub 端点的派生。
#include "oauth_client.h"

#include <openssl/rand.h>

#include <exception>
#include <memory>

#include "acme_crypto.hpp"
#include "base64.h"
#include "func.h"
#include "httpclient.h"
#include "urlcode.h"

namespace http
{
namespace oauth_client
{

namespace
{
// RFC 7636 PKCE verifier 字符集（unreserved）。
// 这份 VERIFIER_CHARS / VERIFIER_LEN / make_code_verifier / pkce_challenge
// 跟 libs/github/github_oauth.cpp 那份形状完全一样，因为 PKCE 的标准就是一份。
const char   VERIFIER_CHARS[]    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
constexpr unsigned int VERIFIER_LEN = 64;

bool base_ok(const std::string &url, std::string &err_out)
{
    err_out.clear();
    if (url.empty())
    {
        err_out = "endpoint not configured";
        return false;
    }
    if (url.find("https://") != 0)
    {
        err_out = "endpoint must be https";
        return false;
    }
    return true;
}

// token 端点响应的统一解析器。grant_type 差异只影响请求表单，响应形状是同一份。
void parse_token_body(const std::string &body, bool is_json, token_answer &out)
{
    out = token_answer();
    if (!is_json)
    {
        out.error = "response is not json";
        return;
    }
    obj_val root;
    try
    {
        root.from_json(body);
    }
    catch (const std::exception &)
    {
        out.error = "bad json body";
        return;
    }
    if (root.isset("error"))
    {
        std::string reason = root["error"].to_string();
        if (root.isset("error_description"))
        {
            reason += ": ";
            reason += root["error_description"].to_string();
        }
        out.error = reason;
        return;
    }
    out.access_token = root["access_token"].to_string();
    if (out.access_token.empty())
    {
        out.error = "access_token missing";
        return;
    }
    if (root.isset("refresh_token"))
    {
        out.refresh_token = root["refresh_token"].to_string();
    }
    if (root.isset("scope"))
    {
        out.scope = root["scope"].to_string();
    }
    if (root.isset("token_type"))
    {
        out.token_type = root["token_type"].to_string();
    }
    if (root.isset("expires_in"))
    {
        long long v = root["expires_in"].to_int();
        if (v > 0) out.access_expires_in = v;
    }
    if (root.isset("refresh_token_expires_in"))
    {
        long long v = root["refresh_token_expires_in"].to_int();
        if (v > 0) out.refresh_expires_in = v;
    }
    out.ok = true;
}

// Basic 字符串组装 —— client_id:client_secret 的 base64（标准变体，非 url-safe）。
std::string make_basic(const std::string &client_id, const std::string &secret)
{
    std::string pair = client_id + ":" + secret;
    return "Basic " + http::base64_encode(pair.data(), (unsigned int)pair.size(), 0);
}

// token / refresh / client_credentials 三端点公共 POST 逻辑。
// use_basic=true → Authorization: Basic（RFC 6749 §2.3.1 推荐，paozhu OAuth server 用这个）；
// use_basic=false → client_id/client_secret 放 POST body（GitHub 只认这个）。
// 注意两种是互斥的，不是"两种都塞进去让服务端挑"——那样反而会触发某些服务端的安全拒绝。
bool post_token(const oauth_endpoints &ep,
                const std::string     &client_id,
                const std::string     &client_secret_plain,
                bool                   use_basic,
                const std::vector<std::pair<std::string, std::string>> &fields,
                token_answer          &out)
{
    out = token_answer();
    std::string err;
    if (!base_ok(ep.token_url, err))
    {
        out.error = err;
        return false;
    }
    std::shared_ptr<http::client> c = std::make_shared<http::client>();
    c->post(ep.token_url);
    c->set_header("Accept", "application/json");
    c->set_header("Content-Type", "application/x-www-form-urlencoded");
    if (use_basic)
    {
        c->set_header("Authorization", make_basic(client_id, client_secret_plain));
    }
    for (auto &[k, v] : fields)
    {
        c->add_post(k, v);
    }
    if (!use_basic)
    {
        // Body 里的 client_id / client_secret（不走 Basic 时才塞进来）。
        c->add_post("client_id", client_id);
        c->add_post("client_secret", client_secret_plain);
    }
    c->timeout(ep.timeout_seconds);
    c->send();
    unsigned int status = c->get_status();
    c->close_connect();
    if (c->iserror > 0 || status == 0)
    {
        out.http_status = status;
        out.error = "transport error";
        return false;
    }
    parse_token_body(c->page.content, c->page.isjson, out);
    out.http_status = status;
    if (!out.ok && status != 200)
    {
        out.error = "http " + std::to_string(status);
    }
    return out.ok;
}
} // namespace

// ---------------------------------------------------------------------------
// PKCE
// ---------------------------------------------------------------------------
std::string make_code_verifier()
{
    unsigned char rand[VERIFIER_LEN];
    if (RAND_bytes(rand, (int)sizeof(rand)) != 1)
    {
        return "";
    }
    std::string out;
    out.reserve(VERIFIER_LEN);
    for (unsigned int i = 0; i < VERIFIER_LEN; i++)
    {
        out.push_back(VERIFIER_CHARS[rand[i] % sizeof(VERIFIER_CHARS)]);
    }
    return out;
}

std::string pkce_challenge(const std::string &code_verifier)
{
    if (code_verifier.empty())
    {
        return "";
    }
    try
    {
        std::string digest = sha256::hash(code_verifier);
        return b64url::encode(reinterpret_cast<const unsigned char *>(digest.data()), digest.size());
    }
    catch (const std::exception &)
    {
        return "";
    }
}

// ---------------------------------------------------------------------------
// authorize URL
// ---------------------------------------------------------------------------
bool build_authorize_url(const oauth_endpoints &ep,
                         const std::string     &client_id,
                         const std::string     &redirect_uri,
                         const std::string     &scope_space_separated,
                         const std::string     &state,
                         const std::string     &code_challenge,
                         std::string           &url_out)
{
    url_out.clear();
    std::string err;
    if (!base_ok(ep.authorize_url, err))
    {
        return false;
    }
    if (client_id.empty() || redirect_uri.empty() || state.empty())
    {
        return false;
    }
    url_out = ep.authorize_url;
    url_out += "?response_type=code";
    url_out += "&client_id="     + url_encode(client_id.data(), (int)client_id.size());
    url_out += "&redirect_uri="  + url_encode(redirect_uri.data(), (int)redirect_uri.size());
    url_out += "&state="         + url_encode(state.data(), (int)state.size());
    if (!scope_space_separated.empty())
    {
        url_out += "&scope="      + url_encode(scope_space_separated.data(), (int)scope_space_separated.size());
    }
    if (!code_challenge.empty())
    {
        url_out += "&code_challenge=" + url_encode(code_challenge.data(), (int)code_challenge.size());
        url_out += "&code_challenge_method=S256";
    }
    return true;
}

// ---------------------------------------------------------------------------
// 三个 grant
// ---------------------------------------------------------------------------
bool exchange_code(const oauth_endpoints &ep,
                   const std::string     &client_id,
                   const std::string     &client_secret_plain,
                   const std::string     &code,
                   const std::string     &redirect_uri,
                   const std::string     &code_verifier,
                   token_answer          &out)
{
    std::vector<std::pair<std::string, std::string>> fields;
    fields.emplace_back("grant_type", "authorization_code");
    fields.emplace_back("code", code);
    fields.emplace_back("redirect_uri", redirect_uri);
    if (!code_verifier.empty())
    {
        fields.emplace_back("code_verifier", code_verifier);
    }
    return post_token(ep, client_id, client_secret_plain, ep.prefer_basic_auth, fields, out);
}

bool refresh_access(const oauth_endpoints &ep,
                    const std::string     &client_id,
                    const std::string     &client_secret_plain,
                    const std::string     &refresh_token_in,
                    token_answer          &out)
{
    std::vector<std::pair<std::string, std::string>> fields;
    fields.emplace_back("grant_type", "refresh_token");
    fields.emplace_back("refresh_token", refresh_token_in);
    return post_token(ep, client_id, client_secret_plain, ep.prefer_basic_auth, fields, out);
}

bool client_credentials(const oauth_endpoints &ep,
                        const std::string     &client_id,
                        const std::string     &client_secret_plain,
                        const std::string     &scope,
                        token_answer          &out)
{
    std::vector<std::pair<std::string, std::string>> fields;
    fields.emplace_back("grant_type", "client_credentials");
    if (!scope.empty())
    {
        fields.emplace_back("scope", scope);
    }
    return post_token(ep, client_id, client_secret_plain, ep.prefer_basic_auth, fields, out);
}

// ---------------------------------------------------------------------------
// introspect
// ---------------------------------------------------------------------------
bool do_introspect(const oauth_endpoints &ep,
                   const std::string     &client_id,
                   const std::string     &client_secret_plain,
                   const std::string     &token,
                   const std::string     &token_type_hint,
                   introspect_answer     &out)
{
    out = introspect_answer();
    std::string err;
    if (!base_ok(ep.introspect_url, err))
    {
        out.error = err;
        return false;
    }
    std::shared_ptr<http::client> c = std::make_shared<http::client>();
    c->post(ep.introspect_url);
    c->set_header("Accept", "application/json");
    c->set_header("Content-Type", "application/x-www-form-urlencoded");
    if (ep.prefer_basic_auth)
    {
        c->set_header("Authorization", make_basic(client_id, client_secret_plain));
    }
    c->add_post("token", token);
    if (!token_type_hint.empty())
    {
        c->add_post("token_type_hint", token_type_hint);
    }
    if (!ep.prefer_basic_auth)
    {
        c->add_post("client_id", client_id);
        c->add_post("client_secret", client_secret_plain);
    }
    c->timeout(ep.timeout_seconds);
    c->send();
    out.http_status = c->get_status();
    if (c->iserror > 0 || out.http_status == 0)
    {
        c->close_connect();
        out.error = "transport error";
        return false;
    }
    c->close_connect();

    // 401 = 凭据错，整条请求没进 introspect 逻辑；200 但 isjson=false 是服务端坏
    if (out.http_status == 401)
    {
        out.error = "invalid_client";
        return true;
    }
    if (!c->page.isjson)
    {
        out.error = "response is not json";
        return false;
    }
    obj_val root;
    try { root.from_json(c->page.content); }
    catch (const std::exception &)
    {
        out.error = "bad json body";
        return false;
    }
    out.ok     = true;
    out.active = root["active"].to_bool();
    if (!out.active)
    {
        return true;
    }
    if (root.isset("token_type")) out.token_type = root["token_type"].to_string();
    if (root.isset("client_id"))  out.client_id  = root["client_id"].to_string();
    if (root.isset("iss"))        out.iss        = root["iss"].to_string();
    if (root.isset("sub"))        out.sub        = root["sub"].to_string();
    if (root.isset("aud"))        out.aud        = root["aud"].to_string();
    if (root.isset("scope"))      out.scope      = root["scope"].to_string();
    if (root.isset("jti"))        out.jti        = root["jti"].to_string();
    if (root.isset("exp"))        out.exp        = root["exp"].to_int();
    if (root.isset("iat"))        out.iat        = root["iat"].to_int();
    return true;
}

// ---------------------------------------------------------------------------
// revoke
// ---------------------------------------------------------------------------
revoke_result do_revoke(const oauth_endpoints &ep,
                        const std::string     &client_id,
                        const std::string     &client_secret_plain,
                        const std::string     &token,
                        const std::string     &token_type_hint,
                        unsigned int          &http_status_out)
{
    http_status_out = 0;
    std::string err;
    if (!base_ok(ep.revoke_url, err))
    {
        return revoke_result::transport;
    }
    std::shared_ptr<http::client> c = std::make_shared<http::client>();
    c->post(ep.revoke_url);
    c->set_header("Accept", "application/json");
    c->set_header("Content-Type", "application/x-www-form-urlencoded");
    if (ep.prefer_basic_auth)
    {
        c->set_header("Authorization", make_basic(client_id, client_secret_plain));
    }
    c->add_post("token", token);
    if (!token_type_hint.empty())
    {
        c->add_post("token_type_hint", token_type_hint);
    }
    if (!ep.prefer_basic_auth)
    {
        c->add_post("client_id", client_id);
        c->add_post("client_secret", client_secret_plain);
    }
    c->timeout(ep.timeout_seconds);
    c->send();
    http_status_out = c->get_status();
    c->close_connect();
    if (c->iserror > 0 || http_status_out == 0)
    {
        return revoke_result::transport;
    }
    if (http_status_out == 401)
    {
        return revoke_result::auth_failed;
    }
    // RFC 7009：200 / 202 / 204 都是成功；"本来就没有这枚令牌"服务端也回 200（防枚举）
    return revoke_result::ok;
}

} // namespace oauth_client
} // namespace http
