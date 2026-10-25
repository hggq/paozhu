/**
 * jwt.cpp
 *
 * Implementation of http::jwt public class.
 * Depends on OpenSSL 3.0+ (required by acme_crypto.hpp).
 */

#include "jwt.h"

#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <ctime>

namespace http
{
namespace jwt
{

// ============================================================
//  Anonymous namespace: internal helpers
// ============================================================
namespace
{

// ---- algorithm <-> string ----
// enum → "HS256" / "ES256" / "RS256" / "PS256" string for header comparison
inline const char *alg_to_string(algorithm a)
{
    switch (a)
    {
    case algorithm::HS256: return "HS256";
    case algorithm::ES256: return "ES256";
    case algorithm::RS256: return "RS256";
    case algorithm::PS256: return "PS256";
    }
    return "";
}

// ---- constant-time memory compare ----
// OpenSSL CRYPTO_memcmp — side-channel safe
inline bool constant_time_memcmp(const std::string &a, const std::string &b)
{
    if (a.size() != b.size())
        return false;
    return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
}

// ---- token -> 3 segments ----
// Split "h.b64.p.b64.s.b64" by '.', return exactly 3 parts
// Signature segment (3rd) is allowed to be empty (alg:none case).
// Returns empty vector if fewer than 2 dots present (definitely malformed).
inline std::vector<std::string> split_token(const std::string &token)
{
    std::vector<std::string> parts;
    size_t start = 0;
    for (int i = 0; i < 3; ++i)
    {
        size_t pos = token.find('.', start);
        if (pos == std::string::npos)
            break;
        parts.emplace_back(token.substr(start, pos - start));
        start = pos + 1;
    }
    // last segment (after 2nd dot) — may be empty for alg:none
    parts.emplace_back(token.substr(start));
    return parts;
}

// ---- strict base64url segment check ----
// b64url::decode never fails: it pads, and drops characters it cannot read, so garbage
// decodes into bytes or into nothing. A JWS segment is unpadded base64url, so check the
// alphabet before decoding; a '=' (or any other byte) means the segment is not a JWT one.
inline bool is_b64url_segment(const std::string &s)
{
    if (s.size() % 4 == 1)
        return false;
    for (char c : s)
    {
        const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                        (c >= '0' && c <= '9') || c == '-' || c == '_';
        if (!ok)
            return false;
    }
    return true;
}

// ---- top-level JSON member scan ----
// Looking a key up with find("\"key\"") reads a nested same-named key or a string value as
// the field, and a number reader that hands back 0 for anything it cannot parse makes a
// quoted or null exp look exactly like a missing one. JWT needs the three apart.
enum class claim_state
{
    absent,  // no such top-level member
    present, // member read as a number
    invalid  // member is there, but not a number this class can use
};

inline bool is_json_space(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// index of the closing quote of the string literal that starts at open, or npos
inline size_t skip_string(const std::string &s, size_t open)
{
    for (size_t i = open + 1; i < s.size(); ++i)
    {
        if (s[i] == '\\')
        {
            ++i;
            continue;
        }
        if (s[i] == '"')
            return i;
    }
    return std::string::npos;
}

inline std::string trim_json_text(const std::string &s)
{
    size_t b = 0;
    size_t e = s.size();
    while (b < e && is_json_space(s[b]))
        ++b;
    while (e > b && is_json_space(s[e - 1]))
        --e;
    return s.substr(b, e - b);
}

// End of a value: the first ',' or closing bracket that is not inside a nested container
inline size_t value_end(const std::string &s, size_t start)
{
    int depth = 0;
    for (size_t i = start; i < s.size(); ++i)
    {
        const char c = s[i];
        if (c == '"')
        {
            const size_t close = skip_string(s, i);
            if (close == std::string::npos)
                return s.size();
            i = close;
        }
        else if (c == '{' || c == '[')
            ++depth;
        else if (c == '}' || c == ']')
        {
            if (depth == 0)
                return i;
            --depth;
        }
        else if (c == ',' && depth == 0)
            return i;
    }
    return s.size();
}

// Raw text of the value of a top-level member named key. Members of nested objects are at
// depth > 1 and never match; a string sitting in a value position is skipped because what
// follows it is not a colon.
// 不在首个命中处返回: hits != nullptr 时回报同名顶层成员的总数，重复与否交给调用方裁决。
// 两个 "exp" 谁生效由各自的解析器决定，本类不能悄悄选一个而与调用方的 JSON 库选另一个。
// Scanning does not stop at the first hit: with hits != nullptr the caller gets the total
// and decides what a duplicate means. Which of two "exp" members wins is reader-defined,
// and this class must not quietly pick one while the caller's parser picks the other.
inline bool find_top_member(const std::string &json, const std::string &key, std::string &out,
                            size_t *hits = nullptr)
{
    const std::string quoted = "\"" + key + "\"";
    size_t hit_count = 0;
    bool   filled    = false;
    int depth = 0;
    for (size_t i = 0; i < json.size(); ++i)
    {
        const char c = json[i];
        if (c != '"')
        {
            if (c == '{' || c == '[')
                ++depth;
            else if (c == '}' || c == ']')
                --depth;
            continue;
        }
        const size_t close = skip_string(json, i);
        if (close == std::string::npos)
            break; // 尾部有字符串没闭合，只是不能再往下扫；已经读到的值和命中数照旧交出
        const bool hit = (depth == 1) && (json.compare(i, close - i + 1, quoted) == 0);
        i = close;
        if (!hit)
            continue;

        size_t p = close + 1;
        while (p < json.size() && is_json_space(json[p]))
            ++p;
        if (p >= json.size() || json[p] != ':')
            continue; // this string was a value, not a key
        ++p;
        while (p < json.size() && is_json_space(json[p]))
            ++p;
        ++hit_count;
        if (filled)
            continue; // 值已取到，继续走只为把重复次数数完
        const size_t end = value_end(json, p);
        out = trim_json_text(json.substr(p, end - p));
        filled = true;
    }
    if (hits)
        *hits = hit_count;
    return filled;
}

// The member has to be a quoted string: {"alg":null} and {"alg":256} name no algorithm.
inline bool find_top_string(const std::string &json, const std::string &key, std::string &out,
                            size_t *hits = nullptr)
{
    std::string raw;
    if (!find_top_member(json, key, raw, hits))
        return false;
    if (raw.size() < 2 || raw.front() != '"' || raw.back() != '"')
        return false;
    out = raw.substr(1, raw.size() - 2);
    return true;
}

// A JWT section is a JSON object. Without this gate encode() happily signs a stray string
// or a cut-off statement and hands back a token whose signature is perfectly valid.
// Braces inside a quoted value do not count, so {"note":"a{b}"} stays signable.
inline bool is_json_object(const std::string &json)
{
    const std::string trimmed = trim_json_text(json);
    if (trimmed.size() < 2 || trimmed.front() != '{' || trimmed.back() != '}')
        return false;
    int depth = 0;
    for (size_t i = 0; i < trimmed.size(); ++i)
    {
        const char c = trimmed[i];
        if (c == '"')
        {
            const size_t close = skip_string(trimmed, i);
            if (close == std::string::npos)
                return false; // string never closes, so the object cannot be complete
            i = close;
        }
        else if (c == '{')
            ++depth;
        else if (c == '}')
        {
            --depth;
            if (depth < 0)
                return false;
        }
    }
    return depth == 0;
}

// RFC 7519 NumericDate: plain decimal seconds, optional '-' and an optional fraction that
// is truncated. An exponent, quotes or null are not read as a date.
inline claim_state parse_date(const std::string &v, long long &out)
{
    out = 0;
    if (v.empty())
        return claim_state::invalid;

    size_t i = 0;
    const bool negative = (v[0] == '-');
    if (negative)
        i = 1;
    const size_t first = i;
    while (i < v.size() && v[i] >= '0' && v[i] <= '9')
        ++i;
    if (i == first)
        return claim_state::invalid;

    const long long limit = static_cast<long long>(std::numeric_limits<time_t>::max());
    long long value = 0;
    for (size_t k = first; k < i; ++k)
    {
        const long long digit = v[k] - '0';
        if (value > (limit - digit) / 10)
            return claim_state::invalid; // no longer representable as a time_t
        value = value * 10 + digit;
    }

    if (i < v.size() && v[i] == '.')
    {
        ++i;
        const size_t frac = i;
        while (i < v.size() && v[i] >= '0' && v[i] <= '9')
            ++i;
        if (i == frac)
            return claim_state::invalid; // "5." carries no fraction digits
    }
    if (i != v.size())
        return claim_state::invalid;

    out = negative ? -value : value;
    return claim_state::present;
}

inline claim_state read_date_claim(const std::string &json, const std::string &key, long long &out)
{
    std::string raw;
    size_t      hits = 0;
    if (!find_top_member(json, key, raw, &hits))
        return claim_state::absent;
    if (hits > 1)
        return claim_state::invalid; // 同名成员重复: 谁生效没有定论，判非法而不是挑一个
    return parse_date(raw, out);
}

// exp / iat / nbf of one payload, read in a single pass over the text
struct claim_times
{
    claim_state exp_state = claim_state::absent;
    claim_state iat_state = claim_state::absent;
    claim_state nbf_state = claim_state::absent;
    long long   exp = 0;
    long long   iat = 0;
    long long   nbf = 0;

    bool has_invalid() const
    {
        return exp_state == claim_state::invalid ||
               iat_state == claim_state::invalid ||
               nbf_state == claim_state::invalid;
    }
    time_t exp_as_time() const
    {
        return exp_state == claim_state::present ? static_cast<time_t>(exp) : 0;
    }
    time_t iat_as_time() const
    {
        return iat_state == claim_state::present ? static_cast<time_t>(iat) : 0;
    }
    time_t nbf_as_time() const
    {
        return nbf_state == claim_state::present ? static_cast<time_t>(nbf) : 0;
    }
};

inline claim_times read_claims(const std::string &payload_json)
{
    claim_times c;
    c.exp_state = read_date_claim(payload_json, "exp", c.exp);
    c.iat_state = read_date_claim(payload_json, "iat", c.iat);
    c.nbf_state = read_date_claim(payload_json, "nbf", c.nbf);
    return c;
}

// ---- load EC public key from PEM string ----
// ec_crypto::import_pem only reads private-key PEMs; verify needs public-key variant.
inline EVP_PKEY_ptr import_pub_pem(const std::string &pem)
{
    BIO_ptr bio(BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size())));
    if (!bio)
        throw std::runtime_error("BIO_new_mem_buf failed");
    EVP_PKEY *raw = PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
    if (!raw)
        throw std::runtime_error("PEM_read_bio_PUBKEY failed");
    return EVP_PKEY_ptr(raw);
}

// ---- inject "typ":"JWT" into header JSON if missing ----
// Inserts right before the closing '}', handles both empty {} and existing fields
inline std::string inject_typ(const std::string &header_json)
{
    // Already declares "typ" at the top level? -> keep as-is
    std::string typ_raw;
    if (find_top_member(header_json, "typ", typ_raw))
        return header_json;

    size_t rbrace = header_json.rfind('}');
    if (rbrace == std::string::npos || header_json.size() < 2)
        return header_json; // Malformed, leave as-is

    // Empty object "{}" -> becomes {"typ":"JWT"}
    if (header_json[1] == '}')
    {
        return header_json.substr(0, 1) + "\"typ\":\"JWT\"" + header_json.substr(1);
    }
    // Non-empty object -> insert ",\"typ\":\"JWT\"" before '}'
    return header_json.substr(0, rbrace) + ",\"typ\":\"JWT\"" + header_json.substr(rbrace);
}

// ---- ES256 verify (r||s → DER → EVP_DigestVerify) ----
// Core: decode JWS "r||s" (64 bytes) into BIGNUMs, build ECDSA_SIG, DER-encode, then verify
// Returns false on any failure (length mismatch, DER build fail, verify fail)
bool es256_verify(
    const std::string &data,          // signed payload ("h.b64.p.b64")
    const std::string &sig_b64,       // JWT signature segment (base64url)
    const EVP_PKEY *pub_key)          // loaded EC public key (from PEM)
{
    // Step 1: b64url decode → raw bytes
    std::string rs_bytes = b64url::decode(sig_b64);
    if (rs_bytes.size() != 64)
        return false; // ES256 signature MUST be 64 bytes (32+32)

    // Step 2: split 32B r + 32B s
    const unsigned char *r_ptr = reinterpret_cast<const unsigned char *>(rs_bytes.data());
    const unsigned char *s_ptr = r_ptr + 32;

    // Step 3: raw bytes → BIGNUM (BN_bin2bn handles leading zeros, treats as positive)
    BIGNUM *r_bn = BN_bin2bn(r_ptr, 32, nullptr);
    BIGNUM *s_bn = BN_bin2bn(s_ptr, 32, nullptr);
    if (!r_bn || !s_bn)
    {
        BN_free(r_bn);
        BN_free(s_bn);
        return false;
    }

    // Step 4: build ECDSA_SIG — ECDSA_SIG_set0 takes ownership of r_bn/s_bn
    ECDSA_SIG_ptr sig(ECDSA_SIG_new());
    if (!sig)
    {
        BN_free(r_bn);
        BN_free(s_bn);
        return false;
    }
    if (ECDSA_SIG_set0(sig.get(), r_bn, s_bn) != 1)
    {
        BN_free(r_bn);
        BN_free(s_bn);
        return false;
    }

    // Step 5: ECDSA_SIG → DER encoding
    unsigned char *der_ptr = nullptr;
    int der_len = i2d_ECDSA_SIG(sig.get(), &der_ptr);
    if (der_len <= 0 || !der_ptr)
        return false;
    std::vector<unsigned char> der_sig(der_ptr, der_ptr + der_len);
    OPENSSL_free(der_ptr);

    // Step 6: EVP_DigestVerify with SHA-256
    EVP_MD_CTX_ptr md_ctx(EVP_MD_CTX_new());
    if (!md_ctx)
        return false;

    if (EVP_DigestVerifyInit(md_ctx.get(), nullptr, EVP_sha256(), nullptr,
                             const_cast<EVP_PKEY *>(pub_key)) != 1)
        return false;
    if (EVP_DigestVerifyUpdate(md_ctx.get(), data.data(), data.size()) != 1)
        return false;

    // 1=pass, 0=fail, -1=error — only 1 means success
    int rc = EVP_DigestVerifyFinal(md_ctx.get(), der_sig.data(), der_sig.size());
    return (rc == 1);
}

// ---- RSA sign: PKCS#1 v1.5 (RS256) or PSS (PS256) ----
// Shared EVP_DigestSign path, only padding differs
// padding_type: RSA_PKCS1_PADDING(0) = RS256, RSA_PKCS1_PSS_PADDING = PS256
// Returns raw RSA signature bytes (length = key size in bytes)
std::string rsa_sign(
    const std::string &data,
    const EVP_PKEY *priv_key,
    int padding_type)
{
    EVP_MD_CTX_ptr md_ctx(EVP_MD_CTX_new());
    if (!md_ctx)
        throw std::runtime_error("EVP_MD_CTX_new failed (rsa_sign)");

    EVP_PKEY_CTX *pctx = nullptr;
    if (EVP_DigestSignInit(md_ctx.get(), &pctx, EVP_sha256(), nullptr,
                           const_cast<EVP_PKEY *>(priv_key)) != 1)
        throw std::runtime_error("EVP_DigestSignInit failed (rsa_sign)");

    if (pctx)
    {
        if (EVP_PKEY_CTX_set_rsa_padding(pctx, padding_type) != 1)
            throw std::runtime_error("EVP_PKEY_CTX_set_rsa_padding failed");
        if (padding_type == RSA_PKCS1_PSS_PADDING)
        {
            // RFC 7518 §3.5: the salt is exactly the digest length (32 for SHA-256).
            // DIGEST means that; the separate AUTO form asks for the largest salt the key
            // allows, so the verifier here must pin it rather than let it be derived.
            if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, RSA_PSS_SALTLEN_DIGEST) != 1)
                throw std::runtime_error("EVP_PKEY_CTX_set_rsa_pss_saltlen failed");
        }
    }

