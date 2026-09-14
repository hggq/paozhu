/*
 * 流程结构来自 mysql_conn
 * The process structure comes from mysql_conn
 */
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <charconv>
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

// SASL 用户名转义——用户名可含 '=' 与 ','（RFC 5802），
// 必须转义为 =3D / =2C，否则服务器解析用户名出错（认证失败或错位）
std::string escape_sasl_name(const std::string &name)
{
    std::string out;
    out.reserve(name.size());
    for (unsigned char c : name)
    {
        if (c == '=')
            out += "=3D";
        else if (c == ',')
            out += "=2C";
        else
            out.push_back(static_cast<char>(c));
    }
    return out;
}

std::string generate_nonce(size_t len = 18)
{
    if (len > 18)
        len = 18;
    unsigned char buf[18];
    // RAND_bytes 失败（随机源不可用）时返回空 nonce，使认证必然失败，
    // 绝不静默使用弱/确定性数据
    if (RAND_bytes(buf, len) != 1)
    {
        return std::string();
    }
    return base64_encode(buf, len);
}

std::string xor_strings(const std::string &a, const std::string &b)
{
    // 长度不匹配说明 SCRAM 实现/协议错误（两值应为 32 字节），
    // 不得再静默返回明文；返回空串使 proof 无效、认证失败
    if (a.size() != b.size())
        return std::string();
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
        while (pos < payload.size() && payload[pos] != 0)
            value.push_back(payload[pos++]);
        if (pos >= payload.size())
        {
            // 消息截断：value 无 null 终止符，直接退出避免跳过不存在的 null
            break;
        }
        pos++;// 跳过 null 终止符
        if (field == 'M')
            error_msg = value;
    }
    if (error_msg.empty())
        error_msg = "unknown error";
    error_code = 11;
    isclose    = true;
}

// 构造"不支持的服务端认证方式"报错。设计口径：本客户端仅支持 trust(AuthenticationOk,0)
// 与 SCRAM-SHA-256(10/11/12，PostgreSQL 10+ 默认)；md5/明文/GSS/SSPI 等旧认证为有意不支持
// （PG 14 起服务端已移除 md5），命中时给出可操作的修复提示而非仅回数字。
static std::string unsupported_auth_message(int32_t auth_type)
{
    const char *name = "unknown";
    switch (auth_type)
    {
    case PG_AUTH_KERBEROS_V5: name = "Kerberos V5"; break;
    case PG_AUTH_CLEARTEXT_PASSWORD: name = "cleartext password (pg_hba method 'password')"; break;
    case PG_AUTH_MD5_PASSWORD: name = "MD5 password (pg_hba method 'md5')"; break;
    case PG_AUTH_SCM_CREDENTIAL: name = "SCM credential"; break;
    case PG_AUTH_GSS: name = "GSSAPI"; break;
    case PG_AUTH_GSS_CONTINUE: name = "GSSAPI continue"; break;
    case PG_AUTH_SSPI: name = "SSPI"; break;
    default: break;
    }
    std::string err = "unsupported authentication method: ";
    err += name;
    err += " (auth code " + std::to_string(auth_type) + "). ";
    err += "This client supports only trust and SCRAM-SHA-256 (PostgreSQL 10+ default), ";
    err += "not MD5/cleartext/GSS/SSPI. ";
    err += "Fix: set pg_hba.conf password rows to method 'scram-sha-256' ";
    err += "(md5 authentication was removed in PostgreSQL 14).";
    return err;
}

