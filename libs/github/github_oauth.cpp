// GitHub 的授权/令牌/吊销三个端点。
// 出站形状照 controller/src/test_weixin.cpp 的"拿 code 换令牌再读 JSON"那一段，
// 凭据来自调用方传进来的行内值（解密后的 client_secret），不来自全局 conf。
#include "github_oauth.h"

#include <openssl/rand.h>

#include <cctype>
#include <exception>
#include <memory>

#include "acme_crypto.hpp"
#include "base64.h"
#include "cost_define.h"
#include "func.h"
#include "github_crypto.h"
#include "httpclient.h"
#include "request.h"
#include "urlcode.h"

namespace http
{
namespace github
{

namespace
{
// 这一段参与拼接的字符集：GitHub 的 verifier 允许 RFC 7636 的 unreserved 字符。
const char VERIFIER_CHARS[]               = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
constexpr unsigned int VERIFIER_CHARS_LEN = 64;
constexpr unsigned int VERIFIER_LEN       = 64;

bool is_client_id_char(char c)
{
    unsigned char u = (unsigned char)c;
    // 点必须是允许的：GitHub App 换出来的 client_id 形如 Iv1.2345678901abcdef，
    // 少了这一划，每一条合法的绑定都会在回调这一步被判成伪造票据。
    return std::isalnum(u) != 0 || c == '-' || c == '_' || c == '.';
}
}// namespace

std::string normalize_scopes(const std::string &scopes_comma)
{
    std::vector<std::string> items;
    for (auto &one : mb_split(",", scopes_comma))
    {
        std::string item = str_trim(one);
        if (item.empty())
        {
            continue;
        }
        bool dup = false;
        for (auto &kept : items)
        {
            if (kept == item)
            {
                dup = true;
                break;
            }
        }
        if (!dup)
        {
            items.push_back(item);
        }
    }
    // read:user 强制附加：没有它 GET /user 回 403，绑定行上的身份三列永远填不上。
    bool has_user = false;
    for (auto &kept : items)
    {
        if (kept == "read:user")
        {
            has_user = true;
            break;
        }
    }
    if (!has_user)
    {
        items.push_back("read:user");
    }
    std::string out;
    for (size_t i = 0; i < items.size(); i++)
    {
        if (i > 0)
        {
            out.push_back(' ');
        }
        out.append(items[i]);
    }
    return out;
}

std::string make_state(unsigned long long adminid, const std::string &client_id)
{
    unsigned char rand[32];
    if (RAND_bytes(rand, (int)sizeof(rand)) != 1)
    {
        return "";
    }
    std::string seg_id  = b64url::encode(std::to_string(adminid));
    std::string seg_cid = b64url::encode(client_id);
    std::string seg_rnd = b64url::encode(rand, sizeof(rand));
    return seg_id + "." + seg_cid + "." + seg_rnd;
}

bool state_shape_ok(const std::string &state, unsigned long long &adminid_out, std::string &client_id_out)
{
    adminid_out = 0;
    client_id_out.clear();
    auto parts = mb_split(".", state);
    if (parts.size() != 3)
    {
        return false;
    }
    // 第一段：adminid 的十进制。走 str2uint64_strict 而不是 stoul——非数字与越界一律拒，
    // 而且 0 不是合法的 adminid（登录 session 里没有 adminid=0 的用户）。
    std::string admin_text = b64url::decode(parts[0]);
    if (admin_text.empty() || admin_text.size() > 20)
    {
        return false;
    }
    unsigned long long adminid = 0;
    if (!str2uint64_strict(admin_text, adminid) || adminid == 0)
    {
        return false;
    }
    // 第二段：client_id 只按形状判，**不查库**——首次绑定时占位行还没插。
    std::string client_id = b64url::decode(parts[1]);
    if (client_id.empty() || client_id.size() > CONST_GH_CLIENT_ID_MAX_LEN)
    {
        return false;
    }
    for (char c : client_id)
    {
        if (!is_client_id_char(c))
        {
            return false;
        }
    }
    // 第三段：随机值的 base64url。内容不解释，只要求非空；逐字节比对由调用方拿 session 做。
    if (parts[2].empty())
    {
        return false;
    }
    adminid_out   = adminid;
    client_id_out = client_id;
    return true;
}

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
        out.push_back(VERIFIER_CHARS[rand[i] % VERIFIER_CHARS_LEN]);
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
        // 摘要失败就没有 challenge；调用方按"这一趟不带 PKCE"走，不能发半条 URL。
        return "";
    }
}

