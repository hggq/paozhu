/**
 * acme_crypto.hpp
 *
 * OpenSSL 3.0+ 加密工具集：Base64url 编解码、SHA-256、EVP_PKEY 密钥管理、
 * JWS ES256 签名、JWK Thumbprint、HMAC-SHA256、CSR 生成
 */
#pragma once

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/ecdsa.h>// d2i_ECDSA_SIG, ECDSA_SIG_get0 (3.0 仍可用)
#include <openssl/core_names.h>
// EVP_MAC API 已包含在 <openssl/evp.h> 中

#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include "crypto_ptr.h"

namespace http
{
// ============================================================
//  Base64url 编解码
// ============================================================
namespace b64url
{

inline std::string encode(const unsigned char *data, size_t len)
{
    BIO_ptr b64(BIO_new(BIO_f_base64()));
    BIO_ptr mem(BIO_new(BIO_s_mem()));
    BIO_push(b64.get(), mem.get());
    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64.get(), data, static_cast<int>(len));
    BIO_flush(b64.get());

    BUF_MEM *buf = nullptr;
    BIO_get_mem_ptr(b64.get(), &buf);

    std::string result(buf->data, buf->length);
    for (auto &c : result)
    {
        if (c == '+')
            c = '-';
        else if (c == '/')
            c = '_';
    }
    while (!result.empty() && result.back() == '=')
        result.pop_back();
    return result;
}

inline std::string encode(const std::string &s)
{
    return encode(reinterpret_cast<const unsigned char *>(s.data()), s.size());
}

inline std::string decode(const std::string &input)
{
    std::string b64str = input;
    for (auto &c : b64str)
    {
        if (c == '-')
            c = '+';
        else if (c == '_')
            c = '/';
    }
    while (b64str.size() % 4 != 0)
        b64str += '=';

    BIO_ptr b64(BIO_new(BIO_f_base64()));
    BIO_ptr mem(BIO_new_mem_buf(b64str.data(), static_cast<int>(b64str.size())));
    BIO_push(b64.get(), mem.get());
    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);

    std::string result(b64str.size(), '\0');
    int n = BIO_read(b64.get(), result.data(), static_cast<int>(result.size()));
    if (n < 0)
        n = 0;
    result.resize(static_cast<size_t>(n));
    return result;
}

}// namespace b64url

// ============================================================
//  SHA-256
// ============================================================
namespace sha256
{

inline std::string hash(const std::string &input)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    size_t digest_len = 0;
    if (EVP_Q_digest(nullptr, "SHA256", nullptr, input.data(), input.size(), digest, &digest_len) != 1)
        throw std::runtime_error("EVP_Q_digest SHA256 failed");
    return std::string(reinterpret_cast<char *>(digest), digest_len);
}

}// namespace sha256

// ============================================================
//  HMAC-SHA256 (OpenSSL 3.0 EVP_MAC API)
// ============================================================
namespace hmac_sha256
{

inline std::string sign(const std::string &key, const std::string &data)
{
    EVP_MAC_ptr mac(EVP_MAC_fetch(nullptr, "HMAC", nullptr));
    if (!mac)
        throw std::runtime_error("EVP_MAC_fetch HMAC failed");

    EVP_MAC_CTX_ptr ctx(EVP_MAC_CTX_new(mac.get()));
    if (!ctx)
        throw std::runtime_error("EVP_MAC_CTX_new failed");

    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest",
                                                 const_cast<char *>("SHA256"),
                                                 0);
    params[1] = OSSL_PARAM_construct_end();

    if (EVP_MAC_init(ctx.get(),
                     reinterpret_cast<const unsigned char *>(key.data()),
                     key.size(),
                     params) != 1)
        throw std::runtime_error("EVP_MAC_init failed");

    if (EVP_MAC_update(ctx.get(),
                       reinterpret_cast<const unsigned char *>(data.data()),
                       data.size()) != 1)
        throw std::runtime_error("EVP_MAC_update failed");