// NoticeResponse('N') 载荷解析。真实结构为：自首字节起循环 {1字节字段码, C风格字符串}，直到字段码 0
// 结束。旧实现误按首字节=='C' 后取 'S' 字段解析，与 parse_error_from_payload 的结构不符，导致
// 实际 notices_ 恒为空；此处与 parse_error_from_payload 同构，仅把 'M'(正文) 字段收集到 notices_。
static void collect_notice_messages(std::vector<std::string> &notices,
                                    const unsigned char *data,
                                    unsigned int len)
{
    unsigned int pos = 0;
    while (pos < len)
    {
        unsigned char ftype = data[pos++];
        if (ftype == '\0')
            break;
        size_t str_end = pos;
        while (str_end < len && data[str_end] != '\0')
            str_end++;
        if (ftype == 'M')
            notices.emplace_back(reinterpret_cast<const char *>(data + pos), str_end - pos);
        if (str_end >= len)
            break;
        pos = static_cast<unsigned int>(str_end) + 1;
    }
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
        error_msg  = unsupported_auth_message(auth_type);
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

    std::string client_nonce = generate_nonce(18);
    // 用户名必须 SASL 转义
    std::string client_first_bare    = "n=" + escape_sasl_name(conn_config.user) + ",r=" + client_nonce;
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
    // 认证流程任一步都可能先收到 ErrorResponse('E')（机制被禁/角色不存在等），需解析透传，而非静默失败
    if (!read_message_sync(msg_type, payload))
        return false;
    if (msg_type == 'E')
    {
        parse_error(payload);
        return false;
    }
    if (msg_type != 'R')
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

    std::string salt = base64_decode(salt_b64);
    // 口令统一为纯 ASCII，SCRAM 免去 SASLprep/NFKC，直接参与 PBKDF2
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

    // 口令错误/服务端拒绝通常在此步以 ErrorResponse 返回（SQLSTATE 28P01），解析透传服务器原文
    if (!read_message_sync(msg_type, payload))
        return false;
    if (msg_type == 'E')
    {
        parse_error(payload);
        return false;
    }
    if (msg_type != 'R')
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
            const unsigned char *p   = reinterpret_cast<const unsigned char *>(payload.data());
            const unsigned char *end = p + payload.size();
            const char *name_start   = reinterpret_cast<const char *>(p);
            const char *name_end     = static_cast<const char *>(memchr(name_start, '\0', end - p));
            if (name_end && name_end < reinterpret_cast<const char *>(end))
            {
                std::string param_name(name_start, name_end);
                const char *val_start = name_end + 1;
                const char *val_end   = static_cast<const char *>(memchr(val_start, '\0', end - reinterpret_cast<const unsigned char *>(val_start)));
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
            // NoticeResponse 载荷为 {1字节字段码, 字符串}\0...，取 'M'(正文) 入 notices_
            collect_notice_messages(notices_, reinterpret_cast<const unsigned char *>(payload.data()), static_cast<unsigned int>(payload.size()));
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
    conn_link->sock_type = 0;
    conn_link->sslsocket.reset();
    conn_link->ssl_context.reset();
    conn_link->localsocket.reset();
    conn_link->socket = std::make_unique<asio::ip::tcp::socket>(*conn_link->io_ctx);
    conn_link->ec.clear();
    server_enable_ssl = false;
    in_transaction_.store(false);
    notices_.clear();

    // 保存配置用于断线自动重连（仅在首次或配置变化时存储）
    if (!has_conn_config_ || !(last_conn_config_ == conn_config))
    {
        last_conn_config_ = conn_config;
        has_conn_config_  = true;
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
            if (conn_config.sslverify)
            {
                // sslverify 开启时做主机名校验；未显式配置 sslhost 时以连接 host 兜底，
                // 避免"只验证书链不验主机名"的中间人缝隙
                conn_link->sslsocket->set_verify_callback(
                    asio::ssl::host_name_verification(conn_config.sslhost.empty() ? conn_config.host : conn_config.sslhost));
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
    conn_link->sock_type = 0;
    conn_link->sslsocket.reset();
    conn_link->ssl_context.reset();
    conn_link->localsocket.reset();
    conn_link->socket = std::make_unique<asio::ip::tcp::socket>(*conn_link->io_ctx);
    conn_link->ec.clear();
    server_enable_ssl = false;
    in_transaction_.store(false);
    notices_.clear();

    // 保存配置用于断线自动重连（仅在首次或配置变化时存储）
    if (!has_conn_config_ || !(last_conn_config_ == conn_config))
    {
        last_conn_config_ = conn_config;
        has_conn_config_  = true;
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
            if (conn_config.sslverify)
            {
                // sslverify 开启时做主机名校验；未显式配置 sslhost 时以连接 host 兜底
                conn_link->sslsocket->set_verify_callback(
                    asio::ssl::host_name_verification(conn_config.sslhost.empty() ? conn_config.host : conn_config.sslhost));
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

                std::string client_nonce = generate_nonce(18);
                // 用户名必须 SASL 转义（同步路径 sasl_scram_sha256_sync 已做；此处异步内联副本遗漏，补回）
                std::string client_first_bare    = "n=" + escape_sasl_name(conn_config.user) + ",r=" + client_nonce;
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

                std::string salt = base64_decode(salt_b64);
                // 口令统一为纯 ASCII，SCRAM 免去 SASLprep/NFKC，直接参与 PBKDF2
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
                // 长度强校验与 read_message_sync 顶层一致（<4 或 >64MB 即非法）。旧实现仅在
                // <=8MB 时消费载荷，超限(但仍<=外层64MB)时既不消费也不置 auth_done，外层循环会把
                // 载荷字节误当消息头继续解析，造成协议错位/挂死。
                if (msg_len < 4 || msg_len > 64 * 1024 * 1024)
                {
                    error_msg  = "invalid final auth message length: " + std::to_string(msg_len);
                    error_code = 21;
                    isclose    = true;
                    co_return false;
                }
                // 无论 msg_type 是 'R'/'E'/其它，都把载荷完整读出并消费，再行判定
                std::vector<char> final_payload(msg_len - 4);
                try
                {
                    if (conn_link->sock_type == 2)
                        co_await asio::async_read(*conn_link->sslsocket, asio::buffer(final_payload), asio::use_awaitable);
                    else if (conn_link->sock_type == 3)
                        co_await asio::async_read(*conn_link->localsocket, asio::buffer(final_payload), asio::use_awaitable);
                    else
                        co_await asio::async_read(*conn_link->socket, asio::buffer(final_payload), asio::use_awaitable);
                }
                catch (const std::exception &e)
                {
                    error_msg  = "read final auth payload error: " + std::string(e.what());
                    error_code = 21;
                    isclose    = true;
                    co_return false;
                }

                // (async): 口令错误/拒绝通常以 ErrorResponse('E') 返回（SQLSTATE 28P01 等），
                // 解析其 'M' 字段透传服务器原文，而非笼统报 "unexpected message type"
                if (msg_type == 'E')
                {
                    parse_error_from_payload(reinterpret_cast<const unsigned char *>(final_payload.data()),
                                             static_cast<unsigned int>(final_payload.size()));
                    error_code = 21;// 认证失败码（parse_error_from_payload 默认 10 是查询错）
                    isclose    = true;
                    co_return false;
                }
                if (msg_type != 'R')
                {
                    error_msg  = "unexpected message type after SASL final request: got " + std::to_string(msg_type);
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
            else
            {
                error_msg  = unsupported_auth_message(auth_type);
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
            const unsigned char *p   = reinterpret_cast<const unsigned char *>(payload.data());
            const unsigned char *end = p + payload.size();
            const char *name_start   = reinterpret_cast<const char *>(p);
            const char *name_end     = static_cast<const char *>(memchr(name_start, '\0', end - p));
            if (name_end && name_end < reinterpret_cast<const char *>(end))
            {
                std::string param_name(name_start, name_end);
                const char *val_start = name_end + 1;
                const char *val_end   = static_cast<const char *>(memchr(val_start, '\0', end - reinterpret_cast<const unsigned char *>(val_start)));
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
            // NoticeResponse 载荷为 {1字节字段码, 字符串}\0...，取 'M'(正文) 入 notices_
            collect_notice_messages(notices_, reinterpret_cast<const unsigned char *>(payload.data()), static_cast<unsigned int>(payload.size()));
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
        int32_t len           = 4 + static_cast<int32_t>(sizeof(ping_sql) - 1) + 1;
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
        // 主动关闭前先发送 Terminate('X')（best effort），使后端按正常结束回收连接，
        // 而非记录 "unexpected EOF on client connection"。写失败仅说明连接已不可用，忽略即可。
        unsigned char xbuf[5] = {'X', 0, 0, 0, 4};
        if (conn_link->sock_type == 1 && conn_link->socket && conn_link->socket->is_open())
            asio::write(*conn_link->socket, asio::buffer(xbuf, 5), conn_link->ec);
        else if (conn_link->sock_type == 2 && conn_link->sslsocket && conn_link->sslsocket->lowest_layer().is_open())
            asio::write(*conn_link->sslsocket, asio::buffer(xbuf, 5), conn_link->ec);
        else if (conn_link->sock_type == 3 && conn_link->localsocket && conn_link->localsocket->is_open())
            asio::write(*conn_link->localsocket, asio::buffer(xbuf, 5), conn_link->ec);
        conn_link->ec.clear();

        isclose = true;
        prepared_meta_.clear();// 仅清客户端 prepared 元数据缓存，不重置 name_counter_（避免 42P07 撞名）
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
        // 缓存的 (OID, 格式码) 描述的是旧会话：try_reconnect() 必经此路，故这里是唯一失效点
        prepared_meta_.clear();
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
        // 异步主动关闭前先发送 Terminate('X')（best effort），失败仅说明连接已不可用，忽略
        unsigned char xbuf[5] = {'X', 0, 0, 0, 4};
        try
        {
            if (conn_link->sock_type == 1 && conn_link->socket && conn_link->socket->is_open())
                co_await asio::async_write(*conn_link->socket, asio::buffer(xbuf, 5), asio::use_awaitable);
            else if (conn_link->sock_type == 2 && conn_link->sslsocket && conn_link->sslsocket->lowest_layer().is_open())
                co_await asio::async_write(*conn_link->sslsocket, asio::buffer(xbuf, 5), asio::use_awaitable);
            else if (conn_link->sock_type == 3 && conn_link->localsocket && conn_link->localsocket->is_open())
                co_await asio::async_write(*conn_link->localsocket, asio::buffer(xbuf, 5), asio::use_awaitable);
        }
        catch (...)
        {
        }

        isclose = true;
        prepared_meta_.clear();// 仅清客户端 prepared 元数据缓存，不重置 name_counter_（避免 42P07 撞名）
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

// PG 类型 OID -> MySQL 字段类型码映射（field_type 以 MySQL 为准，
// 不能直接取 type_oid 低 8 位，否则与 MySQL 类型码语义冲突）
unsigned char pg_oid_to_mysql_type(unsigned int type_oid)
{
    switch (type_oid)
    {
    case 16:        // bool
        return 0x01;// MYSQL_TYPE_TINY
    case 17:        // bytea
        return 0xFC;// MYSQL_TYPE_BLOB
    case 18:        // char
        return 0xFE;// MYSQL_TYPE_STRING
    case 19:        // name
        return 0xFD;// MYSQL_TYPE_VAR_STRING
    case 20:        // int8 (bigint)
        return 0x08;// MYSQL_TYPE_LONGLONG
    case 21:        // int2
        return 0x02;// MYSQL_TYPE_SHORT
    case 23:        // int4
        return 0x03;// MYSQL_TYPE_LONG
    case 25:        // text
        return 0xFC;// MySQL text 对应 BLOB 类型码
    case 26:        // oid
        return 0x08;// MYSQL_TYPE_LONGLONG
    case 114:       // json
    case 3802:      // jsonb
        return 0xF5;// MYSQL_TYPE_JSON
    case 700:       // float4
        return 0x04;// MYSQL_TYPE_FLOAT
    case 701:       // float8
        return 0x05;// MYSQL_TYPE_DOUBLE
    case 790:       // money
        return 0xF6;// MYSQL_TYPE_NEWDECIMAL
    case 869:       // inet
    case 2950:      // uuid
    case 1042:      // bpchar
        return 0xFE;// MYSQL_TYPE_STRING
    case 1043:      // varchar
        return 0xFD;// MYSQL_TYPE_VAR_STRING
    case 1082:      // date
        return 0x0A;// MYSQL_TYPE_DATE
    case 1083:      // time
    case 1266:      // timetz
        return 0x0B;// MYSQL_TYPE_TIME
    case 1114:      // timestamp
    case 1184:      // timestamptz
        return 0x0C;// MYSQL_TYPE_DATETIME
    case 1560:      // bit
        return 0x10;// MYSQL_TYPE_BIT
    case 1700:      // numeric / decimal（定点数，既非 int 也非 float）
        return 0xF6;// MYSQL_TYPE_NEWDECIMAL
    default:
        return 0xFD;// MYSQL_TYPE_VAR_STRING
    }
}

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

        // type OID (4 bytes) —— 映射为 MySQL 字段类型码（以 MySQL 类型为准）
        uint32_t type_oid = static_cast<uint32_t>(buf_to_int32(data + pos));
        fi.field_type     = pg_oid_to_mysql_type(type_oid);
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

// ---- W3-5：帧机械的唯一实现（§18.2#2 收窄版）----
// 只合并"喂一批字节 + 取一条完整帧"这套机械（累计上限、残帧压缩、consumed 推进、分帧），
// 各路径的消息 switch 语义不动 —— text 路径要 field_infos、binary 路径要格式码，
// 强行合并发派会为了 DRY 引入分支参数。
// 改前这 10 条累积式读循环各写一遍机械（查询 2 + DML 2 + 直取 2 + async 文本预编译 2 + binary 2），
// 压缩策略就有三种：binary 两条每批都压、查询/直取四条到 256KB 才压、
// DML 两条与 async 文本预编译两条从不压 —— 同一个协议动作三份实现，
// 双胞胎路径漏写一处就是 §2.3#1 那类缺陷的成因。这里把机械收成一份：
// 已有压缩的路径用 compact_at 原样保住策略（binary 传 0，查询/直取传 256KB）；
// 四条从不压缩的路径顺带拿到 256KB 阀值 —— 对外值不变，只是已消费前缀不再一路攒着。
// 另有两条 sync 文本预编译路径逐条 read_message_sync、没有累积缓冲 ⇒ 不在本次合并范围。
// 缓冲所有权始终在调用方（std::string 或 pooled_accum_buf ⇒ 共用容量池语义不变），
// 这里按引用操作并用模板覆盖两种字节容器。
// 已消费前缀超过这个长度才压缩。压缩本身是一次 memmove，每批（4KB 量级）都压会把
// "排空缓冲"退化成 O(n^2) 拷贝；阈值换的是"峰值多持有 256KB"。
// binary 预编译两条传 0（每批都压）—— 那是它们改前就有的策略，一条在追加前压、
// 一条在排空后压，两件事在时序上等价，收成一份后由 compact_at 表达。
static constexpr unsigned int k_pg_compact_at = 256 * 1024;

// 流式路径（handler 逐行消费）传这个值表示"只压缩、不按总字节数截断"：
// 残帧压缩已经把缓冲峰值钉在 k_pg_compact_at + 一批，行数上限归业务管。
static constexpr size_t k_pg_no_byte_cap = static_cast<size_t>(-1);

struct _pg_frame_state
{
    unsigned int consumed   = 0;// 已消费前缀长度 = 下一条帧的起点
    size_t total_read_bytes = 0;
};

// 并入一批刚读到的字节：先按 compact_at 阈值压掉已消费前缀，再追加。
// 返回 false ⇒ 累计字节越过 max_bytes；此时本批**未**并入（与改前各路径一致），
// 调用方按各自的错误码/断连策略收尾。上限检查必须在追加之前，否则越界批先落进缓冲。
template <class Buf>
static bool _pg_feed(Buf &accum, _pg_frame_state &st, const char *src, size_t n, size_t max_bytes, unsigned int compact_at)
{
    st.total_read_bytes += n;
    if (st.total_read_bytes > max_bytes)
        return false;
    using V = typename Buf::value_type;
    if (st.consumed > compact_at)
    {
        size_t keep = accum.size() - st.consumed;
        if (keep)
            std::memmove(accum.data(), accum.data() + st.consumed, keep);
        accum.resize(keep);
        st.consumed = 0;
    }
    accum.insert(accum.end(), reinterpret_cast<const V *>(src), reinterpret_cast<const V *>(src + n));
    return true;
}

// 取缓冲里下一条完整帧。返回 0 ⇒ 还没有完整帧，调用方该读一批了
// （peek_pg_message 只在成功时推进 offset，所以 consumed 此时仍指着半包起点）。
// 成功时 consumed 已推进到下一帧起始；payload 指向本帧载荷，有效期到下一次 _pg_feed
// （压缩或追加都会动这块内存）⇒ 处理与 handler 调用都必须在本次迭代内完成。
template <class Buf>
static unsigned char _pg_next_frame(Buf &accum, _pg_frame_state &st, const unsigned char *&payload, unsigned int &payload_len)
{
    return peek_pg_message(reinterpret_cast<const unsigned char *>(accum.data()),
                           static_cast<unsigned int>(accum.size()),
                           st.consumed,
                           payload,
                           payload_len);
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
    this->clear_error();// 入口清陈旧错误状态，避免上次失败残留 error_code=10 污染本次结果判定

    unsigned int n = write_sql(sql);
    if (n == 0)
        return 1;

    // We need to accumulate data and parse messages
    // Simple approach: read into buffer, parse messages as they arrive
    std::string accum_buf;
    bool got_ready = false;
    _pg_frame_state fs;
    // 结果集上限 16MB，与 MySQL 协议单包上限保持一致；
    // 按累计读取字节数限制（替代原先 1000 轮 * 4KB 的轮次上限）
    constexpr size_t k_max_result_bytes = 16u * 1024u * 1024u;

    while (!got_ready)
    {
        n = read_loop();
        if (n == 0)
        {
            error_msg = "read error during query";
            return 1;
        }
        if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_max_result_bytes, k_pg_compact_at))
        {
            error_msg  = "execute_and_fetch: result set exceeds 16MB limit";
            error_code = 8;
            isclose    = true;// 结果集过大，断开连接避免脏状态
            return 1;
        }

        // Try to parse all complete messages in accum_buf
        while (true)
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = _pg_next_frame(accum_buf, fs, payload_start, payload_len);

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
                // 载荷为 {1字节字段码, 字符串}\0...，取 'M'(正文) 入 notices_
                collect_notice_messages(notices_, payload_start, payload_len);
                break;
            }
            case 'I':// EmptyQueryResponse - 正常忽略
                break;
            case 'G':// CopyInResponse(COPY ... FROM STDIN) — 服务端等待客户端持续供数据，
                     // 本客户端不支持；后端不会自行回 Z，报错并断连避免假死
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                return 1;
            default:
                break;
            }
        }
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
    this->clear_error();// 入口清陈旧错误状态，避免上次失败残留 error_code=10 污染本次结果判定

    unsigned int n = co_await async_write_sql(sql);
    if (n == 0)
        co_return 1;

    std::string accum_buf;
    bool got_ready = false;
    _pg_frame_state fs;
    // 结果集上限 16MB，与 MySQL 协议单包上限保持一致
    constexpr size_t k_max_result_bytes = 16u * 1024u * 1024u;

    while (!got_ready)
    {
        n = co_await async_read_loop();
        if (n == 0)
        {
            error_msg = "async read error during query";
            co_return 1;
        }
        if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_max_result_bytes, k_pg_compact_at))
        {
            error_msg  = "async_execute_and_fetch: result set exceeds 16MB limit";
            error_code = 8;
            isclose    = true;
            co_return 1;
        }

        while (true)
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = _pg_next_frame(accum_buf, fs, payload_start, payload_len);

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
                // 载荷为 {1字节字段码, 字符串}\0...，取 'M'(正文) 入 notices_
                collect_notice_messages(notices_, payload_start, payload_len);
                break;
            }
            case 'I':
                break;
            case 'G':// CopyInResponse(COPY ... FROM STDIN) — 不支持，报错断连避免假死
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                co_return 1;
            default:
                break;
            }
        }
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

static bool _pg_is_ddl(const std::string &s)
{
    // 保守 DDL 判定：trim 前导空白后取首词，不解析完整语法，避免误伤 SELECT
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
        i++;
    size_t sp      = s.find(' ', i);
    std::string kw = (sp == std::string::npos) ? s.substr(i) : s.substr(i, sp - i);
    return kw == "CREATE" || kw == "ALTER" || kw == "DROP" || kw == "TRUNCATE" || kw == "RENAME" || kw == "COMMENT";
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

    unsigned int affected = 0;
    bool got_ready        = false;
    bool has_error        = false;
    // DML 响应（正常应仅 C+Z，若有函数/触发器大量 RAISE NOTICE 等）也设 16MB 累计上限，与查询路径一致
    _pg_frame_state fs;
    constexpr size_t k_max_dml_bytes = 16u * 1024u * 1024u;

    while (!got_ready)
    {
        n = read_loop();
        if (n == 0)
        {
            // 对端被杀/断连 → 读立即失败. 自动提交下 PG 断连回滚隐式事务, DML 未落库, 可安全重连重发;
            // 显式事务内不可重发(重复执行风险)
            if (!in_transaction_.load() && retry < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
            {
                retry++;
                error_msg.clear();
                goto RETRY_LABEL;
            }
            isclose = true;
            if (error_msg.empty())
                error_msg = "exec_dml: read error";
            return (unsigned int)-1;
        }
        // 改前这条路径从不压缩已消费前缀（RAISE NOTICE 风暴下缓冲会一路长到 16MB），
        // 用共用机械后与查询路径同阀值。
        if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_max_dml_bytes, k_pg_compact_at))
        {
            // 超过上限按协议异常处理：断连，避免无界缓存
            error_msg  = "exec_dml: response exceeds 16MB limit";
            error_code = 8;
            isclose    = true;
            return (unsigned int)-1;
        }

        while (true)
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
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
            case 'G':// CopyInResponse(COPY ... FROM STDIN) — 不支持，报错断连避免假死
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                return (unsigned int)-1;
            default:
                break;// 忽略 T, D, N, I 等
            }
        }
    }

    if (has_error)
    {
        return (unsigned int)-1;
    }
    if (error_code == 0 && _pg_is_ddl(sql))
        prepared_meta_.mark_all_needs_reparse();
    return affected;
}

asio::awaitable<unsigned int> pg_conn_base::async_exec_dml(const std::string &sql)
{
    unsigned int retry = 0;
RETRY_LABEL:
    if (isclose)
    {
        co_return (unsigned int) - 1;
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
        co_return (unsigned int) - 1;
    }

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;

    unsigned int affected = 0;
    bool got_ready        = false;
    bool has_error        = false;
    // DML 响应累计上限 16MB，与查询路径保持一致（防无界缓存）
    _pg_frame_state fs;
    constexpr size_t k_max_dml_bytes = 16u * 1024u * 1024u;

    while (!got_ready)
    {
        n = co_await async_read_loop();
        if (n == 0)
        {
            // 对端被杀/断连 → 读立即失败. 自动提交下 PG 断连回滚隐式事务, DML 未落库, 可安全重连重发
            if (!in_transaction_.load() && retry < kMaxReconnect && is_last_error_reconnectable() && co_await async_try_reconnect())
            {
                retry++;
                error_msg.clear();
                goto RETRY_LABEL;
            }
            isclose = true;
            if (error_msg.empty())
                error_msg = "async_exec_dml: read error";
            co_return (unsigned int) - 1;
        }
        // 改前这条路径从不压缩已消费前缀，与 sync 孪生同病；共用机械后同阀值。
        if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_max_dml_bytes, k_pg_compact_at))
        {
            error_msg  = "async_exec_dml: response exceeds 16MB limit";
            error_code = 8;
            isclose    = true;
            co_return (unsigned int) - 1;
        }

        while (true)
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
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
            case 'G':// CopyInResponse(COPY ... FROM STDIN) — 不支持，报错断连避免假死
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                co_return (unsigned int) - 1;
            default:
                break;
            }
        }
    }

    if (has_error)
    {
        co_return (unsigned int) - 1;
    }
    if (error_code == 0 && _pg_is_ddl(sql))
        prepared_meta_.mark_all_needs_reparse();
    co_return affected;
}

