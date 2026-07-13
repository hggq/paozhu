#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>

#ifdef WIN32
#define stat _stat
#endif

#include "terminal_color.h"
#include "serverconfig.h"
#include "http2_define.h"
#include "http_domain.h"
#include "server_localvar.h"
#include "crypto_ptr.h"
#include "httpclient.h"

namespace http
{

std::vector<unsigned char> get_default_alpn()
{
    std::vector<unsigned char> res = {0x02, 'h', '2', 0x08, 'h', 't', 't', 'p', '/', '1', '.', '1'};// \x02h2\x08http/1.1"};
    //    std::vector<unsigned char>({});
    // auto p = std::begin(res);

    // p = std::copy_n(std::begin(PZ_HTTP2_H2_ALPN), PZ_HTTP2_H2_ALPN.size(), p);
    // p = std::copy_n(std::begin(PZ_HTTP2_H2_16_ALPN), PZ_HTTP2_H2_16_ALPN.size(), p);
    // p = std::copy_n(std::begin(PZ_HTTP2_H2_14_ALPN), PZ_HTTP2_H2_14_ALPN.size(), p);

    return res;
}

int next_proto_cb(SSL *s, const unsigned char **data, unsigned int *len, void *arg)
{
    //auto next_proto = static_cast<std::vector<unsigned char> *>(arg);
    auto next_proto = static_cast<unsigned char *>(arg);
    *data           = next_proto;
    if (next_proto[0] == 0x02)
    {
        *len = 12;
    }
    else
    {
        *len = 0;
    }
    //*data         = next_proto->data();
    //*len            = next_proto->size();
    if (true || s)
    {
    }
    return SSL_TLSEXT_ERR_OK;
}

int alpn_cb([[maybe_unused]] SSL *ssl,
            const unsigned char **out,
            unsigned char *outlen,
            const unsigned char *in,
            unsigned int inlen,
            void *arg)
{

    int ret = SSL_TLSEXT_ERR_NOACK;
    *out    = NULL;
    *outlen = 0;

    serverconfig &sysconfigpath = getserversysconfig();
    bool isusehttp2             = true;

    unsigned long long host_index = (unsigned long long)arg;
    if (host_index < sysconfigpath.sitehostinfos.size())
    {
        isusehttp2 = sysconfigpath.sitehostinfos[host_index].http2_enable;
    }

    host_index       = 0;
    unsigned char *p = (unsigned char *)in;
    while (inlen > 0 && in && (p - in) < inlen)
    {
        int len = p[0];
        p++;
        if (len == 2 && p[1] == '2')// && memcmp(p, "h2", 2) == 0)
        {
            host_index = host_index + 10;
        }
        else if (len == 8)//&& memcmp(p, "http/1.1", 8) == 0)
        {
            host_index = host_index + 1;
        }
        p = p + len;
    }

    unsigned int protos_len;
    const char *protos;
    unsigned char *tmp_out;

    if (isusehttp2 && host_index > 9)
    {
        protos     = HTTP2_H2_ALPN HTTP1_NPN;
        protos_len = sizeof(HTTP2_H2_ALPN HTTP1_NPN) - 1;
    }
    else if (host_index > 0)
    {
        protos     = HTTP1_NPN;
        protos_len = sizeof(HTTP1_NPN) - 1;
    }
    if (host_index > 0)
    {
        ret  = SSL_select_next_proto(&tmp_out, outlen, (const unsigned char *)protos, protos_len, in, inlen);
        *out = tmp_out;
    }

    ret = SSL_TLSEXT_ERR_OK;

    return ret;
}

std::string get_password() { return "sslpem123456"; }
int serverNameCallback(SSL *ssl, [[maybe_unused]] int *ad, [[maybe_unused]] void *arg)
{
    if (ssl == NULL)
        return SSL_TLSEXT_ERR_NOACK;

    const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
    if (servername == NULL)
    {
        //only ip
        return SSL_TLSEXT_ERR_ALERT_WARNING;
    }
    SSL_CTX *ctx                = NULL;
    serverconfig &sysconfigpath = getserversysconfig();

    if (sysconfigpath.checkmaindomain(servername))
    {
        //same hostname
        if (SSL_set_cipher_list(ssl, DEFAULT_CIPHER_LIST) == 0)
        {
            return SSL_TLSEXT_ERR_ALERT_FATAL;
        }
        return SSL_TLSEXT_ERR_OK;
    }

    if (servername && strlen(servername) > 0)
    {
        ctx = sysconfigpath.getctx(servername);
    }

    if (ctx == NULL)
    {
        return SSL_TLSEXT_ERR_ALERT_FATAL;
    }
    SSL_set_SSL_CTX(ssl, ctx);

    SSL_set_verify(ssl, SSL_CTX_get_verify_mode(ctx), SSL_CTX_get_verify_callback(ctx));

    SSL_set_verify_depth(ssl, SSL_CTX_get_verify_depth(ctx));
    SSL_clear_options(ssl, 0xFFFFFFFFL);
    SSL_set_options(ssl, SSL_CTX_get_options(ctx));

    if (SSL_set_cipher_list(ssl, DEFAULT_CIPHER_LIST) == 0)
    {
        return SSL_TLSEXT_ERR_ALERT_FATAL;
    }

    unsigned long long temp_domain = sysconfigpath.get_hostindex(servername);

    SSL_CTX_set_alpn_select_cb(ctx, alpn_cb, (void *)temp_domain);
    return SSL_TLSEXT_ERR_OK;
}

cert_validity_t get_cert_validity(const std::string &cert_path)
{
    // 1. Open certificate file
    BIO_ptr bio(BIO_new_file(cert_path.c_str(), "r"));
    if (!bio)
    {
        throw std::runtime_error("Failed to open certificate file: " + cert_path);
    }

    // 2. Parse PEM-encoded X.509 certificate
    X509_ptr cert(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
    if (!cert)
    {
        unsigned long err = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        throw std::runtime_error("Failed to parse certificate: " + std::string(err_buf));
    }

    // 3. Extract ASN1_TIME pointers (non-owning, valid as long as cert lives)
    const ASN1_TIME *nb = X509_get0_notBefore(cert.get());
    const ASN1_TIME *na = X509_get0_notAfter(cert.get());
    if (!nb || !na)
    {
        throw std::runtime_error("Certificate missing validity fields");
    }

    // 4. Convert ASN1_TIME -> struct tm -> time_t
    struct tm tm_before{}, tm_after{};
    if (!ASN1_TIME_to_tm(nb, &tm_before))
    {
        throw std::runtime_error("Failed to convert notBefore to tm");
    }
    if (!ASN1_TIME_to_tm(na, &tm_after))
    {
        throw std::runtime_error("Failed to convert notAfter to tm");
    }

    // Use timegm() because ASN1_TIME is always UTC
    time_t t_before = timegm(&tm_before);
    time_t t_after  = timegm(&tm_after);

    // 5. Calculate remaining days and expiration status
    time_t now          = time(nullptr);
    double seconds_left = difftime(t_after, now);
    double days_left    = seconds_left / 86400.0;

    unsigned int expired_day = days_left > 0 ? days_left : 0;

    return {
        /* .not_before     = */ t_before,
        /* .not_after      = */ t_after,
        /* .days_remaining = */ expired_day,
        /* .is_expired     = */ (now >= t_after)};
}

eab_credentials_t fetch_eab_by_email(std::string_view url, std::string_view email)
{
    //"Step 0: 通过邮箱申请 EAB 凭证..."
    //post https://api.zerossl.com/acme/eab-credentials-email
    std::string body = "email=";
    body.append(email);

    std::shared_ptr<http::client> https = std::make_shared<http::client>();
    https->post(url);
    https->add_header("Content-Type", "application/x-www-form-urlencoded");
    https->add_header("Connection", "close");
    https->add_header("User-Agent", "ZeroSSL-Asio-ACME/1.0");
    https->timeout(60);
    https->set_body(body);
    https->send();

    if (https->get_status() != 200)
        throw std::runtime_error("EAB 请求失败: HTTP " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());

    eab_credentials_t eab;
    if (https->page.isjson == 1)
    {
        if (https->page.json["success"].to_bool())
        {
            eab.eab_kid      = https->page.json["eab_kid"].to_string();
            eab.eab_hmac_key = https->page.json["eab_hmac_key"].to_string();
        }
        else
        {
            throw std::runtime_error("EAB 申请失败:\n" + https->get_body());
        }
    }
    else
    {
        throw std::runtime_error("EAB 请求失败: JSON " + std::to_string(https->get_status()) + "\n响应: " + https->get_body());
    }

    if (eab.eab_kid.empty() || eab.eab_hmac_key.empty())
    {
        throw std::runtime_error("EAB 响应缺少字段:\n" + https->get_body());
    }
    return eab;
}

}// namespace http