    if (EVP_DigestSignUpdate(md_ctx.get(), data.data(), data.size()) != 1)
        throw std::runtime_error("EVP_DigestSignUpdate failed (rsa_sign)");

    size_t sig_len = 0;
    if (EVP_DigestSignFinal(md_ctx.get(), nullptr, &sig_len) != 1)
        throw std::runtime_error("EVP_DigestSignFinal (size) failed (rsa_sign)");

    std::vector<unsigned char> sig(sig_len);
    if (EVP_DigestSignFinal(md_ctx.get(), sig.data(), &sig_len) != 1)
        throw std::runtime_error("EVP_DigestSignFinal failed (rsa_sign)");

    return std::string(reinterpret_cast<char *>(sig.data()), sig_len);
}

// ---- RSA verify: PKCS#1 v1.5 (RS256) or PSS (PS256) ----
// Mirrors rsa_sign — same padding split
bool rsa_verify(
    const std::string &data,
    const std::string &sig_b64,
    const EVP_PKEY *pub_key,
    int padding_type)
{
    std::string sig_bytes = b64url::decode(sig_b64);
    // RSA signature length = key size in bytes; OpenSSL validates internally,
    // so no explicit clamp needed here. Length mismatch surfaces as verify==0.

    EVP_MD_CTX_ptr md_ctx(EVP_MD_CTX_new());
    if (!md_ctx)
        return false;

    EVP_PKEY_CTX *pctx = nullptr;
    if (EVP_DigestVerifyInit(md_ctx.get(), &pctx, EVP_sha256(), nullptr,
                             const_cast<EVP_PKEY *>(pub_key)) != 1)
        return false;

    if (pctx)
    {
        if (EVP_PKEY_CTX_set_rsa_padding(pctx, padding_type) != 1)
            return false;
        if (padding_type == RSA_PKCS1_PSS_PADDING)
        {
            if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, RSA_PSS_SALTLEN_DIGEST) != 1)
                return false;
        }
    }

    if (EVP_DigestVerifyUpdate(md_ctx.get(), data.data(), data.size()) != 1)
        return false;

    int rc = EVP_DigestVerifyFinal(
        md_ctx.get(),
        reinterpret_cast<const unsigned char *>(sig_bytes.data()),
        sig_bytes.size());
    return (rc == 1);
}

} // anonymous namespace