unsigned int pg_conn_base::fetch_directly_impl(
    const std::string &sql,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    unsigned int retry = 0;
RETRY_LABEL:
    if (isclose)
    {
        return 0;
    }

    error_msg.clear();
    error_code = 0;

    unsigned int n = write_sql(sql);
    if (n == 0)
    {
        if (is_last_error_reconnectable() && retry < kMaxReconnect && try_reconnect())
        {
            retry++;
            error_msg.clear();
            goto RETRY_LABEL;
        }
        return 0;
    }

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;

    std::vector<field_info_t> fields_cache;// T 消息解析一次，org_name 作为列名数组（与 MySQL/SQLite handler 约定一致）
    std::vector<char *> name_ptrs_cache;   // 函数局部，生命周期覆盖本次查询（MySQL P0-7 悬垂指针教训）

    // 行内各列 (ptr, len) 一遍扫描预存（避免 get_data 每列重扫）；函数局部，禁用 static thread_local
    struct col_ref
    {
        unsigned char *ptr;// nullptr 即 NULL 值
        size_t len;
    };
    std::vector<col_ref> cols_cache;

    unsigned int row_num = 0;
    bool got_ready       = false;
    bool handler_aborted = false;
    _pg_frame_state fs;

    while (!got_ready)
    {
        n = read_loop();
        if (n == 0)
        {
            // 对端被杀/断连 → 读立即失败(CLOSE_WAIT 假写成功). SELECT 幂等, 可安全重连重发
            if (retry < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
            {
                retry++;
                error_msg.clear();
                goto RETRY_LABEL;
            }
            if (error_msg.empty())
                error_msg = "fetch_directly: read error";
            return row_num;
        }
        // 流式路径不设累计上限（改前也没有）：handler 逐行消费 ⇒ 压缩把缓冲峰值钉在
        // 阀值 + 一批，行数由业务控制，不需要按总字节数截断。
        if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_pg_no_byte_cap, k_pg_compact_at))
            return row_num;

        while (true)
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
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
                auto get_data = [&cols_cache](int col_idx) -> std::tuple<unsigned char *, size_t>
                {
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
            case 'G':// CopyInResponse(COPY ... FROM STDIN) — 不支持，报错断连避免假死
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                return row_num;
            default:
                break;// N, I 等忽略
            }
        }
    }

    if (error_code == 0 && _pg_is_ddl(sql))
        prepared_meta_.mark_all_needs_reparse();
    return row_num;
}

asio::awaitable<unsigned int> pg_conn_base::async_fetch_directly_impl(
    const std::string &sql,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    unsigned int retry = 0;
RETRY_LABEL:
    if (isclose)
    {
        co_return 0;
    }

    error_msg.clear();
    error_code = 0;

    unsigned int n = co_await async_write_sql(sql);
    if (n == 0)
    {
        if (is_last_error_reconnectable() && retry < kMaxReconnect && co_await async_try_reconnect())
        {
            retry++;
            error_msg.clear();
            goto RETRY_LABEL;
        }
        co_return 0;
    }

    pooled_accum_buf accum_holder;// 全 ORM 共用容量池借出/归还（RAII，见 orm_common.h）
    auto &accum_buf = accum_holder.buf;

    std::vector<field_info_t> fields_cache;
    std::vector<char *> name_ptrs_cache;

    struct col_ref
    {
        unsigned char *ptr;
        size_t len;
    };
    std::vector<col_ref> cols_cache;

    unsigned int row_num = 0;
    bool got_ready       = false;
    bool handler_aborted = false;
    _pg_frame_state fs;

    while (!got_ready)
    {
        n = co_await async_read_loop();
        if (n == 0)
        {
            // 对端被杀/断连 → 读立即失败(CLOSE_WAIT 假写成功). SELECT 幂等, 可安全重连重发
            if (retry < kMaxReconnect && is_last_error_reconnectable() && co_await async_try_reconnect())
            {
                retry++;
                error_msg.clear();
                goto RETRY_LABEL;
            }
            if (error_msg.empty())
                error_msg = "async_fetch_directly: read error";
            co_return row_num;
        }
        // 与 sync 孪生同：流式路径靠压缩定峰值，不设累计字节上限。
        if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_pg_no_byte_cap, k_pg_compact_at))
            co_return row_num;

        while (true)
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
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

                auto get_data = [&cols_cache](int col_idx) -> std::tuple<unsigned char *, size_t>
                {
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
            case 'G':// CopyInResponse(COPY ... FROM STDIN) — 不支持，报错断连避免假死
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                co_return row_num;
            default:
                break;
            }
        }
    }

    if (error_code == 0 && _pg_is_ddl(sql))
        prepared_meta_.mark_all_needs_reparse();
    co_return row_num;
}

// ===== 断线自动重连实现 =====

bool pg_conn_base::is_last_error_reconnectable() const
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

bool pg_conn_base::try_reconnect()
{
    if (!has_conn_config_)
        return false;
    if (isclose)
        return false;

    hard_close();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return connect(last_conn_config_);
}

asio::awaitable<bool> pg_conn_base::async_try_reconnect()
{
    if (!has_conn_config_)
        co_return false;
    if (isclose)
        co_return false;

    hard_close();

    asio::steady_timer t(*conn_link->io_ctx, std::chrono::milliseconds(50));
    co_await t.async_wait(asio::use_awaitable);

    co_return co_await async_connect(last_conn_config_);
}

// ===== PostgreSQL Extended Query 预编译协议实现 =====
// MVP: 全部 text format, 每次闭环 (Parse+Bind+Execute+Sync) + (Close+Sync)

namespace
{

// obj_val → text string
std::string _obj_to_text(const http::obj_val &v)
{
    switch (v.get_type())
    {
    case http::obj_type::NIL: return {};
    case http::obj_type::INT: return std::to_string(v.lval);
    case http::obj_type::LONG: return std::to_string(v.lval);
    case http::obj_type::UINT: return std::to_string(v.uval);
    case http::obj_type::ULONG: return std::to_string(v.uval);
    case http::obj_type::DOUBLE:
    {
        char buf[64];
        // 最短往返表示：%.17g 会把 12345.6789 编成 "12345.678900000001"，
        // 服务端按 numeric 解析后与列值不相等（WHERE $1 命中 0 行），
        // 写入无限精度 numeric 列时更是静默存进错值。
        // nan/inf/-inf 服务端 numeric_in 与 float8in 都大小写无关接受。
        auto res = std::to_chars(buf, buf + sizeof(buf), v.dval, std::chars_format::general);
        if (res.ec == std::errc())
            return std::string(buf, static_cast<size_t>(res.ptr - buf));
        snprintf(buf, sizeof(buf), "%.17g", v.dval);// 兜底（几乎不会走到）
        return buf;
    }
    case http::obj_type::BOOL: return v.lval ? "t" : "f";
    case http::obj_type::STRING: return v.str_;
    default: return v.str_;
    }
}

// 写一个完整的 PG 消息到 send_data
// 格式: type(1) + length(4, BE) + payload
void _write_pg_msg(std::string &out, char type, const std::string &payload)
{
    out.push_back(type);
    char len_buf[4];
    int32_to_buf(static_cast<int32_t>(payload.size() + 4), reinterpret_cast<unsigned char *>(len_buf));
    out.append(len_buf, 4);
    out.append(payload);
}

// 追加 32-bit BE 到 string
void _append_i32(std::string &out, int32_t v)
{
    char buf[4];
    int32_to_buf(v, reinterpret_cast<unsigned char *>(buf));
    out.append(buf, 4);
}

// 追加 16-bit BE 到 string
void _append_i16(std::string &out, int16_t v)
{
    unsigned char buf[2];
    buf[0] = static_cast<unsigned char>((v >> 8) & 0xFF);
    buf[1] = static_cast<unsigned char>(v & 0xFF);
    out.append(reinterpret_cast<char *>(buf), 2);
}

// 追加 null-terminated string
void _append_str(std::string &out, const std::string &s)
{
    out.append(s);
    out.push_back('\0');
}

// 发送完整 buffer 到 socket（复用 write_sql 的 asio::write 模式）
bool _pg_write_buf(pg_conn_base *self, const std::string &buf)
{
    if (self->isclose)
        return false;
    try
    {
        if (self->conn_link->sock_type == 1)
        {
            asio::write(*self->conn_link->socket, asio::buffer(buf), self->conn_link->ec);
        }
        else if (self->conn_link->sock_type == 2)
        {
            asio::write(*self->conn_link->sslsocket, asio::buffer(buf), self->conn_link->ec);
        }
        else if (self->conn_link->sock_type == 3)
        {
            asio::write(*self->conn_link->localsocket, asio::buffer(buf), self->conn_link->ec);
        }
        if (self->conn_link->ec)
        {
            self->error_code = 20;
            self->error_msg  = self->conn_link->ec.message();
            return false;
        }
    }
    catch (const std::exception &e)
    {
        self->error_code = 20;
        self->error_msg  = e.what();
        return false;
    }
    self->query_num++;
    return true;
}

// Extended Query / prepared statement 异步写辅助
asio::awaitable<bool> _pg_async_write_buf(pg_conn_base *self, const std::string &buf)
{
    if (self->isclose)
        co_return false;
    try
    {
        asio::error_code ec;
        size_t n = 0;
        if (self->conn_link->sock_type == 1)
        {
            std::tie(ec, n) = co_await asio::async_write(*self->conn_link->socket, asio::buffer(buf), asio::as_tuple(asio::use_awaitable));
        }
        else if (self->conn_link->sock_type == 2)
        {
            std::tie(ec, n) = co_await asio::async_write(*self->conn_link->sslsocket, asio::buffer(buf), asio::as_tuple(asio::use_awaitable));
        }
        else if (self->conn_link->sock_type == 3)
        {
            std::tie(ec, n) = co_await asio::async_write(*self->conn_link->localsocket, asio::buffer(buf), asio::as_tuple(asio::use_awaitable));
        }
        if (ec)
        {
            self->error_code = 20;
            self->error_msg  = ec.message();
            co_return false;
        }
    }
    catch (const std::exception &e)
    {
        self->error_code = 20;
        self->error_msg  = e.what();
        co_return false;
    }
    self->query_num++;
    co_return true;
}

// 终版重构：命名 statement 消息构建 + SQLSTATE 辅助
// 热路径: Bind + Execute + Sync（跳过 Parse + 跳过 Describe —— 对齐 MySQL COM_STMT_EXECUTE）
// 冷路径: [Close] + Parse(named) + Bind + Describe + Execute + Sync
// ═══════════════════════════════════════════════════════════════════════

static void _append_bind_named(std::string &out,
                               const std::string &stmt_name,
                               const std::vector<http::obj_val> &params,
                               const std::vector<uint16_t> &req_fmts)
{
    std::string p;
    _append_str(p, "");
    _append_str(p, stmt_name);
    _append_i16(p, 0);
    _append_i16(p, (int16_t)params.size());
    for (size_t i = 0; i < params.size(); ++i)
    {
        if (params[i].get_type() == http::obj_type::NIL)
        {
            _append_i32(p, -1);
            continue;
        }
        std::string text = _obj_to_text(params[i]);
        _append_i32(p, (int32_t)text.size());
        p.append(text);
    }
    if (req_fmts.empty())
    {
        _append_i16(p, 0);
    }
    else
    {
        _append_i16(p, (int16_t)req_fmts.size());
        for (uint16_t f : req_fmts)
            _append_i16(p, (int16_t)f);
    }
    _write_pg_msg(out, 'B', p);
}

static void _append_describe_portal(std::string &out)
{
    std::string p;
    p.push_back('P');
    _append_str(p, "");
    _write_pg_msg(out, 'D', p);
}

static void _append_execute_sync(std::string &out)
{
    {
        std::string p;
        _append_str(p, "");
        _append_i32(p, 0);
        _write_pg_msg(out, 'E', p);
    }
    {
        std::string p;
        _write_pg_msg(out, 'S', p);
    }
}

static void _build_cold_exec(std::string &out,
                             const std::string &stmt_name,
                             const std::string &sql,
                             const std::vector<http::obj_val> &params,
                             const std::vector<uint16_t> &req_fmts,
                             const std::vector<std::string> *close_old_stmts)
{
    if (close_old_stmts)
    {
        for (const auto &n : *close_old_stmts)
        {
            if (n.empty())
                continue;
            std::string p;
            p.push_back('S');
            _append_str(p, n);
            _write_pg_msg(out, 'C', p);
        }
    }
    {
        std::string p;
        _append_str(p, stmt_name);
        _append_str(p, sql);
        _append_i16(p, 0);
        _write_pg_msg(out, 'P', p);
    }
    _append_bind_named(out, stmt_name, params, req_fmts);
    _append_describe_portal(out);
    _append_execute_sync(out);
}

static void _build_warm_exec(std::string &out,
                             const std::string &stmt_name,
                             const std::vector<http::obj_val> &params,
                             const std::vector<uint16_t> &req_fmts)
{
    _append_bind_named(out, stmt_name, params, req_fmts);
    _append_execute_sync(out);
}

static std::string _extract_sqlstate(const unsigned char *data, size_t len)
{
    size_t pos = 0;
    while (pos < len)
    {
        unsigned char ft = data[pos++];
        if (ft == '\0')
            break;
        size_t s_end = pos;
        while (s_end < len && data[s_end] != '\0')
            ++s_end;
        if (ft == 'C' && s_end - pos >= 5)
            return std::string((const char *)data + pos, 5);
        if (s_end >= len)
            break;
        pos = s_end + 1;
    }
    return std::string();
}

static bool _pg_sqlstate_needs_reparse(const std::string &s)
{
    if (s == "42P01" || s == "42703" || s == "42804" || s == "42601" || s == "42P07" || s == "0A000" ||
        s == "26000" /* INVALID_SQL_STATEMENT_NAME — 命名语句被服务器回收，必须重 Parse */)
        return true;
    return false;
}

static bool _pg_sqlstate_is_transient(const std::string &s)
{
    if (s == "08P01")
        return true;
    if (s.size() >= 2 && s[0] == '0' && s[1] == '8')
        return true;
    return false;
}

// ---- binary 白名单：解码器 fmt==1 分支真正覆盖的 OID ----
// 只有这些列向服务端要 binary；白名单外一律要 text：typout 是服务端权威输出，
// "我们没实现"不等于"错误数据"。13 个 OID 实测 typsend<>0（§17.1），
// 因此按此清单请求 binary 在结构上不可能触发 0A000。
// 新增解码分支时必须同步这里，判据见 §17.4 #6（双向对拍，防漏同步）。
static bool _pg_binary_decodable(uint32_t oid)
{
    switch (oid)
    {
    case 16:
    case 20:
    case 21:
    case 23:
    case 26:
    case 700:
    case 701:
    case 1700:
    case 1082:
    case 1083:
    case 1114:
    case 1184:
    case 1266:
        return true;
    default:
        return false;
    }
}

// ---- _parse_row_description: RowDescription → (列名, 类型 OID, 格式码) ----
// 每字段 = name(NUL 结尾) + 定长 18 字节。以列名的 NUL 位置为基准：
//   tableOid(+1..+4) attnum(+5..+6) typeOid(+7..+10) typlen(+11..+12)
//   atttypmod(+13..+16) format(+17..+18)
// 三个出参始终等长成组 push，字段不完整即整体停止，调用方按下标取用时必一致。
// 前提：format 字段只在 RowDescription 来自 Describe Portal('P') 时可信；
// Describe Statement('S') 服务端恒填 0。
static void _parse_row_description(const unsigned char *d, size_t n, std::vector<std::string> &names, std::vector<uint32_t> &oids, std::vector<uint16_t> &formats)
{
    names.clear();
    oids.clear();
    formats.clear();
    if (n < 2)
        return;
    int16_t nf = (int16_t)(((uint32_t)d[0] << 8) | d[1]);
    if (nf < 0)
        return;
    size_t pos = 2;
    for (int16_t i = 0; i < nf; ++i)
    {
        size_t name_end = pos;
        while (name_end < n && d[name_end] != 0)
            ++name_end;
        if (name_end >= n)
            return;// 列名未 NUL 终止 → 载荷损坏
        if (name_end + 1 + 18 > n)
            return;// 定长字段不完整
        names.push_back(std::string((const char *)(d + pos), name_end - pos));
        // name_end 指向列名的 NUL 本身，18 字节定长块从 name_end+1 起算
        oids.push_back((uint32_t)(((uint32_t)d[name_end + 7] << 24) | ((uint32_t)d[name_end + 8] << 16) | ((uint32_t)d[name_end + 9] << 8) | d[name_end + 10]));
        formats.push_back((uint16_t)(((uint32_t)d[name_end + 17] << 8) | d[name_end + 18]));
        pos = name_end + 1 + 18;
    }
}

// ---- 整数日历（替掉解码热路径上的 gmtime_r / strftime）----
// floor 除法：C++ 的 / 向零取整，1970 前的时间戳必须向下取整才能正确跨日
static int64_t _pg_floor_div(int64_t a, int64_t b)
{
    int64_t q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0)))
        --q;
    return q;
}

