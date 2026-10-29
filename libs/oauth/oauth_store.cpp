#include "oauth_store.h"

#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "acme_crypto.hpp"
#include "cost_define.h"
#include "datetime.h"
#include "func.h"
#include "jwt.h"
#include "orm.h"
#include "request.h"
#include "urlcode.h"

namespace http
{
namespace oauth
{

std::string random_token()
{
    unsigned char buf[32];
    if (RAND_bytes(buf, sizeof(buf)) != 1)
    {
        return "";
    }
    return b64url::encode(buf, sizeof(buf));
}

std::string lowerhex_sha256(const std::string &text)
{
    std::string digest = sha256::hash(text);
    return char2hex(reinterpret_cast<const unsigned char *>(digest.data()),
                    (unsigned int)digest.size(), 0);
}

bool pkce_s256_ok(const std::string &code_verifier, const std::string &code_challenge)
{
    if (code_verifier.size() < 43 || code_verifier.size() > 128)
    {
        return false;
    }
    // 32 字节摘要的无填充 base64url 恒为 43 字符，先按长度否掉非 S256 的登记值
    if (code_challenge.size() != 43)
    {
        return false;
    }
    std::string digest    = sha256::hash(code_verifier);
    std::string computed  = b64url::encode(reinterpret_cast<const unsigned char *>(digest.data()),
                                          digest.size());
    if (computed.size() != code_challenge.size())
    {
        return false;
    }
    return CRYPTO_memcmp(computed.data(), code_challenge.data(), computed.size()) == 0;
}

bool parse_basic_client_auth(const std::string &auth_header,
                             std::string &client_id,
                             std::string &client_secret)
{
    client_id.clear();
    client_secret.clear();
    if (auth_header.size() < 7 || !str_casecmp_pre(auth_header, "Basic "))
    {
        return false;
    }
    // b64url::decode 也吃得下标准 base64，但解不开只会给空串或被截断的前缀，不报错
    std::string decoded = b64url::decode(auth_header.substr(6));
    size_t colon        = decoded.find(':');
    if (colon == std::string::npos)
    {
        return false;
    }
    // 用 url_decode_path 而不是 url_decode：后者是表单编码，会把凭据里的 '+' 解成空格，
    // 而 RFC 6749 §2.3.1 指的是 RFC 3986 的百分号编码，'+' 是字面量
    client_id     = url_decode_path(decoded.data(), (int)colon);
    client_secret = url_decode_path(decoded.data() + colon + 1, (int)(decoded.size() - colon - 1));
    return !client_id.empty() && !client_secret.empty();
}

bool verify_client_secret(const std::string &auth_header,
                          const std::string &body_client_id,
                          const std::string &body_client_secret,
                          const std::string &stored_secret_hash,
                          std::string &out_client_id)
{
    out_client_id.clear();
    std::string submitted_id;
    std::string submitted_secret;
    // 带了 Authorization 头就只认它：头的形状不对时不回落到 body，
    // 否则一个坏头 + body 里的明文口令等于绕过了 scheme 检查
    if (!auth_header.empty())
    {
        if (!parse_basic_client_auth(auth_header, submitted_id, submitted_secret))
        {
            return false;
        }
    }
    else
    {
        submitted_id     = body_client_id;
        submitted_secret = body_client_secret;
    }
    if (submitted_id.empty() || submitted_secret.empty())
    {
        return false;
    }
    // 两处都给了 client_id 就必须相等，否则"A 的口令 + B 的 client_id"能过
    if (!body_client_id.empty() && body_client_id != submitted_id)
    {
        return false;
    }
    // 未配口令的保密客户端一律拒绝：空库值对空提交会因为长度相等而放过
    // （CRYPTO_memcmp 对 0 长度返回 0）
    if (stored_secret_hash.empty())
    {
        return false;
    }
    std::string submitted_hash = lowerhex_sha256(submitted_secret);
    if (submitted_hash.size() != stored_secret_hash.size())
    {
        return false;
    }
    if (CRYPTO_memcmp(submitted_hash.data(), stored_secret_hash.data(), stored_secret_hash.size()) != 0)
    {
        return false;
    }
    out_client_id = submitted_id;
    return true;
}

namespace
{
// 令牌的库内表示。sha256::hash 失败会抛，而这一层的失败形状是"空串/false"，
// 所以异常收在这里，不让它从 store 里逃到端点上。
bool token_hash_hex(const std::string &plain, std::string &out_hash)
{
    out_hash.clear();
    if (plain.empty())
    {
        return false;
    }
    try
    {
        out_hash = lowerhex_sha256(plain);
    }
    catch (const std::exception &)
    {
        out_hash.clear();
        return false;
    }
    return !out_hash.empty();
}

// 定长前缀的 ASCII 大小写不敏感比较。只给 scheme 用（RFC 3986 §3.1 说 scheme 大小写不敏感），
// 不改写原串——落库的与换码时逐字节比的必须是同一份原文。
bool prefix_ci(const std::string &text, const char *prefix, std::size_t len)
{
    if (text.size() < len)
    {
        return false;
    }
    for (std::size_t i = 0; i < len; i++)
    {
        char a = text[i];
        char b = prefix[i];
        if (a >= 'A' && a <= 'Z')
        {
            a = (char)(a - 'A' + 'a');
        }
        if (b >= 'A' && b <= 'Z')
        {
            b = (char)(b - 'A' + 'a');
        }
        if (a != b)
        {
            return false;
        }
    }
    return true;
}
} // namespace

bool load_client(const std::string &client_id, client_record &out)
{
    out = client_record();
    if (client_id.empty())
    {
        return false;
    }
    auto model = orm::cms::OauthClient();
    model.where("client_id", client_id);
    if (model.fetch_one() == 0)
    {
        return false;
    }
    out.found          = true;
    out.client_id      = model.data.client_id;
    out.secret_hash    = model.data.client_secret;
    // 公开客户端是真 NULL，不是 ''：读侧的位图由 fetch 填，两者在库里分得开
    out.secret_is_null = model.isNullClientSecret();
    out.redirect_uris  = model.data.redirect_uris;
    out.name           = model.data.name;
    out.confidential   = model.data.is_confidential != 0;
    out.isopen         = model.data.isopen != 0;
    out.allowed_scopes = model.data.allowed_scopes;
    out.grant_types    = model.data.grant_types;
    return true;
}

// token 与 revoke 两个端点共用这一套判定：选 id → 查表 → 按 is_confidential 分路。
bool authenticate_client(const std::string &auth_header,
                         const std::string &body_client_id,
                         const std::string &body_client_secret,
                         client_record &out_rec,
                         std::string &out_client_id)
{
    out_rec = client_record();
    out_client_id.clear();
    std::string probe_id = body_client_id;
    if (!auth_header.empty())
    {
        std::string basic_id;
        std::string basic_secret;
        if (!parse_basic_client_auth(auth_header, basic_id, basic_secret))
        {
            return false;
        }
        // 两处都给了 id 就必须相等，否则"拿 A 的口令配 B 的 client_id"能过
        if (!body_client_id.empty() && body_client_id != basic_id)
        {
            return false;
        }
        probe_id = basic_id;
    }
    // 停用的客户端在这里就断掉：token 与 revoke 都当"没有这个 client"，
    // 和不存在 id 完全同形（端点只回 invalid_client），否则这个口就成了"谁注册过又被停用"的探针。
    if (probe_id.empty() || !load_client(probe_id, out_rec) || !out_rec.found || !out_rec.isopen)
    {
        return false;
    }
    if (out_rec.confidential)
    {
        return verify_client_secret(auth_header, body_client_id, body_client_secret, out_rec.secret_hash, out_client_id);
    }
    // 公开客户端只靠 PKCE，没有口令这一说：任何认证形态一律拒
    if (!auth_header.empty() || !body_client_secret.empty())
    {
        return false;
    }
    out_client_id = out_rec.client_id;
    return true;
}

bool match_redirect_uri(const std::string &whitelist,
                        const std::string &submitted,
                        std::string &out_matched)
{
    out_matched.clear();
    if (whitelist.empty() || submitted.empty())
    {
        return false;
    }
    // 逐字节比较放在 C++ 里：这一列是 TEXT，不进 WHERE，也就躲开了库的排序规则
    for (const std::string &one : mb_split(",", whitelist))
    {
        std::string item = str_trim(one);
        if (!item.empty() && item == submitted)
        {
            out_matched = item;
            return true;
        }
    }
    return false;
}

bool scope_within(const std::string &allowed, const std::string &requested)
{
    if (allowed.empty() || requested.empty())
    {
        return true;
    }
    // 先切请求、再切白名单，逐项 ==。用 find() 会让 allowed="admin" 放过 requested="dmi"，
    // 那一条是提权，不是"宽松"。
    for (const std::string &want : mb_split(" ", requested))
    {
        std::string item = str_trim(want);
        if (item.empty())
        {
            continue;
        }
        bool hit = false;
        for (const std::string &have : mb_split(" ", allowed))
        {
            if (str_trim(have) == item)
            {
                hit = true;
                break;
            }
        }
        if (!hit)
        {
            return false;
        }
    }
    return true;
}

bool grant_within(const std::string &allowed, const std::string &grant)
{
    // 没有"没申请 grant"这种情况：走到这一句的分支一定带着一个 grant 名，
    // 所以空 grant 在白名单非空时是拒，不是放。
    if (allowed.empty())
    {
        return true;
    }
    if (grant.empty())
    {
        return false;
    }
    for (const std::string &have : mb_split(",", allowed))
    {
        if (str_trim(have) == grant)
        {
            return true;
        }
    }
    return false;
}

bool redirect_uri_scheme_ok(const std::string &uri)
{
    // https:// 后面至少要有一个字符，且不能直接是路径/查询/片段：空主机名匹配不上任何回调
    if (prefix_ci(uri, "https://", 8))
    {
        if (uri.size() == 8)
        {
            return false;
        }
        char c = uri[8];
        return c != '/' && c != '?' && c != '#';
    }
    // 明文只放行环回 IP 字面量（RFC 9700 §4.1 给原生应用的例外），127.0.0.1 与 [::1]
    if (!prefix_ci(uri, "http://", 7))
    {
        return false;
    }
    std::string rest = uri.substr(7);
    for (const char *loopback : {"127.0.0.1", "[::1]"})
    {
        std::string host(loopback);
        if (rest.size() < host.size() || rest.compare(0, host.size(), host) != 0)
        {
            continue;
        }
        // 主机名后面必须立刻是端口、路径或结尾，否则 "http://127.0.0.1.evil.com/cb" 也带着环回前缀
        if (rest.size() == host.size())
        {
            return true;
        }
        char c = rest[host.size()];
        return c == ':' || c == '/' || c == '?';
    }
    return false;
}

bool add_client(const std::string &client_id,
                const std::string &name,
                const std::string &redirect_uris,
                bool confidential,
                const std::string &allowed_scopes,
                const std::string &grant_types,
                bool isopen,
                std::string &out_secret_plain)
{
    out_secret_plain.clear();
    if (client_id.empty() || client_id.size() > CONST_OAUTH_CLIENT_ID_MAX_LEN)
    {
        return false;
    }
    if (name.size() > CONST_OAUTH_CLIENT_NAME_MAX_LEN || redirect_uris.empty())
    {
        return false;
    }
    if (allowed_scopes.size() > CONST_OAUTH_CLIENT_SCOPES_MAX_LEN ||
        grant_types.size() > CONST_OAUTH_CLIENT_GRANTS_MAX_LEN)
    {
        return false;
    }
    // scheme 闸门在写入前：http:// 只放行环回字面量，其余必须 https://
    for (const std::string &one : mb_split(",", redirect_uris))
    {
        std::string item = str_trim(one);
        if (item.empty() || !redirect_uri_scheme_ok(item))
        {
            return false;
        }
    }
    auto model = orm::cms::OauthClient();
    model.data.client_id       = client_id;
    model.data.name            = name;
    model.data.redirect_uris   = redirect_uris;
    model.data.is_confidential = confidential ? 1 : 0;
    model.data.isopen          = isopen ? 1 : 0;
    model.data.allowed_scopes  = allowed_scopes;
    model.data.grant_types     = grant_types;
    if (confidential)
    {
        out_secret_plain = random_token();
        std::string secret_hash;
        if (out_secret_plain.empty() || !token_hash_hex(out_secret_plain, secret_hash))
        {
            out_secret_plain.clear();
            return false;
        }
        model.data.client_secret = secret_hash;
    }
    else
    {
        model.setNullClientSecret();
    }
    // insert() 恒为 INSERT；effect==0 覆盖 client_id 撞唯一键和语句本身失败两种情况
    auto [effect, new_id] = model.insert();
    (void)new_id;
    if (effect == 0)
    {
        out_secret_plain.clear();
        return false;
    }
    return true;
}

std::string issue_code(const std::string &client_id,
                       long long user_id,
                       const std::string &redirect_uri,
                       const std::string &scope,
                       const std::string &code_challenge,
                       long long expires_at)
{
    std::string code = random_token();
    if (code.empty())
    {
        return "";
    }
    std::string code_hash;
    if (!token_hash_hex(code, code_hash))
    {
        return "";
    }
    auto model = orm::cms::OauthCode();
    model.data.code_hash      = code_hash;
    model.data.client_id      = client_id;
    model.data.user_id        = user_id;
    model.data.redirect_uri   = redirect_uri;
    model.data.scope          = scope;
    model.data.code_challenge = code_challenge;
    model.data.expires_at     = expires_at;
    model.data.used           = 0;
    // insert() 恒为 INSERT；save() 在模型带主键时会变成 UPDATE
    auto [effect, new_id] = model.insert();
    (void)new_id;
    if (effect == 0)
    {
        return "";
    }
    return code;
}

bool find_code(const std::string &code_plain, code_record &out)
{
    out = code_record();
    std::string code_hash;
    if (!token_hash_hex(code_plain, code_hash))
    {
        return false;
    }
    auto model = orm::cms::OauthCode();
    model.where("code_hash", code_hash);
    if (model.fetch_one() == 0)
    {
        return false;
    }
    out.found          = true;
    out.client_id      = model.data.client_id;
    out.user_id        = model.data.user_id;
    out.redirect_uri   = model.data.redirect_uri;
    out.scope          = model.data.scope;
    out.code_challenge = model.data.code_challenge;
    out.expires_at     = model.data.expires_at;
    out.used           = model.data.used != 0;
    out.family_id      = model.data.family_id;
    return true;
}

bool consume_code(const std::string &code_plain)
{
    std::string code_hash;
    if (!token_hash_hex(code_plain, code_hash))
    {
        return false;
    }
    auto model = orm::cms::OauthCode();
    model.data.used = 1;
    model.where("code_hash", code_hash).where("used", 0);
    return model.update("used") == 1;
}

bool bind_code_family(const std::string &code_plain, unsigned int family_id)
{
    // 0 是"还没回填"的状态，不是族号：写进码行就等于把这条码挂到"所有未回填的族根"那一堆上
    if (family_id == 0)
    {
        return false;
    }
    std::string code_hash;
    if (!token_hash_hex(code_plain, code_hash))
    {
        return false;
    }
    auto model = orm::cms::OauthCode();
    model.data.family_id = family_id;
    // 只给已消费的码绑族：没换过码的行绑上了也没有族可撤，绑错了还会让重放撤掉别人的族
    model.where("code_hash", code_hash).where("used", 1);
    return model.update("family_id") == 1;
}

bool find_refresh(const std::string &token_plain, refresh_record &out)
{
    out = refresh_record();
    std::string token_hash;
    if (!token_hash_hex(token_plain, token_hash))
    {
        return false;
    }
    auto model = orm::cms::OauthRefresh();
    model.where("token_hash", token_hash);
    if (model.fetch_one() == 0)
    {
        return false;
    }
    out.found      = true;
    out.row_id     = model.data.id;
    out.family_id  = model.data.family_id;
    out.client_id  = model.data.client_id;
    out.user_id    = model.data.user_id;
    out.scope      = model.data.scope;
    out.expires_at = model.data.expires_at;
    out.revoked    = model.data.revoked != 0;
    return true;
}

std::string issue_refresh(unsigned int family_id,
                          const std::string &client_id,
                          long long user_id,
                          const std::string &scope,
                          long long expires_at,
                          unsigned int &out_row_id)
{
    out_row_id = 0;
    std::string token = random_token();
    if (token.empty())
    {
        return "";
    }
    std::string token_hash;
    if (!token_hash_hex(token, token_hash))
    {
        return "";
    }
    auto model = orm::cms::OauthRefresh();
    model.data.family_id  = family_id;
    model.data.token_hash = token_hash;
    model.data.client_id  = client_id;
    model.data.user_id    = user_id;
    model.data.scope      = scope;
    model.data.expires_at = expires_at;
    model.data.revoked    = 0;
    auto [effect, new_id] = model.insert();
    if (effect == 0 || new_id == 0)
    {
        return "";
    }
    // 自增 id 是 unsigned long long，族根要写回 unsigned int 的 family_id 列
    out_row_id = (unsigned int)new_id;
    if (family_id == 0)
    {
        // 族根回填自己的 id：留着 0，revoke_refresh_family 那句 WHERE family_id=?
        // 就会把所有未回填的族一起撤掉。宁可不发这个令牌，也不留那样的行。
        model.data.family_id = out_row_id;
        model.where("id", (long long)out_row_id);
        if (model.update("family_id") != 1)
        {
            return "";
        }
    }
    return token;
}

bool revoke_refresh_row(unsigned long long row_id)
{
    if (row_id == 0)
    {
        return false;
    }
    auto model = orm::cms::OauthRefresh();
    model.data.revoked = 1;
    model.where("id", (long long)row_id).where("revoked", 0);
    return model.update("revoked") == 1;
}

unsigned int revoke_refresh_family(unsigned int family_id)
{
    // 0 不是族号，是"这行还没回填"的状态；拿它当条件会命中所有族根
    if (family_id == 0)
    {
        return 0;
    }
    auto model = orm::cms::OauthRefresh();
    model.data.revoked = 1;
    model.where("family_id", family_id).where("revoked", 0);
    int effect = model.update("revoked");
    return effect > 0 ? (unsigned int)effect : 0;
}

bool revoke_refresh_by_token(const std::string &token_plain, const std::string &client_id)
{
    if (client_id.empty())
    {
        return false;
    }
    std::string token_hash;
    if (!token_hash_hex(token_plain, token_hash))
    {
        return false;
    }
    auto model = orm::cms::OauthRefresh();
    model.data.revoked = 1;
    model.where("token_hash", token_hash).where("client_id", client_id).where("revoked", 0);
    return model.update("revoked") == 1;
}

unsigned int purge_codes_before(long long cutoff, unsigned int max_rows)
{
    auto model = orm::cms::OauthCode();
    model.where("expires_at", orm::wq::lt, cutoff);
    model.limit(max_rows);
    int effect = model.remove();
    return effect > 0 ? (unsigned int)effect : 0;
}

unsigned int purge_refreshes_before(long long cutoff, unsigned int max_rows)
{
    auto model = orm::cms::OauthRefresh();
    model.where("expires_at", orm::wq::lt, cutoff);
    model.limit(max_rows);
    int effect = model.remove();
    return effect > 0 ? (unsigned int)effect : 0;
}

// ---------------------------------------------------------------------------
// RS256 签发与验签
// ---------------------------------------------------------------------------

namespace
{
// 整份读入一枚密钥文件。先断"是常规文件"：macOS/Linux 上 ifstream 打开目录也报 good()，
// 光看流会把配错成目录的路径判成"能读"，到取正文那一刻才失败。
// 尺寸上限挡住"路径写成日志文件"这类配置错误：代价是这一单签失败，不是把整份文件读进内存。
bool slurp_key_file(const std::string &path, std::string &out_text)
{
    out_text.clear();
    if (path.empty())
    {
        return false;
    }
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec))
    {
        return false;
    }
    std::uintmax_t size = std::filesystem::file_size(path, ec);
    if (ec || size == 0 || size > (std::uintmax_t)CONST_OAUTH_PEM_MAX_SIZE)
    {
        return false;
    }
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    out_text = ss.str();
    return !out_text.empty();
}

// kid 会被原样拼进 JWT 头和 /oauth/jwks，所以只收"配置里的短标识"这一形状：
// 定长上限 + 字母数字和 - . _ :。带引号或反斜杠的值等于让配置决定 header 的结构，直接拒。
bool kid_usable(const std::string &kid)
{
    if (kid.empty() || kid.size() > CONST_OAUTH_KID_MAX_LEN)
    {
        return false;
    }
    for (char raw : kid)
    {
        unsigned char c = (unsigned char)raw;
        bool        ok  = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                   c == '-' || c == '.' || c == '_' || c == ':';
        if (!ok)
        {
            return false;
        }
    }
    return true;
}

// verify_error 转一行文本，只进日志。jwt.h 里只有枚举，没有现成的转文本函数。
const char *jwt_error_name(jwt::verify_error err)
{
    switch (err)
    {
    case jwt::verify_error::ok: return "ok";
    case jwt::verify_error::malformed_token: return "malformed_token";
    case jwt::verify_error::invalid_base64: return "invalid_base64";
    case jwt::verify_error::alg_mismatch: return "alg_mismatch";
    case jwt::verify_error::alg_none_rejected: return "alg_none_rejected";
    case jwt::verify_error::signature_mismatch: return "signature_mismatch";
    case jwt::verify_error::key_mismatch: return "key_mismatch";
    case jwt::verify_error::token_expired: return "token_expired";
    case jwt::verify_error::token_not_yet_valid: return "token_not_yet_valid";
    case jwt::verify_error::invalid_issuer: return "invalid_issuer";
    case jwt::verify_error::invalid_audience: return "invalid_audience";
    case jwt::verify_error::invalid_claims: return "invalid_claims";
    case jwt::verify_error::malformed_json: return "malformed_json";
    case jwt::verify_error::unsupported_critical: return "unsupported_critical";
    case jwt::verify_error::parsed_but_unverified: return "parsed_but_unverified";
    }
    return "unknown";
}

// 大端无符号整数 → 无填充 base64url，前导零字节按 RFC 7518 §6.3.1 去掉（模数可能带一枚 0x00
// 保正数）。EVP_PKEY_get_bn_param 发的是新分配的 BIGNUM，所有权在这里一起收尾：调用一次就没了。
std::string bn_to_b64url(BIGNUM *bn)
{
    if (bn == nullptr)
    {
        return "";
    }
    int len = BN_num_bytes(bn);
    if (len <= 0)
    {
        BN_free(bn);
        return "";
    }
    std::string bytes((size_t)len, '\0');
    BN_bn2bin(bn, reinterpret_cast<unsigned char *>(&bytes[0]));
    BN_free(bn);
    size_t first = bytes.find_first_not_of('\0');
    if (first == std::string::npos)
    {
        return "";
    }
    return b64url::encode(reinterpret_cast<const unsigned char *>(bytes.data()) + first,
                          bytes.size() - first);
}
} // namespace