token_answer parse_token_body(const std::string &body, bool is_json)
{
    token_answer out;
    if (!is_json)
    {
        // 没带 Accept: application/json 时 GitHub 回的是 urlencoded 文本；框架不抛、
        // page.json 是空的。这一格为 false 就是失败，不是"取不到 expires_in"。
        out.error = "response is not json";
        return out;
    }
    obj_val root;
    try
    {
        root.from_json(body);
    }
    catch (const std::exception &)
    {
        out.error = "bad json body";
        return out;
    }
    if (root.isset("error"))
    {
        // error_description 可能把你提交的东西原样带回来，这里就过一遍过滤。
        std::string reason = root["error"].to_string();
        if (root.isset("error_description"))
        {
            reason += ": ";
            reason += root["error_description"].to_string();
        }
        out.error = scrub_error_text(reason);
        return out;
    }
    out.access_token = root["access_token"].to_string();
    if (out.access_token.empty())
    {
        out.error = "access_token missing";
        return out;
    }
    out.scopes = root["scope"].to_string();
    // 第一态：响应里没有 expires_in，也没有 refresh_token——这一枚令牌永不过期，不是"未知"。
    if (root.isset("expires_in"))
    {
        long long in_seconds = root["expires_in"].to_int();
        if (in_seconds > 0)
        {
            out.access_expires_in = in_seconds;
        }
    }
    out.refresh_token = root["refresh_token"].to_string();
    if (!out.refresh_token.empty() && root.isset("refresh_token_expires_in"))
    {
        long long in_seconds = root["refresh_token_expires_in"].to_int();
        if (in_seconds > 0)
        {
            out.refresh_expires_in = in_seconds;
        }
    }
    out.ok = true;
    return out;
}

namespace
{
// 令牌端点的公共发送部分：换令牌与刷新只差在表单里带哪几个字段。
bool post_token_request(const github_settings &cfg,
                        const std::string &client_id,
                        const std::string &client_secret_plain,
                        const std::vector<std::pair<std::string, std::string>> &fields,
                        token_answer &out)
{
    out = token_answer();
    if (!outbound_base_ok(cfg.auth_base))
    {
        out.error = "auth_base is not https";
        return false;
    }
    if (client_id.empty() || client_secret_plain.empty())
    {
        out.error = "client credentials missing";
        return false;
    }
    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    a->post(cfg.auth_base + "/login/oauth/access_token");
    a->set_header("Accept", "application/json");
    a->set_header("Content-Type", "application/x-www-form-urlencoded");
    a->add_post("client_id", client_id);
    a->add_post("client_secret", client_secret_plain);
    for (auto &[key, value] : fields)
    {
        a->add_post(key, value);
    }
    a->timeout(cfg.http_timeout);
    a->send();
    unsigned int status = a->get_status();
    if (a->iserror > 0 || status == 0)
    {
        a->close_connect();
        out.error = "transport error";
        return false;
    }
    a->close_connect();
    out = parse_token_body(a->page.content, a->page.isjson);
    if (!out.ok && status != 200)
    {
        // 状态码本身比"正文里没写 error"更有信息量，且这个数绝不含秘密。
        out.error = "http " + std::to_string(status);
    }
    return out.ok;
}
}// namespace

bool exchange_code(const github_settings &cfg,
                   const std::string &client_id,
                   const std::string &client_secret_plain,
                   const std::string &code,
                   const std::string &redirect_uri,
                   const std::string &code_verifier,
                   token_answer &out)
{
    std::vector<std::pair<std::string, std::string>> fields;
    fields.emplace_back("grant_type", "authorization_code");
    fields.emplace_back("code", code);
    fields.emplace_back("redirect_uri", redirect_uri);
    if (!code_verifier.empty())
    {
        fields.emplace_back("code_verifier", code_verifier);
    }
    return post_token_request(cfg, client_id, client_secret_plain, fields, out);
}