// days-since-1970 → 年/月/日（proleptic Gregorian，天文纪年含 year 0）。对负天数成立，
// 且不存在"失败"分支，而 gmtime_r 越界会返回 NULL、调用方会读到未初始化 tm。
static void _pg_days_to_civil(int64_t z, int &year, int &month, int &day)
{
    z += 719468;// 从 0000-03-01 起算的偏移
    const int64_t era  = (z >= 0 ? z : z - 146096) / 146097;
    const uint64_t doe = (uint64_t)(z - era * 146097);                         // [0, 146096]
    uint64_t yoe       = (doe - doe / 1461 + doe / 36524 - doe / 146096) / 365;// [0, 400]
    // 闭式解只在两处偏大：era 末（doe=146096 解出 400）与 366 天 March-年的最后一天
    // （闰日补在 yoe+1 上，闭式解提前一年）。各修正一次即回到 [0,399]，doy 才不会下溢回绕。
    if (yoe > 399)
        yoe = 399;
    if (365 * yoe + yoe / 4 - yoe / 100 > doe)
        --yoe;
    const int64_t yy   = (int64_t)yoe + era * 400;
    const uint64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);// [0, 365]
    const int64_t mp   = (int64_t)((5 * doy + 2) / 153);         // [0, 11]，3 月起算
    day                = (int)(doy - (153 * mp + 2) / 5 + 1);    // [1, 31]
    month              = (int)(mp + (mp < 10 ? 3 : -9));         // [1, 12]
    year               = (int)(yy + (month <= 2));
}

// ---- _decode_pg_binary_row: binary 格式 DataRow → col_value_variant ----
// payload 用 string_view：只读取、不持有，调用方的缓冲在本函数返回前始终有效。
static bool _decode_pg_binary_row(
    std::string_view payload,
    const std::vector<uint32_t> &col_oids,
    const std::vector<uint16_t> &col_formats,
    std::vector<orm::col_value_variant> &out_variants,
    std::vector<std::string> &date_backing)
{
    out_variants.clear();
    if (payload.size() < 2)
        return false;
    const unsigned char *d = (const unsigned char *)payload.data();
    size_t total           = payload.size();
    int16_t cc             = (int16_t)((d[0] << 8) | d[1]);
    // 与 libpq 同条判定（fe-protocol3.c getAnotherTuple: tupnfields != nfields ⇒ "unexpected
    // field count in \"D\" message"）：'D' 少列时原样按 cc 循环会静默给出短行，业务侧读到 monostate。
    if (cc < 0 || (size_t)cc != col_oids.size())
        return false;
    size_t pos = 2;
    out_variants.reserve(cc);
    auto be_i32 = [&](size_t p)
    {
        return (int32_t)((d[p] << 24) | (d[p + 1] << 16) | (d[p + 2] << 8) | d[p + 3]);
    };
    auto be_i64 = [&](size_t p)
    {
        uint64_t u = 0;
        for (int i = 0; i < 8; ++i)
            u = (u << 8) | d[p + i];
        return (int64_t)u;
    };
    auto be_u32 = [&](size_t p)
    {
        return (uint32_t)((d[p] << 24) | (d[p + 1] << 16) | (d[p + 2] << 8) | d[p + 3]);
    };
    auto be_u64 = [&](size_t p)
    {
        uint64_t u = 0;
        for (int i = 0; i < 8; ++i)
            u = (u << 8) | d[p + i];
        return u;
    };

    for (int16_t i = 0; i < cc; ++i)
    {
        if (pos + 4 > total)
            return false;
        int32_t vlen = be_i32(pos);
        pos += 4;
        if (vlen == -1)
        {
            out_variants.push_back(orm::col_value_variant{std::monostate{}});
            continue;
        }
        // 无符号减法形式：原 `pos + vlen > total` 三者皆 int，损坏流下 vlen 可大到
        // 让加法溢出为负从而绕过守卫，default 分支会按 2GB 长度构造 string
        if (vlen < 0 || (size_t)vlen > total - pos)
            return false;
        const unsigned char *p = d + pos;
        uint32_t oid           = (i < (int16_t)col_oids.size()) ? col_oids[(size_t)i] : 0;

        // format==0 ⇒ 该列服务端发的是文本输出（typout），与文本协议逐字节一致，
        // 原样返回即为权威值；binary 解码器只服务 format==1 的列。
        uint16_t fmt = (i < (int16_t)col_formats.size()) ? col_formats[(size_t)i] : 1;
        if (fmt == 0)
        {
            date_backing.emplace_back((const char *)p, (size_t)vlen);
            out_variants.push_back(orm::col_value_variant{std::string_view{date_backing.back()}});
            pos += (size_t)vlen;
            continue;
        }

        switch (oid)
        {
        case 16:
        {// bool
            if (vlen != 1)
                return false;
            out_variants.push_back(orm::col_value_variant{(int64_t)(p[0] ? 1 : 0)});
            break;
        }
        case 21:
        {// int2
            if (vlen != 2)
                return false;
            out_variants.push_back(orm::col_value_variant{(int64_t)(int16_t)((p[0] << 8) | p[1])});
            break;
        }
        case 23:
        {// int4
            if (vlen != 4)
                return false;
            out_variants.push_back(orm::col_value_variant{(int64_t)be_i32(pos)});
            break;
        }
        case 20:
        {// int8
            if (vlen != 8)
                return false;
            out_variants.push_back(orm::col_value_variant{be_i64(pos)});
            break;
        }
        case 26:
        {// oid (uint32)
            if (vlen != 4)
                return false;
            out_variants.push_back(orm::col_value_variant{(uint64_t)be_u32(pos)});
            break;
        }
        case 700:
        {// float4
            if (vlen != 4)
                return false;
            uint32_t u32 = be_u32(pos);
            float f;
            memcpy(&f, &u32, 4);
            out_variants.push_back(orm::col_value_variant{(double)f});
            break;
        }
        case 701:
        {// float8
            if (vlen != 8)
                return false;
            uint64_t u64 = be_u64(pos);
            double dd;
            memcpy(&dd, &u64, 8);
            out_variants.push_back(orm::col_value_variant{dd});
            break;
        }
        case 1700:
        {// numeric — 正确的 canonical decode(对齐 test_pg_numeric.cpp)

            if (vlen < 8)
                return false;// 头部 8 字节本身就必须完整
            size_t bp       = pos;
            int16_t ndigits = (int16_t)((d[bp] << 8) | d[bp + 1]);
            int16_t weight  = (int16_t)((d[bp + 2] << 8) | d[bp + 3]);
            uint16_t sign   = (uint16_t)((d[bp + 4] << 8) | d[bp + 5]);
            int16_t dscale  = (int16_t)((d[bp + 6] << 8) | d[bp + 7]);

            if (sign == 0xC000)
            {
                out_variants.push_back(orm::col_value_variant{std::string_view{"NaN"}});
                break;
            }
            if (sign == 0xD000)
            {
                out_variants.push_back(orm::col_value_variant{std::string_view{"Infinity"}});
                break;
            }
            if (sign == 0xF000)
            {
                out_variants.push_back(orm::col_value_variant{std::string_view{"-Infinity"}});
                break;
            }
            if (ndigits < 0 || 8 + ndigits * 2 > vlen)
            {
                out_variants.push_back(orm::col_value_variant{std::string_view{}});
                break;
            }

            // W3-2：不再物化 digit 数组。dg[g] 的所有用法都是前向单次索引（下面
            // 全零扫描、跳前导零、整数段、小数段），直接从载荷取 BE uint16；
            // g < ndigits 的越界可能已由 8 + ndigits*2 > vlen 挡掉。
            auto dig = [&](int g) -> uint16_t
            {
                return (uint16_t)((d[bp + 8 + g * 2] << 8) | d[bp + 9 + g * 2]);
            };

            // 全零
            bool is_zero = true;
            for (int16_t g = 0; g < ndigits; ++g)
                if (dig(g))
                {
                    is_zero = false;
                    break;
                }

            // canonicalize: 跳过前导零组
            int g_start = 0;
            int16_t wt  = weight;
            while (g_start < ndigits && dig(g_start) == 0)
            {
                g_start++;
                wt--;
            }// 跳过前导零组：最高有效组权重减一（PG-8 修正，原 wt++ 方向反）

            // W3-2：长度先算准（符号 + 整数组首位实际位数 + 其余每组 4 位 + 小数点 +
            // dscale），再 emplace 出 backing 元素。算准是关键：用 ndigits*4 的松上界
            // 会把 numeric(18,4) 的 "1234.5678" 也顶出 SSO 容量，每行白分配一次。
            int int_digits = 1;
            if (!is_zero && wt >= 0)
            {
                uint16_t v0 = dig(g_start);
                int_digits  = (v0 >= 1000 ? 4 : v0 >= 100 ? 3 :
                                            v0 >= 10      ? 2 :
                                                            1) +
                             4 * wt;
            }
            size_t len = (size_t)(is_zero ? 1 : int_digits) +
                         (dscale > 0 ? 1u + (size_t)dscale : 0u) +
                         (sign == 0x4000 ? 1u : 0u);
            date_backing.emplace_back();
            std::string &out = date_backing.back();
            if (len > out.capacity())
                out.reserve(len);// ≤22 位走 SSO，一次分配都不做
            if (sign == 0x4000)
                out += '-';

            if (is_zero)
            {
                out += "0";
                if (dscale > 0)
                {
                    out += ".";
                    out.append(dscale, '0');
                }
                out_variants.push_back(orm::col_value_variant{std::string_view{out}});
                break;
            }

            if (wt >= 0)
            {
                int int_groups = wt + 1;
                for (int gi = 0; gi < int_groups; ++gi)
                {
                    uint16_t v = (g_start + gi < ndigits) ? dig(g_start + gi) : 0;
                    char buf[8];
                    if (gi == 0)
                        std::snprintf(buf, sizeof(buf), "%u", v);
                    else
                        std::snprintf(buf, sizeof(buf), "%04u", v);
                    out += buf;
                }
                g_start += int_groups;
            }
            else
            {
                out += "0";
            }

            if (dscale > 0)
            {
                out += ".";
                int lead_zero_groups = (wt < 0) ? (-wt - 1) : 0;
                int frac_written     = 0;
                while (lead_zero_groups > 0 && frac_written < dscale)
                {
                    int chunk = std::min(4, (int)dscale - frac_written);
                    out.append(chunk, '0');
                    frac_written += chunk;
                    lead_zero_groups--;
                }
                while (g_start < ndigits && frac_written < dscale)
                {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "%04u", dig(g_start));
                    for (int k = 0; k < 4 && frac_written < dscale; ++k)
                    {
                        out += buf[k];
                        frac_written++;
                    }
                    g_start++;
                }
                out.append(dscale - frac_written, '0');
            }
            out_variants.push_back(orm::col_value_variant{std::string_view{out}});
            break;
        }
        case 1082:
        {// date — days since 2000-01-01 (PG epoch)
            if (vlen != 4)
                return false;
            int32_t days = be_i32(pos);
            // date 的 ±infinity 在 binary 里是 INT32_MIN/INT32_MAX 哨兵，
            // 参与天数换算会得出 -5881611-06-22 这类垃圾值
            if (days == 2147483647 || days == -2147483648)
            {
                date_backing.push_back(days == 2147483647 ? "infinity" : "-infinity");
                out_variants.push_back(orm::col_value_variant{std::string_view{date_backing.back()}});
                break;
            }
            // Unix epoch 1970-01-01, diff from PG epoch 2000-01-01 = 10957 days
            // W3-3：整数日历直出，省掉 *86400 + gmtime_r + strftime 三步
            int yy, mm, dd;
            _pg_days_to_civil((int64_t)days + 10957, yy, mm, dd);
            char buf[32];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d", yy, mm, dd);
            date_backing.push_back(buf);
            out_variants.push_back(orm::col_value_variant{std::string_view{date_backing.back()}});
            break;
        }
        case 1083:// time(no tz)
        case 1266:
        {// timetz
            if (vlen != (oid == 1266 ? 12 : 8))
                return false;
            int64_t usec      = be_i64(pos);
            int64_t total_sec = usec / 1000000;
            int h             = (int)(total_sec / 3600);
            int m             = (int)((total_sec % 3600) / 60);
            int s             = (int)(total_sec % 60);
            int frac          = (int)(usec % 1000000);
            char buf[32];
            if (frac > 0)
                snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%06d", h, m, s, frac);
            else
                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
            date_backing.push_back(buf);
            out_variants.push_back(orm::col_value_variant{std::string_view{date_backing.back()}});
            break;
        }
        case 1114:// timestamp(no tz)
        case 1184:
        {// timestamptz
            if (vlen != 8)
                return false;
            int64_t usec = be_i64(pos);
            // ±infinity 是 INT64_MIN/MAX 哨兵，直接参与下面的 µs 加法是有符号
            // 溢出 UB，实测得出 -290278-12-22 19:59:05 这类垃圾值
            if (usec == 9223372036854775807LL || usec == (int64_t)9223372036854775808ULL)
            {
                date_backing.push_back(usec == 9223372036854775807LL ? "infinity" : "-infinity");
                out_variants.push_back(orm::col_value_variant{std::string_view{date_backing.back()}});
                break;
            }
            // PG epoch: 2000-01-01 00:00:00 UTC
            // Unix epoch: 1970-01-01 00:00:00 UTC
            // Difference: 10957 days * 86400 sec = 946684800 sec
            const int64_t pg_to_unix_sec = 946684800LL;
            // floor division: C++ / 和 % 向零取整,负值需修正
            int64_t tot = usec / 1000000;
            int64_t mic = usec % 1000000;
            if (mic < 0)
            {
                tot -= 1;
                mic += 1000000;
            }
            tot += pg_to_unix_sec;// 偏移在秒上加：µs 级加法在 |usec|≈INT64_MAX 时溢出
            int frac = (int)mic;
            // W3-3：整数日历 + floor 除法，替掉 gmtime_r（它要做时区表与年代循环）
            const int64_t days = _pg_floor_div(tot, 86400);
            const int64_t sod  = tot - days * 86400;// [0, 86399]
            int yy, mm, dd;
            _pg_days_to_civil(days, yy, mm, dd);
            char buf[48];
            if (frac > 0)
                snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%06d", yy, mm, dd, (int)(sod / 3600), (int)((sod % 3600) / 60), (int)(sod % 60), frac);
            else
                snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", yy, mm, dd, (int)(sod / 3600), (int)((sod % 3600) / 60), (int)(sod % 60));
            date_backing.push_back(buf);
            out_variants.push_back(orm::col_value_variant{std::string_view{date_backing.back()}});
            break;
        }
        default:
            // 走到这里 = 服务端对「白名单外」的 OID 发了 binary，唯一现实成因是缓存
            // OID 与本轮真实类型不一致（两轮之间 ALTER COLUMN TYPE）。原样拷贝会把
            // 二进制塞进业务字符串且无任何错误标记（P0-D），因此按失败返回：
            // 调用方丢该 SQL 缓存项并记错误，冷路径全文本自愈。
            // 未实现类型的对外行为由「向服务端要 text」提供（_pg_binary_decodable）。
            return false;
        }
        pos += (size_t)vlen;
    }
    return true;
}

}// anonymous namespace

