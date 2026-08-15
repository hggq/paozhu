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
#include <stdexcept>
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
unsigned int mysql_charset_store::mysql_charset_find(const std::string &str)
{
    auto iter = mysql_charset.find(str);
    if (iter == mysql_charset.end())
    {
        return 45;//default utf8mb4;
    }
    else
    {
        return iter->second;
    }
    return 45;
}
void mysql_charset_store::mysql_charset_clear()
{
    mysql_charset.clear();
}
mysql_charset_store::~mysql_charset_store()
{
    mysql_charset.clear();
}
void mysql_charset_store::mysql_charset_init()
{
    mysql_charset["armscii8"] = 32;
    mysql_charset["ascii"]    = 11;
    mysql_charset["big5"]     = 1;
    mysql_charset["binary"]   = 63;
    mysql_charset["cp1250"]   = 26;
    mysql_charset["cp1251"]   = 14;
    mysql_charset["cp1256"]   = 57;
    mysql_charset["cp1257"]   = 29;
    mysql_charset["cp850"]    = 4;
    mysql_charset["cp852"]    = 40;
    mysql_charset["cp866"]    = 36;
    mysql_charset["cp932"]    = 95;
    mysql_charset["dec8"]     = 3;
    mysql_charset["eucjpms"]  = 97;
    mysql_charset["euckr"]    = 19;
    mysql_charset["gb18030"]  = 248;
    mysql_charset["gb2312"]   = 24;
    mysql_charset["gbk"]      = 28;
    mysql_charset["geostd8"]  = 92;
    mysql_charset["greek"]    = 25;
    mysql_charset["hebrew"]   = 16;
    mysql_charset["hp8"]      = 6;
    mysql_charset["keybcs2"]  = 37;
    mysql_charset["koi8r"]    = 7;
    mysql_charset["koi8u"]    = 22;
    mysql_charset["latin1"]   = 5;
    mysql_charset["latin2"]   = 2;
    mysql_charset["latin5"]   = 30;
    mysql_charset["latin7"]   = 20;
    mysql_charset["macce"]    = 38;
    mysql_charset["macroman"] = 39;
    mysql_charset["sjis"]     = 13;
    mysql_charset["swe7"]     = 10;
    mysql_charset["tis620"]   = 18;
    mysql_charset["ucs2"]     = 35;
    mysql_charset["ujis"]     = 12;
    mysql_charset["utf16"]    = 54;
    mysql_charset["utf16le"]  = 56;
    mysql_charset["utf32"]    = 60;
    mysql_charset["utf8mb3"]  = 33;
    mysql_charset["utf8mb4"]  = 45;
}

mysql_charset_store &get_orm_mysql_charset()
{
    static mysql_charset_store instance;
    return instance;
}
mysql_conn_base::mysql_conn_base(std::unique_ptr<orm_conn_link_t> link) : conn_link(std::move(link))
{
    error_code  = 0;
    auto &cc    = http::get_client_data_cache();
    _cache_data = cc.get_data_ptr();
    time_start  = time((time_t *)NULL);
    query_num   = 0;
}
mysql_conn_base::~mysql_conn_base()
{
    if (isclose == false)
    {
        if (conn_link)
        {
            try
            {
                conn_link->close();
            }
            catch (...)
            {
            }
        }
    }
    isclose = true;
    if (_cache_data != nullptr)
    {
        auto &cc = http::get_client_data_cache();
        cc.back_data_ptr(_cache_data);
        _cache_data = nullptr;
    }
}

// Ref: php-src ext/mysqlnd/mysqlnd_auth.c mysqlnd_xor_string
//      sql-common/client_authentication.cc xor_string
// The upstream condition is i <= dst_len, because its dst_len = strlen(passwd) (excluding '\0'),
// 上游使用 i <= dst_len, 因为其 dst_len = strlen(passwd)(不含'\0'),
// <= 是为了把结尾 '\0' 也异或(RSA 明文包含 password+'\0').
// 本实现调用方已 push_back(0x00), dst_len = send_data.size() 含 '\0',
// 故用 < 即可, 避免越界写入 std::string 内部 '\0'(UB).
void mysql_conn_base::mysqlnd_xor_string(char *dst, const size_t dst_len, const char *xor_str, const size_t xor_str_len)
{
    unsigned int i;
    for (i = 0; i < dst_len; ++i)
    {
        dst[i] ^= xor_str[i % xor_str_len];
    }
}
unsigned int mysql_conn_base::read_pack(unsigned char *data, unsigned int offset)
{
    unsigned int pack_length = 0;

    pack_length = (data[offset + 2] & 0xFF);
    pack_length = pack_length << 8 | (data[offset + 1] & 0xFF);

    pack_length = pack_length << 8 | (data[offset] & 0xFF);

    offset = offset + 4;
    offset = offset + pack_length;
    return offset;
}

