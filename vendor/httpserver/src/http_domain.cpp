#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#ifdef ENABLE_VCPKG
#include <openssl/ssl.h>
#endif

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

#include "terminal_color.h"
#include "serverconfig.h"
#include "http2_define.h"
#include "http_domain.h"
#include "server_localvar.h"

namespace http
{

std::vector<unsigned char> get_default_alpn()
{
    auto res =
        std::vector<unsigned char>(PZ_HTTP2_H2_ALPN.size() + PZ_HTTP2_H2_16_ALPN.size() + PZ_HTTP2_H2_14_ALPN.size());
    auto p = std::begin(res);

    p = std::copy_n(std::begin(PZ_HTTP2_H2_ALPN), PZ_HTTP2_H2_ALPN.size(), p);
    p = std::copy_n(std::begin(PZ_HTTP2_H2_16_ALPN), PZ_HTTP2_H2_16_ALPN.size(), p);
    p = std::copy_n(std::begin(PZ_HTTP2_H2_14_ALPN), PZ_HTTP2_H2_14_ALPN.size(), p);

    return res;
}

int next_proto_cb(SSL *s, const unsigned char **data, unsigned int *len, void *arg)
{
    auto next_proto = static_cast<std::vector<unsigned char> *>(arg);
    *data           = next_proto->data();
    *len            = next_proto->size();
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
    if (sysconfigpath.isallnothttp2)
    {
        isusehttp2 = (bool)sysconfigpath.domain_http2[(unsigned long long)arg];
    }

    unsigned char *p = (unsigned char *)in;
    while (inlen > 0 && in && (p - in) < inlen)
    {
        int len = p[0];
        p++;

        // fprintf(stderr, "%.*s\n", len, p);

        if (len == 2 && memcmp(p, "h2", 2) == 0)
        {
            unsigned char *tmp_out;
            ret = SSL_TLSEXT_ERR_OK;

            unsigned int protos_len;
            const char *protos;

            protos     = HTTP2_H2_ALPN HTTP1_NPN;
            protos_len = sizeof(HTTP2_H2_ALPN HTTP1_NPN) - 1;

            ret  = SSL_select_next_proto(&tmp_out, outlen, (const unsigned char *)protos, protos_len, in, inlen);
            *out = tmp_out;
            ret  = SSL_TLSEXT_ERR_OK;

            if (isusehttp2)
            {

                break;
            }
            else
            {
                p = p + len;
                continue;
            }
        }
        else if (len == 8 && memcmp(p, "http/1.1", 8) == 0)
        {
            unsigned char *tmp_out;
            ret = SSL_TLSEXT_ERR_OK;

            unsigned int protos_len;
            const char *protos;

            if (isusehttp2)
            {
                protos     = HTTP2_H2_ALPN HTTP1_NPN;
                protos_len = sizeof(HTTP2_H2_ALPN HTTP1_NPN) - 1;
            }
            else
            {
                protos     = HTTP1_NPN;
                protos_len = sizeof(HTTP1_NPN) - 1;
            }

            ret  = SSL_select_next_proto(&tmp_out, outlen, (const unsigned char *)protos, protos_len, in, inlen);
            *out = tmp_out;

            ret = SSL_TLSEXT_ERR_OK;

            if (isusehttp2)
            {

                break;
            }
            else
            {
                p = p + len;
                continue;
            }
        }
        p = p + len;
    }

    return ret;
}

std::string get_password() { return "sslpem123456"; }
int serverNameCallback(SSL *ssl, [[maybe_unused]] int *ad, [[maybe_unused]] void *arg)
{
    // 如果是默认证书，可以直接返回，不用更改上下文
    if (ssl == NULL)
        return SSL_TLSEXT_ERR_NOACK;

    const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
    if (servername == NULL)
    {
        return SSL_TLSEXT_ERR_OK;
    }
    SSL_CTX *ctx                = NULL;
    serverconfig &sysconfigpath = getserversysconfig();

    if (sysconfigpath.checkmaindomain(servername))
    {
        return SSL_TLSEXT_ERR_OK;
    }

    if (servername && strlen(servername) > 0)
    {
        ctx = sysconfigpath.getctx(servername);
    }
    // else
    // {
    //   // ctxmar *ctxptr = ctxmar::instance();
    //   // ctx = ctxptr->getdefaultctx();
    //   ctx =  sysconfigpath.getdefaultctx();
    // }
    if (ctx == NULL)
    {
        return SSL_TLSEXT_ERR_OK;
    }
    SSL_set_SSL_CTX(ssl, ctx);

    SSL_set_verify(ssl,
                   SSL_CTX_get_verify_mode(ctx),

                   SSL_CTX_get_verify_callback(ctx));

    SSL_set_verify_depth(ssl, SSL_CTX_get_verify_depth(ctx));
    SSL_clear_options(ssl, 0xFFFFFFFFL);
    SSL_set_options(ssl, SSL_CTX_get_options(ctx));

    constexpr char DEFAULT_CIPHER_LIST[] = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-"
                                           "AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-"
                                           "POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-"
                                           "AES256-GCM-SHA384";

    if (SSL_CTX_set_cipher_list(ctx, DEFAULT_CIPHER_LIST) == 0)
    {
        std::cerr << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    }

    std::vector<unsigned char> next_proto;
    next_proto = get_default_alpn();

    unsigned int protos_len;
    const char *protos;

    protos     = HTTP2_H2_ALPN HTTP1_NPN;
    protos_len = sizeof(HTTP2_H2_ALPN HTTP1_NPN) - 1;

    if (SSL_CTX_set_alpn_protos(ctx, (const unsigned char *)protos, protos_len) < 0)
    {
    }
    server_loaclvar &localvar      = get_server_global_var();
    unsigned long long temp_domain = 0;
    if (sysconfigpath.isallnothttp2)
    {
        temp_domain = std::hash<const char *>{}(servername);
        if (localvar.http2_enable)
        {
            sysconfigpath.domain_http2[temp_domain] = true;
        }
        else
        {
            sysconfigpath.domain_http2[temp_domain] = false;
            if (sysconfigpath.map_value.find(servername) != sysconfigpath.map_value.end())
            {
                if (sysconfigpath.map_value[servername].find("http2_enable") !=
                    sysconfigpath.map_value[servername].end())
                {
                    if (sysconfigpath.map_value[servername]["http2_enable"][0] == '1')
                    {
                        sysconfigpath.domain_http2[temp_domain] = true;
                    }
                }
            }
        }
    }
    SSL_CTX_set_next_protos_advertised_cb(ctx, next_proto_cb, &next_proto);
    SSL_CTX_set_alpn_select_cb(ctx, alpn_cb, (void *)temp_domain);
    return SSL_TLSEXT_ERR_OK;
}

}// namespace http