// ===== PG Extended Query SELECT BINARY =====
unsigned int pg_conn_base::fetch_prepared_impl(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    unsigned int reconnect_attempt = 0;
RETRY_RECONNECT:
    this->clear_error();
    unsigned int rows                = 0;
    constexpr size_t k_max_rows_text = 16u * 1024u * 1024u;
    std::vector<std::string> col_names;
    std::vector<char *> name_ptrs_cache;

    pg_stmt_meta_t dropped;// PG-5: warm 轮摘掉的旧语句名，随下一轮冷包一起 DEALLOCATE
    for (unsigned int warm_attempt = 0; warm_attempt < 2; ++warm_attempt)
    {
        error_msg.clear();
        error_code = 0;
        col_names.clear();
        name_ptrs_cache.clear();

        const pg_stmt_meta_t *m = (warm_attempt == 0) ? prepared_meta_.find(sql) : nullptr;
        const bool warm         = (m != nullptr && !m->needs_reparse);
        std::string send_buf;

        if (warm)
        {
            _build_warm_exec(send_buf, m->stmt_name, params, {});
            col_names = m->col_names;
            col_names.reserve(col_names.size());
            name_ptrs_cache.reserve(col_names.size());
            for (auto &n : col_names)
                name_ptrs_cache.push_back(n.data());
        }
        else
        {
            pg_stmt_meta_t needs_close;
            const pg_stmt_meta_t *old = (warm_attempt == 0) ? m : nullptr;
            if (old && old->needs_reparse)
            {
                prepared_meta_.erase(sql, &needs_close);
            }

            pg_stmt_meta_t evicted;
            std::string stmt_name = prepared_meta_.next_stmt_name();
            auto &slot            = prepared_meta_.upsert(sql, &evicted);
            slot.stmt_name        = stmt_name;
            slot.needs_reparse    = false;

            std::vector<std::string> close_targets;
            if (!needs_close.stmt_name.empty())
                close_targets.push_back(needs_close.stmt_name);
            if (!evicted.stmt_name.empty())
                close_targets.push_back(evicted.stmt_name);
            if (!dropped.stmt_name.empty())
            {
                close_targets.push_back(std::move(dropped.stmt_name));
                dropped.stmt_name.clear();
            }

            _build_cold_exec(send_buf, stmt_name, sql, params, {}, close_targets.empty() ? nullptr : &close_targets);
        }

        if (!_pg_write_buf(this, send_buf))
        {
            error_msg  = "Extended Query write failed";
            error_code = 20;
            if (warm)
                prepared_meta_.erase(sql);
            if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
            {
                reconnect_attempt++;
                goto RETRY_RECONNECT;
            }
            isclose = true;
            return rows;
        }

        rows           = 0;
        bool abandon   = false;
        bool got_ready = false;

        pooled_accum_buf accum_holder;
        std::vector<uint8_t> &accum_buf = accum_holder.buf;
        _pg_frame_state fs;

        while (!got_ready)
        {
            unsigned char type = 0;
            std::string_view payload;
            {
                const unsigned char *payload_start = nullptr;
                unsigned int payload_len           = 0;
                type                               = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
                if (type != 0)
                    payload = std::string_view(reinterpret_cast<const char *>(payload_start), payload_len);
            }
            if (type == 0)
            {
                unsigned int n = read_loop();
                if (n == 0)
                {
                    error_msg  = "read failed";
                    error_code = 21;
                    // 对端被杀/断连 → 读立即失败(CLOSE_WAIT 假写成功). SELECT 幂等, 可安全重连重发
                    if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
                    {
                        reconnect_attempt++;
                        goto RETRY_RECONNECT;
                    }
                    isclose = true;
                    break;
                }
                if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_max_rows_text, 0))
                {
                    error_msg  = "fetch_prepared: response exceeds limit";
                    error_code = 8;
                    isclose    = true;
                    break;
                }
                continue;
            }
            if (abandon)
            {
                if (type == 'Z')
                    got_ready = true;
                continue;
            }
            switch (type)
            {
            case '1': break;
            case '2': break;
            case 'n': break;
            case 'C': break;

            case 'T':
            {
                // RowDescription: 前 2 字节 = int16 字段数（大端）
                if (payload.size() < 2)
                {
                    abandon = true;
                    break;
                }
                int16_t fc = static_cast<int16_t>(
                    (static_cast<unsigned char>(payload[0]) << 8) | static_cast<unsigned char>(payload[1]));
                size_t pos = 2;
                col_names.clear();
                for (int16_t i = 0; i < fc; ++i)
                {
                    if (pos >= payload.size())
                    {
                        abandon = true;
                        col_names.clear();
                        break;
                    }
                    size_t start = pos;
                    while (pos < payload.size() && payload[pos] != 0)
                        pos++;
                    col_names.emplace_back(payload.substr(start, pos - start));
                    pos++;
                    if (pos + 18 > payload.size())
                    {
                        abandon = true;
                        col_names.clear();
                        break;
                    }
                    pos += 18;
                }
                if (!abandon)
                {
                    auto &slot         = prepared_meta_.upsert(sql);
                    slot.col_names     = col_names;
                    slot.needs_reparse = false;
                    name_ptrs_cache.clear();
                    for (auto &n : col_names)
                        name_ptrs_cache.push_back(n.data());
                }
                break;
            }

            case 'D':
            {
                if (col_names.empty())
                    break;
                if (payload.size() < 2)
                {
                    abandon = true;
                    break;
                }
                size_t pos        = 0;
                int16_t col_count = static_cast<int16_t>(
                    (static_cast<unsigned char>(payload[0]) << 8) | static_cast<unsigned char>(payload[1]));
                pos += 2;

                std::vector<std::string> row_values;
                std::vector<char> row_is_null;
                row_values.reserve(col_count > 0 ? static_cast<size_t>(col_count) : 0);
                row_is_null.reserve(col_count > 0 ? static_cast<size_t>(col_count) : 0);
                for (int16_t i = 0; i < col_count; ++i)
                {
                    if (pos + 4 > payload.size())
                    {
                        abandon = true;
                        break;
                    }
                    int32_t vlen = buf_to_int32(reinterpret_cast<const unsigned char *>(payload.data() + pos));
                    pos += 4;
                    if (vlen == -1)
                    {
                        row_values.emplace_back();
                        row_is_null.push_back(1);
                    }
                    else if (vlen >= 0 && pos + static_cast<size_t>(vlen) <= payload.size())
                    {
                        row_values.emplace_back(payload.substr(pos, vlen));
                        row_is_null.push_back(0);
                        pos += vlen;
                    }
                    else
                    {
                        abandon = true;
                        break;
                    }
                }
                if (abandon)
                    break;

                if (!warm)
                {
                    name_ptrs_cache.clear();
                    for (auto &n : col_names)
                        name_ptrs_cache.push_back(n.data());
                }

                auto captured      = std::move(row_values);
                auto captured_null = std::move(row_is_null);
                bool cont          = handler(
                    static_cast<int>(col_count),
                    name_ptrs_cache.data(),
                    [captured, captured_null](int idx) -> std::tuple<unsigned char *, size_t>
                    {
                        if (idx < 0 || idx >= static_cast<int>(captured.size()))
                            return {nullptr, 0};
                        if (captured_null[static_cast<size_t>(idx)])
                            return {nullptr, 0};
                        const auto &s = captured[idx];
                        return {reinterpret_cast<unsigned char *>(const_cast<char *>(s.data())), s.size()};
                    });
                rows++;
                if (!cont)
                    abandon = true;
                break;
            }

            case 'Z':
            {
                if (payload.size() >= 1)
                    in_transaction_.store(payload[0] == 'T' || payload[0] == 'E');
                got_ready = true;
                break;
            }
            case 'E':
            {
                std::string err_msg;
                const unsigned char *pd = (const unsigned char *)payload.data();
                size_t pp               = 0;
                while (pp < payload.size())
                {
                    unsigned char ft = pd[pp++];
                    if (ft == 0)
                        break;
                    size_t start = pp;
                    while (pp < payload.size() && pd[pp] != 0)
                        ++pp;
                    if (ft == 'M')
                        err_msg.assign((const char *)(pd + start), pp - start);
                    if (pp >= payload.size())
                        break;
                    ++pp;
                }
                error_msg  = err_msg.empty() ? "PG error response" : err_msg;
                error_code = 22;

                std::string sqlstate = _extract_sqlstate(pd, payload.size());
                if (_pg_sqlstate_needs_reparse(sqlstate))
                    prepared_meta_.mark_needs_reparse(sql);
                else if (_pg_sqlstate_is_transient(sqlstate))
                    prepared_meta_.erase(sql, &dropped);
                abandon = true;
                break;
            }
            case 'G':
            {
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                got_ready  = true;
                break;
            }
            case 'I':
            case 's':
            case 'S':
            case 'N':
            case 'A':
            default: break;
            }
        }

        const bool skip_close = warm && got_ready && !abandon && !isclose && error_code == 0;
        if (!isclose && !skip_close)
        {
            std::string close_buf;
            {
                std::string p;
                p.push_back('P');
                _append_str(p, "");
                _write_pg_msg(close_buf, 'C', p);
            }
            {
                std::string p;
                _write_pg_msg(close_buf, 'S', p);
            }
            _pg_write_buf(this, close_buf);
            drain_until_ready_sync(accum_buf, fs.consumed);
        }

        const bool need_cold =
            warm && !isclose && error_code != 21 &&
            error_code != 0;
        if (!need_cold)
            break;
        prepared_meta_.erase(sql, &dropped);
    }
    return rows;
}

