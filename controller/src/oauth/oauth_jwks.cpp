// GET /oauth/jwks：把 conf/oauth.conf 里那枚公钥按 RFC 7517 的 keys 数组发出去。
// 资源侧只拿这一页就能验签，不需要访问授权服务器的库或私钥——这是 RS256 相对 HS256 换来的东西。
//
// 正文由 libs/oauth 拼（n/e 要从 EVP_PKEY 取，见 oauth_store.cpp 的 jwks_json()），
// 这一层只负责读配置、把空串翻译成 500。

#include <string>

#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_config.h"
#include "oauth/oauth_store.h"
#include "oauth_jwks.h"

namespace http
{
//@urlpath(null,oauth/jwks)
std::string oauth_jwks(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();

    oauth::oauth_settings   settings = oauth::oauth_settings_cached();
    oauth::signing_material keys     = oauth::signing_material_cached();
    // 私钥这一格跟着一起读：两枚文件任一读不出都不算就绪，所以私钥缺位时这一页也回 500，
    // 尽管公钥本身是好的。那种部署本来就是配错的（token 端点同样签不出东西），
    // 不为了它再开一个只读公钥的口子。
    if (!keys.ready)
    {
        client.status(500);
        client.val["error"] = "server_error";
        client.out_json();
        return "";
    }
    std::string body = oauth::jwks_json(keys.pubkey_pem, settings.kid);
    if (body.empty())
    {
        // 公钥正文读得出但不是 RSA（或 kid 形状不合格），拼不出和签发头对得上的集合 ⇒ 宁不发
        client.status(500);
        client.val["error"] = "server_error";
        client.out_json();
        return "";
    }
    client.type("application/json; charset=UTF-8");
    client.output = body;
    return "";
}
}// namespace http