bool load_signing_keys(const oauth_settings &settings,
                       std::string &privkey_pem,
                       std::string &pubkey_pem)
{
    privkey_pem.clear();
    pubkey_pem.clear();
    if (!slurp_key_file(settings.privkey_file, privkey_pem))
    {
        return false;
    }
    if (!slurp_key_file(settings.pubkey_file, pubkey_pem))
    {
        privkey_pem.clear();
        return false;
    }
    return true;
}

std::string sign_access_token(const std::string &privkey_pem,
                              const std::string &payload_json,
                              const std::string &kid)
{
    if (privkey_pem.empty() || payload_json.empty() || !kid_usable(kid))
    {
        return "";
    }
    std::string header_json = std::string("{\"alg\":\"RS256\",\"typ\":\"JWT\",\"kid\":\"") + kid + "\"}";
    try
    {
        return jwt::encode(header_json, payload_json, privkey_pem, jwt::algorithm::RS256);
    }
    catch (const std::exception &)
    {
        // 私钥读不出、payload 不是 JSON object、OpenSSL 失败都从这一个口子出来。
        // 端点回 500 server_error，细节留在日志，不回显。
        return "";
    }
}

bool verify_access_token(const std::string &pubkey_pem,
                         const std::string &token,
                         int leeway,
                         std::string &payload_json,
                         std::string &out_error)
{
    payload_json.clear();
    out_error.clear();
    if (pubkey_pem.empty())
    {
        out_error = "public key not loaded";
        return false;
    }
    // iss 和 aud 不在这里判：jwt::verify 明确不查这两项，接受的签发方由端点手里那份配置决定
    jwt::verify_result r = jwt::verify(token, pubkey_pem, jwt::algorithm::RS256, leeway);
    if (r.err != jwt::verify_error::ok)
    {
        out_error = jwt_error_name(r.err);
        return false;
    }
    payload_json = r.payload_json;
    return true;
}

