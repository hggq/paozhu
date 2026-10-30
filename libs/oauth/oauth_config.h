#ifndef __OAUTH_CONFIG_H__
#define __OAUTH_CONFIG_H__

#include <string>

namespace http
{
namespace oauth
{

// conf/oauth.conf 的 [oauth] 段读数。五行键值，进程里只读一次并留在内存，
// 取值走 oauth_settings_cached()。read_oauth_settings() 本身保持"调用一次读一次盘"，
// 探针和测试要能指着一个文件强读。
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

// 取常驻的那份读数：第一次调用现读盘，读到了（[oauth] 段在且 issuer 非空）就留在进程里，
// 之后每次只回一份拷贝。issuer 为空是"还没配好"，不钉死——下一次调用再读盘，
// 免得启动时序里 conf 晚到一步就把默认值用一辈子。
// 反过来说：改过同一路径下的 oauth.conf 不会被看见，要重启进程才生效。
oauth_settings oauth_settings_cached();

}// namespace oauth
}// namespace http
#endif