// ===== PG Extended Query DML =====
unsigned int pg_conn_base::exec_dml_prepared_impl(
    const std::string &sql,
    const std::vector<http::obj_val> &params)
{
    unsigned int reconnect_attempt = 0;
RETRY_RECONNECT:
    this->clear_error();
    unsigned int affected = (unsigned int)-1;

    pg_stmt_meta_t dropped;// PG-5: warm 轮摘掉的旧语句名，随下一轮冷包一起 DEALLOCATE
    for (unsigned int warm_attempt = 0; warm_attempt < 2; ++warm_attempt)
    {
        error_msg.clear();
        error_code = 0;

        const pg_stmt_meta_t *m = (warm_attempt == 0) ? prepared_meta_.find(sql) : nullptr;
        const bool warm         = (m != nullptr && !m->needs_reparse);
        std::string send_buf;

        if (warm)
        {
            _build_warm_exec(send_buf, m->stmt_name, params, {});
        }
        else
        {
            pg_stmt_meta_t needs_close;
            const pg_stmt_meta_t *old = (warm_attempt == 0) ? m : nullptr;
            if (old && old->needs_reparse)
            {
                prepared_meta_.erase(sql, &needs_close);
            }

            pg_stmt_meta_t evicted;
            std::string stmt_name = prepared_meta_.next_stmt_name();
            auto &slot            = prepared_meta_.upsert(sql, &evicted);
            slot.stmt_name        = stmt_name;
            slot.needs_reparse    = false;

            std::vector<std::string> close_targets;
            if (!needs_close.stmt_name.empty())
                close_targets.push_back(needs_close.stmt_name);
            if (!evicted.stmt_name.empty())
                close_targets.push_back(evicted.stmt_name);
            if (!dropped.stmt_name.empty())
            {
                close_targets.push_back(std::move(dropped.stmt_name));
                dropped.stmt_name.clear();
            }

            _build_cold_exec(send_buf, stmt_name, sql, params, {}, close_targets.empty() ? nullptr : &close_targets);
        }

        if (!_pg_write_buf(this, send_buf))
        {
            error_msg  = "Extended Query write failed";
            error_code = 20;
            if (warm)
                prepared_meta_.erase(sql);
            if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
            {
                reconnect_attempt++;
                goto RETRY_RECONNECT;
            }
            isclose = true;
            return (unsigned int)-1;
        }

        bool abandon   = false;
        bool got_ready = false;

        pooled_accum_buf accum_holder;
        std::vector<uint8_t> &accum_buf = accum_holder.buf;
        _pg_frame_state fs;

        while (!got_ready)
        {
            unsigned char type = 0;
            std::string_view payload;
            {
                const unsigned char *payload_start = nullptr;
                unsigned int payload_len           = 0;
                type                               = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
                if (type != 0)
                    payload = std::string_view(reinterpret_cast<const char *>(payload_start), payload_len);
            }
            if (type == 0)
            {
                unsigned int n = read_loop();
                if (n == 0)
                {
                    error_msg  = "read failed";
                    error_code = 21;
                    // 对端被杀/断连 → 读立即失败. 自动提交下 PG 断连回滚隐式事务, DML 未落库, 可安全重连重发
                    if (!in_transaction_.load() && reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
                    {
                        reconnect_attempt++;
                        goto RETRY_RECONNECT;
                    }
                    isclose = true;
                    break;
                }
                if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_pg_no_byte_cap, k_pg_compact_at))
                {
                    error_msg  = "exec_dml: response exceeds buffer limit";
                    error_code = 8;
                    isclose    = true;
                    break;
                }
                continue;
            }
            if (abandon)
            {
                if (type == 'Z')
                    got_ready = true;
                continue;
            }
            switch (type)
            {
            case '1': break;// ParseComplete
            case '2': break;// BindComplete
            case 'n': break;// NoData（Describe DML 的响应）
            case 'C':
            {
                // "UPDATE 123" / "DELETE 456" / "INSERT 0 789"
                size_t last_space = payload.find_last_of(' ');
                if (last_space != std::string::npos)
                {
                    std::string num_str(payload.substr(last_space + 1));
                    try
                    {
                        affected = std::stoul(num_str);
                    }
                    catch (...)
                    {
                    }
                }
                break;
            }
            case 'Z':
            {
                if (payload.size() >= 1)
                    in_transaction_.store(payload[0] == 'T' || payload[0] == 'E');
                got_ready = true;
                break;
            }
            case 'E':
            {
                std::string err_msg;
                const unsigned char *pd = (const unsigned char *)payload.data();
                size_t pp               = 0;
                while (pp < payload.size())
                {
                    unsigned char ft = pd[pp++];
                    if (ft == 0)
                        break;
                    size_t start = pp;
                    while (pp < payload.size() && pd[pp] != 0)
                        ++pp;
                    if (ft == 'M')
                        err_msg.assign((const char *)(pd + start), pp - start);
                    if (pp >= payload.size())
                        break;
                    ++pp;
                }
                error_msg  = err_msg.empty() ? "PG error response" : err_msg;
                error_code = 22;

                std::string sqlstate = _extract_sqlstate(pd, payload.size());
                if (_pg_sqlstate_needs_reparse(sqlstate))
                    prepared_meta_.mark_needs_reparse(sql);
                else if (_pg_sqlstate_is_transient(sqlstate))
                    prepared_meta_.erase(sql, &dropped);
                break;
            }
            case 'G':
            {
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                got_ready  = true;
                break;
            }
            case 'S':
            case 'N':
            case 'A':
            default: break;
            }
        }

        const bool skip_close = warm && got_ready && !abandon && !isclose && error_code == 0;
        if (!isclose && !skip_close)
        {
            std::string close_buf;
            {
                std::string p;
                p.push_back('P');
                _append_str(p, "");
                _write_pg_msg(close_buf, 'C', p);
            }
            {
                std::string p;
                _write_pg_msg(close_buf, 'S', p);
            }
            _pg_write_buf(this, close_buf);
            drain_until_ready_sync(accum_buf, fs.consumed);
        }

        const bool need_cold =
            warm && !isclose && error_code != 21 &&
            error_code != 0;
        if (!need_cold)
            break;
        prepared_meta_.erase(sql, &dropped);
    }
    return affected;
}

asio::awaitable<unsigned int> pg_conn_base::async_exec_dml_prepared_impl(
    const std::string &sql,
    const std::vector<http::obj_val> &params)
{
    unsigned int reconnect_attempt = 0;
RETRY_RECONNECT:
    if (isclose)
        co_return (unsigned int) - 1;

    this->clear_error();
    unsigned int affected = (unsigned int)-1;

    pg_stmt_meta_t dropped;// PG-5: warm 轮摘掉的旧语句名，随下一轮冷包一起 DEALLOCATE
    for (unsigned int warm_attempt = 0; warm_attempt < 2; ++warm_attempt)
    {
        error_msg.clear();
        error_code = 0;

        const pg_stmt_meta_t *m = (warm_attempt == 0) ? prepared_meta_.find(sql) : nullptr;
        const bool warm         = (m != nullptr && !m->needs_reparse);
        std::string send_buf;

        if (warm)
        {
            _build_warm_exec(send_buf, m->stmt_name, params, {});
        }
        else
        {
            pg_stmt_meta_t needs_close;
            const pg_stmt_meta_t *old = (warm_attempt == 0) ? m : nullptr;
            if (old && old->needs_reparse)
            {
                prepared_meta_.erase(sql, &needs_close);
            }

            pg_stmt_meta_t evicted;
            std::string stmt_name = prepared_meta_.next_stmt_name();
            auto &slot            = prepared_meta_.upsert(sql, &evicted);
            slot.stmt_name        = stmt_name;
            slot.needs_reparse    = false;

            std::vector<std::string> close_targets;
            if (!needs_close.stmt_name.empty())
                close_targets.push_back(needs_close.stmt_name);
            if (!evicted.stmt_name.empty())
                close_targets.push_back(evicted.stmt_name);
            if (!dropped.stmt_name.empty())
            {
                close_targets.push_back(std::move(dropped.stmt_name));
                dropped.stmt_name.clear();
            }

            _build_cold_exec(send_buf, stmt_name, sql, params, {}, close_targets.empty() ? nullptr : &close_targets);
        }

        if (!co_await _pg_async_write_buf(this, send_buf))
        {
            error_msg  = "Extended Query async write failed";
            error_code = 20;
            if (warm)
                prepared_meta_.erase(sql);
            if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && co_await async_try_reconnect())
            {
                reconnect_attempt++;
                goto RETRY_RECONNECT;
            }
            isclose = true;
            co_return (unsigned int) - 1;
        }

        bool abandon   = false;
        bool got_ready = false;

        pooled_accum_buf accum_holder;
        std::vector<uint8_t> &accum_buf = accum_holder.buf;
        _pg_frame_state fs;

        while (!got_ready)
        {
            unsigned char type = 0;
            std::string_view payload;
            {
                const unsigned char *payload_start = nullptr;
                unsigned int payload_len           = 0;
                type                               = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
                if (type != 0)
                    payload = std::string_view(reinterpret_cast<const char *>(payload_start), payload_len);
            }
            if (type == 0)
            {
                unsigned int n = co_await async_read_loop();
                if (n == 0)
                {
                    error_msg  = "async read failed";
                    error_code = 21;
                    // 对端被杀/断连 → 读立即失败. 自动提交下 PG 断连回滚隐式事务, DML 未落库, 可安全重连重发
                    if (!in_transaction_.load() && reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && co_await async_try_reconnect())
                    {
                        reconnect_attempt++;
                        goto RETRY_RECONNECT;
                    }
                    isclose = true;
                    break;
                }
                if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_pg_no_byte_cap, k_pg_compact_at))
                {
                    error_msg  = "exec_dml_async: response exceeds buffer limit";
                    error_code = 8;
                    isclose    = true;
                    break;
                }
                continue;
            }
            if (abandon)
            {
                if (type == 'Z')
                    got_ready = true;
                continue;
            }
            switch (type)
            {
            case '1': break;// ParseComplete
            case '2': break;// BindComplete
            case 'n': break;// NoData
            case 'C':
            {
                size_t last_space = payload.find_last_of(' ');
                if (last_space != std::string::npos)
                {
                    std::string num_str(payload.substr(last_space + 1));
                    try
                    {
                        affected = std::stoul(num_str);
                    }
                    catch (...)
                    {
                    }
                }
                break;
            }
            case 'Z':
            {
                if (payload.size() >= 1)
                    in_transaction_.store(payload[0] == 'T' || payload[0] == 'E');
                got_ready = true;
                break;
            }
            case 'E':
            {
                std::string err_msg;
                const unsigned char *pd = (const unsigned char *)payload.data();
                size_t pp               = 0;
                while (pp < payload.size())
                {
                    unsigned char ft = pd[pp++];
                    if (ft == 0)
                        break;
                    size_t start = pp;
                    while (pp < payload.size() && pd[pp] != 0)
                        ++pp;
                    if (ft == 'M')
                        err_msg.assign((const char *)(pd + start), pp - start);
                    if (pp >= payload.size())
                        break;
                    ++pp;
                }
                error_msg  = err_msg.empty() ? "PG error response" : err_msg;
                error_code = 22;

                std::string sqlstate = _extract_sqlstate(pd, payload.size());
                if (_pg_sqlstate_needs_reparse(sqlstate))
                    prepared_meta_.mark_needs_reparse(sql);
                else if (_pg_sqlstate_is_transient(sqlstate))
                    prepared_meta_.erase(sql, &dropped);
                break;
            }
            case 'G':
            {
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                got_ready  = true;
                break;
            }
            case 'S':
            case 'N':
            case 'A':
            default: break;
            }
        }

        const bool skip_close = warm && got_ready && !abandon && !isclose && error_code == 0;
        if (!isclose && !skip_close)
        {
            std::string close_buf;
            {
                std::string p;
                p.push_back('P');
                _append_str(p, "");
                _write_pg_msg(close_buf, 'C', p);
            }
            {
                std::string p;
                _write_pg_msg(close_buf, 'S', p);
            }
            co_await _pg_async_write_buf(this, close_buf);
            co_await async_drain_until_ready(accum_buf, fs.consumed);
        }

        const bool need_cold =
            warm && !isclose && error_code != 21 && error_code != 0;
        if (!need_cold)
            break;
        prepared_meta_.erase(sql, &dropped);
    }
    co_return affected;
}

// 读掉 Extended Query 收尾 Close+Sync 的响应，直到本轮 ReadyForQuery。
// 这些字节留在 socket 里时，下一条语句会先读到残留的 'Z' 并误判"查询已结束"，直接返回 0 行。
void pg_conn_base::drain_until_ready_sync(std::vector<uint8_t> &accum_buf, unsigned int &consumed)
{
    for (unsigned int round = 0; round < 64; ++round)
    {
        while (consumed < accum_buf.size())
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = peek_pg_message(
                accum_buf.data(),
                static_cast<unsigned int>(accum_buf.size()),
                consumed,
                payload_start,
                payload_len);
            if (msg_type == 0)
                break;// 半包，等更多字节
            // peek_pg_message 内部已按整包推进 consumed

            if (msg_type == 'Z')
                return;
        }

        unsigned int n = read_loop();
        if (n == 0)
            return;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }
    return;
}

// async 孪生：同上，先消费缓冲里的残帧再 co_await async_read_loop
asio::awaitable<void> pg_conn_base::async_drain_until_ready(std::vector<uint8_t> &accum_buf, unsigned int &consumed)
{
    for (unsigned int round = 0; round < 64; ++round)
    {
        while (consumed < accum_buf.size())
        {
            const unsigned char *payload_start = nullptr;
            unsigned int payload_len           = 0;
            unsigned char msg_type             = peek_pg_message(
                accum_buf.data(),
                static_cast<unsigned int>(accum_buf.size()),
                consumed,
                payload_start,
                payload_len);
            if (msg_type == 0)
                break;// 半包，等更多字节
            // peek_pg_message 内部已按整包推进 consumed

            if (msg_type == 'Z')
                co_return;
        }

        unsigned int n = co_await async_read_loop();
        if (n == 0)
            co_return;
        accum_buf.insert(accum_buf.end(), _cache_data, _cache_data + n);
    }
    co_return;
}

