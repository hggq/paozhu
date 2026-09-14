/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <charconv>
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
    stmt_cache_.clear([this](uint32_t id)
                      { stmt_close_impl(id); });
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
    // stmt_id 是「这条会话」的句柄：二次 connect() 若不丢缓存，最多 128 个死句柄
    // 会留在 map 里，而 warm 路径永远等不到它们的失效错误（服务端早已随会话销毁）。
    // 必须在替换 socket 之前清，此时旧 socket 还可能在，COM_STMT_CLOSE 才发得出去。
    stmt_cache_.clear([this](uint32_t id)
                      { stmt_close_impl(id); });
    isclose              = false;
    conn_link->sock_type = 0;
    conn_link->sslsocket.reset();
    conn_link->ssl_context.reset();
    conn_link->localsocket.reset();
    conn_link->socket = std::make_unique<asio::ip::tcp::socket>(*conn_link->io_ctx);
    conn_link->ec.clear();
    server_enable_ssl = false;
    seq_next_id       = 0;
    std::memset(static_cast<void *>(&server_hello), 0, sizeof(server_hello));

    // 保存配置用于断线自动重连（仅在首次或配置变化时存储）
    if (!has_conn_config_ || !(last_conn_config_ == conn_config))
    {
        last_conn_config_ = conn_config;
        has_conn_config_  = true;
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

    // CLIENT_DEPRECATE_EOF(bit 24) 是本库结果集解析的硬基线：结果集终结包(OK-via-EOF)与旧协议
    // 的列段分隔包(经典 EOF)首字节同为 254、载荷都 < 8 字节，只有这一位能把两者分开
    // (服务器源码 protocol_classic.cc end_result_metadata)。服务器不 advertise ⇒ 没有无歧义的
    // 判定依据 ⇒ 拒绝连接；让旧协议包进了解析器只会静默丢行或协议失步，比连不上更难查。
    const unsigned int server_caps =
        server_hello.capability_flags_low |
        (static_cast<unsigned int>(server_hello.capability_flags_high) << 16);
    if ((server_caps & CLIENT_DEPRECATE_EOF) == 0)
    {
        error_msg  = "server does not advertise CLIENT_DEPRECATE_EOF (bit 24), not supported";
        error_code = 20;
        isclose    = true;
        return false;
    }

    // 仅支持 caching_sha2_password（禁止实现 mysql_native_password：MySQL 9 已移除，
    // MariaDB 12.3+ 支持 caching_sha2_password）。初始应答固定按该插件计算并声明；
    // 服务器插件不一致时由 Auth Switch 携带新 salt 重协商
    std::string init_plugin = "caching_sha2_password";
    std::string init_resp   = compute_auth_response(init_plugin, server_hello.auth_plugin_salt_data, conn_config.password);
    if (init_resp.empty() && !conn_config.password.empty())
    {
        error_code = 2;
        isclose    = true;
        return false;
    }

    const bool is_mariadb = server_hello.server_version.find("MariaDB") != std::string::npos;
    client_flags          = is_mariadb ? CLIENT_PZORM_MARIADB_FLAGS : CLIENT_PZORM_MYSQL_FLAGS;
    seq_next_id           = 0;
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
    send_data.push_back(static_cast<char>(init_resp.size() & 0xFF));
    send_data.append(init_resp);
    send_data.append(conn_config.dbname);
    send_data.push_back(0x00);
    send_data.append(init_plugin);
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
    send_data[0] = (payload_len) & 0xFF;
    send_data[1] = (payload_len >> 8) & 0xFF;
    send_data[2] = (payload_len >> 16) & 0xFF;
    if (conn_link->sock_type == 3)
    {
        // Unix socket: no SSL, keep sock_type=3
    }
    else
    {
        conn_link->sock_type = 1;
    }
    // ssl=ON 语义是"必须加密"：服务器未声明 CLIENT_SSL 时直接失败，不再静默降级成明文连接
    //（与 pg_conn 同语义/同 error_code）。Unix socket 不经网络、无中间人面，故 issock 不受此限制。
    if (conn_config.isssl && !conn_config.issock && !server_enable_ssl)
    {
        error_msg  = "server does not support SSL (requested by client config): plaintext downgrade refused; "
                     "enable TLS on the server (or set ssl=OFF), and verify server identity with sslverify=ON + sslhost=<host/domain in certificate>";
        error_code = 5;
        isclose    = true;
        return false;
    }
    if (conn_config.isssl && server_enable_ssl && !conn_config.issock)
    {
        if (!conn_config.islocal)
        {
            //.sock please use localsocket
            send_data[0] = 32;
            client_flags = is_mariadb ? CLIENT_PZORM_MARIADB_SSL_FLAGS : CLIENT_PZORM_MYSQL_SSL_FLAGS;
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
            // verify_mode 与 CA 路径必须在创建 stream（SSL_new）之前设在 context 上：
            // SSL 对象只在创建那一刻继承这两项，之后再设到 context 上不生效（sslverify 会静默失效）。
            if (conn_config.sslverify)
            {
                conn_link->ssl_context->set_default_verify_paths();
                conn_link->ssl_context->set_verify_mode(asio::ssl::verify_peer);
            }
            asio::ssl::stream<asio::ip::tcp::socket> ssl_temp_socket(std::move(*conn_link->socket.release()), *conn_link->ssl_context);
            //std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket=std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));
            conn_link->sslsocket = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));

            const std::string &sni_host = conn_config.sslhost.empty() ? conn_config.host : conn_config.sslhost;
            if (!sni_host.empty())
            {
                SSL_set_tlsext_host_name(conn_link->sslsocket->native_handle(), sni_host.c_str());
            }
            if (conn_config.sslverify)
            {
                // 主机名校验回调必须设在 stream 上（SSL_set_verify）；设在 context 上不会作用到已创建的 SSL 对象。
                // 未显式配置 sslhost 时以连接 host 兜底，避免"只验证书链不验主机名"的缝隙（与 PG 侧一致）
                conn_link->sslsocket->set_verify_callback(asio::ssl::host_name_verification(sni_host));
            }

            conn_link->sslsocket->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

            conn_link->sslsocket->handshake(asio::ssl::stream_base::client, conn_link->ec);
            if (conn_link->ec)
            {
                error_msg = conn_link->ec.message();
                // 提醒排查方向：自签 CA 不受信 / 证书 CN|SAN 与主机名不符
                error_msg += conn_config.sslverify ? " [TLS handshake failed with sslverify=ON: make sure the server CA is trusted "
                                                     "(system CA or SSL_CERT_FILE) and the certificate CN/SAN matches sslhost "
                                                     "(falls back to host when sslhost is empty)]" :
                                                     " [TLS handshake failed with sslverify=OFF: server identity was not verified; "
                                                     "in production use sslverify=ON together with sslhost=<host/domain in certificate>]";
                error_code = 2;
                isclose    = true;
                return false;
            }
            conn_link->sock_type = 2;// 握手成功后才标记 SSL 已建立

            // SSL 通道建立后重发握手响应，包长必须写满 3 字节
            //（此前只写低 8 位，payload > 255 字节时协议失步）
            {
                auto pl      = send_data.size() - 4;
                send_data[0] = (pl) & 0xFF;
                send_data[1] = (pl >> 8) & 0xFF;
                send_data[2] = (pl >> 16) & 0xFF;
            }
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

            // Auth Switch / 公钥请求可能多轮：以有界循环替代 goto（防服务器反复挑战致无限循环），与 async_connect SSL 分支一致
            for (int auth_round = 0; auth_round < 4; ++auth_round)
            {
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

                unsigned char pkt_type = (unsigned char)_cache_data[4];

                // caching_sha2_password Auth More Data under SSL:
                // subtype 0x03 → plaintext password (TLS already encrypts channel)
                // subtype 0x04 → public key request (still plaintext over TLS)
                if (pkt_type == 0x01 && n >= 6 && _cache_data[5] == 0x04)
                {
                    // Public key request under TLS: send plaintext password
                    // (channel is already encrypted by TLS, no RSA needed)
                    send_data.clear();
                    unsigned char more_seq = (unsigned char)(_cache_data[3] & 0xFF);
                    seq_next_id            = more_seq + 1;
                    send_data.push_back(0x00);
                    send_data.push_back(0x00);
                    send_data.push_back(0x00);
                    send_data.push_back(seq_next_id);
                    send_data.append(conn_config.password);
                    send_data.push_back(0x00);
                    auto pl2     = send_data.size() - 4;
                    send_data[0] = (pl2) & 0xFF;
                    send_data[1] = (pl2 >> 8) & 0xFF;
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
                    continue;
                }
                else if (pkt_type == 0x01 && n >= 6 && _cache_data[5] == 0x03)
                {
                    // subtype 0x03: fast auth success（缓存命中），客户端不应答，服务器紧跟终态 OK/ERR 包。
                    // 循环补读读满第二个整包再判终态（与非 SSL 路径一致）：不用脆弱阈值、
                    // 不 goto+memset 从头重读（会丢弃已到达字节，partial-merge 时错位或阻塞）。
                    unsigned int offset = static_cast<unsigned char>(_cache_data[0]) | (static_cast<unsigned char>(_cache_data[1]) << 8) | (static_cast<unsigned char>(_cache_data[2]) << 16);
                    offset += 4;// 第一个包([01 03])总长 = payload + 4 字节包头

                    // 读满第二个包的 4 字节包头(payload_len[3] + seq[1])
                    while (n < offset + 4)
                    {
                        if (n >= CACHE_DATA_LENGTH)
                        {
                            error_msg  = " connect fail! fast-auth packet overflow ";
                            error_code = 8;
                            isclose    = true;
                            return false;
                        }
                        std::size_t got = 0;
                        try
                        {
                            got = conn_link->sslsocket->read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n));
                        }
                        catch (const std::exception &e)
                        {
                            error_msg.append(e.what());
                            isclose = true;
                            return false;
                        }
                        if (got == 0)
                        {
                            error_msg  = " connect fail! fast-auth read closed ";
                            error_code = 1;
                            isclose    = true;
                            return false;
                        }
                        n += got;
                    }

                    // 解析第二个包 payload 长度，循环补读至整包到达
                    unsigned int term_len = static_cast<unsigned char>(_cache_data[offset]) | (static_cast<unsigned char>(_cache_data[offset + 1]) << 8) | (static_cast<unsigned char>(_cache_data[offset + 2]) << 16);
                    unsigned int need     = offset + 4 + term_len;
                    while (n < need)
                    {
                        if (n >= CACHE_DATA_LENGTH)
                        {
                            error_msg  = " connect fail! fast-auth packet overflow ";
                            error_code = 8;
                            isclose    = true;
                            return false;
                        }
                        std::size_t got = 0;
                        try
                        {
                            got = conn_link->sslsocket->read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n));
                        }
                        catch (const std::exception &e)
                        {
                            error_msg.append(e.what());
                            isclose = true;
                            return false;
                        }
                        if (got == 0)
                        {
                            error_msg  = " connect fail! fast-auth read closed ";
                            error_code = 1;
                            isclose    = true;
                            return false;
                        }
                        n += got;
                    }

                    // 判定终态包类型（第二个包 payload 首字节）
                    unsigned char final_type = static_cast<unsigned char>(_cache_data[offset + 4]);
                    if (final_type == 0xFF)
                    {
                        parse_error_packet(_cache_data + offset, n - offset);
                        isclose = true;
                        return false;
                    }
                    if (final_type == 0x00)
                    {
                        seq_next_id = static_cast<unsigned char>((_cache_data[offset + 3] & 0xFF) + 1);
                        return true;
                    }
                    error_msg  = " connect fail! unexpected fast-auth final packet ";
                    error_code = 8;
                    isclose    = true;
                    return false;
                }

                // Auth Switch Request (0xFE): server switches auth plugin
                // (MariaDB 12.3 + caching_sha2_password always triggers this
                // because server hello declares mysql_native_password first).
                if (pkt_type == 0xFE && n >= 6)
                {
                    unsigned int off = 5;
                    std::string sw_plugin;
                    while (off < n && _cache_data[off] != 0)
                    {
                        sw_plugin.push_back(_cache_data[off]);
                        off++;
                    }
                    if (off < n)
                    {
                        off++;
                    }
                    std::string sw_salt;
                    while (off < n)
                    {
                        sw_salt.push_back(_cache_data[off]);
                        off++;
                    }
                    while (!sw_salt.empty() && sw_salt.back() == '\0')
                    {
                        sw_salt.pop_back();
                    }

                    server_hello.auth_plugin_salt_data = sw_salt;
                    server_hello.auth_plugin_name      = sw_plugin;
                    std::string auth_resp              = compute_auth_response(sw_plugin, sw_salt, conn_config.password);
                    if (auth_resp.empty() && !conn_config.password.empty())
                    {
                        error_code = 2;
                        isclose    = true;
                        return false;
                    }

                    unsigned char sw_seq = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
                    send_data.clear();
                    send_data.push_back(0x00);
                    send_data.push_back(0x00);
                    send_data.push_back(0x00);
                    send_data.push_back(sw_seq);
                    send_data.append(auth_resp);
                    auto sw_len  = auth_resp.size();
                    send_data[0] = static_cast<char>(sw_len & 0xFF);
                    send_data[1] = static_cast<char>((sw_len >> 8) & 0xFF);
                    send_data[2] = static_cast<char>((sw_len >> 16) & 0xFF);
                    seq_next_id  = sw_seq;

                    try
                    {
                        n = asio::write(*conn_link->sslsocket, asio::buffer(send_data));
                    }
                    catch (const std::exception &e)
                    {
                        error_msg.append(e.what());
                        error_code = 2;
                        isclose    = true;
                        return false;
                    }
                    continue;
                }

                // Error packet
                if (pkt_type == 0xFF)
                {
                    parse_error_packet(_cache_data, n);
                    isclose = true;
                    return false;
                }

                // OK packet
                if (pkt_type == 0x00)
                {
                    seq_next_id = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
                    return true;
                }

                // Multi-result OK (status packet with 0x03 type)
                if (pkt_type == 0x03)
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

                        if ((unsigned char)_cache_data[4] == 0xFF)
                        {
                            parse_error_packet(_cache_data, n);
                            isclose = true;
                            return false;
                        }
                        else if ((unsigned char)_cache_data[4] == 0x00)
                        {
                            return true;
                        }
                    }
                }

                error_msg  = " connect fail! unexpected auth response";
                error_code = 9;
                isclose    = true;
                return false;
            }

            error_msg  = " auth switch rounds exhausted ";
            error_code = 9;
            isclose    = true;
            return false;
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

    // Auth Switch 可能多轮挑战-应答：以有界循环替代 goto（防服务器反复 0xFE 致无限循环），与 async_connect 一致
    for (int auth_round = 0; auth_round < 4; ++auth_round)
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
            if (n < 255 || n > 2048)
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
                error_code      = ec;
                // 跳过 4 字节 pkt header + 1 byte 0xFF + 2 byte ec + 1 byte '#' + 5 byte sql_state
                unsigned int msg_off = 4 + 1 + 2 + 1 + 5;
                error_msg            = "MySQL auth error: ";
                if (n > msg_off)
                    error_msg.append(reinterpret_cast<const char *>(_cache_data + msg_off), n - msg_off);
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

            // 下发公钥先做钉住校验（配置 server_public_key 才生效），防中间人换公钥骗密码。
            // 注意 sock_type==2（SSL）不会走到这里：SSL 分支在上方直接回明文密码并已返回。
            if (!check_server_public_key(conn_config, &_cache_data[5], n - 5))
            {
                isclose = true;
                return false;
            }

            bool isok = server_public_key_encrypt(conn_config.password, &_cache_data[5], n - 5);
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
            // Auth Switch Request: [0xFE][plugin][NUL][salt]
            unsigned int off = 5;
            std::string sw_plugin;
            while (off < n && _cache_data[off] != 0)
            {
                sw_plugin.push_back(_cache_data[off]);
                off++;
            }
            if (off < n)
            {
                off++;
            }
            std::string sw_salt;
            while (off < n)
            {
                sw_salt.push_back(_cache_data[off]);
                off++;
            }
            while (!sw_salt.empty() && sw_salt.back() == '\0')
            {
                sw_salt.pop_back();
            }

            server_hello.auth_plugin_salt_data = sw_salt;
            server_hello.auth_plugin_name      = sw_plugin;
            std::string auth_resp              = compute_auth_response(sw_plugin, sw_salt, conn_config.password);
            if (auth_resp.empty() && !conn_config.password.empty())
            {
                error_code = 2;
                isclose    = true;
                return false;
            }

            unsigned char sw_seq = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
            send_data.clear();
            send_data.push_back(0x00);
            send_data.push_back(0x00);
            send_data.push_back(0x00);
            send_data.push_back(sw_seq);
            send_data.append(auth_resp);
            auto sw_len  = auth_resp.size();
            send_data[0] = static_cast<char>(sw_len & 0xFF);
            send_data[1] = static_cast<char>((sw_len >> 8) & 0xFF);
            send_data[2] = static_cast<char>((sw_len >> 16) & 0xFF);
            seq_next_id  = sw_seq;

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
                error_code = 2;
                isclose    = true;
                return false;
            }
            continue;
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
        else if ((unsigned char)_cache_data[4] == 0x00)
        {
            seq_next_id = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
            return true;
        }
        else if ((unsigned char)_cache_data[5] == 0x03)
        {
            // fast-auth success（缓存命中）：服务器发 [01 03] 后紧跟终态 OK/ERR 包，
            // 两包可能合并到达(一次 read_some 读满)或分片到达。此处循环补读，读满第二个
            // 整包后再判终态：
            //  1) 不用脆弱的 n>=offset+len+5 阈值（曾 off-by-one：整包只需 +4，误判为未完整）
            //  2) 不 goto/memset 从头重读（会丢弃已到达字节，并阻塞等待服务器永不再发的数据 -> 永久挂起）
            unsigned int offset = static_cast<unsigned char>(_cache_data[0]) | (static_cast<unsigned char>(_cache_data[1]) << 8) | (static_cast<unsigned char>(_cache_data[2]) << 16);
            offset += 4;// 第一个包([01 03])总长 = payload + 4 字节包头

            // 读满第二个包的 4 字节包头(payload_len[3] + seq[1])
            while (n < offset + 4)
            {
                if (n >= CACHE_DATA_LENGTH)
                {
                    error_msg  = " connect fail! fast-auth packet overflow ";
                    error_code = 8;
                    isclose    = true;
                    return false;
                }
                std::size_t got = 0;
                if (conn_link->sock_type == 3)
                {
                    got = conn_link->localsocket->read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n), conn_link->ec);
                }
                else
                {
                    got = conn_link->socket->read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n), conn_link->ec);
                }
                if (conn_link->ec || got == 0)
                {
                    error_msg.append("connect fast-auth read error! ");
                    error_msg.append(conn_link->ec.message());
                    error_code = 1;
                    isclose    = true;
                    return false;
                }
                n += got;
            }

            // 解析第二个包 payload 长度，循环补读至整包到达
            unsigned int term_len = static_cast<unsigned char>(_cache_data[offset]) | (static_cast<unsigned char>(_cache_data[offset + 1]) << 8) | (static_cast<unsigned char>(_cache_data[offset + 2]) << 16);
            unsigned int need     = offset + 4 + term_len;
            while (n < need)
            {
                if (n >= CACHE_DATA_LENGTH)
                {
                    error_msg  = " connect fail! fast-auth packet overflow ";
                    error_code = 8;
                    isclose    = true;
                    return false;
                }
                std::size_t got = 0;
                if (conn_link->sock_type == 3)
                {
                    got = conn_link->localsocket->read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n), conn_link->ec);
                }
                else
                {
                    got = conn_link->socket->read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n), conn_link->ec);
                }
                if (conn_link->ec || got == 0)
                {
                    error_msg.append("connect fast-auth read error! ");
                    error_msg.append(conn_link->ec.message());
                    error_code = 1;
                    isclose    = true;
                    return false;
                }
                n += got;
            }

            // 判定终态包类型（第二个包 payload 首字节）
            unsigned char final_type = static_cast<unsigned char>(_cache_data[offset + 4]);
            if (final_type == 0xFF)
            {
                parse_error_packet(_cache_data + offset, n - offset);
                isclose = true;
                return false;
            }
            if (final_type == 0x00)
            {
                seq_next_id = static_cast<unsigned char>((_cache_data[offset + 3] & 0xFF) + 1);
                return true;
            }
            error_msg  = " connect fail! unexpected fast-auth final packet ";
            error_code = 8;
            isclose    = true;
            return false;
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

    error_msg  = " auth switch rounds exhausted ";
    error_code = 9;
    isclose    = true;
    return false;
}

std::string mysql_conn_base::compute_auth_response(const std::string &plugin, const std::string &salt, const std::string &password)
{
    if (password.empty())
    {
        return std::string();
    }
    if (salt.size() < 20)
    {
        error_msg = "auth switch salt too short";
        return std::string();
    }

    std::string resp;
    if (plugin == "caching_sha2_password")
    {
        // SHA256(pwd) XOR SHA256( SHA256(SHA256(pwd)) || salt )
        unsigned char h1[32], h2[32], h3[32];
        SHA256(reinterpret_cast<const unsigned char *>(password.data()), password.size(), h1);
        SHA256(h1, 32, h2);
        unsigned char buf[52];
        std::memcpy(buf, h2, 32);
        std::memcpy(buf + 32, salt.data(), 20);
        SHA256(buf, 52, h3);
        resp.resize(32);
        for (unsigned i = 0; i < 32; ++i)
        {
            resp[i] = static_cast<char>(h1[i] ^ h3[i]);
        }
    }
    else
    {
        error_msg = "unsupported auth switch plugin: " + plugin;
        return std::string();
    }
    return resp;
}

namespace
{
// 公钥指纹：对 DER 编码的 SubjectPublicKeyInfo 求 SHA-256，输出十六进制小写
std::string pubkey_fingerprint_hex(EVP_PKEY *key)
{
    unsigned char *der = nullptr;
    int der_len        = i2d_PUBKEY(key, &der);
    if (der_len <= 0 || der == nullptr)
    {
        return std::string();
    }
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256(der, static_cast<size_t>(der_len), md);
    OPENSSL_free(der);

    static const char hex_table[] = "0123456789abcdef";
    std::string out;
    out.reserve(SHA256_DIGEST_LENGTH * 2);
    for (unsigned int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        out.push_back(hex_table[md[i] >> 4]);
        out.push_back(hex_table[md[i] & 0x0F]);
    }
    return out;
}

// 可信公钥既可以是内联 PEM 文本，也可以是 PEM 文件路径
EVP_PKEY *load_trusted_pubkey(const std::string &trusted)
{
    // 配置解析保留值内空格（密码语义），这里裁掉首尾空白：
    // 否则模板行内注释残留的空格会让路径打不开
    std::size_t begin = trusted.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos)
    {
        return nullptr;
    }
    std::size_t end = trusted.find_last_not_of(" \t\r\n");
    std::string val = trusted.substr(begin, end - begin + 1);

    BIO *bio = nullptr;
    if (val.find("-----BEGIN") != std::string::npos)
    {
        bio = BIO_new_mem_buf(val.data(), static_cast<int>(val.size()));
    }
    else
    {
        bio = BIO_new_file(val.c_str(), "r");
    }
    if (bio == nullptr)
    {
        return nullptr;
    }
    EVP_PKEY *key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return key;
}
}// namespace

