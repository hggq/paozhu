#ifndef __GITHUB_CONFIG_H__
#define __GITHUB_CONFIG_H__

#include <string>

namespace http
{
namespace github
{

// conf/github.conf 的 [github] 段读数。每次用到时现读，不缓存：
// 本仓库已经踩过"启动读一次把正文留在内存里、换同一路径的密钥要重启"的坑。
struct github_settings
{
    // 读到文件、且 [github] 段在，才是 true。段里没有的键回落到编译期默认，不算失败。
    bool loaded = false;
    // 实际读到的路径，诊断用；文件不存在时为空。
    std::string file;
    // 两个端点根。读进来时已过 https 闸：不以任何一个以 https:// 开头的值会被丢掉并记日志，
    // 保留编译期默认。运行期每次出站前还要再过一次（outbound_base_ok）。
    std::string api_base;
    std::string auth_base;
    // AES-256-GCM 的 master key，已解码成 32 字节二进制。**绝不写日志、绝不进回显。**
    // key_ok 为 false 时这一格为空且不可使用。
    std::string crypto_key;
    // 密钥可用旗。crypto_key 那一格缺失、非 64 个 hex 字符、或解不出 32 字节时为 false。
    // 配置加载不因它退出进程：绑定与回调据它拒绝服务并回"服务器未配置加密密钥"，
    // 定时那一拍据它整轮跳过（不解密、不发 HTTP）。
    bool key_ok = false;
    // 以下四个数都是秒/个，默认由 read_github_settings() 从 CONST_GH_* 填。
    unsigned int sync_interval    = 0;
    unsigned int sync_batch       = 0;
    unsigned int sync_max_pages   = 0;
    unsigned int sync_backoff_max = 0;
    // 出站超时（秒）。框架内部钳在 90，写更大的值不会生效。
    unsigned int http_timeout = 0;
    // client_secret 的**明文**长度闸；密文另按 CONST_GH_CIPHER_MAX_LEN 校验。
    unsigned int secret_max_len = 0;
    // 申请的 scope，逗号分隔原文；read:user 由 normalize_scopes() 强制附加。
    std::string default_scopes;
    // state 票据有效期（秒）。
    unsigned int state_ttl = 0;
    // --- 默认 OAuth App 凭据（quickbind 用） ---
    // 三键同时非空时 default_app_ready() 返回 true，quickbind 路由才可用。
    // default_client_secret 也需要 crypto_key 才能加密，所以 key_ok=false 时就算填了也用不了。
    std::string default_client_id;
    std::string default_client_secret;
    std::string default_redirect_uri;
};

// quickbind 三个默认凭据都配了、且 crypto_key 可用 → 返回 true。
// 缺一不可——少一个就宁可不出现"一键授权"入口，免得点进去又被拒。
bool default_app_ready(const github_settings &cfg);

// 读 conf/github.conf。conffile 传空串时按顺序回落：
// serverconfig::configpath + "github.conf" → "conf/github.conf"（独立进程没加载过 server.conf）。
// 文件缺失、打不开、解析抛异常都只让 loaded=false 并保留默认值，不抛给调用方。
github_settings read_github_settings(const std::string &conffile = "");

// 出站闸：只有以 https:// 开头的端点根才允许带凭据出去。
// 换令牌、刷新、吊销、读仓库四处都要过它；不过就一律不发，让调用方回错误。
bool outbound_base_ok(const std::string &base);

}// namespace github
}// namespace http
#endif
