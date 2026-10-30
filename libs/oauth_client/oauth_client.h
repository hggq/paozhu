// OAuth 2.x / 2.1 通用客户端库。
// 只依赖 httpclient + SHA-256 + base64url + JSON 解析，不绑定具体授权服务器——
// endpoints 里填四个 URL 就能对 GitHub / Google / 自搭的 paozhu oauth 端点通吃。
//
// 跟 libs/github/github_oauth.cpp 的关系：
//   - github_oauth 是 GitHub 专用（state 三段式、revoke 用 DELETE+Basic、
//     scope 强制追加 read:user），接口签名也带 github_settings；
//   - 本文件是通用版，所有 GitHub 专属逻辑都不出现，PKCE 形状两者完全一致。
//
// 用法：
//   1) 构造 oauth_endpoints，填上 authorize / token / introspect / revoke 四个 URL
//      以及 prefer_basic_auth=false 如果端点是 GitHub 这种不认 Basic 的
//   2) make_code_verifier() + pkce_challenge() 生成 PKCE
//   3) build_authorize_url() 拼 authorize URL，浏览器跳过去
//   4) 回调拿到 code 之后 exchange_code() 换 access + refresh
//   5) refresh_access() / client_credentials() / do_introspect() / do_revoke()

#ifndef __OAUTH_CLIENT_H__
#define __OAUTH_CLIENT_H__

#include <string>
#include <vector>

namespace http
{
namespace oauth_client
{

// ---------------------------------------------------------------------------
// 端点配置。不猜任何字段的格式——调用方直接填完整 URL。
// introspect / revoke 留空时，对应调用返回 false + "endpoint not configured"。
// ---------------------------------------------------------------------------
struct oauth_endpoints
{
    std::string authorize_url; // 例如 https://www.hggq.com/oauth/authorize
    std::string token_url;     // 例如 https://www.hggq.com/oauth/token
    std::string introspect_url;// 例如 https://www.hggq.com/oauth/introspect
    std::string revoke_url;    // 例如 https://www.hggq.com/oauth/revoke
    int timeout_seconds    = 10;
    bool prefer_basic_auth = true;// true=Authorization: Basic（RFC 6749 §2.3.1 推荐）；false=client_id/secret 放 POST body（GitHub 要求）
};

// ---------------------------------------------------------------------------
// PKCE。复用 github_oauth 的形状——直接抄过来避免两份漂移。
// ---------------------------------------------------------------------------
std::string make_code_verifier();
std::string pkce_challenge(const std::string &code_verifier);

// ---------------------------------------------------------------------------
// authorize URL 拼装。state 与 code_challenge 必须预先生成好传进来，
// 这里不做状态管理（调用方自己决定把 state + verifier 存在哪——session / DB / cookie）。
// 返回 false 时 url_out 被清空。
// ---------------------------------------------------------------------------
bool build_authorize_url(const oauth_endpoints &ep,
                         const std::string &client_id,
                         const std::string &redirect_uri,
                         const std::string &scope_space_separated,
                         const std::string &state,
                         const std::string &code_challenge,// 空串＝不带 PKCE（仅授权请求）
                         std::string &url_out);

// ---------------------------------------------------------------------------
// token 端点响应。http_status 独立出来方便调用方区分"传输成功但业务拒绝"
// （http 200 + error=invalid_grant）和"整条请求没到达服务端"（http 0）。
// ---------------------------------------------------------------------------
struct token_answer
{
    bool ok                  = false;
    unsigned int http_status = 0;
    std::string access_token;
    std::string refresh_token;// 空 = 这一轮响应没给 refresh（client_credentials）
    std::string scope;
    long long access_expires_in  = 0;// 0 = 无过期
    long long refresh_expires_in = 0;// 0 = 响应里没给
    std::string token_type;          // Bearer 或 MAC（绝大多数字段忽略后者）
    std::string error;               // 失败原因，永远不含密钥
};

// ---------------------------------------------------------------------------
// 三个 grant：authorization_code / refresh_token / client_credentials。
// client_secret 只在本函数栈帧存活，绝不写日志、绝不塞进结构体。
// ---------------------------------------------------------------------------
bool exchange_code(const oauth_endpoints &ep,
                   const std::string &client_id,
                   const std::string &client_secret_plain,
                   const std::string &code,
                   const std::string &redirect_uri,
                   const std::string &code_verifier,
                   token_answer &out);

bool refresh_access(const oauth_endpoints &ep,
                    const std::string &client_id,
                    const std::string &client_secret_plain,
                    const std::string &refresh_token_in,
                    token_answer &out);

bool client_credentials(const oauth_endpoints &ep,
                        const std::string &client_id,
                        const std::string &client_secret_plain,
                        const std::string &scope,
                        token_answer &out);

// ---------------------------------------------------------------------------
// introspect / revoke
// ---------------------------------------------------------------------------
struct introspect_answer
{
    bool ok                  = false;
    bool active              = false;
    unsigned int http_status = 0;
    std::string error;

    // RFC 7662 §2.2：只在 active=true 时服务端才可能回，缺的格不写。
    std::string token_type;
    std::string client_id;
    std::string iss;
    std::string sub;
    std::string aud;
    std::string scope;
    std::string jti;
    long long exp = 0;
    long long iat = 0;
};
bool do_introspect(const oauth_endpoints &ep,
                   const std::string &client_id,
                   const std::string &client_secret_plain,
                   const std::string &token,
                   const std::string &token_type_hint,
                   introspect_answer &out);

enum class revoke_result
{
    ok,         // 200 / 202 / 204：服务端已经把这枚令牌撤掉了（或本来就没有）
    auth_failed,// 401：客户端认证失败，这条请求根本没进 revoke 逻辑
    transport   // 网络错 / 超时 / 端点未配置
};
revoke_result do_revoke(const oauth_endpoints &ep,
                        const std::string &client_id,
                        const std::string &client_secret_plain,
                        const std::string &token,
                        const std::string &token_type_hint,
                        unsigned int &http_status_out);

}// namespace oauth_client
}// namespace http

#endif
