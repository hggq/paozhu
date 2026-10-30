// AES-256-GCM 与 last_error 的落库前过滤。
// OpenSSL 3 的调用顺序照 vendor/webpay/src/wxpay.cpp 的 aesGcmDecrypt（那一处入参是
// base64 密文 + 独立 nonce + 微信特有的 AAD，格式不同，只有 API 调用次序可参考）。
#include "github_crypto.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <cstring>
#include <memory>
#include <stdexcept>

#include "cost_define.h"

namespace http
{
namespace github
{

namespace
{
struct EVP_CIPHER_CTX_del
{
    void operator()(EVP_CIPHER_CTX *p) { EVP_CIPHER_CTX_free(p); }
};
using EVP_CIPHER_CTX_ptr = std::unique_ptr<EVP_CIPHER_CTX, EVP_CIPHER_CTX_del>;

constexpr int GH_GCM_IV_LEN  = 12;
constexpr int GH_GCM_TAG_LEN = 16;

// "键名=值"形态的泄露点：值一直吃到下一个分隔符。
const char *const GH_PAIR_MARKERS[] = {"client_secret=", "access_token=", "refresh_token="};
// GitHub 令牌前缀本身就带在值里（gho_…、ghr_…），单独按前缀吃一遍。
const char *const GH_PREFIX_MARKERS[] = {"gho_", "ghp_", "ghu_", "ghs_", "ghr_"};

bool is_text_break(char c)
{
    switch (c)
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case ',':
    case ';':
    case '}':
    case ']':
    case '"':
    case '\'':
    case '&':
        return true;
    default:
        return false;
    }
}

// 从 in 的 pos 起吃掉一段值（pos 已经指向 marker 之后的第一个字符），返回吃掉之后的位置。
size_t eat_value(const std::string &in, size_t pos)
{
    while (pos < in.size() && !is_text_break(in[pos]))
    {
        pos++;
    }
    return pos;
}
}// namespace

bool crypto_key_from_hex(const std::string &hex, std::string &key_out)
{
    key_out.clear();
    if (hex.size() != CONST_GH_CRYPTO_KEY_LEN * 2)
    {
        return false;
    }
    std::string bytes;
    bytes.reserve(CONST_GH_CRYPTO_KEY_LEN);
    for (size_t i = 0; i < hex.size(); i += 2)
    {
        int nib[2];
        for (int k = 0; k < 2; k++)
        {
            char c = hex[i + k];
            if (c >= '0' && c <= '9')
                nib[k] = c - '0';
            else if (c >= 'a' && c <= 'f')
                nib[k] = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F')
                nib[k] = c - 'A' + 10;
            else
            {
                key_out.clear();
                return false;
            }
        }
        bytes.push_back((char)((nib[0] << 4) | nib[1]));
    }
    key_out = std::move(bytes);
    return true;
}

std::string gcm_encrypt(const std::string &plaintext, const std::string &key)
{
    if (key.size() != CONST_GH_CRYPTO_KEY_LEN)
    {
        throw std::runtime_error("github gcm: key must be exactly 32 bytes");
    }
    unsigned char iv[GH_GCM_IV_LEN];
    if (RAND_bytes(iv, GH_GCM_IV_LEN) != 1)
    {
        throw std::runtime_error("github gcm: RAND_bytes failed");
    }

    std::string out;
    out.assign(reinterpret_cast<const char *>(iv), GH_GCM_IV_LEN);
    out.append(std::string(GH_GCM_TAG_LEN + plaintext.size(), '\0'));

    unsigned char *tagp = reinterpret_cast<unsigned char *>(out.data()) + GH_GCM_IV_LEN;
    unsigned char *outp = tagp + GH_GCM_TAG_LEN;

    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
    if (!ctx)
    {
        throw std::runtime_error("github gcm: EVP_CIPHER_CTX_new failed");
    }
    int outlen = 0;
    int finlen = 0;
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
    {
        throw std::runtime_error("github gcm: EncryptInit failed");
    }
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, GH_GCM_IV_LEN, nullptr) != 1)
    {
        throw std::runtime_error("github gcm: SET_IVLEN failed");
    }
    if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, reinterpret_cast<const unsigned char *>(key.data()), iv) != 1)
    {
        throw std::runtime_error("github gcm: key/iv init failed");
    }
    if (EVP_EncryptUpdate(ctx.get(), outp, &outlen, reinterpret_cast<const unsigned char *>(plaintext.data()), (int)plaintext.size()) != 1)
    {
        throw std::runtime_error("github gcm: EncryptUpdate failed");
    }
    if (EVP_EncryptFinal_ex(ctx.get(), outp + outlen, &finlen) != 1)
    {
        throw std::runtime_error("github gcm: EncryptFinal failed");
    }
    outlen += finlen;
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, GH_GCM_TAG_LEN, tagp) != 1)
    {
        throw std::runtime_error("github gcm: GET_TAG failed");
    }
    out.resize((size_t)GH_GCM_IV_LEN + GH_GCM_TAG_LEN + (size_t)outlen);
    return out;
}

