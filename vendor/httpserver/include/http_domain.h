#pragma once
#ifndef HTTP_DOMAIN_H
#define HTTP_DOMAIN_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <memory>
#include <stdexcept>

namespace http
{
std::vector<unsigned char> get_default_alpn();

int next_proto_cb(SSL *s, const unsigned char **data, unsigned int *len, void *arg);

int alpn_cb(SSL *ssl,
            const unsigned char **out,
            unsigned char *outlen,
            const unsigned char *in,
            unsigned int inlen,
            void *arg);

std::string get_password();
int serverNameCallback(SSL *ssl, int *ad, void *arg);
static const unsigned char HTTP2_H2H1_STR[13] = "\x02h2\x08http/1.1";
static const char *DEFAULT_CIPHER_LIST        = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-"
                                                "AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-"
                                                "POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-"
                                                "AES256-GCM-SHA384";

struct cert_validity_t
{
    time_t not_before;          // 生效时间 (Unix timestamp, UTC)
    time_t not_after;           // 过期时间 (Unix timestamp, UTC)
    unsigned int days_remaining;// 距过期的剩余天数
    bool is_expired;            // 是否已过期
};
struct eab_credentials_t
{
    std::string eab_kid;
    std::string eab_hmac_key;
};
cert_validity_t get_cert_validity(const std::string &cert_path);
eab_credentials_t fetch_eab_by_email(std::string_view url, std::string_view email);
}// namespace http
#endif