#pragma once
#ifndef HTTP_CRYPTO_H
#define HTTP_CRYPTO_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ocsp.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <memory>
#include <stdexcept>

namespace http
{
// ============================================================
//  RAII 智能指针 (OpenSSL 3.0 对象自动释放)
// ============================================================
struct EVP_PKEY_deleter
{
    void operator()(EVP_PKEY *p) { EVP_PKEY_free(p); }
};
struct BIO_deleter
{
    void operator()(BIO *p) { BIO_free(p); }
};
struct X509_REQ_deleter
{
    void operator()(X509_REQ *p) { X509_REQ_free(p); }
};
struct X509_deleter
{
    void operator()(X509 *p) { X509_free(p); }
};
struct EVP_MD_CTX_del
{
    void operator()(EVP_MD_CTX *p) { EVP_MD_CTX_free(p); }
};
struct EVP_MAC_CTX_del
{
    void operator()(EVP_MAC_CTX *p) { EVP_MAC_CTX_free(p); }
};
struct EVP_MAC_del
{
    void operator()(EVP_MAC *p) { EVP_MAC_free(p); }
};
struct ECDSA_SIG_del
{
    void operator()(ECDSA_SIG *p) { ECDSA_SIG_free(p); }
};
struct X509_STORE_deleter
{
    void operator()(X509_STORE *p) { X509_STORE_free(p); }
};
struct X509_STACK_deleter
{
    void operator()(STACK_OF(X509) * p) { sk_X509_free(p); }
};
struct OCSP_REQUEST_deleter
{
    void operator()(OCSP_REQUEST *p) { OCSP_REQUEST_free(p); }
};
struct OCSP_RESPONSE_deleter
{
    void operator()(OCSP_RESPONSE *p) { OCSP_RESPONSE_free(p); }
};
struct OCSP_BASICRESP_deleter
{
    void operator()(OCSP_BASICRESP *p) { OCSP_BASICRESP_free(p); }
};
struct OCSP_CERTID_deleter
{
    void operator()(OCSP_CERTID *p) { OCSP_CERTID_free(p); }
};

using EVP_PKEY_ptr       = std::unique_ptr<EVP_PKEY, EVP_PKEY_deleter>;
using BIO_ptr            = std::unique_ptr<BIO, BIO_deleter>;
using X509_REQ_ptr       = std::unique_ptr<X509_REQ, X509_REQ_deleter>;
using X509_ptr           = std::unique_ptr<X509, X509_deleter>;
using EVP_MD_CTX_ptr     = std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_del>;
using EVP_MAC_CTX_ptr    = std::unique_ptr<EVP_MAC_CTX, EVP_MAC_CTX_del>;
using EVP_MAC_ptr        = std::unique_ptr<EVP_MAC, EVP_MAC_del>;
using ECDSA_SIG_ptr      = std::unique_ptr<ECDSA_SIG, ECDSA_SIG_del>;
using X509_STORE_ptr     = std::unique_ptr<X509_STORE, X509_STORE_deleter>;
using X509_STACK_ptr     = std::unique_ptr<STACK_OF(X509), X509_STACK_deleter>;
using OCSP_REQUEST_ptr   = std::unique_ptr<OCSP_REQUEST, OCSP_REQUEST_deleter>;
using OCSP_RESPONSE_ptr  = std::unique_ptr<OCSP_RESPONSE, OCSP_RESPONSE_deleter>;
using OCSP_BASICRESP_ptr = std::unique_ptr<OCSP_BASICRESP, OCSP_BASICRESP_deleter>;
using OCSP_CERTID_ptr    = std::unique_ptr<OCSP_CERTID, OCSP_CERTID_deleter>;

}// namespace http
#endif