bool refresh_access(const github_settings &cfg,
                    const std::string &client_id,
                    const std::string &client_secret_plain,
                    const std::string &refresh_token,
                    token_answer &out)
{
    std::vector<std::pair<std::string, std::string>> fields;
    fields.emplace_back("grant_type", "refresh_token");
    fields.emplace_back("refresh_token", refresh_token);
    return post_token_request(cfg, client_id, client_secret_plain, fields, out);
}

bool build_authorize_url(const std::string &auth_base,
                         const std::string &client_id,
                         const std::string &redirect_uri,
                         const std::string &scopes_comma,
                         const std::string &state,
                         const std::string &code_challenge,
                         std::string &url_out)
{
    url_out.clear();
    if (!outbound_base_ok(auth_base))
    {
        return false;
    }
    if (client_id.empty() || redirect_uri.empty() || state.empty())
    {
        return false;
    }
    std::string scopes = normalize_scopes(scopes_comma);
    if (scopes.empty())
    {
        return false;
    }
    // 请求带来的/库里存的值一律先编码再拼：不编码的话一个 '&' 就能把这条 Location
    // 导向别的 redirect_uri 或别的 client_id。state 已经是 base64url，再过一次只是兜底。
    url_out = auth_base + "/login/oauth/authorize?client_id=" +
              url_encode(client_id.data(), (int)client_id.size()) + "&redirect_uri=" +
              url_encode(redirect_uri.data(), (int)redirect_uri.size()) + "&scope=" +
              url_encode(scopes.data(), (int)scopes.size()) + "&state=" +
              url_encode(state.data(), (int)state.size());
    if (!code_challenge.empty())
    {
        url_out += "&code_challenge=" +
                   url_encode(code_challenge.data(), (int)code_challenge.size()) +
                   "&code_challenge_method=S256";
    }
    return true;
}

revoke_outcome revoke_token(const github_settings &cfg,
                            const std::string &client_id,
                            const std::string &client_secret_plain,
                            const std::string &access_token,
                            unsigned int &status_out)
{
    status_out = 0;
    if (!outbound_base_ok(cfg.api_base))
    {
        return revoke_outcome::failed;
    }
    if (client_id.empty() || client_secret_plain.empty() || access_token.empty())
    {
        return revoke_outcome::failed;
    }
    // 请求体用 obj_val 生成，令牌里的任何字符都由序列化负责转义。
    obj_val body;
    body["access_token"] = access_token;

    // HTTP Basic：标准字母表的 base64（框架已有 http::base64_encode，第三参 0＝非 URL 安全变体）。
    std::string pair  = client_id + ":" + client_secret_plain;
    std::string basic = "Basic " + http::base64_encode(pair.data(), (unsigned int)pair.size(), 0);
    pair.clear();

    std::shared_ptr<http::client> a = std::make_shared<http::client>();
    a->del(cfg.api_base + "/applications/" + url_encode(client_id.data(), (int)client_id.size()) +
           "/token");
    a->set_header("Accept", "application/json");
    a->set_header("Content-Type", "application/json");
    a->set_header("Authorization", basic);
    a->set_body(body.to_json());
    a->timeout(cfg.http_timeout);
    a->send();
    status_out = a->get_status();
    a->close_connect();
    if (a->iserror > 0 || status_out == 0)
    {
        return revoke_outcome::failed;
    }
    if (status_out == 200 || status_out == 204)
    {
        return revoke_outcome::revoked;
    }
    // 404/422 的语义是"GitHub 侧已经没有可吊销的东西"，重试永远失败；
    // 留着本地行只会变成一行删不掉的僵尸绑定。
    if (status_out == 404 || status_out == 422)
    {
        return revoke_outcome::already_gone;
    }
    return revoke_outcome::failed;
}

}// namespace github
}// namespace http