    unsigned char mac_buf[EVP_MAX_MD_SIZE];
    size_t mac_len = 0;
    if (EVP_MAC_final(ctx.get(), mac_buf, &mac_len, sizeof(mac_buf)) != 1)
        throw std::runtime_error("EVP_MAC_final failed");

    return std::string(reinterpret_cast<char *>(mac_buf), mac_len);
}

}// namespace hmac_sha256

// ============================================================
//  EC P-256 密钥操作 (OpenSSL 3.0 EVP_PKEY API)
// ============================================================
namespace ec_crypto
{

// 生成 EC P-256 密钥对 (3.0: EVP_PKEY_Q_keygen 替代 EC_KEY_new_by_curve_name)
inline EVP_PKEY_ptr generate_p256()
{
    EVP_PKEY *raw = EVP_PKEY_Q_keygen(nullptr, nullptr, "EC", "P-256");
    if (!raw)
        throw std::runtime_error("EVP_PKEY_Q_keygen EC P-256 failed");
    return EVP_PKEY_ptr(raw);
}

// 获取公钥 x, y 坐标 (base64url)
// 3.0: EVP_PKEY_get_bn_param 获取 BIGNUM，再转为固定 32 字节
inline void get_pub_xy(const EVP_PKEY *key, std::string &x_b64, std::string &y_b64)
{
    BIGNUM *x_bn = nullptr;
    BIGNUM *y_bn = nullptr;

    if (EVP_PKEY_get_bn_param(key, OSSL_PKEY_PARAM_EC_PUB_X, &x_bn) != 1 || !x_bn)
        throw std::runtime_error("get pub-x failed");
    if (EVP_PKEY_get_bn_param(key, OSSL_PKEY_PARAM_EC_PUB_Y, &y_bn) != 1 || !y_bn)
    {
        BN_free(x_bn);
        throw std::runtime_error("get pub-y failed");
    }

    // BIGNUM -> 固定 32 字节 (左补零)
    auto bn_to_fixed = [](const BIGNUM *bn) -> std::string
    {
        std::string out(32, '\0');
        int n   = BN_num_bytes(bn);
        int pad = 32 - n;
        if (pad < 0)
            throw std::runtime_error("BN too large");
        BN_bn2bin(bn, reinterpret_cast<unsigned char *>(const_cast<char *>(out.data())) + pad);
        return out;
    };

    std::string x_bytes = bn_to_fixed(x_bn);
    std::string y_bytes = bn_to_fixed(y_bn);
    BN_free(x_bn);
    BN_free(y_bn);

    x_b64 = b64url::encode(x_bytes);
    y_b64 = b64url::encode(y_bytes);
}

// 将 EVP_PKEY 导出为 PEM 私钥字符串
// 3.0: PEM_write_bio_PrivateKey 替代 PEM_write_bio_ECPrivateKey
inline std::string export_pem(const EVP_PKEY *key)
{
    BIO_ptr mem(BIO_new(BIO_s_mem()));
    PEM_write_bio_PrivateKey(mem.get(),
                             const_cast<EVP_PKEY *>(key),
                             nullptr,
                             nullptr,
                             0,
                             nullptr,
                             nullptr);
    char *data = nullptr;
    long len   = BIO_get_mem_data(mem.get(), &data);
    return std::string(data, static_cast<size_t>(len));
}

// 从 PEM 私钥字符串加载 EVP_PKEY
inline EVP_PKEY_ptr import_pem(const std::string &pem)
{
    BIO_ptr bio(BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size())));
    if (!bio)
        throw std::runtime_error("BIO_new_mem_buf failed");

    EVP_PKEY *raw = PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr);
    if (!raw)
        throw std::runtime_error("PEM_read_bio_PrivateKey failed");
    return EVP_PKEY_ptr(raw);
}

}// namespace ec_crypto