// ===== PG Extended Query 异步版 =====
asio::awaitable<unsigned int> pg_conn_base::async_fetch_prepared_impl(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    unsigned int reconnect_attempt = 0;
RETRY_RECONNECT:
    if (isclose)
        co_return 0;

    this->clear_error();
    unsigned int rows = 0;
    std::vector<std::string> col_names;
    std::vector<char *> name_ptrs_cache;

    pg_stmt_meta_t dropped;// PG-5: warm 轮摘掉的旧语句名，随下一轮冷包一起 DEALLOCATE
    for (unsigned int warm_attempt = 0; warm_attempt < 2; ++warm_attempt)
    {
        error_msg.clear();
        error_code = 0;
        col_names.clear();
        name_ptrs_cache.clear();

        const pg_stmt_meta_t *m = (warm_attempt == 0) ? prepared_meta_.find(sql) : nullptr;
        const bool warm         = (m != nullptr && !m->needs_reparse);
        std::string send_buf;

        if (warm)
        {
            _build_warm_exec(send_buf, m->stmt_name, params, {});
            col_names = m->col_names;
            name_ptrs_cache.reserve(col_names.size());
            for (auto &n : col_names)
                name_ptrs_cache.push_back(n.data());
        }
        else
        {
            pg_stmt_meta_t needs_close;
            const pg_stmt_meta_t *old = (warm_attempt == 0) ? m : nullptr;
            if (old && old->needs_reparse)
            {
                prepared_meta_.erase(sql, &needs_close);
            }

            pg_stmt_meta_t evicted;
            std::string stmt_name = prepared_meta_.next_stmt_name();
            auto &slot            = prepared_meta_.upsert(sql, &evicted);
            slot.stmt_name        = stmt_name;
            slot.needs_reparse    = false;

            std::vector<std::string> close_targets;
            if (!needs_close.stmt_name.empty())
                close_targets.push_back(needs_close.stmt_name);
            if (!evicted.stmt_name.empty())
                close_targets.push_back(evicted.stmt_name);
            if (!dropped.stmt_name.empty())
            {
                close_targets.push_back(std::move(dropped.stmt_name));
                dropped.stmt_name.clear();
            }

            _build_cold_exec(send_buf, stmt_name, sql, params, {}, close_targets.empty() ? nullptr : &close_targets);
        }

        if (!co_await _pg_async_write_buf(this, send_buf))
        {
            error_msg  = "Extended Query async write failed";
            error_code = 20;
            if (warm)
                prepared_meta_.erase(sql);
            if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && co_await async_try_reconnect())
            {
                reconnect_attempt++;
                goto RETRY_RECONNECT;
            }
            isclose = true;
            co_return 0;
        }

        rows           = 0;
        bool got_ready = false;
        bool abandon   = false;

        pooled_accum_buf accum_holder;
        std::vector<uint8_t> &accum_buf = accum_holder.buf;
        _pg_frame_state fs;

        while (!got_ready)
        {
            unsigned int n = co_await async_read_loop();
            if (n == 0)
            {
                error_msg  = "async read failed";
                error_code = 21;
                // 对端被杀/断连 → 读立即失败(CLOSE_WAIT 假写成功). SELECT 幂等, 可安全重连重发
                if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && co_await async_try_reconnect())
                {
                    reconnect_attempt++;
                    goto RETRY_RECONNECT;
                }
                isclose = true;
                break;
            }
            if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_pg_no_byte_cap, k_pg_compact_at))
            {
                error_msg  = "async_fetch_prepared: response exceeds buffer limit";
                error_code = 8;
                isclose    = true;
                break;
            }

            while (true)
            {
                const unsigned char *payload_start = nullptr;
                unsigned int payload_len           = 0;
                unsigned char type                 = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
                if (type == 0)
                    break;

                std::string_view payload(reinterpret_cast<const char *>(payload_start), payload_len);

                if (abandon)
                {
                    if (type == 'Z')
                        got_ready = true;
                    continue;
                }
                switch (type)
                {
                case '1': break;
                case '2': break;
                case 'n': break;
                case 'C': break;

                case 'T':
                {
                    if (payload.size() < 2)
                    {
                        abandon = true;
                        break;
                    }
                    int16_t fc = static_cast<int16_t>(
                        (static_cast<unsigned char>(payload[0]) << 8) | static_cast<unsigned char>(payload[1]));
                    size_t pos = 2;
                    col_names.clear();
                    for (int16_t i = 0; i < fc; ++i)
                    {
                        if (pos >= payload.size())
                        {
                            abandon = true;
                            col_names.clear();
                            break;
                        }
                        size_t start = pos;
                        while (pos < payload.size() && payload[pos] != 0)
                            pos++;
                        col_names.emplace_back(payload.substr(start, pos - start));
                        pos++;
                        if (pos + 18 > payload.size())
                        {
                            abandon = true;
                            col_names.clear();
                            break;
                        }
                        pos += 18;
                    }
                    if (!abandon)
                    {
                        auto &slot         = prepared_meta_.upsert(sql);
                        slot.col_names     = col_names;
                        slot.needs_reparse = false;
                        name_ptrs_cache.clear();
                        for (auto &cn : col_names)
                            name_ptrs_cache.push_back(cn.data());
                    }
                    break;
                }

                case 'D':
                {
                    if (col_names.empty())
                        break;
                    if (payload.size() < 2)
                    {
                        abandon = true;
                        break;
                    }
                    size_t pos        = 0;
                    int16_t col_count = static_cast<int16_t>(
                        (static_cast<unsigned char>(payload[0]) << 8) | static_cast<unsigned char>(payload[1]));
                    pos += 2;

                    std::vector<std::string> row_values;
                    std::vector<char> row_is_null;
                    row_values.reserve(col_count > 0 ? static_cast<size_t>(col_count) : 0);
                    row_is_null.reserve(col_count > 0 ? static_cast<size_t>(col_count) : 0);
                    for (int16_t i = 0; i < col_count; ++i)
                    {
                        if (pos + 4 > payload.size())
                        {
                            abandon = true;
                            break;
                        }
                        int32_t vlen = buf_to_int32(reinterpret_cast<const unsigned char *>(payload.data() + pos));
                        pos += 4;
                        if (vlen == -1)
                        {
                            row_values.emplace_back();
                            row_is_null.push_back(1);
                        }
                        else if (vlen >= 0 && pos + static_cast<size_t>(vlen) <= payload.size())
                        {
                            row_values.emplace_back(payload.substr(pos, vlen));
                            row_is_null.push_back(0);
                            pos += vlen;
                        }
                        else
                        {
                            abandon = true;
                            break;
                        }
                    }
                    if (abandon)
                        break;

                    if (!warm)
                    {
                        name_ptrs_cache.clear();
                        for (auto &cn : col_names)
                            name_ptrs_cache.push_back(cn.data());
                    }

                    auto captured      = std::move(row_values);
                    auto captured_null = std::move(row_is_null);
                    bool cont          = handler(
                        static_cast<int>(col_count),
                        name_ptrs_cache.data(),
                        [captured, captured_null](int idx) -> std::tuple<unsigned char *, size_t>
                        {
                            if (idx < 0 || idx >= static_cast<int>(captured.size()))
                                return {nullptr, 0};
                            if (captured_null[static_cast<size_t>(idx)])
                                return {nullptr, 0};
                            const auto &s = captured[idx];
                            return {reinterpret_cast<unsigned char *>(const_cast<char *>(s.data())), s.size()};
                        });
                    rows++;
                    if (!cont)
                        abandon = true;
                    break;
                }

                case 'Z':
                {
                    if (payload.size() >= 1)
                        in_transaction_.store(payload[0] == 'T' || payload[0] == 'E');
                    got_ready = true;
                    break;
                }
                case 'E':
                {
                    parse_error_from_payload((const unsigned char *)payload.data(),
                                             static_cast<unsigned int>(payload.size()));
                    std::string sqlstate = _extract_sqlstate(
                        (const unsigned char *)payload.data(),
                        payload.size());
                    if (_pg_sqlstate_needs_reparse(sqlstate))
                        prepared_meta_.mark_needs_reparse(sql);
                    else if (_pg_sqlstate_is_transient(sqlstate))
                        prepared_meta_.erase(sql, &dropped);
                    abandon = true;
                    break;
                }
                case 'G':
                {
                    error_msg  = "COPY FROM STDIN is not supported by this client";
                    error_code = 8;
                    isclose    = true;
                    got_ready  = true;
                    break;
                }
                case 'I':
                case 's':
                case 'S':
                case 'N':
                case 'A':
                default: break;
                }
            }
        }

        const bool skip_close = warm && got_ready && !abandon && !isclose && error_code == 0;
        if (!isclose && !skip_close)
        {
            std::string close_buf;
            {
                std::string p;
                p.push_back('P');
                _append_str(p, "");
                _write_pg_msg(close_buf, 'C', p);
            }
            {
                std::string p;
                _write_pg_msg(close_buf, 'S', p);
            }
            co_await _pg_async_write_buf(this, close_buf);
            co_await async_drain_until_ready(accum_buf, fs.consumed);
        }

        const bool need_cold =
            warm && !isclose && error_code != 21 &&
            error_code != 0;
        if (!need_cold)
            break;
        prepared_meta_.erase(sql, &dropped);
    }
    co_return rows;
}

// ===== 连接级 SQL→结果列元数据缓存实现（§17.3 W2-1）=====
// 所有权在 pg_conn_base 上 ⇒ 查找天然本连接私有，不需要全局 hashid（§15.4）。
// 淘汰用「插入时线性扫 lru_seq 最小项」：容量 64、只在冷路径插入，
// 省掉一个 list 容器与一处迭代器失效面。
//
// 缓存 key 直接用原始 SQL 文本，不做规范化：
//   - PG Parse('S', name, sql) 与 MySQL COM_STMT_PREPARE 服务器端都按原始 SQL 做 parse，
//     客户端规范化反而会出现 "cache hit 但服务器端是两条不同语句" 的语义错位；
//   - 本项目 SQL 全部由 ORM 生成，同一条 query builder 产出的文本完全稳定，
//     "同语义不同写法"（尾部空格、分号等）在实际流量中不存在；
//   - 如果将来支持手写 SQL 直连，可以加 trim + 去尾分号的轻量规范化（PG 服务器自身也忽略这两项），
//     但需要同步改 Parse 发送的 SQL 才能保证 key 与服务器端行为一致，不能只改一边。
const pg_conn_base::pg_stmt_meta_t *pg_conn_base::pg_stmt_meta_cache::find(const std::string &sql)
{
    auto it = map_.find(sql);
    if (it == map_.end())
    {
        ++misses_;
        return nullptr;
    }
    ++hits_;
    it->second.lru_seq = ++seq_;
    return &it->second;
}

pg_conn_base::pg_stmt_meta_t &pg_conn_base::pg_stmt_meta_cache::upsert(const std::string &sql, pg_stmt_meta_t *evicted_out)
{
    auto it = map_.find(sql);
    if (it != map_.end())
    {
        it->second.lru_seq = ++seq_;
        return it->second;
    }
    if (map_.size() >= cap_)
    {
        auto victim = map_.begin();
        for (auto i = map_.begin(); i != map_.end(); ++i)
            if (i->second.lru_seq < victim->second.lru_seq)
                victim = i;
        if (evicted_out)
            *evicted_out = std::move(victim->second);
        map_.erase(victim);
    }
    auto ins                  = map_.emplace(sql, pg_stmt_meta_t{});
    ins.first->second.lru_seq = ++seq_;
    return ins.first->second;
}

void pg_conn_base::pg_stmt_meta_cache::erase(const std::string &sql, pg_stmt_meta_t *removed_out)
{
    auto it = map_.find(sql);
    if (it == map_.end())
        return;
    if (removed_out)
        *removed_out = std::move(it->second);
    map_.erase(it);
}

void pg_conn_base::pg_stmt_meta_cache::mark_needs_reparse(const std::string &sql)
{
    auto it = map_.find(sql);
    if (it != map_.end())
        it->second.needs_reparse = true;
}

void pg_conn_base::pg_stmt_meta_cache::mark_all_needs_reparse()
{
    // DDL 经文本路径执行成功后调用：粗暴全失效（entry 不存表名，精确失效难做且易漏）。
    // 只影响性能（下次全 cold），不影响正确性，是安全选择。
    for (auto &kv : map_)
        kv.second.needs_reparse = true;
}

std::string pg_conn_base::pg_stmt_meta_cache::next_stmt_name()
{
    return "pg_" + std::to_string(++name_counter_);
}

