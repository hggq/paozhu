#ifndef __OAUTH_STORE_H__
#define __OAUTH_STORE_H__

#include <string>

#include "oauth_config.h"

namespace http
{
namespace oauth
{

// 令牌明文（授权码、refresh token）：32 字节随机 → 43 字符无填充 base64url。
// RAND_bytes 失败返回空串，不降级到时间戳；调用方据此拒绝签发。
std::string random_token();

// 令牌的库内表示：小写 hex(SHA-256(明文))，64 字符。sha256::hash 失败会抛异常。
std::string lowerhex_sha256(const std::string &text);

// PKCE 验算：b64url(SHA-256(code_verifier)) 是否等于授权时登记的 code_challenge。
// verifier 长度先按 RFC 7636 §4.1 限在 43..128，两边定长比较。
bool pkce_s256_ok(const std::string &code_verifier, const std::string &code_challenge);

// 解析 Authorization: Basic base64(client_id:client_secret)。
// 两段是 percent-encoded（RFC 6749 §2.3.1），解完要 url_decode；任一段为空返回 false，
// 否则"空 secret 对空提交"会因为长度相等通过比较。口令本身怎么比由调用方决定。
bool parse_basic_client_auth(const std::string &auth_header,
                             std::string &client_id,
                             std::string &client_secret);

// 保密客户端的口令比对（RFC 6749 §2.3.1，两种认证方式走同一条比较）：
//   auth_header          peer->header["authorization"]，key 已被框架转小写；非空就按 Basic 解析，
//                        解析不出来直接拒绝，不回落到 body
//   body_client_id       peer->post["client_id"]，可能为空
//   body_client_secret   peer->post["client_secret"]，可能为空（框架已做表单解码）
//   stored_secret_hash   oauth_client.client_secret，即 lowerhex(SHA-256(口令))
//   out_client_id        认证通过的 client_id（Basic 与 body 都给时取 Basic 那个，两者必须相等）
// 公钥客户端（is_confidential=0）不该调这里，它没有口令。
bool verify_client_secret(const std::string &auth_header,
                          const std::string &body_client_id,
                          const std::string &body_client_secret,
                          const std::string &stored_secret_hash,
                          std::string &out_client_id);

// ---------------------------------------------------------------------------
// 三张表读写（实现要 orm.h ⇒ 这一带连着 [cms] 真库才跑得动）
// 只搬字节：查到的行原样回传，"used 是不是 1、过期没有、client_id 匹不匹配"
// 由端点按自己的顺序判，这层不做策略。
// ---------------------------------------------------------------------------

struct client_record
{
    bool        found           = false;
    bool        isopen          = true;   // isopen != 0；0 ⇒ 端点当"没有这个客户端"
    std::string client_id;
    std::string secret_hash;    // oauth_client.client_secret，lowerhex(SHA-256(口令))
    bool        secret_is_null  = false;  // 真 NULL（公开客户端）；它和 '' 在库里是两回事
    std::string redirect_uris;  // TEXT 原文，逗号分隔
    std::string name;
    bool        confidential    = true;   // is_confidential != 0
    std::string allowed_scopes; // 空格分隔原文；空串=不设上限
    std::string grant_types;    // 逗号分隔原文；空串=三种 grant 全开
};

// 按 client_id 取一行（走唯一键 uq_client_id）。查不到返回 false，out 保持全默认值。
bool load_client(const std::string &client_id, client_record &out);

// 客户端认证的一整套判定：从两个认证形态里选出 client_id、查表、
// 再按 is_confidential 决定走口令比较还是只认 id。token 与 revoke 两个端点必须共用这一套，
// 漂开一处就等于留下一个能用弱形状过的端点。
// 入参是 Authorization 头与表单字段的原始字符串，不碰 httppeer——这一层因此能脱离请求上下文单独调用。
// 两种失败（没这个客户端、口令不对）都只返回 false：端点回同一句话，
// 否则这个端点就变成"哪些 client_id 注册过"的探针。
// out_rec 回传查到的客户端行（失败时可能是全默认值，调用方只看返回值）。
bool authenticate_client(const std::string &auth_header,
                         const std::string &body_client_id,
                         const std::string &body_client_secret,
                         client_record &out_rec,
                         std::string &out_client_id);

// 回调白名单逐字节匹配，逗号分隔。命中时 out_matched 是**白名单里的那一条**，
// 不是提交值原文——授权时要落库的就是它，否则未规范化的值进了库，换码时的比对就跟着请求走。
// 提交值为空一律不命中（白名单里的空项不算匹配）。这一列是 TEXT，不进 WHERE。
bool match_redirect_uri(const std::string &whitelist,
                        const std::string &submitted,
                        std::string &out_matched);

// 两枚策略串的判定，和 match_redirect_uri 同族（纯函数、不碰库、逐字节比）。
// 都是**整词匹配**：把 allowed 切词后逐项 ==，不是 find() 子串——`admin` 放过 `dmi` 是提权。
// allowed 为空串视为"不设上限"；requested 为空串恒 true（没申请就是允许的）。
bool scope_within(const std::string &allowed, const std::string &requested);
bool grant_within(const std::string &allowed, const std::string &grant);

// 回调地址的 scheme 闸门（RFC 9700 §4.1）：一律 https://，明文只放行环回 IP 字面量
// http://127.0.0.1[:port] 与 http://[::1][:port]。只判这一层——路径、查询、片段、是否 https
// 可达都不在此列，运行时防护仍是 match_redirect_uri 的逐字节比较。
// scheme 按 RFC 3986 大小写不敏感；主机名之后的字符大小写敏感。
bool redirect_uri_scheme_ok(const std::string &uri);

// 注册一个客户端，返回是否写进了库（client_id 撞唯一键、长度越界、随机源失败都是 false）。
// 保密客户端现生成口令，明文只从 out_secret_plain 回传这一次，库里落 hex(SHA-256(明文))；
// 公开客户端不生成口令、out_secret_plain 留空，那一列写真 NULL——空串会和"散列过的空串"
// 在库里长得一样，而只有文本路径的 insert 才发得出 NULL。
// redirect_uris 是逗号分隔的原文：每一项要非空且过 redirect_uri_scheme_ok()，其余形状不校验
// （注册方是已登录的后台用户）。allowed_scopes / grant_types 同样只搬字节，空串＝不设上限／全开；
// isopen=false 表示注册即停用。
bool add_client(const std::string &client_id,
                const std::string &name,
                const std::string &redirect_uris,
                bool confidential,
                const std::string &allowed_scopes,
                const std::string &grant_types,
                bool isopen,
                std::string &out_secret_plain);

struct code_record
{
    bool        found          = false;
    std::string client_id;
    long long   user_id        = 0;
    std::string redirect_uri;
    std::string scope;
    std::string code_challenge;
    long long   expires_at     = 0;
    bool        used           = false;
    // 换码成功时由 bind_code_family() 回填的 refresh 族根 id；0＝还没换过码，或那次回填没成。
    // 重放要不要撤族就看这一格：它是 code 到 family 唯一的连线。
    unsigned int family_id     = 0;
};

// 落一行授权码，返回它的明文（43 字符 base64url，库里只存 hex(SHA-256)）。
// 返回空串＝没签发出来（随机源失败、摘要抛、INSERT 无效），调用方据此回 server_error。
std::string issue_code(const std::string &client_id,
                       long long user_id,
                       const std::string &redirect_uri,
                       const std::string &scope,
                       const std::string &code_challenge,
                       long long expires_at);

// 按授权码明文取那一行（内部先算 hex(SHA-256) 再走唯一键）。不判 used，也不判过期。
bool find_code(const std::string &code_plain, code_record &out);

// CAS 消费：UPDATE oauth_code SET used=1 WHERE code_hash=? AND used=0。
// true＝这一次抢到；false＝已消费或根本没有这一行。WHERE 里带 used=0，所以命中必然发生变更。
bool consume_code(const std::string &code_plain);

// 把族根 id 写进那一行授权码：UPDATE oauth_code SET family_id=? WHERE code_hash=? AND used=1。
// 只在换码成功之后调用（0 不是族号，写 0 会让"按族撤销"命中所有未回填的族根，所以这里直接拒）。
// false＝没写进去（码不存在/还没消费/库错）。绑定失败不影响这次换码已经发出去的令牌，
// 代价只是这条码将来被重放时撤不到族。
bool bind_code_family(const std::string &code_plain, unsigned int family_id);

struct refresh_record
{
    bool        found           = false;
    unsigned long long row_id   = 0;  // 轮换时 CAS 的键
    unsigned int family_id      = 0;  // 0＝这一行就是族根（见 issue_refresh）
    std::string client_id;
    long long   user_id         = 0;
    std::string scope;
    long long   expires_at      = 0;
    bool        revoked         = false;
};

// 按 refresh token 明文取那一行。不判 revoked，也不判过期。
bool find_refresh(const std::string &token_plain, refresh_record &out);

// 落一行 refresh，返回它的明文（库里只存 hex(SHA-256)），out_row_id 回新增行的自增 id。
// family_id 传 0 表示这是族里的第一行：插完会把该行自己的 id 回填成 family_id，
// 回填失败就整笔作废（返回空串）——留一行 family_id=0 的族根，"撤整族"就变成"撤所有族根"。
std::string issue_refresh(unsigned int family_id,
                          const std::string &client_id,
                          long long user_id,
                          const std::string &scope,
                          long long expires_at,
                          unsigned int &out_row_id);

// 轮换的 CAS：UPDATE oauth_refresh SET revoked=1 WHERE id=? AND revoked=0。
// true＝正常轮换；false＝这一行早就 revoked，即重放信号。
bool revoke_refresh_row(unsigned long long row_id);

// 重放检测：撤整族（WHERE family_id=? AND revoked=0，走 idx_family），返回撤掉的行数。
// family_id 为 0 时不发语句直接返回 0：那条条件会命中所有未回填族根的族。
unsigned int revoke_refresh_family(unsigned int family_id);

// RFC 7009：按令牌明文撤，条件里必须带 client_id，否则 A 客户端能撤 B 客户端的同值 token。
// true＝撤掉了正好一行未撤销的；false＝不存在、已撤销、或属于别的客户端。
bool revoke_refresh_by_token(const std::string &token_plain, const std::string &client_id);

// 过期行清理：DELETE ... WHERE expires_at < cutoff LIMIT max_rows，返回删除行数。
// max_rows=0 是"删 0 行"，不是"不限"；保留期由调用方算（编译期常量按仓库惯例进 common/cost_define.h）。
unsigned int purge_codes_before(long long cutoff, unsigned int max_rows);
unsigned int purge_refreshes_before(long long cutoff, unsigned int max_rows);

// ---------------------------------------------------------------------------
// RS256 签发与验签（不碰数据库，所以这一层能脱离真库单独调用）
// 密钥路径、iss、kid 都来自 conf/oauth.conf，见 libs/oauth/oauth_config.h。
// ---------------------------------------------------------------------------

// 按 settings 算好的两个路径把 PEM 正文读进来。每次请求现读：文件只有 1~2 KB，
// 而且换过同一路径下的密钥不需要重启进程。超过 CONST_OAUTH_PEM_MAX_SIZE 的文件不当密钥读。
// 任一个读不出返回 false 且两个出参都是空串——签发/验签端点据此回 500，不降级算法。
bool load_signing_keys(const oauth_settings &settings,
                       std::string &privkey_pem,
                       std::string &pubkey_pem);

// access_token 签发。payload_json 由调用方拼好（iss/sub/aud/scope/exp/iat/jti），
// 这里只负责 header（alg/typ/kid）与 RS256 签名；失败返回空串——jwt::encode 会抛，
// 私钥读不出、header 与实参算法不符都从这一个口子出来。
// kid 直接拼进 header JSON，所以带引号或反斜杠的 kid 一律拒（那是把 header 结构交给配置值）。
std::string sign_access_token(const std::string &privkey_pem,
                              const std::string &payload_json,
                              const std::string &kid);

// Bearer 验签：签名、alg 白名单（只有 RS256）、exp/nbf 由 jwt::verify 判，
// iss/aud 归调用方（jwt.h 明确不查这两项）。通过时 payload_json 回填明文 claims。
// out_error 是日志用的定长文本，不回显给调用方。
bool verify_access_token(const std::string &pubkey_pem,
                         const std::string &token,
                         int leeway,
                         std::string &payload_json,
                         std::string &out_error);

// 令牌形状判定：恰好两枚 '.'，且三段都在无填充 base64url 字母表（A-Za-z0-9-_）里、都不为空。
// random_token() 发的 opaque 令牌是 43 字符同字母表、**不含 '.'**，所以这两种形状互斥且穷尽
// ——introspect 据此分流，不看调用方给的 token_type_hint（那是调用方可控的字符串）。
bool looks_like_jwt(const std::string &token);

// introspect 的结果（RFC 7662 §2.2）。active=false 时其余各格一律是空串/0：
// 端点只回 {"active":false}，不区分"不存在""过期""不是你的"——那三种分开回就是枚举口。
// 库里没有对应列的格（opaque 分支的 iss/aud/jti/iat）留空，端点不发这几格，不编一个值出来。
struct introspect_record
{
    bool        active     = false;
    std::string token_type;   // "Bearer"（access token）或 "refresh_token"
    std::string client_id;    // 令牌的主人；active=true 时恒等于认证进来的那个客户端
    std::string iss;
    std::string sub;
    std::string aud;
    std::string scope;
    std::string jti;
    long long   exp        = 0;
    long long   iat        = 0;
};

// 判"这枚令牌对认证进来的这个客户端而言还活着吗"，并把要公布的元数据填进 out。
//   返回 false ＝ **判不了**（公钥没加载、或验签通过的 payload 解析不成 JSON 对象）⇒ 端点回 500；
//   返回 true  ＝ 判完了，答案在 out.active（令牌无效、过期、属于别的客户端都是 active=false）。
// JWT 分支：验签 + exp（jwt::verify）→ iss 与配置逐字符相等 → aud == authed_client_id。
// opaque 分支：find_refresh() → 存在、未撤销、未过期、client_id == authed_client_id。
// aud/client_id 那一格是这条端点的实质防线：少了它，任何注册成功的客户端都能拿别人的令牌
// 进来问"还活着吗、是谁的、有哪些 scope"。
bool introspect_token(const std::string &authed_client_id,
                      const std::string &pubkey_pem,
                      const std::string &issuer,
                      int leeway,
                      const std::string &token,
                      introspect_record &out);

// /oauth/jwks 的 {"keys":[...]} 正文：从公钥 PEM 取 RSA 的 n/e，按 RFC 7518 §6.3
// 去前导零后转无填充 base64url。公钥读不出来返回空串。
std::string jwks_json(const std::string &pubkey_pem, const std::string &kid);

} // namespace oauth
} // namespace http
#endif