std::string gcm_decrypt(const std::string &cipher, const std::string &key)
{
    if (key.size() != CONST_GH_CRYPTO_KEY_LEN)
    {
        throw std::runtime_error("github gcm: key must be exactly 32 bytes");
    }
    if (cipher.size() < (size_t)GH_GCM_IV_LEN + GH_GCM_TAG_LEN)
    {
        throw std::runtime_error("github gcm: stored blob shorter than IV+tag");
    }
    const unsigned char *base = reinterpret_cast<const unsigned char *>(cipher.data());
    size_t clen               = cipher.size() - (size_t)GH_GCM_IV_LEN - (size_t)GH_GCM_TAG_LEN;

    std::string out(clen + 1, '\0');
    unsigned char *outp = reinterpret_cast<unsigned char *>(out.data());

    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
    if (!ctx)
    {
        throw std::runtime_error("github gcm: EVP_CIPHER_CTX_new failed");
    }
    int outlen = 0;
    int finlen = 0;
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
    {
        throw std::runtime_error("github gcm: DecryptInit failed");
    }
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, GH_GCM_IV_LEN, nullptr) != 1)
    {
        throw std::runtime_error("github gcm: SET_IVLEN failed");
    }
    if (EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, reinterpret_cast<const unsigned char *>(key.data()), base) != 1)
    {
        throw std::runtime_error("github gcm: key/iv init failed");
    }
    if (EVP_DecryptUpdate(ctx.get(), outp, &outlen, base + GH_GCM_IV_LEN + GH_GCM_TAG_LEN, (int)clen) != 1)
    {
        throw std::runtime_error("github gcm: DecryptUpdate failed");
    }
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, GH_GCM_TAG_LEN, (void *)(base + GH_GCM_IV_LEN)) != 1)
    {
        throw std::runtime_error("github gcm: SET_TAG failed");
    }
    // tag 不过就是"这串密文被改过"，绝不能只是返回空串让调用方以为"没数据"。
    if (EVP_DecryptFinal_ex(ctx.get(), outp + outlen, &finlen) != 1)
    {
        throw std::runtime_error("github gcm: tag check failed");
    }
    out.resize((size_t)outlen + (size_t)finlen);
    return out;
}

std::string scrub_error_text(const std::string &in)
{
    std::string out;
    out.reserve(in.size());
    size_t i = 0;
    while (i < in.size())
    {
        bool hit = false;
        for (const char *m : GH_PAIR_MARKERS)
        {
            size_t mlen = std::strlen(m);
            if (in.compare(i, mlen, m) == 0)
            {
                out.append(m);
                out.append("[redacted]");
                i   = eat_value(in, i + mlen);
                hit = true;
                break;
            }
        }
        if (hit)
        {
            continue;
        }
        for (const char *m : GH_PREFIX_MARKERS)
        {
            size_t mlen = std::strlen(m);
            if (in.compare(i, mlen, m) == 0)
            {
                out.append("[redacted]");
                i   = eat_value(in, i + mlen);
                hit = true;
                break;
            }
        }
        if (hit)
        {
            continue;
        }
        out.push_back(in[i]);
        i++;
    }
    if (out.size() > CONST_GH_ERROR_MAX_LEN)
    {
        out.resize(CONST_GH_ERROR_MAX_LEN);
    }
    return out;
}

}// namespace github
}// namespace http
