#ifndef __OAUTH_CONFIG_H__
#define __OAUTH_CONFIG_H__

#include <string>

namespace http
{
namespace oauth
{

// conf/oauth.conf 的 [oauth] 段读数。
// 每次请求现读，不缓存：本仓库已经踩过"启动读一次把正文留在内存里、换同一路径的密钥要重启"
// 的坑（vendor/webpay/src/alipay.cpp 的密钥正文缓存），而这枚 ini 只有五行键值，
// 比 RS256 签一次便宜三个数量级。
struct oauth_settings
{
    // 读到文件、且 [oauth] 段在，才是 true。段里没有的键回落到编译期默认，不算失败。
    bool loaded = false;
    // 实际读到的路径，诊断用；文件不存在时为空。
    std::string file;
    // token 里的 iss。必须与对外访问地址逐字符一致（含 scheme、不带结尾 '/'），
    // 结尾的 '/' 在读取时截掉。空串＝没配，签发端点要据此拒绝。
    std::string issuer;
    // JWT 头与 /oauth/jwks 里的密钥 id。空值已回落成 CONST_OAUTH_KID，不会为空。
    std::string kid;
    // 两个密钥文件，已经按 conf 所在目录规整成可直接打开的路径（绝对值原样保留）。
    std::string privkey_file;
    std::string pubkey_file;
    // refresh token 的有效期（秒）。头里不写默认值：
    // 默认由 read_oauth_settings() 从 CONST_OAUTH_REFRESH_TTL 填，两个数写在两处迟早会漂。
    // 读到 0 说明调用方没走 read_oauth_settings()。
    long long refresh_ttl = 0;
};

// 读 conf/oauth.conf。conffile 传空串时按顺序回落：
// serverconfig::configpath + "oauth.conf" → "conf/oauth.conf"（独立进程没加载过 server.conf）。
// 文件缺失、打不开、解析抛异常都只让 loaded=false 并保留默认值，不抛给调用方；
// 私钥路径读不出来的那一步由 load_signing_keys() 判。
oauth_settings read_oauth_settings(const std::string &conffile = "");

} // namespace oauth
} // namespace http
#endif
