/**
 * jwt.h
 *
 * JWT (RFC 7519) public class: HS256 / ES256 / RS256 / PS256 encode & verify.
 * Public namespace: http::jwt
 *
 * verify() checks the signature, the algorithm whitelist and the exp / nbf timings.
 * iat is read into verify_result.iat and reported, never compared: a token carrying a future
 * iat still verifies. Age and "issued after <time>" rules belong to the caller, made after
 * verify() returned ok.
 * iss and aud are not checked — the accepted issuer / audience lists belong to the
 * caller, which reads them out of payload_json.
 *
 * Depends on acme_crypto.hpp primitives:
 *   b64url::encode/decode, hmac_sha256::sign, jws::es256_sign, ec_crypto::import_pem
 */
#pragma once

#include "acme_crypto.hpp"

#include <ctime>
#include <string>
#include <vector>
#include <stdexcept>
#include <utility>

namespace http
{
namespace jwt
{

// ============================================================
//  Algorithm whitelist —— hardcoded, no string parsing
// ============================================================
enum class algorithm
{
    HS256, // HMAC-SHA256, 对称
    ES256, // ECDSA P-256 + SHA-256, 非对称
    RS256, // RSA PKCS#1 v1.5 + SHA-256, 非对称
    PS256  // RSA-PSS + SHA-256 (salt==hash), 非对称
};

// ============================================================
//  Verify failure reasons —— fine-grained for diagnostics
// ============================================================
enum class verify_error
{
    ok,                   // 验签通过 | verification passed
    malformed_token,      // 段数不对 / 空段 | wrong segment count / empty segment
    invalid_base64,       // base64url 解码失败 | b64url decode failed
    alg_mismatch,         // header.alg != 预期 | header.alg does not match expected
    alg_none_rejected,    // alg:none 明确拒绝 | alg:none explicitly rejected
    signature_mismatch,   // 签名不匹配 | signature mismatch
    key_mismatch,         // 密钥类型不对 / PEM 加载失败 | wrong key type / PEM load failed
    token_expired,        // exp < now - leeway | token expired
    token_not_yet_valid,  // nbf > now + leeway | token not yet valid
    invalid_issuer,       // iss 不匹配 | issuer mismatch (placeholder, not default-checked)
    invalid_audience,     // aud 不匹配 | audience mismatch (placeholder, not default-checked)
    invalid_claims,       // exp/iat/nbf 在 payload 里但不是数字 | claim present, not a number
    malformed_json,       // 解出来的段不是 JSON object | decoded section is not a JSON object
    unsupported_critical, // header 带 crit 扩展 | header carries a crit extension
    parsed_but_unverified, // 只解析、未验签 | decoded, deliberately not verified
    token_too_long        // 整枚令牌超过 CONST_JWT_TOKEN_MAX_LEN | token over the length cap
};

// ============================================================
//  Verification result —— err==ok means all checks passed
// ============================================================
struct verify_result
{
    verify_error err        = verify_error::ok;
    std::string  header_json;  // raw JSON (NOT base64); verify() fills it only when err==ok
    std::string  payload_json; // raw JSON (NOT base64); verify() fills it only when err==ok
    std::string  alg;          // alg string read from header ("HS256"/"ES256"/"RS256"/"PS256")
    time_t       exp = 0;      // extracted claim (0 when absent or unreadable)
    time_t       iat = 0;
    time_t       nbf = 0;
};

// ============================================================
//  Encode: build "h.p.s" signed JWT
// ============================================================
/**
 * 签发 JWT 三段式字符串 (header.payload.signature).
 * Build JWT three-part string "header.payload.signature".
 *
 * @param header_json  header JSON object, must contain {"alg":"HS256"|"ES256"|"RS256"|"PS256"}
 *                     auto-injects "typ":"JWT" if missing
 * @param payload_json claims JSON object (caller constructs; exp/nbf/iss/aud etc.)
 * @param key          HS256: raw HMAC bytes, must not be empty;
 *                     ES256/RS256/PS256: PEM-encoded private key
 *                     (RSA key auto-detected by OpenSSL 3.0; no EC vs RSA split needed)
 * @param alg          explicit algorithm — must match header_json["alg"], else throw
 * @return JWT string
 * @throws std::invalid_argument on param mismatch (including a section that is not a JSON
 *         object, or an empty HMAC secret); std::runtime_error on OpenSSL failure
 */
std::string encode(
    const std::string &header_json,
    const std::string &payload_json,
    const std::string &key,
    algorithm alg);

// ============================================================
//  Verify: decode + verify signature + claim checks
// ============================================================
/**
 * 解析 + 验签 + claim 时效校验 (alg whitelist, exp, nbf, alg:none reject).
 * Parse, verify signature, and check standard claims.
 *
 * @param token          "h.p.s" JWT
 * @param key            HS256: same raw bytes as encode, empty secret is refused with
 *                       key_mismatch; ES256: PEM-encoded EC public key;
 *                       RS256/PS256: PEM-encoded RSA public key
 * @param expected_alg   allowed algorithm (whitelist)
 * @param leeway_seconds clock skew tolerance for exp/nbf, default 0
 * @return verify_result (err==ok means success)
 *
 * Only the signature and the exp / nbf timings are checked. iss and aud are left to the
 * caller — it owns the accepted issuer and audience lists. iat is the one timing read but
 * never compared: a numeric iat in the future (or in 1970) verifies, and only a non-numeric
 * iat is refused with invalid_claims. verify_result.iat carries the value so the caller can
 * judge age itself.
 *
 * All three segments must be base64url text, including the signature: a segment that is not
 * is refused with invalid_base64 rather than silently decoded to whatever the reader stops at.
 * On a refused token the err is reported and header_json / payload_json stay empty, so an
 * unauthenticated section never reaches the caller as text.
 *
 * The whole token is bounded by CONST_JWT_TOKEN_MAX_LEN (common/cost_define.h) before any
 * segment is decoded; over it the err is token_too_long. For ES256 the public key must be on
 * P-256 — another curve, or a key that is not an EC one at all, is reported as key_mismatch
 * instead of the signature_mismatch a wrong-curve verification would otherwise produce.
 */
verify_result verify(
    const std::string &token,
    const std::string &key,
    algorithm expected_alg,
    int leeway_seconds = 0);

// ============================================================
//  Decode only —— parse segments without verification
// ============================================================
/**
 * 仅 base64url 解码拆段，不验签不校验 alg/claim.
 * Useful when caller needs to read header.alg first, then choose a key.
 *
 * Does NOT reject alg:none — that's intentional for gateway routing.
 *
 * WARNING: err==parsed_but_unverified 只表示"三段解析成功"，**不等于** token 可信。
 * 绝不能用它替代 verify()：签名、alg、exp/nbf 一个都没查。
 * err==parsed_but_unverified means "the segments parsed", NOT "the token is authentic".
 * Never use it in place of verify(): no signature, alg or claim is checked.
 *
 * The two section texts are the decoded bytes exactly as the token carried them — this
 * reader does not confirm they are valid JSON, and a claim may come from the tail of a
 * payload that stops mid-string.
 */
verify_result decode_without_verify(const std::string &token);

} // namespace jwt
} // namespace http