void mysql_conn_base::read_server_hello(unsigned int offset, unsigned int length)
{
    // 外部已检查 n >= 70，但为了安全仍添加边界检查
    if (length < 70)
    {
        error_msg  = "server hello packet too short";
        error_code = 255;
        isclose    = true;
        return;
    }

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

    // 检查剩余数据是否足够读取 connection_id (4字节)
    if (offset + 4 > length)
    {
        error_msg  = "server hello packet truncated at connection_id";
        error_code = 255;
        isclose    = true;
        return;
    }

    // connection_id 为小端序 4 字节整数
    unsigned int server_thread_num = static_cast<unsigned int>(_cache_data[offset]) | (static_cast<unsigned int>(_cache_data[offset + 1]) << 8) | (static_cast<unsigned int>(_cache_data[offset + 2]) << 16) | (static_cast<unsigned int>(_cache_data[offset + 3]) << 24);
    offset += 4;
    server_hello.connection_id = server_thread_num;

    // 检查 auth_plugin_salt_data (8字节)
    if (offset + 8 > length)
    {
        error_msg  = "server hello packet truncated at salt";
        error_code = 255;
        isclose    = true;
        return;
    }
    for (unsigned int i = 0; i < 8; i++)
    {
        server_hello.auth_plugin_salt_data.push_back(_cache_data[offset]);
        offset++;
    }
    offset++;

    // 检查 capability_flags_low (2字节)
    if (offset + 2 > length)
    {
        error_msg  = "server hello packet truncated at capability_flags_low";
        error_code = 255;
        isclose    = true;
        return;
    }
    server_hello.capability_flags_low = _cache_data[offset + 1];
    server_hello.capability_flags_low = server_hello.capability_flags_low << 8 | (_cache_data[offset] & 0xFF);
    offset += 2;

    // 检查 character_set (1字节) + status_flags (2字节)
    if (offset + 3 > length)
    {
        error_msg  = "server hello packet truncated at charset/status";
        error_code = 255;
        isclose    = true;
        return;
    }
    server_hello.character_set = _cache_data[offset];
    offset++;
    server_hello.status_flags = _cache_data[offset + 1];
    server_hello.status_flags = server_hello.status_flags << 8 | _cache_data[offset];
    offset += 2;

    // 检查 capability_flags_high (2字节)
    if (offset + 2 > length)
    {
        error_msg  = "server hello packet truncated at capability_flags_high";
        error_code = 255;
        isclose    = true;
        return;
    }
    server_hello.capability_flags_high = _cache_data[offset + 1];
    server_hello.capability_flags_high = server_hello.capability_flags_high << 8 | (_cache_data[offset] & 0xFF);
    offset += 2;

    if ((CLIENT_SSL & server_hello.capability_flags_low) == CLIENT_SSL)
    {
        server_enable_ssl = true;
    }

    // 检查 auth_plugin_data_len (1字节)
    if (offset >= length)
    {
        error_msg  = "server hello packet truncated at auth_plugin_data_len";
        error_code = 255;
        isclose    = true;
        return;
    }
    server_hello.auth_plugin_data_len = _cache_data[offset];
    offset++;

    // 跳过10字节保留字段
    if (offset + 10 > length)
    {
        error_msg  = "server hello packet truncated at reserved";
        error_code = 255;
        isclose    = true;
        return;
    }
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
        if (offset >= length)
        {
            break;
        }
        if (_cache_data[offset] == 0x00)
        {
            offset++;
            break;
        }
        server_hello.auth_plugin_name.push_back(_cache_data[offset]);
        offset++;
    }
}
bool mysql_conn_base::connect(const orm_conn_t &conn_config)
{
    error_msg.clear();

    if (conn_config.issock)
    {
        // Unix domain socket connection
        try
        {
            conn_link->localsocket = std::make_unique<asio::local::stream_protocol::socket>(*conn_link->io_ctx);
            asio::local::stream_protocol::endpoint ep(conn_config.host);
            conn_link->localsocket->connect(ep, conn_link->ec);
        }
        catch (const std::exception &e)
        {
            error_msg = "local socket connect error: ";
            error_msg.append(e.what());
            error_code = 1;
            isclose    = true;
            return false;
        }
        if (conn_link->ec)
        {
            error_msg.append("local socket connect error! ");
            error_msg.append(conn_link->ec.message());
            error_code = 1;
            isclose    = true;
            return false;
        }
        conn_link->sock_type = 3;
    }
    else
    {
        asio::ip::tcp::resolver resolver(*conn_link->io_ctx);
        auto endpoints = resolver.resolve(conn_config.host, conn_config.port);
        asio::connect(*conn_link->socket, endpoints, conn_link->ec);

        // asio::ip::tcp::resolver::query checkquery(conn_config.host, conn_config.port);
        // asio::ip::tcp::resolver::iterator iter = resolver.resolve(checkquery);
        // asio::ip::tcp::resolver::iterator end;
        // asio::ip::tcp::endpoint endpoint;

        // while (iter != end)
        // {
        //     endpoint = *iter++;
        //     socket->connect(endpoint, ec);
        //     if (ec)
        //     {
        //         continue;
        //     }
        //     else
        //     {
        //         break;
        //     }
        // }
        // asio::error_code ec;
        if (conn_link->ec)
        {
            error_msg.append("host connect error! ");
            error_msg.append(conn_link->ec.message());
            error_code = 1;
            isclose    = true;
            return false;
        }
    }// end else (TCP)
    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    std::size_t n = 0;
    if (conn_link->sock_type == 3)
    {
        n = conn_link->localsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
    }
    else
    {
        n = conn_link->socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
    }
    if (n < 70)
    {
        error_msg.append("mysql server back data error! ");
        error_code = 255;
        isclose    = true;
        return false;
    }

    if (conn_link->ec)
    {

        error_msg.append("connect read_some error! ");
        error_msg.append(conn_link->ec.message());
        error_code = 1;
        isclose    = true;
        return false;
    }

    read_server_hello(0, n);

    if (server_hello.auth_plugin_salt_data.size() < 20)
    {
        error_msg = "auth plugin salt too short";
        return false;
    }

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
    seq_next_id  = 0;
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

    send_data.push_back(static_cast<char>(0xFF));// max pack
    send_data.push_back(static_cast<char>(0xFF));
    send_data.push_back(static_cast<char>(0xFF));

    send_data.push_back(0x00);
    if (conn_config.charset_val > 0)
    {
        send_data.push_back(conn_config.charset_val);
    }
    else
    {
        send_data.push_back(0x2D);// charset utf8mb4_general_ci
    }

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
    if (conn_link->sock_type == 3)
    {
        // Unix socket: no SSL, keep sock_type=3
    }
    else
    {
        conn_link->sock_type = 1;
    }
    if (conn_config.isssl && server_enable_ssl && !conn_config.issock)
    {
        if (!conn_config.islocal)
        {
            //.sock please use localsocket
            send_data[0] = 32;
            client_flags = CLIENT_PZORM_SSL_FLAGS;
            send_data[4] = client_flags & 0xFF;
            send_data[5] = client_flags >> 8 & 0xFF;
            send_data[6] = client_flags >> 16 & 0xFF;
            send_data[7] = client_flags >> 24 & 0xFF;

            try
            {
                n = asio::write(*conn_link->socket, asio::buffer(send_data.substr(0, 36)));
            }
            catch (const std::exception &e)
            {
                error_msg  = e.what();
                error_code = 1;
                isclose    = true;
                return false;
            }

            conn_link->ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
            asio::ssl::stream<asio::ip::tcp::socket> ssl_temp_socket(std::move(*conn_link->socket.release()), *conn_link->ssl_context);
            //std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket=std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));
            conn_link->sslsocket = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));

            if (conn_config.sslverify)
            {
                conn_link->ssl_context->set_default_verify_paths();
                conn_link->ssl_context->set_verify_mode(asio::ssl::verify_peer);
            }
            const std::string &sni_host = conn_config.sslhost.empty() ? conn_config.host : conn_config.sslhost;
            if (!sni_host.empty())
            {
                SSL_set_tlsext_host_name(conn_link->sslsocket->native_handle(), sni_host.c_str());
            }
            if (conn_config.sslverify && !conn_config.sslhost.empty())
            {
                conn_link->ssl_context->set_verify_callback(asio::ssl::host_name_verification(conn_config.sslhost));
            }

            conn_link->sslsocket->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

            conn_link->sslsocket->handshake(asio::ssl::stream_base::client, conn_link->ec);
            conn_link->sock_type = 2;
            if (conn_link->ec)
            {
                error_msg  = conn_link->ec.message();
                error_code = 2;
                isclose    = true;
                return false;
            }

            send_data[0] = send_data.size() - 4;
            seq_next_id += 1;
            send_data[3] = seq_next_id;

            try
            {
                n = asio::write(*conn_link->sslsocket, asio::buffer(send_data));
            }
            catch (const std::exception &e)
            {
                error_msg  = e.what();
                error_code = 1;
                isclose    = true;
                return false;
            }

            std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
            try
            {
                n = conn_link->sslsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH));
            }
            catch (const std::exception &e)
            {
                error_msg.append(e.what());
                isclose = true;
                return false;
            }

            if (n < 5)
            {
                error_msg = "response too short";
                return false;
            }

            if (_cache_data[0] == 0x02 && _cache_data[4] == 0x01 && _cache_data[5] == 0x04)
            {
                send_data.clear();
                seq_next_id += 1;
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.append(conn_config.password);
                send_data.push_back(0x00);
                send_data[0] = (send_data.size() & 0xFF) - 4;
                try
                {
                    n = asio::write(*conn_link->sslsocket, asio::buffer(send_data));
                }
                catch (const std::exception &e)
                {
                    error_msg  = e.what();
                    error_code = 1;
                    isclose    = true;
                    return false;
                }

                std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
                try
                {
                    n = conn_link->sslsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH));
                }
                catch (const std::exception &e)
                {
                    error_msg.append(e.what());
                    isclose = true;
                    return false;
                }
            }

            if (n < 5)
            {
                error_msg = "response too short";
                return false;
            }

            //feedback
            if ((unsigned char)_cache_data[4] == 0xFE)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 2;
                isclose    = true;
                return false;
            }
            else if ((unsigned char)_cache_data[4] == 0xFF)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 6;
                isclose    = true;
                return false;
            }
            else if ((unsigned char)_cache_data[5] == 0x03)
            {
                unsigned int pack_length = _cache_data[2];
                pack_length              = pack_length << 8 | (_cache_data[1]);
                pack_length              = pack_length << 8 | (_cache_data[0]);
                unsigned int offset      = pack_length + 4;

                if (n >= offset + 3)
                {
                    pack_length = _cache_data[offset + 2];
                    pack_length = pack_length << 8 | (_cache_data[offset + 1]);
                    pack_length = pack_length << 8 | (_cache_data[offset]);

                    if (pack_length < 5)
                    {
                        error_msg  = " connect fail! server status error! ";
                        error_code = 8;
                        isclose    = true;
                        return false;
                    }

                    if (_cache_data[offset + 4] != 0x00)
                    {
                        error_msg  = " connect fail! ";
                        error_code = 8;
                        isclose    = true;
                        return false;
                    }
                }
                else
                {
                    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
                    n = conn_link->sslsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);

                    if (n < 5)
                    {
                        error_msg = "response too short";
                        return false;
                    }

                    if ((unsigned char)_cache_data[4] == 0xFF)
                    {
                        for (unsigned int i = 5; i < n; i++)
                        {
                            error_msg.push_back(_cache_data[i]);
                        }
                        error_code = 8;
                        isclose    = true;
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
                error_msg  = " connect fail! ";
                error_code = 8;
                isclose    = true;
                return false;
            }

            seq_next_id = 0;
            return true;
        }
    }

    if (conn_link->sock_type == 3)
    {
        n = asio::write(*conn_link->localsocket, asio::buffer(send_data));
    }
    else
    {
        n = asio::write(*conn_link->socket, asio::buffer(send_data));
    }

    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    if (conn_link->sock_type == 3)
    {
        n = conn_link->localsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
    }
    else
    {
        n = conn_link->socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
    }
    if (conn_link->ec)
    {

        error_msg.append("connect handshake error! ");
        error_msg.append(conn_link->ec.message());
        error_code = 1;
        isclose    = true;
        return false;
    }

    if (n < 5)
    {
        error_msg = "response too short";
        return false;
    }

    if (_cache_data[0] == 0x02 && _cache_data[4] == 0x01 && _cache_data[5] == 0x04)
    {
        seq_next_id += 2;
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        _cache_data[0] = 0x01;
        _cache_data[3] = seq_next_id;
        _cache_data[4] = 0x02;
        if (conn_link->sock_type == 3)
        {
            n = asio::write(*conn_link->localsocket, asio::buffer(_cache_data, 5));
        }
        else
        {
            n = asio::write(*conn_link->socket, asio::buffer(_cache_data, 5));
        }
        if (n == 0)
        {
            error_msg.append(" request server_public_key error ! ");
            error_code = 3;
            isclose    = true;
            return false;
        }

        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        if (conn_link->sock_type == 3)
        {
            n = conn_link->localsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
        }
        else
        {
            n = conn_link->socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
        }
        if (n < 256 || n > 1024)
        {
            error_msg.append(" get server_public_key return size: ");
            error_msg.append(std::to_string(n));
            error_code = 2;
            isclose    = true;
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
            if (conn_link->sock_type == 3)
            {
                n = asio::write(*conn_link->localsocket, asio::buffer(send_data));
            }
            else
            {
                n = asio::write(*conn_link->socket, asio::buffer(send_data));
            }
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
            error_code = 4;
            isclose    = true;
            return false;
        }

        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);

        try
        {
            if (conn_link->sock_type == 3)
            {
                n = conn_link->localsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
            }
            else
            {
                n = conn_link->socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
            }
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
            error_code = 4;
            isclose    = true;
            return false;
        }

        if (n < 5)
        {
            error_msg = "response too short";
            return false;
        }

        if ((unsigned char)_cache_data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(_cache_data[i]);
            }
            error_code = 5;
            isclose    = true;
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
        isclose    = true;
        return false;
    }
    else if ((unsigned char)_cache_data[4] == 0xFF)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(_cache_data[i]);
        }
        error_code = 6;
        isclose    = true;
        return false;
    }
    else if ((unsigned char)_cache_data[5] == 0x03)
    {
        unsigned int pack_length = _cache_data[2];
        pack_length              = pack_length << 8 | (_cache_data[1]);
        pack_length              = pack_length << 8 | (_cache_data[0]);
        unsigned int offset      = pack_length + 4;

        if (n >= offset + 3)
        {
            pack_length = _cache_data[offset + 2];
            pack_length = pack_length << 8 | (_cache_data[offset + 1]);
            pack_length = pack_length << 8 | (_cache_data[offset]);

            if (pack_length < 5)
            {
                error_msg  = " connect fail! server status error! ";
                error_code = 8;
                isclose    = true;
                return false;
            }

            if (_cache_data[offset + 4] != 0x00)
            {
                error_msg  = " connect fail! ";
                error_code = 8;
                isclose    = true;
                return false;
            }
        }
        else
        {
            std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
            if (conn_link->sock_type == 3)
            {
                n = conn_link->localsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
            }
            else
            {
                n = conn_link->socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
            }

            if (n < 5)
            {
                error_msg = "response too short";
                return false;
            }

            if ((unsigned char)_cache_data[4] == 0xFF)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 8;
                isclose    = true;
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
        error_msg  = " connect fail! ";
        error_code = 9;
        isclose    = true;
        return false;
    }

    return true;
}

