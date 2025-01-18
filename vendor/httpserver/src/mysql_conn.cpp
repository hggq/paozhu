/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <iostream>
#include <memory>
#include <string>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#if defined(_WIN32) && !defined(_OPENSSL_Applink) && \
    defined(HAVE_OPENSSL_APPLINK_C)
#include <openssl/applink.c>
#endif
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "mysql_conn.h"
#include "clientdatacache.h"

namespace orm
{
mysql_conn_base::mysql_conn_base(asio::io_context &ioc) : io_ctx(&ioc)
{
    error_code  = 0;
    auto &cc    = http::get_client_data_cache();
    _cache_data = cc.get_data_ptr();
}
mysql_conn_base::~mysql_conn_base()
{
    if (_cache_data != nullptr)
    {
        auto &cc = http::get_client_data_cache();
        cc.back_data_ptr(_cache_data);
    }
}

void mysql_conn_base::mysqlnd_xor_string(char *dst, const size_t dst_len, const char *xor_str, const size_t xor_str_len)
{
    unsigned int i;
    for (i = 0; i <= dst_len; ++i)
    {
        dst[i] ^= xor_str[i % xor_str_len];
    }
}
unsigned int mysql_conn_base::read_pack(unsigned char *data, unsigned int offset)
{
    unsigned int pack_length  = 0;
    unsigned int begin_length = offset;
    pack_length               = (data[offset + 2] & 0xFF);
    pack_length               = pack_length << 8 | (data[offset + 1] & 0xFF);

    pack_length = pack_length << 8 | (data[offset] & 0xFF);

    offset               = offset + 3;
    unsigned char seq_id = data[offset];
    offset++;
    offset = offset + pack_length;
    // dump_data((const unsigned char *)&data[begin_length], pack_length + 4);
    return offset;
}

void mysql_conn_base::read_field_pack(unsigned char *data, unsigned int total_num, unsigned int &offset, pack_info_t &pack_info)
{

    unsigned int begin_length = offset;
    unsigned int pack_length  = 0;
    if (pack_info.length > pack_info.current_length)
    {
        pack_length = pack_info.length - pack_info.current_length;
        if ((pack_length + offset) > total_num)
        {
            pack_length = total_num - offset;
        }
        pack_info.current_length = pack_info.current_length + pack_length;
        pack_info.data.append((char *)&data[begin_length], pack_length);
        offset = offset + pack_length;
    }
    else
    {
        pack_info.length         = 0;
        pack_info.current_length = 0;
        pack_length              = (data[offset + 2] & 0xFF);
        pack_length              = pack_length << 8 | (data[offset + 1] & 0xFF);
        pack_length              = pack_length << 8 | (data[offset] & 0xFF);
        offset                   = offset + 3;
        unsigned char seq_id     = data[offset];

        if (seq_id != pack_info.seq_id)
        {
            pack_info.error = 1;
            pack_info.data  = "mysql read package seq_id error !";
            return;
        }
        pack_info.seq_id = (pack_info.seq_id + 1) % 256;
        offset++;
        offset = offset + pack_length;

        pack_info.length         = pack_length;
        pack_info.current_length = pack_length;
        if (offset > total_num)
        {

            pack_info.current_length = total_num - begin_length - 4;
            pack_length              = total_num - begin_length - 4;
        }

        pack_info.data.clear();
        pack_info.data.append((char *)&data[begin_length + 4], pack_length);
    }
}
void mysql_conn_base::read_server_hello(unsigned int offset, unsigned int length)
{
    // unsigned int payload;
    // payload = _cache_data[2];
    // payload = payload << 4 | _cache_data[1];
    // payload = payload << 4 | _cache_data[0];

    seq_next_id                   = _cache_data[3];
    server_hello.protocol_version = _cache_data[4];
    offset                        = 5;
    for (; offset < length; offset++)
    {
        if (_cache_data[offset] == 0x00)
        {
            offset++;
            break;
        }
        server_hello.server_version.push_back(_cache_data[offset]);
    }
    unsigned int server_thread_num;
    server_thread_num = _cache_data[offset + 3];
    server_thread_num = server_thread_num << 4 | _cache_data[offset + 2];
    server_thread_num = server_thread_num << 4 | _cache_data[offset + 1];
    server_thread_num = server_thread_num << 4 | _cache_data[offset];
    offset += 4;
    server_hello.connection_id = server_thread_num;

    for (unsigned int i = 0; i < 8; i++)
    {
        server_hello.auth_plugin_salt_data.push_back(_cache_data[offset]);
        offset++;
    }
    offset++;

    server_hello.capability_flags_low = _cache_data[offset + 1];
    server_hello.capability_flags_low = server_hello.capability_flags_low << 8 | (_cache_data[offset] & 0xFF);
    offset += 2;

    server_hello.character_set = _cache_data[offset];
    offset++;
    server_hello.status_flags = _cache_data[offset + 1];
    server_hello.status_flags = server_hello.status_flags << 8 | _cache_data[offset];
    offset += 2;

    server_hello.capability_flags_high = _cache_data[offset + 1];
    server_hello.capability_flags_high = server_hello.capability_flags_high << 8 | (_cache_data[offset] & 0xFF);
    offset += 2;

    server_hello.auth_plugin_data_len = _cache_data[offset];
    offset++;

    offset += 10;
    for (; offset < length; offset++)
    {
        if (_cache_data[offset] == 0x00)
        {
            offset++;
            break;
        }
        server_hello.auth_plugin_salt_data.push_back(_cache_data[offset]);
    }

    for (unsigned int i = 0; i < server_hello.auth_plugin_data_len; i++)
    {
        if (_cache_data[offset] == 0x00)
        {
            offset++;
            break;
        }
        server_hello.auth_plugin_name.push_back(_cache_data[offset]);
        offset++;
        if (offset >= length)
        {
            break;
        }
    }
}
bool mysql_conn_base::connect(const std::string &host, const std::string &port, const std::string &user, const std::string &password, const std::string &dbname, bool ssl_flag)
{
    socket = std::make_unique<asio::ip::tcp::socket>(*io_ctx);
    asio::ip::tcp::resolver resolver(*io_ctx);

    asio::ip::tcp::resolver::query checkquery(host, port);
    asio::ip::tcp::resolver::iterator iter = resolver.resolve(checkquery);
    asio::ip::tcp::resolver::iterator end;
    asio::ip::tcp::endpoint endpoint;

    while (iter != end)
    {
        endpoint = *iter++;
        socket->connect(endpoint, ec);
        if (ec)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    // asio::error_code ec;
    if (ec)
    {
        error_msg.append("host connect error! ");
        error_msg.append(ec.message());
        error_code = 1;
        return false;
    }
    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    std::size_t n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
    if (n < 78)
    {
        error_msg.append("mysql server back data error! ");
        error_code = 255;
        return false;
    }
    read_server_hello(0, n);

    constexpr std::size_t challenge_length = 20;
    constexpr std::size_t response_length  = 32;

    using sha_buffer = std::uint8_t[response_length];
    sha_buffer password_sha;

    SHA256(reinterpret_cast<const unsigned char *>(password.data()), password.size(), password_sha);

    // std::uint8_t buffer[response_length + challenge_length];
    std::memset(_cache_data, 0x00, (response_length + challenge_length));
    SHA256(password_sha, response_length, _cache_data);
    std::memcpy(_cache_data + response_length, server_hello.auth_plugin_salt_data.data(), challenge_length);

    sha_buffer salted_password;
    SHA256(_cache_data, (response_length + challenge_length), salted_password);

    // salted_password XOR password_sha
    //std::uint8_t output[response_length] = {0};
    std::memset(_cache_data, 0x00, (response_length + challenge_length));

    for (unsigned i = 0; i < response_length; ++i)
    {
        //static_cast<std::uint8_t *>(output)[i] = salted_password[i] ^ password_sha[i];
        _cache_data[i] = salted_password[i] ^ password_sha[i];
    }

    client_flags = CLIENT_PZORM_FLAGS;

    seq_next_id++;
    send_data.clear();

    send_data.push_back(0x00);
    send_data.push_back(0x00);
    send_data.push_back(0x00);
    send_data.push_back(seq_next_id);
    send_data.push_back((client_flags & 0xFF));
    send_data.push_back((client_flags >> 8 & 0xFF));
    send_data.push_back((client_flags >> 16 & 0xFF));
    send_data.push_back((client_flags >> 24 & 0xFF));

    send_data.push_back(0xFF);// max pack
    send_data.push_back(0xFF);
    send_data.push_back(0xFF);

    send_data.push_back(0x00);
    send_data.push_back(0x2D);// charset utf8mb4_general_ci

    for (size_t i = 0; i < 23; i++)
    {
        send_data.push_back(0x00);
    }
    send_data.append(user);
    send_data.push_back(0x00);
    send_data.push_back(0x20);
    for (unsigned i = 0; i < response_length; ++i)
    {
        send_data.push_back(_cache_data[i]);
    }
    send_data.append(dbname);
    send_data.push_back(0x00);
    send_data.append("caching_sha2_password");
    send_data.push_back(0x00);

    send_data[0] = send_data.size() - 4;

    n = asio::write(*socket, asio::buffer(send_data));

    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);

    bool is_fast_auth = false;

    if (_cache_data[0] == 0x02 && _cache_data[4] == 0x01 && _cache_data[5] == 0x04)
    {
        seq_next_id += 2;
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        _cache_data[0] = 0x01;
        _cache_data[3] = seq_next_id;
        _cache_data[4] = 0x02;
        n              = asio::write(*socket, asio::buffer(_cache_data, 5));
        if (n == 0)
        {
            error_msg.append(" request server_public_key error ! ");
            error_code = 3;
            return false;
        }

        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
        if (n < 256 || n > 1024)
        {
            error_msg.append(" get server_public_key return size: ");
            error_msg.append(std::to_string(n));
            error_code = 2;
            return false;
        }

        seq_next_id = (_cache_data[3] & 0xFF) + 1;
        bool isok   = server_public_key_encrypt(password, &_cache_data[5], n - 5);
        if (isok == false)
        {
            return false;
        }

        try
        {
            n = asio::write(*socket, asio::buffer(send_data));
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
            error_code = 4;
            return false;
        }

        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);

        try
        {
            n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
            error_code = 4;
            return false;
        }

        if ((unsigned char)_cache_data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(_cache_data[i]);
            }
            error_code = 5;
            return false;
        }
        return true;
    }
    else if ((unsigned char)_cache_data[4] == 0xFE)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(_cache_data[i]);
        }
        error_code = 2;
        return false;
    }
    else if ((unsigned char)_cache_data[4] == 0xFF)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(_cache_data[i]);
        }
        error_code = 6;
        return false;
    }
    else
    {
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
        if ((unsigned char)_cache_data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(_cache_data[i]);
            }
            error_code = 7;
            return false;
        }
        else if ((unsigned char)_cache_data[4] == 0x00)
        {
            return true;
        }
    }

    return true;
}