// caching_sha2_password 在非 SSL 通道上做 full auth 时，服务器随握手下发 RSA 公钥，
// 客户端用它加密"密码 XOR salt"后回传。公钥若不校验，中间人可下发自己的公钥、
// 用私钥解出凭据（CVE-2015-3152/BACKRONYM 的成因）。
// 这里按配置做钉住校验：conn_config.server_public_key 指定本地可信公钥（PEM 路径
// 或内联 PEM）时，与下发公钥做 EVP_PKEY_eq 比对，不一致立即中止认证；未配置时
// 保持原有行为（不阻断非 SSL 连接），并把下发公钥留在 server_pubkey_pem 供 TOFU 固化。
bool mysql_conn_base::check_server_public_key(const orm_conn_t &conn_config, const unsigned char *data, unsigned int length)
{
    server_pubkey_pem.assign(reinterpret_cast<const char *>(data), length);

    BIO *bio             = BIO_new_mem_buf(data, static_cast<int>(length));
    EVP_PKEY *server_key = (bio == nullptr) ? nullptr : PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (server_key == nullptr)
    {
        error_msg  = "server public key parse failed";
        error_code = 4;
        return false;
    }

    if (conn_config.server_public_key.empty())
    {
        EVP_PKEY_free(server_key);
        return true;
    }

    EVP_PKEY *trusted_key = load_trusted_pubkey(conn_config.server_public_key);
    if (trusted_key == nullptr)
    {
        EVP_PKEY_free(server_key);
        error_msg  = "configured server_public_key load failed: " + conn_config.server_public_key;
        error_code = 4;
        return false;
    }

    bool ismatch = (EVP_PKEY_eq(server_key, trusted_key) == 1);
    if (!ismatch)
    {
        error_msg = "possible MITM: server RSA public key mismatch (server fp=" + pubkey_fingerprint_hex(server_key) +
                    ", trusted fp=" + pubkey_fingerprint_hex(trusted_key) + ")";
        error_code = 4;
    }
    EVP_PKEY_free(server_key);
    EVP_PKEY_free(trusted_key);
    return ismatch;
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
    // stmt_id 是「这条会话」的句柄：二次 connect() 若不丢缓存，最多 128 个死句柄
    // 会留在 map 里，而 warm 路径永远等不到它们的失效错误（服务端早已随会话销毁）。
    // 必须在替换 socket 之前清，此时旧 socket 还可能在，COM_STMT_CLOSE 才发得出去。
    {
        std::vector<uint32_t> ids;
        stmt_cache_.clear([&ids](uint32_t id)
                          { ids.push_back(id); });
        for (uint32_t id : ids)
            co_await async_stmt_close(id);
    }
    isclose              = false;
    conn_link->sock_type = 0;
    conn_link->sslsocket.reset();
    conn_link->ssl_context.reset();
    conn_link->localsocket.reset();
    conn_link->socket = std::make_unique<asio::ip::tcp::socket>(*conn_link->io_ctx);
    conn_link->ec.clear();
    server_enable_ssl = false;
    seq_next_id       = 0;
    std::memset(static_cast<void *>(&server_hello), 0, sizeof(server_hello));

    // 保存配置用于断线自动重连（仅在首次或配置变化时存储）
    if (!has_conn_config_ || !(last_conn_config_ == conn_config))
    {
        last_conn_config_ = conn_config;
        has_conn_config_  = true;
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

    // CLIENT_DEPRECATE_EOF(bit 24) 是本库结果集解析的硬基线：结果集终结包(OK-via-EOF)与旧协议
    // 的列段分隔包(经典 EOF)首字节同为 254、载荷都 < 8 字节，只有这一位能把两者分开
    // (服务器源码 protocol_classic.cc end_result_metadata)。服务器不 advertise ⇒ 没有无歧义的
    // 判定依据 ⇒ 拒绝连接；让旧协议包进了解析器只会静默丢行或协议失步，比连不上更难查。
    const unsigned int server_caps =
        server_hello.capability_flags_low |
        (static_cast<unsigned int>(server_hello.capability_flags_high) << 16);
    if ((server_caps & CLIENT_DEPRECATE_EOF) == 0)
    {
        error_msg  = "server does not advertise CLIENT_DEPRECATE_EOF (bit 24), not supported";
        error_code = 20;
        isclose    = true;
        co_return false;
    }

    // Only caching_sha2_password is supported, and mysql_native_password is prohibited
    // 仅支持 caching_sha2_password（禁止实现 mysql_native_password：MySQL 9 已移除，
    // MariaDB 12.3+ 支持 caching_sha2_password）。初始应答固定按该插件计算并声明；
    // 服务器插件不一致时由 Auth Switch 携带新 salt 重协商
    std::string init_plugin = "caching_sha2_password";
    std::string init_resp   = compute_auth_response(init_plugin, server_hello.auth_plugin_salt_data, conn_config.password);
    if (init_resp.empty() && !conn_config.password.empty())
    {
        error_code = 2;
        isclose    = true;
        co_return false;
    }

    const bool is_mariadb = server_hello.server_version.find("MariaDB") != std::string::npos;
    client_flags          = is_mariadb ? CLIENT_PZORM_MARIADB_FLAGS : CLIENT_PZORM_MYSQL_FLAGS;
    seq_next_id           = 0;
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
    send_data.push_back(static_cast<char>(init_resp.size() & 0xFF));
    send_data.append(init_resp);
    send_data.append(conn_config.dbname);
    send_data.push_back(0x00);
    send_data.append(init_plugin);
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
    send_data[0] = (payload_len) & 0xFF;
    send_data[1] = (payload_len >> 8) & 0xFF;
    send_data[2] = (payload_len >> 16) & 0xFF;
    if (conn_link->sock_type == 3)
    {
        // Unix socket: no SSL, keep sock_type=3
    }
    else
    {
        conn_link->sock_type = 1;
    }
    // ssl=ON 语义是"必须加密"：服务器未声明 CLIENT_SSL 时直接失败，不再静默降级成明文连接
    //（与同步 connect / pg_conn 同语义同 error_code）。issock 无中间人面，不受此限制。
    if (conn_config.isssl && !conn_config.issock && !server_enable_ssl)
    {
        error_msg  = "server does not support SSL (requested by client config): plaintext downgrade refused; "
                     "enable TLS on the server (or set ssl=OFF), and verify server identity with sslverify=ON + sslhost=<host/domain in certificate>";
        error_code = 5;
        isclose    = true;
        co_return false;
    }
    if (conn_config.isssl && server_enable_ssl && !conn_config.issock)
    {
        if (!conn_config.islocal)
        {
            //.sock please use localsocket
            send_data[0] = 32;
            client_flags = is_mariadb ? CLIENT_PZORM_MARIADB_SSL_FLAGS : CLIENT_PZORM_MYSQL_SSL_FLAGS;
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
            // verify_mode 与 CA 路径必须在创建 stream（SSL_new）之前设在 context 上：
            // SSL 对象只在创建那一刻继承这两项，之后再设到 context 上不生效（sslverify 会静默失效）。
            if (conn_config.sslverify)
            {
                conn_link->ssl_context->set_default_verify_paths();
                conn_link->ssl_context->set_verify_mode(asio::ssl::verify_peer);
            }
            asio::ssl::stream<asio::ip::tcp::socket> ssl_temp_socket(std::move(*conn_link->socket.release()), *conn_link->ssl_context);
            //std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket=std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));
            conn_link->sslsocket = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));

            const std::string &sni_host = conn_config.sslhost.empty() ? conn_config.host : conn_config.sslhost;
            if (!sni_host.empty())
            {
                SSL_set_tlsext_host_name(conn_link->sslsocket->native_handle(), sni_host.c_str());
            }
            if (conn_config.sslverify)
            {
                // 主机名校验回调必须设在 stream 上（SSL_set_verify）；设在 context 上不会作用到已创建的 SSL 对象。
                // 未显式配置 sslhost 时以连接 host 兜底，避免"只验证书链不验主机名"的缝隙（与 PG 侧一致）
                conn_link->sslsocket->set_verify_callback(asio::ssl::host_name_verification(sni_host));
            }

            conn_link->sslsocket->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

            std::tie(conn_link->ec) = co_await conn_link->sslsocket->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
            if (conn_link->ec)
            {
                error_msg = conn_link->ec.message();
                // 提醒排查方向：自签 CA 不受信 / 证书 CN|SAN 与主机名不符
                error_msg += conn_config.sslverify ? " [TLS handshake failed with sslverify=ON: make sure the server CA is trusted "
                                                     "(system CA or SSL_CERT_FILE) and the certificate CN/SAN matches sslhost "
                                                     "(falls back to host when sslhost is empty)]" :
                                                     " [TLS handshake failed with sslverify=OFF: server identity was not verified; "
                                                     "in production use sslverify=ON together with sslhost=<host/domain in certificate>]";
                error_code = 2;
                isclose    = true;
                co_return false;
            }
            conn_link->sock_type = 2;// 握手成功后才标记 SSL 已建立

            // SSL 通道建立后重发握手响应，包长必须写满 3 字节
            //（此前只写低 8 位，payload > 255 字节时协议失步）
            {
                auto pl      = send_data.size() - 4;
                send_data[0] = (pl) & 0xFF;
                send_data[1] = (pl >> 8) & 0xFF;
                send_data[2] = (pl >> 16) & 0xFF;
            }
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

            // Auth Switch may happen multiple rounds under TLS; loop until terminal packet.
            for (int auth_round = 0; auth_round < 4; ++auth_round)
            {
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

                unsigned char pkt_type = (unsigned char)_cache_data[4];

                // Auth More Data (0x01)
                if (pkt_type == 0x01 && n >= 6)
                {
                    unsigned char subtype  = (unsigned char)_cache_data[5];
                    unsigned char recv_seq = (unsigned char)(_cache_data[3] & 0xFF);
                    seq_next_id            = recv_seq + 1;

                    if (subtype == 0x04)
                    {
                        // Public key request under TLS: send plaintext password
                        // (channel is already encrypted by TLS, no RSA needed)
                        send_data.clear();
                        send_data.push_back(0x00);
                        send_data.push_back(0x00);
                        send_data.push_back(0x00);
                        send_data.push_back(seq_next_id);
                        send_data.append(conn_config.password);
                        send_data.push_back(0x00);
                        auto pl = send_data.size() - 4;
                        if (pl >= 0xFFFFFF)
                        {
                            error_msg  = "auth packet payload too large: " + std::to_string(pl);
                            error_code = 1;
                            isclose    = true;
                            co_return false;
                        }
                        send_data[0] = (pl) & 0xFF;
                        send_data[1] = (pl >> 8) & 0xFF;
                        send_data[2] = (pl >> 16) & 0xFF;
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
                        continue;
                    }
                    else if (subtype == 0x03)
                    {
                        // subtype 0x03: fast auth success（缓存命中），客户端不应答，
                        // 终态 OK/Error 可能合并在同一读缓冲内
                        unsigned int pack_length = _cache_data[0] | (_cache_data[1] << 8) | (_cache_data[2] << 16);
                        unsigned int offset      = pack_length + 4;
                        if (n >= offset + 5)
                        {
                            if ((unsigned char)_cache_data[offset + 4] == 0xFF)
                            {
                                parse_error_packet(_cache_data + offset, n - offset);
                                isclose = true;
                                co_return false;
                            }
                            if ((unsigned char)_cache_data[offset + 4] == 0x00)
                            {
                                seq_next_id = static_cast<unsigned char>((_cache_data[offset + 3] & 0xFF) + 1);
                                co_return true;
                            }
                        }
                        continue;
                    }
                    else
                    {
                        error_msg  = " unknown auth more data subtype: " + std::to_string(subtype);
                        error_code = 2;
                        isclose    = true;
                        co_return false;
                    }
                }
                // Auth Switch Request (0xFE)
                else if (pkt_type == 0xFE)
                {
                    unsigned int off = 5;
                    std::string sw_plugin;
                    while (off < n && _cache_data[off] != 0)
                    {
                        sw_plugin.push_back(_cache_data[off]);
                        off++;
                    }
                    if (off < n)
                    {
                        off++;
                    }
                    std::string sw_salt;
                    while (off < n)
                    {
                        sw_salt.push_back(_cache_data[off]);
                        off++;
                    }
                    while (!sw_salt.empty() && sw_salt.back() == '\0')
                        sw_salt.pop_back();

                    server_hello.auth_plugin_salt_data = sw_salt;
                    server_hello.auth_plugin_name      = sw_plugin;
                    std::string auth_resp              = compute_auth_response(sw_plugin, sw_salt, conn_config.password);
                    if (auth_resp.empty() && !conn_config.password.empty())
                    {
                        error_code = 2;
                        isclose    = true;
                        co_return false;
                    }

                    unsigned char sw_seq = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
                    send_data.clear();
                    send_data.push_back(0x00);
                    send_data.push_back(0x00);
                    send_data.push_back(0x00);
                    send_data.push_back(sw_seq);
                    send_data.append(auth_resp);
                    auto sw_len  = auth_resp.size();
                    send_data[0] = static_cast<char>(sw_len & 0xFF);
                    send_data[1] = static_cast<char>((sw_len >> 8) & 0xFF);
                    send_data[2] = static_cast<char>((sw_len >> 16) & 0xFF);
                    seq_next_id  = sw_seq;

                    try
                    {
                        n = co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), asio::use_awaitable);
                    }
                    catch (const std::exception &e)
                    {
                        error_msg.append(e.what());
                        error_code = 2;
                        isclose    = true;
                        co_return false;
                    }
                    continue;
                }
                // Error packet
                else if (pkt_type == 0xFF)
                {
                    parse_error_packet(_cache_data, n);
                    isclose = true;
                    co_return false;
                }
                // OK / EOF packet — auth success
                else if (pkt_type == 0x00 || pkt_type == 0xFE)
                {
                    seq_next_id = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
                    co_return true;
                }
                // Multi-result OK packet (type 0x03)
                else if (pkt_type == 0x03)
                {
                    seq_next_id = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
                    co_return true;
                }
                else
                {
                    error_msg  = " unexpected auth response pkt_type: " + std::to_string(pkt_type);
                    error_code = 8;
                    isclose    = true;
                    co_return false;
                }
            }
            error_msg  = " auth handshake exceeded max rounds ";
            error_code = 8;
            isclose    = true;
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
    // Auth Switch 可能多轮挑战-应答：循环读取直到终态包（协程下以循环替代 goto）
    for (int auth_round = 0; auth_round < 4; ++auth_round)
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
                unsigned int ec      = (unsigned char)_cache_data[5] | ((unsigned char)_cache_data[6] << 8);
                error_code           = ec;
                unsigned int msg_off = 4 + 1 + 2 + 1 + 5;
                error_msg            = "MySQL auth error: ";
                if (n > msg_off)
                    error_msg.append(reinterpret_cast<const char *>(_cache_data + msg_off), n - msg_off);
                isclose = true;
                co_return false;
            }
            if (pkt_type == 0xFE || pkt_type == 0x00)
            {
                seq_next_id = (_cache_data[3] & 0xFF) + 1;
                co_return true;
            }

            seq_next_id = (_cache_data[3] & 0xFF) + 1;

            // 下发公钥先做钉住校验（配置 server_public_key 才生效），防中间人换公钥骗密码。
            // 注意 sock_type==2（SSL）不会走到这里：SSL 分支在上方直接回明文密码并已返回。
            if (!check_server_public_key(conn_config, &_cache_data[5], n - 5))
            {
                isclose = true;
                co_return false;
            }

            bool isok = server_public_key_encrypt(conn_config.password, &_cache_data[5], n - 5);
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
            // Auth Switch Request: [0xFE][plugin][NUL][salt]
            unsigned int off = 5;
            std::string sw_plugin;
            while (off < n && _cache_data[off] != 0)
            {
                sw_plugin.push_back(_cache_data[off]);
                off++;
            }
            if (off < n)
            {
                off++;
            }
            std::string sw_salt;
            while (off < n)
            {
                sw_salt.push_back(_cache_data[off]);
                off++;
            }
            while (!sw_salt.empty() && sw_salt.back() == '\0')
            {
                sw_salt.pop_back();
            }

            server_hello.auth_plugin_salt_data = sw_salt;
            server_hello.auth_plugin_name      = sw_plugin;
            std::string auth_resp              = compute_auth_response(sw_plugin, sw_salt, conn_config.password);
            if (auth_resp.empty() && !conn_config.password.empty())
            {
                error_code = 2;
                isclose    = true;
                co_return false;
            }

            unsigned char sw_seq = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
            send_data.clear();
            send_data.push_back(0x00);
            send_data.push_back(0x00);
            send_data.push_back(0x00);
            send_data.push_back(sw_seq);
            send_data.append(auth_resp);
            auto sw_len  = auth_resp.size();
            send_data[0] = static_cast<char>(sw_len & 0xFF);
            send_data[1] = static_cast<char>((sw_len >> 8) & 0xFF);
            send_data[2] = static_cast<char>((sw_len >> 16) & 0xFF);
            seq_next_id  = sw_seq;

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
                error_code = 2;
                isclose    = true;
                co_return false;
            }
            continue;
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
        else if ((unsigned char)_cache_data[4] == 0x00)
        {
            seq_next_id = static_cast<unsigned char>((_cache_data[3] & 0xFF) + 1);
            co_return true;
        }
        else if ((unsigned char)_cache_data[5] == 0x03)
        {
            // fast-auth success（缓存命中）：服务器发 [01 03] 后紧跟终态 OK/ERR 包，
            // 两包可能合并到达或分片到达。此处循环补读，读满第二个整包后再判终态：
            //   1) 不用脆弱的 n>=offset+len+5 阈值（曾 off-by-one：整包只需 +4，误判为未完整）
            //   2) 不 memset 从头重读（会丢弃已到达字节，并挂起等待服务器永不再发的数据）
            unsigned int offset = static_cast<unsigned char>(_cache_data[0]) | (static_cast<unsigned char>(_cache_data[1]) << 8) | (static_cast<unsigned char>(_cache_data[2]) << 16);
            offset += 4;// 第一个包([01 03])总长 = payload + 4 字节包头

            // 读满第二个包的 4 字节包头(payload_len[3] + seq[1])
            while (n < offset + 4)
            {
                if (n >= CACHE_DATA_LENGTH)
                {
                    error_msg  = " connect fail! fast-auth packet overflow ";
                    error_code = 8;
                    isclose    = true;
                    co_return false;
                }
                std::size_t got = 0;
                try
                {
                    if (conn_link->sock_type == 3)
                    {
                        got = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n), asio::use_awaitable);
                    }
                    else
                    {
                        got = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n), asio::use_awaitable);
                    }
                }
                catch (const std::exception &e)
                {
                    error_msg.append(e.what());
                    isclose = true;
                    co_return false;
                }
                if (got == 0)
                {
                    error_msg  = " connect fail! fast-auth read closed ";
                    error_code = 1;
                    isclose    = true;
                    co_return false;
                }
                n += got;
            }

            // 解析第二个包 payload 长度，循环补读至整包到达
            unsigned int term_len = static_cast<unsigned char>(_cache_data[offset]) | (static_cast<unsigned char>(_cache_data[offset + 1]) << 8) | (static_cast<unsigned char>(_cache_data[offset + 2]) << 16);
            unsigned int need     = offset + 4 + term_len;
            while (n < need)
            {
                if (n >= CACHE_DATA_LENGTH)
                {
                    error_msg  = " connect fail! fast-auth packet overflow ";
                    error_code = 8;
                    isclose    = true;
                    co_return false;
                }
                std::size_t got = 0;
                try
                {
                    if (conn_link->sock_type == 3)
                    {
                        got = co_await conn_link->localsocket->async_read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n), asio::use_awaitable);
                    }
                    else
                    {
                        got = co_await conn_link->socket->async_read_some(asio::buffer(_cache_data + n, CACHE_DATA_LENGTH - n), asio::use_awaitable);
                    }
                }
                catch (const std::exception &e)
                {
                    error_msg.append(e.what());
                    isclose = true;
                    co_return false;
                }
                if (got == 0)
                {
                    error_msg  = " connect fail! fast-auth read closed ";
                    error_code = 1;
                    isclose    = true;
                    co_return false;
                }
                n += got;
            }

            // 判定终态包类型（第二个包 payload 首字节）
            unsigned char final_type = static_cast<unsigned char>(_cache_data[offset + 4]);
            if (final_type == 0xFF)
            {
                parse_error_packet(_cache_data + offset, n - offset);
                isclose = true;
                co_return false;
            }
            if (final_type == 0x00)
            {
                seq_next_id = static_cast<unsigned char>((_cache_data[offset + 3] & 0xFF) + 1);
                co_return true;
            }
            error_msg  = " connect fail! unexpected fast-auth final packet ";
            error_code = 8;
            isclose    = true;
            co_return false;
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

    error_msg  = " auth switch rounds exhausted ";
    error_code = 9;
    isclose    = true;
    co_return false;
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
    // n >= 1 都是有效数据（允许半包），累积式调用方会循环补足
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
        if (n == 0)
        {
            error_code = 1;
            error_msg  = "connection closed by peer";
            return 0;
        }
        // n >= 1 都是有效数据（允许半包）：
        //   TCP read_some 在慢网/大包分段时可能返回 1-4 字节的半包，
        //   累积式调用方（如 fetch_prepared_impl 的 read_more）会循环补足；
        //   直接解析调用方（如 connect 握手）有自己的协议级长度校验（n<70 等）。
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

    // MySQL 协议包长度字段为 3 字节，上限 0xFFFFFF（16MB-1）。
    // 超过即拒绝发送（未实现 >16MB 分片，直接报错避免长度溢出导致协议失步）
    if (n >= 0xFFFFFF)
    {
        error_msg  = "write_sql: SQL too large (>16MB), cannot send in single packet: " + std::to_string(sql.length());
        error_code = 20;
        isclose    = true;
        return 0;
    }

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

    // 与同步版一致：超过 16MB 拒绝发送，避免 3 字节长度字段溢出
    if (n >= 0xFFFFFF)
    {
        error_msg  = "async_write_sql: SQL too large (>16MB), cannot send in single packet: " + std::to_string(sql.length());
        error_code = 20;
        isclose    = true;
        co_return 0;
    }

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
    char send_buf[5]  = {0x01, 0x00, 0x00, 0x00, 0x0E};
    char recv_buf[16] = {0};
    error_code        = 0;
    error_msg.clear();
    try
    {
        if (isclose)
        {
            return false;
        }

        auto *sock = (conn_link->sock_type == 1) ? static_cast<void *>(conn_link->socket.get()) : (conn_link->sock_type == 2) ? static_cast<void *>(conn_link->sslsocket.get()) :
                                                                                              (conn_link->sock_type == 3)     ? static_cast<void *>(conn_link->localsocket.get()) :
                                                                                                                                nullptr;
        if (!sock)
        {
            error_code = 3;
            error_msg  = "ping: no socket";
            return false;
        }

        auto do_write = [&](auto &stream)
        {
            asio::write(stream, asio::buffer(send_buf, 5), conn_link->ec);
            return !conn_link->ec;
        };
        auto do_read_full = [&](auto &stream)
        {
            // 先读 4 字节包头拿 payload 长度，再读完整 payload，避免半包截断
            char header[4] = {0};
            size_t n       = asio::read(stream, asio::buffer(header, 4), conn_link->ec);
            if (conn_link->ec || n < 4)
                return false;
            size_t payload_len = (static_cast<unsigned char>(header[0])) |
                                 (static_cast<unsigned char>(header[1]) << 8) |
                                 (static_cast<unsigned char>(header[2]) << 16);
            if (payload_len > sizeof(recv_buf))
                return false;
            if (payload_len > 0)
            {
                n = asio::read(stream, asio::buffer(recv_buf, payload_len), conn_link->ec);
                if (conn_link->ec || n < payload_len)
                    return false;
            }
            // OK 判定：payload 至少 1 字节且首字节为 0x00；ERR 为 0xFF
            if (payload_len == 0)
                return false;
            unsigned char status = static_cast<unsigned char>(recv_buf[0]);
            if (status == 0xFF && payload_len >= 2)
            {
                error_code = static_cast<unsigned char>(recv_buf[1]) | (static_cast<unsigned char>(recv_buf[2]) << 8);
                isclose    = true;
                return false;
            }
            return true;
        };

        bool ok = false;
        if (conn_link->sock_type == 1)
        {
            ok = do_write(*conn_link->socket);
            if (ok)
                ok = do_read_full(*conn_link->socket);
        }
        else if (conn_link->sock_type == 2)
        {
            ok = do_write(*conn_link->sslsocket);
            if (ok)
                ok = do_read_full(*conn_link->sslsocket);
        }
        else
        {
            ok = do_write(*conn_link->localsocket);
            if (ok)
                ok = do_read_full(*conn_link->localsocket);
        }

        if (!ok)
        {
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
    stmt_cache_.clear([this](uint32_t id)
                      { stmt_close_impl(id); });
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
    stmt_cache_.clear([this](uint32_t id)
                      { stmt_close_impl(id); });
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

    constexpr unsigned int kPktHeader     = 4;// 3-byte length + 1-byte seq_id
    constexpr unsigned int kErrorMinTotal = 7;// header + status(0xFF) + errcode(2)

    if (pkt_len < kErrorMinTotal)
    {
        error_msg  = "MySQL error packet truncated (< 7 bytes)";
        error_code = 255;
        return;
    }

    // 从 error_code 字段开始解析 (跳过 packet header + status byte)
    unsigned int offset = kPktHeader + 1;// = 5

    // ---- error_code (2 bytes) ----
    error_code = data[offset] | (data[offset + 1] << 8);
    offset += 2;// offset = 7

    // ---- SQL state: '#' + 5 bytes state ----
    if (offset < pkt_len && data[offset] == 0x23)
    {
        if (offset + 6 <= pkt_len)
        {
            offset += 6;// skip '#'(1) + SQL state(5)
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

namespace
{

void _le_write(std::vector<unsigned char> &out, uint64_t v, int bytes)
{
    for (int i = 0; i < bytes; ++i)
    {
        out.push_back(static_cast<unsigned char>((v >> (i * 8)) & 0xFF));
    }
}

void _le_write(std::string &out, uint64_t v, int bytes)
{
    for (int i = 0; i < bytes; ++i)
    {
        out.push_back(static_cast<char>((v >> (i * 8)) & 0xFF));
    }
}

void _le_read(const unsigned char *data, int offset, void *dst, int bytes)
{
    unsigned char *p = static_cast<unsigned char *>(dst);
    for (int i = 0; i < bytes; ++i)
    {
        p[i] = data[offset + i];
    }
}

// varint 编码（MySQL 长度编码整数）
void _write_varint(std::vector<unsigned char> &out, uint64_t v)
{
    if (v < 251)
    {
        out.push_back(static_cast<unsigned char>(v));
    }
    else if (v < (1ULL << 16))
    {
        out.push_back(0xFC);
        _le_write(out, v, 2);
    }
    else if (v < (1ULL << 24))
    {
        out.push_back(0xFD);
        _le_write(out, v, 3);
    }
    else
    {
        out.push_back(0xFE);
        _le_write(out, v, 8);
    }
}

// 解码 COM_STMT_EXECUTE binary RowData 中的一个列值。
// buf: 行 packet body 起点(0x00 header 之后), buf_len: body 长度,
// off: 当前列值起始偏移(成功解码后前进), tc: MYSQL_TYPE 类型码,
// out: 该列的字符串表示(NULL 列不进本函数, 由调用方用 null_bitmap 判断)。
// 返回 false: 数据不足或无法解码(上层应停止解析本行, 保留已解出的列)。
//
// MySQL Binary Protocol 列值编码:
//   - 整数: TINY=1B / SHORT·YEAR=2B / LONG·INT24=4B / LONGLONG=8B, 小端
//   - FLOAT=4B / DOUBLE=8B (IEEE754 小端)
//   - DATE/DATETIME/TIMESTAMP: 1B length 前缀(0/4/7/11) + 字段
//       len=0: 零日期; len=4: year_le(2)+month+day;
//       len=7: +hour+min+sec; len=11: +microsecond_le(4)
//   - TIME: 1B length 前缀(0/8/12) + 字段
//       len=0: 零时间; len=8: is_negative(1)+days_le(4)+hour+min+sec;
//       len=12: +microsecond_le(4)
//   - 其余变长类型(STRING/VARCHAR/VAR_STRING/BLOB/DECIMAL/BIT/JSON...):
//     lenenc 长度前缀 + 内容

// 读取 MySQL length-encoded integer, 返回 std::pair<值, 消费字节数>
// MySQL 规范:
//   0x00-0xFA: 1B 直接值 (0-250)
//   0xFB (251): NULL marker (不是 varint, 返回 {0, 0} 让调用方识别)
//   0xFC (252): 3B total (1 header + 2B LE value)
//   0xFD (253): 4B total (1 header + 3B LE value)
//   0xFE (254): 9B total (1 header + 8B LE value)
//   0xFF (255): 无效
std::pair<uint64_t, int> _read_varint(const unsigned char *data, int offset)
{
    unsigned char b = data[offset];
    if (b < 251)
        return {b, 1};
    if (b == 0xFB)
        return {0, 0};// NULL marker, 不消费字节
    if (b == 0xFC)
    {
        uint64_t v = 0;
        _le_read(data, offset + 1, &v, 2);
        return {v, 3};
    }
    if (b == 0xFD)
    {
        uint64_t v = 0;
        _le_read(data, offset + 1, &v, 3);
        return {v, 4};
    }
    if (b == 0xFE)
    {
        uint64_t v = 0;
        _le_read(data, offset + 1, &v, 8);
        return {v, 9};
    }
    // 0xFF: 无效
    return {0, 0};
}

// 解析一个 Column Definition 4.1 packet payload 的 type 字段(供 sync/async prepare 复用)。
// payload/plen 为单包 payload(不含 4B 包头); 返回 false 表示包体损坏/越界。

// DOUBLE/FLOAT 最短往返文本化: 用 std::to_chars(general) 输出能 round-trip 的
// 最短表示, 避免 std::to_string 固定 6 位小数造成量级/精度丢失
// (如 1e-10 → "0.000000", 1/3 → "0.333333")。FLOAT 按 float 原类型输出。
template <typename T>
std::string _fmt_double(T v)
{
    if (v == 0)
        return "0";
    if (std::isnan(v))
        return "nan";
    if (std::isinf(v))
        return (v < 0) ? "-inf" : "inf";
    char tmp[64];
    auto res = std::to_chars(tmp, tmp + sizeof(tmp), v, std::chars_format::general);
    if (res.ec == std::errc())
        return std::string(tmp, static_cast<size_t>(res.ptr - tmp));
    std::snprintf(tmp, sizeof(tmp), "%.17g", (double)v);// 兜底(几乎不会走到)
    return tmp;
}

// binary 载荷里那 4 字节分数秒实测恒为微秒（DATETIME(3) 的 .123 传 123000），
// 而服务端文本按列的 decimals 打印，故须先缩放再按该宽度输出，否则位数对不上。
void _append_frac_seconds(std::string &out, uint8_t decimals, uint32_t micro)
{
    unsigned int d = (decimals > 6) ? 6u : static_cast<unsigned int>(decimals);
    if (d == 0)
        return;
    static const uint32_t divs[7] = {1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u};
    char tmp[16];
    std::snprintf(tmp, sizeof(tmp), ".%0*u", d, micro / divs[d]);
    out += tmp;
}

bool _decode_binary_value(const unsigned char *buf, int buf_len, int &off, uint8_t tc, bool uflag, uint8_t decimals, std::string &out)
{
    out.clear();
    char tmp[48];
    switch (tc)
    {
    case 0x01:// MYSQL_TYPE_TINY
        if (off + 1 > buf_len)
            return false;
        if (uflag)
            out = std::to_string((unsigned int)buf[off]);
        else
            out = std::to_string((int)(int8_t)buf[off]);
        off += 1;
        return true;
    case 0x02:// MYSQL_TYPE_SHORT
    case 0x0D:// MYSQL_TYPE_YEAR
        if (off + 2 > buf_len)
            return false;
        {
            unsigned int raw = (unsigned int)(buf[off] | (uint16_t)(buf[off + 1] << 8));
            if (uflag || tc == 0x0D)// YEAR 语义恒为无符号
                out = std::to_string(raw);
            else
                out = std::to_string((int)(int16_t)raw);
        }
        off += 2;
        return true;
    case 0x03:// MYSQL_TYPE_LONG
    case 0x09:// MYSQL_TYPE_INT24 — binary 协议中按 4B LE 传输
        if (off + 4 > buf_len)
            return false;
        if (uflag)
        {
            uint32_t v = 0;
            std::memcpy(&v, buf + off, 4);
            out = std::to_string((unsigned long long)v);
        }
        else
        {
            int32_t v = 0;
            std::memcpy(&v, buf + off, 4);
            out = std::to_string((long long)v);
        }
        off += 4;
        return true;
    case 0x08:// MYSQL_TYPE_LONGLONG
        if (off + 8 > buf_len)
            return false;
        if (uflag)
        {
            uint64_t v = 0;
            std::memcpy(&v, buf + off, 8);
            out = std::to_string(v);
        }
        else
        {
            int64_t v = 0;
            std::memcpy(&v, buf + off, 8);
            out = std::to_string(v);
        }
        off += 8;
        return true;
    case 0x04:// MYSQL_TYPE_FLOAT
        if (off + 4 > buf_len)
            return false;
        {
            float f = 0;
            std::memcpy(&f, buf + off, 4);
            out = _fmt_double(f);
        }
        off += 4;
        return true;
    case 0x05:// MYSQL_TYPE_DOUBLE
        if (off + 8 > buf_len)
            return false;
        {
            double f = 0;
            std::memcpy(&f, buf + off, 8);
            out = _fmt_double(f);
        }
        off += 8;
        return true;
    case 0x06:// MYSQL_TYPE_NULL — 不进 values 段, 安全兜底
        return true;
    case 0x07:// MYSQL_TYPE_TIMESTAMP
    case 0x0A:// MYSQL_TYPE_DATE
    case 0x0C:// MYSQL_TYPE_DATETIME
    {
        if (off + 1 > buf_len)
            return false;
        unsigned char tlen = buf[off];
        if (tlen == 0)
        {
            off += 1;
            if (tc == 0x0A)
                out = "0000-00-00";
            else
            {
                out = "0000-00-00 00:00:00";
                _append_frac_seconds(out, decimals, 0);
            }
            return true;// 零日期/零时间: 服务端文本协议打印全零字面量
        }
        if (tlen == 4)// 仅日期
        {
            // TIMESTAMP/DATETIME 时间部分为 00:00:00 时, MySQL 只发日期段(len=4),
            // 需按列类型补全时间, 保证 round-trip 得到 'YYYY-MM-DD 00:00:00'
            if (off + 5 > buf_len)
                return false;
            uint16_t year = (uint16_t)(buf[off + 1] | (uint16_t)(buf[off + 2] << 8));
            if (tc == 0x07 || tc == 0x0C)
                std::snprintf(tmp, sizeof(tmp), "%04u-%02u-%02u 00:00:00", year, buf[off + 3], buf[off + 4]);
            else
                std::snprintf(tmp, sizeof(tmp), "%04u-%02u-%02u", year, buf[off + 3], buf[off + 4]);
            out = tmp;
            _append_frac_seconds(out, decimals, 0);
            off += 5;
            return true;
        }
        if (tlen == 7 || tlen == 11)// 日期+时间 (+微秒)
        {
            if (off + 8 > buf_len)
                return false;
            uint16_t year = (uint16_t)(buf[off + 1] | (uint16_t)(buf[off + 2] << 8));
            std::snprintf(tmp, sizeof(tmp), "%04u-%02u-%02u %02u:%02u:%02u", year, buf[off + 3], buf[off + 4], buf[off + 5], buf[off + 6], buf[off + 7]);
            out = tmp;
            off += 8;
            uint32_t micro = 0;
            if (tlen == 11)
            {
                if (off + 4 > buf_len)
                    return false;
                std::memcpy(&micro, buf + off, 4);
                off += 4;
            }
            _append_frac_seconds(out, decimals, micro);
            return true;
        }
        return false;// 非法 length 前缀
    }
    case 0x0B:// MYSQL_TYPE_TIME
    {
        if (off + 1 > buf_len)
            return false;
        unsigned char tlen = buf[off];
        if (tlen == 0)
        {
            off += 1;
            out = "00:00:00";
            _append_frac_seconds(out, decimals, 0);
            return true;
        }
        if (tlen == 8 || tlen == 12)
        {
            if (off + 9 > buf_len)
                return false;
            bool neg      = buf[off + 1] != 0;
            uint32_t days = 0;
            std::memcpy(&days, buf + off + 2, 4);
            uint32_t hh = days * 24 + buf[off + 6];
            std::snprintf(tmp, sizeof(tmp), "%s%02u:%02u:%02u", neg ? "-" : "", hh, buf[off + 7], buf[off + 8]);
            out = tmp;
            off += 9;
            uint32_t micro = 0;
            if (tlen == 12)
            {
                if (off + 4 > buf_len)
                    return false;
                std::memcpy(&micro, buf + off, 4);
                off += 4;
            }
            _append_frac_seconds(out, decimals, micro);
            return true;
        }
        return false;
    }
    default:
        // STRING/VARCHAR/VAR_STRING/TINY_BLOB/MEDIUM_BLOB/BLOB/LONG_BLOB/
        // DECIMAL/NEWDECIMAL/BIT/ENUM/SET/JSON/GEOMETRY 等:
        // lenenc 长度前缀 + 内容
        {
            if (off + 1 > buf_len)
                return false;
            auto [vlen, vlen_size] = _read_varint(buf, off);
            if (vlen_size == 0)
                return false;
            // 守卫必须全程无符号且不截断：先把长度前缀本身扣掉，再比较剩余字节数
            if (vlen_size > buf_len - off)
                return false;
            if (vlen > static_cast<uint64_t>(buf_len - off - vlen_size))
                return false;
            out.assign(reinterpret_cast<const char *>(buf + off + vlen_size),
                       static_cast<size_t>(vlen));
            off += vlen_size + static_cast<int>(vlen);
            return true;
        }
    }
}

// Column Definition 4.1 布局:
//   catalog, schema, table, org_table, name, org_name (6 个 lenenc string)
//   filler(1B=0x00), charset(2B LE), column_length(4B LE), type(1B), ...
//   flags(2B LE), decimals(1B), reserved(2B)
// type 之前的累计偏移 = 6 个 lenenc string 之后固定 1+2+4=7 字节。
bool _parse_definition_type(const unsigned char *payload, unsigned int plen, uint8_t &type_code, bool &is_unsigned, uint8_t *decimals_out = nullptr)
{
    int off = 0;
    for (int i = 0; i < 6; ++i)// 跳过 6 个 lenenc strings
    {
        if (off >= static_cast<int>(plen))
            return false;
        auto [slen, slen_size] = _read_varint(payload, off);
        if (slen_size == 0)
            return false;
        if (slen > static_cast<uint64_t>(plen - static_cast<unsigned int>(off + slen_size)))
            return false;
        off += slen_size + static_cast<int>(slen);
    }
    off += 1 + 2 + 4;// filler + charset + column_length
    if (off + 3 > static_cast<int>(plen))
        return false;// type(1) + flags(2)
    type_code = payload[off];
    // ColumnDefinition41: type 后紧跟 flags(2B LE); UNSIGNED_FLAG = 0x20
    unsigned int flags = (unsigned int)(payload[off + 1] | (payload[off + 2] << 8));
    is_unsigned        = (flags & 0x20U) != 0;
    // decimals 在 flags 后 1 字节 (off+3); 预留 decimals+reserved=3B
    if (decimals_out)
    {
        if (off + 4 > static_cast<int>(plen))
            return false;
        *decimals_out = payload[off + 3];
    }
    return true;
}

// 解析 Column Definition 4.1 payload 的显示列名与原始(物理)列名。
// payload/plen 为单包 payload(不含 4B 包头); 返回 false 表示包体损坏/越界。
// 字段顺序(见 mysql_src sql/protocol_classic.cc):
//   catalog, schema, table, org_table, name, org_name, filler, charset, ...
// name   = 结果集列名: SELECT id AS myid → "myid"
// org_name = 底层表字段名: 同上 → "id"; 表达式/聚合列没有物理名, 恒为空
//            (见 mysql_src sql/item.cc: itemize_fields 只填 col_name)
bool _parse_definition_names(const unsigned char *payload, unsigned int plen, std::string &name, std::string &org_name)
{
    name.clear();
    org_name.clear();
    int off = 0;
    for (int i = 0; i < 6; ++i)
    {
        if (off >= static_cast<int>(plen))
            return false;
        unsigned char b0 = payload[off];
        int prefix       = (b0 < 0xFB) ? 1 : (b0 == 0xFC ? 3 : (b0 == 0xFD ? 4 : (b0 == 0xFE ? 9 : 0)));
        // 0xFB(NULL)/0xFF(无效) 不是列名字符串; 长度前缀本身也不允许跨过包尾,
        // 否则 _read_varint 会读出 payload 之外。
        if (prefix == 0 || off + prefix > static_cast<int>(plen))
            return false;
        auto [slen, slen_size] = _read_varint(payload, off);
        if (slen_size != prefix)
            return false;
        off += prefix;
        if (slen > static_cast<uint64_t>(plen - static_cast<unsigned int>(off)))
            return false;
        if (i == 4)
            name.assign(reinterpret_cast<const char *>(payload + off), static_cast<size_t>(slen));
        else if (i == 5)
            org_name.assign(reinterpret_cast<const char *>(payload + off), static_cast<size_t>(slen));
        off += static_cast<int>(slen);
    }
    return true;
}

// 热路径校验：本轮 Column Definition 包是否与缓存的列元数据逐项一致。
// 单趟遍历 6 个 lenenc 字符串，第 5/6 个(name/org_name)逐字节比对(不构造 std::string)，
// 再按 _parse_definition_type 同样的偏移读 type(1B)+flags(2B, UNSIGNED_FLAG=0x20)。
// 返回 false ⇒ 类型或列名已变：服务器 ALTER 后会自行 re-prepare(stmt_id 依旧有效,
// Prepared_stmt_count 不涨)，缓存侧收不到任何失效信号，只能靠每轮比对发现。
bool _definition_matches(const unsigned char *payload, unsigned int plen, const std::string &name, const std::string &org_name, uint8_t type, uint8_t uflag, uint8_t decimals)
{
    int off = 0;
    for (int i = 0; i < 6; ++i)
    {
        if (off >= static_cast<int>(plen))
            return false;
        auto [slen, slen_size] = _read_varint(payload, off);
        if (slen_size == 0)
            return false;
        if (slen > static_cast<uint64_t>(plen - static_cast<unsigned int>(off + slen_size)))
            return false;
        off += slen_size;
        if (i == 4 || i == 5)
        {
            const std::string &exp = (i == 4) ? name : org_name;
            if (slen != exp.size())
                return false;
            if (slen != 0 && memcmp(payload + off, exp.data(), static_cast<size_t>(slen)) != 0)
                return false;
        }
        off += static_cast<int>(slen);
    }
    off += 1 + 2 + 4;// next_length + charset(2) + column_length(4)
    if (off + 4 > static_cast<int>(plen))
        return false;
    if (payload[off] != type)
        return false;
    unsigned int flags = (unsigned int)(payload[off + 1] | (payload[off + 2] << 8));
    if (((flags & 0x20U) != 0) != (uflag != 0))
        return false;
    return payload[off + 3] == decimals;// 分数秒精度同样参与行解码
}

}// namespace
unsigned int mysql_conn_base::fetch_directly_impl(
    const std::string &sql,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    if (isclose)
    {
        return 0;
    }

    unsigned int retry = 0;
RETRY_LABEL:
    error_msg.clear();
    error_code      = 0;
    last_insert_id_ = 0;
    col_org_names_.clear();// 清空预编译路径残留，本次结果集重新填充

    unsigned int n = write_sql(sql);
    if (n == 0)
    {
        // write 失败 = SQL 还没发出去, 幂等安全, 可以重连
        if (retry < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
        {
            retry++;
            error_msg.clear();
            goto RETRY_LABEL;
        }
        return 0;
    }

    // accum_holder 放在子作用域里, 这样 goto RETRY_LABEL 时析构函数会被调用,
    // 防止累积多个 accum_holder 或数据串扰
    {
        pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
        auto &accum_buf       = accum_holder.buf;
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
        phase current_phase  = phase::INIT;
        bool stop            = false;
        unsigned int rows    = 0;
        bool handler_aborted = false;

        while (!stop)
        {
            n = read_loop();
            if (n == 0)
            {
                // write_sql 成功 = 那一刻连接是活的, 所以这里不再重连重发:
                // 重发会把已经交给 handler 的行再投一遍, 而真掉线时重连也救不回本次结果。
                // error_msg 里已经是 read_loop 写下的网络错误, 交给上层判定。
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
                        unsigned int ar  = parse_affected_rows_fast(&accum_buf[consumed], total_pkt_len, &iid);
                        if (ar != (unsigned int)-1)
                        {
                            last_insert_id_ = iid;
                        }
                        current_phase = phase::DONE;
                        stop          = true;
                        consumed += total_pkt_len;
                        break;
                    }
                    if (status >= 1 && status <= 250 && pkt_len <= 9)
                    {
                        // 列数包：经典格式为 1 字节；容忍 lenenc 变体（如服务端带
                        // metadata_follows 后缀的 2 字节形式），按 lenenc 取列数。
                        unsigned int cc_off   = consumed + 4;
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
                        // 0xFE 头 = 结果集终结包(OK-via-EOF)。服务器在列定义段之后是否再发一个
                        // 5B 经典 EOF 分隔包，只取决于客户端有没有协商 CLIENT_DEPRECATE_EOF
                        // (protocol_classic.cc end_result_metadata)；本库的连接期硬门禁已要求
                        // 服务器 advertise 该位 ⇒ 分隔包不存在，此位置的 0xFE 只可能是终结包。
                        // 把它当分隔符吃掉就会转 ROWS 去等永不再来的行包(0 行结果集挂死)。
                        if (expected_cols > 0 && col_names.size() >= expected_cols)
                        {
                            consumed += total_pkt_len;
                            current_phase = phase::DONE;
                            stop          = true;
                            break;
                        }
                        // 列定义未收齐即遇 0xFE → 空结果集/异常, 直接终止防阻塞。
                        current_phase = phase::DONE;
                        stop          = true;
                        consumed += total_pkt_len;
                        break;
                    }

                    // 列定义已收齐而本包不是终结包：转入 ROWS 且不消费，
                    // 让本包在 ROWS 阶段重新判定（兼作列数包解析失败时的兜底，
                    // 防止把行包当列定义误吃）。
                    if (expected_cols > 0 && col_names.size() >= expected_cols)
                    {
                        current_phase = phase::ROWS;
                        break;
                    }

                    unsigned int col_offset = consumed + 4;

                    std::string col_name, org_name;
                    // 文本协议 Column Definition: 复用预编译路径的共享解析器
                    // (同文件 sync/async 两处 fetch_directly_impl 都走这里)
                    if (_parse_definition_names(&accum_buf[col_offset], total_pkt_len - 4, col_name, org_name))
                    {
                        col_org_names_.push_back(std::move(org_name));
                    }
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

                    auto get_data = [&](int col_idx) -> std::tuple<unsigned char *, size_t>
                    {
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
                            // 以 row_end 为长度边界：防止行内最后字段的长度前缀
                            // 跨行读取下一行字节作为长度（M-4）
                            unsigned long long length = pack_real_num(accum_buf.data(), row_end, pos);
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
    // 跳过前导空白与 SQL 注释（-- 行注释、/* */ 块注释）后再取首个有效词，
    // 避免 "/* x */ BEGIN"、" -- 注释\nBEGIN" 等场景误判/漏判事务关键字
    size_t i = 0;
    while (i < sql.size())
    {
        unsigned char c = static_cast<unsigned char>(sql[i]);
        if (isspace(c))
        {
            i++;
            continue;
        }
        if (i + 1 < sql.size() && sql[i] == '-' && sql[i + 1] == '-')
        {
            i += 2;
            while (i < sql.size() && sql[i] != '\n')
                i++;
            continue;
        }
        if (i + 1 < sql.size() && sql[i] == '/' && sql[i + 1] == '*')
        {
            i += 2;
            while (i + 1 < sql.size() && !(sql[i] == '*' && sql[i + 1] == '/'))
                i++;
            i += 2;
            continue;
        }
        break;
    }
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
    error_code      = 0;
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

    // OK 包可能被 TCP 分段，单次 read_loop() 可能读到半包
    // 累积循环直到拿到完整的包头(4字节) + payload
    // accum_holder 放在子作用域里, 这样 goto RETRY_LABEL 时析构函数会被调用,
    // 防止累积多个 accum_holder 或数据串扰
    {
        n = read_loop();
        pooled_accum_buf accum_holder;
        auto &accum_buf = accum_holder.buf;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
        while (accum_buf.size() < 4)
        {
            n = read_loop();
            if (n == 0)
                break;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
        }
        unsigned int pkt_len = 0;
        if (accum_buf.size() >= 4)
        {
            pkt_len = accum_buf[0] | (accum_buf[1] << 8) | (accum_buf[2] << 16);
        }
        while (accum_buf.size() < 4 + pkt_len)
        {
            n = read_loop();
            if (n == 0)
                break;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
        }

        unsigned int iid     = 0;
        unsigned int result  = (unsigned int)-1;
        unsigned char status = (accum_buf.size() >= 5) ? accum_buf[4] : 0x00;
        if (status == 0xFF)
        {
            // 服务器错误包; parse_error_packet 约定 data 指向包头起点(含 4B header), len 含包头
            if (accum_buf.size() >= 4 + pkt_len)
                parse_error_packet(&accum_buf[0], pkt_len + 4);
        }
        else if (status == 0x00)
        {
            // OK packet = 真正 DML 的最终响应
            result = parse_affected_rows_fast(accum_buf.data(), accum_buf.size(), &iid);
            if (result != (unsigned int)-1)
            {
                last_insert_id_ = iid;
                txn_keyword kw  = detect_txn_keyword(sql);
                if (kw == txn_keyword::begin)
                    in_transaction_.store(true);
                else if (kw == txn_keyword::end)
                    in_transaction_.store(false);
            }
        }
        else
        {
            // 首个响应既非 OK 也非 ERR = column count, 语句实际返回了结果集(把 SELECT
            // 误用成 exec_dml)。显式报错并排空整个结果集到终结包, 避免残留字节让下一条
            // 命令错位 → 永久卡读(与 exec_dml_prepared_impl 同因同修)。
            error_msg  = "exec_dml: statement returned a result set (not DML)";
            error_code = 10;

            auto drain_read = [&]() -> bool
            {
                unsigned int k = read_loop();
                if (k == 0)
                    return false;
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                return true;
            };
            // 跳过「一个逻辑包」(列定义/行), 处理 >0xFFFFFF 的满块续包链。
            // 返回 {逻辑包首包 payload 首字节, 逻辑包起始偏移}; 读失败返回 {-1,0}。
            auto skip_logical = [&](size_t &cur) -> std::pair<int, size_t>
            {
                size_t start = cur;
                auto ensure  = [&](size_t need) -> bool
                {
                    while (accum_buf.size() - cur < need)
                        if (!drain_read())
                            return false;
                    return true;
                };
                if (!ensure(4))
                    return {-1, 0};
                unsigned int plen = accum_buf[cur] | (accum_buf[cur + 1] << 8) |
                                    (accum_buf[cur + 2] << 16);
                if (!ensure(4 + plen))
                    return {-1, 0};
                unsigned char first = accum_buf[cur + 4];
                cur += 4 + plen;
                while (plen == 0xFFFFFFu)
                {
                    if (!ensure(4))
                        return {-1, 0};
                    plen = accum_buf[cur] | (accum_buf[cur + 1] << 8) |
                           (accum_buf[cur + 2] << 16);
                    if (!ensure(4 + plen))
                        return {-1, 0};
                    cur += 4 + plen;
                }
                return {first, start};
            };

            size_t cur       = 4 + pkt_len;// 首包(column count)已消费
            uint64_t col_cnt = 0;
            if (pkt_len >= 1)
            {
                unsigned char b0 = accum_buf[4];
                if (b0 < 0xFB)
                    col_cnt = b0;
                else if (b0 == 0xFC && pkt_len >= 3)
                    col_cnt = accum_buf[5] | (static_cast<unsigned int>(accum_buf[6]) << 8);
                else if (b0 == 0xFD && pkt_len >= 4)
                    col_cnt = accum_buf[5] | (static_cast<unsigned int>(accum_buf[6]) << 8) |
                              (static_cast<unsigned int>(accum_buf[7]) << 16);
                else if (b0 == 0xFE && pkt_len >= 9)
                {
                    for (int i = 0; i < 8; ++i)
                        col_cnt |= static_cast<uint64_t>(accum_buf[5 + i]) << (i * 8);
                }
            }

            bool drain_ok = true;
            // 1) column definition 包(col_cnt 个)
            for (uint64_t i = 0; i < col_cnt && drain_ok; ++i)
            {
                auto [f, start] = skip_logical(cur);
                if (f < 0)
                {
                    drain_ok = false;
                    break;
                }
                if (f == 0xFF)
                {
                    parse_error_packet(&accum_buf[start], static_cast<unsigned int>(cur - start));
                    drain_ok = false;
                    break;
                }
            }
            // 2) 行直到终结包(0xFE = OK-via-EOF, 0xFF = ERR)
            while (drain_ok)
            {
                auto [f, start] = skip_logical(cur);
                if (f < 0)
                {
                    drain_ok = false;
                    break;
                }
                if (f == 0xFF)
                {
                    parse_error_packet(&accum_buf[start], static_cast<unsigned int>(cur - start));
                    drain_ok = false;
                    break;
                }
                if (f == 0xFE)
                    break;// 结果集终结包
            }
        }

        // read 断连不重连：SQL 已经写进 socket, 服务端很可能已经执行完毕,
        // 重发一次会让 INSERT 落库两遍却仍返回成功。只保留 write 侧的幂等重试。
        return result;
    }
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

    unsigned int retry = 0;
RETRY_LABEL:
    error_msg.clear();
    error_code      = 0;
    last_insert_id_ = 0;
    col_org_names_.clear();// 清空预编译路径残留，本次结果集重新填充

    unsigned int n = co_await async_write_sql(sql);
    if (n == 0)
    {
        // write 失败 = SQL 还没发出去, 幂等安全, 可以重连
        if (retry < kMaxReconnect && is_last_error_reconnectable() &&
            co_await async_try_reconnect())
        {
            retry++;
            error_msg.clear();
            goto RETRY_LABEL;
        }
        co_return 0;
    }

    // accum_holder 放在子作用域里, 这样 goto RETRY_LABEL 时析构函数会被调用,
    // 防止累积多个 accum_holder 或数据串扰
    {
        pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
        auto &accum_buf       = accum_holder.buf;
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
        phase current_phase  = phase::INIT;
        bool stop            = false;
        unsigned int rows    = 0;
        bool handler_aborted = false;

        while (!stop)
        {
            n = co_await async_read_loop();
            if (n == 0)
            {
                // async_write_sql 成功 = 那一刻连接是活的, 所以这里不再重连重发:
                // 重发会把已经交给 handler 的行再投一遍, 而真掉线时重连也救不回本次结果。
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
                        unsigned int ar  = parse_affected_rows_fast(&accum_buf[consumed], total_pkt_len, &iid);
                        if (ar != (unsigned int)-1)
                        {
                            last_insert_id_ = iid;
                        }
                        current_phase = phase::DONE;
                        stop          = true;
                        consumed += total_pkt_len;
                        break;
                    }
                    if (status >= 1 && status <= 250 && pkt_len <= 9)
                    {
                        // 列数包：经典格式为 1 字节；容忍 lenenc 变体（如服务端带
                        // metadata_follows 后缀的 2 字节形式），按 lenenc 取列数。
                        unsigned int cc_off   = consumed + 4;
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
                        // 0xFE 头 = 结果集终结包(OK-via-EOF)。服务器在列定义段之后是否再发一个
                        // 5B 经典 EOF 分隔包，只取决于客户端有没有协商 CLIENT_DEPRECATE_EOF
                        // (protocol_classic.cc end_result_metadata)；本库的连接期硬门禁已要求
                        // 服务器 advertise 该位 ⇒ 分隔包不存在，此位置的 0xFE 只可能是终结包。
                        // 把它当分隔符吃掉就会转 ROWS 去等永不再来的行包(0 行结果集挂死)。
                        if (expected_cols > 0 && col_names.size() >= expected_cols)
                        {
                            consumed += total_pkt_len;
                            current_phase = phase::DONE;
                            stop          = true;
                            break;
                        }
                        // 列定义未收齐即遇 0xFE → 空结果集/异常, 直接终止防阻塞。
                        current_phase = phase::DONE;
                        stop          = true;
                        consumed += total_pkt_len;
                        break;
                    }

                    // 列定义已收齐而本包不是终结包：转入 ROWS 且不消费，重新判定。
                    if (expected_cols > 0 && col_names.size() >= expected_cols)
                    {
                        current_phase = phase::ROWS;
                        break;
                    }

                    unsigned int col_offset = consumed + 4;

                    std::string col_name, org_name;
                    // 文本协议 Column Definition: 复用预编译路径的共享解析器
                    // (同文件 sync/async 两处 fetch_directly_impl 都走这里)
                    if (_parse_definition_names(&accum_buf[col_offset], total_pkt_len - 4, col_name, org_name))
                    {
                        col_org_names_.push_back(std::move(org_name));
                    }
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

                    auto get_data = [&](int col_idx) -> std::tuple<unsigned char *, size_t>
                    {
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
                            // 以 row_end 为长度边界：防止行内最后字段的长度前缀
                            // 跨行读取下一行字节作为长度
                            unsigned long long length = pack_real_num(accum_buf.data(), row_end, pos);
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
}

asio::awaitable<unsigned int> mysql_conn_base::async_exec_dml(const std::string &sql)
{
    unsigned int retry = 0;
RETRY_LABEL:
    if (isclose)
    {
        co_return (unsigned int) - 1;
    }

    error_msg.clear();
    error_code      = 0;
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
        co_return (unsigned int) - 1;
    }

    // OK 包可能被 TCP 分段，单次 async_read_loop() 可能读到半包
    // accum_holder 放在子作用域里, 这样 goto RETRY_LABEL 时析构函数会被调用,
    // 防止累积多个 accum_holder 或数据串扰
    {
        n = co_await async_read_loop();
        pooled_accum_buf accum_holder;
        auto &accum_buf = accum_holder.buf;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
        while (accum_buf.size() < 4)
        {
            n = co_await async_read_loop();
            if (n == 0)
                break;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
        }
        unsigned int pkt_len = 0;
        if (accum_buf.size() >= 4)
        {
            pkt_len = accum_buf[0] | (accum_buf[1] << 8) | (accum_buf[2] << 16);
        }
        while (accum_buf.size() < 4 + pkt_len)
        {
            n = co_await async_read_loop();
            if (n == 0)
                break;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
        }

        unsigned int iid    = 0;
        unsigned int result = parse_affected_rows_fast(accum_buf.data(), accum_buf.size(), &iid);
        if (result == (unsigned int)-1)
        {
            // read 断连不重连：SQL 已经写进 socket, 服务端很可能已经执行完毕,
            // 重发一次会让 INSERT 落库两遍却仍返回成功。只保留 write 侧的幂等重试。
            co_return (unsigned int) - 1;
        }
        last_insert_id_ = iid;
        txn_keyword kw  = detect_txn_keyword(sql);
        if (kw == txn_keyword::begin)
            in_transaction_.store(true);
        else if (kw == txn_keyword::end)
            in_transaction_.store(false);
        co_return result;
    }
}

// ===== 断线自动重连实现 =====

bool mysql_conn_base::is_last_error_reconnectable() const
{
    if (!conn_link)
        return false;
    const auto &ec = conn_link->ec;
    if (!ec)
        return false;

    // 只认 asio 抽象错误码, 白名单策略
    return ec == asio::error::eof ||
           ec == asio::error::connection_reset ||
           ec == asio::error::timed_out ||
           ec == asio::error::broken_pipe ||
           ec == asio::error::not_connected;
}

// ===== COM_STMT 预编译协议实现 =====
// 先加一些匿名 namespace 里的小工具函数

// ===== COM_STMT_PREPARE =====
unsigned int mysql_conn_base::stmt_prepare_impl(const std::string &sql)
{
    unsigned int n = 0;

    // 构建 COM_STMT_PREPARE packet: 3B length + 1B seq + 0x16 + sql
    unsigned int payload_len = 1 + sql.size();
    if (payload_len >= 0xFFFFFF)
    {
        error_msg  = "COM_STMT_PREPARE: SQL too large";
        error_code = 20;
        return 0;
    }

    send_data.clear();
    send_data.push_back((payload_len & 0xFF));
    send_data.push_back((payload_len >> 8 & 0xFF));
    send_data.push_back((payload_len >> 16 & 0xFF));
    send_data.push_back(0x00);// seq
    send_data.push_back(0x16);// COM_STMT_PREPARE
    send_data.append(sql);

    n = write();
    if (n == 0)
    {
        // 21 = 纯客户端码: prepare 包一个字节都没写出去, 服务端必然没收到,
        // 上层据此判断"可以重连一次"(读阶段的失败不能重连, 见调用处)。
        error_code = 21;
        return 0;
    }

    // 读响应，累积到完整 packet
    pooled_accum_buf accum_holder;
    auto &accum_buf = accum_holder.buf;

    n = read_loop();
    accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

    while (accum_buf.size() < 4)
    {
        n = read_loop();
        if (n == 0)
            return 0;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }

    unsigned int pkt_len = accum_buf[0] | (accum_buf[1] << 8) | (accum_buf[2] << 16);
    while (accum_buf.size() < 4 + pkt_len)
    {
        n = read_loop();
        if (n == 0)
            return 0;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }

    unsigned char status = accum_buf[4];
    if (status == 0xFF)
    {
        // parse_error_packet 约定 data 指向包头起点(含 4B header), pkt_len 含包头
        parse_error_packet(&accum_buf[0], pkt_len + 4);
        return 0;
    }

    // COM_STMT_PREPARE OK packet: 0x00 + stmt_id(4B LE) + col_count(2B LE) + param_count(2B LE) + ...
    if (status != 0x00)
    {
        error_msg  = "COM_STMT_PREPARE: unexpected status " + std::to_string(status);
        error_code = 20;
        return 0;
    }

    if (4 + 9 > accum_buf.size())
    {
        error_msg  = "COM_STMT_PREPARE: truncated response";
        error_code = 20;
        return 0;
    }

    unsigned int stmt_id = 0;
    uint16_t col_count   = 0;
    uint16_t param_count = 0;
    _le_read(&accum_buf[5], 0, &stmt_id, 4);
    _le_read(&accum_buf[9], 0, &col_count, 2);
    _le_read(&accum_buf[11], 0, &param_count, 2);

    // ---- 消费 param/column definition packets ----
    // COM_STMT_PREPARE 响应段布局(官方协议文档 page_protocol_com_stmt_prepare):
    //   Parameter definition block: param_count × Column Definition 包
    //   Column definition block:    col_count × Column Definition 包
    // 连接期硬门禁已要求服务器 advertise CLIENT_DEPRECATE_EOF ⇒ 段尾/段间都没有
    // EOF/OK 分隔包。
    // 首包(OK)之后的每个 definition packet 都是完整 MySQL packet(3B len + 1B seq +
    // payload), 按包长度逐个消费, 与 TCP segment 是否合并无关。
    stmt_param_types_.clear();
    stmt_param_unsigned_.clear();
    stmt_col_types_.clear();
    stmt_col_unsigned_.clear();
    stmt_col_decimals_.clear();
    stmt_col_names_.clear();
    stmt_col_org_names_.clear();

    // 从 pos 取下一个完整 MySQL packet: 结果经 pkt_payload/pkt_plen 返回(注意指针
    // 随 accum_buf 扩容可能失效, 只在取包后立即使用, 不跨次持有)。
    const unsigned char *pkt_payload = nullptr;
    unsigned int pkt_plen            = 0;
    auto next_packet                 = [&](unsigned int &pos) -> bool
    {
        while (pos + 4 > accum_buf.size())
        {
            n = read_loop();
            if (n == 0)
                return false;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
        }
        unsigned int pl = accum_buf[pos] | (accum_buf[pos + 1] << 8) | (accum_buf[pos + 2] << 16);
        while (pos + 4 + pl > accum_buf.size())
        {
            n = read_loop();
            if (n == 0)
                return false;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
        }
        pkt_payload = &accum_buf[pos + 4];
        pkt_plen    = pl;
        pos += 4 + pl;
        return true;
    };

    // 消费 count 个 Column Definition 包, 提取 type / unsigned 标志。
    auto read_def_block = [&](unsigned int count, std::vector<uint8_t> &types, std::vector<uint8_t> &unsigned_flags, unsigned int &pos, std::vector<std::string> *out_names = nullptr, std::vector<std::string> *out_org_names = nullptr, std::vector<uint8_t> *out_decimals = nullptr) -> bool
    {
        for (unsigned int i = 0; i < count; ++i)
        {
            if (!next_packet(pos))
                return false;
            if (pkt_payload[0] == 0xFF)
            {
                parse_error_packet(pkt_payload - 4, pkt_plen + 4);
                return false;
            }
            if (pkt_payload[0] == 0xFE)
            {
                // def 包 payload 首字节必为 lenenc 长度(如 0x03 表示 catalog "def"),
                // 0xFE 只可能是段尾 EOF/OK 分隔包 → 计数与服务器不符(协议失步)
                error_msg  = "COM_STMT_PREPARE: unexpected EOF inside definition block";
                error_code = 20;
                return false;
            }
            uint8_t type_code = 0;
            bool is_unsigned  = false;
            uint8_t decimals  = 0;
            if (!_parse_definition_type(pkt_payload, pkt_plen, type_code, is_unsigned, &decimals))
            {
                error_msg  = "COM_STMT_PREPARE: malformed definition packet";
                error_code = 20;
                return false;
            }
            types.push_back(type_code);
            unsigned_flags.push_back(is_unsigned ? 1 : 0);
            if (out_decimals)
                out_decimals->push_back(decimals);
            // 可选：同时提取列名（只在 col block 时传 out_names/out_org_names）
            if (out_names || out_org_names)
            {
                std::string name, org_name;
                if (!_parse_definition_names(pkt_payload, pkt_plen, name, org_name))
                {
                    error_msg  = "COM_STMT_PREPARE: malformed definition names";
                    error_code = 20;
                    return false;
                }
                if (out_names)
                    out_names->push_back(std::move(name));
                if (out_org_names)
                    out_org_names->push_back(std::move(org_name));
            }
        }
        return true;
    };

    unsigned int pos = 4 + pkt_len;// 首包(COM_STMT_PREPARE_OK)之后
    if (param_count > 0)
    {
        if (!read_def_block(param_count, stmt_param_types_, stmt_param_unsigned_, pos))
            return 0;
    }
    if (col_count > 0)
    {
        if (!read_def_block(col_count, stmt_col_types_, stmt_col_unsigned_, pos, &stmt_col_names_, &stmt_col_org_names_, &stmt_col_decimals_))
            return 0;
    }

    return stmt_id;
}

// ===== COM_STMT_CLOSE =====
void mysql_conn_base::stmt_close_impl(unsigned int stmt_id)
{
    // 构建 COM_STMT_CLOSE packet: 3B length + 1B seq + 0x19 + stmt_id(4B LE)
    unsigned int payload_len = 1 + 4;

    send_data.clear();
    send_data.push_back((payload_len & 0xFF));
    send_data.push_back((payload_len >> 8 & 0xFF));
    send_data.push_back((payload_len >> 16 & 0xFF));
    send_data.push_back(0x00);
    send_data.push_back(0x19);
    _le_write(send_data, stmt_id, 4);

    // 忽略发送结果（闭环收尾）
    try
    {
        unsigned int n = 0;
        if (conn_link->sock_type == 1)
            n = asio::write(*conn_link->socket, asio::buffer(send_data), conn_link->ec);
        else if (conn_link->sock_type == 2)
            n = asio::write(*conn_link->sslsocket, asio::buffer(send_data), conn_link->ec);
        else if (conn_link->sock_type == 3)
            n = asio::write(*conn_link->localsocket, asio::buffer(send_data), conn_link->ec);
        (void)n;
    }
    catch (...)
    {
        // 忽略
    }
    // COM_STMT_CLOSE 服务器不响应
}

// ===== mysql_stmt_cache 极简 LRU =====
// find() 返回的指针指向 map 内部节点：持有期间不得再触发 upsert() 的新键插入、erase()
// 或 clear()（会淘汰/rehash ⇒ 指针失效）。当前 fetch_*_impl 满足这一约束：冷路径 find
// 之后紧跟的那一次 upsert 就是本条目自身，热路径刷新只走 update_meta（键已存在 ⇒ 不插入、
// 不淘汰，指针稳定）。
const mysql_conn_base::mysql_stmt_cache_entry_t *
mysql_conn_base::mysql_stmt_cache::find(const std::string &sql)
{
    auto it = map_.find(sql);
    if (it == map_.end())
        return nullptr;
    it->second.lru_seq = ++seq_;
    return &it->second;
}

mysql_conn_base::mysql_stmt_cache_entry_t &
mysql_conn_base::mysql_stmt_cache::upsert(const std::string &sql, uint32_t *closed_out)
{
    auto it = map_.find(sql);
    if (it != map_.end())
    {
        it->second.lru_seq = ++seq_;
        return it->second;
    }
    if (map_.size() >= cap_)
    {
        // 淘汰 lru_seq 最小项（容量 128，冷路径扫一次 ~100 ns）
        uint64_t min_seq = ~uint64_t(0);
        auto min_it      = map_.end();
        for (auto m = map_.begin(); m != map_.end(); ++m)
            if (m->second.lru_seq < min_seq)
            {
                min_seq = m->second.lru_seq;
                min_it  = m;
            }
        // stmt_id 是服务端句柄：只删 map 会让服务端预备语句按"会话内不同 SQL 条数"
        // 单调累积，直到撞上全局 max_prepared_stmt_count。带出去由调用方 close。
        if (min_it != map_.end() && closed_out)
            *closed_out = min_it->second.stmt_id;
        map_.erase(min_it);
    }
    auto [ins, _]       = map_.emplace(sql, mysql_stmt_cache_entry_t{});
    ins->second.lru_seq = ++seq_;
    return ins->second;
}

void mysql_conn_base::mysql_stmt_cache::erase(const std::string &sql, uint32_t *closed_out)
{
    auto it = map_.find(sql);
    if (it == map_.end())
        return;
    // 失败路径上这条服务端语句通常还活着（例如协议/表相关错误，连接并未断）
    if (closed_out)
        *closed_out = it->second.stmt_id;
    map_.erase(it);
}

bool mysql_conn_base::mysql_stmt_cache::update_meta(const std::string &sql,
                                                    std::vector<uint8_t> types,
                                                    std::vector<uint8_t> uns,
                                                    std::vector<uint8_t> decimals,
                                                    std::vector<std::string> names,
                                                    std::vector<std::string> orgs)
{
    auto it = map_.find(sql);
    if (it == map_.end())
        return false;
    it->second.col_types     = std::move(types);
    it->second.col_unsigned  = std::move(uns);
    it->second.col_decimals  = std::move(decimals);
    it->second.col_names     = std::move(names);
    it->second.col_org_names = std::move(orgs);
    it->second.lru_seq       = ++seq_;
    return true;
}

void mysql_conn_base::stmt_cache_drop(const std::string &sql)
{
    uint32_t gone = 0;
    stmt_cache_.erase(sql, &gone);
    if (gone != 0 && !isclose)
        stmt_close_impl(gone);
}

void mysql_conn_base::mysql_stmt_cache::clear(std::function<void(uint32_t)> closer)
{
    for (auto &kv : map_)
        closer(kv.second.stmt_id);
    map_.clear();
    seq_ = 0;
}

namespace
{

// MY-13 判定用：obj_val 运行时值落在 union 的哪一族（BOOL 与整数同族，构造时都写 lval）。
enum class stmt_param_kind
{
    OTHER,
    INTEGER,
    FLOAT
};

stmt_param_kind stmt_param_val_kind(const http::obj_val &v)
{
    switch (v.get_type())
    {
    case http::obj_type::BOOL:
    case http::obj_type::INT:
    case http::obj_type::LONG:
    case http::obj_type::UINT:
    case http::obj_type::ULONG: return stmt_param_kind::INTEGER;
    case http::obj_type::DOUBLE: return stmt_param_kind::FLOAT;
    default: return stmt_param_kind::OTHER;
    }
}

// COM_STMT_EXECUTE 值段中按整数/浮点 binary 编码的声明类型，与运行时值种类是否同族。
// 不同族时该分支读的是从未被这个 obj_val 写过的 union 成员：字符串参数读到栈垃圾，
// DOUBLE 参数绑 INT 列则把 IEEE 位模式当整数发 —— 两者都是静默错值而非报错。
bool stmt_param_binary_decl_matches(uint8_t declared, stmt_param_kind kind)
{
    switch (declared)
    {
    case 0x01:// TINY
    case 0x02:// SHORT
    case 0x03:// LONG
    case 0x08:// LONGLONG
    case 0x09:// INT24
        return kind == stmt_param_kind::INTEGER;
    case 0x04:// FLOAT
    case 0x05:// DOUBLE
        return kind == stmt_param_kind::FLOAT;
    default:// 其余声明类型走字符串/日期时间分支，读 str_，与运行时类型无关
        return true;
    }
}

}// namespace

// ===== write_stmt_execute_params =====
// MY-6 已知边界（仅文档化，不修）：参数类型基于 COM_STMT_PREPARE 时快照（param_types_src）。
// D-1 失效只刷新结果列（col_*），不刷新参数类型；若 prepare 后 ALTER 改了列类型，param_types_src
// 是旧快照。当前 MySQL 对参数类型除 YEAR(0x0D)/BIT(0x10) 外不严格（隐式转换），且下方类型段循环
// 已对 YEAR/BIT 做 VAR_STRING 特例，真正越界概率极低、不影响功能。若需绝对正确：prepare 后 ALTER
// 列类型时重新 prepare 或换用不同 SQL 文本。
void mysql_conn_base::write_stmt_execute_params(
    const std::vector<http::obj_val> &params,
    const std::vector<uint8_t> &param_types_src,
    const std::vector<uint8_t> &param_unsigned_src,
    std::vector<unsigned char> &out)
{
    // MySQL Prepared Statement Protocol (COM_STMT_EXECUTE) 参数编码:
    //   1. null_bitmap      : ceil(num_params/8) bytes
    //   2. new_params_bound : 1 byte (= 1 表示后面带 param_types)
    //   3. param_types      : 2 * num_params bytes (每个 param 2B: 1B field_type + 1B unsigned_flag(0x80))
    //   4. param_values     : 按 param_types[].field_type 格式依次写每个 param 的值
    // 关键: null_bitmap 标记 null 的 param, null 的 param 不需要写 type 和 value

    size_t param_count       = params.size();
    size_t null_bitmap_bytes = (param_count + 7) / 8;

    // 1. null_bitmap (先占位, 计算完后回填)
    size_t nb_start = out.size();
    for (size_t i = 0; i < null_bitmap_bytes; ++i)
        out.push_back(0);

    // 2. new_params_bound = 1 (表示携带 param_types)
    out.push_back(1);

    // 计算 null_bitmap
    for (size_t i = 0; i < param_count; ++i)
    {
        if (params[i].is_null())
        {
            size_t byte_idx = i / 8;
            size_t bit_idx  = i % 8;
            out[nb_start + byte_idx] |= (1 << bit_idx);
        }
    }

    // 3. param_types: 使用 COM_STMT_PREPARE 返回的真实列类型 + unsigned 属性
    //    协议: 每个 param 写 2 字节: field_type(低) | unsigned_flag(高, 0x80=unsigned)
    std::vector<uint16_t> param_types(param_count, 0);
    for (size_t i = 0; i < param_count; ++i)
    {
        // MY-7 修复：NULL 参数也按真实列类型声明类型段。服务端按 null_bitmap 跳过取值并忽略
        // 类型字节，故类型声明值不影响功能；保留真实列类型与 libmysql "bind INT 运行时 NULL
        // → 发 INT" 的语义一致（回退 0xFE），比原 0x00(DECIMAL) 更准确。
        uint8_t base_type = (i < param_types_src.size()) ? param_types_src[i] : 0xFE;
        // YEAR(0x0D)/BIT(0x10) 不能作为 COM_STMT_EXECUTE 的参数类型声明：实测 8.4.4 单参数报
        // 1210 Incorrect arguments to mysqld_stmt_execute，后面还有参数时报 1835 Malformed
        // communication packet（与值字节数无关）。统一按 VAR_STRING 声明，值段发十进制文本，
        // 由服务器隐式转换 —— 见下面值循环里的对应分支。
        if (base_type == 0x0D || base_type == 0x10)
            base_type = 0xFD;
        bool col_unsigned = (i < param_unsigned_src.size()) && param_unsigned_src[i];

        const auto &v     = params[i];
        bool val_unsigned = (v.get_type() == http::obj_type::UINT || v.get_type() == http::obj_type::ULONG);

        // MY-8 参数类型兜底（支持基线 MySQL 8 / MariaDB 12.1+，两个平台通用）：
        // COM_STMT_PREPARE 参数定义段返回 MYSQL_TYPE_NULL(0x06) 表示"该参数无类型上下文"。
        //   · MariaDB：对所有参数一律返回 0x06（不推断类型）；
        //   · MySQL 8：对无法推断类型的参数同样返回 0x06（如 `SELECT ?`、`SELECT ? + 1`），
        //     有列上下文时（如 `INSERT INTO t(col) VALUES(?)`）才返回列的真实类型。
        // 若沿用 0x06 声明 COM_STMT_EXECUTE 类型段，下方值循环 case 0x06 不写任何字节，
        // 服务器却按声明类型取值：MariaDB 报 1210 Incorrect arguments to mysqld_stmt_execute，
        // MySQL 8 则把该参数当 NULL 静默取错值。
        // 故服务端未给出具体类型且参数非 NULL 时，按运行时 obj_val 实际类型声明类型段，
        // 与 libmysql "bind 什么类型就发什么类型" 的语义一致。
        if (base_type == 0x06 && !v.is_null())
        {
            switch (v.get_type())
            {
            case http::obj_type::BOOL: base_type = 0x01; break;// TINY
            case http::obj_type::INT:
            case http::obj_type::LONG:
            case http::obj_type::UINT:
            case http::obj_type::ULONG: base_type = 0x08; break; // LONGLONG (8B)
            case http::obj_type::DOUBLE: base_type = 0x05; break;// DOUBLE (8B)
            default: base_type = 0xFD; break;                    // VAR_STRING
            }
        }

        // MY-13 类型一致性守卫：base_type 来自 COM_STMT_PREPARE 的列类型快照，值段却按它选编码
        // 分支并直接读 obj_val 的 union 成员。声明与运行时值不同族时（数值列传 "77"，或 DOUBLE
        // 传 obj_val(int)），读到的是从未被这个 obj_val 写过的成员：UPDATE affected=0 静默 0 行，
        // INSERT affected=1 但落库 0，或把 IEEE 位模式当整数发。
        // 与 libmysql "类型段声明必须与实际要发的值的类型一致" 对齐：统一降级为 VAR_STRING，
        // 值段发文本，由服务端做隐式转换 —— 与文本协议 exec_dml 同一条转换路径。
        // 残留差异（实测，非编码问题）：DOUBLE 参数绑整数列时，MariaDB 12.3.3 对文本 "12.5"
        // 与 binary DOUBLE 都截断成 12，而文本协议的精确字面量 12.5 四舍五入成 13；改按
        // 运行时族声明 0x05 实测不改变该结果，故不引入这条只在单一平台上验证过的分支。
        if (!v.is_null() && !stmt_param_binary_decl_matches(base_type, stmt_param_val_kind(v)))
            base_type = 0xFD;

        param_types[i] = base_type | ((col_unsigned || val_unsigned) ? 0x8000 : 0);
    }
    // 一次性写所有 type (小端 2B each)
    for (uint16_t t : param_types)
    {
        out.push_back(t & 0xFF);       // field_type
        out.push_back((t >> 8) & 0xFF);// flags (0x80 = unsigned)
    }

    // 4. param_values: 按 param_types[i].field_type (strip flags byte) 写值
    for (size_t i = 0; i < param_count; ++i)
    {
        if (params[i].is_null())
            continue;
        const auto &v = params[i];
        uint8_t ft    = param_types[i] & 0xFF;// field_type only

        // YEAR/BIT 列：声明已改为 VAR_STRING，这里配套发与之一致的字节
        if ((i < param_types_src.size()) && (param_types_src[i] == 0x0D || param_types_src[i] == 0x10))
        {
            std::string txt;
            const bool is_year = (param_types_src[i] == 0x0D);
            if (!is_year && !v.str_.empty())
                txt = v.str_;// BIT + 字符串参数：按 binary 字面量原样发（与行解码器对称）
            else if (is_year)
            {
                txt = v.str_;
                if (txt.empty())
                    txt = (v.get_type() == http::obj_type::UINT || v.get_type() == http::obj_type::ULONG) ? std::to_string(v.uval) : std::to_string(v.lval);
                // YEAR 的 "0" 会被服务器的两位年规则吃成 2000（实测），零必须写成 4 位形式
                if (txt.find_first_not_of('0') == std::string::npos)
                    txt = "0000";
            }
            else
            {
                // BIT + 数值参数：大端最小字节（1 字节起），服务器按位取
                uint64_t u = (v.get_type() == http::obj_type::UINT || v.get_type() == http::obj_type::ULONG) ? v.uval : static_cast<uint64_t>(v.lval);
                unsigned char be[8];
                int nb = 0;
                do
                {
                    be[nb++] = static_cast<unsigned char>(u & 0xFF);
                    u >>= 8;
                } while (u);
                txt.assign(nb, '\0');
                for (int z = 0; z < nb; ++z)
                    txt[z] = static_cast<char>(be[nb - 1 - z]);
            }
            _write_varint(out, txt.size());
            out.insert(out.end(), txt.begin(), txt.end());
            continue;
        }

        switch (ft)
        {
        case 0x01:// TINYINT(1) / BOOL: 1 byte
            if (v.get_type() == http::obj_type::BOOL)
                out.push_back(v.lval ? 1 : 0);
            else
                out.push_back(static_cast<uint8_t>(v.lval));
            break;
        case 0x02:// SMALLINT: 2 bytes LE
            _le_write(out, static_cast<uint16_t>(v.lval), 2);
            break;
        case 0x03:// INT
        case 0x09:// INT24 (MEDIUMINT): 4 bytes LE
            _le_write(out, static_cast<uint32_t>(v.lval), 4);
            break;
        case 0x08:// BIGINT (LONGLONG): 8 bytes LE
            if (v.get_type() == http::obj_type::UINT || v.get_type() == http::obj_type::ULONG)
                _le_write(out, v.uval, 8);
            else
                _le_write(out, static_cast<uint64_t>(v.lval), 8);
            break;
        case 0x04:// FLOAT: 4 bytes IEEE 754
        {
            float f = static_cast<float>(v.dval);
            uint32_t raw;
            std::memcpy(&raw, &f, 4);
            _le_write(out, raw, 4);
        }
        break;
        case 0x05:// DOUBLE: 8 bytes IEEE 754
        {
            uint64_t raw;
            std::memcpy(&raw, &v.dval, 8);
            _le_write(out, raw, 8);
        }
        break;
        case 0x06:// NULL — 不写 value (null_bitmap 已覆盖)
            break;
        case 0x0B:// TIME — COM_STMT_EXECUTE binary format
            // lenenc length(8 或 12) + neg(1B) + days(4B LE) + hh/mm/ss 各 1B + [micro(4B LE)]
            {
                const std::string &s = v.str_;
                bool has_micro       = s.find('.') != std::string::npos;
                size_t pos           = 0;
                bool neg             = false;
                if (!s.empty() && s[0] == '-')
                {
                    neg = true;
                    pos = 1;
                }
                long long days = 0, hh = 0, mm = 0, ss = 0, micro = 0;
                // 超 24h 格式: D H:MM:SS 或 -D H:MM:SS
                char colon = 0, dot = 0;
                int scanned = sscanf(s.c_str() + pos, "%lld%c %lld:%lld:%lld%c%lld", &days, &colon, &hh, &mm, &ss, &dot, &micro);
                if (scanned < 3 || colon != ' ')
                {
                    // 普通格式: HH:MM:SS 或 HH:MM:SS.micro
                    long long total_h = 0;
                    scanned           = sscanf(s.c_str() + pos, "%lld:%lld:%lld%c%lld", &total_h, &mm, &ss, &dot, &micro);
                    if (scanned >= 3)
                    {
                        days = total_h / 24;
                        hh   = total_h % 24;
                    }
                    else
                    {
                        // 解析失败 → 发 0 长度 TIME。不能退回字符串：声明已是 0x0B，
                        // 字符串字节数与 binary 格式不一致会错位整包（连带后面所有参数）。
                        out.push_back(0);
                        break;
                    }
                }
                // 校验微秒位数，确保补零到 6 位
                if (dot == '.' && scanned >= 4)
                {
                    int us_len = (int)s.length() - (int)(s.find('.') + 1) - pos;
                    // micro 已经是整数了，补零到 6 位
                    for (int z = us_len; z < 6; z++)
                        micro *= 10;
                }
                // MySQL TIME binary: days 是 unsigned, 负数用 is_negative flag 表示
                // 我们已把 total_h 拆成 days+hh，确保 days*24+hh == abs(total_h)
                out.push_back(has_micro ? 12 : 8);// length prefix
                out.push_back(neg ? 1 : 0);
                _le_write(out, (uint32_t)(days < 0 ? -days : days), 4);
                out.push_back((unsigned char)((hh < 0 ? -hh : hh) & 0xFF));
                out.push_back((unsigned char)(mm & 0xFF));
                out.push_back((unsigned char)(ss & 0xFF));
                if (has_micro)
                    _le_write(out, (uint32_t)(micro < 0 ? -micro : micro), 4);
            }
            break;
        case 0x0A:// DATE — COM_STMT_EXECUTE binary format
            // 4B: year(2B LE) + month(1B) + day(1B)
            {
                const std::string &s = v.str_;
                long long year = 0, month = 0, day = 0;
                char dash = 0, dash2 = 0;
                int scanned = sscanf(s.c_str(), "%lld%c%lld%c%lld", &year, &dash, &month, &dash2, &day);
                if (dash == '-' && dash2 == '-' && scanned >= 5)
                {
                    out.push_back(4);// length prefix (固定 4)
                    _le_write(out, (uint16_t)(year & 0xFFFF), 2);
                    out.push_back((unsigned char)(month & 0xFF));
                    out.push_back((unsigned char)(day & 0xFF));
                }
                else
                {
                    // 解析失败 → 0 长度 DATE，保持与声明 0x0A 的字节布局一致（见 TIME 分支说明）
                    out.push_back(0);
                }
            }
            break;
        case 0x07:// TIMESTAMP
        case 0x0C:// DATETIME — COM_STMT_EXECUTE binary format
            // lenenc length(4=纯日期, 7=日期+时间, 11=日期+时间+微秒)
            // year(2B LE) + month(1B) + day(1B) + hh/mm/ss 各 1B + [micro(4B LE)]
            {
                const std::string &s = v.str_;
                long long year = 0, month = 0, day = 0, hh = 0, mm = 0, ss = 0, micro = 0;
                int has_time = 0, has_micro = 0;
                char dash = 0, dash2 = 0, space = 0, colon = 0, colon2 = 0, dot = 0;
                int scanned = sscanf(s.c_str(), "%lld%c%lld%c%lld%c%lld%c%lld%c%lld%c%lld", &year, &dash, &month, &dash2, &day, &space, &hh, &colon, &mm, &colon2, &ss, &dot, &micro);
                // 日期部分必须有
                if (dash == '-' && dash2 == '-' && scanned >= 5)
                {
                    if (space == ' ')
                        has_time = 1;
                    if (dot == '.')
                    {
                        has_micro = 1;
                        has_time  = 1;
                    }
                    if (has_micro)
                    {
                        int us_len = (int)s.length() - (int)(s.find('.') + 1);
                        for (int z = us_len; z < 6; z++)
                            micro *= 10;
                    }
                }
                else
                {
                    // 解析失败 → 0 长度 DATETIME，保持与声明的字节布局一致（见 TIME 分支说明）
                    out.push_back(0);
                    break;
                }
                unsigned char lprefix;
                if (!has_time)
                    lprefix = 4;
                else if (!has_micro)
                    lprefix = 7;
                else
                    lprefix = 11;
                out.push_back(lprefix);
                _le_write(out, (uint16_t)(year & 0xFFFF), 2);
                out.push_back((unsigned char)(month & 0xFF));
                out.push_back((unsigned char)(day & 0xFF));
                if (has_time)
                {
                    out.push_back((unsigned char)(hh & 0xFF));
                    out.push_back((unsigned char)(mm & 0xFF));
                    out.push_back((unsigned char)(ss & 0xFF));
                    if (has_micro)
                        _le_write(out, (uint32_t)(micro < 0 ? -micro : micro), 4);
                }
            }
            break;
        case 0x0F:// VARCHAR
        case 0x10:// BIT
        case 0xF8:// ENUM
        case 0xF9:// SET
        case 0xF2:// VECTOR
        case 0xF6:// NEWDECIMAL
        case 0xFD:// VAR_STRING
        case 0xFE:// STRING (CHAR)
        case 0xFF:// GEOMETRY
        default:
            // MY-13 反向不匹配：列是文本类型而参数是运行时数值（如 obj_val(int) 绑 VARCHAR 列）。
            // 数值构造只写 union、不写 str_，此处若照发 str_ 就把列静默写成空值。
            // BOOL 也与文本协议保持一致写 0/1（MySQL 无 bool 字面量，to_string 的 "true" 不可用）。
            {
                const std::string *txt = &v.str_;
                std::string num_txt;
                if (txt->empty() && stmt_param_val_kind(v) != stmt_param_kind::OTHER)
                {
                    num_txt = (v.get_type() == http::obj_type::BOOL) ? (v.lval ? "1" : "0") : v.to_string();
                    txt     = &num_txt;
                }
                _write_varint(out, txt->size());
                out.insert(out.end(), txt->begin(), txt->end());
            }
            break;
        }
    }
}

// ===== exec_dml_prepared_impl（同步闭环）=====
unsigned int mysql_conn_base::exec_dml_prepared_impl(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    // COM_STMT 缓存：同 SQL 复用 stmt_id，避免每次 prepare + close 往返
    // 冷路径(未命中) → stmt_prepare_impl 后写入 cache entry
    // 热路径(命中)   → 直接用缓存的 stmt_id + types，跳过 prepare
    // warm 失败(1243/1615/连接断) → 丢缓存 + retry cold 一次
    unsigned int attempt = 0;
RETRY_EXEC_DML:
    error_msg.clear();
    error_code                            = 0;
    last_insert_id_                       = 0;
    const mysql_stmt_cache_entry_t *entry = stmt_cache_.find(sql);
    bool warm                             = (entry != nullptr);
    unsigned int stmt_id;
    const std::vector<uint8_t> *param_types;
    const std::vector<uint8_t> *param_unsigned;

    if (warm)
    {
        stmt_id        = entry->stmt_id;
        param_types    = &entry->param_types;
        param_unsigned = &entry->param_unsigned;
    }
    else
    {
        stmt_id = stmt_prepare_impl(sql);
        if (stmt_id == 0)
        {
            // error_code == 21 表示 prepare 包根本没写出去, 重连一次是安全的;
            // 其余失败(响应读不到/服务器回了 ERR)包已到服务端, 一律不重连不重发。
            if (attempt < kMaxReconnect && error_code == 21 && is_last_error_reconnectable() && try_reconnect())
            {
                attempt++;
                goto RETRY_EXEC_DML;
            }
            return (unsigned int)-1;
        }
        uint32_t evicted = 0;
        auto &e          = stmt_cache_.upsert(sql, &evicted);
        // 淘汰即释放：stmt_id 是服务端句柄，只删 map 会让预备语句按会话累积
        // 直到全局 max_prepared_stmt_count(16382)。此处不在任何响应读取中途，
        // 且 COM_STMT_CLOSE 无响应 ⇒ 一个 9 字节包插在两条命令之间是安全的。
        if (evicted != 0)
            stmt_close_impl(evicted);
        e.stmt_id        = stmt_id;
        e.param_types    = stmt_param_types_;
        e.param_unsigned = stmt_param_unsigned_;
        e.col_types      = stmt_col_types_;
        e.col_unsigned   = stmt_col_unsigned_;
        e.col_decimals   = stmt_col_decimals_;
        e.col_names      = stmt_col_names_;
        e.col_org_names  = stmt_col_org_names_;
        entry            = &e;
        param_types      = &e.param_types;
        param_unsigned   = &e.param_unsigned;
    }

    unsigned int result = (unsigned int)-1;
    unsigned int iid    = 0;

    // 发送 COM_STMT_EXECUTE
    {
        std::vector<unsigned char> pkt_body;
        pkt_body.push_back(0x17);// COM_STMT_EXECUTE
        _le_write(pkt_body, stmt_id, 4);
        pkt_body.push_back(0x00); // flags = CURSOR_TYPE_NO_CURSOR
        _le_write(pkt_body, 1, 4);// iteration_count = 1

        // N-9: 发送前本地强校验参数个数 == 占位符数。参数不足旧行为等服务器
        // 返回 1835; 参数过多会留下未消费字节导致流错位/静默 0 行 —— 统一本地拒绝。
        if (params.size() != param_types->size())
        {
            error_msg  = "COM_STMT_EXECUTE: prepared statement has " + std::to_string(param_types->size()) + " placeholder(s) but got " + std::to_string(params.size()) + " parameter value(s)";
            error_code = 20;
            // 不 close：stmt_id 本身仍然有效，是调用方参数不匹配
            return (unsigned int)-1;
        }

        write_stmt_execute_params(params, *param_types, *param_unsigned, pkt_body);

        // 构建完整 MySQL packet; 单物理包上限 0xFFFFFF, 超过直接拒绝(不拆包)
        unsigned int payload_len = pkt_body.size();
        if (payload_len >= 0xFFFFFF)
        {
            error_msg = "COM_STMT_EXECUTE: payload too large";
            // 不 close：stmt_id 仍然有效
            return (unsigned int)-1;
        }

        send_data.clear();
        send_data.push_back((payload_len & 0xFF));
        send_data.push_back((payload_len >> 8 & 0xFF));
        send_data.push_back((payload_len >> 16 & 0xFF));
        send_data.push_back(0x00);
        send_data.insert(send_data.end(), pkt_body.begin(), pkt_body.end());

        unsigned int n = write();
        if (n == 0)
        {
            // write 失败：连接可能断了，warm 下 erase 并 retry 一次 cold
            if (attempt < kMaxReconnect && is_last_error_reconnectable())
            {
                if (warm)
                    stmt_cache_drop(sql);
                if (try_reconnect())
                {
                    attempt++;
                    goto RETRY_EXEC_DML;
                }
            }
            return (unsigned int)-1;
        }
    }

    // 读 COM_STMT_EXECUTE 响应（OK 或 Error，DML 不会返回 ResultSet）
    {
        {
            pooled_accum_buf accum_holder;
            auto &accum_buf = accum_holder.buf;

            unsigned int n = read_loop();
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

            while (accum_buf.size() < 4)
            {
                n = read_loop();
                if (n == 0)
                {
                    // execute 已发出, 响应丢失不等于服务端没做: 不重发, 交由上层判定。
                    // 缓存句柄留给下一次 write 失败分支(那里才做 drop + 重连)。
                    return (unsigned int)-1;
                }
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            }

            unsigned int pkt_len = accum_buf[0] | (accum_buf[1] << 8) | (accum_buf[2] << 16);
            while (accum_buf.size() < 4 + pkt_len)
            {
                n = read_loop();
                if (n == 0)
                    break;
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            }

            unsigned char status = accum_buf[4];
            if (status == 0xFF)
            {
                // parse_error_packet 约定 data 指向包头起点(含 4B header), pkt_len 含包头
                if (accum_buf.size() >= 4 + pkt_len)
                    parse_error_packet(&accum_buf[0], pkt_len + 4);
            }
            else if (status == 0x00)
            {
                // OK packet = DML/无结果集语句的最终响应
                unsigned int affected = parse_affected_rows_fast(accum_buf.data(), accum_buf.size(), &iid);
                if (affected != (unsigned int)-1)
                {
                    last_insert_id_ = iid;
                    result          = affected;
                }
            }
            else
            {
                // 首个响应既非 OK 也非 ERR = column count, 即语句实际返回了结果集
                // (把 SELECT 误用成 exec_dml_prepared)。此前只读首包就返回, 结果集其余
                // 字节残留在 socket(可能停在任意物理包中间), 下一条命令会把行内容当
                // 包头读出超大长度, 在 read_loop 永久等待 → 连接卡死(探针 E 段复现)。
                // 因此显式报错, 并把整个结果集排空到终结包(0xFE/0xFF)后保持连接可用。
                error_msg  = "exec_dml_prepared: statement returned a result set (not DML)";
                error_code = 10;
                result     = (unsigned int)-1;

                auto drain_read = [&]() -> bool
                {
                    unsigned int k = read_loop();
                    if (k == 0)
                        return false;
                    accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                    return true;
                };
                // 跳过「一个逻辑包」(列定义/行)。MySQL 会把 >0xFFFFFF 的逻辑包切成多个
                // 连续满块物理包, 最后一块(可空)长度 < 0xFFFFFF 表示逻辑包结束。
                // 返回 {逻辑包首包 payload 首字节, 该逻辑包起始偏移}; 读失败返回 {-1,0}。
                auto skip_logical = [&](size_t &cur) -> std::pair<int, size_t>
                {
                    size_t start = cur;
                    auto ensure  = [&](size_t need) -> bool
                    {
                        while (accum_buf.size() - cur < need)
                            if (!drain_read())
                                return false;
                        return true;
                    };
                    if (!ensure(4))
                        return {-1, 0};
                    unsigned int plen = accum_buf[cur] | (accum_buf[cur + 1] << 8) |
                                        (accum_buf[cur + 2] << 16);
                    if (!ensure(4 + plen))
                        return {-1, 0};
                    unsigned char first = accum_buf[cur + 4];
                    cur += 4 + plen;
                    while (plen == 0xFFFFFFu)
                    {
                        if (!ensure(4))
                            return {-1, 0};
                        plen = accum_buf[cur] | (accum_buf[cur + 1] << 8) |
                               (accum_buf[cur + 2] << 16);
                        if (!ensure(4 + plen))
                            return {-1, 0};
                        cur += 4 + plen;
                    }
                    return {first, start};
                };

                // 首包(column count)已在 accum 中, 从 4+pkt_len 起消费后续内容。
                size_t cur       = 4 + pkt_len;
                uint64_t col_cnt = 0;
                if (pkt_len >= 1)
                {
                    unsigned char b0 = accum_buf[4];
                    if (b0 < 0xFB)
                        col_cnt = b0;
                    else if (b0 == 0xFC && pkt_len >= 3)
                        col_cnt = accum_buf[5] | (static_cast<unsigned int>(accum_buf[6]) << 8);
                    else if (b0 == 0xFD && pkt_len >= 4)
                        col_cnt = accum_buf[5] | (static_cast<unsigned int>(accum_buf[6]) << 8) |
                                  (static_cast<unsigned int>(accum_buf[7]) << 16);
                    else if (b0 == 0xFE && pkt_len >= 9)
                    {
                        for (int i = 0; i < 8; ++i)
                            col_cnt |= static_cast<uint64_t>(accum_buf[5 + i]) << (i * 8);
                    }
                }

                bool drain_ok = true;
                // 1) column definition 包(col_cnt 个)
                for (uint64_t i = 0; i < col_cnt && drain_ok; ++i)
                {
                    auto [f, start] = skip_logical(cur);
                    if (f < 0)
                    {
                        drain_ok = false;
                        break;
                    }
                    if (f == 0xFF)
                    {
                        parse_error_packet(&accum_buf[start], static_cast<unsigned int>(cur - start));
                        drain_ok = false;
                        break;
                    }
                }
                // 2) 行直到终结包(0xFE = OK-via-EOF, 0xFF = ERR)
                while (drain_ok)
                {
                    auto [f, start] = skip_logical(cur);
                    if (f < 0)
                    {
                        drain_ok = false;
                        break;
                    }
                    if (f == 0xFF)
                    {
                        parse_error_packet(&accum_buf[start], static_cast<unsigned int>(cur - start));
                        drain_ok = false;
                        break;
                    }
                    if (f == 0xFE)
                        break;// 结果集终结包
                }
            }
        }

        // 正常执行完不 close（缓存 stmt_id 留着下次用）。
        // 只有服务端明确拒绝了这个 stmt_id 才重跑一次：
        //   - 1243 ER_UNKNOWN_STMT_HANDLER / 1615 ER_NEED_REPREPARE：语句根本没执行,
        //     丢缓存 + 冷路径重新 prepare 是幂等安全的。
        //     注意 error_code 装的是 ERR packet 里的服务器 errno，客户端 API 码(2030 一类)不会出现在这。
        //     能收到 ERR 包说明连接是活的, 所以这里只 re-prepare, 不重连。
        // 连接断不在此列：execute 包已经写出去了, 重发可能把 DML 落库两遍。
        if (result == (unsigned int)-1 && attempt < kMaxReconnect)
        {
            bool stmt_gone = (error_code == 1243 || error_code == 1615);
            if (stmt_gone)
            {
                if (warm)
                    stmt_cache_drop(sql);
                attempt++;
                goto RETRY_EXEC_DML;
            }
        }

        return result;
    }
}

static std::optional<orm::col_value_variant> _decode_binary_value_typed(
    const unsigned char *buf, int buf_len, int &off, uint8_t tc, bool uflag)
{
    using orm::col_value_variant;
    switch (tc)
    {
    case 0x01:
        if (off + 1 > buf_len)
            return std::nullopt;
        if (uflag)
        {
            uint64_t v = buf[off];
            off += 1;
            return col_value_variant{v};
        }
        else
        {
            int64_t v = (int64_t)(int8_t)buf[off];
            off += 1;
            return col_value_variant{v};
        }
    case 0x02:
    case 0x0D:
        if (off + 2 > buf_len)
            return std::nullopt;
        {
            uint16_t r = (uint16_t)(buf[off] | (uint16_t)(buf[off + 1] << 8));
            off += 2;
            if (uflag || tc == 0x0D)
                return col_value_variant{(uint64_t)r};
            return col_value_variant{(int64_t)(int16_t)r};
        }
    case 0x03:
    case 0x09:
        if (off + 4 > buf_len)
            return std::nullopt;
        {
            uint32_t r = 0;
            std::memcpy(&r, buf + off, 4);
            off += 4;
            if (uflag)
                return col_value_variant{(uint64_t)r};
            return col_value_variant{(int64_t)(int32_t)r};
        }
    case 0x08:
        if (off + 8 > buf_len)
            return std::nullopt;
        {
            if (uflag)
            {
                uint64_t v = 0;
                std::memcpy(&v, buf + off, 8);
                off += 8;
                return col_value_variant{v};
            }
            else
            {
                int64_t v = 0;
                std::memcpy(&v, buf + off, 8);
                off += 8;
                return col_value_variant{v};
            }
        }
    case 0x04:
        if (off + 4 > buf_len)
            return std::nullopt;
        {
            float f = 0;
            std::memcpy(&f, buf + off, 4);
            off += 4;
            return col_value_variant{(double)f};
        }
    case 0x05:
        if (off + 8 > buf_len)
            return std::nullopt;
        {
            double d = 0;
            std::memcpy(&d, buf + off, 8);
            off += 8;
            return col_value_variant{d};
        }
    case 0x06: return col_value_variant{std::monostate{}};
    case 0x07:
    case 0x0A:
    case 0x0C:
    case 0x0B:
        return std::nullopt;
    case 0xF2:// MYSQL_TYPE_VECTOR(242): MySQL 8.0.17+, 二进制 varlen 格式
        if (off + 1 > buf_len)
            return std::nullopt;
        {
            auto [vlen, vsize] = _read_varint(buf, off);
            if (vsize == 0)
                return std::nullopt;
            if (vsize > buf_len - off)
                return std::nullopt;
            if (vlen > static_cast<uint64_t>(buf_len - off - vsize))
                return std::nullopt;
            const char *p = reinterpret_cast<const char *>(buf + off + vsize);
            off += vsize + static_cast<int>(vlen);
            return col_value_variant{std::string_view(p, static_cast<size_t>(vlen))};
        }
    default:
        if (off + 1 > buf_len)
            return std::nullopt;
        {
            auto [vlen, vsize] = _read_varint(buf, off);
            if (vsize == 0)
                return std::nullopt;
            if (vsize > buf_len - off)
                return std::nullopt;
            if (vlen > static_cast<uint64_t>(buf_len - off - vsize))
                return std::nullopt;
            const char *p = reinterpret_cast<const char *>(buf + off + vsize);
            off += vsize + static_cast<int>(vlen);
            return col_value_variant{std::string_view(p, static_cast<size_t>(vlen))};
        }
    }
}

// MySQL binary 时间值 → 字符串，结果写入 out。
// 返回 true = 成功（out 可以是空串：tlen==0 的零值是协议允许的合法结果）；
// 返回 false = 解码失败，且**保证不推进 off**，由调用方决定是否中止整行解码。
// 注意：out 必须是**行级**持久缓冲 —— col_value_variant 持有 string_view，
// 若 out 是局部 std::string，本函数返回后视图立即悬垂（短日期走 SSO 落在栈上）。
static bool _fmt_date(const unsigned char *buf, int len, int &off, uint8_t tc, uint8_t decimals, std::string &out)
{
    out.clear();
    if (off + 1 > len)
        return false;
    unsigned char tlen = buf[off];
    if (tlen == 0)
    {
        // 零长度时间值是协议规定的"零值"标记，不是失败；文本协议同一行的形态是
        // '0000-00-00' / '0000-00-00 00:00:00'，binary 侧必须还原成同样的串，
        // 否则 binary 与 text 两条路径对同一行会给出不同结果。
        off += 1;
        out.assign((tc == 0x0A) ? "0000-00-00" : "0000-00-00 00:00:00");
        _append_frac_seconds(out, decimals, 0);
        return true;
    }
    if (tlen != 4 && tlen != 7 && tlen != 11)
        return false;
    {
        int need = (tlen == 4) ? 5 : 8;
        if (off + need > len)
            return false;
        uint16_t year = (uint16_t)(buf[off + 1] | (uint16_t)(buf[off + 2] << 8));
        char tmp[64];
        int n;
        uint32_t micro = 0;
        if (tlen == 4)
        {
            n = (tc == 0x0A) ? std::snprintf(tmp, sizeof(tmp), "%04u-%02u-%02u", year, buf[off + 3], buf[off + 4]) : std::snprintf(tmp, sizeof(tmp), "%04u-%02u-%02u 00:00:00", year, buf[off + 3], buf[off + 4]);
            off += 5;
        }
        else
        {
            n = std::snprintf(tmp, sizeof(tmp), "%04u-%02u-%02u %02u:%02u:%02u", year, buf[off + 3], buf[off + 4], buf[off + 5], buf[off + 6], buf[off + 7]);
            off += 8;
            if (tlen == 11)
            {
                if (off + 4 > len)
                    return false;
                std::memcpy(&micro, buf + off, 4);
                off += 4;
            }
        }
        if (n > 0)
            out.assign(tmp, static_cast<size_t>(n));
        _append_frac_seconds(out, decimals, micro);
        return true;
    }
}

static bool _fmt_time(const unsigned char *buf, int len, int &off, uint8_t decimals, std::string &out)
{
    out.clear();
    if (off + 1 > len)
        return false;
    unsigned char tlen = buf[off];
    char tmp[64];
    if (tlen == 0)
    {// TIME 零值的文本形态是 "00:00:00"，不是空串
        off += 1;
        int n = std::snprintf(tmp, sizeof(tmp), "00:00:00");
        if (n > 0)
            out.assign(tmp, static_cast<size_t>(n));
        _append_frac_seconds(out, decimals, 0);
        return true;
    }
    if (tlen != 8 && tlen != 12)
        return false;
    if (off + 9 > len)
        return false;
    {
        bool neg      = buf[off + 1] != 0;
        uint32_t days = 0;
        std::memcpy(&days, buf + off + 2, 4);
        uint32_t hh = days * 24 + buf[off + 6];
        int n       = std::snprintf(tmp, sizeof(tmp), "%s%02u:%02u:%02u", neg ? "-" : "", hh, buf[off + 7], buf[off + 8]);
        off += 9;
        uint32_t micro = 0;
        if (tlen == 12)
        {
            if (off + 4 > len)
                return false;
            std::memcpy(&micro, buf + off, 4);
            off += 4;
        }
        if (n > 0)
            out.assign(tmp, static_cast<size_t>(n));
        _append_frac_seconds(out, decimals, micro);
        return true;
    }
}

static bool _decode_to_variant(
    const unsigned char *buf, int buf_len, int &off, uint8_t tc, bool uflag, uint8_t decimals, orm::col_value_variant &out, std::string *date_str)
{
    auto v = _decode_binary_value_typed(buf, buf_len, off, tc, uflag);
    if (v)
    {
        out = std::move(*v);
        return true;
    }
    // 时间类型需要一块**行级**持久缓冲：col_value_variant 持有 std::string_view，
    // 指向局部 std::string 会在本函数返回后立即悬垂（短日期走 SSO 落在栈上，随即被覆盖）。
    // 调用方在 tc ∈ {0x07(TIMESTAMP), 0x0A(DATE), 0x0B(TIME), 0x0C(DATETIME)} 时
    // 必须传入 &row_date_backing[ci]；其余类型传 nullptr。
    if (tc == 0x07 || tc == 0x0A || tc == 0x0C)
    {
        if (!date_str)
            return false;// 没有行级缓冲就无法安全交出 string_view
        if (!_fmt_date(buf, buf_len, off, tc, decimals, *date_str))
            return false;
        out = orm::col_value_variant{std::string_view{date_str->data(), date_str->size()}};
        return true;
    }
    if (tc == 0x0B)
    {
        if (!date_str)
            return false;
        if (!_fmt_time(buf, buf_len, off, decimals, *date_str))
            return false;
        out = orm::col_value_variant{std::string_view{date_str->data(), date_str->size()}};
        return true;
    }
    return false;
}

// ===== fetch_prepared_impl（同步闭环，SELECT + binary RowData 解析）=====
unsigned int mysql_conn_base::fetch_prepared_impl(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    // COM_STMT 缓存：同 SQL 复用 stmt_id，冷路径同时提取列名写入 entry
    unsigned int attempt = 0;
RETRY_FETCH:
    error_msg.clear();
    error_code                            = 0;
    last_insert_id_                       = 0;
    const mysql_stmt_cache_entry_t *entry = stmt_cache_.find(sql);
    bool warm                             = (entry != nullptr);
    unsigned int stmt_id;
    const std::vector<uint8_t> *param_types;
    const std::vector<uint8_t> *param_unsigned;

    if (warm)
    {
        stmt_id        = entry->stmt_id;
        param_types    = &entry->param_types;
        param_unsigned = &entry->param_unsigned;
    }
    else
    {
        stmt_id = stmt_prepare_impl(sql);
        if (stmt_id == 0)
        {
            // error_code == 21 表示 prepare 包根本没写出去, 重连一次是安全的;
            // 其余失败(响应读不到/服务器回了 ERR)包已到服务端, 一律不重连不重发。
            if (attempt < kMaxReconnect && error_code == 21 && is_last_error_reconnectable() && try_reconnect())
            {
                attempt++;
                goto RETRY_FETCH;
            }
            return 0;
        }
        uint32_t evicted = 0;
        auto &e          = stmt_cache_.upsert(sql, &evicted);
        // 淘汰即释放：stmt_id 是服务端句柄，只删 map 会让预备语句按会话累积
        // 直到全局 max_prepared_stmt_count(16382)。此处不在任何响应读取中途，
        // 且 COM_STMT_CLOSE 无响应 ⇒ 一个 9 字节包插在两条命令之间是安全的。
        if (evicted != 0)
            stmt_close_impl(evicted);
        e.stmt_id        = stmt_id;
        e.param_types    = stmt_param_types_;
        e.param_unsigned = stmt_param_unsigned_;
        e.col_types      = stmt_col_types_;
        e.col_unsigned   = stmt_col_unsigned_;
        e.col_decimals   = stmt_col_decimals_;
        e.col_names      = stmt_col_names_;
        e.col_org_names  = stmt_col_org_names_;
        entry            = &e;
        param_types      = &e.param_types;
        param_unsigned   = &e.param_unsigned;
    }

    unsigned int rows = 0;

    // 发送 COM_STMT_EXECUTE
    {
        std::vector<unsigned char> pkt_body;
        pkt_body.push_back(0x17);
        _le_write(pkt_body, stmt_id, 4);
        pkt_body.push_back(0x00);
        _le_write(pkt_body, 1, 4);

        // N-9: 参数个数与占位符数强校验。参数不足/过多都本地拒绝,
        // 避免服务器侧"参数过多静默 0 行"或残留未消费字节造成流错位。
        if (params.size() != param_types->size())
        {
            error_msg  = "COM_STMT_EXECUTE: prepared statement has " + std::to_string(param_types->size()) + " placeholder(s) but got " + std::to_string(params.size()) + " parameter value(s)";
            error_code = 20;
            return 0;
        }

        write_stmt_execute_params(params, *param_types, *param_unsigned, pkt_body);

        unsigned int payload_len = pkt_body.size();

        send_data.clear();
        send_data.push_back((payload_len & 0xFF));
        send_data.push_back((payload_len >> 8 & 0xFF));
        send_data.push_back((payload_len >> 16 & 0xFF));
        send_data.push_back(0x00);
        send_data.insert(send_data.end(), pkt_body.begin(), pkt_body.end());

        if (write() == 0)
        {
            if (attempt < kMaxReconnect && is_last_error_reconnectable())
            {
                if (warm)
                    stmt_cache_drop(sql);
                if (try_reconnect())
                {
                    attempt++;
                    goto RETRY_FETCH;
                }
            }
            return 0;
        }
    }

    // 读 ResultSet: column_defs + EOF + binary RowData rows + EOF
    {
        pooled_accum_buf accum_holder;
        auto &accum_buf       = accum_holder.buf;
        unsigned int consumed = 0;

        std::vector<std::string> col_names;
        std::vector<char *> name_ptrs_cache;
        std::vector<unsigned char> col_types;   // MYSQL_TYPE per column
        std::vector<unsigned char> col_uflags;  // per column UNSIGNED_FLAG, 与 col_types 对齐
        std::vector<unsigned char> col_decimals;// per column decimals, 与 col_types 对齐
        unsigned int expected_cols = 0;
        col_org_names_.clear();// 只描述本次结果集, 不留下一次可读到的旧列名
        enum class phase
        {
            INIT,
            COLUMNS,
            ROWS,
            DONE
        };
        phase current_phase  = phase::INIT;
        bool stop            = false;
        bool handler_aborted = false;// 提前中止后继续排空到 0xFE 终结包

        auto read_more = [&]() -> bool
        {
            unsigned int n = read_loop();
            if (n == 0)
                return false;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            return true;
        };

        while (!stop)
        {
            // 确保有完整 packet header
            while (accum_buf.size() - consumed < 4)
            {
                if (!read_more())
                {
                    // 不 close：缓存 stmt_id 留着下次用
                    return rows;
                }
            }

            unsigned int pkt_len = accum_buf[consumed] |
                                   (accum_buf[consumed + 1] << 8) |
                                   (accum_buf[consumed + 2] << 16);
            unsigned int total_pkt_len = 4 + pkt_len;

            // 确保完整 packet
            while (accum_buf.size() - consumed < total_pkt_len)
            {
                if (!read_more())
                    break;
            }

            if (accum_buf.size() - consumed < total_pkt_len)
                break;

            unsigned char status = accum_buf[consumed + 4];

            switch (current_phase)
            {
            case phase::INIT:
            {
                if (status == 0xFF)
                {
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    if (attempt < kMaxReconnect && (error_code == 1243 || error_code == 1615))
                    {
                        if (warm)
                            stmt_cache_drop(sql);
                        attempt++;
                        goto RETRY_FETCH;
                    }
                    return rows;
                }
                if (status == 0x00)
                {
                    // OK packet (0x00 头) = DML/DO 等无结果集语句的最终响应。
                    // 官方 page_protocol_com_stmt_execute_response: COM_STMT_EXECUTE
                    // 响应仅 OK/ERR/Binary Resultset 三选一, SELECT 结果集首包必为
                    // column_count(lenenc, 恒 >0), 不会以 0x00 开头。
                    // 解析 affected_rows/last_insert_id 后立即结束; 旧逻辑"消费后继续
                    // 读下一包"在服务器无后续包时会阻塞读。
                    unsigned int iid = 0;
                    unsigned int ar  = parse_affected_rows_fast(&accum_buf[consumed],
                                                               total_pkt_len,
                                                               &iid);
                    if (ar != (unsigned int)-1)
                        last_insert_id_ = iid;
                    current_phase = phase::DONE;
                    stop          = true;
                    consumed += total_pkt_len;
                    break;
                }
                if (status == 0xFE)
                {
                    // 0xFE 头 (经典 EOF / OK-via-EOF) 不在 COM_STMT_EXECUTE 首包定义内
                    // (Resultset column_count 恒 >0, 无结果集语句发 OK 而非 EOF);
                    // 防御性按"空结果集结束"直接终止, 避免转入 ROWS 后等行包阻塞。
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                // 不是 OK/EOF/Error → 一定是 column count (length-encoded integer)
                // 这是 SELECT 的 ResultSet 开始
                auto [col_cnt, col_cnt_size] = _read_varint(&accum_buf[consumed + 4], 0);
                expected_cols                = static_cast<unsigned int>(col_cnt);
                col_names.reserve(expected_cols);
                col_types.reserve(expected_cols);
                col_decimals.reserve(expected_cols);
                consumed += 4 + col_cnt_size;

                if (warm && entry->col_names.size() == expected_cols &&
                    entry->col_org_names.size() == expected_cols &&
                    entry->col_types.size() == expected_cols &&
                    entry->col_unsigned.size() == expected_cols &&
                    entry->col_decimals.size() == expected_cols)
                {
                    // ★ 热路径: N 个 Column Definition 包仍要按长度消费(否则流错位), 但逐包只做
                    // "列名逐字节比对 + type/unsigned 定长读", 不构造 std::string。
                    // 不变式: 行解码只认本轮元数据, 缓存只允许省掉"分配与拷贝", 不允许省掉
                    // "本轮元数据的读取"。服务器 ALTER 后会自行 re-prepare(stmt_id 仍有效),
                    // 缓存侧收不到失效信号 ⇒ 唯一的发现手段就是这次比对。
                    // 非破坏式: 用 probe 游标探测, 全部匹配才推进 consumed; 一旦不符就回到
                    // 第一个列定义包走冷路径重解析(名字+类型), 并用本轮结果刷新 entry。
                    bool meta_same     = true;
                    unsigned int probe = consumed;
                    for (unsigned int i = 0; i < expected_cols; ++i)
                    {
                        while (accum_buf.size() - probe < 4)
                            if (!read_more())
                                break;
                        if (accum_buf.size() - probe < 4)
                        {
                            meta_same = false;
                            break;
                        }
                        unsigned int pl = accum_buf[probe] | (accum_buf[probe + 1] << 8) |
                                          (accum_buf[probe + 2] << 16);
                        while (accum_buf.size() - probe < 4 + pl)
                            if (!read_more())
                                break;
                        if (accum_buf.size() - probe < 4 + pl)
                        {
                            meta_same = false;
                            break;
                        }
                        if (!_definition_matches(&accum_buf[probe + 4], pl, entry->col_names[i], entry->col_org_names[i], entry->col_types[i], entry->col_unsigned[i], entry->col_decimals[i]))
                        {
                            meta_same = false;
                            break;
                        }
                        probe += 4 + pl;
                    }
                    if (!meta_same)
                    {
                        // consumed 未推进 ⇒ 冷路径从第一个列定义包完整解析, 行解码用本轮类型
                        current_phase = phase::COLUMNS;
                        break;
                    }
                    consumed       = probe;
                    col_names      = entry->col_names;// 本轮已逐字节校验 ⇒ 零拷贝复用是安全的
                    col_org_names_ = entry->col_org_names;
                    col_types      = entry->col_types;
                    col_uflags     = entry->col_unsigned;
                    col_decimals   = entry->col_decimals;
                    current_phase  = phase::ROWS;
                }
                else if (expected_cols == 0)
                {
                    current_phase = phase::ROWS;
                }
                else
                {
                    current_phase = phase::COLUMNS;
                }
                break;
            }

            case phase::COLUMNS:
            {
                if (status == 0xFE)
                {
                    // 硬基线（连接期门禁已要求服务器 advertise CLIENT_DEPRECATE_EOF）之下，列定义段
                    // 中途不会再有任何 0xFE 包：分隔包不存在，终结包只在全列收齐之后才发。走到这里
                    // 的前提只有"本轮列数与实际列定义包数不符 = 协议失步"，转 ROWS 会去等一个永不再来
                    // 的行包(D-10 形状)；按终结包消费并收束结果集才能保住流对齐。
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    return rows;
                }
                // column_definition packet — 列名解析出显示名/原始名, type 用 stmt_prepare 缓存
                {
                    const unsigned char *body = &accum_buf[consumed + 4];
                    std::string col_name, org_name;
                    if (!_parse_definition_names(body, static_cast<unsigned int>(total_pkt_len - 4), col_name, org_name))
                    {
                        // 定义包损坏: 显示名置空(findcolpos 返回 255 → 该列不参与映射),
                        // 但仍须按整包消费, 保持与 expected_cols 计数对齐, 否则行包会被
                        // 误判成列定义。
                        col_name.clear();
                    }
                    // 行解码只认本轮列定义包里的类型：stmt_col_types_ 是「最近一次 prepare」
                    // 的类型，ALTER 之后或换一条 SQL 都会让解码错位且不产生任何错误码(D-1/D-2)。
                    uint8_t t_code  = 0xFE;// 解析不出类型时按 STRING，与行解码越界默认一致
                    bool t_unsigned = false;
                    uint8_t t_dec   = 0;// 解析不出精度时按 0 位小数(不打印分数部分)
                    if (!_parse_definition_type(body, static_cast<unsigned int>(total_pkt_len - 4), t_code, t_unsigned, &t_dec))
                        t_code = 0xFE;
                    col_types.push_back(t_code);
                    col_uflags.push_back(t_unsigned ? 1 : 0);
                    col_decimals.push_back(t_dec);
                    col_org_names_.push_back(std::move(org_name));
                    col_names.push_back(std::move(col_name));
                }

                consumed += total_pkt_len;
                if (col_names.size() >= expected_cols)
                {
                    // 本轮列元数据回写缓存：键已存在 ⇒ update_meta 不插入、不淘汰，
                    // 因此同轮内持有的 entry 指针不会失效。类型漂移后下一轮重新变热。
                    stmt_cache_.update_meta(sql, col_types, col_uflags, col_decimals, col_names, col_org_names_);
                    current_phase = phase::ROWS;
                }
                break;
            }

            case phase::ROWS:
            {
                if (status == 0xFE)
                {
                    // EOF / EOF-via-OK packet — 结果集结束
                    // MySQL 8.4 用 OK packet 格式的 EOF (pkt_len=7) 取代传统 5 字节 EOF
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    return rows;
                }
                // 注意: 不再用 status==0x00 && pkt_len==7 判断行集结束 —
                // 单列短行(如 1 列 DATE, payload 恰 7 字节)会被误判成 OK 终结而整行丢失。
                // 行集终结符在 MySQL 8.4 (CLIENT_DEPRECATE_EOF) 为 0xFE 头的 OK-via-EOF,
                // 旧版为 0xFE 头的 5 字节 EOF, 均由上面的 status==0xFE 分支处理。

                // 首包长度 == 0xFFFFFF(满块) = 该逻辑包(行/单值)被 MySQL net 层拆成多个
                // 物理包, 即结果 >16MB。范围决定(2026-09-06): 不支持续包拼接,
                // 显式报错并排空到该行边界/终结包, 不再静默截断/变 NULL。
                if (handler_aborted || pkt_len == 0xFFFFFFu)
                {
                    if (pkt_len == 0xFFFFFFu && !handler_aborted)
                    {
                        error_msg       = "fetch_prepared: result row over 16MB (0xFFFFFF split) not supported";
                        error_code      = 20;
                        handler_aborted = true;// 后续进入排空, 直到 0xFE 终结包
                    }
                    // 排空模式：整包丢弃直到 0xFE 终结包，不再解码也不再回调。
                    // 拆包行必须先把续包全部消费到整行(逻辑包)边界, 避免把行内容
                    // 的任意字节误判成行集终结包(status 0xFE)。
                    size_t row_end = consumed + total_pkt_len;
                    if (pkt_len == 0xFFFFFFu)
                    {
                        size_t cur = row_end;
                        for (;;)
                        {
                            while (accum_buf.size() - cur < 4)
                            {
                                if (!read_more())
                                    break;
                            }
                            if (accum_buf.size() - cur < 4)
                            {
                                consumed = cur;
                                break;
                            }
                            unsigned int cplen = accum_buf[cur] |
                                                 (accum_buf[cur + 1] << 8) |
                                                 (accum_buf[cur + 2] << 16);
                            unsigned int ctotal = 4 + cplen;
                            while (accum_buf.size() - cur < ctotal)
                            {
                                if (!read_more())
                                    break;
                            }
                            if (accum_buf.size() - cur < ctotal)
                            {
                                consumed = cur;
                                break;
                            }
                            cur += ctotal;
                            if (cplen < 0xFFFFFFu)
                                break;// 短块(可空) = 该逻辑包结束
                        }
                        row_end = cur;
                    }
                    consumed = row_end;
                    break;
                }

                // binary RowData packet (单物理包内完整, pkt_len < 0xFFFFFF)
                // 格式:
                //   0x00 header
                //   null_bitmap (ceil(col_count/8) bytes, skip first bit)
                //   column values
                const unsigned char *row_body = &accum_buf[consumed + 4];
                int row_body_len              = static_cast<int>(pkt_len);
                size_t row_end                = consumed + total_pkt_len;

                if (row_body_len < 1)
                {
                    consumed = row_end;
                    break;
                }
                if (row_body[0] != 0x00)
                {
                    // 不是 binary RowData header, 跳过
                    consumed = row_end;
                    break;
                }

                // null_bitmap
                size_t nb_bytes = (expected_cols + 9) / 8;// +1 for skip header byte
                if (row_body_len < static_cast<int>(1 + nb_bytes))
                {
                    consumed = row_end;
                    break;
                }
                const unsigned char *nb_start = row_body + 1;

                // 收集每列的值 + 逐列 NULL 标志(N-8: 空串必须与 NULL 区分)
                std::vector<std::string> row_values;
                row_values.reserve(expected_cols);
                std::vector<unsigned char> row_nulls;// 与 row_values 对齐: 1=SQL NULL
                row_nulls.reserve(expected_cols);

                int val_off = 1 + static_cast<int>(nb_bytes);
                for (unsigned int ci = 0; ci < expected_cols; ++ci)
                {
                    // 检查 null
                    size_t bit_pos  = ci + 2;// +1 header byte + 位偏移
                    size_t byte_idx = bit_pos / 8;
                    size_t bit_idx  = bit_pos % 8;
                    bool is_null    = (nb_start[byte_idx] >> bit_idx) & 1;

                    row_nulls.push_back(is_null ? 1 : 0);
                    if (is_null)
                    {
                        row_values.push_back("");
                        continue;
                    }

                    if (val_off >= row_body_len)
                        break;

                    // MySQL COM_STMT_EXECUTE binary RowData 按真实列类型解码 (共享解码器)
                    uint8_t tc  = (ci < col_types.size()) ? col_types[ci] : 0xFE;
                    bool uflag  = (ci < col_uflags.size()) && col_uflags[ci] != 0;
                    uint8_t dec = (ci < col_decimals.size()) ? col_decimals[ci] : 0;
                    std::string val_str;
                    if (!_decode_binary_value(row_body, row_body_len, val_off, tc, uflag, dec, val_str))
                        break;// 数据不足/无法解码 → 终止本行解析
                    row_values.push_back(std::move(val_str));
                }

                // 构建回调参数
                name_ptrs_cache.clear();
                for (auto &n : col_names)
                    name_ptrs_cache.push_back(n.data());

                // 用 row_values + row_nulls 做 get_data lambda:
                //   SQL NULL → (nullptr, 0); 空串等非 NULL 值 → (有效指针, len, 可为 0)
                auto captured = std::move(row_values);
                auto colnull  = std::move(row_nulls);
                bool cont     = handler(
                    static_cast<int>(expected_cols),
                    name_ptrs_cache.data(),
                    [captured, colnull](int idx) -> std::tuple<unsigned char *, size_t>
                    {
                        if (idx < 0 || idx >= static_cast<int>(captured.size()))
                            return {nullptr, 0};
                        if (idx < static_cast<int>(colnull.size()) && colnull[idx] != 0)
                            return {nullptr, 0};// SQL NULL
                        const auto &s = captured[idx];
                        return {reinterpret_cast<unsigned char *>(const_cast<char *>(s.data())), s.size()};
                    });

                rows++;
                if (!cont)
                    handler_aborted = true;// 不中断读取：继续排空到 0xFE 终结包

                consumed = row_end;
                break;
            }

            case phase::DONE:
                stop = true;
                break;
            }

            if (consumed > 256 * 1024)
            {
                accum_buf.erase(accum_buf.begin(), accum_buf.begin() + consumed);
                consumed = 0;
            }
        }

        // 读中途断连不重连: write 成功已证明当时连接是活的; 缓存 stmt_id 留着下次用
        return rows;
    }
}

unsigned int mysql_conn_base::fetch_prepared_impl_binary(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<orm::col_value_variant(int)>)> handler)
{
    unsigned int attempt = 0;
RETRY_FETCH_BIN:
    error_msg.clear();
    error_code                            = 0;
    last_insert_id_                       = 0;
    const mysql_stmt_cache_entry_t *entry = stmt_cache_.find(sql);
    bool warm                             = (entry != nullptr);
    unsigned int stmt_id;
    const std::vector<uint8_t> *param_types;
    const std::vector<uint8_t> *param_unsigned;

    if (warm)
    {
        stmt_id        = entry->stmt_id;
        param_types    = &entry->param_types;
        param_unsigned = &entry->param_unsigned;
    }
    else
    {
        stmt_id = stmt_prepare_impl(sql);
        if (stmt_id == 0)
        {
            // error_code == 21 表示 prepare 包根本没写出去, 重连一次是安全的;
            // 其余失败(响应读不到/服务器回了 ERR)包已到服务端, 一律不重连不重发。
            if (attempt < kMaxReconnect && error_code == 21 && is_last_error_reconnectable() && try_reconnect())
            {
                attempt++;
                goto RETRY_FETCH_BIN;
            }
            return 0;
        }
        uint32_t evicted = 0;
        auto &e          = stmt_cache_.upsert(sql, &evicted);
        // 淘汰即释放：stmt_id 是服务端句柄，只删 map 会让预备语句按会话累积
        // 直到全局 max_prepared_stmt_count(16382)。此处不在任何响应读取中途，
        // 且 COM_STMT_CLOSE 无响应 ⇒ 一个 9 字节包插在两条命令之间是安全的。
        if (evicted != 0)
            stmt_close_impl(evicted);
        e.stmt_id        = stmt_id;
        e.param_types    = stmt_param_types_;
        e.param_unsigned = stmt_param_unsigned_;
        e.col_types      = stmt_col_types_;
        e.col_unsigned   = stmt_col_unsigned_;
        e.col_decimals   = stmt_col_decimals_;
        e.col_names      = stmt_col_names_;
        e.col_org_names  = stmt_col_org_names_;
        entry            = &e;
        param_types      = &e.param_types;
        param_unsigned   = &e.param_unsigned;
    }

    unsigned int rows = 0;

    // 发送 COM_STMT_EXECUTE
    {
        std::vector<unsigned char> pkt_body;
        pkt_body.push_back(0x17);
        _le_write(pkt_body, stmt_id, 4);
        pkt_body.push_back(0x00);
        _le_write(pkt_body, 1, 4);

        if (params.size() != param_types->size())
        {
            error_msg  = "COM_STMT_EXECUTE: prepared statement has " + std::to_string(param_types->size()) + " placeholder(s) but got " + std::to_string(params.size()) + " parameter value(s)";
            error_code = 20;
            return 0;
        }

        write_stmt_execute_params(params, *param_types, *param_unsigned, pkt_body);

        unsigned int payload_len = pkt_body.size();

        send_data.clear();
        send_data.push_back((payload_len & 0xFF));
        send_data.push_back((payload_len >> 8 & 0xFF));
        send_data.push_back((payload_len >> 16 & 0xFF));
        send_data.push_back(0x00);
        send_data.insert(send_data.end(), pkt_body.begin(), pkt_body.end());

        if (write() == 0)
        {
            if (attempt < kMaxReconnect && is_last_error_reconnectable())
            {
                if (warm)
                    stmt_cache_drop(sql);
                if (try_reconnect())
                {
                    attempt++;
                    goto RETRY_FETCH_BIN;
                }
            }
            return 0;
        }
    }

    // 读 ResultSet: column_defs + EOF + binary RowData rows + EOF
    {
        pooled_accum_buf accum_holder;
        auto &accum_buf       = accum_holder.buf;
        unsigned int consumed = 0;

        std::vector<std::string> col_names;
        std::vector<char *> name_ptrs_cache;
        std::vector<unsigned char> col_types;   // MYSQL_TYPE per column
        std::vector<unsigned char> col_uflags;  // per column UNSIGNED_FLAG, 与 col_types 对齐
        std::vector<unsigned char> col_decimals;// per column decimals, 与 col_types 对齐
        unsigned int expected_cols = 0;
        col_org_names_.clear();// 只描述本次结果集, 不留下一次可读到的旧列名
        enum class phase
        {
            INIT,
            COLUMNS,
            ROWS,
            DONE
        };
        phase current_phase  = phase::INIT;
        bool stop            = false;
        bool handler_aborted = false;// 提前中止后继续排空到 0xFE 终结包

        auto read_more = [&]() -> bool
        {
            unsigned int n = read_loop();
            if (n == 0)
                return false;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            return true;
        };

        while (!stop)
        {
            // 确保有完整 packet header
            while (accum_buf.size() - consumed < 4)
            {
                if (!read_more())
                {
                    // 不 close：缓存 stmt_id 留着下次用
                    return rows;
                }
            }

            unsigned int pkt_len = accum_buf[consumed] |
                                   (accum_buf[consumed + 1] << 8) |
                                   (accum_buf[consumed + 2] << 16);
            unsigned int total_pkt_len = 4 + pkt_len;

            // 确保完整 packet
            while (accum_buf.size() - consumed < total_pkt_len)
            {
                if (!read_more())
                    break;
            }

            if (accum_buf.size() - consumed < total_pkt_len)
                break;

            unsigned char status = accum_buf[consumed + 4];

            switch (current_phase)
            {
            case phase::INIT:
            {
                if (status == 0xFF)
                {
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    if (attempt < kMaxReconnect && (error_code == 1243 || error_code == 1615))
                    {
                        if (warm)
                            stmt_cache_drop(sql);
                        attempt++;
                        goto RETRY_FETCH_BIN;
                    }
                    return rows;
                }
                if (status == 0x00)
                {
                    // OK packet (0x00 头) = DML/DO 等无结果集语句的最终响应。
                    // 官方 page_protocol_com_stmt_execute_response: COM_STMT_EXECUTE
                    // 响应仅 OK/ERR/Binary Resultset 三选一, SELECT 结果集首包必为
                    // column_count(lenenc, 恒 >0), 不会以 0x00 开头。
                    // 解析 affected_rows/last_insert_id 后立即结束; 旧逻辑"消费后继续
                    // 读下一包"在服务器无后续包时会阻塞读。
                    unsigned int iid = 0;
                    unsigned int ar  = parse_affected_rows_fast(&accum_buf[consumed],
                                                               total_pkt_len,
                                                               &iid);
                    if (ar != (unsigned int)-1)
                        last_insert_id_ = iid;
                    current_phase = phase::DONE;
                    stop          = true;
                    consumed += total_pkt_len;
                    break;
                }
                if (status == 0xFE)
                {
                    // 0xFE 头 (经典 EOF / OK-via-EOF) 不在 COM_STMT_EXECUTE 首包定义内
                    // (Resultset column_count 恒 >0, 无结果集语句发 OK 而非 EOF);
                    // 防御性按"空结果集结束"直接终止, 避免转入 ROWS 后等行包阻塞。
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                // 不是 OK/EOF/Error → 一定是 column count (length-encoded integer)
                // 这是 SELECT 的 ResultSet 开始
                auto [col_cnt, col_cnt_size] = _read_varint(&accum_buf[consumed + 4], 0);
                expected_cols                = static_cast<unsigned int>(col_cnt);
                col_names.reserve(expected_cols);
                col_types.reserve(expected_cols);
                col_decimals.reserve(expected_cols);
                consumed += 4 + col_cnt_size;

                if (warm && entry->col_names.size() == expected_cols &&
                    entry->col_org_names.size() == expected_cols &&
                    entry->col_types.size() == expected_cols &&
                    entry->col_unsigned.size() == expected_cols &&
                    entry->col_decimals.size() == expected_cols)
                {
                    // ★ 热路径: N 个 Column Definition 包仍要按长度消费(否则流错位), 但逐包只做
                    // "列名逐字节比对 + type/unsigned 定长读", 不构造 std::string。
                    // 不变式: 行解码只认本轮元数据, 缓存只允许省掉"分配与拷贝", 不允许省掉
                    // "本轮元数据的读取"。服务器 ALTER 后会自行 re-prepare(stmt_id 仍有效),
                    // 缓存侧收不到失效信号 ⇒ 唯一的发现手段就是这次比对。
                    // 非破坏式: 用 probe 游标探测, 全部匹配才推进 consumed; 一旦不符就回到
                    // 第一个列定义包走冷路径重解析(名字+类型), 并用本轮结果刷新 entry。
                    bool meta_same     = true;
                    unsigned int probe = consumed;
                    for (unsigned int i = 0; i < expected_cols; ++i)
                    {
                        while (accum_buf.size() - probe < 4)
                            if (!read_more())
                                break;
                        if (accum_buf.size() - probe < 4)
                        {
                            meta_same = false;
                            break;
                        }
                        unsigned int pl = accum_buf[probe] | (accum_buf[probe + 1] << 8) |
                                          (accum_buf[probe + 2] << 16);
                        while (accum_buf.size() - probe < 4 + pl)
                            if (!read_more())
                                break;
                        if (accum_buf.size() - probe < 4 + pl)
                        {
                            meta_same = false;
                            break;
                        }
                        if (!_definition_matches(&accum_buf[probe + 4], pl, entry->col_names[i], entry->col_org_names[i], entry->col_types[i], entry->col_unsigned[i], entry->col_decimals[i]))
                        {
                            meta_same = false;
                            break;
                        }
                        probe += 4 + pl;
                    }
                    if (!meta_same)
                    {
                        // consumed 未推进 ⇒ 冷路径从第一个列定义包完整解析, 行解码用本轮类型
                        current_phase = phase::COLUMNS;
                        break;
                    }
                    consumed       = probe;
                    col_names      = entry->col_names;// 本轮已逐字节校验 ⇒ 零拷贝复用是安全的
                    col_org_names_ = entry->col_org_names;
                    col_types      = entry->col_types;
                    col_uflags     = entry->col_unsigned;
                    col_decimals   = entry->col_decimals;
                    current_phase  = phase::ROWS;
                }
                else if (expected_cols == 0)
                {
                    current_phase = phase::ROWS;
                }
                else
                {
                    current_phase = phase::COLUMNS;
                }
                break;
            }

            case phase::COLUMNS:
            {
                if (status == 0xFE)
                {
                    // 硬基线（连接期门禁已要求服务器 advertise CLIENT_DEPRECATE_EOF）之下，列定义段
                    // 中途不会再有任何 0xFE 包：分隔包不存在，终结包只在全列收齐之后才发。走到这里
                    // 的前提只有"本轮列数与实际列定义包数不符 = 协议失步"，转 ROWS 会去等一个永不再来
                    // 的行包(D-10 形状)；按终结包消费并收束结果集才能保住流对齐。
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    return rows;
                }
                // column_definition packet — 列名解析出显示名/原始名, type 用 stmt_prepare 缓存
                {
                    const unsigned char *body = &accum_buf[consumed + 4];
                    std::string col_name, org_name;
                    if (!_parse_definition_names(body, static_cast<unsigned int>(total_pkt_len - 4), col_name, org_name))
                    {
                        // 定义包损坏: 显示名置空(findcolpos 返回 255 → 该列不参与映射),
                        // 但仍须按整包消费, 保持与 expected_cols 计数对齐, 否则行包会被
                        // 误判成列定义。
                        col_name.clear();
                    }
                    // 行解码只认本轮列定义包里的类型：stmt_col_types_ 是「最近一次 prepare」
                    // 的类型，ALTER 之后或换一条 SQL 都会让解码错位且不产生任何错误码(D-1/D-2)。
                    uint8_t t_code  = 0xFE;// 解析不出类型时按 STRING，与行解码越界默认一致
                    bool t_unsigned = false;
                    uint8_t t_dec   = 0;// 解析不出精度时按 0 位小数(不打印分数部分)
                    if (!_parse_definition_type(body, static_cast<unsigned int>(total_pkt_len - 4), t_code, t_unsigned, &t_dec))
                        t_code = 0xFE;
                    col_types.push_back(t_code);
                    col_uflags.push_back(t_unsigned ? 1 : 0);
                    col_decimals.push_back(t_dec);
                    col_org_names_.push_back(std::move(org_name));
                    col_names.push_back(std::move(col_name));
                }

                consumed += total_pkt_len;
                if (col_names.size() >= expected_cols)
                {
                    // 本轮列元数据回写缓存：键已存在 ⇒ update_meta 不插入、不淘汰，
                    // 因此同轮内持有的 entry 指针不会失效。类型漂移后下一轮重新变热。
                    stmt_cache_.update_meta(sql, col_types, col_uflags, col_decimals, col_names, col_org_names_);
                    current_phase = phase::ROWS;
                }
                break;
            }

            case phase::ROWS:
            {
                if (status == 0xFE)
                {
                    // EOF / EOF-via-OK packet — 结果集结束
                    // MySQL 8.4 用 OK packet 格式的 EOF (pkt_len=7) 取代传统 5 字节 EOF
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    return rows;
                }
                // 注意: 不再用 status==0x00 && pkt_len==7 判断行集结束 —
                // 单列短行(如 1 列 DATE, payload 恰 7 字节)会被误判成 OK 终结而整行丢失。
                // 行集终结符在 MySQL 8.4 (CLIENT_DEPRECATE_EOF) 为 0xFE 头的 OK-via-EOF,
                // 旧版为 0xFE 头的 5 字节 EOF, 均由上面的 status==0xFE 分支处理。

                // 首包长度 == 0xFFFFFF(满块) = 该逻辑包(行/单值)被 MySQL net 层拆成多个
                // 物理包, 即结果 >16MB。范围决定(2026-09-06): 不支持续包拼接,
                // 显式报错并排空到该行边界/终结包, 不再静默截断/变 NULL。
                if (handler_aborted || pkt_len == 0xFFFFFFu)
                {
                    if (pkt_len == 0xFFFFFFu && !handler_aborted)
                    {
                        error_msg       = "fetch_prepared: result row over 16MB (0xFFFFFF split) not supported";
                        error_code      = 20;
                        handler_aborted = true;// 后续进入排空, 直到 0xFE 终结包
                    }
                    // 排空模式：整包丢弃直到 0xFE 终结包，不再解码也不再回调。
                    // 拆包行必须先把续包全部消费到整行(逻辑包)边界, 避免把行内容
                    // 的任意字节误判成行集终结包(status 0xFE)。
                    size_t row_end = consumed + total_pkt_len;
                    if (pkt_len == 0xFFFFFFu)
                    {
                        size_t cur = row_end;
                        for (;;)
                        {
                            while (accum_buf.size() - cur < 4)
                            {
                                if (!read_more())
                                    break;
                            }
                            if (accum_buf.size() - cur < 4)
                            {
                                consumed = cur;
                                break;
                            }
                            unsigned int cplen = accum_buf[cur] |
                                                 (accum_buf[cur + 1] << 8) |
                                                 (accum_buf[cur + 2] << 16);
                            unsigned int ctotal = 4 + cplen;
                            while (accum_buf.size() - cur < ctotal)
                            {
                                if (!read_more())
                                    break;
                            }
                            if (accum_buf.size() - cur < ctotal)
                            {
                                consumed = cur;
                                break;
                            }
                            cur += ctotal;
                            if (cplen < 0xFFFFFFu)
                                break;// 短块(可空) = 该逻辑包结束
                        }
                        row_end = cur;
                    }
                    consumed = row_end;
                    break;
                }

                // binary RowData packet (单物理包内完整, pkt_len < 0xFFFFFF)
                // 格式:
                //   0x00 header
                //   null_bitmap (ceil(col_count/8) bytes, skip first bit)
                //   column values
                const unsigned char *row_body = &accum_buf[consumed + 4];
                int row_body_len              = static_cast<int>(pkt_len);
                size_t row_end                = consumed + total_pkt_len;

                if (row_body_len < 1)
                {
                    consumed = row_end;
                    break;
                }
                if (row_body[0] != 0x00)
                {
                    // 不是 binary RowData header, 跳过
                    consumed = row_end;
                    break;
                }

                // null_bitmap
                size_t nb_bytes = (expected_cols + 9) / 8;// +1 for skip header byte
                if (row_body_len < static_cast<int>(1 + nb_bytes))
                {
                    consumed = row_end;
                    break;
                }
                const unsigned char *nb_start = row_body + 1;

                // 收集每列的值 + 逐列 NULL 标志(N-8: 空串必须与 NULL 区分)
                std::vector<orm::col_value_variant> row_variants;
                row_variants.reserve(expected_cols);
                std::vector<std::string> row_date_backing(expected_cols);

                int val_off = 1 + static_cast<int>(nb_bytes);
                for (unsigned int ci = 0; ci < expected_cols; ++ci)
                {
                    // 检查 null
                    size_t bit_pos  = ci + 2;// +1 header byte + 位偏移
                    size_t byte_idx = bit_pos / 8;
                    size_t bit_idx  = bit_pos % 8;
                    bool is_null    = (nb_start[byte_idx] >> bit_idx) & 1;

                    if (is_null)
                    {
                        row_variants.push_back(orm::col_value_variant{std::monostate{}});
                        continue;
                    }

                    if (val_off >= row_body_len)
                        break;

                    // MySQL COM_STMT_EXECUTE binary RowData 按真实列类型解码 (共享解码器)
                    uint8_t tc  = (ci < col_types.size()) ? col_types[ci] : 0xFE;
                    bool uflag  = (ci < col_uflags.size()) && col_uflags[ci] != 0;
                    uint8_t dec = (ci < col_decimals.size()) ? col_decimals[ci] : 0;
                    orm::col_value_variant col_v;
                    std::string *ds = nullptr;
                    if (tc == 0x07 || tc == 0x0A || tc == 0x0B || tc == 0x0C)
                        ds = &row_date_backing[ci];
                    if (!_decode_to_variant(row_body, row_body_len, val_off, tc, uflag, dec, col_v, ds))
                        break;
                    row_variants.push_back(std::move(col_v));
                }

                // 构建回调参数
                name_ptrs_cache.clear();
                for (auto &n : col_names)
                    name_ptrs_cache.push_back(n.data());

                // 用 row_values + row_nulls 做 get_data lambda:
                //   SQL NULL → (nullptr, 0); 空串等非 NULL 值 → (有效指针, len, 可为 0)
                auto row_v = std::move(row_variants);
                bool cont  = handler(
                    static_cast<int>(expected_cols),
                    name_ptrs_cache.data(),
                    [row_v](int idx) -> orm::col_value_variant
                    {
                        if (idx < 0 || idx >= static_cast<int>(row_v.size()))
                            return orm::col_value_variant{std::monostate{}};
                        return row_v[idx];
                    });

                rows++;
                if (!cont)
                    handler_aborted = true;// 不中断读取：继续排空到 0xFE 终结包

                consumed = row_end;
                break;
            }

            case phase::DONE:
                stop = true;
                break;
            }

            if (consumed > 256 * 1024)
            {
                accum_buf.erase(accum_buf.begin(), accum_buf.begin() + consumed);
                consumed = 0;
            }
        }

        // 读中途断连不重连: write 成功已证明当时连接是活的; 缓存 stmt_id 留着下次用
        return rows;
    }
}

// ===== async_stmt_prepare / async_stmt_close（真正的 async I/O）=====
asio::awaitable<unsigned int> mysql_conn_base::async_stmt_prepare(const std::string &sql)
{
    unsigned int payload_len = 1 + sql.size();
    if (payload_len >= 0xFFFFFF)
    {
        error_msg  = "COM_STMT_PREPARE: SQL too large";
        error_code = 20;
        co_return 0;
    }

    send_data.clear();
    send_data.push_back((payload_len & 0xFF));
    send_data.push_back((payload_len >> 8 & 0xFF));
    send_data.push_back((payload_len >> 16 & 0xFF));
    send_data.push_back(0x00);
    send_data.push_back(0x16);
    send_data.append(sql);

    unsigned int n = co_await async_write();
    if (n == 0)
    {
        // 同 stmt_prepare_impl: 21 表示包根本没写出去, 上层才可以重连一次。
        error_code = 21;
        co_return 0;
    }

    pooled_accum_buf accum_holder;
    auto &accum_buf = accum_holder.buf;

    n = co_await async_read_loop();
    if (n == 0)
        co_return 0;
    accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

    while (accum_buf.size() < 4)
    {
        n = co_await async_read_loop();
        if (n == 0)
            co_return 0;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }

    unsigned int pkt_len = accum_buf[0] | (accum_buf[1] << 8) | (accum_buf[2] << 16);
    while (accum_buf.size() < 4 + pkt_len)
    {
        n = co_await async_read_loop();
        if (n == 0)
            co_return 0;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }

    unsigned char status = accum_buf[4];
    if (status == 0xFF)
    {
        // parse_error_packet 约定 data 指向包头起点(含 4B header), pkt_len 含包头
        parse_error_packet(&accum_buf[0], pkt_len + 4);
        co_return 0;
    }
    if (status != 0x00)
    {
        error_msg  = "COM_STMT_PREPARE: unexpected status " + std::to_string(status);
        error_code = 20;
        co_return 0;
    }
    if (4 + 9 > accum_buf.size())
    {
        error_msg  = "COM_STMT_PREPARE: truncated response";
        error_code = 20;
        co_return 0;
    }

    unsigned int stmt_id = 0;
    uint16_t col_count   = 0;
    uint16_t param_count = 0;
    _le_read(&accum_buf[5], 0, &stmt_id, 4);
    _le_read(&accum_buf[9], 0, &col_count, 2);
    _le_read(&accum_buf[11], 0, &param_count, 2);

    // ---- 消费 param/column definition packets (布局同 sync stmt_prepare_impl, 见其注释) ----
    // 每个 definition packet 按 3B len + 1B seq + payload 逐个消费; 连接期硬门禁已要求
    // 服务器 advertise CLIENT_DEPRECATE_EOF ⇒ 段尾/段间都没有 EOF/OK 分隔包。
    stmt_param_types_.clear();
    stmt_param_unsigned_.clear();
    stmt_col_types_.clear();
    stmt_col_unsigned_.clear();
    stmt_col_decimals_.clear();
    stmt_col_names_.clear();
    stmt_col_org_names_.clear();

    // async 补读: 返回 false 表示连接已断
    auto read_more = [&]() -> asio::awaitable<bool>
    {
        unsigned int k = co_await async_read_loop();
        if (k == 0)
            co_return false;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
        co_return true;
    };

    // 从 pos 取下一个完整 MySQL packet(pkt_payload/pkt_plen 返回, 只在取包后立即使用)
    const unsigned char *pkt_payload = nullptr;
    unsigned int pkt_plen            = 0;
    auto next_packet                 = [&](unsigned int &pos) -> asio::awaitable<bool>
    {
        while (pos + 4 > accum_buf.size())
        {
            if (!co_await read_more())
                co_return false;
        }
        unsigned int pl = accum_buf[pos] | (accum_buf[pos + 1] << 8) | (accum_buf[pos + 2] << 16);
        while (pos + 4 + pl > accum_buf.size())
        {
            if (!co_await read_more())
                co_return false;
        }
        pkt_payload = &accum_buf[pos + 4];
        pkt_plen    = pl;
        pos += 4 + pl;
        co_return true;
    };

    // 消费 count 个 Column Definition 包, 提取 type / unsigned 标志
    auto read_def_block = [&](unsigned int count, std::vector<uint8_t> &types, std::vector<uint8_t> &unsigned_flags, unsigned int &pos, std::vector<std::string> *out_names = nullptr, std::vector<std::string> *out_org_names = nullptr, std::vector<uint8_t> *out_decimals = nullptr) -> asio::awaitable<bool>
    {
        for (unsigned int i = 0; i < count; ++i)
        {
            if (!co_await next_packet(pos))
                co_return false;
            if (pkt_payload[0] == 0xFF)
            {
                parse_error_packet(pkt_payload - 4, pkt_plen + 4);
                co_return false;
            }
            if (pkt_payload[0] == 0xFE)
            {
                // def 包 payload 首字节必为 lenenc 长度(如 0x03 "def"),
                // 0xFE 只可能是段尾 EOF/OK 分隔包 → 计数与服务器不符(协议失步)
                error_msg  = "COM_STMT_PREPARE: unexpected EOF inside definition block";
                error_code = 20;
                co_return false;
            }
            uint8_t type_code = 0;
            bool is_unsigned  = false;
            uint8_t decimals  = 0;
            if (!_parse_definition_type(pkt_payload, pkt_plen, type_code, is_unsigned, &decimals))
            {
                error_msg  = "COM_STMT_PREPARE: malformed definition packet";
                error_code = 20;
                co_return false;
            }
            types.push_back(type_code);
            unsigned_flags.push_back(is_unsigned ? 1 : 0);
            if (out_decimals)
                out_decimals->push_back(decimals);
            if (out_names || out_org_names)
            {
                std::string name, org_name;
                if (!_parse_definition_names(pkt_payload, pkt_plen, name, org_name))
                {
                    error_msg  = "COM_STMT_PREPARE: malformed definition names";
                    error_code = 20;
                    co_return false;
                }
                if (out_names)
                    out_names->push_back(std::move(name));
                if (out_org_names)
                    out_org_names->push_back(std::move(org_name));
            }
        }
        co_return true;
    };

    unsigned int pos = 4 + pkt_len;// 首包(COM_STMT_PREPARE_OK)之后
    if (param_count > 0)
    {
        if (!co_await read_def_block(param_count, stmt_param_types_, stmt_param_unsigned_, pos))
            co_return 0;
    }
    if (col_count > 0)
    {
        if (!co_await read_def_block(col_count, stmt_col_types_, stmt_col_unsigned_, pos, &stmt_col_names_, &stmt_col_org_names_, &stmt_col_decimals_))
            co_return 0;
    }

    co_return stmt_id;
}

asio::awaitable<void> mysql_conn_base::async_stmt_close(unsigned int stmt_id)
{
    unsigned int payload_len = 1 + 4;

    send_data.clear();
    send_data.push_back((payload_len & 0xFF));
    send_data.push_back((payload_len >> 8 & 0xFF));
    send_data.push_back((payload_len >> 16 & 0xFF));
    send_data.push_back(0x00);
    send_data.push_back(0x19);
    _le_write(send_data, stmt_id, 4);

    try
    {
        co_await async_write();
    }
    catch (...)
    {
        // 忽略
    }
}

asio::awaitable<void> mysql_conn_base::async_stmt_cache_drop(const std::string &sql)
{
    uint32_t gone = 0;
    stmt_cache_.erase(sql, &gone);
    if (gone != 0 && !isclose)
        co_await async_stmt_close(gone);
}

// ===== async 版本（DML）=====
asio::awaitable<unsigned int> mysql_conn_base::async_exec_dml_prepared_impl(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    unsigned int attempt = 0;
RETRY_ASYNC_DML:
    error_msg.clear();
    error_code                            = 0;
    last_insert_id_                       = 0;
    const mysql_stmt_cache_entry_t *entry = stmt_cache_.find(sql);
    bool warm                             = (entry != nullptr);
    unsigned int stmt_id;
    const std::vector<uint8_t> *param_types;
    const std::vector<uint8_t> *param_unsigned;

    if (warm)
    {
        stmt_id        = entry->stmt_id;
        param_types    = &entry->param_types;
        param_unsigned = &entry->param_unsigned;
    }
    else
    {
        stmt_id = co_await async_stmt_prepare(sql);
        if (stmt_id == 0)
        {
            // error_code == 21 表示 prepare 包根本没写出去, 重连一次是安全的;
            // 其余失败(响应读不到/服务器回了 ERR)包已到服务端, 一律不重连不重发。
            if (attempt < kMaxReconnect && error_code == 21 && is_last_error_reconnectable() && co_await async_try_reconnect())
            {
                attempt++;
                goto RETRY_ASYNC_DML;
            }
            co_return (unsigned int) - 1;
        }
        uint32_t evicted = 0;
        auto &e          = stmt_cache_.upsert(sql, &evicted);
        // 淘汰即释放：stmt_id 是服务端句柄，只删 map 会让预备语句按会话累积
        // 直到全局 max_prepared_stmt_count(16382)。此处不在任何响应读取中途，
        // 且 COM_STMT_CLOSE 无响应 ⇒ 一个 9 字节包插在两条命令之间是安全的。
        if (evicted != 0)
            co_await async_stmt_close(evicted);
        e.stmt_id        = stmt_id;
        e.param_types    = stmt_param_types_;
        e.param_unsigned = stmt_param_unsigned_;
        e.col_types      = stmt_col_types_;
        e.col_unsigned   = stmt_col_unsigned_;
        e.col_decimals   = stmt_col_decimals_;
        e.col_names      = stmt_col_names_;
        e.col_org_names  = stmt_col_org_names_;
        entry            = &e;
        param_types      = &e.param_types;
        param_unsigned   = &e.param_unsigned;
    }

    unsigned int result = (unsigned int)-1;
    unsigned int iid    = 0;

    {
        std::vector<unsigned char> pkt_body;
        pkt_body.push_back(0x17);
        _le_write(pkt_body, stmt_id, 4);
        pkt_body.push_back(0x00);
        _le_write(pkt_body, 1, 4);

        if (params.size() != param_types->size())
        {
            error_msg  = "COM_STMT_EXECUTE: prepared statement has " + std::to_string(param_types->size()) + " placeholder(s) but got " + std::to_string(params.size()) + " parameter value(s)";
            error_code = 20;
            co_return (unsigned int) - 1;
        }

        write_stmt_execute_params(params, *param_types, *param_unsigned, pkt_body);

        unsigned int payload_len = pkt_body.size();
        if (payload_len >= 0xFFFFFF)
        {
            error_msg = "COM_STMT_EXECUTE: payload too large";
            // 不 close：缓存 stmt_id 留着下次用
            co_return (unsigned int) - 1;
        }

        send_data.clear();
        send_data.push_back((payload_len & 0xFF));
        send_data.push_back((payload_len >> 8 & 0xFF));
        send_data.push_back((payload_len >> 16 & 0xFF));
        send_data.push_back(0x00);
        send_data.insert(send_data.end(), pkt_body.begin(), pkt_body.end());

        unsigned int n = co_await async_write();
        if (n == 0)
        {
            // write 失败 = 连接是否还活着未知, 这是唯一需要重连的分支(同 sync 版)
            if (attempt < kMaxReconnect && is_last_error_reconnectable())
            {
                if (warm)
                    co_await async_stmt_cache_drop(sql);
                if (co_await async_try_reconnect())
                {
                    attempt++;
                    goto RETRY_ASYNC_DML;
                }
            }
            co_return (unsigned int) - 1;
        }
    }

    {
        {
            pooled_accum_buf accum_holder;
            auto &accum_buf = accum_holder.buf;

            unsigned int n = co_await async_read_loop();
            // execute 已发出, 响应丢失不等于服务端没做: 不重发 (D3)
            if (n == 0)
                co_return (unsigned int) - 1;
            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

            while (accum_buf.size() < 4)
            {
                n = co_await async_read_loop();
                if (n == 0)
                    co_return (unsigned int) - 1;
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            }

            unsigned int pkt_len = accum_buf[0] | (accum_buf[1] << 8) | (accum_buf[2] << 16);
            while (accum_buf.size() < 4 + pkt_len)
            {
                n = co_await async_read_loop();
                if (n == 0)
                    break;
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            }

            unsigned char status = accum_buf[4];
            if (status == 0xFF)
            {
                // parse_error_packet 约定 data 指向包头起点(含 4B header), pkt_len 含包头
                if (accum_buf.size() >= 4 + pkt_len)
                    parse_error_packet(&accum_buf[0], pkt_len + 4);
            }
            else if (status == 0x00)
            {
                // OK packet = DML/无结果集语句的最终响应
                unsigned int affected = parse_affected_rows_fast(accum_buf.data(), accum_buf.size(), &iid);
                if (affected != (unsigned int)-1)
                {
                    last_insert_id_ = iid;
                    result          = affected;
                }
            }
            else
            {
                // 首个响应既非 OK 也非 ERR = column count, 语句实际返回了结果集(把 SELECT
                // 误用成 exec_dml_prepared)。与 sync 版一致: 显式报错并把整个结果集排空
                // 到终结包, 避免残留字节让下一条命令错位 → 永久卡读。
                error_msg  = "async_exec_dml_prepared: statement returned a result set (not DML)";
                error_code = 10;
                result     = (unsigned int)-1;

                auto drain_read = [&]() -> asio::awaitable<bool>
                {
                    unsigned int k = co_await async_read_loop();
                    if (k == 0)
                        co_return false;
                    accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                    co_return true;
                };
                // 确保 c 起有 need 字节; 断连(读失败)返回 false
                auto ensure_avail = [&](size_t &c, size_t need) -> asio::awaitable<bool>
                {
                    while (accum_buf.size() - c < need)
                        if (!(co_await drain_read()))
                            co_return false;
                    co_return true;
                };
                // 跳过「一个逻辑包」(列定义/行), 处理 >0xFFFFFF 的满块续包链。
                // 返回 {逻辑包首包 payload 首字节, 该逻辑包起始偏移}; 读失败返回 {-1,0}。
                auto skip_logical = [&](size_t &c) -> asio::awaitable<std::pair<int, size_t>>
                {
                    std::pair<int, size_t> fail(-1, 0);
                    size_t start = c;
                    if (!(co_await ensure_avail(c, 4)))
                        co_return fail;
                    unsigned int plen = accum_buf[c] | (accum_buf[c + 1] << 8) |
                                        (accum_buf[c + 2] << 16);
                    if (!(co_await ensure_avail(c, 4 + plen)))
                        co_return fail;
                    unsigned char first = accum_buf[c + 4];
                    c += 4 + plen;
                    while (plen == 0xFFFFFFu)
                    {
                        if (!(co_await ensure_avail(c, 4)))
                            co_return fail;
                        plen = accum_buf[c] | (accum_buf[c + 1] << 8) |
                               (accum_buf[c + 2] << 16);
                        if (!(co_await ensure_avail(c, 4 + plen)))
                            co_return fail;
                        c += 4 + plen;
                    }
                    std::pair<int, size_t> ok(first, start);
                    co_return ok;
                };

                size_t cur       = 4 + pkt_len;// 首包(column count)已消费
                uint64_t col_cnt = 0;
                if (pkt_len >= 1)
                {
                    unsigned char b0 = accum_buf[4];
                    if (b0 < 0xFB)
                        col_cnt = b0;
                    else if (b0 == 0xFC && pkt_len >= 3)
                        col_cnt = accum_buf[5] | (static_cast<unsigned int>(accum_buf[6]) << 8);
                    else if (b0 == 0xFD && pkt_len >= 4)
                        col_cnt = accum_buf[5] | (static_cast<unsigned int>(accum_buf[6]) << 8) |
                                  (static_cast<unsigned int>(accum_buf[7]) << 16);
                    else if (b0 == 0xFE && pkt_len >= 9)
                    {
                        for (int i = 0; i < 8; ++i)
                            col_cnt |= static_cast<uint64_t>(accum_buf[5 + i]) << (i * 8);
                    }
                }

                bool drain_ok = true;
                // 1) column definition 包(col_cnt 个)
                for (uint64_t i = 0; i < col_cnt && drain_ok; ++i)
                {
                    auto sk = co_await skip_logical(cur);
                    if (sk.first < 0)
                    {
                        drain_ok = false;
                        break;
                    }
                    if (sk.first == 0xFF)
                    {
                        parse_error_packet(&accum_buf[sk.second],
                                           static_cast<unsigned int>(cur - sk.second));
                        drain_ok = false;
                        break;
                    }
                }
                // 2) 行直到终结包(0xFE = OK-via-EOF, 0xFF = ERR)
                while (drain_ok)
                {
                    auto sk = co_await skip_logical(cur);
                    if (sk.first < 0)
                    {
                        drain_ok = false;
                        break;
                    }
                    if (sk.first == 0xFF)
                    {
                        parse_error_packet(&accum_buf[sk.second],
                                           static_cast<unsigned int>(cur - sk.second));
                        drain_ok = false;
                        break;
                    }
                    if (sk.first == 0xFE)
                        break;// 结果集终结包
                }
            }
        }

        // 只有服务端明确拒绝了这个 stmt_id 才重跑一次（与 sync 版同判据）：
        // 1243/1615 说明语句根本没执行, 丢缓存 + 冷路径重新 prepare 是幂等安全的;
        // 能收到 ERR 包说明连接是活的, 所以这里只 re-prepare, 不重连。
        // 连接断不在此列：execute 包已经写出去了, 重发可能把 DML 落库两遍。
        if (result == (unsigned int)-1 && attempt < kMaxReconnect &&
            (error_code == 1243 || error_code == 1615))
        {
            if (warm)
                co_await async_stmt_cache_drop(sql);
            attempt++;
            goto RETRY_ASYNC_DML;
        }
        co_return result;
        co_return result;
    }
}

asio::awaitable<unsigned int> mysql_conn_base::async_exec_dml_prepared(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    co_return co_await async_exec_dml_prepared_impl(sql, params);
}

// ===== async 版本（fetch）=====
asio::awaitable<unsigned int> mysql_conn_base::async_fetch_prepared_impl(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    unsigned int attempt = 0;
RETRY_ASYNC_FETCH:
    const mysql_stmt_cache_entry_t *entry = stmt_cache_.find(sql);
    bool warm                             = (entry != nullptr);
    unsigned int stmt_id;
    const std::vector<uint8_t> *param_types;
    const std::vector<uint8_t> *param_unsigned;

    if (warm)
    {
        stmt_id        = entry->stmt_id;
        param_types    = &entry->param_types;
        param_unsigned = &entry->param_unsigned;
    }
    else
    {
        stmt_id = co_await async_stmt_prepare(sql);
        if (stmt_id == 0)
        {
            // error_code == 21 表示 prepare 包根本没写出去, 重连一次是安全的;
            // 其余失败(响应读不到/服务器回了 ERR)包已到服务端, 一律不重连不重发。
            if (attempt < kMaxReconnect && error_code == 21 && is_last_error_reconnectable() && co_await async_try_reconnect())
            {
                attempt++;
                goto RETRY_ASYNC_FETCH;
            }
            co_return 0;
        }
        uint32_t evicted = 0;
        auto &e          = stmt_cache_.upsert(sql, &evicted);
        // 淘汰即释放：stmt_id 是服务端句柄，只删 map 会让预备语句按会话累积
        // 直到全局 max_prepared_stmt_count(16382)。此处不在任何响应读取中途，
        // 且 COM_STMT_CLOSE 无响应 ⇒ 一个 9 字节包插在两条命令之间是安全的。
        if (evicted != 0)
            co_await async_stmt_close(evicted);
        e.stmt_id        = stmt_id;
        e.param_types    = stmt_param_types_;
        e.param_unsigned = stmt_param_unsigned_;
        e.col_types      = stmt_col_types_;
        e.col_unsigned   = stmt_col_unsigned_;
        e.col_decimals   = stmt_col_decimals_;
        e.col_names      = stmt_col_names_;
        e.col_org_names  = stmt_col_org_names_;
        entry            = &e;
        param_types      = &e.param_types;
        param_unsigned   = &e.param_unsigned;
    }

    if (isclose)
        co_return 0;

    error_msg.clear();
    error_code      = 0;
    last_insert_id_ = 0;

    unsigned int rows = 0;

    {
        std::vector<unsigned char> pkt_body;
        pkt_body.push_back(0x17);
        _le_write(pkt_body, stmt_id, 4);
        pkt_body.push_back(0x00);
        _le_write(pkt_body, 1, 4);

        // N-9: 参数个数与占位符数强校验(同 sync fetch_prepared_impl)
        if (params.size() != param_types->size())
        {
            error_msg  = "COM_STMT_EXECUTE: prepared statement has " + std::to_string(param_types->size()) + " placeholder(s) but got " + std::to_string(params.size()) + " parameter value(s)";
            error_code = 20;
            // 不 close：缓存 stmt_id 留着下次用
            co_return 0;
        }

        write_stmt_execute_params(params, *param_types, *param_unsigned, pkt_body);

        unsigned int payload_len = pkt_body.size();

        send_data.clear();
        send_data.push_back((payload_len & 0xFF));
        send_data.push_back((payload_len >> 8 & 0xFF));
        send_data.push_back((payload_len >> 16 & 0xFF));
        send_data.push_back(0x00);
        send_data.insert(send_data.end(), pkt_body.begin(), pkt_body.end());

        if (co_await async_write() == 0)
        {
            // write 失败 = 连接是否还活着未知, 这是本函数唯一需要重连的分支
            if (attempt < kMaxReconnect && is_last_error_reconnectable())
            {
                if (warm)
                    co_await async_stmt_cache_drop(sql);
                if (co_await async_try_reconnect())
                {
                    attempt++;
                    goto RETRY_ASYNC_FETCH;
                }
            }
            co_return 0;
        }
    }

    {
        pooled_accum_buf accum_holder;
        auto &accum_buf       = accum_holder.buf;
        unsigned int consumed = 0;

        std::vector<std::string> col_names;
        std::vector<char *> name_ptrs_cache;
        std::vector<unsigned char> col_types;
        std::vector<unsigned char> col_uflags;  // per column UNSIGNED_FLAG, 与 col_types 对齐
        std::vector<unsigned char> col_decimals;// per column decimals, 与 col_types 对齐
        unsigned int expected_cols = 0;
        col_org_names_.clear();// 只描述本次结果集, 不留下一次可读到的旧列名
        enum class phase
        {
            INIT,
            COLUMNS,
            ROWS,
            DONE
        };
        phase current_phase  = phase::INIT;
        bool stop            = false;
        bool handler_aborted = false;// 提前中止后继续排空到 0xFE 终结包

        while (!stop)
        {
            while (accum_buf.size() - consumed < 4)
            {
                unsigned int n = co_await async_read_loop();
                if (n == 0)
                {
                    // 不 close：缓存 stmt_id 留着下次用
                    co_return rows;
                }
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            }

            unsigned int pkt_len = accum_buf[consumed] |
                                   (accum_buf[consumed + 1] << 8) |
                                   (accum_buf[consumed + 2] << 16);
            unsigned int total_pkt_len = 4 + pkt_len;

            while (accum_buf.size() - consumed < total_pkt_len)
            {
                unsigned int n = co_await async_read_loop();
                if (n == 0)
                    break;
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            }

            if (accum_buf.size() - consumed < total_pkt_len)
                break;

            unsigned char status = accum_buf[consumed + 4];

            switch (current_phase)
            {
            case phase::INIT:
            {
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    if (attempt < kMaxReconnect && (error_code == 1243 || error_code == 1615))
                    {
                        if (warm)
                            co_await async_stmt_cache_drop(sql);
                        attempt++;
                        goto RETRY_ASYNC_FETCH;
                    }
                    co_return rows;
                }
                if (status == 0x00)
                {
                    // OK packet (0x00 头) = DML/DO 等无结果集语句的最终响应。
                    // 官方 page_protocol_com_stmt_execute_response: COM_STMT_EXECUTE
                    // 响应仅 OK/ERR/Binary Resultset 三选一, SELECT 结果集首包必为
                    // column_count(lenenc, 恒 >0), 不会以 0x00 开头。
                    // 解析 affected_rows/last_insert_id 后立即结束; 旧逻辑"消费后继续
                    // 读下一包"在服务器无后续包时会阻塞读。
                    unsigned int iid = 0;
                    unsigned int ar  = parse_affected_rows_fast(&accum_buf[consumed],
                                                               total_pkt_len,
                                                               &iid);
                    if (ar != (unsigned int)-1)
                        last_insert_id_ = iid;
                    current_phase = phase::DONE;
                    stop          = true;
                    consumed += total_pkt_len;
                    break;
                }
                if (status == 0xFE)
                {
                    // 0xFE 头 (经典 EOF / OK-via-EOF) 不在 COM_STMT_EXECUTE 首包定义内
                    // (Resultset column_count 恒 >0, 无结果集语句发 OK 而非 EOF);
                    // 防御性按"空结果集结束"直接终止, 避免转入 ROWS 后等行包阻塞。
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                // column count (length-encoded integer)
                auto [col_cnt, col_cnt_size] = _read_varint(&accum_buf[consumed + 4], 0);
                expected_cols                = static_cast<unsigned int>(col_cnt);
                col_names.reserve(expected_cols);
                col_types.reserve(expected_cols);
                col_decimals.reserve(expected_cols);
                consumed += 4 + col_cnt_size;

                if (warm && entry->col_names.size() == expected_cols &&
                    entry->col_org_names.size() == expected_cols &&
                    entry->col_types.size() == expected_cols &&
                    entry->col_unsigned.size() == expected_cols &&
                    entry->col_decimals.size() == expected_cols)
                {
                    // ★ 热路径: 同同步版, 逐包校验本轮元数据, 全部匹配才推进 consumed。
                    // 每次 co_await async_read_loop() 读到的字节必须 insert 进 accum_buf —
                    // 早先这 4 个读取点只看返回值判是否出错, 把读到的列定义/行字节直接丢掉,
                    // 列定义段跨 4096B 读块时 consumed 会越过缓冲区末尾(size_t 下溢)→ 挂死。
                    bool meta_same     = true;
                    unsigned int probe = consumed;
                    for (unsigned int i = 0; i < expected_cols; ++i)
                    {
                        while (accum_buf.size() - probe < 4)
                        {
                            unsigned int k = co_await async_read_loop();
                            if (k == 0)
                                break;
                            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                        }
                        if (accum_buf.size() - probe < 4)
                        {
                            meta_same = false;
                            break;
                        }
                        unsigned int pl = accum_buf[probe] | (accum_buf[probe + 1] << 8) |
                                          (accum_buf[probe + 2] << 16);
                        while (accum_buf.size() - probe < 4 + pl)
                        {
                            unsigned int k = co_await async_read_loop();
                            if (k == 0)
                                break;
                            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                        }
                        if (accum_buf.size() - probe < 4 + pl)
                        {
                            meta_same = false;
                            break;
                        }
                        if (!_definition_matches(&accum_buf[probe + 4], pl, entry->col_names[i], entry->col_org_names[i], entry->col_types[i], entry->col_unsigned[i], entry->col_decimals[i]))
                        {
                            meta_same = false;
                            break;
                        }
                        probe += 4 + pl;
                    }
                    if (!meta_same)
                    {
                        // consumed 未推进 ⇒ 冷路径从第一个列定义包完整解析, 行解码用本轮类型
                        current_phase = phase::COLUMNS;
                        break;
                    }
                    consumed       = probe;
                    col_names      = entry->col_names;// 本轮已逐字节校验 ⇒ 零拷贝复用是安全的
                    col_org_names_ = entry->col_org_names;
                    col_types      = entry->col_types;
                    col_uflags     = entry->col_unsigned;
                    col_decimals   = entry->col_decimals;
                    current_phase  = phase::ROWS;
                }
                else if (expected_cols == 0)
                {
                    current_phase = phase::ROWS;
                }
                else
                {
                    current_phase = phase::COLUMNS;
                }
                break;
            }

            case phase::COLUMNS:
            {
                if (status == 0xFE)
                {
                    // 硬基线（连接期门禁已要求服务器 advertise CLIENT_DEPRECATE_EOF）之下，列定义段
                    // 中途不会再有任何 0xFE 包：分隔包不存在，终结包只在全列收齐之后才发。走到这里
                    // 的前提只有"本轮列数与实际列定义包数不符 = 协议失步"，转 ROWS 会去等一个永不再来
                    // 的行包(D-10 形状)；按终结包消费并收束结果集才能保住流对齐。
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    co_return rows;
                }
                // column_definition packet — 列名解析出显示名/原始名, type 用 stmt_prepare 缓存
                {
                    const unsigned char *body = &accum_buf[consumed + 4];
                    std::string col_name, org_name;
                    if (!_parse_definition_names(body, static_cast<unsigned int>(total_pkt_len - 4), col_name, org_name))
                    {
                        // 定义包损坏: 显示名置空, 但仍按整包消费保持计数对齐(同同步版)
                        col_name.clear();
                    }
                    // 行解码只认本轮列定义包里的类型：stmt_col_types_ 是「最近一次 prepare」
                    // 的类型，ALTER 之后或换一条 SQL 都会让解码错位且不产生任何错误码(D-1/D-2)。
                    uint8_t t_code  = 0xFE;// 解析不出类型时按 STRING，与行解码越界默认一致
                    bool t_unsigned = false;
                    uint8_t t_dec   = 0;// 解析不出精度时按 0 位小数(不打印分数部分)
                    if (!_parse_definition_type(body, static_cast<unsigned int>(total_pkt_len - 4), t_code, t_unsigned, &t_dec))
                        t_code = 0xFE;
                    col_types.push_back(t_code);
                    col_uflags.push_back(t_unsigned ? 1 : 0);
                    col_decimals.push_back(t_dec);
                    col_org_names_.push_back(std::move(org_name));
                    col_names.push_back(std::move(col_name));
                }
                consumed += total_pkt_len;
                if (col_names.size() >= expected_cols)
                {
                    // 本轮列元数据回写缓存：键已存在 ⇒ update_meta 不插入、不淘汰，
                    // 因此同轮内持有的 entry 指针不会失效。类型漂移后下一轮重新变热。
                    stmt_cache_.update_meta(sql, col_types, col_uflags, col_decimals, col_names, col_org_names_);
                    current_phase = phase::ROWS;
                }
                break;
            }

            case phase::ROWS:
            {
                if (status == 0xFE)
                {
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    co_return rows;
                }

                // 注意: 不用 status==0x00 && pkt_len==7 判断行集结束 (会误吞 1 列短行),
                // 行集终结符一律 0xFE 头 (OK-via-EOF / 5B EOF), 由 status==0xFE 分支处理

                // 首包长度 == 0xFFFFFF(满块) = 该逻辑包(行/单值)被拆成多个物理包(结果 >16MB)。
                // 范围决定(2026-09-06): 不支持续包拼接, 显式报错并排空(同 sync 版)。
                if (handler_aborted || pkt_len == 0xFFFFFFu)
                {
                    if (pkt_len == 0xFFFFFFu && !handler_aborted)
                    {
                        error_msg       = "async_fetch_prepared: result row over 16MB (0xFFFFFF split) not supported";
                        error_code      = 20;
                        handler_aborted = true;// 后续进入排空, 直到 0xFE 终结包
                    }
                    // 排空模式：整包丢弃直到 0xFE 终结包；拆包行先把续包消费到整行
                    // (逻辑包)边界, 避免把行内容字节误判成行集终结包。
                    size_t row_end = consumed + total_pkt_len;
                    if (pkt_len == 0xFFFFFFu)
                    {
                        size_t cur = row_end;
                        for (;;)
                        {
                            while (accum_buf.size() - cur < 4)
                            {
                                unsigned int k = co_await async_read_loop();
                                if (k == 0)
                                    break;
                                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                            }
                            if (accum_buf.size() - cur < 4)
                            {
                                consumed = cur;
                                break;
                            }
                            unsigned int cplen = accum_buf[cur] |
                                                 (accum_buf[cur + 1] << 8) |
                                                 (accum_buf[cur + 2] << 16);
                            unsigned int ctotal = 4 + cplen;
                            while (accum_buf.size() - cur < ctotal)
                            {
                                unsigned int k = co_await async_read_loop();
                                if (k == 0)
                                    break;
                                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                            }
                            if (accum_buf.size() - cur < ctotal)
                            {
                                consumed = cur;
                                break;
                            }
                            cur += ctotal;
                            if (cplen < 0xFFFFFFu)
                                break;// 短块(可空) = 该逻辑包结束
                        }
                        row_end = cur;
                    }
                    consumed = row_end;
                    break;
                }

                // binary RowData packet (单物理包内完整, pkt_len < 0xFFFFFF)
                const unsigned char *row_body = &accum_buf[consumed + 4];
                int row_body_len              = static_cast<int>(pkt_len);
                size_t row_end                = consumed + total_pkt_len;

                if (row_body_len < 1)
                {
                    consumed = row_end;
                    break;
                }
                if (row_body[0] != 0x00)
                {
                    consumed = row_end;
                    break;
                }

                size_t nb_bytes = (expected_cols + 9) / 8;
                if (row_body_len < static_cast<int>(1 + nb_bytes))
                {
                    consumed = row_end;
                    break;
                }
                const unsigned char *nb_start = row_body + 1;

                // 收集每列的值 + 逐列 NULL 标志(N-8: 空串必须与 NULL 区分, 同 sync 版)
                std::vector<std::string> row_values;
                row_values.reserve(expected_cols);
                std::vector<unsigned char> row_nulls;// 与 row_values 对齐: 1=SQL NULL
                row_nulls.reserve(expected_cols);

                int val_off = 1 + static_cast<int>(nb_bytes);
                for (unsigned int ci = 0; ci < expected_cols; ++ci)
                {
                    size_t bit_pos  = ci + 2;
                    size_t byte_idx = bit_pos / 8;
                    size_t bit_idx  = bit_pos % 8;
                    bool is_null    = (nb_start[byte_idx] >> bit_idx) & 1;

                    row_nulls.push_back(is_null ? 1 : 0);
                    if (is_null)
                    {
                        row_values.push_back("");
                        continue;
                    }
                    if (val_off >= row_body_len)
                        break;

                    // 与 sync 版一致: 按真实列类型解码 (共享解码器)
                    uint8_t tc  = (ci < col_types.size()) ? col_types[ci] : 0xFE;
                    bool uflag  = (ci < col_uflags.size()) && col_uflags[ci] != 0;
                    uint8_t dec = (ci < col_decimals.size()) ? col_decimals[ci] : 0;
                    std::string val_str;
                    if (!_decode_binary_value(row_body, row_body_len, val_off, tc, uflag, dec, val_str))
                        break;// 数据不足/无法解码 → 终止本行解析
                    row_values.push_back(std::move(val_str));
                }

                name_ptrs_cache.clear();
                for (auto &n : col_names)
                    name_ptrs_cache.push_back(n.data());

                // 用 row_values + row_nulls 做 get_data lambda(同 sync 版):
                //   SQL NULL → (nullptr, 0); 空串等非 NULL 值 → (有效指针, len, 可为 0)
                auto captured = std::move(row_values);
                auto colnull  = std::move(row_nulls);
                bool cont     = handler(
                    static_cast<int>(expected_cols),
                    name_ptrs_cache.data(),
                    [captured, colnull](int idx) -> std::tuple<unsigned char *, size_t>
                    {
                        if (idx < 0 || idx >= static_cast<int>(captured.size()))
                            return {nullptr, 0};
                        if (idx < static_cast<int>(colnull.size()) && colnull[idx] != 0)
                            return {nullptr, 0};// SQL NULL
                        const auto &s = captured[idx];
                        return {reinterpret_cast<unsigned char *>(const_cast<char *>(s.data())), s.size()};
                    });

                rows++;
                if (!cont)
                    handler_aborted = true;// 不中断读取：继续排空到 0xFE 终结包

                consumed = row_end;
                break;
            }

            case phase::DONE:
                stop = true;
                break;
            }

            if (consumed > 256 * 1024)
            {
                accum_buf.erase(accum_buf.begin(), accum_buf.begin() + consumed);
                consumed = 0;
            }
        }

        // 读中途断连不重连: write 成功已证明当时连接是活的; 缓存 stmt_id 留着下次用
        co_return rows;
    }
}

bool mysql_conn_base::try_reconnect()
{
    if (!has_conn_config_)
        return false;
    if (isclose)
        return false;

    // 缓存在 hard_close() 内部统一清理（那里 socket 状态确定），不重复清
    hard_close();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return connect(last_conn_config_);
}

asio::awaitable<bool> mysql_conn_base::async_try_reconnect()
{
    // 缓存在下面的 hard_close() 内部统一清理，不重复清
    if (!has_conn_config_)
        co_return false;
    if (isclose)
        co_return false;

    co_await async_hard_close();

    asio::steady_timer t(*conn_link->io_ctx, std::chrono::milliseconds(50));
    co_await t.async_wait(asio::use_awaitable);

    co_return co_await async_connect(last_conn_config_);
}

asio::awaitable<void> mysql_conn_base::async_hard_close()
{
    std::vector<uint32_t> ids;
    stmt_cache_.clear([&ids](uint32_t id)
                      { ids.push_back(id); });
    for (uint32_t id : ids)
        co_await async_stmt_close(id);
    hard_close();
}

asio::awaitable<unsigned int> mysql_conn_base::async_fetch_prepared_impl_binary(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<orm::col_value_variant(int)>)> handler)
{
    unsigned int attempt = 0;
RETRY_ASYNC_FETCH_BIN:
    const mysql_stmt_cache_entry_t *entry = stmt_cache_.find(sql);
    bool warm                             = (entry != nullptr);
    unsigned int stmt_id;
    const std::vector<uint8_t> *param_types;
    const std::vector<uint8_t> *param_unsigned;

    if (warm)
    {
        stmt_id        = entry->stmt_id;
        param_types    = &entry->param_types;
        param_unsigned = &entry->param_unsigned;
    }
    else
    {
        stmt_id = co_await async_stmt_prepare(sql);
        if (stmt_id == 0)
        {
            // error_code == 21 表示 prepare 包根本没写出去, 重连一次是安全的;
            // 其余失败(响应读不到/服务器回了 ERR)包已到服务端, 一律不重连不重发。
            if (attempt < kMaxReconnect && error_code == 21 && is_last_error_reconnectable() && co_await async_try_reconnect())
            {
                attempt++;
                goto RETRY_ASYNC_FETCH_BIN;
            }
            co_return 0;
        }
        uint32_t evicted = 0;
        auto &e          = stmt_cache_.upsert(sql, &evicted);
        // 淘汰即释放：stmt_id 是服务端句柄，只删 map 会让预备语句按会话累积
        // 直到全局 max_prepared_stmt_count(16382)。此处不在任何响应读取中途，
        // 且 COM_STMT_CLOSE 无响应 ⇒ 一个 9 字节包插在两条命令之间是安全的。
        if (evicted != 0)
            co_await async_stmt_close(evicted);
        e.stmt_id        = stmt_id;
        e.param_types    = stmt_param_types_;
        e.param_unsigned = stmt_param_unsigned_;
        e.col_types      = stmt_col_types_;
        e.col_unsigned   = stmt_col_unsigned_;
        e.col_decimals   = stmt_col_decimals_;
        e.col_names      = stmt_col_names_;
        e.col_org_names  = stmt_col_org_names_;
        entry            = &e;
        param_types      = &e.param_types;
        param_unsigned   = &e.param_unsigned;
    }

    if (isclose)
        co_return 0;

    error_msg.clear();
    error_code      = 0;
    last_insert_id_ = 0;

    unsigned int rows = 0;

    {
        std::vector<unsigned char> pkt_body;
        pkt_body.push_back(0x17);
        _le_write(pkt_body, stmt_id, 4);
        pkt_body.push_back(0x00);
        _le_write(pkt_body, 1, 4);

        // N-9: 参数个数与占位符数强校验(同 sync fetch_prepared_impl)
        if (params.size() != param_types->size())
        {
            error_msg  = "COM_STMT_EXECUTE: prepared statement has " + std::to_string(param_types->size()) + " placeholder(s) but got " + std::to_string(params.size()) + " parameter value(s)";
            error_code = 20;
            // 不 close：缓存 stmt_id 留着下次用
            co_return 0;
        }

        write_stmt_execute_params(params, *param_types, *param_unsigned, pkt_body);

        unsigned int payload_len = pkt_body.size();

        send_data.clear();
        send_data.push_back((payload_len & 0xFF));
        send_data.push_back((payload_len >> 8 & 0xFF));
        send_data.push_back((payload_len >> 16 & 0xFF));
        send_data.push_back(0x00);
        send_data.insert(send_data.end(), pkt_body.begin(), pkt_body.end());

        if (co_await async_write() == 0)
        {
            // write 失败 = 连接是否还活着未知, 这是本函数唯一需要重连的分支
            if (attempt < kMaxReconnect && is_last_error_reconnectable())
            {
                if (warm)
                    co_await async_stmt_cache_drop(sql);
                if (co_await async_try_reconnect())
                {
                    attempt++;
                    goto RETRY_ASYNC_FETCH_BIN;
                }
            }
            co_return 0;
        }
    }

    {
        pooled_accum_buf accum_holder;
        auto &accum_buf       = accum_holder.buf;
        unsigned int consumed = 0;

        std::vector<std::string> col_names;
        std::vector<char *> name_ptrs_cache;
        std::vector<unsigned char> col_types;
        std::vector<unsigned char> col_uflags;  // per column UNSIGNED_FLAG, 与 col_types 对齐
        std::vector<unsigned char> col_decimals;// per column decimals, 与 col_types 对齐
        unsigned int expected_cols = 0;
        col_org_names_.clear();// 只描述本次结果集, 不留下一次可读到的旧列名
        enum class phase
        {
            INIT,
            COLUMNS,
            ROWS,
            DONE
        };
        phase current_phase  = phase::INIT;
        bool stop            = false;
        bool handler_aborted = false;// 提前中止后继续排空到 0xFE 终结包

        while (!stop)
        {
            while (accum_buf.size() - consumed < 4)
            {
                unsigned int n = co_await async_read_loop();
                if (n == 0)
                {
                    // 不 close：缓存 stmt_id 留着下次用
                    co_return rows;
                }
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            }

            unsigned int pkt_len = accum_buf[consumed] |
                                   (accum_buf[consumed + 1] << 8) |
                                   (accum_buf[consumed + 2] << 16);
            unsigned int total_pkt_len = 4 + pkt_len;

            while (accum_buf.size() - consumed < total_pkt_len)
            {
                unsigned int n = co_await async_read_loop();
                if (n == 0)
                    break;
                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
            }

            if (accum_buf.size() - consumed < total_pkt_len)
                break;

            unsigned char status = accum_buf[consumed + 4];

            switch (current_phase)
            {
            case phase::INIT:
            {
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    if (attempt < kMaxReconnect && (error_code == 1243 || error_code == 1615))
                    {
                        if (warm)
                            co_await async_stmt_cache_drop(sql);
                        attempt++;
                        goto RETRY_ASYNC_FETCH_BIN;
                    }
                    co_return rows;
                }
                if (status == 0x00)
                {
                    // OK packet (0x00 头) = DML/DO 等无结果集语句的最终响应。
                    // 官方 page_protocol_com_stmt_execute_response: COM_STMT_EXECUTE
                    // 响应仅 OK/ERR/Binary Resultset 三选一, SELECT 结果集首包必为
                    // column_count(lenenc, 恒 >0), 不会以 0x00 开头。
                    // 解析 affected_rows/last_insert_id 后立即结束; 旧逻辑"消费后继续
                    // 读下一包"在服务器无后续包时会阻塞读。
                    unsigned int iid = 0;
                    unsigned int ar  = parse_affected_rows_fast(&accum_buf[consumed],
                                                               total_pkt_len,
                                                               &iid);
                    if (ar != (unsigned int)-1)
                        last_insert_id_ = iid;
                    current_phase = phase::DONE;
                    stop          = true;
                    consumed += total_pkt_len;
                    break;
                }
                if (status == 0xFE)
                {
                    // 0xFE 头 (经典 EOF / OK-via-EOF) 不在 COM_STMT_EXECUTE 首包定义内
                    // (Resultset column_count 恒 >0, 无结果集语句发 OK 而非 EOF);
                    // 防御性按"空结果集结束"直接终止, 避免转入 ROWS 后等行包阻塞。
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                // column count (length-encoded integer)
                auto [col_cnt, col_cnt_size] = _read_varint(&accum_buf[consumed + 4], 0);
                expected_cols                = static_cast<unsigned int>(col_cnt);
                col_names.reserve(expected_cols);
                col_types.reserve(expected_cols);
                col_decimals.reserve(expected_cols);
                consumed += 4 + col_cnt_size;

                if (warm && entry->col_names.size() == expected_cols &&
                    entry->col_org_names.size() == expected_cols &&
                    entry->col_types.size() == expected_cols &&
                    entry->col_unsigned.size() == expected_cols &&
                    entry->col_decimals.size() == expected_cols)
                {
                    // ★ 热路径: 同同步版, 逐包校验本轮元数据, 全部匹配才推进 consumed。
                    // 每次 co_await async_read_loop() 读到的字节必须 insert 进 accum_buf —
                    // 早先这 4 个读取点只看返回值判是否出错, 把读到的列定义/行字节直接丢掉,
                    // 列定义段跨 4096B 读块时 consumed 会越过缓冲区末尾(size_t 下溢)→ 挂死。
                    bool meta_same     = true;
                    unsigned int probe = consumed;
                    for (unsigned int i = 0; i < expected_cols; ++i)
                    {
                        while (accum_buf.size() - probe < 4)
                        {
                            unsigned int k = co_await async_read_loop();
                            if (k == 0)
                                break;
                            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                        }
                        if (accum_buf.size() - probe < 4)
                        {
                            meta_same = false;
                            break;
                        }
                        unsigned int pl = accum_buf[probe] | (accum_buf[probe + 1] << 8) |
                                          (accum_buf[probe + 2] << 16);
                        while (accum_buf.size() - probe < 4 + pl)
                        {
                            unsigned int k = co_await async_read_loop();
                            if (k == 0)
                                break;
                            accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                        }
                        if (accum_buf.size() - probe < 4 + pl)
                        {
                            meta_same = false;
                            break;
                        }
                        if (!_definition_matches(&accum_buf[probe + 4], pl, entry->col_names[i], entry->col_org_names[i], entry->col_types[i], entry->col_unsigned[i], entry->col_decimals[i]))
                        {
                            meta_same = false;
                            break;
                        }
                        probe += 4 + pl;
                    }
                    if (!meta_same)
                    {
                        // consumed 未推进 ⇒ 冷路径从第一个列定义包完整解析, 行解码用本轮类型
                        current_phase = phase::COLUMNS;
                        break;
                    }
                    consumed       = probe;
                    col_names      = entry->col_names;// 本轮已逐字节校验 ⇒ 零拷贝复用是安全的
                    col_org_names_ = entry->col_org_names;
                    col_types      = entry->col_types;
                    col_uflags     = entry->col_unsigned;
                    col_decimals   = entry->col_decimals;
                    current_phase  = phase::ROWS;
                }
                else if (expected_cols == 0)
                {
                    current_phase = phase::ROWS;
                }
                else
                {
                    current_phase = phase::COLUMNS;
                }
                break;
            }

            case phase::COLUMNS:
            {
                if (status == 0xFE)
                {
                    // 硬基线（连接期门禁已要求服务器 advertise CLIENT_DEPRECATE_EOF）之下，列定义段
                    // 中途不会再有任何 0xFE 包：分隔包不存在，终结包只在全列收齐之后才发。走到这里
                    // 的前提只有"本轮列数与实际列定义包数不符 = 协议失步"，转 ROWS 会去等一个永不再来
                    // 的行包(D-10 形状)；按终结包消费并收束结果集才能保住流对齐。
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    co_return rows;
                }
                // column_definition packet — 列名解析出显示名/原始名, type 用 stmt_prepare 缓存
                {
                    const unsigned char *body = &accum_buf[consumed + 4];
                    std::string col_name, org_name;
                    if (!_parse_definition_names(body, static_cast<unsigned int>(total_pkt_len - 4), col_name, org_name))
                    {
                        // 定义包损坏: 显示名置空, 但仍按整包消费保持计数对齐(同同步版)
                        col_name.clear();
                    }
                    // 行解码只认本轮列定义包里的类型：stmt_col_types_ 是「最近一次 prepare」
                    // 的类型，ALTER 之后或换一条 SQL 都会让解码错位且不产生任何错误码(D-1/D-2)。
                    uint8_t t_code  = 0xFE;// 解析不出类型时按 STRING，与行解码越界默认一致
                    bool t_unsigned = false;
                    uint8_t t_dec   = 0;// 解析不出精度时按 0 位小数(不打印分数部分)
                    if (!_parse_definition_type(body, static_cast<unsigned int>(total_pkt_len - 4), t_code, t_unsigned, &t_dec))
                        t_code = 0xFE;
                    col_types.push_back(t_code);
                    col_uflags.push_back(t_unsigned ? 1 : 0);
                    col_decimals.push_back(t_dec);
                    col_org_names_.push_back(std::move(org_name));
                    col_names.push_back(std::move(col_name));
                }
                consumed += total_pkt_len;
                if (col_names.size() >= expected_cols)
                {
                    // 本轮列元数据回写缓存：键已存在 ⇒ update_meta 不插入、不淘汰，
                    // 因此同轮内持有的 entry 指针不会失效。类型漂移后下一轮重新变热。
                    stmt_cache_.update_meta(sql, col_types, col_uflags, col_decimals, col_names, col_org_names_);
                    current_phase = phase::ROWS;
                }
                break;
            }

            case phase::ROWS:
            {
                if (status == 0xFE)
                {
                    consumed += total_pkt_len;
                    current_phase = phase::DONE;
                    stop          = true;
                    break;
                }
                if (status == 0xFF)
                {
                    // parse_error_packet 约定 data 指向包头起点(含 4B), pkt_len 含包头
                    parse_error_packet(&accum_buf[consumed], total_pkt_len);
                    // 不 close：缓存 stmt_id 留着下次用
                    co_return rows;
                }

                // 注意: 不用 status==0x00 && pkt_len==7 判断行集结束 (会误吞 1 列短行),
                // 行集终结符一律 0xFE 头 (OK-via-EOF / 5B EOF), 由 status==0xFE 分支处理

                // 首包长度 == 0xFFFFFF(满块) = 该逻辑包(行/单值)被拆成多个物理包(结果 >16MB)。
                // 范围决定(2026-09-06): 不支持续包拼接, 显式报错并排空(同 sync 版)。
                if (handler_aborted || pkt_len == 0xFFFFFFu)
                {
                    if (pkt_len == 0xFFFFFFu && !handler_aborted)
                    {
                        error_msg       = "async_fetch_prepared: result row over 16MB (0xFFFFFF split) not supported";
                        error_code      = 20;
                        handler_aborted = true;// 后续进入排空, 直到 0xFE 终结包
                    }
                    // 排空模式：整包丢弃直到 0xFE 终结包；拆包行先把续包消费到整行
                    // (逻辑包)边界, 避免把行内容字节误判成行集终结包。
                    size_t row_end = consumed + total_pkt_len;
                    if (pkt_len == 0xFFFFFFu)
                    {
                        size_t cur = row_end;
                        for (;;)
                        {
                            while (accum_buf.size() - cur < 4)
                            {
                                unsigned int k = co_await async_read_loop();
                                if (k == 0)
                                    break;
                                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                            }
                            if (accum_buf.size() - cur < 4)
                            {
                                consumed = cur;
                                break;
                            }
                            unsigned int cplen = accum_buf[cur] |
                                                 (accum_buf[cur + 1] << 8) |
                                                 (accum_buf[cur + 2] << 16);
                            unsigned int ctotal = 4 + cplen;
                            while (accum_buf.size() - cur < ctotal)
                            {
                                unsigned int k = co_await async_read_loop();
                                if (k == 0)
                                    break;
                                accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + k);
                            }
                            if (accum_buf.size() - cur < ctotal)
                            {
                                consumed = cur;
                                break;
                            }
                            cur += ctotal;
                            if (cplen < 0xFFFFFFu)
                                break;// 短块(可空) = 该逻辑包结束
                        }
                        row_end = cur;
                    }
                    consumed = row_end;
                    break;
                }

                // binary RowData packet (单物理包内完整, pkt_len < 0xFFFFFF)
                const unsigned char *row_body = &accum_buf[consumed + 4];
                int row_body_len              = static_cast<int>(pkt_len);
                size_t row_end                = consumed + total_pkt_len;

                if (row_body_len < 1)
                {
                    consumed = row_end;
                    break;
                }
                if (row_body[0] != 0x00)
                {
                    consumed = row_end;
                    break;
                }

                size_t nb_bytes = (expected_cols + 9) / 8;
                if (row_body_len < static_cast<int>(1 + nb_bytes))
                {
                    consumed = row_end;
                    break;
                }
                const unsigned char *nb_start = row_body + 1;

                // 收集每列的值 + 逐列 NULL 标志(N-8: 空串必须与 NULL 区分, 同 sync 版)
                std::vector<orm::col_value_variant> row_variants;
                row_variants.reserve(expected_cols);
                std::vector<std::string> row_date_backing(expected_cols);

                int val_off = 1 + static_cast<int>(nb_bytes);
                for (unsigned int ci = 0; ci < expected_cols; ++ci)
                {
                    size_t bit_pos  = ci + 2;
                    size_t byte_idx = bit_pos / 8;
                    size_t bit_idx  = bit_pos % 8;
                    bool is_null    = (nb_start[byte_idx] >> bit_idx) & 1;

                    if (is_null)
                    {
                        row_variants.push_back(orm::col_value_variant{std::monostate{}});
                        continue;
                    }
                    if (val_off >= row_body_len)
                        break;

                    // 与 sync 版一致: 按真实列类型解码 (共享解码器)
                    uint8_t tc  = (ci < col_types.size()) ? col_types[ci] : 0xFE;
                    bool uflag  = (ci < col_uflags.size()) && col_uflags[ci] != 0;
                    uint8_t dec = (ci < col_decimals.size()) ? col_decimals[ci] : 0;
                    orm::col_value_variant col_v;
                    std::string *ds = nullptr;
                    if (tc == 0x07 || tc == 0x0A || tc == 0x0B || tc == 0x0C)
                        ds = &row_date_backing[ci];
                    if (!_decode_to_variant(row_body, row_body_len, val_off, tc, uflag, dec, col_v, ds))
                        break;
                    row_variants.push_back(std::move(col_v));
                }

                name_ptrs_cache.clear();
                for (auto &n : col_names)
                    name_ptrs_cache.push_back(n.data());

                // 用 row_values + row_nulls 做 get_data lambda(同 sync 版):
                //   SQL NULL → (nullptr, 0); 空串等非 NULL 值 → (有效指针, len, 可为 0)
                auto row_v = std::move(row_variants);
                bool cont  = handler(
                    static_cast<int>(expected_cols),
                    name_ptrs_cache.data(),
                    [row_v](int idx) -> orm::col_value_variant
                    {
                        if (idx < 0 || idx >= static_cast<int>(row_v.size()))
                            return orm::col_value_variant{std::monostate{}};
                        return row_v[idx];
                    });

                rows++;
                if (!cont)
                    handler_aborted = true;// 不中断读取：继续排空到 0xFE 终结包

                consumed = row_end;
                break;
            }

            case phase::DONE:
                stop = true;
                break;
            }

            if (consumed > 256 * 1024)
            {
                accum_buf.erase(accum_buf.begin(), accum_buf.begin() + consumed);
                consumed = 0;
            }
        }

        // 读中途断连不重连: write 成功已证明当时连接是活的; 缓存 stmt_id 留着下次用
        co_return rows;
    }
}

}// namespace orm
