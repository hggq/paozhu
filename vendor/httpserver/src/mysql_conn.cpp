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

    // NOTE: 此函数为遗留/死代码（未被调用），缺少 total_num 参数无法做边界检查。
    // 如需启用，请改用带长度参数的 read_field_pack。
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

    // 重连场景：彻底重置连接层状态
    conn_link->sock_type  = 0;
    conn_link->sslsocket.reset();
    conn_link->ssl_context.reset();
    conn_link->localsocket.reset();
    conn_link->socket     = std::make_unique<asio::ip::tcp::socket>(*conn_link->io_ctx);
    conn_link->ec.clear();
    server_enable_ssl     = false;
    seq_next_id           = 0;
    std::memset(static_cast<void *>(&server_hello), 0, sizeof(server_hello));

    // 保存配置用于断线自动重连（仅在首次或配置变化时存储）
    if (!has_conn_config_ || !(last_conn_config_ == conn_config)) {
        last_conn_config_ = conn_config;
        has_conn_config_ = true;
    }

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

    // 握手头部没有大于4K
    auto payload_len = send_data.size() - 4;
    if (payload_len > 4096)
    {
        error_msg  = "handshake response payload too large: " + std::to_string(payload_len);
        error_code = 1;
        isclose    = true;
        return false;
    }
    send_data[0] = (payload_len)       & 0xFF;
    send_data[1] = (payload_len >>  8) & 0xFF;
    send_data[2] = (payload_len >> 16) & 0xFF;
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
            if (conn_link->ec)
            {
                error_msg  = conn_link->ec.message();
                error_code = 2;
                isclose    = true;
                return false;
            }
            conn_link->sock_type = 2;// 握手成功后才标记 SSL 已建立

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
                send_data.push_back(seq_next_id);
                send_data.append(conn_config.password);
                send_data.push_back(0x00);
                auto pl2 = send_data.size() - 4;
                if (pl2 > 0xFFFFFF)
                {
                    error_msg  = "auth packet payload too large: " + std::to_string(pl2);
                    error_code = 1;
                    isclose    = true;
                    return false;
                }
                send_data[0] = (pl2)       & 0xFF;
                send_data[1] = (pl2 >>  8) & 0xFF;
                send_data[2] = (pl2 >> 16) & 0xFF;
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
        if (n < 5 || n > CACHE_DATA_LENGTH)
        {
            error_msg.append(" get server_public_key return size: ");
            error_msg.append(std::to_string(n));
            error_code = 2;
            isclose    = true;
            return false;
        }

        // 先检查 MySQL 返回的是不是 Error/EOF/OK 包（非公钥）
        unsigned char pkt_type = (unsigned char)_cache_data[4];
        if (pkt_type == 0xFF && n >= 7)
        {
            // Error packet: [0xFF][ec_lo][ec_hi][#][state5][msg...]
            unsigned int ec = (unsigned char)_cache_data[5] | ((unsigned char)_cache_data[6] << 8);
            error_code = ec;
            // 跳过 4 字节 pkt header + 1 byte 0xFF + 2 byte ec + 1 byte '#' + 5 byte sql_state
            unsigned int msg_off = 4 + 1 + 2 + 1 + 5;
            error_msg = "MySQL auth error: ";
            if (n > msg_off) error_msg.append(reinterpret_cast<const char *>(_cache_data + msg_off), n - msg_off);
            isclose = true;
            return false;
        }
        if (pkt_type == 0xFE || pkt_type == 0x00)
        {
            // EOF / OK — auth 完成但 server 不要求 RSA，直接继续
            seq_next_id = (_cache_data[3] & 0xFF) + 1;
            return true;
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

    // 重连场景：彻底重置连接层状态
    conn_link->sock_type  = 0;
    conn_link->sslsocket.reset();
    conn_link->ssl_context.reset();
    conn_link->localsocket.reset();
    conn_link->socket     = std::make_unique<asio::ip::tcp::socket>(*conn_link->io_ctx);
    conn_link->ec.clear();
    server_enable_ssl     = false;
    seq_next_id           = 0;
    std::memset(static_cast<void *>(&server_hello), 0, sizeof(server_hello));

    // 保存配置用于断线自动重连（仅在首次或配置变化时存储）
    if (!has_conn_config_ || !(last_conn_config_ == conn_config)) {
        last_conn_config_ = conn_config;
        has_conn_config_ = true;
    }

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

    // 握手没有大于4K
    auto payload_len = send_data.size() - 4;
    if (payload_len > 4096)
    {
        error_msg  = "handshake response payload too large: " + std::to_string(payload_len);
        error_code = 1;
        isclose    = true;
        co_return false;
    }
    send_data[0] = (payload_len)       & 0xFF;
    send_data[1] = (payload_len >>  8) & 0xFF;
    send_data[2] = (payload_len >> 16) & 0xFF;
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
            if (conn_link->ec)
            {
                error_msg  = conn_link->ec.message();
                error_code = 2;
                isclose    = true;
                co_return false;
            }
            conn_link->sock_type = 2;// 握手成功后才标记 SSL 已建立

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
                auto pl4 = send_data.size() - 4;
                if (pl4 > 0xFFFFFF)
                {
                    error_msg  = "auth packet payload too large: " + std::to_string(pl4);
                    error_code = 1;
                    isclose    = true;
                    co_return false;
                }
                send_data[0] = (pl4)       & 0xFF;
                send_data[1] = (pl4 >>  8) & 0xFF;
                send_data[2] = (pl4 >> 16) & 0xFF;

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
        if (n < 5 || n > CACHE_DATA_LENGTH)
        {
            error_msg.append(" get server_public_key return size: ");
            error_msg.append(std::to_string(n));
            error_code = 2;
            isclose    = true;
            co_return false;
        }

        // 先检查 MySQL 返回的是不是 Error/EOF/OK 包（非公钥）
        unsigned char pkt_type = (unsigned char)_cache_data[4];
        if (pkt_type == 0xFF && n >= 7)
        {
            unsigned int ec = (unsigned char)_cache_data[5] | ((unsigned char)_cache_data[6] << 8);
            error_code = ec;
            unsigned int msg_off = 4 + 1 + 2 + 1 + 5;
            error_msg = "MySQL auth error: ";
            if (n > msg_off) error_msg.append(reinterpret_cast<const char *>(_cache_data + msg_off), n - msg_off);
            isclose = true;
            co_return false;
        }
        if (pkt_type == 0xFE || pkt_type == 0x00)
        {
            seq_next_id = (_cache_data[3] & 0xFF) + 1;
            co_return true;
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
    if (isclose)
    {
        co_return 0;
    }
    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    conn_link->ec.clear();

    std::size_t n = 0;
    if (conn_link->sock_type == 1)
    {
        std::tie(conn_link->ec, n) = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), tuple_awaitable);
    }
    else if (conn_link->sock_type == 2)
    {
        std::tie(conn_link->ec, n) = co_await conn_link->sslsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), tuple_awaitable);
    }
    else if (conn_link->sock_type == 3)
    {
        std::tie(conn_link->ec, n) = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data, CACHE_DATA_LENGTH), tuple_awaitable);
    }

    if (conn_link->ec)
    {
        error_code = 1;
        error_msg  = conn_link->ec.message();
        co_return 0;
    }
    if (n == 0)
    {
        error_code = 1;
        error_msg  = "connection closed by peer";
        co_return 0;
    }
    if (n < 5)
    {
        error_msg = "response too short";
        co_return 0;
    }
    co_return n;
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
    if (isclose)
    {
        co_return 0;
    }

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    conn_link->ec.clear();

    if (conn_link->sock_type == 1)
    {
        std::tie(conn_link->ec, n) = co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), tuple_awaitable);
    }
    else if (conn_link->sock_type == 2)
    {
        std::tie(conn_link->ec, n) = co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), tuple_awaitable);
    }
    else if (conn_link->sock_type == 3)
    {
        std::tie(conn_link->ec, n) = co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), tuple_awaitable);
    }

    if (conn_link->ec)
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
    if (isclose)
    {
        co_return 0;
    }

    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    conn_link->ec.clear();

    if (conn_link->sock_type == 1)
    {
        std::tie(conn_link->ec, n) = co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), tuple_awaitable);
    }
    else if (conn_link->sock_type == 2)
    {
        std::tie(conn_link->ec, n) = co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), tuple_awaitable);
    }
    else if (conn_link->sock_type == 3)
    {
        std::tie(conn_link->ec, n) = co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), tuple_awaitable);
    }

    if (conn_link->ec)
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
    // MySQL COM_PING OK 响应完整 = 4 字节包头 + 7 字节负载 = 11 字节
    char send_buf[5]    = {0x01, 0x00, 0x00, 0x00, 0x0E};
    char recv_buf[16]   = {0};
    error_code         = 0;
    error_msg.clear();
    try
    {
        if (isclose)
        {
            return false;
        }

        auto *sock = (conn_link->sock_type == 1) ? static_cast<void *>(conn_link->socket.get())
                   : (conn_link->sock_type == 2) ? static_cast<void *>(conn_link->sslsocket.get())
                   : (conn_link->sock_type == 3) ? static_cast<void *>(conn_link->localsocket.get())
                   : nullptr;
        if (!sock)
        {
            error_code = 3;
            error_msg  = "ping: no socket";
            return false;
        }

        auto do_write = [&](auto &stream) {
            asio::write(stream, asio::buffer(send_buf, 5), conn_link->ec);
            return !conn_link->ec;
        };
        auto do_read_full = [&](auto &stream) {
            // 先读 4 字节包头拿 payload 长度，再读完整 payload，避免半包截断
            char header[4] = {0};
            size_t n = asio::read(stream, asio::buffer(header, 4), conn_link->ec);
            if (conn_link->ec || n < 4) return false;
            size_t payload_len = (static_cast<unsigned char>(header[0])      ) |
                                 (static_cast<unsigned char>(header[1]) <<  8) |
                                 (static_cast<unsigned char>(header[2]) << 16);
            if (payload_len > sizeof(recv_buf)) return false;
            if (payload_len > 0) {
                n = asio::read(stream, asio::buffer(recv_buf, payload_len), conn_link->ec);
                if (conn_link->ec || n < payload_len) return false;
            }
            // OK 判定：payload 至少 1 字节且首字节为 0x00；ERR 为 0xFF
            if (payload_len == 0) return false;
            unsigned char status = static_cast<unsigned char>(recv_buf[0]);
            if (status == 0xFF && payload_len >= 2) {
                error_code = static_cast<unsigned char>(recv_buf[1]) | (static_cast<unsigned char>(recv_buf[2]) << 8);
                isclose    = true;
                return false;
            }
            return true;
        };

        bool ok = false;
        if (conn_link->sock_type == 1) {
            ok = do_write(*conn_link->socket);
            if (ok) ok = do_read_full(*conn_link->socket);
        } else if (conn_link->sock_type == 2) {
            ok = do_write(*conn_link->sslsocket);
            if (ok) ok = do_read_full(*conn_link->sslsocket);
        } else {
            ok = do_write(*conn_link->localsocket);
            if (ok) ok = do_read_full(*conn_link->localsocket);
        }

        if (!ok) {
            error_code = 3;
            error_msg  = "ping failed: " + conn_link->ec.message();
            isclose    = true;
            return false;
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
    // 校验 [pos, pos+len) 区间在 pack_info 内（无整数溢出）
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
    if ((unsigned char)temp_pack_data.data[0] == 0xFE && temp_pack_data.data.size() >= 2)
    {
        // DEPRECATE_EOF 终结包首字节 0xFE；必须 size>=2 才能与真实行数据区分
        // （0xFE 作为 1 字节列值时已废弃，但 0x00 是合法空字符串首字节）
        return true;
    }
    // 注意：0x00 是合法数据（首列为空字符串的行包），不能当终结符 —— 移除原 0x00 分支
    return false;
}

// fetch_directly / exec_dml 实现 减少中间变量

void mysql_conn_base::parse_error_packet(const unsigned char *data, unsigned int pkt_len)
{
    // MySQL Error Packet layout (after 4-byte packet header):
    //   [4]      0xFF              error marker (caller verified)
    //   [5..6]   error_code        2 bytes little-endian
    //   [7]      '#' (0x23)        SQL state marker (always present in MySQL 4.1+)
    //   [8..12]  SQL state         5 bytes (e.g. "HY000")
    //   [13..]   error message    rest of packet
    //
    // data 指向包起始位置, pkt_len 为完整包长度 (含 4 字节包头)。

    constexpr unsigned int kPktHeader     = 4;   // 3-byte length + 1-byte seq_id
    constexpr unsigned int kErrorMinTotal = 7;   // header + status(0xFF) + errcode(2)

    if (pkt_len < kErrorMinTotal)
    {
        error_msg  = "MySQL error packet truncated (< 7 bytes)";
        error_code = 255;
        return;
    }

    // 从 error_code 字段开始解析 (跳过 packet header + status byte)
    unsigned int offset = kPktHeader + 1;  // = 5

    // ---- error_code (2 bytes) ----
    error_code = data[offset] | (data[offset + 1] << 8);
    offset += 2;  // offset = 7

    // ---- SQL state: '#' + 5 bytes state ----
    if (offset < pkt_len && data[offset] == 0x23)
    {
        if (offset + 6 <= pkt_len)
        {
            offset += 6;  // skip '#'(1) + SQL state(5)
        }
        else
        {
            // SQL state 被截断: data[7] 是 '#' 但后续 5 字节不全
            offset = pkt_len;
        }
    }
    // 若 data[offset] != 0x23: 可能是 MySQL 4.0 旧协议, 无 SQL state,
    // 直接把剩余内容当 error message 处理

    // ---- error message ----
    if (offset < pkt_len)
    {
        error_msg = std::string(reinterpret_cast<const char *>(data + offset), pkt_len - offset);
    }
    else
    {
        error_msg = "MySQL error packet received (no message)";
    }
}

unsigned int mysql_conn_base::parse_affected_rows_fast(const unsigned char *data, size_t data_len, unsigned int *insert_id)
{
    if (data_len < 9)
    {
        error_msg  = "parse_affected_rows_fast: response too short";
        error_code = 10;
        return (unsigned int)-1;
    }

    unsigned int packet_len = data[0] | (data[1] << 8) | (data[2] << 16);
    if (packet_len < 4 || data_len < packet_len + 4)
    {
        error_msg  = "parse_affected_rows_fast: incomplete packet";
        error_code = 10;
        return (unsigned int)-1;
    }

    unsigned char status = data[4];
    if (status == 0xFF)
    {
        parse_error_packet(data, packet_len + 4);
        return (unsigned int)-1;
    }

    if (status != 0x00)
    {
        return (unsigned int)-1;
    }

    unsigned int offset     = 5;
    unsigned long long rows = pack_real_num(data, packet_len + 4, offset);
    if (rows > 0xFFFFFFFFULL)
    {
        return (unsigned int)-1;
    }

    if (insert_id)
    {
        unsigned long long iid = pack_real_num(data, packet_len + 4, offset);
        if (iid <= 0xFFFFFFFFULL)
        {
            *insert_id = static_cast<unsigned int>(iid);
        }
    }

    return static_cast<unsigned int>(rows);
}

unsigned int mysql_conn_base::fetch_directly_impl(
    const std::string &sql,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    if (isclose)
    {
        return 0;
    }

    error_msg.clear();
    error_code  = 0;
    last_insert_id_ = 0;

    unsigned int n = write_sql(sql);
    if (n == 0)
    {
        return 0;
    }

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;
    unsigned int consumed = 0;

    std::vector<std::string> col_names;
    col_names.reserve(32);
    unsigned int expected_cols = 0;
    std::vector<char *> name_ptrs_cache;

    enum class phase
    {
        INIT,
        COLUMNS,
        ROWS,
        DONE
    };
    phase current_phase = phase::INIT;
    bool stop           = false;
    unsigned int rows   = 0;
    bool handler_aborted = false;

    while (!stop)
    {
        n = read_loop();
        if (n == 0)
        {
            if (!error_msg.empty())
                return rows;
            return rows;
        }

        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

        while (consumed < accum_buf.size())
        {
            if (consumed + 4 > accum_buf.size())
                break;

            unsigned int pkt_len = accum_buf[consumed] |
                                   (accum_buf[consumed + 1] << 8) |
                                   (accum_buf[consumed + 2] << 16);
            if (pkt_len == 0)
            {
                consumed += 4;
                continue;
            }

            unsigned int total_pkt_len = 4 + pkt_len;
            if (consumed + total_pkt_len > accum_buf.size())
                break;

            unsigned char status = accum_buf[consumed + 4];

            switch (current_phase)
            {
            case phase::INIT:
            {
                if (status == 0xFF)
                {
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    return rows;
                }
                if (status == 0x00)
                {
                    // INIT 阶段收到 OK 包（DML/INSERT 无结果集），直接 DONE
                    // 原逻辑错误地置 COLUMNS 继续等列，且 pkt_len<8 时无 else 分支 → 永挂
                    unsigned int iid = 0;
                    unsigned int ar = parse_affected_rows_fast(&accum_buf[consumed], total_pkt_len, &iid);
                    if (ar != (unsigned int)-1)
                    {
                        last_insert_id_ = iid;
                    }
                    current_phase = phase::DONE;
                    stop = true;
                    consumed += total_pkt_len;
                    break;
                }
                if (status >= 1 && status <= 250 && pkt_len <= 9)
                {
                    // 列数包：经典格式为 1 字节；容忍 lenenc 变体（如服务端带
                    // metadata_follows 后缀的 2 字节形式），按 lenenc 取列数。
                    unsigned int cc_off = consumed + 4;
                    unsigned long long cc = pack_real_num(accum_buf.data(), accum_buf.size(), cc_off);
                    if (cc <= 0xFFFFFFFFULL)
                    {
                        expected_cols = static_cast<unsigned int>(cc);
                    }
                    current_phase = phase::COLUMNS;
                }
                consumed += total_pkt_len;
                break;
            }

            case phase::COLUMNS:
            {
                if (status == 0xFF)
                {
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    return rows;
                }
                if (status == 0xFE)
                {
                    // 0xFE 标记的 OK 包是结果集唯一终结符（可短至 7 字节）。
                    // 不支持经典 EOF 旧协议，无其他兜底。
                    current_phase   = phase::DONE;
                    stop            = true;
                    consumed += total_pkt_len;
                    break;
                }

                // 列定义已收齐而本包不是终结包：转入 ROWS 且不消费，
                // 让本包在 ROWS 阶段重新判定（兼作列数包解析失败时的兜底，
                // 防止把行包当列定义误吃）。
                if (expected_cols > 0 && col_names.size() >= expected_cols)
                {
                    current_phase   = phase::ROWS;
                    break;
                }

                unsigned int col_offset = consumed + 4;
                unsigned int col_end    = consumed + total_pkt_len;

                auto skip_len_str = [&](unsigned int &pos) {
                    if (pos >= col_end)
                        return;
                    unsigned char len = accum_buf[pos];
                    if (pos + 1 + len > col_end)
                    {
                        pos = col_end;  // truncated, advance to end
                        return;
                    }
                    pos += 1 + len;
                };
                auto read_len_str = [&](unsigned int &pos) -> std::string {
                    if (pos >= col_end)
                        return {};
                    unsigned char len = accum_buf[pos];
                    pos++;
                    if (len == 0 || pos + len > col_end)
                        return {};
                    return std::string(reinterpret_cast<char *>(&accum_buf[pos]), len);
                };

                skip_len_str(col_offset);
                skip_len_str(col_offset);
                skip_len_str(col_offset);
                skip_len_str(col_offset);
                std::string col_name = read_len_str(col_offset);
                skip_len_str(col_offset);

                if (col_offset + 12 <= col_end)
                    col_offset += 12;

                col_names.push_back(std::move(col_name));

                consumed += total_pkt_len;
                break;
            }

            case phase::ROWS:
            {
                if (status == 0xFE)
                {
                    // 0xFE 标记的 OK 包是结果集唯一终结符（可短至 7 字节）。
                    // 不支持经典 EOF 旧协议，无其他兜底。
                    current_phase = phase::DONE;
                    stop          = true;
                    consumed += total_pkt_len;
                    break;
                }
                if (status == 0xFF)
                {
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    return rows;
                }

                if (handler_aborted)
                {
                    consumed += total_pkt_len;
                    break;
                }

                unsigned int row_start = consumed + 4;
                unsigned int row_end   = consumed + total_pkt_len;

                std::vector<char *> &name_ptrs = name_ptrs_cache;
                if (name_ptrs.size() != col_names.size())
                {
                    name_ptrs.resize(col_names.size());
                    for (size_t i = 0; i < col_names.size(); i++)
                        name_ptrs[i] = const_cast<char *>(col_names[i].c_str());
                }
                char **col_names_arr = name_ptrs.empty() ? nullptr : &name_ptrs[0];

                auto get_data = [&](int col_idx) -> std::tuple<unsigned char *, size_t> {
                    if (col_idx < 0 || (size_t)col_idx >= row_data_cache_ptrs_.size())
                        return {nullptr, 0};
                    auto &entry = row_data_cache_ptrs_[col_idx];
                    return {entry.ptr, entry.len};
                };

                row_data_cache_ptrs_.clear();
                row_data_cache_ptrs_.reserve(col_names.size());

                unsigned int pos = row_start;
                for (size_t ci = 0; ci < col_names.size() && pos < row_end; ci++)
                {
                    if (accum_buf[pos] == 0xFB)
                    {
                        row_data_cache_ptrs_.push_back({nullptr, 0});
                        pos++;
                    }
                    else
                    {
                        unsigned long long length = pack_real_num(accum_buf.data(), accum_buf.size(), pos);
                        if (length > 0 && pos + (unsigned int)length <= row_end)
                        {
                            row_data_cache_ptrs_.push_back({&accum_buf[pos], (size_t)length});
                            pos += (unsigned int)length;
                        }
                        else
                        {
                            row_data_cache_ptrs_.push_back({nullptr, 0});
                        }
                    }
                }

                bool keep_going = handler((int)col_names.size(), col_names_arr, get_data);
                rows++;

                if (!keep_going)
                {
                    handler_aborted = true;
                }

                consumed += total_pkt_len;
                break;
            }

            case phase::DONE:
                stop = true;
                break;
            }
        }

        if (consumed > 256 * 1024)
        {
            accum_buf.erase(accum_buf.begin(), accum_buf.begin() + consumed);
            consumed = 0;
        }
    }

    return rows;
}

// MySQL 无 PG ReadyForQuery 那样的协议级事务状态字节，只能按 SQL 关键字跟踪：
// 首词 BEGIN / START TRANSACTION 视为开启，COMMIT / ROLLBACK / END 视为结束。
// 仅用于连接池归还路径判断是否需要自动 ROLLBACK 清理，跟踪不完美不影响安全性（多滚一次是幂等清理）。
namespace
{
enum class txn_keyword : unsigned char
{
    none,
    begin,
    end
};

txn_keyword detect_txn_keyword(const std::string &sql)
{
    size_t i = 0;
    while (i < sql.size() && isspace((unsigned char)sql[i]))
        i++;
    size_t j = i;
    while (j < sql.size() && isalpha((unsigned char)sql[j]))
        j++;
    if (j - i < 3)
        return txn_keyword::none;
    std::string word;
    word.reserve(j - i);
    for (size_t k = i; k < j; k++)
        word.push_back((char)toupper((unsigned char)sql[k]));

    if (word == "BEGIN")
        return txn_keyword::begin;
    if (word == "START")
    {
        while (j < sql.size() && isspace((unsigned char)sql[j]))
            j++;
        size_t m = j;
        while (m < sql.size() && isalpha((unsigned char)sql[m]))
            m++;
        std::string second;
        second.reserve(m - j);
        for (size_t k = j; k < m; k++)
            second.push_back((char)toupper((unsigned char)sql[k]));
        if (second == "TRANSACTION")
            return txn_keyword::begin;
        return txn_keyword::none;
    }
    if (word == "COMMIT" || word == "ROLLBACK" || word == "END")
        return txn_keyword::end;
    return txn_keyword::none;
}
}// namespace

unsigned int mysql_conn_base::exec_dml_impl(const std::string &sql)
{
    unsigned int retry = 0;
RETRY_LABEL:
    if (isclose)
    {
        return (unsigned int)-1;
    }

    error_msg.clear();
    error_code  = 0;
    last_insert_id_ = 0;

    unsigned int n = write_sql(sql);
    if (n == 0)
    {
        // write 失败 = SQL 还没发出去, 幂等安全, 可以重连
        if (is_last_error_reconnectable() && retry < kMaxReconnect && try_reconnect())
        {
            retry++;
            error_msg.clear();
            goto RETRY_LABEL;
        }
        return (unsigned int)-1;
    }

    n = read_loop();
    // OK 包可能被 TCP 分段，单次 read_loop() 可能读到半包
    // 累积循环直到拿到完整的包头(4字节) + payload
    pooled_accum_buf accum_holder;
    auto &accum_buf = accum_holder.buf;
    accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    while (accum_buf.size() < 4)
    {
        n = read_loop();
        if (n == 0) break;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }
    unsigned int pkt_len = 0;
    if (accum_buf.size() >= 4) {
        pkt_len = accum_buf[0] | (accum_buf[1] << 8) | (accum_buf[2] << 16);
    }
    while (accum_buf.size() < 4 + pkt_len)
    {
        n = read_loop();
        if (n == 0) break;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }

    unsigned int iid = 0;
    unsigned int result = parse_affected_rows_fast(accum_buf.data(), accum_buf.size(), &iid);
    if (result != (unsigned int)-1)
    {
        last_insert_id_ = iid;
        txn_keyword kw  = detect_txn_keyword(sql);
        if (kw == txn_keyword::begin)
            in_transaction_.store(true);
        else if (kw == txn_keyword::end)
            in_transaction_.store(false);
    }
    return result;
}

// ---- 异步版本 ----

asio::awaitable<unsigned int> mysql_conn_base::async_fetch_directly_impl(
    const std::string &sql,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    if (isclose)
    {
        co_return 0;
    }

    error_msg.clear();
    error_code  = 0;
    last_insert_id_ = 0;

    unsigned int n = co_await async_write_sql(sql);
    if (n == 0)
    {
        co_return 0;
    }

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;
    unsigned int consumed = 0;

    std::vector<std::string> col_names;
    col_names.reserve(32);
    unsigned int expected_cols = 0;
    std::vector<char *> name_ptrs_cache;

    enum class phase
    {
        INIT,
        COLUMNS,
        ROWS,
        DONE
    };
    phase current_phase = phase::INIT;
    bool stop           = false;
    unsigned int rows   = 0;
    bool handler_aborted = false;

    while (!stop)
    {
        n = co_await async_read_loop();
        if (n == 0)
        {
            if (!error_msg.empty())
                co_return rows;
            co_return rows;
        }

        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

        while (consumed < accum_buf.size())
        {
            if (consumed + 4 > accum_buf.size())
                break;

            unsigned int pkt_len = accum_buf[consumed] |
                                   (accum_buf[consumed + 1] << 8) |
                                   (accum_buf[consumed + 2] << 16);
            if (pkt_len == 0)
            {
                consumed += 4;
                continue;
            }

            unsigned int total_pkt_len = 4 + pkt_len;
            if (consumed + total_pkt_len > accum_buf.size())
                break;

            unsigned char status = accum_buf[consumed + 4];

            switch (current_phase)
            {
            case phase::INIT:
            {
                if (status == 0xFF)
                {
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    co_return rows;
                }
                if (status == 0x00)
                {
                    // async：INIT 阶段收到 OK 包（DML/INSERT 无结果集），直接 DONE
                    unsigned int iid = 0;
                    unsigned int ar = parse_affected_rows_fast(&accum_buf[consumed], total_pkt_len, &iid);
                    if (ar != (unsigned int)-1)
                    {
                        last_insert_id_ = iid;
                    }
                    current_phase = phase::DONE;
                    stop = true;
                    consumed += total_pkt_len;
                    break;
                }
                if (status >= 1 && status <= 250 && pkt_len <= 9)
                {
                    // 列数包：经典格式为 1 字节；容忍 lenenc 变体（如服务端带
                    // metadata_follows 后缀的 2 字节形式），按 lenenc 取列数。
                    unsigned int cc_off = consumed + 4;
                    unsigned long long cc = pack_real_num(accum_buf.data(), accum_buf.size(), cc_off);
                    if (cc <= 0xFFFFFFFFULL)
                    {
                        expected_cols = static_cast<unsigned int>(cc);
                    }
                    current_phase = phase::COLUMNS;
                }
                consumed += total_pkt_len;
                break;
            }

            case phase::COLUMNS:
            {
                if (status == 0xFF)
                {
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    co_return rows;
                }
                if (status == 0xFE)
                {
                    // 0xFE 标记的 OK 包是结果集唯一终结符（可短至 7 字节）。
                    // 出现在 COLUMNS 阶段 = 空结果集，必须直接终止，否则阻塞挂死。
                    // 不支持经典 EOF 旧协议，无其他兜底。
                    current_phase   = phase::DONE;
                    stop            = true;
                    consumed += total_pkt_len;
                    break;
                }

                // 列定义已收齐而本包不是终结包：转入 ROWS 且不消费，重新判定。
                if (expected_cols > 0 && col_names.size() >= expected_cols)
                {
                    current_phase   = phase::ROWS;
                    break;
                }

                unsigned int col_offset = consumed + 4;
                unsigned int col_end    = consumed + total_pkt_len;

                auto skip_len_str = [&](unsigned int &pos) {
                    if (pos >= col_end)
                        return;
                    unsigned char len = accum_buf[pos];
                    if (pos + 1 + len > col_end)
                    {
                        pos = col_end;  // truncated, advance to end
                        return;
                    }
                    pos += 1 + len;
                };
                auto read_len_str = [&](unsigned int &pos) -> std::string {
                    if (pos >= col_end)
                        return {};
                    unsigned char len = accum_buf[pos];
                    pos++;
                    if (len == 0 || pos + len > col_end)
                        return {};
                    return std::string(reinterpret_cast<char *>(&accum_buf[pos]), len);
                };

                skip_len_str(col_offset);
                skip_len_str(col_offset);
                skip_len_str(col_offset);
                skip_len_str(col_offset);
                std::string col_name = read_len_str(col_offset);
                skip_len_str(col_offset);

                if (col_offset + 12 <= col_end)
                    col_offset += 12;

                col_names.push_back(std::move(col_name));

                consumed += total_pkt_len;
                break;
            }

            case phase::ROWS:
            {
                if (status == 0xFE)
                {
                    // 0xFE 标记的 OK 包是结果集唯一终结符（可短至 7 字节）。
                    // 不支持经典 EOF 旧协议，无其他兜底。
                    current_phase = phase::DONE;
                    stop          = true;
                    consumed += total_pkt_len;
                    break;
                }
                if (status == 0xFF)
                {
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    co_return rows;
                }

                if (handler_aborted)
                {
                    consumed += total_pkt_len;
                    break;
                }

                unsigned int row_start = consumed + 4;
                unsigned int row_end   = consumed + total_pkt_len;

                std::vector<char *> &name_ptrs = name_ptrs_cache;
                if (name_ptrs.size() != col_names.size())
                {
                    name_ptrs.resize(col_names.size());
                    for (size_t i = 0; i < col_names.size(); i++)
                        name_ptrs[i] = const_cast<char *>(col_names[i].c_str());
                }
                char **col_names_arr = name_ptrs.empty() ? nullptr : &name_ptrs[0];

                auto get_data = [&](int col_idx) -> std::tuple<unsigned char *, size_t> {
                    if (col_idx < 0 || (size_t)col_idx >= row_data_cache_ptrs_.size())
                        return {nullptr, 0};
                    auto &entry = row_data_cache_ptrs_[col_idx];
                    return {entry.ptr, entry.len};
                };

                row_data_cache_ptrs_.clear();
                row_data_cache_ptrs_.reserve(col_names.size());

                unsigned int pos = row_start;
                for (size_t ci = 0; ci < col_names.size() && pos < row_end; ci++)
                {
                    if (accum_buf[pos] == 0xFB)
                    {
                        row_data_cache_ptrs_.push_back({nullptr, 0});
                        pos++;
                    }
                    else
                    {
                        unsigned long long length = pack_real_num(accum_buf.data(), accum_buf.size(), pos);
                        if (length > 0 && pos + (unsigned int)length <= row_end)
                        {
                            row_data_cache_ptrs_.push_back({&accum_buf[pos], (size_t)length});
                            pos += (unsigned int)length;
                        }
                        else
                        {
                            row_data_cache_ptrs_.push_back({nullptr, 0});
                        }
                    }
                }

                bool keep_going = handler((int)col_names.size(), col_names_arr, get_data);
                rows++;

                if (!keep_going)
                {
                    handler_aborted = true;
                }

                consumed += total_pkt_len;
                break;
            }

            case phase::DONE:
                stop = true;
                break;
            }
        }

        if (consumed > 256 * 1024)
        {
            accum_buf.erase(accum_buf.begin(), accum_buf.begin() + consumed);
            consumed = 0;
        }
    }

    co_return rows;
}

asio::awaitable<unsigned int> mysql_conn_base::async_exec_dml(const std::string &sql)
{
    unsigned int retry = 0;
RETRY_LABEL:
    if (isclose)
    {
        co_return (unsigned int)-1;
    }

    error_msg.clear();
    error_code  = 0;
    last_insert_id_ = 0;

    unsigned int n = co_await async_write_sql(sql);
    if (n == 0)
    {
        // write 失败 = SQL 还没发出去, 幂等安全, 可以重连
        if (is_last_error_reconnectable() && retry < kMaxReconnect && co_await async_try_reconnect())
        {
            retry++;
            error_msg.clear();
            goto RETRY_LABEL;
        }
        co_return (unsigned int)-1;
    }

    n = co_await async_read_loop();
    // OK 包可能被 TCP 分段，单次 async_read_loop() 可能读到半包
    pooled_accum_buf accum_holder;
    auto &accum_buf = accum_holder.buf;
    accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    while (accum_buf.size() < 4)
    {
        n = co_await async_read_loop();
        if (n == 0) break;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }
    unsigned int pkt_len = 0;
    if (accum_buf.size() >= 4) {
        pkt_len = accum_buf[0] | (accum_buf[1] << 8) | (accum_buf[2] << 16);
    }
    while (accum_buf.size() < 4 + pkt_len)
    {
        n = co_await async_read_loop();
        if (n == 0) break;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }

    unsigned int iid = 0;
    unsigned int result = parse_affected_rows_fast(accum_buf.data(), accum_buf.size(), &iid);
    if (result != (unsigned int)-1)
    {
        last_insert_id_ = iid;
        txn_keyword kw  = detect_txn_keyword(sql);
        if (kw == txn_keyword::begin)
            in_transaction_.store(true);
        else if (kw == txn_keyword::end)
            in_transaction_.store(false);
    }
    co_return result;
}

// ===== 断线自动重连实现 =====

bool mysql_conn_base::is_last_error_reconnectable() const
{
    if (!conn_link) return false;
    const auto &ec = conn_link->ec;
    if (!ec) return false;

    // 只认 asio 抽象错误码, 白名单策略
    return ec == asio::error::eof              ||
           ec == asio::error::connection_reset ||
           ec == asio::error::timed_out        ||
           ec == asio::error::broken_pipe      ||
           ec == asio::error::not_connected;
}

bool mysql_conn_base::try_reconnect()
{
    if (!has_conn_config_)   return false;
    if (isclose)             return false;

    hard_close();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return connect(last_conn_config_);
}

asio::awaitable<bool> mysql_conn_base::async_try_reconnect()
{
    if (!has_conn_config_)   co_return false;
    if (isclose)             co_return false;

    hard_close();

    asio::steady_timer t(*conn_link->io_ctx, std::chrono::milliseconds(50));
    co_await t.async_wait(asio::use_awaitable);

    co_return co_await async_connect(last_conn_config_);
}

}// namespace orm