// ============================================================
//  JWS 签名 (ES256 - ECDSA P-256 + SHA-256)
// ============================================================
namespace jws
{

// ES256 签名 (3.0: EVP_DigestSign 替代 ECDSA_do_sign)
// 返回 r||s 拼接的 base64url (JWS 格式)
inline std::string es256_sign(const std::string &data, const EVP_PKEY *key)
{
    // 1. EVP_DigestSign: SHA-256 + ECDSA -> DER 签名
    EVP_MD_CTX_ptr md_ctx(EVP_MD_CTX_new());
    if (!md_ctx)
        throw std::runtime_error("EVP_MD_CTX_new failed");

    if (EVP_DigestSignInit(md_ctx.get(), nullptr, EVP_sha256(), nullptr, const_cast<EVP_PKEY *>(key)) != 1)
        throw std::runtime_error("EVP_DigestSignInit failed");

    if (EVP_DigestSignUpdate(md_ctx.get(),
                             data.data(),
                             data.size()) != 1)
        throw std::runtime_error("EVP_DigestSignUpdate failed");

    size_t der_len = 0;
    if (EVP_DigestSignFinal(md_ctx.get(), nullptr, &der_len) != 1)
        throw std::runtime_error("EVP_DigestSignFinal (size) failed");

    std::vector<unsigned char> der_sig(der_len);
    if (EVP_DigestSignFinal(md_ctx.get(), der_sig.data(), &der_len) != 1)
        throw std::runtime_error("EVP_DigestSignFinal failed");
    der_sig.resize(der_len);

    // 2. 解析 DER 签名为 ECDSA_SIG，提取 r, s
    const unsigned char *p = der_sig.data();
    ECDSA_SIG_ptr sig(d2i_ECDSA_SIG(nullptr, &p, static_cast<long>(der_sig.size())));
    if (!sig)
        throw std::runtime_error("d2i_ECDSA_SIG failed");

    const BIGNUM *r_bn = nullptr;
    const BIGNUM *s_bn = nullptr;
    ECDSA_SIG_get0(sig.get(), &r_bn, &s_bn);

    // 3. r, s -> 固定 32 字节 (左补零)
    auto bn_to_fixed = [](const BIGNUM *bn) -> std::string
    {
        std::string out(32, '\0');
        int n   = BN_num_bytes(bn);
        int pad = 32 - n;
        if (pad < 0)
            throw std::runtime_error("BN too large");
        BN_bn2bin(bn, reinterpret_cast<unsigned char *>(const_cast<char *>(out.data())) + pad);
        return out;
    };

    std::string r_bytes = bn_to_fixed(r_bn);
    std::string s_bytes = bn_to_fixed(s_bn);

    // 4. r || s (64 字节)
    std::string rs(64, '\0');
    std::memcpy(rs.data(), r_bytes.data(), 32);
    std::memcpy(rs.data() + 32, s_bytes.data(), 32);

    return b64url::encode(
        reinterpret_cast<const unsigned char *>(rs.data()),
        rs.size());
}

// 构造 JWK JSON (EC P-256 公钥)
inline std::string make_jwk(const EVP_PKEY *key)
{
    std::string x_b64, y_b64;
    ec_crypto::get_pub_xy(key, x_b64, y_b64);
    return R"({"crv":"P-256","kty":"EC","x":")" + x_b64 + R"(","y":")" + y_b64 + R"("})";
}

// 计算 JWK Thumbprint (RFC 7638)
inline std::string jwk_thumbprint(const EVP_PKEY *key)
{
    std::string x_b64, y_b64;
    ec_crypto::get_pub_xy(key, x_b64, y_b64);

    // 按字母顺序排列字段: crv, kty, x, y
    auto canonical     = R"({"crv":"P-256","kty":"EC","x":")" + x_b64 + R"(","y":")" + y_b64 + R"("})";
    std::string digest = sha256::hash(canonical);
    return b64url::encode(
        reinterpret_cast<const unsigned char *>(digest.data()),
        digest.size());
}

// 构造完整 JWS body (ACME signed request)
inline std::string make_signed_body(
    const std::string &payload,
    const std::string &url,
    const std::string &nonce,
    const EVP_PKEY *key,
    const std::string &kid = "")
{
    std::string protected_json;
    if (kid.empty())
    {
        protected_json = R"({"alg":"ES256","jwk":)" + make_jwk(key) +
                         R"(,"nonce":")" + nonce + R"(","url":")" + url + R"("})";
    }
    else
    {
        protected_json = R"({"alg":"ES256","kid":")" + kid +
                         R"(","nonce":")" + nonce + R"(","url":")" + url + R"("})";
    }

    std::string protected_b64 = b64url::encode(protected_json);
    std::string payload_b64   = b64url::encode(payload);
    std::string sign_input    = protected_b64 + "." + payload_b64;
    std::string signature     = es256_sign(sign_input, key);

    return R"({"protected":")" + protected_b64 + R"(","payload":")" + payload_b64 + R"(","signature":")" + signature + R"("})";
}