// ============================================================
//  encode —— build signed "h.p.s"
// ============================================================
std::string encode(
    const std::string &header_json,
    const std::string &payload_json,
    const std::string &key,
    algorithm alg)
{
    // 0. both sections must be JSON objects, and HMAC must have a secret to use
    if (!is_json_object(header_json))
        throw std::invalid_argument("jwt encode: header_json is not a JSON object");
    if (!is_json_object(payload_json))
        throw std::invalid_argument("jwt encode: payload_json is not a JSON object");
    if (alg == algorithm::HS256 && key.empty())
        throw std::invalid_argument("jwt encode: empty HMAC secret");

    // 1. double-check header.alg matches requested algorithm
    // Header must declare alg, and it must equal what we plan to sign with
    const char *expected_alg_str = alg_to_string(alg);
    std::string header_alg;
    if (!find_top_string(header_json, "alg", header_alg) || header_alg != expected_alg_str)
        throw std::invalid_argument(
            std::string("jwt encode: header alg mismatch or missing, expected ") + expected_alg_str);

    // 2. inject "typ":"JWT" if caller didn't provide it
    std::string final_header = inject_typ(header_json);

    // 3. base64url encode header + payload
    std::string header_b64  = b64url::encode(final_header);
    std::string payload_b64 = b64url::encode(payload_json);
    std::string sign_input  = header_b64 + "." + payload_b64;

    // 4. sign per algorithm
    // One case per enum value: an algorithm nobody wrote a case for is refused instead of
    // falling into whatever the last branch happens to be.
    std::string signature;
    switch (alg)
    {
    case algorithm::HS256:
    {
        std::string mac_bytes = hmac_sha256::sign(key, sign_input);
        signature = b64url::encode(
            reinterpret_cast<const unsigned char *>(mac_bytes.data()),
            mac_bytes.size());
        break;
    }
    case algorithm::ES256:
    {
        EVP_PKEY_ptr priv_key;
        try
        {
            priv_key = ec_crypto::import_pem(key);
        }
        catch (...)
        {
            throw std::invalid_argument("jwt encode: failed to load EC private key from PEM");
        }
        signature = jws::es256_sign(sign_input, priv_key.get());
        break;
    }
    case algorithm::RS256:
    {
        EVP_PKEY_ptr priv_key;
        try
        {
            priv_key = ec_crypto::import_pem(key); // PEM_read_bio_PrivateKey auto-detects RSA
        }
        catch (...)
        {
            throw std::invalid_argument("jwt encode: failed to load RSA private key from PEM");
        }
        std::string sig_raw = rsa_sign(sign_input, priv_key.get(), RSA_PKCS1_PADDING);
        signature = b64url::encode(
            reinterpret_cast<const unsigned char *>(sig_raw.data()),
            sig_raw.size());
        break;
    }
    case algorithm::PS256:
    {
        EVP_PKEY_ptr priv_key;
        try
        {
            priv_key = ec_crypto::import_pem(key);
        }
        catch (...)
        {
            throw std::invalid_argument("jwt encode: failed to load RSA private key from PEM");
        }
        std::string sig_raw = rsa_sign(sign_input, priv_key.get(), RSA_PKCS1_PSS_PADDING);
        signature = b64url::encode(
            reinterpret_cast<const unsigned char *>(sig_raw.data()),
            sig_raw.size());
        break;
    }
    default:
        throw std::invalid_argument("jwt encode: unsupported algorithm");
    }

    return header_b64 + "." + payload_b64 + "." + signature;
}

