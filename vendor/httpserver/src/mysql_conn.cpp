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

namespace orm
{
mysql_conn_base::mysql_conn_base(asio::io_context &ioc) : io_ctx(&ioc)
{
    error_code = 0;
}
mysql_conn_base::~mysql_conn_base()
{
}

void mysql_conn_base::print_hex(const std::string &data)
{
    std::string hexstr = "0123456789ABCDEF";
    for (unsigned int i = 0; i < data.size(); i++)
    {
        unsigned char a, b;
        a = data[i] & 0xF0;
        a = a >> 4;
        b = data[i] & 0x0F;

        std::cout << hexstr[a] << hexstr[b] << " ";
    }
}
void mysql_conn_base::print_hex(unsigned int data)
{
    std::string hexstr = "0123456789ABCDEF";
    std::cout << std::to_string(data) << " ";
    unsigned int a, b;
    a = data & 0xF0000000;
    a = a >> 28;
    b = data & 0x0F000000;
    b = b >> 24;
    std::cout << hexstr[a] << hexstr[b] << " ";
    a = data & 0xF00000;
    a = a >> 20;
    b = data & 0x0F0000;
    b = b >> 16;
    std::cout << hexstr[a] << hexstr[b] << " ";

    a = data & 0xF000;
    a = a >> 12;
    b = data & 0x0F00;
    b = b >> 8;
    std::cout << hexstr[a] << hexstr[b] << " ";
    a = data & 0xF0;
    a = a >> 4;
    b = data & 0x0F;

    std::cout << hexstr[a] << hexstr[b] << " ";
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

    std::size_t n = socket->read_some(asio::buffer(data), ec);

    unsigned int payload;
    payload = data[2];
    payload = payload << 4 | data[1];
    payload = payload << 4 | data[0];

    if (n < (payload + 4) && n < 78)
    {
        error_msg.append("mysql server back data error! ");
        error_code = 255;
        return false;
    }

    seq_next_id                   = data[3];
    server_hello.protocol_version = data[4];
    unsigned int offset           = 5;
    for (unsigned int i = 5; i < n; i++)
    {
        if (data[i] == 0x00)
        {
            i++;
            offset = i;
            break;
        }
        server_hello.server_version.push_back(data[i]);
    }
    unsigned int server_thread_num;
    server_thread_num = data[offset + 3];
    server_thread_num = server_thread_num << 4 | data[offset + 2];
    server_thread_num = server_thread_num << 4 | data[offset + 1];
    server_thread_num = server_thread_num << 4 | data[offset];
    offset += 4;
    server_hello.connection_id = server_thread_num;

    for (unsigned int i = 0; i < 8; i++)
    {
        server_hello.auth_plugin_salt_data.push_back(data[offset]);
        offset++;
    }
    offset++;

    server_hello.capability_flags_low = data[offset + 1];
    server_hello.capability_flags_low = server_hello.capability_flags_low << 8 | (data[offset] & 0xFF);
    offset += 2;

    server_hello.character_set = data[offset];
    offset++;
    server_hello.status_flags = data[offset + 1];
    server_hello.status_flags = server_hello.status_flags << 8 | data[offset];
    offset += 2;

    server_hello.capability_flags_high = data[offset + 1];
    server_hello.capability_flags_high = server_hello.capability_flags_high << 8 | (data[offset] & 0xFF);
    offset += 2;

    server_hello.auth_plugin_data_len = data[offset];
    offset++;

    offset += 10;
    for (; offset < n; offset++)
    {
        if (data[offset] == 0x00)
        {
            offset++;
            break;
        }
        server_hello.auth_plugin_salt_data.push_back(data[offset]);
    }

    for (unsigned int i = 0; i < server_hello.auth_plugin_data_len; i++)
    {
        if (data[offset] == 0x00)
        {
            offset++;
            break;
        }
        server_hello.auth_plugin_name.push_back(data[offset]);
        offset++;
    }

    constexpr std::size_t challenge_length = 20;
    constexpr std::size_t response_length  = 32;

    using sha_buffer = std::uint8_t[response_length];
    sha_buffer password_sha;

    SHA256(reinterpret_cast<const unsigned char *>(password.data()), password.size(), password_sha);

    std::uint8_t buffer[response_length + challenge_length];
    SHA256(password_sha, response_length, buffer);
    std::memcpy(buffer + response_length, server_hello.auth_plugin_salt_data.data(), challenge_length);

    sha_buffer salted_password;
    SHA256(buffer, sizeof(buffer), salted_password);

    // salted_password XOR password_sha
    std::uint8_t output[response_length] = {0};

    for (unsigned i = 0; i < response_length; ++i)
    {
        static_cast<std::uint8_t *>(output)[i] = salted_password[i] ^ password_sha[i];
    }

    client_flags = CLIENT_PZORM_FLAGS;

    seq_next_id++;
    std::string response_client;

    response_client.push_back(0x00);
    response_client.push_back(0x00);
    response_client.push_back(0x00);
    response_client.push_back(seq_next_id);
    response_client.push_back((client_flags & 0xFF));
    response_client.push_back((client_flags >> 8 & 0xFF));
    response_client.push_back((client_flags >> 16 & 0xFF));
    response_client.push_back((client_flags >> 24 & 0xFF));

    response_client.push_back(0xFF);// max pack
    response_client.push_back(0xFF);
    response_client.push_back(0xFF);

    response_client.push_back(0x00);
    response_client.push_back(0x2D);// charset utf8mb4_general_ci

    for (size_t i = 0; i < 23; i++)
    {
        response_client.push_back(0x00);
    }
    response_client.append(user);
    response_client.push_back(0x00);
    response_client.push_back(0x20);
    for (unsigned i = 0; i < response_length; ++i)
    {
        response_client.push_back(output[i]);
    }
    response_client.append(dbname);
    response_client.push_back(0x00);
    response_client.append("caching_sha2_password");
    response_client.push_back(0x00);

    response_client[0] = response_client.size() - 4;
    n                  = asio::write(*socket, asio::buffer(response_client));

    std::memset(data, 0x00, 1024);
    n = socket->read_some(asio::buffer(data), ec);

    bool is_fast_auth = false;

    if (data[0] == 0x02 && data[4] == 0x01 && data[5] == 0x04)
    {
        seq_next_id += 2;
        std::memset(data, 0x00, 1024);
        data[0] = 0x01;
        data[3] = seq_next_id;
        data[4] = 0x02;
        n       = asio::write(*socket, asio::buffer(data, 5));
        if (n == 0)
        {
            error_msg.append(" request server_public_key error ! ");
            error_code = 3;
            return false;
        }

        std::memset(data, 0x00, 1024);
        n = socket->read_some(asio::buffer(data), ec);
        if (n < 256 || n > 1024)
        {
            error_msg.append(" get server_public_key return size: ");
            error_msg.append(std::to_string(n));
            error_code = 2;
            return false;
        }

        seq_next_id = (data[3] & 0xFF) + 1;
        server_public_key.clear();

        for (size_t i = 5; i < n; i++)
        {
            server_public_key.push_back(data[i]);
        }

        std::string request_auth;
        request_auth = password;
        request_auth.push_back(0x00);

        mysqlnd_xor_string(&request_auth[0], request_auth.size(), server_hello.auth_plugin_salt_data.data(), challenge_length);

        // see sql-common/client_authenthication.cc line 706
        BIO *bio             = BIO_new_mem_buf(&server_public_key[0], server_public_key.size());
        EVP_PKEY *public_key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
        BIO_free(bio);

        std::size_t server_public_key_len = EVP_PKEY_get_size(public_key);
        std::string public_key_str(256, 0x00);
        // see sql-common/client_authenthication.cc line 144 or 968 php-src ext/mysqlnd/mysqlnd_auth.c
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(public_key, NULL);
        if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0 ||
            EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
            EVP_PKEY_encrypt(ctx, (unsigned char *)public_key_str.data(), &server_public_key_len, (unsigned char *)&request_auth[0], request_auth.size()) <= 0)
        {
            EVP_PKEY_CTX_free(ctx);
            error_msg.append(" server_public_key encrypt error ! ");
            error_code = 4;
            return false;
        }
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(public_key);

        request_auth.clear();

        unsigned char a_length = 0;
        a_length               = server_public_key_len & 0xFF;
        request_auth.push_back(a_length);
        server_public_key_len = server_public_key_len >> 8;
        a_length              = server_public_key_len & 0xFF;
        request_auth.push_back(a_length);
        server_public_key_len = server_public_key_len >> 8;
        a_length              = server_public_key_len & 0xFF;
        request_auth.push_back(a_length);
        request_auth.push_back(seq_next_id);

        for (size_t i = 0; i < public_key_str.size(); i++)
        {
            request_auth.push_back(public_key_str[i]);
        }
        n = asio::write(*socket, asio::buffer(request_auth));

        std::memset(data, 0x00, 1024);
        n = socket->read_some(asio::buffer(data), ec);
        if ((unsigned char)data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(data[i]);
            }
            error_code = 5;
            return false;
        }
    }
    else if ((unsigned char)data[4] == 0xFE)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(data[i]);
        }
        error_code = 2;
        return false;
    }
    else if ((unsigned char)data[4] == 0xFF)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(data[i]);
        }
        error_code = 6;
        return false;
    }
    else
    {
        std::memset(data, 0x00, 1024);
        n = socket->read_some(asio::buffer(data), ec);
        if ((unsigned char)data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(data[i]);
            }
            error_code = 7;
            return false;
        }
        else if ((unsigned char)data[4] == 0x00)
        {
            return true;
        }
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
        n = co_await socket->async_read_some(asio::buffer(data), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        co_return false;
    }

    unsigned int payload;
    payload = data[2];
    payload = payload << 4 | data[1];
    payload = payload << 4 | data[0];

    if (n < (payload + 4) && n < 78)
    {
        error_msg.append("mysql server back data error! ");
        error_code = 255;
        co_return false;
    }

    seq_next_id                   = data[3];
    server_hello.protocol_version = data[4];
    unsigned int offset           = 5;
    for (unsigned int i = 5; i < n; i++)
    {
        if (data[i] == 0x00)
        {
            i++;
            offset = i;
            break;
        }
        server_hello.server_version.push_back(data[i]);
    }
    unsigned int server_thread_num;
    server_thread_num = data[offset + 3];
    server_thread_num = server_thread_num << 4 | data[offset + 2];
    server_thread_num = server_thread_num << 4 | data[offset + 1];
    server_thread_num = server_thread_num << 4 | data[offset];
    offset += 4;
    server_hello.connection_id = server_thread_num;

    for (unsigned int i = 0; i < 8; i++)
    {
        server_hello.auth_plugin_salt_data.push_back(data[offset]);
        offset++;
    }
    offset++;

    server_hello.capability_flags_low = data[offset + 1];
    server_hello.capability_flags_low = server_hello.capability_flags_low << 8 | (data[offset] & 0xFF);
    offset += 2;

    server_hello.character_set = data[offset];
    offset++;
    server_hello.status_flags = data[offset + 1];
    server_hello.status_flags = server_hello.status_flags << 8 | data[offset];
    offset += 2;

    server_hello.capability_flags_high = data[offset + 1];
    server_hello.capability_flags_high = server_hello.capability_flags_high << 8 | (data[offset] & 0xFF);
    offset += 2;

    server_hello.auth_plugin_data_len = data[offset];
    offset++;

    offset += 10;
    for (; offset < n; offset++)
    {
        if (data[offset] == 0x00)
        {
            offset++;
            break;
        }
        server_hello.auth_plugin_salt_data.push_back(data[offset]);
    }

    for (unsigned int i = 0; i < server_hello.auth_plugin_data_len; i++)
    {
        if (data[offset] == 0x00)
        {
            offset++;
            break;
        }
        server_hello.auth_plugin_name.push_back(data[offset]);
        offset++;
    }

    constexpr std::size_t challenge_length = 20;
    constexpr std::size_t response_length  = 32;

    using sha_buffer = std::uint8_t[response_length];
    sha_buffer password_sha;

    SHA256(reinterpret_cast<const unsigned char *>(password.data()), password.size(), password_sha);

    std::uint8_t buffer[response_length + challenge_length];
    SHA256(password_sha, response_length, buffer);
    std::memcpy(buffer + response_length, server_hello.auth_plugin_salt_data.data(), challenge_length);

    sha_buffer salted_password;
    SHA256(buffer, sizeof(buffer), salted_password);

    // salted_password XOR password_sha
    std::uint8_t output[response_length] = {0};

    for (unsigned i = 0; i < response_length; ++i)
    {
        static_cast<std::uint8_t *>(output)[i] = salted_password[i] ^ password_sha[i];
    }

    client_flags = CLIENT_PZORM_FLAGS;

    seq_next_id++;
    std::string response_client;

    response_client.push_back(0x00);
    response_client.push_back(0x00);
    response_client.push_back(0x00);
    response_client.push_back(seq_next_id);
    response_client.push_back((client_flags & 0xFF));
    response_client.push_back((client_flags >> 8 & 0xFF));
    response_client.push_back((client_flags >> 16 & 0xFF));
    response_client.push_back((client_flags >> 24 & 0xFF));

    response_client.push_back(0xFF);// max pack
    response_client.push_back(0xFF);
    response_client.push_back(0xFF);

    response_client.push_back(0x00);
    response_client.push_back(0x2D);// charset utf8mb4_general_ci

    for (size_t i = 0; i < 23; i++)
    {
        response_client.push_back(0x00);
    }
    response_client.append(user);
    response_client.push_back(0x00);
    response_client.push_back(0x20);
    for (unsigned i = 0; i < response_length; ++i)
    {
        response_client.push_back(output[i]);
    }
    response_client.append(dbname);
    response_client.push_back(0x00);
    response_client.append("caching_sha2_password");
    response_client.push_back(0x00);

    response_client[0] = response_client.size() - 4;
    try
    {
        n = co_await asio::async_write(*socket, asio::buffer(response_client), asio::use_awaitable);
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        co_return false;
    }
    std::memset(data, 0x00, 1024);
    try
    {
        n = co_await socket->async_read_some(asio::buffer(data), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        co_return false;
    }

    bool is_fast_auth = false;

    if (data[0] == 0x02 && data[4] == 0x01 && data[5] == 0x04)
    {
        seq_next_id += 2;
        std::memset(data, 0x00, 1024);
        data[0] = 0x01;
        data[3] = seq_next_id;
        data[4] = 0x02;
        try
        {
            n = co_await asio::async_write(*socket, asio::buffer(data, 5), asio::use_awaitable);
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

        std::memset(data, 0x00, 1024);
        try
        {
            n = co_await socket->async_read_some(asio::buffer(data), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
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

        seq_next_id = (data[3] & 0xFF) + 1;
        server_public_key.clear();

        for (size_t i = 5; i < n; i++)
        {
            server_public_key.push_back(data[i]);
        }

        std::string request_auth;
        request_auth = password;
        request_auth.push_back(0x00);

        mysqlnd_xor_string(&request_auth[0], request_auth.size(), server_hello.auth_plugin_salt_data.data(), challenge_length);

        // see sql-common/client_authenthication.cc line 706
        BIO *bio             = BIO_new_mem_buf(&server_public_key[0], server_public_key.size());
        EVP_PKEY *public_key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
        BIO_free(bio);

        std::size_t server_public_key_len = EVP_PKEY_get_size(public_key);
        std::string public_key_str(256, 0x00);
        // see sql-common/client_authenthication.cc line 144 or 968 php-src ext/mysqlnd/mysqlnd_auth.c
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(public_key, NULL);
        if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0 ||
            EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
            EVP_PKEY_encrypt(ctx, (unsigned char *)public_key_str.data(), &server_public_key_len, (unsigned char *)&request_auth[0], request_auth.size()) <= 0)
        {
            EVP_PKEY_CTX_free(ctx);
            error_msg.append(" server_public_key encrypt error ! ");
            error_code = 4;
            co_return false;
        }
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(public_key);

        request_auth.clear();

        unsigned char a_length = 0;
        a_length               = server_public_key_len & 0xFF;
        request_auth.push_back(a_length);
        server_public_key_len = server_public_key_len >> 8;
        a_length              = server_public_key_len & 0xFF;
        request_auth.push_back(a_length);
        server_public_key_len = server_public_key_len >> 8;
        a_length              = server_public_key_len & 0xFF;
        request_auth.push_back(a_length);
        request_auth.push_back(seq_next_id);

        for (size_t i = 0; i < public_key_str.size(); i++)
        {
            request_auth.push_back(public_key_str[i]);
        }

        try
        {
            n = co_await asio::async_write(*socket, asio::buffer(request_auth), asio::use_awaitable);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            co_return false;
        }
        std::memset(data, 0x00, 1024);

        try
        {
            n = co_await socket->async_read_some(asio::buffer(data), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            co_return false;
        }
        if ((unsigned char)data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(data[i]);
            }
            error_code = 5;
            co_return false;
        }
    }
    else if ((unsigned char)data[4] == 0xFE)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(data[i]);
        }
        error_code = 2;
        co_return false;
    }
    else if ((unsigned char)data[4] == 0xFF)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(data[i]);
        }
        error_code = 6;
        co_return false;
    }
    else
    {
        std::memset(data, 0x00, 1024);
        try
        {
            n = co_await socket->async_read_some(asio::buffer(data), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            co_return false;
        }
        if ((unsigned char)data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(data[i]);
            }
            error_code = 7;
            co_return false;
        }
        else if ((unsigned char)data[4] == 0x00)
        {
            co_return true;
        }
    }

    co_return true;
}

bool mysql_conn_base::ping()
{
    std::memset(data, 0x00, 1024);
    data[0] = 0x01;
    data[3] = 0x00;
    data[4] = 0x0E;
    try
    {
        std::size_t n = asio::write(*socket, asio::buffer(data, 5), ec);
        std::memset(data, 0x00, 1024);
        n = socket->read_some(asio::buffer(data), ec);

        return true;
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        return false;
    }
}
bool mysql_conn_base::close()
{
    std::memset(data, 0x00, 1024);
    data[0] = 0x01;
    data[3] = 0x00;
    data[4] = 0x01;
    try
    {
        isclose = false;
        asio::write(*socket, asio::buffer(data, 5), ec);
        socket->close();
        return true;
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        return false;
    }
}
}// namespace orm