// EAB JWS (HS256 签名, 3.0 EVP_MAC API)
inline std::string make_eab_jws(
    const std::string &account_jwk_json,
    const std::string &kid,
    const std::string &hmac_key_b64,
    const std::string &url)
{
    auto protected_json = R"({"alg":"HS256","kid":")" + kid + R"(","url":")" + url + R"("})";

    std::string protected_b64 = b64url::encode(protected_json);
    std::string payload_b64   = b64url::encode(account_jwk_json);
    std::string sign_input    = protected_b64 + "." + payload_b64;

    std::string hmac_key  = b64url::decode(hmac_key_b64);
    std::string mac_bytes = hmac_sha256::sign(hmac_key, sign_input);
    std::string signature = b64url::encode(mac_bytes);

    return R"({"protected":")" + protected_b64 + R"(","payload":")" + payload_b64 + R"(","signature":")" + signature + R"("})";
}

}// namespace jws

// ============================================================
//  CSR 生成 (EC P-256, EVP_PKEY)
// ============================================================
namespace csr
{

inline std::string generate_csr_pem(EVP_PKEY *pkey,
                                    const std::vector<std::string> &domains)
{
    X509_REQ_ptr req(X509_REQ_new());
    if (!req)
        throw std::runtime_error("X509_REQ_new failed");

    X509_REQ_set_version(req.get(), 0);
    X509_REQ_set_pubkey(req.get(), pkey);

    X509_NAME *name = X509_REQ_get_subject_name(req.get());
    if (!domains.empty())
    {
        X509_NAME_add_entry_by_txt(
            name,
            "CN",
            MBSTRING_ASC,
            reinterpret_cast<const unsigned char *>(domains[0].c_str()),
            -1,
            -1,
            0);
    }

    if (!domains.empty())
    {
        std::string san_str;
        for (size_t i = 0; i < domains.size(); ++i)
        {
            if (i > 0)
                san_str += ",";
            san_str += "DNS:" + domains[i];
        }

        STACK_OF(X509_EXTENSION) *exts = sk_X509_EXTENSION_new_null();
        X509_EXTENSION *ext            = X509V3_EXT_conf_nid(
            nullptr,
            nullptr,
            NID_subject_alt_name,
            const_cast<char *>(san_str.c_str()));
        sk_X509_EXTENSION_push(exts, ext);
        X509_REQ_add_extensions(req.get(), exts);
        sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
    }

    if (X509_REQ_sign(req.get(), pkey, EVP_sha256()) == 0)
        throw std::runtime_error("X509_REQ_sign failed");

    BIO_ptr mem(BIO_new(BIO_s_mem()));
    PEM_write_bio_X509_REQ(mem.get(), req.get());
    char *data = nullptr;
    long len   = BIO_get_mem_data(mem.get(), &data);
    return std::string(data, static_cast<size_t>(len));
}

inline std::string pem_to_der_b64url(const std::string &pem)
{
    BIO_ptr pem_bio(BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size())));
    X509_REQ_ptr req(PEM_read_bio_X509_REQ(pem_bio.get(), nullptr, nullptr, nullptr));
    if (!req)
        throw std::runtime_error("Failed to parse CSR PEM");

    BIO_ptr der_bio(BIO_new(BIO_s_mem()));
    i2d_X509_REQ_bio(der_bio.get(), req.get());
    char *data = nullptr;
    long len   = BIO_get_mem_data(der_bio.get(), &data);
    return b64url::encode(reinterpret_cast<const unsigned char *>(data),
                          static_cast<size_t>(len));
}

}// namespace csr

