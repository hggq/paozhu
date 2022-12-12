#pragma once
#ifndef HTTP_DOMAIN_H
#define HTTP_DOMAIN_H

#include <asio.hpp>
// #include <asio/ssl.hpp>
// #include <asio/co_spawn.hpp>
// #include <asio/detached.hpp>
// #include <asio/io_context.hpp>
// #include <asio/ip/tcp.hpp>
// #include <asio/signal_set.hpp>
// #include <asio/write.hpp>

namespace http
{
    std::vector<unsigned char> get_default_alpn();

    int next_proto_cb(SSL *s, const unsigned char **data, unsigned int *len,
                      void *arg);

    int alpn_cb(SSL *ssl,
                const unsigned char **out,
                unsigned char *outlen,
                const unsigned char *in,
                unsigned int inlen,
                void *arg);

    std::string get_password();
    long serverNameCallback(SSL *ssl, int *ad, void *arg);
}
#endif