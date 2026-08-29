#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <ctime>
#include <chrono>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/core_names.h>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "pg_conn.h"
#include "clientdatacache.h"

namespace orm
{

pg_conn_base::pg_conn_base(std::unique_ptr<orm_conn_link_t> link) : conn_link(std::move(link))
{
    error_code  = 0;
    time_start  = time((time_t *)NULL);
    query_num   = 0;
    auto &cc    = http::get_client_data_cache();
    _cache_data = cc.get_data_ptr();
}

pg_conn_base::~pg_conn_base()
{
    if (isclose == false)
    {
        if (conn_link)
        {
            try
            {
                conn_link->close();
            }
            catch (const std::exception &)
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

static const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const unsigned char *data, size_t len)
{
    std::string ret;
    int i = 0, j = 0;
    unsigned char char_array_3[3], char_array_4[4];
    while (len--)
    {
        char_array_3[i++] = *(data++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];
        while (i++ < 3)
            ret += '=';
    }
    return ret;
}

std::string base64_decode(const std::string &input)
{
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
        T[base64_chars[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : input)
    {
        if (T[c] == -1)
            break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string sha256(const std::string &data)
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    EVP_Digest(data.data(), data.size(), hash, &len, EVP_sha256(), nullptr);
    return std::string(reinterpret_cast<char *>(hash), len);
}

std::string hmac_sha256(const std::string &key, const std::string &data)
{
    EVP_MAC *mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);
    if (!mac)
        throw std::runtime_error("EVP_MAC_fetch HMAC failed");
    EVP_MAC_CTX *ctx = EVP_MAC_CTX_new(mac);
    EVP_MAC_free(mac);
    if (!ctx)
        throw std::runtime_error("EVP_MAC_CTX_new failed");

    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char *>("SHA256"), 0);
    params[1] = OSSL_PARAM_construct_end();

    EVP_MAC_init(ctx, reinterpret_cast<const unsigned char *>(key.data()), key.size(), params);
    EVP_MAC_update(ctx, reinterpret_cast<const unsigned char *>(data.data()), data.size());

    unsigned char result[EVP_MAX_MD_SIZE];
    size_t result_len = 0;
    EVP_MAC_final(ctx, result, &result_len, sizeof(result));
    EVP_MAC_CTX_free(ctx);

    return std::string(reinterpret_cast<char *>(result), result_len);
}

std::string pbkdf2_hmac_sha256(const std::string &password, const std::string &salt, int iterations, size_t dklen)
{
    EVP_KDF *kdf = EVP_KDF_fetch(nullptr, "PBKDF2", nullptr);
    if (!kdf)
        throw std::runtime_error("EVP_KDF_fetch PBKDF2 failed");
    EVP_KDF_CTX *ctx = EVP_KDF_CTX_new(kdf);
    EVP_KDF_free(kdf);
    if (!ctx)
        throw std::runtime_error("EVP_KDF_CTX_new failed");

    OSSL_PARAM params[5];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char *>("SHA256"), 0);
    params[1] = OSSL_PARAM_construct_octet_string("pass", const_cast<char *>(password.data()), password.size());
    params[2] = OSSL_PARAM_construct_octet_string("salt", const_cast<char *>(salt.data()), salt.size());
    params[3] = OSSL_PARAM_construct_int("iter", &iterations);
    params[4] = OSSL_PARAM_construct_end();

    std::string derived(dklen, '\0');
    if (EVP_KDF_derive(ctx, reinterpret_cast<unsigned char *>(derived.data()), dklen, params) <= 0)
    {
        EVP_KDF_CTX_free(ctx);
        throw std::runtime_error("EVP_KDF_derive failed");
    }
    EVP_KDF_CTX_free(ctx);
    return derived;
}

std::string generate_nonce(size_t len = 18)
{
    if (len > 18)
        len = 18;
    unsigned char buf[18];
    RAND_bytes(buf, len);
    return base64_encode(buf, len);
}

std::string xor_strings(const std::string &a, const std::string &b)
{
    if (a.size() > b.size())
        return a;
    std::string res;
    res.resize(a.size());
    for (size_t i = 0; i < a.size(); ++i)
        res[i] = a[i] ^ b[i];
    return res;
}

unsigned int pg_conn_base::read_pack(unsigned char *data, unsigned int offset)
{
    // NOTE: 此函数为遗留/死代码（未被调用），缺少缓冲区总长度参数无法做边界检查。
    // PG 协议使用 peek_pg_message() 进行消息解析，该函数不再使用。
    unsigned int pack_length = buf_to_int32(data + offset);
    offset                   = offset + 4;
    offset                   = offset + pack_length;
    return offset;
}

void pg_conn_base::read_col_info(const std::string &pack_info, field_info_t &temp_field_data)
{
    size_t pos = 0;

    size_t null_pos = pack_info.find('\0', pos);
    if (null_pos != std::string::npos)
    {
        temp_field_data.name = pack_info.substr(pos, null_pos - pos);
        pos                  = null_pos + 1;
    }

    null_pos = pack_info.find('\0', pos);
    if (null_pos != std::string::npos)
    {
        temp_field_data.table = pack_info.substr(pos, null_pos - pos);
        pos                   = null_pos + 1;
    }

    null_pos = pack_info.find('\0', pos);
    if (null_pos != std::string::npos)
    {
        temp_field_data.org_table = pack_info.substr(pos, null_pos - pos);
        pos                       = null_pos + 1;
    }

    null_pos = pack_info.find('\0', pos);
    if (null_pos != std::string::npos)
    {
        temp_field_data.org_name = pack_info.substr(pos, null_pos - pos);
        pos                      = null_pos + 1;
    }

    null_pos = pack_info.find('\0', pos);
    if (null_pos != std::string::npos)
    {
        temp_field_data.dbname = pack_info.substr(pos, null_pos - pos);
        pos                    = null_pos + 1;
    }

    if (pos + 4 <= pack_info.size())
    {
        temp_field_data.column_length = buf_to_int32(reinterpret_cast<const unsigned char *>(pack_info.data() + pos));
        pos += 4;
    }

    if (pos + 2 <= pack_info.size())
    {
        temp_field_data.charset = (pack_info[pos] << 8) | pack_info[pos + 1];
        pos += 2;
    }

    if (pos + 4 <= pack_info.size())
    {
        temp_field_data.field_type = pack_info[pos + 3];
        pos += 4;
    }

    if (pos + 2 <= pack_info.size())
    {
        temp_field_data.flags = (pack_info[pos] << 8) | pack_info[pos + 1];
        pos += 2;
    }

    if (pos + 2 <= pack_info.size())
    {
        temp_field_data.decimals = pack_info[pos];
    }
}

bool pg_conn_base::pack_eof_check(const pack_info_t &temp_pack_data)
{
    return temp_pack_data.error == 0;
}

bool pg_conn_base::read_full_sync(unsigned char *buf, size_t len)
{
    asio::error_code ec;
    size_t n = 0;
    if (conn_link->sock_type == 2)
    {
        n = asio::read(*conn_link->sslsocket, asio::buffer(buf, len), ec);
    }
    else if (conn_link->sock_type == 3)
    {
        n = asio::read(*conn_link->localsocket, asio::buffer(buf, len), ec);
    }
    else
    {
        n = asio::read(*conn_link->socket, asio::buffer(buf, len), ec);
    }
    if (ec || n != len)
    {
        error_msg  = "read error: " + ec.message();
        error_code = 7;
        isclose    = true;
        return false;
    }
    return true;
}

bool pg_conn_base::read_message_sync(unsigned char &msg_type, std::string &payload)
{
    if (!read_full_sync(&msg_type, 1))
        return false;
    unsigned char len_buf[4];
    if (!read_full_sync(len_buf, 4))
        return false;
    int32_t msg_len = buf_to_int32(len_buf);
    if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
    {
        error_msg  = "invalid message length";
        error_code = 8;
        isclose    = true;
        return false;
    }
    int32_t data_len = msg_len - 4;
    if (data_len > 0)
    {
        std::vector<unsigned char> tmp(data_len);
        if (!read_full_sync(tmp.data(), data_len))
            return false;
        payload.assign(reinterpret_cast<char *>(tmp.data()), data_len);
    }
    else
    {
        payload.clear();
    }
    return true;
}

bool pg_conn_base::send_message_sync(unsigned char type, const std::string &payload)
{
    int32_t len = 4 + payload.size();
    unsigned char header[5];
    header[0] = type;
    int32_to_buf(len, header + 1);
    asio::error_code ec;
    if (conn_link->sock_type == 2)
    {
        asio::write(*conn_link->sslsocket, asio::buffer(header, 5), ec);
        if (!ec)
            asio::write(*conn_link->sslsocket, asio::buffer(payload), ec);
    }
    else if (conn_link->sock_type == 3)
    {
        asio::write(*conn_link->localsocket, asio::buffer(header, 5), ec);
        if (!ec)
            asio::write(*conn_link->localsocket, asio::buffer(payload), ec);
    }
    else
    {
        asio::write(*conn_link->socket, asio::buffer(header, 5), ec);
        if (!ec)
            asio::write(*conn_link->socket, asio::buffer(payload), ec);
    }
    if (ec)
    {
        error_msg  = "send error: " + ec.message();
        error_code = 9;
        isclose    = true;
        return false;
    }
    return true;
}

void pg_conn_base::parse_error(const std::string &payload)
{
    size_t pos = 0;
    while (pos < payload.size())
    {
        char field = payload[pos++];
        if (field == '\0')
            break;
        // 读取 value 直到 null 终止符或 payload 结束
        std::string value;
        while (pos < payload.size() && payload[pos] != '\0')
            value.push_back(payload[pos++]);
        if (pos >= payload.size())
        {
            // 消息截断：value 无 null 终止符，直接退出避免跳过不存在的 null
            break;
        }
        pos++;  // 跳过 null 终止符
        if (field == 'M')
            error_msg = value;
    }
    if (error_msg.empty())
        error_msg = "unknown error";
    error_code = 11;
    isclose    = true;
}

bool pg_conn_base::handle_auth(const std::string &payload, const orm_conn_t &conn_config)
{
    if (payload.size() < 4)
    {
        error_msg  = "bad Authentication message";
        error_code = 12;
        isclose    = true;
        return false;
    }
    int32_t auth_type = buf_to_int32(reinterpret_cast<const unsigned char *>(payload.data()));
    switch (auth_type)
    {
    case 0: return true;
    case 10: return sasl_scram_sha256_sync(payload.substr(4), conn_config);
    default:
        error_msg  = "unsupported authentication method: " + std::to_string(auth_type) + " (PostgreSQL 14+ requires SCRAM-SHA-256)";
        error_code = 14;
        isclose    = true;
        return false;
    }
}

bool pg_conn_base::sasl_scram_sha256_sync(const std::string &mechanisms, const orm_conn_t &conn_config)
{
    if (mechanisms.find("SCRAM-SHA-256") == std::string::npos)
    {
        error_msg  = "SCRAM-SHA-256 not offered by server";
        error_code = 15;
        isclose    = true;
        return false;
    }

    std::string client_nonce         = generate_nonce(18);
    std::string client_first_bare    = "n=" + conn_config.user + ",r=" + client_nonce;
    std::string client_first_message = "n,," + client_first_bare;

    std::string sasl_init;
    sasl_init.append("SCRAM-SHA-256");
    sasl_init.push_back('\0');
    int32_t sasl_data_len = client_first_message.size();
    unsigned char len_buf[4];
    int32_to_buf(sasl_data_len, len_buf);
    sasl_init.append(reinterpret_cast<char *>(len_buf), 4);
    sasl_init.append(client_first_message);

    if (!send_message_sync('p', sasl_init))
        return false;

    unsigned char msg_type;
    std::string payload;
    if (!read_message_sync(msg_type, payload) || msg_type != 'R')
        return false;
    if (payload.size() < 4 || buf_to_int32(reinterpret_cast<const unsigned char *>(payload.data())) != 11)
    {
        error_msg  = "expected SASL continue";
        error_code = 16;
        isclose    = true;
        return false;
    }
    std::string server_first_msg = payload.substr(4);

    std::string server_nonce, salt_b64;
    int iteration = 0;
    size_t pos    = 0;
    while (pos < server_first_msg.size())
    {
        char key = server_first_msg[pos++];
        if (pos >= server_first_msg.size())
            break;
        std::string value;
        while (pos < server_first_msg.size() && server_first_msg[pos] != ',')
            value.push_back(server_first_msg[pos++]);
        pos++;
        if (key == 'r')
            server_nonce = value;
        else if (key == 's')
            salt_b64 = value;
        else if (key == 'i')
        {
            try
            {
                iteration = std::stoi(value);
            }
            catch (...)
            {
                iteration = -1;
            }
        }
    }

    if (server_nonce.empty() || salt_b64.empty() || iteration <= 0)
    {
        error_msg  = "invalid server-first-message";
        error_code = 17;
        isclose    = true;
        return false;
    }
    if (server_nonce.rfind(client_nonce, 0) != 0)
    {
        error_msg  = "server nonce does not start with client nonce";
        error_code = 18;
        isclose    = true;
        return false;
    }

    std::string salt            = base64_decode(salt_b64);
    std::string salted_password = pbkdf2_hmac_sha256(conn_config.password, salt, iteration, 32);

    std::string channel_binding            = "c=biws";
    std::string nonce_part                 = "r=" + server_nonce;
    std::string client_final_without_proof = channel_binding + "," + nonce_part;
    std::string auth_message               = client_first_bare + "," + server_first_msg + "," + client_final_without_proof;

    std::string client_key       = hmac_sha256(salted_password, "Client Key");
    std::string stored_key       = sha256(client_key);
    std::string client_signature = hmac_sha256(stored_key, auth_message);
    std::string client_proof     = xor_strings(client_key, client_signature);
    std::string client_proof_b64 = base64_encode(
        reinterpret_cast<const unsigned char *>(client_proof.data()),
        client_proof.size());

    std::string client_final = client_final_without_proof + ",p=" + client_proof_b64;
    if (!send_message_sync('p', client_final))
        return false;

    if (!read_message_sync(msg_type, payload) || msg_type != 'R')
        return false;

    if (payload.size() < 4)
    {
        error_msg = "SCRAM auth response too short";
        isclose   = true;
        return false;
    }
    int32_t auth_type = buf_to_int32(reinterpret_cast<const unsigned char *>(payload.data()));
    if (auth_type == 0)
    {
        return true;
    }
    else if (auth_type == 12)
    {
        if (payload.size() < 4)
        {
            error_msg = "auth payload too short";
            isclose   = true;
            return false;
        }
        std::string server_final = payload.substr(4);
        std::string server_signature_b64;
        pos = 0;
        while (pos < server_final.size())
        {
            char key = server_final[pos++];
            if (key == 'v')
            {
                while (pos < server_final.size() && server_final[pos] != ',')
                    server_signature_b64.push_back(server_final[pos++]);
                break;
            }
            while (pos < server_final.size() && server_final[pos] != ',')
                pos++;
            if (pos < server_final.size())
                pos++;
        }
        if (server_signature_b64.empty())
        {
            error_msg  = "missing server signature";
            error_code = 19;
            isclose    = true;
            return false;
        }

        std::string server_key              = hmac_sha256(salted_password, "Server Key");
        std::string expected_server_sig     = hmac_sha256(server_key, auth_message);
        std::string expected_server_sig_b64 = base64_encode(
            reinterpret_cast<const unsigned char *>(expected_server_sig.data()),
            expected_server_sig.size());

        if (server_signature_b64 != expected_server_sig_b64)
        {
            error_msg  = "server signature mismatch";
            error_code = 20;
            isclose    = true;
            return false;
        }
        return true;
    }
    else
    {
        error_msg  = "unexpected authentication step";
        error_code = 21;
        isclose    = true;
        return false;
    }
}

bool pg_conn_base::process_server_messages_until_ready_sync(const orm_conn_t &conn_config)
{
    unsigned char msg_type;
    std::string payload;
    bool auth_done = false;

    while (!isclose)
    {
        if (!read_message_sync(msg_type, payload))
            return false;

        switch (msg_type)
        {
        case 'R':
            if (!handle_auth(payload, conn_config))
                return false;
            auth_done = true;
            break;
        case 'K':
        {
            // BackendKeyData: Int32 length + Int32 backend_pid + Int32 cancel_secret
            if (payload.size() >= 8)
            {
                const unsigned char *p = reinterpret_cast<const unsigned char *>(payload.data());
                // 经 uint32_t 移位，避免字节 >= 0x80 时有符号溢出 UB
                uint32_t pid = (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
                               (static_cast<uint32_t>(p[2]) << 8) | p[3];
                uint32_t secret = (static_cast<uint32_t>(p[4]) << 24) | (static_cast<uint32_t>(p[5]) << 16) |
                                  (static_cast<uint32_t>(p[6]) << 8) | p[7];
                backend_pid_   = static_cast<int>(pid);
                cancel_secret_ = static_cast<int>(secret);
            }
            break;
        }
        case 'S':
        {
            // ParameterStatus: Int32 length + null-terminated name + null-terminated value
            const unsigned char *p = reinterpret_cast<const unsigned char *>(payload.data());
            const unsigned char *end = p + payload.size();
            const char *name_start = reinterpret_cast<const char *>(p);
            const char *name_end = static_cast<const char *>(memchr(name_start, '\0', end - p));
            if (name_end && name_end < reinterpret_cast<const char *>(end))
            {
                std::string param_name(name_start, name_end);
                const char *val_start = name_end + 1;
                const char *val_end = static_cast<const char *>(memchr(val_start, '\0', end - reinterpret_cast<const unsigned char *>(val_start)));
                std::string param_value(val_start, val_end ? val_end : reinterpret_cast<const char *>(end));
                if (param_name == "client_encoding")
                {
                    server_client_encoding_ = param_value;
                }
            }
            break;
        }
        case 'Z':
            if (!auth_done)
            {
                error_msg  = "server sent ReadyForQuery before authentication";
                error_code = 10;
                isclose    = true;
                return false;
            }
            // 认证成功建立连接时必为空闲态；后续查询循环按 'Z' 状态字节持续更新
            in_transaction_.store(false);
            return true;
        case 'E':
            parse_error(payload);
            return false;
        case 'N':
        {
            const unsigned char *np = reinterpret_cast<const unsigned char *>(payload.data());
            const unsigned char *ne = np + payload.size();
            if (np < ne && *np == 'C')
            {
                const char *p = reinterpret_cast<const char *>(np + 1);
                while (p < reinterpret_cast<const char *>(ne))
                {
                    char field = *p++;
                    const char *val_start = p;
                    const char *val_end = static_cast<const char *>(memchr(val_start, '\0', reinterpret_cast<const char *>(ne) - val_start));
                    if (field == 'S' && val_end)
                    {
                        notices_.emplace_back(val_start, val_end);
                        break;
                    }
                    if (!val_end) break;
                    p = val_end + 1;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    return false;
}

bool pg_conn_base::connect(const orm_conn_t &conn_config)
{
    error_msg.clear();
    error_code = 0;
    isclose    = false;

    // 重连场景：彻底重置连接层状态
    conn_link->sock_type  = 0;
    conn_link->sslsocket.reset();
    conn_link->ssl_context.reset();
    conn_link->localsocket.reset();
    conn_link->socket     = std::make_unique<asio::ip::tcp::socket>(*conn_link->io_ctx);
    conn_link->ec.clear();
    server_enable_ssl     = false;
    in_transaction_.store(false);
    notices_.clear();

    // 保存配置用于断线自动重连（仅在首次或配置变化时存储）
    if (!has_conn_config_ || !(last_conn_config_ == conn_config)) {
        last_conn_config_ = conn_config;
        has_conn_config_ = true;
    }

    asio::error_code ec;
    if (conn_config.issock)
    {
        // Unix domain socket connection
        try
        {
            conn_link->localsocket = std::make_unique<asio::local::stream_protocol::socket>(*conn_link->io_ctx);
            asio::local::stream_protocol::endpoint ep(conn_config.host);
            conn_link->localsocket->connect(ep, ec);
        }
        catch (const std::exception &e)
        {
            error_msg = "local socket connect error: ";
            error_msg.append(e.what());
            error_code = 1;
            isclose    = true;
            return false;
        }
        if (ec)
        {
            error_msg  = "local socket connect error: " + ec.message();
            error_code = 2;
            isclose    = true;
            return false;
        }
        conn_link->sock_type = 3;
    }
    else
    {
        asio::ip::tcp::resolver resolver(*conn_link->io_ctx);
        auto endpoints = resolver.resolve(conn_config.host, conn_config.port, ec);
        if (ec)
        {
            error_msg  = "resolve error: " + ec.message();
            error_code = 1;
            isclose    = true;
            return false;
        }
        asio::connect(*conn_link->socket, endpoints, ec);
        if (ec)
        {
            error_msg  = "connect error: " + ec.message();
            error_code = 2;
            isclose    = true;
            return false;
        }
        conn_link->sock_type = 1;
    }

    server_enable_ssl = false;
    if (conn_config.isssl && !conn_config.issock)
    {
        unsigned char ssl_req[8];
        int32_to_buf(8, ssl_req);
        int32_to_buf(PG_SSL_REQUEST_CODE, ssl_req + 4);
        asio::write(*conn_link->socket, asio::buffer(ssl_req, 8), ec);
        if (ec)
        {
            error_msg  = "SSLRequest write error: " + ec.message();
            error_code = 3;
            isclose    = true;
            return false;
        }

        unsigned char ssl_response;
        asio::read(*conn_link->socket, asio::buffer(&ssl_response, 1), ec);
        if (ec)
        {
            error_msg  = "SSL response read error: " + ec.message();
            error_code = 4;
            isclose    = true;
            return false;
        }

        if (ssl_response == 'S')
        {
            conn_link->ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
            if (conn_config.sslverify)
            {
                conn_link->ssl_context->set_verify_mode(asio::ssl::verify_peer);
                conn_link->ssl_context->set_default_verify_paths();
            }
            conn_link->sslsocket = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(
                std::move(*conn_link->socket.release()),
                *conn_link->ssl_context);
            const std::string &sni_host = conn_config.sslhost.empty() ? conn_config.host : conn_config.sslhost;
            if (!sni_host.empty())
            {
                SSL_set_tlsext_host_name(conn_link->sslsocket->native_handle(), sni_host.c_str());
            }
            if (conn_config.sslverify && !conn_config.sslhost.empty())
            {
                conn_link->sslsocket->set_verify_callback(asio::ssl::host_name_verification(conn_config.sslhost));
            }
            conn_link->sslsocket->handshake(asio::ssl::stream_base::client, ec);
            if (ec)
            {
                error_msg  = "SSL handshake error: " + ec.message();
                error_code = 5;
                isclose    = true;
                return false;
            }
            conn_link->sock_type = 2;
            server_enable_ssl    = true;
        }
        else
        {
            // 请求 SSL 但服务器回 'N' 不支持 —— 不允许静默降级
            if (conn_config.isssl)
            {
                error_msg  = "server does not support SSL (requested by client config)";
                error_code = 5;
                isclose    = true;
                return false;
            }
            server_enable_ssl = false;
        }
    }

    std::string startup;
    startup.append(4, '\0');
    int32_t proto_ver = PG_PROTOCOL_3_0;
    unsigned char ver_buf[4];
    int32_to_buf(proto_ver, ver_buf);
    startup.append(reinterpret_cast<char *>(ver_buf), 4);

    auto add_pair = [&](const std::string &key, const std::string &val)
    {
        startup.append(key);
        startup.push_back('\0');
        startup.append(val);
        startup.push_back('\0');
    };
    add_pair("user", conn_config.user);
    if (!conn_config.dbname.empty())
        add_pair("database", conn_config.dbname);
    add_pair("client_encoding", "UTF8");
    startup.push_back('\0');

    int32_t total_len = startup.size();
    unsigned char len_buf[4];
    int32_to_buf(total_len, len_buf);
    startup.replace(0, 4, reinterpret_cast<char *>(len_buf), 4);

    if (conn_link->sock_type == 2)
    {
        asio::write(*conn_link->sslsocket, asio::buffer(startup), ec);
    }
    else if (conn_link->sock_type == 3)
    {
        asio::write(*conn_link->localsocket, asio::buffer(startup), ec);
    }
    else
    {
        asio::write(*conn_link->socket, asio::buffer(startup), ec);
    }
    if (ec)
    {
        error_msg  = "StartupMessage write error: " + ec.message();
        error_code = 6;
        isclose    = true;
        return false;
    }

    return process_server_messages_until_ready_sync(conn_config);
}

bool pg_conn_base::sasl_scram_sha256_async(const std::string &mechanisms, const orm_conn_t &conn_config)
{
    if (mechanisms.find("SCRAM-SHA-256") == std::string::npos)
    {
        error_msg  = "SCRAM-SHA-256 not offered by server";
        error_code = 15;
        isclose    = true;
        return false;
    }

    std::string client_nonce         = generate_nonce(18);
    std::string client_first_bare    = "n=" + conn_config.user + ",r=" + client_nonce;
    std::string client_first_message = "n,," + client_first_bare;

    std::string sasl_init;
    sasl_init.append("SCRAM-SHA-256");
    sasl_init.push_back('\0');
    int32_t sasl_data_len = client_first_message.size();
    unsigned char len_buf[4];
    int32_to_buf(sasl_data_len, len_buf);
    sasl_init.append(reinterpret_cast<char *>(len_buf), 4);
    sasl_init.append(client_first_message);

    send_data.clear();
    send_data.push_back('p');
    int32_t len = 4 + sasl_init.size();
    unsigned char header[4];
    int32_to_buf(len, header);
    send_data.append(reinterpret_cast<char *>(header), 4);
    send_data.append(sasl_init);

    return true;
}

void pg_conn_base::build_startup_message(const orm_conn_t &conn_config)
{
    std::string startup;
    startup.append(4, '\0');
    int32_t proto_ver = PG_PROTOCOL_3_0;
    unsigned char ver_buf[4];
    int32_to_buf(proto_ver, ver_buf);
    startup.append(reinterpret_cast<char *>(ver_buf), 4);

    auto add_pair = [&](const std::string &key, const std::string &val)
    {
        startup.append(key);
        startup.push_back('\0');
        startup.append(val);
        startup.push_back('\0');
    };
    add_pair("user", conn_config.user);
    if (!conn_config.dbname.empty())
        add_pair("database", conn_config.dbname);
    add_pair("client_encoding", "UTF8");
    startup.push_back('\0');

    int32_t total_len = startup.size();
    unsigned char len_buf[4];
    int32_to_buf(total_len, len_buf);
    startup.replace(0, 4, reinterpret_cast<char *>(len_buf), 4);

    send_data.clear();
    send_data = startup;
}

void pg_conn_base::build_ssl_request()
{
    unsigned char ssl_req[8];
    int32_to_buf(8, ssl_req);
    int32_to_buf(PG_SSL_REQUEST_CODE, ssl_req + 4);
    send_data.clear();
    send_data.assign(reinterpret_cast<char *>(ssl_req), 8);
}

asio::awaitable<bool> pg_conn_base::async_connect(const orm_conn_t &conn_config)
{
    error_msg.clear();
    error_code = 0;
    isclose    = false;

    // 重连场景：彻底重置连接层状态
    conn_link->sock_type  = 0;
    conn_link->sslsocket.reset();
    conn_link->ssl_context.reset();
    conn_link->localsocket.reset();
    conn_link->socket     = std::make_unique<asio::ip::tcp::socket>(*conn_link->io_ctx);
    conn_link->ec.clear();
    server_enable_ssl     = false;
    in_transaction_.store(false);
    notices_.clear();

    // 保存配置用于断线自动重连（仅在首次或配置变化时存储）
    if (!has_conn_config_ || !(last_conn_config_ == conn_config)) {
        last_conn_config_ = conn_config;
        has_conn_config_ = true;
    }

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
            if (!conn_link->ec)
                break;
            ++iter;
        }

        if (conn_link->ec)
        {
            error_msg  = "resolve/connect error: " + conn_link->ec.message();
            error_code = 1;
            isclose    = true;
            co_return false;
        }
        conn_link->sock_type = 1;
    }

    server_enable_ssl = false;
    if (conn_config.isssl && !conn_config.issock)
    {
        build_ssl_request();
        try
        {
            co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), asio::use_awaitable);
        }
        catch (const std::exception &e)
        {
            error_msg  = "SSLRequest write error: " + std::string(e.what());
            error_code = 3;
            isclose    = true;
            co_return false;
        }

        unsigned char ssl_response;
        try
        {
            co_await asio::async_read(*conn_link->socket, asio::buffer(&ssl_response, 1), asio::use_awaitable);
        }
        catch (const std::exception &e)
        {
            error_msg  = "SSL response read error: " + std::string(e.what());
            error_code = 4;
            isclose    = true;
            co_return false;
        }

        if (ssl_response == 'S')
        {
            conn_link->ssl_context = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
            if (conn_config.sslverify)
            {
                conn_link->ssl_context->set_verify_mode(asio::ssl::verify_peer);
                conn_link->ssl_context->set_default_verify_paths();
            }
            asio::ssl::stream<asio::ip::tcp::socket> ssl_temp_socket(std::move(*conn_link->socket.release()), *conn_link->ssl_context);
            conn_link->sslsocket = std::make_unique<asio::ssl::stream<asio::ip::tcp::socket>>(std::move(ssl_temp_socket));

            const std::string &sni_host = conn_config.sslhost.empty() ? conn_config.host : conn_config.sslhost;
            if (!sni_host.empty())
            {
                SSL_set_tlsext_host_name(conn_link->sslsocket->native_handle(), sni_host.c_str());
            }
            if (conn_config.sslverify && !conn_config.sslhost.empty())
            {
                conn_link->sslsocket->set_verify_callback(asio::ssl::host_name_verification(conn_config.sslhost));
            }

            std::tie(conn_link->ec) = co_await conn_link->sslsocket->async_handshake(asio::ssl::stream_base::client, tuple_awaitable);
            if (conn_link->ec)
            {
                error_msg  = "SSL handshake error: " + conn_link->ec.message();
                error_code = 5;
                isclose    = true;
                co_return false;
            }
            conn_link->sock_type = 2;
            server_enable_ssl    = true;
        }
        else
        {
            // async：请求 SSL 但服务器回 'N' 不支持 —— 不允许静默降级
            if (conn_config.isssl)
            {
                error_msg  = "server does not support SSL (requested by client config)";
                error_code = 5;
                isclose    = true;
                co_return false;
            }
            server_enable_ssl = false;
        }
    }

    build_startup_message(conn_config);
    try
    {
        if (conn_link->sock_type == 2)
        {
            co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), asio::use_awaitable);
        }
        else if (conn_link->sock_type == 3)
        {
            co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), asio::use_awaitable);
        }
        else
        {
            co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), asio::use_awaitable);
        }
    }
    catch (const std::exception &e)
    {
        error_msg  = "StartupMessage write error: " + std::string(e.what());
        error_code = 6;
        isclose    = true;
        co_return false;
    }

    bool auth_done = false;
    std::string scram_salted_password;
    std::string scram_auth_message;
    while (!isclose)
    {
        unsigned char msg_type;
        unsigned char len_buf[4];
        int32_t msg_len;

        try
        {
            if (conn_link->sock_type == 2)
            {
                co_await asio::async_read(*conn_link->sslsocket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                co_await asio::async_read(*conn_link->sslsocket, asio::buffer(len_buf, 4), asio::use_awaitable);
            }
            else if (conn_link->sock_type == 3)
            {
                co_await asio::async_read(*conn_link->localsocket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                co_await asio::async_read(*conn_link->localsocket, asio::buffer(len_buf, 4), asio::use_awaitable);
            }
            else
            {
                co_await asio::async_read(*conn_link->socket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                co_await asio::async_read(*conn_link->socket, asio::buffer(len_buf, 4), asio::use_awaitable);
            }
        }
        catch (const std::exception &e)
        {
            error_msg  = "read message error: " + std::string(e.what());
            error_code = 7;
            isclose    = true;
            co_return false;
        }

        msg_len = buf_to_int32(len_buf);
        if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
        {
            error_msg  = "invalid message length";
            error_code = 8;
            isclose    = true;
            co_return false;
        }

        int32_t data_len = msg_len - 4;
        std::string payload;
        if (data_len > 0)
        {
            std::vector<char> tmp(data_len);
            try
            {
                if (conn_link->sock_type == 2)
                {
                    co_await asio::async_read(*conn_link->sslsocket, asio::buffer(tmp), asio::use_awaitable);
                }
                else if (conn_link->sock_type == 3)
                {
                    co_await asio::async_read(*conn_link->localsocket, asio::buffer(tmp), asio::use_awaitable);
                }
                else
                {
                    co_await asio::async_read(*conn_link->socket, asio::buffer(tmp), asio::use_awaitable);
                }
            }
            catch (const std::exception &e)
            {
                error_msg  = "read payload error: " + std::string(e.what());
                error_code = 7;
                isclose    = true;
                co_return false;
            }
            payload.assign(tmp.data(), data_len);
        }

        switch (msg_type)
        {
        case 'R':
        {
            if (payload.size() < 4)
            {
                error_msg  = "auth response too short";
                error_code = 7;
                isclose    = true;
                co_return false;
            }
            int32_t auth_type = buf_to_int32(reinterpret_cast<const unsigned char *>(payload.data()));
            if (auth_type == 0)
            {
                auth_done = true;
            }
            else if (auth_type == 10)
            {
                if (payload.size() < 4 || payload.substr(4).find("SCRAM-SHA-256") == std::string::npos)
                {
                    error_msg  = "SCRAM-SHA-256 not offered by server";
                    error_code = 15;
                    isclose    = true;
                    co_return false;
                }

                std::string client_nonce         = generate_nonce(18);
                std::string client_first_bare    = "n=" + conn_config.user + ",r=" + client_nonce;
                std::string client_first_message = "n,," + client_first_bare;

                std::string sasl_init;
                sasl_init.append("SCRAM-SHA-256");
                sasl_init.push_back('\0');
                int32_t sasl_data_len = client_first_message.size();
                unsigned char len_buf2[4];
                int32_to_buf(sasl_data_len, len_buf2);
                sasl_init.append(reinterpret_cast<char *>(len_buf2), 4);
                sasl_init.append(client_first_message);

                send_data.clear();
                send_data.push_back('p');
                int32_t len = 4 + sasl_init.size();
                unsigned char header[4];
                int32_to_buf(len, header);
                send_data.append(reinterpret_cast<char *>(header), 4);
                send_data.append(sasl_init);

                try
                {
                    if (conn_link->sock_type == 2)
                    {
                        co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), asio::use_awaitable);
                    }
                    else if (conn_link->sock_type == 3)
                    {
                        co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), asio::use_awaitable);
                    }
                    else
                    {
                        co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), asio::use_awaitable);
                    }
                }
                catch (const std::exception &e)
                {
                    error_msg  = "SASL init write error: " + std::string(e.what());
                    error_code = 15;
                    isclose    = true;
                    co_return false;
                }

                try
                {
                    if (conn_link->sock_type == 2)
                    {
                        co_await asio::async_read(*conn_link->sslsocket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                        co_await asio::async_read(*conn_link->sslsocket, asio::buffer(len_buf, 4), asio::use_awaitable);
                    }
                    else if (conn_link->sock_type == 3)
                    {
                        co_await asio::async_read(*conn_link->localsocket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                        co_await asio::async_read(*conn_link->localsocket, asio::buffer(len_buf, 4), asio::use_awaitable);
                    }
                    else
                    {
                        co_await asio::async_read(*conn_link->socket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                        co_await asio::async_read(*conn_link->socket, asio::buffer(len_buf, 4), asio::use_awaitable);
                    }
                }
                catch (const std::exception &e)
                {
                    error_msg  = "read SASL continue error: " + std::string(e.what());
                    error_code = 16;
                    isclose    = true;
                    co_return false;
                }

                msg_len = buf_to_int32(len_buf);
                if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
                {
                    error_msg  = "invalid SASL message length";
                    error_code = 16;
                    isclose    = true;
                    co_return false;
                }

                data_len = msg_len - 4;
                std::vector<char> tmp2(data_len);
                try
                {
                    if (conn_link->sock_type == 2)
                    {
                        co_await asio::async_read(*conn_link->sslsocket, asio::buffer(tmp2), asio::use_awaitable);
                    }
                    else if (conn_link->sock_type == 3)
                    {
                        co_await asio::async_read(*conn_link->localsocket, asio::buffer(tmp2), asio::use_awaitable);
                    }
                    else
                    {
                        co_await asio::async_read(*conn_link->socket, asio::buffer(tmp2), asio::use_awaitable);
                    }
                }
                catch (const std::exception &e)
                {
                    error_msg  = "read SASL payload error: " + std::string(e.what());
                    error_code = 16;
                    isclose    = true;
                    co_return false;
                }
                std::string server_first_msg(tmp2.data(), data_len);

                std::string server_nonce, salt_b64;
                int iteration = 0;
                size_t pos    = 0;
                while (pos < server_first_msg.size())
                {
                    char key = server_first_msg[pos++];
                    if (pos >= server_first_msg.size())
                        break;
                    std::string value;
                    while (pos < server_first_msg.size() && server_first_msg[pos] != ',')
                        value.push_back(server_first_msg[pos++]);
                    pos++;
                    if (key == 'r')
                        server_nonce = value;
                    else if (key == 's')
                        salt_b64 = value;
                    else if (key == 'i')
                    {
                        try
                        {
                            iteration = std::stoi(value);
                        }
                        catch (...)
                        {
                            iteration = -1;
                        }
                    }
                }

                if (server_nonce.empty() || salt_b64.empty() || iteration <= 0)
                {
                    error_msg  = "invalid server-first-message";
                    error_code = 17;
                    isclose    = true;
                    co_return false;
                }

                std::string salt      = base64_decode(salt_b64);
                scram_salted_password = pbkdf2_hmac_sha256(conn_config.password, salt, iteration, 32);

                std::string channel_binding            = "c=biws";
                std::string nonce_part                 = "r=" + server_nonce;
                std::string client_final_without_proof = channel_binding + "," + nonce_part;
                scram_auth_message                     = client_first_bare + "," + server_first_msg + "," + client_final_without_proof;

                std::string client_key       = hmac_sha256(scram_salted_password, "Client Key");
                std::string stored_key       = sha256(client_key);
                std::string client_signature = hmac_sha256(stored_key, scram_auth_message);
                std::string client_proof     = xor_strings(client_key, client_signature);
                std::string client_proof_b64 = base64_encode(
                    reinterpret_cast<const unsigned char *>(client_proof.data()),
                    client_proof.size());

                std::string client_final = client_final_without_proof + ",p=" + client_proof_b64;

                send_data.clear();
                send_data.push_back('p');
                len = 4 + client_final.size();
                int32_to_buf(len, header);
                send_data.append(reinterpret_cast<char *>(header), 4);
                send_data.append(client_final);

                try
                {
                    if (conn_link->sock_type == 2)
                    {
                        co_await asio::async_write(*conn_link->sslsocket, asio::buffer(send_data), asio::use_awaitable);
                    }
                    else if (conn_link->sock_type == 3)
                    {
                        co_await asio::async_write(*conn_link->localsocket, asio::buffer(send_data), asio::use_awaitable);
                    }
                    else
                    {
                        co_await asio::async_write(*conn_link->socket, asio::buffer(send_data), asio::use_awaitable);
                    }
                }
                catch (const std::exception &e)
                {
                    error_msg  = "SASL final write error: " + std::string(e.what());
                    error_code = 15;
                    isclose    = true;
                    co_return false;
                }

                try
                {
                    if (conn_link->sock_type == 2)
                    {
                        co_await asio::async_read(*conn_link->sslsocket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                        co_await asio::async_read(*conn_link->sslsocket, asio::buffer(len_buf, 4), asio::use_awaitable);
                    }
                    else if (conn_link->sock_type == 3)
                    {
                        co_await asio::async_read(*conn_link->localsocket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                        co_await asio::async_read(*conn_link->localsocket, asio::buffer(len_buf, 4), asio::use_awaitable);
                    }
                    else
                    {
                        co_await asio::async_read(*conn_link->socket, asio::buffer(&msg_type, 1), asio::use_awaitable);
                        co_await asio::async_read(*conn_link->socket, asio::buffer(len_buf, 4), asio::use_awaitable);
                    }
                }
                catch (const std::exception &e)
                {
                    error_msg  = "read final auth error: " + std::string(e.what());
                    error_code = 21;
                    isclose    = true;
                    co_return false;
                }

                msg_len = buf_to_int32(len_buf);
                // 必须校验 msg_type == 'R'（SASL final 响应），否则可能读到 Error 等其它包
                if (msg_type != 'R')
                {
                    error_msg  = "unexpected message type after SASL final request: got " + std::to_string(msg_type);
                    error_code = 21;
                    isclose    = true;
                    co_return false;
                }
                if (msg_len >= 4 && msg_len <= 8 * 1024 * 1024)
                {
                    // 读取实际的payload数据来获取auth_type
                    std::vector<char> final_payload(msg_len - 4);
                    try
                    {
                        if (conn_link->sock_type == 2)
                        {
                            co_await asio::async_read(*conn_link->sslsocket, asio::buffer(final_payload), asio::use_awaitable);
                        }
                        else if (conn_link->sock_type == 3)
                        {
                            co_await asio::async_read(*conn_link->localsocket, asio::buffer(final_payload), asio::use_awaitable);
                        }
                        else
                        {
                            co_await asio::async_read(*conn_link->socket, asio::buffer(final_payload), asio::use_awaitable);
                        }
                    }
                    catch (const std::exception &e)
                    {
                        error_msg  = "read final auth payload error: " + std::string(e.what());
                        error_code = 21;
                        isclose    = true;
                        co_return false;
                    }

                    if (final_payload.size() < 4)
                    {
                        error_msg  = "final auth payload too short";
                        error_code = 21;
                        isclose    = true;
                        co_return false;
                    }
                    int32_t final_auth_type = buf_to_int32(reinterpret_cast<const unsigned char *>(final_payload.data()));
                    if (final_auth_type == 0)
                    {
                        auth_done = true;
                    }
                    else if (final_auth_type == 12)
                    {
                        // SASL Final - 验证服务器签名，防止中间人冒充服务器
                        if (final_payload.size() < 5)
                        {
                            error_msg  = "SASL final message too short";
                            error_code = 21;
                            isclose    = true;
                            co_return false;
                        }
                        std::string server_final(final_payload.data() + 4, final_payload.size() - 4);
                        if (server_final.size() >= 2 && server_final.compare(0, 2, "e=") == 0)
                        {
                            error_msg  = "SASL server reported error: " + server_final.substr(2);
                            error_code = 21;
                            isclose    = true;
                            co_return false;
                        }
                        std::string server_signature_b64;
                        size_t pos2 = 0;
                        while (pos2 < server_final.size())
                        {
                            char key = server_final[pos2++];
                            if (key == 'v')
                            {
                                while (pos2 < server_final.size() && server_final[pos2] != ',')
                                    server_signature_b64.push_back(server_final[pos2++]);
                                break;
                            }
                            while (pos2 < server_final.size() && server_final[pos2] != ',')
                                pos2++;
                            if (pos2 < server_final.size())
                                pos2++;
                        }
                        if (server_signature_b64.empty())
                        {
                            error_msg  = "missing server signature";
                            error_code = 21;
                            isclose    = true;
                            co_return false;
                        }
                        std::string server_key              = hmac_sha256(scram_salted_password, "Server Key");
                        std::string expected_server_sig     = hmac_sha256(server_key, scram_auth_message);
                        std::string expected_server_sig_b64 = base64_encode(
                            reinterpret_cast<const unsigned char *>(expected_server_sig.data()),
                            expected_server_sig.size());
                        if (server_signature_b64 != expected_server_sig_b64)
                        {
                            error_msg  = "server signature mismatch (possible MITM)";
                            error_code = 20;
                            isclose    = true;
                            co_return false;
                        }
                        auth_done = true;
                    }
                    else
                    {
                        error_msg  = "unexpected final auth type: " + std::to_string(final_auth_type);
                        error_code = 21;
                        isclose    = true;
                        co_return false;
                    }
                }
            }
            else
            {
                error_msg  = "unsupported authentication method: " + std::to_string(auth_type);
                error_code = 14;
                isclose    = true;
                co_return false;
            }
            break;
        }
        case 'K':
        {
            if (payload.size() >= 8)
            {
                const unsigned char *p = reinterpret_cast<const unsigned char *>(payload.data());
                // 经 uint32_t 移位，避免字节 >= 0x80 时有符号溢出 UB
                uint32_t pid = (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
                               (static_cast<uint32_t>(p[2]) << 8) | p[3];
                uint32_t secret = (static_cast<uint32_t>(p[4]) << 24) | (static_cast<uint32_t>(p[5]) << 16) |
                                  (static_cast<uint32_t>(p[6]) << 8) | p[7];
                backend_pid_   = static_cast<int>(pid);
                cancel_secret_ = static_cast<int>(secret);
            }
            break;
        }
        case 'S':
        {
            const unsigned char *p = reinterpret_cast<const unsigned char *>(payload.data());
            const unsigned char *end = p + payload.size();
            const char *name_start = reinterpret_cast<const char *>(p);
            const char *name_end = static_cast<const char *>(memchr(name_start, '\0', end - p));
            if (name_end && name_end < reinterpret_cast<const char *>(end))
            {
                std::string param_name(name_start, name_end);
                const char *val_start = name_end + 1;
                const char *val_end = static_cast<const char *>(memchr(val_start, '\0', end - reinterpret_cast<const unsigned char *>(val_start)));
                std::string param_value(val_start, val_end ? val_end : reinterpret_cast<const char *>(end));
                if (param_name == "client_encoding")
                {
                    server_client_encoding_ = param_value;
                }
            }
            break;
        }
        case 'Z':
            if (!auth_done)
            {
                error_msg  = "server sent ReadyForQuery before authentication";
                error_code = 10;
                isclose    = true;
                co_return false;
            }
            // 认证成功建立连接时必为空闲态；后续查询循环按 'Z' 状态字节持续更新
            in_transaction_.store(false);
            co_return true;
        case 'E':
        {
            parse_error(payload);
            co_return false;
        }
        case 'N':
        {
            const unsigned char *np = reinterpret_cast<const unsigned char *>(payload.data());
            const unsigned char *ne = np + payload.size();
            if (np < ne && *np == 'C')
            {
                const char *p = reinterpret_cast<const char *>(np + 1);
                while (p < reinterpret_cast<const char *>(ne))
                {
                    char field = *p++;
                    const char *val_start = p;
                    const char *val_end = static_cast<const char *>(memchr(val_start, '\0', reinterpret_cast<const char *>(ne) - val_start));
                    if (field == 'S' && val_end)
                    {
                        notices_.emplace_back(val_start, val_end);
                        break;
                    }
                    if (!val_end) break;
                    p = val_end + 1;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    co_return false;
}

unsigned int pg_conn_base::read_loop()
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
        return n;
    }
    catch (const std::exception &e)
    {
        error_code = 1;
        error_msg.append(e.what());
    }
    return 0;
}

asio::awaitable<unsigned int> pg_conn_base::async_read_loop()
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
    co_return n;
}

unsigned int pg_conn_base::write_sql(const std::string &sql)
{
    unsigned int n = 0;
    send_data.clear();
    send_data.push_back('Q');
    int32_t len = 4 + sql.size() + 1;
    unsigned char header[4];
    int32_to_buf(len, header);
    send_data.append(reinterpret_cast<char *>(header), 4);
    send_data.append(sql);
    send_data.push_back('\0');

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
        error_msg  = e.what();
        return 0;
    }
    query_num++;
    return n;
}

unsigned int pg_conn_base::write()
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
        error_msg  = e.what();
        return 0;
    }
    query_num++;
    return n;
}

asio::awaitable<unsigned int> pg_conn_base::async_write_sql(const std::string &sql)
{
    unsigned int n = 0;
    send_data.clear();
    send_data.push_back('Q');
    int32_t len = 4 + sql.size() + 1;
    unsigned char header[4];
    int32_to_buf(len, header);
    send_data.append(reinterpret_cast<char *>(header), 4);
    send_data.append(sql);
    send_data.push_back('\0');

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

asio::awaitable<unsigned int> pg_conn_base::async_write()
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

bool pg_conn_base::ping()
{
    error_code = 0;
    error_msg.clear();
    try
    {
        if (isclose)
        {
            return false;
        }

        // 'Q' 消息：长度字段含自身 4 字节，须为 4 + 8 + 1 = 13。
        // 曾误按 10 字节查询串算成 14，服务端会多等 1 字节导致 ping 双向死锁。
        std::string ping_msg;
        ping_msg.push_back('Q');
        const char ping_sql[] = "SELECT 1";
        int32_t len = 4 + static_cast<int32_t>(sizeof(ping_sql) - 1) + 1;
        unsigned char header[4];
        int32_to_buf(len, header);
        ping_msg.append(reinterpret_cast<char *>(header), 4);
        ping_msg.append(ping_sql, sizeof(ping_sql) - 1);
        ping_msg.push_back('\0');

        if (conn_link->sock_type == 1)
        {
            asio::write(*conn_link->socket, asio::buffer(ping_msg), conn_link->ec);
            if (conn_link->ec)
                return false;

            unsigned char msg_type;
            unsigned char len_buf[4];
            int32_t msg_len;

            asio::read(*conn_link->socket, asio::buffer(&msg_type, 1), conn_link->ec);
            if (conn_link->ec)
                return false;
            asio::read(*conn_link->socket, asio::buffer(len_buf, 4), conn_link->ec);
            if (conn_link->ec)
                return false;

            msg_len          = buf_to_int32(len_buf);
            int32_t data_len = msg_len - 4;
            if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
                return false;
            if (data_len > 0)
            {
                std::vector<char> tmp(data_len);
                asio::read(*conn_link->socket, asio::buffer(tmp), conn_link->ec);
                if (conn_link->ec)
                    return false;
            }

            // 添加最大重试次数防止无限循环
            int max_retries = 100;
            while (msg_type != 'Z' && max_retries-- > 0)
            {
                asio::read(*conn_link->socket, asio::buffer(&msg_type, 1), conn_link->ec);
                if (conn_link->ec)
                    return false;
                asio::read(*conn_link->socket, asio::buffer(len_buf, 4), conn_link->ec);
                if (conn_link->ec)
                    return false;
                msg_len  = buf_to_int32(len_buf);
                data_len = msg_len - 4;
                if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
                    return false;
                if (data_len > 0)
                {
                    std::vector<char> tmp(data_len);
                    asio::read(*conn_link->socket, asio::buffer(tmp), conn_link->ec);
                    if (conn_link->ec)
                        return false;
                }
            }
            if (max_retries <= 0)
            {
                error_msg  = "ping: too many messages without ReadyForQuery";
                error_code = 3;
                return false;
            }
        }
        else if (conn_link->sock_type == 2)
        {
            asio::write(*conn_link->sslsocket, asio::buffer(ping_msg), conn_link->ec);
            if (conn_link->ec)
                return false;

            unsigned char msg_type;
            unsigned char len_buf[4];
            int32_t msg_len;

            asio::read(*conn_link->sslsocket, asio::buffer(&msg_type, 1), conn_link->ec);
            if (conn_link->ec)
                return false;
            asio::read(*conn_link->sslsocket, asio::buffer(len_buf, 4), conn_link->ec);
            if (conn_link->ec)
                return false;

            msg_len          = buf_to_int32(len_buf);
            int32_t data_len = msg_len - 4;
            if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
                return false;
            if (data_len > 0)
            {
                std::vector<char> tmp(data_len);
                asio::read(*conn_link->sslsocket, asio::buffer(tmp), conn_link->ec);
                if (conn_link->ec)
                    return false;
            }

            // 添加最大重试次数防止无限循环
            int max_retries = 100;
            while (msg_type != 'Z' && max_retries-- > 0)
            {
                asio::read(*conn_link->sslsocket, asio::buffer(&msg_type, 1), conn_link->ec);
                if (conn_link->ec)
                    return false;
                asio::read(*conn_link->sslsocket, asio::buffer(len_buf, 4), conn_link->ec);
                if (conn_link->ec)
                    return false;
                msg_len  = buf_to_int32(len_buf);
                data_len = msg_len - 4;
                if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
                    return false;
                if (data_len > 0)
                {
                    std::vector<char> tmp(data_len);
                    asio::read(*conn_link->sslsocket, asio::buffer(tmp), conn_link->ec);
                    if (conn_link->ec)
                        return false;
                }
            }
            if (max_retries <= 0)
            {
                error_msg  = "ping: too many messages without ReadyForQuery";
                error_code = 3;
                return false;
            }
        }
        else if (conn_link->sock_type == 3)
        {
            asio::write(*conn_link->localsocket, asio::buffer(ping_msg), conn_link->ec);
            if (conn_link->ec)
                return false;

            unsigned char msg_type;
            unsigned char len_buf[4];
            int32_t msg_len;

            asio::read(*conn_link->localsocket, asio::buffer(&msg_type, 1), conn_link->ec);
            if (conn_link->ec)
                return false;
            asio::read(*conn_link->localsocket, asio::buffer(len_buf, 4), conn_link->ec);
            if (conn_link->ec)
                return false;

            msg_len          = buf_to_int32(len_buf);
            int32_t data_len = msg_len - 4;
            if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
                return false;
            if (data_len > 0)
            {
                std::vector<char> tmp(data_len);
                asio::read(*conn_link->localsocket, asio::buffer(tmp), conn_link->ec);
                if (conn_link->ec)
                    return false;
            }

            int max_retries = 100;
            while (msg_type != 'Z' && max_retries-- > 0)
            {
                asio::read(*conn_link->localsocket, asio::buffer(&msg_type, 1), conn_link->ec);
                if (conn_link->ec)
                    return false;
                asio::read(*conn_link->localsocket, asio::buffer(len_buf, 4), conn_link->ec);
                if (conn_link->ec)
                    return false;
                msg_len  = buf_to_int32(len_buf);
                data_len = msg_len - 4;
                if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
                    return false;
                if (data_len > 0)
                {
                    std::vector<char> tmp(data_len);
                    asio::read(*conn_link->localsocket, asio::buffer(tmp), conn_link->ec);
                    if (conn_link->ec)
                        return false;
                }
            }
            if (max_retries <= 0)
            {
                error_msg  = "ping: too many messages without ReadyForQuery";
                error_code = 3;
                return false;
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

bool pg_conn_base::close()
{
    if (isclose)
    {
        return false;
    }
    error_code = 0;
    error_msg.clear();
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
        return true;
    }
    catch (const std::exception &e)
    {
        error_code = 2;
        error_msg.append(e.what());
        return false;
    }
}

bool pg_conn_base::hard_close()
{
    try
    {
        isclose = true;
        if (conn_link->sock_type == 1)
        {
            if (conn_link->socket && conn_link->socket->is_open())
            {
                conn_link->socket->close(conn_link->ec);
            }
        }
        else if (conn_link->sock_type == 2)
        {
            if (conn_link->sslsocket && conn_link->sslsocket->lowest_layer().is_open())
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

asio::awaitable<bool> pg_conn_base::async_close()
{
    if (isclose)
    {
        co_return false;
    }
    error_code = 0;
    error_msg.clear();
    try
    {
        isclose = true;
        if (conn_link->sock_type == 1)
        {
            if (conn_link->socket->is_open())
            {
                conn_link->socket->close();
            }
        }
        else if (conn_link->sock_type == 2)
        {
            if (conn_link->sslsocket->lowest_layer().is_open())
            {
                conn_link->sslsocket->lowest_layer().cancel(conn_link->ec);
                conn_link->sslsocket->lowest_layer().close();
            }
        }
        else if (conn_link->sock_type == 3)
        {
            if (conn_link->localsocket && conn_link->localsocket->is_open())
            {
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

void pg_conn_base::reset_data()
{
    std::memset(_cache_data, 0x00, CACHE_DATA_LENGTH);
    send_data.clear();
}

bool pg_conn_base::is_closed()
{
    if (isclose)
    {
        return true;
    }
    if (conn_link)
    {
        return conn_link->is_closed();
    }
    return true;
}

void pg_conn_base::begin_time()
{
    time_begin = std::chrono::steady_clock::now();
}

void pg_conn_base::finish_time()
{
    time_finish = std::chrono::steady_clock::now();
}

long long pg_conn_base::count_time()
{
    long long time_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(time_finish - time_begin).count();
    return time_duration;
}

// ======================== PG Query Result Parsing ========================

std::vector<field_info_t> pg_conn_base::parse_row_description(const unsigned char *data, unsigned int len)
{
    std::vector<field_info_t> fields;
    if (len < 2)
        return fields;

    unsigned int pos    = 0;
    int16_t field_count = buf_to_int16(data + pos);
    pos += 2;

    for (int16_t i = 0; i < field_count && pos < len; i++)
    {
        field_info_t fi;
        // field name (null-terminated string)
        size_t name_end = pos;
        while (name_end < len && data[name_end] != '\0')
            name_end++;
        fi.name     = std::string(reinterpret_cast<const char *>(data + pos), name_end - pos);
        fi.org_name = fi.name;
        pos         = name_end + 1;// skip '\0'

        if (pos + 18 > len)
            break;

        // table_oid (4 bytes)
        // uint32_t table_oid = buf_to_int32(data + pos);
        pos += 4;

        // column attribute number (2 bytes)
        // int16_t col_num = buf_to_int16(data + pos);
        pos += 2;

        // type OID (4 bytes)
        uint32_t type_oid = static_cast<uint32_t>(buf_to_int32(data + pos));
        fi.field_type     = static_cast<unsigned char>(type_oid & 0xFF);
        pos += 4;

        // type length (2 bytes)
        fi.column_length = static_cast<unsigned int>(buf_to_int16(data + pos));
        pos += 2;

        // type modifier (4 bytes)
        pos += 4;

        // format code (2 bytes): 0=text, 1=binary
        // int16_t format = buf_to_int16(data + pos);
        pos += 2;

        fields.emplace_back(std::move(fi));
    }
    return fields;
}

pg_row_data_t pg_conn_base::parse_data_row(const unsigned char *data, unsigned int len)
{
    pg_row_data_t row;
    if (len < 2)
        return row;

    unsigned int pos  = 0;
    int16_t col_count = buf_to_int16(data + pos);
    pos += 2;

    if (col_count < 0 || col_count > 4096)
    {
        return row;
    }

    row.values.resize(col_count);
    row.is_null.resize(col_count, false);

    for (int16_t i = 0; i < col_count && pos < len; i++)
    {
        if (pos + 4 > len)
            break;
        int32_t col_len = buf_to_int32(data + pos);
        pos += 4;

        if (col_len == -1)
        {
            // NULL value
            row.is_null[i] = true;
            row.values[i].clear();
        }
        else if (col_len < 0)
        {
            // 其它负值 = 畸形/恶意包，拒绝
            break;
        }
        else
        {
            if (pos + static_cast<unsigned int>(col_len) > len)
                break;
            row.values[i] = std::string(reinterpret_cast<const char *>(data + pos), col_len);
            pos += col_len;
        }
    }
    return row;
}

unsigned int pg_conn_base::parse_command_complete(const unsigned char *data, unsigned int len)
{
    // CommandComplete payload is a null-terminated string like:
    // "SELECT N\0", "INSERT 0 N\0", "UPDATE N\0", "DELETE N\0"
    if (len == 0)
        return 0;
    std::string tag(reinterpret_cast<const char *>(data), len);
    // Remove trailing null if present
    if (!tag.empty() && tag.back() == '\0')
        tag.pop_back();

    // Find the last space-separated number
    size_t last_space = tag.rfind(' ');
    if (last_space != std::string::npos)
    {
        std::string num_str = tag.substr(last_space + 1);
        try
        {
            return static_cast<unsigned int>(std::stoul(num_str));
        }
        catch (const std::exception &)
        {
            return 0;
        }
    }
    return 0;
}

// Helper: read a complete PG message from _cache_data buffer
// Returns: msg_type, sets payload_start/payload_len
// Advances offset past the message. Returns 0 on incomplete/error.
static unsigned char peek_pg_message(const unsigned char *buf, unsigned int buf_len, unsigned int &offset, const unsigned char *&payload_start, unsigned int &payload_len)
{
    if (offset + 5 > buf_len)
        return 0;// need at least type(1) + len(4)
    unsigned char msg_type = buf[offset];
    int32_t msg_len        = buf_to_int32(buf + offset + 1);
    if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
        return 0;
    unsigned int total = 1 + static_cast<unsigned int>(msg_len);
    if (offset + total > buf_len)
        return 0;// incomplete
    payload_start = buf + offset + 5;
    payload_len   = static_cast<unsigned int>(msg_len) - 4;
    offset += total;
    return msg_type;
}

unsigned int pg_conn_base::execute_and_fetch(
    const std::string &sql,
    std::vector<field_info_t> &fields_out,
    std::vector<pg_row_data_t> &rows_out,
    unsigned int &affected_rows_out)
{
    fields_out.clear();
    rows_out.clear();
    affected_rows_out = 0;

    unsigned int n = write_sql(sql);
    if (n == 0)
        return 1;

    // We need to accumulate data and parse messages
    // Simple approach: read into buffer, parse messages as they arrive
    std::string accum_buf;
    bool got_ready        = false;
    unsigned int consumed = 0;
    int max_iterations    = 1000;

    while (!got_ready && max_iterations-- > 0)
    {
        n = read_loop();
        if (n == 0)
        {
            error_msg = "read error during query";
            return 1;
        }
        accum_buf.append(reinterpret_cast<char *>(_cache_data), n);

        // Try to parse all complete messages in accum_buf
        unsigned int parse_offset = consumed;
        while (parse_offset < accum_buf.size())
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = peek_pg_message(
                reinterpret_cast<const unsigned char *>(accum_buf.data()),
                static_cast<unsigned int>(accum_buf.size()),
                parse_offset,
                payload_start,
                payload_len);

            if (msg_type == 0)
                break;// incomplete message

            switch (msg_type)
            {
            case 'T':// RowDescription
                fields_out = parse_row_description(payload_start, payload_len);
                break;
            case 'D':// DataRow
                rows_out.emplace_back(parse_data_row(payload_start, payload_len));
                break;
            case 'C':// CommandComplete
                affected_rows_out = parse_command_complete(payload_start, payload_len);
                break;
            case 'Z':// ReadyForQuery
                // 'Z' 状态字节: 'I' 空闲 / 'T' 事务中 / 'E' 失败事务中 —— 事务状态的权威来源
                if (payload_len >= 1)
                    in_transaction_.store(payload_start[0] == 'T' || payload_start[0] == 'E');
                got_ready = true;
                break;
            case 'E':// ErrorResponse
            {
                // Parse error message from payload
                // Format: field_type(1) + string\0 repeated, terminated by \0
                std::string err_detail;
                unsigned int epos = 0;
                while (epos < payload_len)
                {
                    unsigned char field_type = payload_start[epos];
                    if (field_type == '\0')
                        break;
                    epos++;
                    size_t str_end = epos;
                    while (str_end < payload_len && payload_start[str_end] != '\0')
                        str_end++;
                    std::string val(reinterpret_cast<const char *>(payload_start + epos), str_end - epos);
                    if (field_type == 'M')// Message
                    {
                        err_detail = val;
                    }
                    if (str_end >= payload_len)
                    {
                        // 消息截断：无 null 终止符
                        break;
                    }
                    epos = str_end + 1;
                }
                error_msg  = err_detail.empty() ? "PG query error" : err_detail;
                error_code = 10;
                // Continue reading until ReadyForQuery
                break;
            }
            case 'N':// NoticeResponse - 收集警告/提示消息
            {
                const unsigned char *np = payload_start;
                const unsigned char *ne = np + payload_len;
                if (np < ne && *np == 'C')
                {
                    // 跳过 severity, 查找 'S' (message) 字段
                    const char *p = reinterpret_cast<const char *>(np + 1);
                    while (p < reinterpret_cast<const char *>(ne))
                    {
                        char field = *p++;
                        const char *val_start = p;
                        const char *val_end = static_cast<const char *>(memchr(val_start, '\0', reinterpret_cast<const char *>(ne) - val_start));
                        if (field == 'S' && val_end)
                        {
                            notices_.emplace_back(val_start, val_end);
                            break;
                        }
                        if (!val_end) break;
                        p = val_end + 1;
                    }
                }
                break;
            }
            case 'I':// EmptyQueryResponse - 正常忽略
                break;
            default:
                break;
            }
        }
        consumed = parse_offset;
        // 定期压缩已消费数据，避免大结果集 O(n^2) 拷贝
        if (consumed > 256 * 1024)
        {
            accum_buf.erase(0, consumed);
            consumed = 0;
        }
    }
    if (max_iterations <= 0)
    {
        error_msg  = "execute_and_fetch: loop exceeded max iterations";
        error_code = 8;
        isclose    = true;  // 挂死时必须断开连接，否则后续调用会在脏状态下继续
        return 1;
    }
    if (error_code == 10)
    {
        return 1;
    }
    return 0;
}

asio::awaitable<unsigned int> pg_conn_base::async_execute_and_fetch(
    const std::string &sql,
    std::vector<field_info_t> &fields_out,
    std::vector<pg_row_data_t> &rows_out,
    unsigned int &affected_rows_out)
{
    fields_out.clear();
    rows_out.clear();
    affected_rows_out = 0;

    unsigned int n = co_await async_write_sql(sql);
    if (n == 0)
        co_return 1;

    std::string accum_buf;
    bool got_ready        = false;
    unsigned int consumed = 0;
    int max_iterations    = 1000;

    while (!got_ready && max_iterations-- > 0)
    {
        n = co_await async_read_loop();
        if (n == 0)
        {
            error_msg = "async read error during query";
            co_return 1;
        }
        accum_buf.append(reinterpret_cast<char *>(_cache_data), n);

        unsigned int parse_offset = consumed;
        while (parse_offset < accum_buf.size())
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = peek_pg_message(
                reinterpret_cast<const unsigned char *>(accum_buf.data()),
                static_cast<unsigned int>(accum_buf.size()),
                parse_offset,
                payload_start,
                payload_len);

            if (msg_type == 0)
                break;

            switch (msg_type)
            {
            case 'T':
                fields_out = parse_row_description(payload_start, payload_len);
                break;
            case 'D':
                rows_out.emplace_back(parse_data_row(payload_start, payload_len));
                break;
            case 'C':
                affected_rows_out = parse_command_complete(payload_start, payload_len);
                break;
            case 'Z':
                if (payload_len >= 1)
                    in_transaction_.store(payload_start[0] == 'T' || payload_start[0] == 'E');
                got_ready = true;
                break;
            case 'E':
            {
                std::string err_detail;
                unsigned int epos = 0;
                while (epos < payload_len)
                {
                    unsigned char field_type = payload_start[epos];
                    if (field_type == '\0')
                        break;
                    epos++;
                    size_t str_end = epos;
                    while (str_end < payload_len && payload_start[str_end] != '\0')
                        str_end++;
                    std::string val(reinterpret_cast<const char *>(payload_start + epos), str_end - epos);
                    if (field_type == 'M')
                    {
                        err_detail = val;
                    }
                    if (str_end >= payload_len)
                    {
                        // 消息截断：无 null 终止符
                        break;
                    }
                    epos = str_end + 1;
                }
                error_msg  = err_detail.empty() ? "PG async query error" : err_detail;
                error_code = 10;
                break;
            }
            case 'N':
            {
                const unsigned char *np = reinterpret_cast<const unsigned char *>(payload_start);
                const unsigned char *ne = np + payload_len;
                if (np < ne && *np == 'C')
                {
                    const char *p = reinterpret_cast<const char *>(np + 1);
                    while (p < reinterpret_cast<const char *>(ne))
                    {
                        char field = *p++;
                        const char *val_start = p;
                        const char *val_end = static_cast<const char *>(memchr(val_start, '\0', reinterpret_cast<const char *>(ne) - val_start));
                        if (field == 'S' && val_end)
                        {
                            notices_.emplace_back(val_start, val_end);
                            break;
                        }
                        if (!val_end) break;
                        p = val_end + 1;
                    }
                }
                break;
            }
            case 'I':
                break;
            default:
                break;
            }
        }
        consumed = parse_offset;
        // 定期压缩已消费数据，避免大结果集 O(n^2) 拷贝
        if (consumed > 256 * 1024)
        {
            accum_buf.erase(0, consumed);
            consumed = 0;
        }
    }
    if (max_iterations <= 0)
    {
        error_msg  = "async_execute_and_fetch: loop exceeded max iterations";
        error_code = 8;
        isclose    = true;  // async：挂死时必须断开连接
        co_return 1;
    }
    if (error_code == 10)
    {
        co_return 1;
    }
    co_return 0;
}

// ============================================================================
// exec_dml / fetch_directly 实现（与 MySQL 侧同形态：容量池累积 + 消息循环）
// ============================================================================

void pg_conn_base::parse_error_from_payload(const unsigned char *data, unsigned int len)
{
    // E/N 消息载荷格式: field_type(1) + string\0 重复，以 \0 结束；只取 'M' (Message)
    std::string err_detail;
    unsigned int epos = 0;
    while (epos < len)
    {
        unsigned char field_type = data[epos];
        if (field_type == '\0')
            break;
        epos++;
        size_t str_end = epos;
        while (str_end < len && data[str_end] != '\0')
            str_end++;
        if (field_type == 'M')
        {
            err_detail.assign(reinterpret_cast<const char *>(data + epos), str_end - epos);
        }
        if (str_end >= len)
        {
            // 消息截断：无 null 终止符，直接退出
            break;
        }
        epos = static_cast<unsigned int>(str_end) + 1;
    }
    error_msg  = err_detail.empty() ? "PG error response" : err_detail;
    error_code = 10;
}

unsigned int pg_conn_base::exec_dml_impl(const std::string &sql)
{
    unsigned int retry = 0;
RETRY_LABEL:
    if (isclose)
    {
        return (unsigned int)-1;
    }

    error_msg.clear();
    error_code = 0;

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

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;
    unsigned int consumed = 0;

    unsigned int affected = 0;
    bool got_ready        = false;
    bool has_error        = false;

    while (!got_ready)
    {
        n = read_loop();
        if (n == 0)
        {
            // 区分断线 vs 语法错误:
            // - 语法错误: 服务器返回 'E' 错误消息, read_loop 能读到数据 (n > 0)
            if (is_last_error_reconnectable())
            {
                isclose = true;
            }
            if (error_msg.empty())
                error_msg = "exec_dml: read error";
            return (unsigned int)-1;
        }
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

        unsigned int parse_offset = consumed;
        while (parse_offset < accum_buf.size())
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = peek_pg_message(
                accum_buf.data(), static_cast<unsigned int>(accum_buf.size()),
                parse_offset, payload_start, payload_len);
            if (msg_type == 0)
                break;// 不完整消息，留待下次读后继续

            switch (msg_type)
            {
            case 'C':// CommandComplete
                affected = parse_command_complete(payload_start, payload_len);
                break;
            case 'Z':// ReadyForQuery：唯一合法终止点
                if (payload_len >= 1)
                    in_transaction_.store(payload_start[0] == 'T' || payload_start[0] == 'E');
                got_ready = true;
                break;
            case 'E':// ErrorResponse
                parse_error_from_payload(payload_start, payload_len);
                has_error = true;// 不直接返回：继续消费到 'Z'，保持连接可用
                break;
            default:
                break;// 忽略 T, D, N, I 等
            }
        }
        consumed = parse_offset;
    }

    if (has_error)
    {
        return (unsigned int)-1;
    }
    return affected;
}

asio::awaitable<unsigned int> pg_conn_base::async_exec_dml(const std::string &sql)
{
    unsigned int retry = 0;
RETRY_LABEL:
    if (isclose)
    {
        co_return (unsigned int)-1;
    }

    error_msg.clear();
    error_code = 0;

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

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;
    unsigned int consumed = 0;

    unsigned int affected = 0;
    bool got_ready        = false;
    bool has_error        = false;

    while (!got_ready)
    {
        n = co_await async_read_loop();
        if (n == 0)
        {
            // 区分断线 vs 语法错误:
            // - 语法错误: 服务器返回 'E' 错误消息, async_read_loop 能读到数据 (n > 0),
            if (is_last_error_reconnectable())
            {
                isclose = true;
            }
            if (error_msg.empty())
                error_msg = "async_exec_dml: read error";
            co_return (unsigned int)-1;
        }
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

        unsigned int parse_offset = consumed;
        while (parse_offset < accum_buf.size())
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = peek_pg_message(
                accum_buf.data(), static_cast<unsigned int>(accum_buf.size()),
                parse_offset, payload_start, payload_len);
            if (msg_type == 0)
                break;

            switch (msg_type)
            {
            case 'C':
                affected = parse_command_complete(payload_start, payload_len);
                break;
            case 'Z':
                if (payload_len >= 1)
                    in_transaction_.store(payload_start[0] == 'T' || payload_start[0] == 'E');
                got_ready = true;
                break;
            case 'E':
                parse_error_from_payload(payload_start, payload_len);
                has_error = true;
                break;
            default:
                break;
            }
        }
        consumed = parse_offset;
    }

    if (has_error)
    {
        co_return (unsigned int)-1;
    }
    co_return affected;
}

unsigned int pg_conn_base::fetch_directly_impl(
    const std::string &sql,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    if (isclose)
    {
        return 0;
    }

    error_msg.clear();
    error_code = 0;

    unsigned int n = write_sql(sql);
    if (n == 0)
    {
        return 0;
    }

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;
    unsigned int consumed = 0;

    std::vector<field_info_t> fields_cache;// T 消息解析一次，org_name 作为列名数组（与 MySQL/SQLite handler 约定一致）
    std::vector<char *> name_ptrs_cache;   // 函数局部，生命周期覆盖本次查询（MySQL P0-7 悬垂指针教训）

    // 行内各列 (ptr, len) 一遍扫描预存（避免 get_data 每列重扫）；函数局部，禁用 static thread_local
    struct col_ref
    {
        unsigned char *ptr;// nullptr 即 NULL 值
        size_t len;
    };
    std::vector<col_ref> cols_cache;

    unsigned int row_num     = 0;
    bool got_ready           = false;
    bool handler_aborted     = false;

    while (!got_ready)
    {
        n = read_loop();
        if (n == 0)
        {
            if (error_msg.empty())
                error_msg = "fetch_directly: read error";
            return row_num;
        }
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

        unsigned int parse_offset = consumed;
        while (parse_offset < accum_buf.size())
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = peek_pg_message(
                accum_buf.data(), static_cast<unsigned int>(accum_buf.size()),
                parse_offset, payload_start, payload_len);
            if (msg_type == 0)
                break;

            switch (msg_type)
            {
            case 'T':// RowDescription：列信息只解析一次（协议保证 T 在第一个 D 之前）
                fields_cache = parse_row_description(payload_start, payload_len);
                name_ptrs_cache.resize(fields_cache.size());
                for (size_t i = 0; i < fields_cache.size(); i++)
                {
                    name_ptrs_cache[i] = const_cast<char *>(fields_cache[i].org_name.c_str());
                }
                break;
            case 'D':// DataRow
            {
                if (handler_aborted)
                    break;// 已中止/出错：只消费不解析，排干到 'Z' 保持连接可用
                if (!handler)
                    break;

                cols_cache.clear();
                unsigned int pos = 0;
                if (payload_len >= 2)
                {
                    int16_t row_col_count = buf_to_int16(payload_start + pos);
                    pos += 2;
                    if (row_col_count > 0 && row_col_count <= 4096)
                    {
                        cols_cache.reserve(row_col_count);
                        for (int16_t ci = 0; ci < row_col_count && pos + 4 <= payload_len; ci++)
                        {
                            int32_t col_len = buf_to_int32(payload_start + pos);
                            pos += 4;
                            if (col_len == -1)
                            {
                                cols_cache.push_back({nullptr, 0});
                            }
                            else if (col_len >= 0 && pos + static_cast<unsigned int>(col_len) <= payload_len)
                            {
                                cols_cache.push_back({const_cast<unsigned char *>(payload_start + pos),
                                                      static_cast<size_t>(col_len)});
                                pos += static_cast<unsigned int>(col_len);
                            }
                            else
                            {
                                break;// 行数据不完整，丢弃剩余列
                            }
                        }
                    }
                }

                // get_data 闭包：O(1) 取列；指针指向 accum_buf 内，回调返回前有效，
                // 回调内需保留的数据自行拷贝（assign_field_value 已拷贝进 model）
                auto get_data = [&cols_cache](int col_idx) -> std::tuple<unsigned char *, size_t> {
                    if (col_idx < 0 || static_cast<size_t>(col_idx) >= cols_cache.size())
                        return {nullptr, 0};
                    return {cols_cache[col_idx].ptr, cols_cache[col_idx].len};
                };

                char **col_names_arr = name_ptrs_cache.empty() ? nullptr : name_ptrs_cache.data();
                bool keep_going      = handler(static_cast<int>(fields_cache.size()), col_names_arr, get_data);
                row_num++;
                if (!keep_going)
                {
                    handler_aborted = true;// 中止后不直接 return：'C'/'Z' 未消费，排干后再返回避免流错位
                }
                break;
            }
            case 'C':// CommandComplete：记录但不终止（'Z' 还在后面）
                break;
            case 'Z':// ReadyForQuery：唯一合法终止点
                if (payload_len >= 1)
                    in_transaction_.store(payload_start[0] == 'T' || payload_start[0] == 'E');
                got_ready = true;
                break;
            case 'E':// ErrorResponse
                parse_error_from_payload(payload_start, payload_len);
                handler_aborted = true;// 同样排干到 'Z'，保持连接可用
                break;
            default:
                break;// N, I 等忽略
            }
        }
        consumed = parse_offset;
        // 定期压缩已消费数据，避免大结果集 O(n^2) 拷贝（压缩发生在所有回调返回之后，指针安全）
        if (consumed > 256 * 1024)
        {
            accum_buf.erase(accum_buf.begin(), accum_buf.begin() + consumed);
            consumed = 0;
        }
    }

    return row_num;
}

asio::awaitable<unsigned int> pg_conn_base::async_fetch_directly_impl(
    const std::string &sql,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    if (isclose)
    {
        co_return 0;
    }

    error_msg.clear();
    error_code = 0;

    unsigned int n = co_await async_write_sql(sql);
    if (n == 0)
    {
        co_return 0;
    }

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;
    unsigned int consumed = 0;

    std::vector<field_info_t> fields_cache;
    std::vector<char *> name_ptrs_cache;

    struct col_ref
    {
        unsigned char *ptr;
        size_t len;
    };
    std::vector<col_ref> cols_cache;

    unsigned int row_num     = 0;
    bool got_ready           = false;
    bool handler_aborted     = false;

    while (!got_ready)
    {
        n = co_await async_read_loop();
        if (n == 0)
        {
            if (error_msg.empty())
                error_msg = "async_fetch_directly: read error";
            co_return row_num;
        }
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);

        unsigned int parse_offset = consumed;
        while (parse_offset < accum_buf.size())
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = peek_pg_message(
                accum_buf.data(), static_cast<unsigned int>(accum_buf.size()),
                parse_offset, payload_start, payload_len);
            if (msg_type == 0)
                break;

            switch (msg_type)
            {
            case 'T':
                fields_cache = parse_row_description(payload_start, payload_len);
                name_ptrs_cache.resize(fields_cache.size());
                for (size_t i = 0; i < fields_cache.size(); i++)
                {
                    name_ptrs_cache[i] = const_cast<char *>(fields_cache[i].org_name.c_str());
                }
                break;
            case 'D':
            {
                if (handler_aborted)
                    break;
                if (!handler)
                    break;

                cols_cache.clear();
                unsigned int pos = 0;
                if (payload_len >= 2)
                {
                    int16_t row_col_count = buf_to_int16(payload_start + pos);
                    pos += 2;
                    if (row_col_count > 0 && row_col_count <= 4096)
                    {
                        cols_cache.reserve(row_col_count);
                        for (int16_t ci = 0; ci < row_col_count && pos + 4 <= payload_len; ci++)
                        {
                            int32_t col_len = buf_to_int32(payload_start + pos);
                            pos += 4;
                            if (col_len == -1)
                            {
                                cols_cache.push_back({nullptr, 0});
                            }
                            else if (col_len >= 0 && pos + static_cast<unsigned int>(col_len) <= payload_len)
                            {
                                cols_cache.push_back({const_cast<unsigned char *>(payload_start + pos),
                                                      static_cast<size_t>(col_len)});
                                pos += static_cast<unsigned int>(col_len);
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }

                auto get_data = [&cols_cache](int col_idx) -> std::tuple<unsigned char *, size_t> {
                    if (col_idx < 0 || static_cast<size_t>(col_idx) >= cols_cache.size())
                        return {nullptr, 0};
                    return {cols_cache[col_idx].ptr, cols_cache[col_idx].len};
                };

                char **col_names_arr = name_ptrs_cache.empty() ? nullptr : name_ptrs_cache.data();
                bool keep_going      = handler(static_cast<int>(fields_cache.size()), col_names_arr, get_data);
                row_num++;
                if (!keep_going)
                {
                    handler_aborted = true;
                }
                break;
            }
            case 'C':
                break;
            case 'Z':
                if (payload_len >= 1)
                    in_transaction_.store(payload_start[0] == 'T' || payload_start[0] == 'E');
                got_ready = true;
                break;
            case 'E':
                parse_error_from_payload(payload_start, payload_len);
                handler_aborted = true;
                break;
            default:
                break;
            }
        }
        consumed = parse_offset;
        if (consumed > 256 * 1024)
        {
            accum_buf.erase(accum_buf.begin(), accum_buf.begin() + consumed);
            consumed = 0;
        }
    }

    co_return row_num;
}

// ===== 断线自动重连实现 =====

bool pg_conn_base::is_last_error_reconnectable() const
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

bool pg_conn_base::try_reconnect()
{
    if (!has_conn_config_)   return false;
    if (isclose)             return false;

    hard_close();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return connect(last_conn_config_);
}

asio::awaitable<bool> pg_conn_base::async_try_reconnect()
{
    if (!has_conn_config_)   co_return false;
    if (isclose)             co_return false;

    hard_close();

    asio::steady_timer t(*conn_link->io_ctx, std::chrono::milliseconds(50));
    co_await t.async_wait(asio::use_awaitable);

    co_return co_await async_connect(last_conn_config_);
}

}// namespace orm