bool mysql_conn_base::server_public_key_encrypt(const std::string &password, unsigned char *data, unsigned int length)
{
    send_data.clear();
    send_data = password;
    send_data.push_back(0x00);

    mysqlnd_xor_string(&send_data[0], send_data.size(), server_hello.auth_plugin_salt_data.data(), 20);

    BIO *bio             = BIO_new_mem_buf(data, length);
    EVP_PKEY *public_key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);

    std::size_t server_public_key_len = 0;// EVP_PKEY_get_size(public_key);
    //std::string public_key_str(256, 0x00);
    std::memset(_cache_data, 0x00, 256);
    // see sql-common/client_authenthication.cc line 144 or 968 php-src ext/mysqlnd/mysqlnd_auth.c
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(public_key, NULL);
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
        EVP_PKEY_encrypt(ctx, _cache_data, &server_public_key_len, (unsigned char *)&send_data[0], send_data.size()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        error_msg.append(" server_public_key encrypt error ! ");
        error_code = 4;
        return false;
    }
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(public_key);

    send_data.clear();
    unsigned char a_length = 0;
    a_length               = server_public_key_len & 0xFF;
    send_data.push_back(a_length);
    a_length = server_public_key_len >> 8 & 0xFF;
    send_data.push_back(a_length);
    a_length = server_public_key_len >> 16 & 0xFF;
    send_data.push_back(a_length);
    send_data.push_back(seq_next_id);

    for (size_t i = 0; i < server_public_key_len; i++)
    {
        send_data.push_back(_cache_data[i]);
    }
    return true;
}
asio::awaitable<bool> mysql_conn_base::async_connect(const std::string &host, const std::string &port, const std::string &user, const std::string &password, const std::string &dbname, bool ssl_flag)
{
    socket = std::make_unique<asio::ip::tcp::socket>(*io_ctx);
    asio::ip::tcp::resolver resolver(*io_ctx);

    asio::ip::tcp::resolver::iterator iter = co_await resolver.async_resolve(host, port, asio::use_awaitable);
    asio::ip::tcp::resolver::iterator end;
    asio::ip::tcp::endpoint endpoint;
    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    while (iter != end)
    {
        endpoint     = *iter++;
        std::tie(ec) = co_await socket->async_connect(endpoint, tuple_awaitable);
        if (ec)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    // asio::error_code ec;
    if (ec)
    {
        error_msg.append("host connect error! ");
        error_msg.append(ec.message());
        error_code = 1;
        co_return false;
    }
    std::size_t n = 0;
    try
    {
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        n = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        co_return false;
    }

    if (n < 78)
    {
        error_msg.append("mysql server back data error! ");
        error_code = 255;
        co_return false;
    }
    read_server_hello(0, n);

    constexpr std::size_t challenge_length = 20;
    constexpr std::size_t response_length  = 32;

    using sha_buffer = std::uint8_t[response_length];
    sha_buffer password_sha;

    SHA256(reinterpret_cast<const unsigned char *>(password.data()), password.size(), password_sha);

    // std::uint8_t buffer[response_length + challenge_length];
    std::memset(_cache_data, 0x00, (response_length + challenge_length));
    SHA256(password_sha, response_length, _cache_data);
    std::memcpy(_cache_data + response_length, server_hello.auth_plugin_salt_data.data(), challenge_length);

    sha_buffer salted_password;
    SHA256(_cache_data, (response_length + challenge_length), salted_password);

    // salted_password XOR password_sha
    //std::uint8_t output[response_length] = {0};
    std::memset(_cache_data, 0x00, (response_length + challenge_length));

    for (unsigned i = 0; i < response_length; ++i)
    {
        //static_cast<std::uint8_t *>(output)[i] = salted_password[i] ^ password_sha[i];
        _cache_data[i] = salted_password[i] ^ password_sha[i];
    }

    client_flags = CLIENT_PZORM_FLAGS;

    seq_next_id++;
    send_data.clear();

    send_data.push_back(0x00);
    send_data.push_back(0x00);
    send_data.push_back(0x00);
    send_data.push_back(seq_next_id);
    send_data.push_back((client_flags & 0xFF));
    send_data.push_back((client_flags >> 8 & 0xFF));
    send_data.push_back((client_flags >> 16 & 0xFF));
    send_data.push_back((client_flags >> 24 & 0xFF));

    send_data.push_back(0xFF);// max pack
    send_data.push_back(0xFF);
    send_data.push_back(0xFF);

    send_data.push_back(0x00);
    send_data.push_back(0x2D);// charset utf8mb4_general_ci

    for (size_t i = 0; i < 23; i++)
    {
        send_data.push_back(0x00);
    }
    send_data.append(user);
    send_data.push_back(0x00);
    send_data.push_back(0x20);
    for (unsigned i = 0; i < response_length; ++i)
    {
        send_data.push_back(_cache_data[i]);
    }
    send_data.append(dbname);
    send_data.push_back(0x00);
    send_data.append("caching_sha2_password");
    send_data.push_back(0x00);

    send_data[0] = send_data.size() - 4;
    try
    {
        n = co_await asio::async_write(*socket, asio::buffer(send_data), asio::use_awaitable);
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        co_return false;
    }
    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    try
    {
        n = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        co_return false;
    }

    bool is_fast_auth = false;

    if (_cache_data[0] == 0x02 && _cache_data[4] == 0x01 && _cache_data[5] == 0x04)
    {
        seq_next_id += 2;
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        _cache_data[0] = 0x01;
        _cache_data[3] = seq_next_id;
        _cache_data[4] = 0x02;
        try
        {
            n = co_await asio::async_write(*socket, asio::buffer(_cache_data, 5), asio::use_awaitable);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            co_return false;
        }
        if (n == 0)
        {
            error_msg.append(" request server_public_key error ! ");
            error_code = 3;
            co_return false;
        }

        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        try
        {
            n = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            co_return false;
        }
        if (n < 256 || n > 1024)
        {
            error_msg.append(" get server_public_key return size: ");
            error_msg.append(std::to_string(n));
            error_code = 2;
            co_return false;
        }

        seq_next_id = (_cache_data[3] & 0xFF) + 1;
        bool isok   = server_public_key_encrypt(password, &_cache_data[5], n - 5);
        if (isok == false)
        {

            co_return false;
        }

        try
        {
            n = co_await asio::async_write(*socket, asio::buffer(send_data), asio::use_awaitable);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            co_return false;
        }
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);

        try
        {
            n = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            co_return false;
        }

        if ((unsigned char)_cache_data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(_cache_data[i]);
            }
            error_code = 5;
            co_return false;
        }
    }
    else if ((unsigned char)_cache_data[4] == 0xFE)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(_cache_data[i]);
        }
        error_code = 2;
        co_return false;
    }
    else if ((unsigned char)_cache_data[4] == 0xFF)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(_cache_data[i]);
        }
        error_code = 6;
        co_return false;
    }
    else
    {
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        try
        {
            n = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            co_return false;
        }
        if ((unsigned char)_cache_data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(_cache_data[i]);
            }
            error_code = 7;
            co_return false;
        }
        else if ((unsigned char)_cache_data[4] == 0x00)
        {
            co_return true;
        }
    }

    co_return true;
}
asio::awaitable<unsigned int> mysql_conn_base::async_read_loop()
{
    try
    {
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        std::size_t n = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        if (n == 0)
        {
            error_code = 1;
            co_return 0;
        }
        co_return n;
    }
    catch (const std::exception &e)
    {
        error_code = 1;
        error_msg.append(e.what());
    }
    co_return 0;
}
unsigned int mysql_conn_base::read_loop()
{
    try
    {
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        std::size_t n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
        if (ec)
        {
            error_code = 1;
            return 0;
        }
        return n;
    }
    catch (const std::exception &e)
    {
        error_code = 1;
        error_msg.append(e.what());
    }
    return 0;
}
bool mysql_conn_base::ping()
{
    char data_send[16] = {0x01, 0x00, 0x00, 0x00, 0x0E, 0x00};
    error_code         = 0;
    error_msg.clear();
    try
    {
        asio::write(*socket, asio::buffer(data_send, 5), ec);
        socket->read_some(asio::buffer(data_send, 16), ec);

        return true;
    }
    catch (std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return false;
    }
}
bool mysql_conn_base::close()
{
    char data_send[6] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
    error_code        = 0;
    error_msg.clear();
    try
    {
        isclose = false;
        asio::write(*socket, asio::buffer(data_send, 5), ec);
        socket->close();
        return true;
    }
    catch (std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return false;
    }
}
bool mysql_conn_base::is_closed()
{
    try
    {
        if (socket->is_open())
        {
            return true;
        }
        return false;
    }
    catch (std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return false;
    }
}
}// namespace orm