bool looks_like_jwt(const std::string &token)
{
    size_t dots = 0;
    size_t seg  = 0;
    for (char raw : token)
    {
        unsigned char c = (unsigned char)raw;
        if (c == '.')
        {
            dots++;
            // 空段（"a..b"、".a.b"、"a.b."）不是三段 base64url
            if (seg == 0)
            {
                return false;
            }
            seg = 0;
            continue;
        }
        bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                  c == '-' || c == '_';
        if (!ok)
        {
            return false;
        }
        seg++;
    }
    return dots == 2 && seg > 0;
}

bool introspect_token(const std::string &authed_client_id,
                      const std::string &pubkey_pem,
                      const std::string &issuer,
                      int leeway,
                      const std::string &token,
                      introspect_record &out)
{
    out = introspect_record();
    if (pubkey_pem.empty())
    {
        // 公钥没加载就没法验签，这不是"令牌不活"。端点在调用前已经把这一格挡成 500，
        // 这里再判一次是为了让"配置坏了"永远不会被回答成 active:false。
        return false;
    }
    if (authed_client_id.empty() || token.empty())
    {
        return true;
    }

    if (looks_like_jwt(token))
    {
        std::string payload_json;
        std::string why;
        if (!verify_access_token(pubkey_pem, token, leeway, payload_json, why))
        {
            // 签名不对、过期、形状坏：都是"不活"，why 只进调用方的日志
            return true;
        }
        obj_val claims;
        try
        {
            claims.from_json(payload_json);
        }
        catch (const std::exception &)
        {
            // 验签通过的令牌，payload 是我们自己签进去的；它不是 JSON 只能说明签发侧坏了
            return false;
        }
        // 今天到不了这一句：验签那步已经挡过非对象的 payload，坏 JSON 走上面那个 catch。
        // 留着是因为两条路的回答不一样——这里回 false 是 500，而落到下面按成员取值只会
        // 读出空串，于是"签发侧坏了"被藏成"这枚令牌不活"。
        if (claims.get_type() != obj_type::OBJECT)
        {
            return false;
        }
        // iss 逐字符相等（只认自己域签出来的），aud 必须是提问的这个客户端。
        // 两格任一不成立都回 active:false，与"根本没有这枚令牌"同形——回 403 等于承认
        // "这是一枚真令牌，只是不是你的"，那是免费的枚举口。
        std::string iss = claims["iss"].to_string();
        std::string aud = claims["aud"].to_string();
        if (iss != issuer || aud != authed_client_id)
        {
            return true;
        }
        out.active     = true;
        out.token_type = "Bearer";
        out.client_id  = aud;
        out.iss        = iss;
        out.sub        = claims["sub"].to_string();
        out.aud        = aud;
        out.scope      = claims["scope"].to_string();
        out.jti        = claims["jti"].to_string();
        out.exp        = claims["exp"].to_int();
        out.iat        = claims["iat"].to_int();
        return true;
    }

    refresh_record rr;
    if (!find_refresh(token, rr) || !rr.found)
    {
        return true;
    }
    if (rr.revoked || rr.expires_at <= (long long)timeid() || rr.client_id != authed_client_id)
    {
        return true;
    }
    out.active     = true;
    out.token_type = "refresh_token";
    out.client_id  = rr.client_id;
    out.sub        = std::to_string(rr.user_id);
    out.scope      = rr.scope;
    out.exp        = rr.expires_at;
    // iss/aud/jti/iat 在 oauth_refresh 里没有对应列（那八列是 id/family_id/token_hash/
    // client_id/user_id/scope/expires_at/revoked），留空由端点决定不发这几格。
    return true;
}