// ============================================================
//  decode_without_verify —— base64url decode only, no checks
// ============================================================
verify_result decode_without_verify(const std::string &token)
{
    verify_result result;

    // 1. split into exactly 3 segments (signature may be empty for alg:none)
    auto parts = split_token(token);
    if (parts.size() != 3 ||
        parts[0].empty() || parts[1].empty())
    {
        result.err = verify_error::malformed_token;
        return result;
    }

    // 2. base64url decode header + payload
    if (!is_b64url_segment(parts[0]) || !is_b64url_segment(parts[1]))
    {
        result.err = verify_error::invalid_base64;
        return result;
    }
    std::string header_json  = b64url::decode(parts[0]);
    std::string payload_json = b64url::decode(parts[1]);

    // 3. extract alg + standard claims for caller convenience
    // This function parses and does not judge, so an unreadable claim reports 0.
    const claim_times claims = read_claims(payload_json);
    result.header_json  = header_json;
    result.payload_json = payload_json;
    find_top_string(header_json, "alg", result.alg);
    result.exp = claims.exp_as_time();
    result.iat = claims.iat_as_time();
    result.nbf = claims.nbf_as_time();
    // 不回 ok: 这里没有验签，ok 与 verify() 成功共用会让"只看 err"的调用方把未验签 token
    // 当合法。err 只表示"三段解析成功"。
    // Not ok: nothing was verified, and sharing ok with a successful verify() lets a caller
    // that only reads err treat an unverified token as authentic.
    result.err = verify_error::parsed_but_unverified;

    return result;
}

