// OAuth 2.1 Bearer 校验过滤器：挂在需要 access_token 的路由前置（admin_islogin 的 JWT 版）。
// 除了验签，它还把令牌主体换成租户号：按 sub 查 sysuser 行，把 companyid 写进本次请求内存里的
// session["userid"]、adminid 写进 session["adminid"]，下游那批 where("userid", ...) 就直接能用。
// 链上语义（router.cpp 的 classify_step，本项目唯一解释点）：返回 "@ok" 放行，返回 "@exit" 拒绝。
// iss 与 aud 由这一层判——jwt::verify 明确不查这两项，接受的签发方是手里那份 conf/oauth.conf。

#include <string>

#include "acme_crypto.hpp"
#include "cost_define.h"
#include "func.h"
#include "httppeer.h"
#include "oauth/oauth_config.h"
#include "oauth/oauth_store.h"
#include "oauth_bearerauth.h"
#include "orm.h"
#include "terminal_color.h"

namespace http
{
namespace
{
// 401 也要有 JSON 体（RFC 6750 §3），空体客户端读不到 error。
// 细节只进日志，回显出去的永远是固定的 invalid_token。
void bearer_denied(httppeer &client, const std::string &why)
{
    DEBUG_LOG("[oauth] bearer denied: %s", why.c_str());
    client.status(401);
    client.set_header("WWW-Authenticate", "Bearer error=\"invalid_token\"");
    client.val["error"] = "invalid_token";
    client.out_json();
}
} // namespace

//@urlpath(null,oauth/bearerauth)
std::string oauth_bearerauth(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.set_header("Cache-Control", "no-store");

    // 这道闸写在校验器里，不写在各个资源端点上：挂这个过滤器的路由一起生效，以后新加的不用再补一遍。
    if (!client.is_ssl())
    {
        client.status(400);
        client.val["error"]             = "invalid_request";
        client.val["error_description"] = "bearer token must be sent over HTTPS";
        client.out_json();
        return "@exit";
    }

    // 只认 Authorization 头，不接受 ?access_token=：查询串会落进代理与访问日志，令牌不会
    auto found = client.header.find("authorization");
    if (found == client.header.end() || !str_casecmp_pre(found->second, "Bearer ", 7))
    {
        bearer_denied(client, "missing or non-Bearer Authorization header");
        return "@exit";
    }
    std::string token = found->second.substr(7);

    oauth::oauth_settings   settings = oauth::oauth_settings_cached();
    oauth::signing_material keys     = oauth::signing_material_cached();
    if (!keys.ready)
    {
        // 公钥读不出来就没法验签。这不是"令牌无效"，回 500 免得运维把它当成客户端问题
        DEBUG_LOG("[oauth] bearer: public key not loaded (conf/oauth.conf)");
        client.status(500);
        client.val["error"] = "server_error";
        client.out_json();
        return "@exit";
    }
    if (settings.issuer.empty())
    {
        DEBUG_LOG("[oauth] bearer: issuer empty in conf/oauth.conf");
        client.status(500);
        client.val["error"] = "server_error";
        client.out_json();
        return "@exit";
    }

    std::string payload;
    std::string why;
    if (!oauth::verify_access_token(keys.pubkey_pem, token, CONST_OAUTH_JWT_LEEWAY, payload, why))
    {
        bearer_denied(client, why);
        return "@exit";
    }
    // 签发方必须与配置逐字符相等；这个 AS 只认自己域签出来的 token
    if (json_util::get_string(payload, "iss") != settings.issuer)
    {
        bearer_denied(client, "issuer mismatch");
        return "@exit";
    }
    // 撤销名单放在签发方判定之后：只有本站签出来的令牌才值一次 Redis 往返，
    // 别人域的串先按 issuer mismatch 掉。名单读不到时按未撤销放行（口径见
    // is_access_jti_revoked 的声明注释），否则 Redis 一挂这里就变成全站 401 的单点故障。
    if (oauth::is_access_jti_revoked(json_util::get_string(payload, "jti")))
    {
        bearer_denied(client, "access token revoked");
        return "@exit";
    }
    std::string sub       = json_util::get_string(payload, "sub");
    std::string client_id = json_util::get_string(payload, "aud");
    std::string scope     = json_util::get_string(payload, "scope");
    if (sub.empty() || client_id.empty())
    {
        bearer_denied(client, "token has no sub or aud");
        return "@exit";
    }

    // 鉴权结果放在本次请求的内存里：session 只被读进来、没有 save_session()，
    // 所以这些键不会写回共享的会话存储（框架里 save_session() 没有任何隐式调用点，
    // 只有控制器自己调那一次才落盘）。不写 client.val——链上一步之间不清 val，
    // 写了就会被下游 out_json() 回显给调用方。
    client.session["oauth_sub"]       = sub;
    client.session["oauth_client_id"] = client_id;
    client.session["oauth_scope"]     = scope;

    // 租户号从 sysuser 行拿：sub 就是 sysuser.adminid，
    // 而后台那批 where("userid", client.session["userid"].to_int()) 认的是企业 id，
    // 那一列就写在同一行上（sysuser 没有 userid 列，session["userid"]＝sysuser.companyid）。
    // client_credentials 的令牌里 sub 等于 aud（都是 client_id 本身），那一种既没有人也没有租户：
    // 整段跳过，不查库、不注入。这一条不是省一次查询，是把两个号段隔开——
    // 注册口的 client_id 由注册方自己指定，管理员写成"42"这种纯数字时，
    // 机器令牌的 sub 就正好落在 adminid=42 那个人身上，白拿他的企业身份。
    unsigned long long sub_adminid = 0;
    if (sub != client_id && str2uint64_strict(sub, sub_adminid) && sub_adminid > 0)
    {
        auto users = orm::cms::Sysuser();
        // 列写死：这行只要主体和租户两格，sysuser 里还有 password/mobile/email
        users.select("adminid,companyid");
        users.where("adminid", sub_adminid);
        if (users.fetch_one() == 0)
        {
            // 令牌本身没问题，是它代表的账号已经查不到了（删号）。在这里就拒，
            // 别让下游端点拿 userid=0 去猜"这是机器令牌还是查无此人"。
            bearer_denied(client, "subject is not a live sysuser row");
            return "@exit";
        }
        client.session["adminid"] = users.data.adminid;
        client.session["userid"]  = users.data.companyid;
    }
    return "@ok";
}
} // namespace http