// ===== PG Extended Query SELECT BINARY (sync) =====
// 冷/热分派 + warm 失败轮内重发一次（§17.2 / W2-5）。
// 不变式（Wave 2 全部正确性论证都压在它上面）：
//   缓存只用来决定「这次向服务端要什么格式」；解码永远只用本轮 'T' 实际给出的 (OID, format)。
unsigned int pg_conn_base::fetch_prepared_impl_binary(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<orm::col_value_variant(int)>)> handler)
{
    unsigned int reconnect_attempt = 0;
RETRY_RECONNECT:
    unsigned int rows = 0;
    std::vector<char *> name_ptrs_cache;
    std::vector<std::string> row_date_backing;
    std::vector<orm::col_value_variant> row_variants;
    std::vector<std::string> col_names;
    std::vector<uint32_t> col_oids_for_result;
    std::vector<uint16_t> col_formats;
    constexpr size_t k_max_result_bytes = 16u * 1024u * 1024u;

    pg_stmt_meta_t dropped;// PG-5: warm 轮摘掉的旧语句名，随下一轮冷包一起 DEALLOCATE
    for (unsigned int warm_attempt = 0; warm_attempt < 2; ++warm_attempt)
    {
        error_msg.clear();
        error_code = 0;

        const pg_stmt_meta_t *m = (warm_attempt == 0) ? prepared_meta_.find(sql) : nullptr;
        const bool warm         = (m != nullptr && !m->needs_reparse);
        std::string send_buf;

        if (warm)
        {
            std::vector<uint16_t> req_fmts;
            for (uint32_t oid : m->col_oids)
                req_fmts.push_back(_pg_binary_decodable(oid) ? 1 : 0);
            _build_warm_exec(send_buf, m->stmt_name, params, req_fmts);

            col_names           = m->col_names;
            col_oids_for_result = m->col_oids;
            col_formats         = m->col_fmt;
            row_variants.reserve(col_oids_for_result.size());
            name_ptrs_cache.clear();
            name_ptrs_cache.reserve(col_names.size());
            for (auto &n : col_names)
                name_ptrs_cache.push_back(n.data());
        }
        else
        {
            pg_stmt_meta_t needs_close;
            const pg_stmt_meta_t *old = (warm_attempt == 0) ? m : nullptr;
            if (old && old->needs_reparse)
            {
                prepared_meta_.erase(sql, &needs_close);
            }

            pg_stmt_meta_t evicted;
            std::string stmt_name = prepared_meta_.next_stmt_name();
            auto &slot            = prepared_meta_.upsert(sql, &evicted);
            slot.stmt_name        = stmt_name;
            slot.needs_reparse    = false;

            std::vector<std::string> close_targets;
            if (!needs_close.stmt_name.empty())
                close_targets.push_back(needs_close.stmt_name);
            if (!evicted.stmt_name.empty())
                close_targets.push_back(evicted.stmt_name);
            if (!dropped.stmt_name.empty())
            {
                close_targets.push_back(std::move(dropped.stmt_name));
                dropped.stmt_name.clear();
            }

            _build_cold_exec(send_buf, stmt_name, sql, params, {}, close_targets.empty() ? nullptr : &close_targets);
        }

        if (!_pg_write_buf(this, send_buf))
        {
            error_msg  = "Extended Query write failed";
            error_code = 20;
            if (warm)
                prepared_meta_.erase(sql);
            if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
            {
                reconnect_attempt++;
                goto RETRY_RECONNECT;
            }
            isclose = true;
            return rows;
        }

        bool abandon   = false;
        bool got_ready = false;
        if (!warm)
        {
            col_names.clear();
            col_oids_for_result.clear();
            col_formats.clear();
        }

        pooled_accum_buf accum_holder;
        std::vector<uint8_t> &accum_buf = accum_holder.buf;
        _pg_frame_state fs;

        while (!got_ready)
        {
            unsigned char type = 0;
            std::string_view payload;
            {
                const unsigned char *payload_start = nullptr;
                unsigned int payload_len           = 0;
                type                               = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
                if (type != 0)
                    payload = std::string_view(reinterpret_cast<const char *>(payload_start), payload_len);
            }
            if (type == 0)
            {
                unsigned int n = read_loop();
                if (n == 0)
                {
                    error_msg  = "read failed";
                    error_code = 21;
                    // 对端被杀/断连 → 读立即失败(CLOSE_WAIT 假写成功). SELECT 幂等, 可安全重连重发
                    if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && try_reconnect())
                    {
                        reconnect_attempt++;
                        goto RETRY_RECONNECT;
                    }
                    isclose = true;
                    break;
                }
                if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_max_result_bytes, 0))
                {
                    error_msg  = "fetch_prepared_binary: result set exceeds 16MB limit";
                    error_code = 8;
                    isclose    = true;
                    break;
                }
                continue;
            }
            if (abandon)
            {
                if (type == 'Z')
                    got_ready = true;
                continue;
            }
            switch (type)
            {
            case '1': break;
            case '3': break;

            case 'T':
            {
                _parse_row_description(reinterpret_cast<const unsigned char *>(payload.data()),
                                       payload.size(),
                                       col_names,
                                       col_oids_for_result,
                                       col_formats);
                auto &slot         = prepared_meta_.upsert(sql);
                slot.col_oids      = col_oids_for_result;
                slot.col_fmt       = col_formats;
                slot.col_names     = col_names;
                slot.needs_reparse = false;
                row_variants.reserve(col_oids_for_result.size());
                name_ptrs_cache.clear();
                name_ptrs_cache.reserve(col_names.size());
                for (auto &n : col_names)
                    name_ptrs_cache.push_back(n.data());
                break;
            }

            case 'D':
            {
                if (col_names.empty())
                    break;
                if (payload.size() < 2)
                {
                    abandon = true;
                    break;
                }
                row_variants.clear();
                row_date_backing.clear();
                row_date_backing.reserve(col_oids_for_result.size());
                if (!_decode_pg_binary_row(payload, col_oids_for_result, col_formats, row_variants, row_date_backing))
                {
                    abandon    = true;
                    error_msg  = "fetch_prepared_binary: unsupported binary column";
                    error_code = 23;
                    break;
                }
                bool cont = handler(
                    (int)col_names.size(),
                    name_ptrs_cache.data(),
                    [&row_variants, &row_date_backing](int idx) -> orm::col_value_variant
                    {
                        (void)&row_date_backing;
                        if (idx < 0 || idx >= (int)row_variants.size())
                            return orm::col_value_variant{std::monostate{}};
                        return row_variants[(size_t)idx];
                    });
                ++rows;
                if (!cont)
                {
                    abandon = true;
                    break;
                }
                break;
            }

            case 'C': break;
            case 'Z':
            {
                if (payload.size() >= 1)
                    in_transaction_.store(payload[0] == 'T' || payload[0] == 'E');
                got_ready = true;
                break;
            }
            case 'E':
            {
                std::string err_msg;
                const unsigned char *pd = (const unsigned char *)payload.data();
                size_t pp               = 0;
                while (pp < payload.size())
                {
                    unsigned char ft = pd[pp++];
                    if (ft == 0)
                        break;
                    size_t start = pp;
                    while (pp < payload.size() && pd[pp] != 0)
                        ++pp;
                    if (ft == 'M')
                        err_msg.assign((const char *)(pd + start), pp - start);
                    if (pp >= payload.size())
                        break;
                    ++pp;
                }
                error_msg  = err_msg.empty() ? "PG error response" : err_msg;
                error_code = 22;

                std::string sqlstate = _extract_sqlstate(pd, payload.size());
                if (_pg_sqlstate_needs_reparse(sqlstate))
                    prepared_meta_.mark_needs_reparse(sql);
                else if (_pg_sqlstate_is_transient(sqlstate))
                    prepared_meta_.erase(sql, &dropped);
                break;
            }
            case 'G':
            {
                error_msg  = "COPY FROM STDIN is not supported by this client";
                error_code = 8;
                isclose    = true;
                got_ready  = true;
                break;
            }
            case 'N':
            case 'S':
            case 'A':
            case 't': break;
            default: break;
            }
        }

        const bool skip_close = warm && got_ready && !abandon && error_code == 0;
        if (!isclose && !skip_close)
        {
            std::string close_buf;
            {
                std::string p;
                p.push_back('P');
                _append_str(p, "");
                _write_pg_msg(close_buf, 'C', p);
            }
            {
                std::string p;
                _write_pg_msg(close_buf, 'S', p);
            }
            _pg_write_buf(this, close_buf);
            drain_until_ready_sync(accum_buf, fs.consumed);
        }

        const bool need_cold =
            warm && rows == 0 && !isclose && error_code != 21 &&
            error_code != 0;
        if (!need_cold)
            break;
        prepared_meta_.erase(sql, &dropped);
    }
    return rows;
}

// ===== PG Extended Query SELECT BINARY (async) =====
asio::awaitable<unsigned int> pg_conn_base::async_fetch_prepared_impl_binary(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<orm::col_value_variant(int)>)> handler)
{
    unsigned int reconnect_attempt = 0;
RETRY_RECONNECT:
    unsigned int rows = 0;
    std::vector<uint32_t> col_oids_for_result;
    std::vector<std::string> col_names;
    std::vector<std::string> row_date_backing;
    std::vector<orm::col_value_variant> row_variants;
    std::vector<char *> name_ptrs_cache;
    std::vector<uint16_t> col_formats;
    constexpr size_t k_max_result_bytes = 16u * 1024u * 1024u;

    pg_stmt_meta_t dropped;// PG-5: warm 轮摘掉的旧语句名，随下一轮冷包一起 DEALLOCATE
    for (unsigned int warm_attempt = 0; warm_attempt < 2; ++warm_attempt)
    {
        error_msg.clear();
        error_code = 0;

        const pg_stmt_meta_t *m = (warm_attempt == 0) ? prepared_meta_.find(sql) : nullptr;
        const bool warm         = (m != nullptr && !m->needs_reparse);
        std::string send_buf;

        if (warm)
        {
            std::vector<uint16_t> req_fmts;
            for (uint32_t oid : m->col_oids)
                req_fmts.push_back(_pg_binary_decodable(oid) ? 1 : 0);
            _build_warm_exec(send_buf, m->stmt_name, params, req_fmts);

            col_names           = m->col_names;
            col_oids_for_result = m->col_oids;
            col_formats         = m->col_fmt;
            row_variants.reserve(col_oids_for_result.size());
            name_ptrs_cache.clear();
            name_ptrs_cache.reserve(col_names.size());
            for (auto &n : col_names)
                name_ptrs_cache.push_back(n.data());
        }
        else
        {
            pg_stmt_meta_t needs_close;
            const pg_stmt_meta_t *old = (warm_attempt == 0) ? m : nullptr;
            if (old && old->needs_reparse)
            {
                prepared_meta_.erase(sql, &needs_close);
            }

            pg_stmt_meta_t evicted;
            std::string stmt_name = prepared_meta_.next_stmt_name();
            auto &slot            = prepared_meta_.upsert(sql, &evicted);
            slot.stmt_name        = stmt_name;
            slot.needs_reparse    = false;

            std::vector<std::string> close_targets;
            if (!needs_close.stmt_name.empty())
                close_targets.push_back(needs_close.stmt_name);
            if (!evicted.stmt_name.empty())
                close_targets.push_back(evicted.stmt_name);
            if (!dropped.stmt_name.empty())
            {
                close_targets.push_back(std::move(dropped.stmt_name));
                dropped.stmt_name.clear();
            }

            _build_cold_exec(send_buf, stmt_name, sql, params, {}, close_targets.empty() ? nullptr : &close_targets);
        }

        if (!co_await _pg_async_write_buf(this, send_buf))
        {
            error_msg  = "Extended Query async write failed";
            error_code = 20;
            if (warm)
                prepared_meta_.erase(sql);
            if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && co_await async_try_reconnect())
            {
                reconnect_attempt++;
                goto RETRY_RECONNECT;
            }
            isclose = true;
            co_return rows;
        }

        bool got_ready = false;
        bool abandon   = false;
        if (!warm)
        {
            col_names.clear();
            col_oids_for_result.clear();
            col_formats.clear();
        }
        _pg_frame_state fs;
        pooled_accum_buf accum_holder;
        auto &accum_buf = accum_holder.buf;

        while (!got_ready)
        {
            unsigned int n = co_await async_read_loop();
            if (n == 0)
            {
                error_msg  = "async read failed";
                error_code = 21;
                // 对端被杀/断连 → 读立即失败(CLOSE_WAIT 假写成功). SELECT 幂等, 可安全重连重发
                if (reconnect_attempt < kMaxReconnect && is_last_error_reconnectable() && co_await async_try_reconnect())
                {
                    reconnect_attempt++;
                    goto RETRY_RECONNECT;
                }
                isclose = true;
                break;
            }
            if (!_pg_feed(accum_buf, fs, reinterpret_cast<const char *>(_cache_data), static_cast<size_t>(n), k_max_result_bytes, 0))
            {
                error_msg  = "async_fetch_prepared_binary: result set exceeds 16MB limit";
                error_code = 8;
                isclose    = true;
                break;
            }

            while (true)
            {
                const unsigned char *payload_start = nullptr;
                unsigned int payload_len           = 0;
                unsigned char type                 = _pg_next_frame(accum_buf, fs, payload_start, payload_len);
                if (type == 0)
                    break;
                std::string_view payload(reinterpret_cast<const char *>(payload_start), payload_len);

                if (abandon)
                {
                    if (type == 'Z')
                        got_ready = true;
                    continue;
                }
                switch (type)
                {
                case '1': break;
                case '3': break;

                case 'T':
                {
                    _parse_row_description(reinterpret_cast<const unsigned char *>(payload.data()),
                                           payload.size(),
                                           col_names,
                                           col_oids_for_result,
                                           col_formats);
                    auto &slot         = prepared_meta_.upsert(sql);
                    slot.col_oids      = col_oids_for_result;
                    slot.col_fmt       = col_formats;
                    slot.col_names     = col_names;
                    slot.needs_reparse = false;
                    row_variants.reserve(col_oids_for_result.size());
                    name_ptrs_cache.clear();
                    name_ptrs_cache.reserve(col_names.size());
                    for (auto &cn : col_names)
                        name_ptrs_cache.push_back(cn.data());
                    break;
                }

                case 'D':
                {
                    if (col_names.empty())
                        break;
                    if (payload.size() < 2)
                    {
                        abandon = true;
                        break;
                    }
                    row_variants.clear();
                    row_date_backing.clear();
                    row_date_backing.reserve(col_oids_for_result.size());
                    if (!_decode_pg_binary_row(payload, col_oids_for_result, col_formats, row_variants, row_date_backing))
                    {
                        abandon    = true;
                        error_msg  = "async_fetch_prepared_binary: unsupported binary column";
                        error_code = 23;
                        break;
                    }
                    bool cont = handler(
                        (int)col_names.size(),
                        name_ptrs_cache.data(),
                        [&row_variants, &row_date_backing](int idx) -> orm::col_value_variant
                        {
                            (void)&row_date_backing;
                            if (idx < 0 || idx >= (int)row_variants.size())
                                return orm::col_value_variant{std::monostate{}};
                            return row_variants[(size_t)idx];
                        });
                    ++rows;
                    if (!cont)
                    {
                        abandon = true;
                        break;
                    }
                    break;
                }

                case 'C': break;
                case 'Z':
                {
                    if (payload.size() >= 1)
                        in_transaction_.store(payload[0] == 'T' || payload[0] == 'E');
                    got_ready = true;
                    break;
                }
                case 'E':
                {
                    parse_error_from_payload((const unsigned char *)payload.data(),
                                             static_cast<unsigned int>(payload.size()));
                    std::string sqlstate = _extract_sqlstate(
                        (const unsigned char *)payload.data(),
                        payload.size());
                    if (_pg_sqlstate_needs_reparse(sqlstate))
                        prepared_meta_.mark_needs_reparse(sql);
                    else if (_pg_sqlstate_is_transient(sqlstate))
                        prepared_meta_.erase(sql, &dropped);
                    break;
                }
                case 'G':
                {
                    error_msg  = "COPY FROM STDIN is not supported by this client";
                    error_code = 8;
                    isclose    = true;
                    got_ready  = true;
                    break;
                }
                case 'N':
                case 'S':
                case 'A':
                case 't': break;
                default: break;
                }
            }
        }

        const bool skip_close = warm && got_ready && !abandon && error_code == 0;
        if (!isclose && !skip_close)
        {
            std::string close_buf;
            {
                std::string p;
                p.push_back('P');
                _append_str(p, "");
                _write_pg_msg(close_buf, 'C', p);
            }
            {
                std::string p;
                _write_pg_msg(close_buf, 'S', p);
            }
            co_await _pg_async_write_buf(this, close_buf);
            co_await async_drain_until_ready(accum_buf, fs.consumed);
        }

        const bool need_cold =
            warm && rows == 0 && !isclose && error_code != 21 &&
            error_code != 0;
        if (!need_cold)
            break;
        prepared_meta_.erase(sql, &dropped);
    }
    co_return rows;
}

}// namespace orm