bool mysql_conn_base::server_public_key_encrypt(const std::string &password, unsigned char *data, unsigned int length)
{
    send_data.clear();
    send_data = password;
    send_data.push_back(0x00);

    if (server_hello.auth_plugin_salt_data.size() < 20)
    {
        error_msg = "auth plugin salt too short";
        return false;
    }

    mysqlnd_xor_string(&send_data[0], send_data.size(), server_hello.auth_plugin_salt_data.data(), 20);

    BIO *bio             = BIO_new_mem_buf((const char *)data, length);
    EVP_PKEY *public_key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    if (public_key == NULL)
    {
        BIO_free(bio);
        error_msg = "server public key parse failed";
        return false;
    }
    BIO_free(bio);

    std::size_t server_public_key_len = 0;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    server_public_key_len = EVP_PKEY_get_size(public_key);
#else  /* OPENSSL_VERSION_NUMBER >= 0x30000000L */
    server_public_key_len = EVP_PKEY_size(public_key);
#endif /* OPENSSL_VERSION_NUMBER >= 0x30000000L */

    if (256 < CACHE_DATA_LENGTH)
    {
        std::memset(_cache_data, 0x00, 256);
    }
    else
    {
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    }

    // see sql-common/client_authenthication.cc line 144 or 968 php-src ext/mysqlnd/mysqlnd_auth.c
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(public_key, NULL);
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
        EVP_PKEY_encrypt(ctx, (unsigned char *)_cache_data, &server_public_key_len, (unsigned char *)&send_data[0], send_data.size()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(public_key);
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
    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);

    if (conn_config.issock)
    {
        // Unix domain socket connection
        try
        {
            conn_link->localsocket = std::make_unique<asio::local::stream_protocol::socket>(*conn_link->io_ctx);
            asio::local::stream_protocol::endpoint ep(conn_config.host);
            co_await conn_link->localsocket->async_connect(ep, asio::use_awaitable);
        }
        catch (const std::exception &e)
        {
            error_msg = "local socket connect error: ";
            error_msg.append(e.what());
            error_code = 1;
            isclose    = true;
            co_return false;
        }
        conn_link->sock_type = 3;
    }
    else
    {
        asio::ip::tcp::resolver resolver(*conn_link->io_ctx);
        auto endpoints = co_await resolver.async_resolve(conn_config.host, conn_config.port, asio::use_awaitable);

        for (auto iter = endpoints.cbegin(); iter != endpoints.cend();)
        {
            std::tie(conn_link->ec) = co_await conn_link->socket->async_connect(*iter, tuple_awaitable);
            if (conn_link->ec)
            {
                continue;
            }
            break;
        }
        // asio::ip::tcp::resolver::iterator iter = co_await resolver.async_resolve(conn_config.host, conn_config.port, asio::use_awaitable);
        // asio::ip::tcp::resolver::iterator end;
        // asio::ip::tcp::endpoint endpoint;
        // constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
        // while (iter != end)
        // {
        //     endpoint     = *iter++;
        //     std::tie(ec) = co_await socket->async_connect(endpoint, tuple_awaitable);
        //     if (ec)
        //     {
        //         continue;
        //     }
        //     else
        //     {
        //         break;
        //     }
        // }
        // asio::error_code ec;
        if (conn_link->sock_type != 3 && conn_link->ec)
        {
            error_msg.append("host connect error! ");
            error_msg.append(conn_link->ec.message());
            error_code = 1;
            isclose    = true;
            co_return false;
        }
    }// end else (TCP)
    std::size_t n = 0;
    try
    {
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        if (conn_link->sock_type == 3)
        {
            n = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
        else
        {
            n = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
    }
    catch (const std::exception &e)
    {
        error_msg.append(e.what());
        isclose = true;
        co_return false;
    }

    if (n < 70)
    {
        error_msg.append("mysql server back data error! ");
        error_code = 255;
        isclose    = true;
        co_return false;
    }

    read_server_hello(0, n);

    if (server_hello.auth_plugin_salt_data.size() < 20)
    {
        error_msg = "auth plugin salt too short";
        co_return false;
    }

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
    seq_next_id  = 0;
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

    send_data.push_back(static_cast<char>(0xFF));// max pack
    send_data.push_back(static_cast<char>(0xFF));
    send_data.push_back(static_cast<char>(0xFF));

    send_data.push_back(0x00);
    if (conn_config.charset_val > 0)
    {
        send_data.push_back(conn_config.charset_val);
    }
    else
    {
        send_data.push_back(0x2D);// charset utf8mb4_general_ci
    }

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
    if (conn_link->sock_type == 3)
    {
        // Unix socket: no SSL, keep sock_type=3
    }
    else
    {
        conn_link->sock_type = 1;
    }
    if (conn_config.isssl && server_enable_ssl && !conn_config.issock)
    {
        if (!conn_config.islocal)
        {
            //.sock please use localsocket
            send_data[0] = 32;
            client_flags = CLIENT_PZORM_SSL_FLAGS;
            send_data[4] = client_flags & 0xFF;
            send_data[5] = client_flags >> 8 & 0xFF;
            send_data[6] = client_flags >> 16 & 0xFF;
            send_data[7] = client_flags >> 24 & 0xFF;

            try
            {
                n = co_await asio::async_write(*conn_link->socket, asio::buffer(send_data.substr(0, 36)), asio::use_awaitable);
            }
            catch (const std::exception &e)
            {
                error_msg  = e.what();
                error_code = 1;
                isclose    = true;
                co_return false;
            }

            conn_link->ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
            asio::ssl::stream<asio::ip::tcp::socket> ssl_temp_socket(std::move(*conn_link->socket.release()), *conn_link->ssl_context);
            //std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket=std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));
            conn_link->sslsocket = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));

            if (conn_config.sslverify)
            {
                conn_link->ssl_context->set_default_verify_paths();
                conn_link->ssl_context->set_verify_mode(asio::ssl::verify_peer);
            }
            const std::string &sni_host = conn_config.sslhost.empty() ? conn_config.host : conn_config.sslhost;
            if (!sni_host.empty())
            {
                SSL_set_tlsext_host_name(conn_link->sslsocket->native_handle(), sni_host.c_str());
            }
            if (conn_config.sslverify && !conn_config.sslhost.empty())
            {
                conn_link->ssl_context->set_verify_callback(asio::ssl::host_name_verification(conn_config.sslhost));
            }

            conn_link->sslsocket->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

            std::tie(conn_link->ec) = co_await conn_link->sslsocket->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
            conn_link->sock_type    = 2;
            if (conn_link->ec)
            {
                error_msg  = conn_link->ec.message();
                error_code = 2;
                isclose    = true;
                co_return false;
            }

            send_data[0] = send_data.size() - 4;
            seq_next_id += 1;
            send_data[3] = seq_next_id;

            try
            {
                n = co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), asio::use_awaitable);
            }
            catch (const std::exception &e)
            {
                error_msg  = e.what();
                error_code = 1;
                isclose    = true;
                co_return false;
            }

            std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
            try
            {
                n = co_await conn_link->sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
            }
            catch (const std::exception &e)
            {
                error_msg.append(e.what());
                isclose = true;
                co_return false;
            }

            if (n < 5)
            {
                error_msg = "response too short";
                co_return false;
            }

            if (_cache_data[0] == 0x02 && _cache_data[4] == 0x01 && _cache_data[5] == 0x04)
            {
                send_data.clear();
                seq_next_id += 1;
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.push_back(0x00);
                send_data.push_back(seq_next_id);
                send_data.append(conn_config.password);
                send_data.push_back(0x00);
                send_data[0] = (send_data.size() & 0xFF) - 4;

                try
                {
                    n = co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), asio::use_awaitable);
                }
                catch (const std::exception &e)
                {
                    error_msg  = e.what();
                    error_code = 1;
                    isclose    = true;
                    co_return false;
                }

                std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
                try
                {
                    n = co_await conn_link->sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
                }
                catch (const std::exception &e)
                {
                    error_msg.append(e.what());
                    isclose = true;
                    co_return false;
                }
            }

            if (n < 5)
            {
                error_msg = "response too short";
                co_return false;
            }

            //feedback
            if ((unsigned char)_cache_data[4] == 0xFE)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 2;
                isclose    = true;
                co_return false;
            }
            else if ((unsigned char)_cache_data[4] == 0xFF)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 6;
                isclose    = true;
                co_return false;
            }
            else if ((unsigned char)_cache_data[5] == 0x03)
            {
                unsigned int pack_length = _cache_data[2];
                pack_length              = pack_length << 8 | (_cache_data[1]);
                pack_length              = pack_length << 8 | (_cache_data[0]);
                unsigned int offset      = pack_length + 4;
                if (n >= offset + 3)
                {
                    pack_length = _cache_data[offset + 2];
                    pack_length = pack_length << 8 | (_cache_data[offset + 1]);
                    pack_length = pack_length << 8 | (_cache_data[offset]);
                    if (pack_length < 5)
                    {
                        error_msg  = " connect fail! server status error! ";
                        error_code = 8;
                        isclose    = true;
                        co_return false;
                    }

                    if (_cache_data[offset + 4] == 0x00)
                    {
                        co_return true;
                    }
                    else
                    {
                        error_msg  = " connect fail! ";
                        error_code = 8;
                        isclose    = true;
                        co_return false;
                    }
                }
                else
                {
                    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
                    try
                    {
                        n = co_await conn_link->sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);// socket->read_some(asio::buffer(data), ec);
                    }
                    catch (const std::exception &e)
                    {
                        error_msg.append(e.what());
                        isclose = true;
                        co_return false;
                    }

                    if (n < 5)
                    {
                        error_msg = "response too short";
                        co_return false;
                    }

                    if ((unsigned char)_cache_data[4] == 0xFF)
                    {
                        for (unsigned int i = 5; i < n; i++)
                        {
                            error_msg.push_back(_cache_data[i]);
                        }
                        error_code = 7;
                        isclose    = true;
                        co_return false;
                    }
                    else if ((unsigned char)_cache_data[4] == 0x00)
                    {
                        co_return true;
                    }
                    else
                    {
                        error_msg  = " async read status connect fail! ";
                        error_code = 9;
                        isclose    = true;
                        co_return false;
                    }
                }
            }
            else
            {
                error_msg  = " connect fail! ";
                error_code = 8;
                isclose    = true;
                co_return false;
            }

            seq_next_id = 0;
            isclose     = true;
            co_return false;
        }
    }

    try
    {
        if (conn_link->sock_type == 3)
        {
            n = co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), asio::use_awaitable);
        }
        else
        {
            n = co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), asio::use_awaitable);
        }
    }
    catch (const std::exception &e)
    {
        error_msg.append(e.what());
        isclose = true;
        co_return false;
    }
    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    try
    {
        if (conn_link->sock_type == 3)
        {
            n = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
        else
        {
            n = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
    }
    catch (const std::exception &e)
    {
        error_msg.append(e.what());
        isclose = true;
        co_return false;
    }

    if (n < 5)
    {
        error_msg = "response too short";
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
            if (conn_link->sock_type == 3)
            {
                n = co_await asio::async_write(*conn_link->localsocket, asio::buffer(_cache_data, 5), asio::use_awaitable);
            }
            else
            {
                n = co_await asio::async_write(*conn_link->socket, asio::buffer(_cache_data, 5), asio::use_awaitable);
            }
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
            isclose = true;
            co_return false;
        }
        if (n == 0)
        {
            error_msg.append(" request server_public_key error ! ");
            error_code = 3;
            isclose    = true;
            co_return false;
        }

        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        try
        {
            if (conn_link->sock_type == 3)
            {
                n = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
            }
            else
            {
                n = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
            }
        }
        catch (const std::exception &e)
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
            isclose    = true;
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
            if (conn_link->sock_type == 3)
            {
                n = co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), asio::use_awaitable);
            }
            else
            {
                n = co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), asio::use_awaitable);
            }
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
            isclose = true;
            co_return false;
        }
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);

        try
        {
            if (conn_link->sock_type == 3)
            {
                n = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
            }
            else
            {
                n = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
            }
        }
        catch (const std::exception &e)
        {
            error_msg.append(e.what());
            isclose = true;
            co_return false;
        }

        if (n < 5)
        {
            error_msg = "response too short";
            co_return false;
        }

        if ((unsigned char)_cache_data[4] == 0xFF)
        {
            for (unsigned int i = 5; i < n; i++)
            {
                error_msg.push_back(_cache_data[i]);
            }
            error_code = 5;
            isclose    = true;
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
        isclose    = true;
        co_return false;
    }
    else if ((unsigned char)_cache_data[4] == 0xFF)
    {
        for (unsigned int i = 5; i < n; i++)
        {
            error_msg.push_back(_cache_data[i]);
        }
        error_code = 6;
        isclose    = true;
        co_return false;
    }
    else if ((unsigned char)_cache_data[5] == 0x03)
    {
        unsigned int pack_length = _cache_data[2];
        pack_length              = pack_length << 8 | (_cache_data[1]);
        pack_length              = pack_length << 8 | (_cache_data[0]);
        unsigned int offset      = pack_length + 4;
        if (n >= offset + 3)
        {
            pack_length = _cache_data[offset + 2];
            pack_length = pack_length << 8 | (_cache_data[offset + 1]);
            pack_length = pack_length << 8 | (_cache_data[offset]);
            if (pack_length < 5)
            {
                error_msg  = " connect fail! server status error! ";
                error_code = 8;
                isclose    = true;
                co_return false;
            }

            if (_cache_data[offset + 4] == 0x00)
            {
                co_return true;
            }
            else
            {
                error_msg  = " connect fail! ";
                error_code = 8;
                isclose    = true;
                co_return false;
            }
        }
        else
        {
            std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
            try
            {
                if (conn_link->sock_type == 3)
                {
                    n = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
                }
                else
                {
                    n = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
                }
            }
            catch (const std::exception &e)
            {
                error_msg.append(e.what());
                isclose = true;
                co_return false;
            }

            if (n < 5)
            {
                error_msg = "response too short";
                co_return false;
            }

            if ((unsigned char)_cache_data[4] == 0xFF)
            {
                for (unsigned int i = 5; i < n; i++)
                {
                    error_msg.push_back(_cache_data[i]);
                }
                error_code = 7;
                isclose    = true;
                co_return false;
            }
            else if ((unsigned char)_cache_data[4] == 0x00)
            {
                co_return true;
            }
            else
            {
                error_msg  = " async read status connect fail! ";
                error_code = 9;
                isclose    = true;
                co_return false;
            }
        }
    }
    else
    {
        error_msg  = " connect fail! ";
        error_code = 8;
        isclose    = true;
        co_return false;
    }

    co_return true;
}
asio::awaitable<unsigned int> mysql_conn_base::async_read_loop()
{
    try
    {
        if (isclose)
        {
            co_return 0;
        }
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        std::size_t n = 0;
        if (conn_link->sock_type == 1)
        {
            n = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
        else if (conn_link->sock_type == 2)
        {
            n = co_await conn_link->sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
        else if (conn_link->sock_type == 3)
        {
            n = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), asio::use_awaitable);
        }
        if (n == 0)
        {
            error_code = 1;
            co_return 0;
        }
        if (n < 5)
        {
            error_msg = "response too short";
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
        if (isclose)
        {
            return 0;
        }
        std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
        std::size_t n = 0;
        if (conn_link->sock_type == 1)
        {
            n = conn_link->socket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
        }
        else if (conn_link->sock_type == 2)
        {
            n = conn_link->sslsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
        }
        else if (conn_link->sock_type == 3)
        {
            n = conn_link->localsocket->read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), conn_link->ec);
        }

        if (conn_link->ec)
        {
            error_code = 1;
            error_msg  = conn_link->ec.message();
            return 0;
        }
        if (n < 5)
        {
            error_msg = "response too short";
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
    unsigned int n = 0;

    n = sql.length() + 1;

    send_data.clear();
    send_data.push_back((n & 0xFF));
    send_data.push_back((n >> 8 & 0xFF));
    send_data.push_back((n >> 16 & 0xFF));
    send_data.push_back(0x00);
    send_data.push_back(0x03);
    send_data.append(sql);

    n = 0;

    if (isclose)
    {
        return 0;
    }
    try
    {

        if (conn_link->sock_type == 1)
        {
            n = asio::write(*conn_link->socket, asio::buffer(send_data), conn_link->ec);
        }
        else if (conn_link->sock_type == 2)
        {
            n = asio::write(*conn_link->sslsocket, asio::buffer(send_data), conn_link->ec);
        }
        else if (conn_link->sock_type == 3)
        {
            n = asio::write(*conn_link->localsocket, asio::buffer(send_data), conn_link->ec);
        }
        if (conn_link->ec)
        {
            error_code = 20;
            error_msg  = conn_link->ec.message();
            return 0;
        }
    }
    catch (const std::exception &e)
    {
        error_code = 20;
        error_msg  = conn_link->ec.message();
        return 0;
    }
    query_num++;
    return n;
}

unsigned int mysql_conn_base::write()
{
    unsigned int n = 0;
    if (isclose)
    {
        return 0;
    }
    try
    {
        if (conn_link->sock_type == 1)
        {
            n = asio::write(*conn_link->socket, asio::buffer(send_data), conn_link->ec);
        }
        else if (conn_link->sock_type == 2)
        {
            n = asio::write(*conn_link->sslsocket, asio::buffer(send_data), conn_link->ec);
        }
        else if (conn_link->sock_type == 3)
        {
            n = asio::write(*conn_link->localsocket, asio::buffer(send_data), conn_link->ec);
        }
        if (conn_link->ec)
        {
            error_code = 20;
            error_msg  = conn_link->ec.message();
            return 0;
        }
    }
    catch (const std::exception &e)
    {
        error_code = 20;
        error_msg  = conn_link->ec.message();
        return 0;
    }
    query_num++;
    return n;
}

asio::awaitable<unsigned int> mysql_conn_base::async_write_sql(const std::string &sql)
{
    unsigned int n = 0;

    n = sql.length() + 1;

    send_data.clear();
    send_data.push_back((n & 0xFF));
    send_data.push_back((n >> 8 & 0xFF));
    send_data.push_back((n >> 16 & 0xFF));
    send_data.push_back(0x00);
    send_data.push_back(0x03);
    send_data.append(sql);

    n = 0;
    try
    {
        if (isclose)
        {
            co_return 0;
        }

        if (conn_link->sock_type == 1)
        {
            n = co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), asio::use_awaitable);
        }
        else if (conn_link->sock_type == 2)
        {
            n = co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), asio::use_awaitable);
        }
        else if (conn_link->sock_type == 3)
        {
            n = co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), asio::use_awaitable);
        }
    }
    catch (const std::exception &e)
    {
        error_code = 20;
        error_msg  = conn_link->ec.message();
        co_return 0;
    }
    query_num++;
    co_return n;
}