// ============================================================
//  简易 JSON 字段提取
// ============================================================
namespace json_util
{

inline std::string get_string(const std::string &json, const std::string &key)
{
    std::string search = "\"" + key + "\"";
    auto pos           = json.find(search);
    if (pos == std::string::npos)
        return {};
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos)
        return {};
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos)
        return {};
    auto end = json.find('"', pos + 1);
    if (end == std::string::npos)
        return {};
    return json.substr(pos + 1, end - pos - 1);
}

inline int get_int(const std::string &json, const std::string &key, int def = 0)
{
    std::string search = "\"" + key + "\"";
    auto pos           = json.find(search);
    if (pos == std::string::npos)
        return def;
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos)
        return def;
    pos++;
    while (pos < json.size() && std::isspace(json[pos]))
        pos++;
    try
    {
        return std::stoi(json.substr(pos));
    }
    catch (...)
    {
        return def;
    }
}

inline float get_float(const std::string &json, const std::string &key, float def = 0.0)
{
    std::string search = "\"" + key + "\"";
    auto pos           = json.find(search);
    if (pos == std::string::npos)
        return def;
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos)
        return def;
    pos++;
    while (pos < json.size() && std::isspace(json[pos]))
        pos++;
    try
    {
        return std::stof(json.substr(pos));
    }
    catch (...)
    {
        return def;
    }
}

inline bool get_bool(const std::string &json, const std::string &key, bool def = false)
{
    std::string search = "\"" + key + "\"";
    auto pos           = json.find(search);
    if (pos == std::string::npos)
        return def;
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos)
        return def;
    ++pos;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos])))
        ++pos;
    if (json.compare(pos, 4, "true") == 0)
        return true;
    if (json.compare(pos, 5, "false") == 0)
        return false;
    return def;
}

inline std::string find_in_array(const std::string &json,
                                 const std::string &array_key,
                                 const std::string &match_key,
                                 const std::string &match_val)
{
    auto arr_pos = json.find("\"" + array_key + "\"");
    if (arr_pos == std::string::npos)
        return {};
    auto bracket = json.find('[', arr_pos);
    if (bracket == std::string::npos)
        return {};
    size_t pos = bracket + 1;
    while (pos < json.size())
    {
        auto obj_start = json.find('{', pos);
        if (obj_start == std::string::npos)
            break;
        int depth      = 1;
        size_t obj_end = obj_start + 1;
        while (obj_end < json.size() && depth > 0)
        {
            if (json[obj_end] == '{')
                depth++;
            else if (json[obj_end] == '}')
                depth--;
            obj_end++;
        }
        std::string obj = json.substr(obj_start, obj_end - obj_start);
        if (obj.find("\"" + match_key + "\"") != std::string::npos &&
            obj.find("\"" + match_val + "\"") != std::string::npos)
            return obj;
        pos = obj_end;
        if (json.find(']', obj_start) < obj_end)
            break;
    }
    return {};
}

}// namespace json_util

}// namespace http