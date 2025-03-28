/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <ctime>
#include <chrono>
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
    time_start = time((time_t *)NULL);
    query_num = 0;
}
mysql_conn_base::~mysql_conn_base()
{
    if(isclose==false)
    {
        //hard_close();
        //alone use orm , exit the program, need here release smart ptr, why?
        //may be use static connect pool obj relation
        if(sock_type == 1)
        {
            socket.release();
        }
        else if(sock_type == 2)
        {
            sslsocket.release();
        }
    }
    isclose = true;
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

    pack_length               = (data[offset + 2] & 0xFF);
    pack_length               = pack_length << 8 | (data[offset + 1] & 0xFF);

    pack_length = pack_length << 8 | (data[offset] & 0xFF);

    offset               = offset + 4;
    offset = offset + pack_length;
    return offset;
}

void mysql_conn_base::read_server_hello(unsigned int offset, unsigned int length)
{
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

    if((CLIENT_SSL & server_hello.capability_flags_low ) == CLIENT_SSL)
    {
        server_enable_ssl = true;
    }

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
bool mysql_conn_base::connect(const orm_conn_t &conn_config)
{
    error_msg.clear();
    socket = std::make_unique<asio::ip::tcp::socket>(*io_ctx);
    asio::ip::tcp::resolver resolver(*io_ctx);

    asio::ip::tcp::resolver::query checkquery(conn_config.host, conn_config.port);
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
        isclose = true;
        return false;
    }
    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    std::size_t n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
    if (n < 70)
    {
        error_msg.append("mysql server back data error! ");
        error_code = 255;
        isclose = true;
        return false;
    }

    if (ec)
    {
        
        error_msg.append("connect read_some error! ");
        error_msg.append(ec.message());
        error_code = 1;
        isclose = true;
        return false;
    }

    read_server_hello(0, n);

    constexpr std::size_t challenge_length = 20;
    constexpr std::size_t response_length  = 32;

    using sha_buffer = std::uint8_t[response_length];
    sha_buffer password_sha;

    SHA256(reinterpret_cast<const unsigned char *>(conn_config.password.data()), conn_config.password.size(), password_sha);

    std::memset(_cache_data, 0x00, (response_length + challenge_length));
    SHA256(password_sha, response_length, _cache_data);
    std::memcpy(_cache_data + response_length, server_hello.auth_plugin_salt_data.data(), challenge_length);

    sha_buffer salted_password;
    SHA256(_cache_data, (response_length + challenge_length), salted_password);

    std::memset(_cache_data, 0x00, (response_length + challenge_length));

    for (unsigned i = 0; i < response_length; ++i)
    {
        _cache_data[i] = salted_password[i] ^ password_sha[i];
    }

    client_flags = CLIENT_PZORM_FLAGS;
    seq_next_id=0;
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
    send_data.append(conn_config.user);
    send_data.push_back(0x00);
    send_data.push_back(0x20);
    for (unsigned i = 0; i < response_length; ++i)
    {
        send_data.push_back(_cache_data[i]);
    }
    send_data.append(conn_config.dbname);
    send_data.push_back(0x00);
    send_data.append("caching_sha2_password");
    send_data.push_back(0x00);

    send_data[0] = send_data.size() - 4;
    sock_type = 1;
    if(conn_config.isssl && server_enable_ssl)
    {
        if(!conn_config.islocal)
        {
            //.sock please use localsocket
            send_data[0]=32;
            client_flags = CLIENT_PZORM_SSL_FLAGS;
            send_data[4]=client_flags & 0xFF;
            send_data[5]=client_flags >> 8 & 0xFF;
            send_data[6]=client_flags >> 16 & 0xFF;
            send_data[7]=client_flags >> 24 & 0xFF;

            try 
            {
                n = asio::write(*socket,asio::buffer(send_data.substr(0,36)));
            } 
            catch (std::exception &e) 
            {
                error_msg=e.what();
                error_code = 1;
                isclose = true;
                return false;
            }

            ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
            asio::ssl::stream<asio::ip::tcp::socket> ssl_temp_socket(std::move(*socket.release()), *ssl_context);
            //std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket=std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));
            sslsocket=std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));

            ssl_context->set_default_verify_paths();
            SSL_set_tlsext_host_name(sslsocket->native_handle(), "mysql");
    
            sslsocket->lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    
            ssl_context->set_verify_mode(asio::ssl::verify_peer);
            ssl_context->set_verify_callback(asio::ssl::host_name_verification("mysql"));
            sslsocket->handshake(asio::ssl::stream_base::client, ec);
            sock_type = 2;
            if (ec)
            {
                error_msg = ec.message();
                error_code = 2;
                isclose = true;
                return false;
            }

            send_data[0] = send_data.size() - 4;
            seq_next_id+=1;
            send_data[3] = seq_next_id;

            try 
            {
                n = asio::write(*sslsocket,asio::buffer(send_data));
            } 
            catch (std::exception &e) 
            {
                error_msg=e.what();
                error_code = 1;
                isclose = true;
                return false;
            }

            std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
            try
            {
                n =sslsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH));
            }
            catch (std::exception &e)
            {
                error_msg.append(e.what());
                isclose = true;
                return false;
            }

            if (_cache_data[0] == 0x02 && _cache_data[4] == 0x01 && _cache_data[5] == 0x04)
            {
                send_data.clear();
                seq_next_id+=1;
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.append(conn_config.password);
                send_data.push_back(0x00);
                send_data[0] = (send_data.size() & 0xFF) - 4;
                try 
                {
                    n = asio::write(*sslsocket,asio::buffer(send_data));
                } 
                catch (std::exception &e) 
                {
                    error_msg=e.what();
                    error_code = 1;
                    isclose = true;
                    return false;
                }
    
                std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
                try
                {
                    n = sslsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH));
                }
                catch (std::exception &e)
                {
                    error_msg.append(e.what());
                    isclose = true;
                    return false;
                }   
            }
            //feedback
            if ((unsigned char)_cache_data[4] == 0xFE)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 2;
                isclose = true;
                return false;
            }
            else if ((unsigned char)_cache_data[4] == 0xFF)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 6;
                isclose = true;
                return false;
            }
            else if ((unsigned char)_cache_data[5] == 0x03)
            {
                unsigned int pack_length = _cache_data[2];
                pack_length = pack_length << 8 | (_cache_data[1]);
                pack_length = pack_length << 8 | (_cache_data[0]);
                unsigned int offset =  pack_length + 4;    
        
                if(n > offset)
                {
                    pack_length = _cache_data[offset+2];
                    pack_length = pack_length << 8 | (_cache_data[offset+1]);
                    pack_length = pack_length << 8 | (_cache_data[offset]);
        
                    if(pack_length < 5)
                    {
                        error_msg =" connect fail! server status error! ";
                        error_code = 8;
                        isclose = true;
                        return false;
                    }
        
                    if(_cache_data[offset+4]!=0x00)   
                    {
                        error_msg =" connect fail! ";
                        error_code = 8;
                        isclose = true;
                        return false;
                    }  
                }else {
                    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
                    n = sslsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);

                    if ((unsigned char)_cache_data[4] == 0xFF)
                    {
                        for (unsigned int i = 5; i < n; i++)
                        {
                            error_msg.push_back(_cache_data[i]);
                        }
                        error_code = 8;
                        isclose = true;
                        return false;
                    }
                    else if ((unsigned char)_cache_data[4] == 0x00)
                    {
                        return true;
                    }
                }
            }
            else
            {   
                error_msg =" connect fail! ";
                error_code = 8;
                isclose = true;
                return false;
            }

            seq_next_id = 0;
            return true;
        }
    }

    n = asio::write(*socket, asio::buffer(send_data));

    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
    if (ec)
    {
        
        error_msg.append("connect handshake error! ");
        error_msg.append(ec.message());
        error_code = 1;
        isclose = true;
        return false;
    }
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
            isclose = true;
            return false;
        }

        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
        if (n < 256 || n > 1024)
        {
            error_msg.append(" get server_public_key return size: ");
            error_msg.append(std::to_string(n));
            error_code = 2;
            isclose = true;
            return false;
        }

        seq_next_id = (_cache_data[3] & 0xFF) + 1;
        bool isok   = server_public_key_encrypt(conn_config.password, &_cache_data[5], n - 5);
        if (isok == false)
        {
            isclose = true;
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
            isclose = true;
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
            isclose = true;
            return false;
        }

        if ((unsigned char)_cache_data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(_cache_data[i]);
            }
            error_code = 5;
            isclose = true;
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
        isclose = true;
        return false;
    }
    else if ((unsigned char)_cache_data[4] == 0xFF)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(_cache_data[i]);
        }
        error_code = 6;
        isclose = true;
        return false;
    }
    else if ((unsigned char)_cache_data[5] == 0x03)
    {
        unsigned int pack_length = _cache_data[2];
        pack_length = pack_length << 8 | (_cache_data[1]);
        pack_length = pack_length << 8 | (_cache_data[0]);
        unsigned int offset =  pack_length + 4;    

        if(n > offset)
        {
            pack_length = _cache_data[offset+2];
            pack_length = pack_length << 8 | (_cache_data[offset+1]);
            pack_length = pack_length << 8 | (_cache_data[offset]);

            if(pack_length < 5)
            {
                error_msg =" connect fail! server status error! ";
                error_code = 8;
                isclose = true;
                return false;
            }

            if(_cache_data[offset+4]!=0x00)   
            {
                error_msg =" connect fail! ";
                error_code = 8;
                isclose = true;
                return false;
            }  
        }else {
            std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
            n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
            if ((unsigned char)_cache_data[4] == 0xFF)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 8;
                isclose = true;
                return false;
            }
            else if ((unsigned char)_cache_data[4] == 0x00)
            {
                return true;
            }
        }
    }
    else
    {
        error_msg =" connect fail! ";
        error_code = 9;
        isclose = true;
        return false;
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
asio::awaitable<bool> mysql_conn_base::async_connect(const orm_conn_t &conn_config)
{
    error_msg.clear();
    socket = std::make_unique<asio::ip::tcp::socket>(*io_ctx);
    asio::ip::tcp::resolver resolver(*io_ctx);

    asio::ip::tcp::resolver::iterator iter = co_await resolver.async_resolve(conn_config.host, conn_config.port, asio::use_awaitable);
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
        isclose = true;
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
        isclose = true;
        co_return false;
    }

    if (n < 70)
    {
        error_msg.append("mysql server back data error! ");
        error_code = 255;
        isclose = true;
        co_return false;
    }

    read_server_hello(0, n);

    constexpr std::size_t challenge_length = 20;
    constexpr std::size_t response_length  = 32;

    using sha_buffer = std::uint8_t[response_length];
    sha_buffer password_sha;

    SHA256(reinterpret_cast<const unsigned char *>(conn_config.password.data()), conn_config.password.size(), password_sha);

    std::memset(_cache_data, 0x00, (response_length + challenge_length));
    SHA256(password_sha, response_length, _cache_data);
    std::memcpy(_cache_data + response_length, server_hello.auth_plugin_salt_data.data(), challenge_length);

    sha_buffer salted_password;
    SHA256(_cache_data, (response_length + challenge_length), salted_password);

    std::memset(_cache_data, 0x00, (response_length + challenge_length));

    for (unsigned i = 0; i < response_length; ++i)
    {
        _cache_data[i] = salted_password[i] ^ password_sha[i];
    }

    client_flags = CLIENT_PZORM_FLAGS;
    seq_next_id=0;
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
    send_data.append(conn_config.user);
    send_data.push_back(0x00);
    send_data.push_back(0x20);
    for (unsigned i = 0; i < response_length; ++i)
    {
        send_data.push_back(_cache_data[i]);
    }
    send_data.append(conn_config.dbname);
    send_data.push_back(0x00);
    send_data.append("caching_sha2_password");
    send_data.push_back(0x00);

    send_data[0] = send_data.size() - 4;
    sock_type = 1;
    if(conn_config.isssl && server_enable_ssl)
    {
        if(!conn_config.islocal)
        {
            //.sock please use localsocket
            send_data[0]=32;
            client_flags = CLIENT_PZORM_SSL_FLAGS;
            send_data[4]=client_flags & 0xFF;
            send_data[5]=client_flags >> 8 & 0xFF;
            send_data[6]=client_flags >> 16 & 0xFF;
            send_data[7]=client_flags >> 24 & 0xFF;

            try 
            {
                n = co_await asio::async_write(*socket,asio::buffer(send_data.substr(0,36)), asio::use_awaitable);
            } 
            catch (std::exception &e) 
            {
                error_msg=e.what();
                error_code = 1;
                isclose = true;
                co_return false;
            }

            ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
            asio::ssl::stream<asio::ip::tcp::socket> ssl_temp_socket(std::move(*socket.release()), *ssl_context);
            //std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket=std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));
            sslsocket=std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));

            ssl_context->set_default_verify_paths();
            SSL_set_tlsext_host_name(sslsocket->native_handle(), "mysql");
    
            sslsocket->lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    
            ssl_context->set_verify_mode(asio::ssl::verify_peer);
            ssl_context->set_verify_callback(asio::ssl::host_name_verification("mysql"));
            std::tie(ec) = co_await sslsocket->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
            sock_type = 2;
            if (ec)
            {
                error_msg = ec.message();
                error_code = 2;
                isclose = true;
                co_return false;
            }

            send_data[0] = send_data.size() - 4;
            seq_next_id+=1;
            send_data[3] = seq_next_id;

            try 
            {
                n = co_await asio::async_write(*sslsocket,asio::buffer(send_data), asio::use_awaitable);
            } 
            catch (std::exception &e) 
            {
                error_msg=e.what();
                error_code = 1;
                isclose = true;
                co_return false;
            }

            std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
            try
            {
                n = co_await sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
            }
            catch (std::exception &e)
            {
                error_msg.append(e.what());
                isclose = true;
                co_return false;
            }
            if (_cache_data[0] == 0x02 && _cache_data[4] == 0x01 && _cache_data[5] == 0x04)
            {
                send_data.clear();
                seq_next_id+=1;
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.push_back(seq_next_id);
                send_data.append(conn_config.password);
                send_data.push_back(0x00);
                send_data[0] = (send_data.size() & 0xFF) - 4;

                try 
                {
                    n = co_await asio::async_write(*sslsocket,asio::buffer(send_data), asio::use_awaitable);
                } 
                catch (std::exception &e) 
                {
                    error_msg=e.what();
                    error_code = 1;
                    isclose = true;
                    co_return false;
                }
    
                std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
                try
                {
                    n = co_await sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
                }
                catch (std::exception &e)
                {
                    error_msg.append(e.what());
                    isclose = true;
                    co_return false;
                }
 
            }
            //feedback
            if ((unsigned char)_cache_data[4] == 0xFE)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 2;
                isclose = true;
                co_return false;
            }
            else if ((unsigned char)_cache_data[4] == 0xFF)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 6;
                isclose = true;
                co_return false;
            }
            else if ((unsigned char)_cache_data[5] == 0x03)
            {
                unsigned int pack_length = _cache_data[2];
                pack_length = pack_length << 8 | (_cache_data[1]);
                pack_length = pack_length << 8 | (_cache_data[0]);
                unsigned int offset =  pack_length + 4;    
                if(n > offset)
                {
                    pack_length = _cache_data[offset+2];
                    pack_length = pack_length << 8 | (_cache_data[offset+1]);
                    pack_length = pack_length << 8 | (_cache_data[offset]);
                    if(pack_length < 5)
                    {
                        error_msg =" connect fail! server status error! ";
                        error_code = 8;
                        isclose = true;
                        co_return false;
                    }

                    if(_cache_data[offset+4]==0x00)   
                    {
                        co_return true;
                    }
                    else 
                    {
                        error_msg =" connect fail! ";
                        error_code = 8;
                        isclose = true;
                        co_return false;
                    }  
                }else {
                    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
                    try
                    {
                        n = co_await sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
                    }
                    catch (std::exception &e)
                    {
                        error_msg.append(e.what());
                        isclose = true;
                        co_return false;
                    }
                    if ((unsigned char)_cache_data[4] == 0xFF)
                    {
                        for (unsigned int i = 5; i < n; i++)
                        {
                            error_msg.push_back(_cache_data[i]);
                        }
                        error_code = 7;
                        isclose = true;
                        co_return false;
                    }
                    else if ((unsigned char)_cache_data[4] == 0x00)
                    {
                        co_return true;
                    }
                    else 
                    {
                        error_msg =" async read status connect fail! ";
                        error_code = 9;
                        isclose = true;
                        co_return false;
                    }
                }
            }
            else
            {   
                error_msg =" connect fail! ";
                error_code = 8;
                isclose = true;
                co_return false;
            }

            seq_next_id = 0;
            isclose = true;
            co_return false;
        }
    }

    try
    {
        n = co_await asio::async_write(*socket, asio::buffer(send_data), asio::use_awaitable);
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        isclose = true;
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
        isclose = true;
        co_return false;
    }

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
            isclose = true;
            co_return false;
        }
        if (n == 0)
        {
            error_msg.append(" request server_public_key error ! ");
            error_code = 3;
            isclose = true;
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
            isclose = true;
            co_return false;
        }
        if (n < 256 || n > 1024)
        {
            error_msg.append(" get server_public_key return size: ");
            error_msg.append(std::to_string(n));
            error_code = 2;
            isclose = true;
            co_return false;
        }

        seq_next_id = (_cache_data[3] & 0xFF) + 1;
        bool isok   = server_public_key_encrypt(conn_config.password, &_cache_data[5], n - 5);
        if (isok == false)
        {
            isclose = true;
            co_return false;
        }

        try
        {
            n = co_await asio::async_write(*socket, asio::buffer(send_data), asio::use_awaitable);
        }
        catch (std::exception &e)
        {
            error_msg.append(e.what());
            isclose = true;
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
            isclose = true;
            co_return false;
        }

        if ((unsigned char)_cache_data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(_cache_data[i]);
            }
            error_code = 5;
            isclose = true;
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
        isclose = true;
        co_return false;
    }
    else if ((unsigned char)_cache_data[4] == 0xFF)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(_cache_data[i]);
        }
        error_code = 6;
        isclose = true;
        co_return false;
    }
    else if ((unsigned char)_cache_data[5] == 0x03)
    {
        unsigned int pack_length = _cache_data[2];
        pack_length = pack_length << 8 | (_cache_data[1]);
        pack_length = pack_length << 8 | (_cache_data[0]);
        unsigned int offset =  pack_length + 4;    
        if(n > offset)
        {
            pack_length = _cache_data[offset+2];
            pack_length = pack_length << 8 | (_cache_data[offset+1]);
            pack_length = pack_length << 8 | (_cache_data[offset]);
            if(pack_length < 5)
            {
                error_msg =" connect fail! server status error! ";
                error_code = 8;
                isclose = true;
                co_return false;
            }

            if(_cache_data[offset+4]==0x00)   
            {
                co_return true;
            }
            else 
            {
                error_msg =" connect fail! ";
                error_code = 8;
                isclose = true;
                co_return false;
            }  
        }else {
            std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
            try
            {
                n = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
            }
            catch (std::exception &e)
            {
                error_msg.append(e.what());
                isclose = true;
                co_return false;
            }
            if ((unsigned char)_cache_data[4] == 0xFF)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 7;
                isclose = true;
                co_return false;
            }
            else if ((unsigned char)_cache_data[4] == 0x00)
            {
                co_return true;
            }
            else 
            {
                error_msg =" async read status connect fail! ";
                error_code = 9;
                isclose = true;
                co_return false;
            }
        }
    }
    else
    {   
        error_msg =" connect fail! ";
        error_code = 8;
        isclose = true;
        co_return false;
    }

    co_return true;
}
asio::awaitable<unsigned int> mysql_conn_base::async_read_loop()
{
    try
    {
        if(isclose)
        {
            co_return 0;
        }
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        std::size_t n=0;
        if(sock_type == 1)
        {
            n = co_await socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
        else if(sock_type == 2)
        {
            n = co_await sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
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
        if(isclose)
        {
            return 0;
        }
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        std::size_t n=0;
        if(sock_type == 1)
        {
            n = socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
        }
        else if(sock_type == 2)
        {
            n = sslsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), ec);
        }

        if (ec)
        {
            error_code = 1;
            error_msg = ec.message();
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

void mysql_conn_base::begin_time()
{
    time_begin = std::chrono::steady_clock::now();
}
void mysql_conn_base::finish_time()
{
    time_finish = std::chrono::steady_clock::now();
}
long long mysql_conn_base::count_time()
{
    long long time_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(time_finish - time_begin).count();
    return time_duration;
}

unsigned int mysql_conn_base::write_sql(const std::string &sql)
{
    unsigned int n=0;

    n = sql.length() + 1;

    send_data.clear();
    send_data.push_back((n & 0xFF));
    send_data.push_back((n >> 8 & 0xFF));
    send_data.push_back((n >> 16 & 0xFF));
    send_data.push_back(0x00);
    send_data.push_back(0x03);
    send_data.append(sql);

    n=0;
    
    if(isclose)
    {
        return 0;
    }
    try 
    {

        if(sock_type == 1)
        {
            n = asio::write(*socket, asio::buffer(send_data),ec);
        }
        else if(sock_type == 2)
        {
            //n = asio::write(*sslsocket, asio::buffer(send_data),ec);
            n = asio::write(*sslsocket,asio::buffer(send_data),ec);
        }
        if(ec)
        {
            error_code         = 20;
            error_msg = ec.message();
            return 0;
        }
    }
    catch (std::exception &e)
    {
        error_code         = 20;
        error_msg = ec.message();
        return 0;
    }
    query_num ++ ;
    return n;
}

unsigned int mysql_conn_base::write()
{
    unsigned int n=0;
    if(isclose)
    {
        return 0;
    }
    try
    {
        if(sock_type == 1)
        {
            n = asio::write(*socket, asio::buffer(send_data),ec);
        }
        else if(sock_type == 2)
        {
            n = asio::write(*sslsocket, asio::buffer(send_data),ec);
        }
        if(ec)
        {
            error_code         = 20;
            error_msg = ec.message();
            return 0;
        }
    }
    catch (std::exception &e)
    {
        error_code         = 20;
        error_msg = ec.message();
        return 0;
    }
    query_num ++ ;
    return n;
}

asio::awaitable<unsigned int> mysql_conn_base::async_write_sql(const std::string &sql)
{
    unsigned int n=0;

    n = sql.length() + 1;

    send_data.clear();
    send_data.push_back((n & 0xFF));
    send_data.push_back((n >> 8 & 0xFF));
    send_data.push_back((n >> 16 & 0xFF));
    send_data.push_back(0x00);
    send_data.push_back(0x03);
    send_data.append(sql);

    n=0;
    try 
    {
        if(isclose)
        {
            co_return 0;
        }

        if(sock_type == 1)
        {
            n = co_await asio::async_write(*socket, asio::buffer(send_data), asio::use_awaitable);
        }
        else if(sock_type == 2)
        {
            n = co_await asio::async_write(*sslsocket, asio::buffer(send_data), asio::use_awaitable);
        }
    } 
    catch (std::exception &e)
    {
        error_code         = 20;
        error_msg = ec.message();
        co_return 0;
    }
    query_num ++ ;
    co_return n;
}

asio::awaitable<unsigned int> mysql_conn_base::async_write()
{
    unsigned int n=0;
    try 
    {
        if(isclose)
        {
            co_return 0;
        }
 
        if(sock_type == 1)
        {
            n = co_await asio::async_write(*socket, asio::buffer(send_data), asio::use_awaitable);
        }
        else if(sock_type == 2)
        {
            n = co_await asio::async_write(*sslsocket, asio::buffer(send_data), asio::use_awaitable);
        }
    } 
    catch (std::exception &e)
    {
        error_code         = 20;
        error_msg = ec.message();
        co_return 0;
    }
    query_num ++ ;
    co_return n;
}

bool mysql_conn_base::ping()
{
    char data_send[16] = {0x01, 0x00, 0x00, 0x00, 0x0E, 0x00};
    error_code         = 0;
    error_msg.clear();
    try
    {
        if(isclose)
        {
            return false;
        }

        if(sock_type == 1)
        {
            asio::write(*socket, asio::buffer(data_send, 5), ec);
            socket->read_some(asio::buffer(data_send, 16), ec);
        }
        else if(sock_type == 2)
        {
            asio::write(*sslsocket, asio::buffer(data_send, 5), ec);
            sslsocket->read_some(asio::buffer(data_send, 16), ec);
        }

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
    if(isclose)
    {
        return false;
    }
    char data_send[6] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
    error_code        = 0;
    error_msg.clear();
    try
    {
        isclose = true;
        if(sock_type == 1)
        {
            if(socket->is_open())
            {
                asio::write(*socket, asio::buffer(data_send, 5), ec);
                socket->close();
            }
        }
        else if(sock_type == 2)
        {
            asio::write(*sslsocket, asio::buffer(data_send, 5), ec);
            sslsocket->shutdown(ec);
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().close();
            }
        }
        return true;
    }
    catch (std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return false;
    }
}
asio::awaitable<bool> mysql_conn_base::async_close()
{
    if(isclose)
    {
        co_return false;
    }
    char data_send[6] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
    error_code        = 0;
    error_msg.clear();
    try
    {
        isclose = true;

        if(sock_type == 1)
        {
            if(socket->is_open())
            {
                co_await asio::async_write(*socket, asio::buffer(data_send, 5), asio::use_awaitable);
                socket->close();
            }
        }
        else if(sock_type == 2)
        {
            co_await asio::async_write(*sslsocket, asio::buffer(data_send, 5), asio::use_awaitable);
            co_await sslsocket->async_shutdown(asio::use_awaitable);
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().close();
            }
        }

        co_return true;
    }
    catch (std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        co_return false;
    }
}

bool mysql_conn_base::hard_close()
{
    try
    {
        isclose = true;
        if(sock_type == 1)
        {
            if(socket->is_open())
            {
                socket->close();
            }
        }
        else if(sock_type == 2)
        {
            sslsocket->shutdown(ec);
            if (sslsocket->lowest_layer().is_open())
            {
                sslsocket->lowest_layer().close();
            }
        }
    }
    catch (std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return false;
    }
    return true;
}

void mysql_conn_base::reset_data()
{
    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
}

bool mysql_conn_base::is_closed()
{
    try
    {
        if(sock_type == 1)
        {
            if (socket->is_open())
            {
                return true;
            }
        }
        else if(sock_type == 2)
        {
            if (sslsocket->lowest_layer().is_open())
            {
                return true;
            }
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
        pack_info.data.clear();
        if(offset + 4 >= total_num)
        {
            pack_info.padd_length=0;
            for (;offset < total_num; offset++)
            {
                pack_info.padd_str[pack_info.padd_length]=data[offset]; 
                pack_info.padd_length+=1;   //not enough pack
            }
            return;
        }
        unsigned char seq_id=0; 
        if(pack_info.padd_length > 0)
        {

            for (;pack_info.padd_length < 4; pack_info.padd_length++)
            {
                pack_info.padd_str[pack_info.padd_length]=data[offset]; 
                offset+=1;   //padding 4 char
            }

            pack_length              = (pack_info.padd_str[seq_id + 2] & 0xFF);
            pack_length              = pack_length << 8 | (pack_info.padd_str[seq_id + 1] & 0xFF);
            pack_length              = pack_length << 8 | (pack_info.padd_str[seq_id] & 0xFF);
            seq_id = pack_info.padd_str[seq_id + 3];
            pack_info.padd_length =0;
            begin_length+=offset;
        }
        else 
        {
            pack_length              = (data[offset + 2] & 0xFF);
            pack_length              = pack_length << 8 | (data[offset + 1] & 0xFF);
            pack_length              = pack_length << 8 | (data[offset] & 0xFF);
            offset                   = offset + 3;
            seq_id = data[offset];
            offset++;
            begin_length+=4;
        }

        if (seq_id != pack_info.seq_id)
        {
            pack_info.error = 1;
            pack_info.data  = "mysql read package seq_id error !";
            return;
        }
        pack_info.seq_id = (pack_info.seq_id + 1) % 256;
        offset = offset + pack_length;

        pack_info.length         = pack_length;
        pack_info.current_length = pack_length;
        if (offset > total_num)
        {

            pack_info.current_length = total_num - begin_length;
            pack_length              = total_num - begin_length;
        }
        pack_info.data.append((char *)&data[begin_length], pack_length);
    }
}

unsigned int mysql_conn_base::pack_real_num(const unsigned char *data, unsigned int &offset)
{
    unsigned int pack_length = 0;

    pack_length = (data[offset] & 0xFF);
    if (pack_length < 251)
    {

        offset++;
    }
    else if (pack_length == 251)
    {

        pack_length = 0;
        offset++;
    }
    else if (pack_length == 252)
    {
        offset++;
        pack_length = (data[offset + 1] & 0xFF);
        pack_length = pack_length << 8 | (data[offset] & 0xFF);

        offset += 2;
    }
    else if (pack_length == 253)
    {
        offset++;
        pack_length = (data[offset + 2] & 0xFF);
        pack_length = pack_length << 8 | (data[offset + 1] & 0xFF);
        pack_length = pack_length << 8 | (data[offset] & 0xFF);

        offset += 3;
    }
    else if (pack_length == 254)
    {
        offset++;
        pack_length = (data[offset + 7] & 0xFF);
        pack_length = pack_length << 8 | (data[offset + 6] & 0xFF);
        pack_length = pack_length << 8 | (data[offset + 5] & 0xFF);
        pack_length = pack_length << 8 | (data[offset + 4] & 0xFF);
        pack_length = pack_length << 8 | (data[offset + 3] & 0xFF);
        pack_length = pack_length << 8 | (data[offset + 2] & 0xFF);
        pack_length = pack_length << 8 | (data[offset + 1] & 0xFF);
        pack_length = pack_length << 8 | (data[offset] & 0xFF);

        offset += 8;
    }
    return pack_length;
}
void mysql_conn_base::read_col_info(const std::string &pack_info, field_info_t &temp_field_data)
{
    if (pack_info.size() < 20)
    {
        error_code = 10;
        error_msg  = " read pakage error field info size < 20";
        return;
    }
    unsigned int tempnum     = 0;
    unsigned int name_length = pack_info[tempnum];
    if (name_length == 3)
    {
        if (pack_info[tempnum + 1] == 0x64 && pack_info[tempnum + 2] == 0x65 && pack_info[tempnum + 3] == 0x66)
        {
            tempnum += 3;
        }
        else
        {
            error_code = 10;
            error_msg  = "read pakage error The catalog used. Currently always 'def' ";
            return;
        }
    }
    else
    {
        error_code = 10;
        error_msg  = "read pakage error The catalog used. Currently always 'def' ";
        return;
    }
    tempnum++;
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.dbname.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.table.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.org_table.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.name.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;

    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.org_name.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    name_length = pack_info[tempnum];

    if (name_length != 0x0C)
    {
        error_code = 10;
        error_msg  = "read pakage error field is not 0x0C fixed! ";
        return;
    }
    tempnum++;

    temp_field_data.charset = pack_info[tempnum + 1];
    temp_field_data.charset = temp_field_data.charset << 8 | (pack_info[tempnum] & 0xFF);
    tempnum += 2;

    temp_field_data.column_length = pack_info[tempnum + 3];
    temp_field_data.column_length = temp_field_data.column_length << 8 | (pack_info[tempnum + 2] & 0xFF);
    temp_field_data.column_length = temp_field_data.column_length << 8 | (pack_info[tempnum + 1] & 0xFF);
    temp_field_data.column_length = temp_field_data.column_length << 8 | (pack_info[tempnum] & 0xFF);

    tempnum += 4;

    temp_field_data.field_type = pack_info[tempnum];
    tempnum++;
    temp_field_data.flags = pack_info[tempnum + 1];
    temp_field_data.flags = temp_field_data.flags << 8 | (pack_info[tempnum] & 0xFF);

    tempnum += 2;
    temp_field_data.decimals = pack_info[tempnum];
}
bool mysql_conn_base::pack_eof_check(const pack_info_t &temp_pack_data)
{
    if(temp_pack_data.data.size()==0)
    {
        return true;
    }

    if ((unsigned char)temp_pack_data.data[0] == 0xFF)
    {
        error_msg = temp_pack_data.data.substr(3);
        return true;
    }
    else if ((unsigned char)temp_pack_data.data[0] == 0xFE)
    {
        return true;
    }
    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
    {
        return true;
    }
    return false;
}

}// namespace orm