// ============================================================
//  verify —— decode + signature check + claim time check
// ============================================================
verify_result verify(
    const std::string &token,
    const std::string &key,
    algorithm expected_alg,
    int leeway_seconds)
{
    verify_result result;

    // ---- Step 1: split token (signature may be empty for alg:none) ----
    auto parts = split_token(token);
    if (parts.size() != 3 ||
        parts[0].empty() || parts[1].empty())
    {
        result.err = verify_error::malformed_token;
        return result;
    }
    const std::string &header_b64  = parts[0];
    const std::string &payload_b64 = parts[1];
    const std::string &sig_b64    = parts[2];

    // ---- Step 2: base64url decode ----
    if (!is_b64url_segment(header_b64) || !is_b64url_segment(payload_b64))
    {
        result.err = verify_error::invalid_base64;
        return result;
    }
    std::string header_json  = b64url::decode(header_b64);
    std::string payload_json = b64url::decode(payload_b64);

    // encode() 要求两段都是 JSON object，verify() 必须一致: payload 若是 [{"exp":...}]，exp
    // 落在 depth==2，顶层扫描器看不见，过期检查会静默消失。
    // Both sections must be JSON objects — the same gate encode() applies on the way in.
    // A payload of [{"exp":...}] puts exp at depth 2 where the top-level scanner never sees
    // it, and the expiry check silently disappears.
    if (!is_json_object(header_json) || !is_json_object(payload_json))
    {
        result.err = verify_error::malformed_json;
        return result;
    }

    // NOTE: decode_without_verify() 刻意不加此校验。它的定位是 "parse-only，供 gateway
    // 路由阶段读 header.alg 选 key"，header.alg 不依赖 payload 形状。
    // NOTE: decode_without_verify() intentionally skips this check — it exists so a gateway
    // can read header.alg and pick a key without caring whether the payload shape is valid.

    // ---- Step 3: alg whitelist check ----
    std::string alg;
    size_t      alg_hits = 0;
    find_top_string(header_json, "alg", alg, &alg_hits);
    if (alg.empty() || alg_hits > 1) // 两个 alg 同样是"谁生效没定论"，一并拒掉
    {
        result.err = verify_error::alg_mismatch;
        return result;
    }
    // alg:none explicit rejection — per RFC 7519 §6 Unsecured JWTs,
    // must reject when caller expects HS256/ES256
    if (alg == "none")
    {
        result.err = verify_error::alg_none_rejected;
        return result;
    }
    // HS/ES cross-attacker guard: header.alg must match expected algorithm
    if (alg != alg_to_string(expected_alg))
    {
        result.err = verify_error::alg_mismatch;
        return result;
    }

    // RFC 7515 §4.1.11: crit 列出的是"接收方必须理解才能接受"的扩展头。本实现不支持任何
    // 扩展头，所以 crit 一旦出现就必须拒绝——不解析它的内容，存在即拒。
    // Any crit entry names an extension the recipient must understand; this implementation
    // understands none, so the mere presence of the member ends the verification.
    std::string crit_raw;
    if (find_top_member(header_json, "crit", crit_raw))
    {
        result.err = verify_error::unsupported_critical;
        return result;
    }

    // ---- Step 4: standard claims ----
    // Read before the signature check so every error path reports the same fields. The
    // verdict on them is only reached once the signature has been checked.
    const claim_times claims = read_claims(payload_json);

    // ---- Step 5: signature verification ----
    std::string sign_input = header_b64 + "." + payload_b64;
    bool sig_ok = false;

    switch (expected_alg)
    {
    case algorithm::HS256:
    {
        // With an empty secret the MAC is public: anyone can sign a matching token
        if (key.empty())
        {
            result.err = verify_error::key_mismatch;
            return result;
        }
        // HS256: recompute HMAC, constant-time compare
        std::string recompute = hmac_sha256::sign(key, sign_input);
        std::string expected  = b64url::decode(sig_b64);
        sig_ok = constant_time_memcmp(recompute, expected);
        break;
    }
    case algorithm::ES256:
    {
        // ES256: load EC public key from PEM (pubkey format), run r||s→DER→verify
        EVP_PKEY_ptr pub_key;
        try
        {
            pub_key = import_pub_pem(key);
        }
        catch (...)
        {
            result.err = verify_error::key_mismatch;
            return result;
        }
        sig_ok = es256_verify(sign_input, sig_b64, pub_key.get());
        break;
    }
    case algorithm::RS256:
    {
        // RS256: RSA PKCS#1 v1.5 + SHA-256
        EVP_PKEY_ptr pub_key;
        try
        {
            pub_key = import_pub_pem(key); // PEM_read_bio_PUBKEY auto-detects RSA
        }
        catch (...)
        {
            result.err = verify_error::key_mismatch;
            return result;
        }
        sig_ok = rsa_verify(sign_input, sig_b64, pub_key.get(), RSA_PKCS1_PADDING);
        break;
    }
    case algorithm::PS256:
    {
        // PS256: RSA-PSS + SHA-256, salt=hash length
        EVP_PKEY_ptr pub_key;
        try
        {
            pub_key = import_pub_pem(key);
        }
        catch (...)
        {
            result.err = verify_error::key_mismatch;
            return result;
        }
        sig_ok = rsa_verify(sign_input, sig_b64, pub_key.get(), RSA_PKCS1_PSS_PADDING);
        break;
    }
    default:
        // The alg whitelist above only lets a declared algorithm through, so this is
        // reachable solely from an out-of-range enum value cast in by the caller.
        result.err = verify_error::key_mismatch;
        return result;
    }

    // Every path past this point reports the same diagnostics
    auto fill = [&](verify_error err) -> verify_result
    {
        result.err          = err;
        result.header_json  = header_json;
        result.payload_json = payload_json;
        result.alg          = alg;
        result.exp = claims.exp_as_time();
        result.iat = claims.iat_as_time();
        result.nbf = claims.nbf_as_time();
        return result;
    };

    if (!sig_ok)
        return fill(verify_error::signature_mismatch);

    // ---- Step 6: claim time validation ----
    // A claim that is there but unreadable is refused, not treated as "no limit" — otherwise
    // a quoted or null exp silently drops the expiry check.
    if (claims.has_invalid())
        return fill(verify_error::invalid_claims);

    // leeway 加在 now 上，不加在 claim 上: parse_date() 允许 exp 顶到 time_t 上限、nbf 顶到
    // 下限，所以 exp + leeway / nbf - leeway 是有符号溢出(UB)，理论上有符号溢出可能回绕成
    // 相反的判定(now 离边界极远, 不会溢出; 用 long long 计算是为了连 32 位 time_t 也安全)。
    // leeway is applied to `now`, never to the claim: parse_date() lets exp sit at time_t's
    // maximum and nbf at its minimum, so `exp + leeway` / `nbf - leeway` are signed overflows
    // (UB) that can wrap into the opposite verdict. `now` is a real clock reading, far from
    // the bounds; the arithmetic runs in long long so even a 32-bit time_t cannot overflow.
    const long long now_ll = static_cast<long long>(std::time(nullptr));

    // exp check: token expired if exp < (now - leeway)
    if (claims.exp_state == claim_state::present &&
        now_ll - static_cast<long long>(leeway_seconds) >
            static_cast<long long>(claims.exp_as_time()))
        return fill(verify_error::token_expired);

    // nbf check: token not yet valid if nbf > (now + leeway)
    if (claims.nbf_state == claim_state::present &&
        now_ll + static_cast<long long>(leeway_seconds) <
            static_cast<long long>(claims.nbf_as_time()))
        return fill(verify_error::token_not_yet_valid);

    // ---- Step 7: all checks passed ----
    return fill(verify_error::ok);
}

} // namespace jwt
} // namespace http