asio::awaitable<unsigned int> mysql_conn_base::async_write()
{
    unsigned int n = 0;
    try
    {
        if (isclose)
        {
            co_return 0;
        }

        if (conn_link->sock_type == 1)
        {
            n = co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), asio::use_awaitable);
        }
        else if (conn_link->sock_type == 2)
        {
            n = co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), asio::use_awaitable);
        }
        else if (conn_link->sock_type == 3)
        {
            n = co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), asio::use_awaitable);
        }
    }
    catch (const std::exception &e)
    {
        error_code = 20;
        error_msg  = conn_link->ec.message();
        co_return 0;
    }
    query_num++;
    co_return n;
}

bool mysql_conn_base::ping()
{
    char data_send[16] = {0x01, 0x00, 0x00, 0x00, 0x0E, 0x00};
    error_code         = 0;
    error_msg.clear();
    try
    {
        if (isclose)
        {
            return false;
        }

        if (conn_link->sock_type == 1)
        {
            asio::write(*conn_link->socket, asio::buffer(data_send, 5), conn_link->ec);
            conn_link->socket->read_some(asio::buffer(data_send, 16), conn_link->ec);
        }
        else if (conn_link->sock_type == 2)
        {
            asio::write(*conn_link->sslsocket, asio::buffer(data_send, 5), conn_link->ec);
            conn_link->sslsocket->read_some(asio::buffer(data_send, 16), conn_link->ec);
        }
        else if (conn_link->sock_type == 3)
        {
            asio::write(*conn_link->localsocket, asio::buffer(data_send, 5), conn_link->ec);
            conn_link->localsocket->read_some(asio::buffer(data_send, 16), conn_link->ec);
        }

        return true;
    }
    catch (const std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return false;
    }
}
bool mysql_conn_base::close()
{
    if (isclose)
    {
        return false;
    }
    char data_send[6] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
    error_code        = 0;
    error_msg.clear();
    try
    {
        isclose = true;
        if (conn_link->sock_type == 1)
        {
            if (conn_link->socket->is_open())
            {
                asio::write(*conn_link->socket, asio::buffer(data_send, 5), conn_link->ec);
                conn_link->socket->close(conn_link->ec);
            }
        }
        else if (conn_link->sock_type == 2)
        {
            asio::write(*conn_link->sslsocket, asio::buffer(data_send, 5), conn_link->ec);
            if (conn_link->sslsocket->lowest_layer().is_open())
            {
                conn_link->sslsocket->lowest_layer().cancel(conn_link->ec);
                conn_link->sslsocket->lowest_layer().close(conn_link->ec);
            }
        }
        else if (conn_link->sock_type == 3)
        {
            if (conn_link->localsocket && conn_link->localsocket->is_open())
            {
                asio::write(*conn_link->localsocket, asio::buffer(data_send, 5), conn_link->ec);
                conn_link->localsocket->close(conn_link->ec);
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return false;
    }
}
asio::awaitable<bool> mysql_conn_base::async_close()
{
    if (isclose)
    {
        co_return false;
    }
    char data_send[6] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
    error_code        = 0;
    error_msg.clear();
    try
    {
        isclose = true;
        if (conn_link->sock_type == 1)
        {
            if (conn_link->socket->is_open())
            {
                co_await asio::async_write(*conn_link->socket, asio::buffer(data_send, 5), asio::use_awaitable);
                conn_link->socket->close();
            }
        }
        else if (conn_link->sock_type == 2)
        {
            co_await asio::async_write(*conn_link->sslsocket, asio::buffer(data_send, 5), asio::use_awaitable);
            if (conn_link->sslsocket->lowest_layer().is_open())
            {
                conn_link->sslsocket->lowest_layer().cancel(conn_link->ec);
                conn_link->sslsocket->lowest_layer().close(conn_link->ec);
            }
        }
        else if (conn_link->sock_type == 3)
        {
            if (conn_link->localsocket && conn_link->localsocket->is_open())
            {
                co_await asio::async_write(*conn_link->localsocket, asio::buffer(data_send, 5), asio::use_awaitable);
                conn_link->localsocket->close();
            }
        }

        co_return true;
    }
    catch (const std::exception &e)
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
        if (conn_link->sock_type == 1)
        {
            if (conn_link->socket->is_open())
            {
                conn_link->socket->close(conn_link->ec);
            }
        }
        else if (conn_link->sock_type == 2)
        {
            if (conn_link->sslsocket->lowest_layer().is_open())
            {
                conn_link->sslsocket->lowest_layer().cancel(conn_link->ec);
                conn_link->sslsocket->lowest_layer().close(conn_link->ec);
            }
        }
        else if (conn_link->sock_type == 3)
        {
            if (conn_link->localsocket && conn_link->localsocket->is_open())
            {
                conn_link->localsocket->close(conn_link->ec);
            }
        }
    }
    catch (const std::exception &e)
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
        if (conn_link->sock_type == 1)
        {
            if (conn_link->socket)
            {
                return !conn_link->socket->is_open();
            }
            return true;
        }
        else if (conn_link->sock_type == 2)
        {
            if (conn_link->sslsocket)
            {
                return !conn_link->sslsocket->lowest_layer().is_open();
            }
            return true;
        }
        else if (conn_link->sock_type == 3)
        {
            if (conn_link->localsocket)
            {
                return !conn_link->localsocket->is_open();
            }
            return true;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return true;
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
        if (offset + 4 >= total_num)
        {
            // 保留已收集的包头残片继续追加，避免连续短批次时重置丢失数据
            for (; offset < total_num && pack_info.padd_length < 4; offset++)
            {
                pack_info.padd_str[pack_info.padd_length] = data[offset];
                pack_info.padd_length += 1;//not enough pack
            }
            return;
        }
        unsigned char seq_id = 0;
        if (pack_info.padd_length > 0)
        {

            for (; pack_info.padd_length < 4; pack_info.padd_length++)
            {
                pack_info.padd_str[pack_info.padd_length] = data[offset];
                offset += 1;//padding 4 char
            }

            pack_length           = (pack_info.padd_str[seq_id + 2] & 0xFF);
            pack_length           = pack_length << 8 | (pack_info.padd_str[seq_id + 1] & 0xFF);
            pack_length           = pack_length << 8 | (pack_info.padd_str[seq_id] & 0xFF);
            seq_id                = pack_info.padd_str[seq_id + 3];
            pack_info.padd_length = 0;
            begin_length          = offset;
        }
        else
        {
            pack_length = (data[offset + 2] & 0xFF);
            pack_length = pack_length << 8 | (data[offset + 1] & 0xFF);
            pack_length = pack_length << 8 | (data[offset] & 0xFF);
            offset      = offset + 3;
            seq_id      = data[offset];
            offset++;
            begin_length += 4;
        }

        if (seq_id != pack_info.seq_id)
        {
            pack_info.error = 1;
            pack_info.data  = "mysql read package seq_id error !";
            return;
        }
        pack_info.seq_id = (pack_info.seq_id + 1) % 256;
        offset           = offset + pack_length;

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

unsigned long long mysql_conn_base::pack_real_num(const unsigned char *data, unsigned int data_len, unsigned int &offset)
{
    unsigned long long pack_length = 0;

    if (offset >= data_len)
    {
        error_msg  = "pack_real_num: offset out of bounds";
        error_code = 10;
        return 0;
    }

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
        if (offset + 2 > data_len)
        {
            error_msg  = "pack_real_num: insufficient data for 2-byte length";
            error_code = 10;
            return 0;
        }
        pack_length = (data[offset + 1] & 0xFF);
        pack_length = pack_length << 8 | (data[offset] & 0xFF);

        offset += 2;
    }
    else if (pack_length == 253)
    {
        offset++;
        if (offset + 3 > data_len)
        {
            error_msg  = "pack_real_num: insufficient data for 3-byte length";
            error_code = 10;
            return 0;
        }
        pack_length = (data[offset + 2] & 0xFF);
        pack_length = pack_length << 8 | (data[offset + 1] & 0xFF);
        pack_length = pack_length << 8 | (data[offset] & 0xFF);

        offset += 3;
    }
    else if (pack_length == 254)
    {
        offset++;
        if (offset + 8 > data_len)
        {
            error_msg  = "pack_real_num: insufficient data for 8-byte length";
            error_code = 10;
            return 0;
        }
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
    // P3 修复：校验 [pos, pos+len) 区间在 pack_info 内（无整数溢出）
    auto need = [&](unsigned int pos, unsigned int len) -> bool
    {
        return pos <= (unsigned int)pack_info.size() &&
               len <= (unsigned int)pack_info.size() - pos;
    };
    auto truncated = [&]() -> void
    {
        error_code = 10;
        error_msg  = "read pakage error field info truncated";
    };

    unsigned int tempnum     = 0;
    unsigned int name_length = pack_info[tempnum];
    if (name_length == 3)
    {
        if (need(tempnum + 1, 3) &&
            pack_info[tempnum + 1] == 0x64 && pack_info[tempnum + 2] == 0x65 && pack_info[tempnum + 3] == 0x66)
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
    if (!need(tempnum, 1))
    {
        truncated();
        return;
    }
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        if (!need(tempnum + 1, name_length))
        {
            truncated();
            return;
        }
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.dbname.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    if (!need(tempnum, 1))
    {
        truncated();
        return;
    }
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        if (!need(tempnum + 1, name_length))
        {
            truncated();
            return;
        }
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.table.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    if (!need(tempnum, 1))
    {
        truncated();
        return;
    }
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        if (!need(tempnum + 1, name_length))
        {
            truncated();
            return;
        }
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.org_table.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    if (!need(tempnum, 1))
    {
        truncated();
        return;
    }
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        if (!need(tempnum + 1, name_length))
        {
            truncated();
            return;
        }
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.name.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    if (!need(tempnum, 1))
    {
        truncated();
        return;
    }
    name_length = pack_info[tempnum];
    if (name_length > 0)
    {
        if (!need(tempnum + 1, name_length))
        {
            truncated();
            return;
        }
        for (unsigned int iin = 0; iin < name_length; iin++)
        {
            tempnum++;
            temp_field_data.org_name.push_back(pack_info[tempnum]);
        }
    }
    tempnum++;
    if (!need(tempnum, 1))
    {
        truncated();
        return;
    }
    name_length = pack_info[tempnum];

    if (name_length != 0x0C)
    {
        error_code = 10;
        error_msg  = "read pakage error field is not 0x0C fixed! ";
        return;
    }
    tempnum++;

    if (!need(tempnum, 2))
    {
        truncated();
        return;
    }
    temp_field_data.charset = pack_info[tempnum + 1];
    temp_field_data.charset = temp_field_data.charset << 8 | (pack_info[tempnum] & 0xFF);
    tempnum += 2;

    if (!need(tempnum, 4))
    {
        truncated();
        return;
    }
    temp_field_data.column_length = pack_info[tempnum + 3];
    temp_field_data.column_length = temp_field_data.column_length << 8 | (pack_info[tempnum + 2] & 0xFF);
    temp_field_data.column_length = temp_field_data.column_length << 8 | (pack_info[tempnum + 1] & 0xFF);
    temp_field_data.column_length = temp_field_data.column_length << 8 | (pack_info[tempnum] & 0xFF);

    tempnum += 4;

    if (!need(tempnum, 1))
    {
        truncated();
        return;
    }
    temp_field_data.field_type = pack_info[tempnum];
    tempnum++;
    if (!need(tempnum, 2))
    {
        truncated();
        return;
    }
    temp_field_data.flags = pack_info[tempnum + 1];
    temp_field_data.flags = temp_field_data.flags << 8 | (pack_info[tempnum] & 0xFF);

    tempnum += 2;
    if (!need(tempnum, 1))
    {
        truncated();
        return;
    }
    temp_field_data.decimals = pack_info[tempnum];
}
bool mysql_conn_base::pack_eof_check(const pack_info_t &temp_pack_data)
{
    if (temp_pack_data.data.size() == 0)
    {
        return true;
    }

    if ((unsigned char)temp_pack_data.data[0] == 0xFF)
    {
        if (temp_pack_data.data.size() > 3)
        {
            error_msg = temp_pack_data.data.substr(3);
        }
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
