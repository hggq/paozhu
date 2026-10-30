// OAuth 2.1 授权服务器元数据（RFC 8414）：GET /oauth/metadata。
// 只读配置、不碰库、不发证，所以这条路径是公开的。
//
// 这条路径是裸的 /oauth/metadata，没有注册 /.well-known/oauth-authorization-server：
// 注册路径带 '.' 时精确匹配与静态文件分支的先后没实测过，先用这条确定的写法。
//
// 每一个 URL 都由 conf/oauth.conf 的 issuer 拼出来，不从请求的 Host 推——
// 那是让调用方决定元数据里说什么，而 issuer 必须是签发 access_token 时 `iss` 的那个值，两者
// 逐字符相同才可能对得上。

#include <string>
#include <vector>

#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_config.h"
#include "oauth_metadata.h"

namespace http
{
//@urlpath(null,oauth/metadata)
std::string oauth_metadata(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    oauth::oauth_settings settings = oauth::oauth_settings_cached();
    if (settings.issuer.empty())
    {
        // 拼不出 endpoint 的元数据比不发布更糟：客户端会照着一份 issuer 为空的文档配。
        client.status(500);
        client.val["error"] = "server_error";
        client.out_json();
        return "";
    }
    const std::string base = settings.issuer;

    client.val["issuer"]                   = base;
    client.val["authorization_endpoint"]   = base + "/oauth/authorize";
    client.val["token_endpoint"]           = base + "/oauth/token";
    client.val["revocation_endpoint"]      = base + "/oauth/revoke";
    client.val["introspection_endpoint"]   = base + "/oauth/introspect";
    client.val["jwks_uri"]                 = base + "/oauth/jwks";
    client.val["response_types_supported"] = std::vector<std::string>{"code"};
    client.val["grant_types_supported"] =
        std::vector<std::string>{"authorization_code", "refresh_token", "client_credentials"};
    client.val["token_endpoint_auth_methods_supported"] =
        std::vector<std::string>{"client_secret_basic", "client_secret_post", "none"};
    // 只支持 S256，纯码流（plain）在这里就不公布。
    client.val["code_challenge_methods_supported"] = std::vector<std::string>{"S256"};
    client.val["response_modes_supported"]         = std::vector<std::string>{"query"};
    // 空数组：这个 AS 不实现 OIDC，
    // 公布一个它不认的 scope 会让客户端把它写进第一次请求里。
    client.val["scopes_supported"] = std::vector<std::string>{};

    client.out_json();
    return "";
}
}// namespace http
