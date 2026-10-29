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

    oauth::oauth_settings settings = oauth::read_oauth_settings("");
    std::string           privkey_pem;
    std::string           pubkey_pem;
    // load_signing_keys() 是两个文件一起读的：任一读不出就两头都给空串，所以这里私钥也在读。
    // 私钥文件缺位时这一页也回 500，尽管公钥本身是好的。
    // 那种部署本来就是配错的（token 端点同样签不出东西），不为了它再开一个只读公钥的口子。
    if (!oauth::load_signing_keys(settings, privkey_pem, pubkey_pem))
    {
        client.status(500);
        client.val["error"] = "server_error";
        client.out_json();
        return "";
    }
    std::string body = oauth::jwks_json(pubkey_pem, settings.kid);
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
} // namespace http