std::string jwks_json(const std::string &pubkey_pem, const std::string &kid)
{
    if (pubkey_pem.empty() || !kid_usable(kid))
    {
        return "";
    }
    BIO_ptr bio(BIO_new_mem_buf(pubkey_pem.data(), (int)pubkey_pem.size()));
    if (!bio)
    {
        return "";
    }
    // 只认 "PUBLIC KEY"（与 jwt.cpp 的验签同一把读法），私钥格式不在这里输出
    EVP_PKEY *raw = PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
    if (raw == nullptr)
    {
        return "";
    }
    EVP_PKEY_ptr pkey(raw);

    BIGNUM *n_bn = nullptr;
    if (EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_N, &n_bn) != 1)
    {
        if (n_bn != nullptr)
        {
            BN_free(n_bn);
        }
        return "";
    }
    std::string n_b64 = bn_to_b64url(n_bn);

    BIGNUM *e_bn = nullptr;
    if (EVP_PKEY_get_bn_param(pkey.get(), OSSL_PKEY_PARAM_RSA_E, &e_bn) != 1)
    {
        if (e_bn != nullptr)
        {
            BN_free(e_bn);
        }
        return "";
    }
    std::string e_b64 = bn_to_b64url(e_bn);
    if (n_b64.empty() || e_b64.empty())
    {
        return "";
    }
    return std::string("{\"keys\":[{\"kty\":\"RSA\",\"alg\":\"RS256\",\"use\":\"sig\",\"kid\":\"") + kid +
           "\",\"n\":\"" + n_b64 + "\",\"e\":\"" + e_b64 + "\"}]}";
}

} // namespace oauth
} // namespace http
