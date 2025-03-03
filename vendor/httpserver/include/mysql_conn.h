#ifndef _MYSQL_CONN_BASE_H
#define _MYSQL_CONN_BASE_H
/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <iostream>
#include <memory>
#include <string>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>

namespace orm
{
enum enum_field_types
{
    MYSQL_TYPE_DECIMAL,
    MYSQL_TYPE_TINY,
    MYSQL_TYPE_SHORT,
    MYSQL_TYPE_LONG,
    MYSQL_TYPE_FLOAT,
    MYSQL_TYPE_DOUBLE,
    MYSQL_TYPE_NULL,
    MYSQL_TYPE_TIMESTAMP,
    MYSQL_TYPE_LONGLONG,
    MYSQL_TYPE_INT24,
    MYSQL_TYPE_DATE,
    MYSQL_TYPE_TIME,
    MYSQL_TYPE_DATETIME,
    MYSQL_TYPE_YEAR,
    MYSQL_TYPE_NEWDATE, /**< Internal to MySQL. Not used in protocol */
    MYSQL_TYPE_VARCHAR,
    MYSQL_TYPE_BIT,
    MYSQL_TYPE_TIMESTAMP2,
    MYSQL_TYPE_DATETIME2,   /**< Internal to MySQL. Not used in protocol */
    MYSQL_TYPE_TIME2,       /**< Internal to MySQL. Not used in protocol */
    MYSQL_TYPE_TYPED_ARRAY, /**< Used for replication only */
    MYSQL_TYPE_INVALID     = 243,
    MYSQL_TYPE_BOOL        = 244, /**< Currently just a placeholder */
    MYSQL_TYPE_JSON        = 245,
    MYSQL_TYPE_NEWDECIMAL  = 246,
    MYSQL_TYPE_ENUM        = 247,
    MYSQL_TYPE_SET         = 248,
    MYSQL_TYPE_TINY_BLOB   = 249,
    MYSQL_TYPE_MEDIUM_BLOB = 250,
    MYSQL_TYPE_LONG_BLOB   = 251,
    MYSQL_TYPE_BLOB        = 252,
    MYSQL_TYPE_VAR_STRING  = 253,
    MYSQL_TYPE_STRING      = 254,
    MYSQL_TYPE_GEOMETRY    = 255
};

#define CLIENT_LONG_PASSWORD 1                           // 1
#define CLIENT_FOUND_ROWS 2                              // 2
#define CLIENT_LONG_FLAG 4                               // 3     Get all column flags
#define CLIENT_CONNECT_WITH_DB 8                         // 4
#define CLIENT_NO_SCHEMA 16                              // 5
#define CLIENT_COMPRESS 32                               // 6
#define CLIENT_ODBC 64                                   // 7
#define CLIENT_LOCAL_FILES 128                           // 8
#define CLIENT_IGNORE_SPACE 256                          // 9
#define CLIENT_PROTOCOL_41 512                           // 10    Uses the 4.1 protocol.
#define CLIENT_INTERACTIVE 1024                          // 11    Client is interactive.
#define CLIENT_SSL 2048                                  // 12    Switch to SSL after sending the capability-flags.
#define CLIENT_IGNORE_SIGPIPE 4096                       // 13
#define CLIENT_TRANSACTIONS 8192                         // 14
#define CLIENT_RESERVED 16384                            // 15
#define CLIENT_RESERVED2 32768                           // 16
#define CLIENT_MULTI_STATEMENTS (1UL << 16)              // 17 Enable/disable multi-stmt support
#define CLIENT_MULTI_RESULTS (1UL << 17)                 // 18 Enable/disable multi-results
#define CLIENT_PS_MULTI_RESULTS (1UL << 18)              // 19 Multi-results and OUT parameters in PS-protocol.
#define CLIENT_PLUGIN_AUTH (1UL << 19)                   // 20 Client supports plugin authentication
#define CLIENT_CONNECT_ATTRS (1UL << 20)                 // 21 Client supports connection attributes
#define CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA (1UL << 21)// 22    Enable authentication response packet to be larger than 255 bytes.
#define CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS (1UL << 22)  // 23    Don't close the connection for a user account with expired password.
#define CLIENT_SESSION_TRACK (1UL << 23)                 // 24
#define CLIENT_DEPRECATE_EOF (1UL << 24)                 // 25     Client no longer needs @ref page_protocol_basic_eof_packet and will use
#define CLIENT_OPTIONAL_RESULTSET_METADATA (1UL << 25)
#define CLIENT_ZSTD_COMPRESSION_ALGORITHM (1UL << 26)// 27    Compression protocol extended to support zstd compression method

#define CLIENT_PZORM_FLAGS                                               \
    (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG |       \
     CLIENT_CONNECT_WITH_DB | CLIENT_PROTOCOL_41 | CLIENT_TRANSACTIONS | \
     CLIENT_MULTI_RESULTS | CLIENT_PLUGIN_AUTH | CLIENT_DEPRECATE_EOF | CLIENT_OPTIONAL_RESULTSET_METADATA)

struct mysql_server_hello_data_t
{
    unsigned char protocol_version = 0;
    std::string server_version;
    unsigned int connection_id = 0;
    std::string auth_plugin_salt_data;
    unsigned short capability_flags_low  = 0;
    unsigned char character_set          = 0;
    unsigned short status_flags          = 0;
    unsigned short capability_flags_high = 0;
    unsigned char auth_plugin_data_len   = 0;
    // std::string auth_plugin_data_part_2;
    std::string auth_plugin_name;
};

struct pack_info_t
{
    unsigned int length         = 0;
    unsigned int current_length = 0;
    unsigned char seq_id        = 0;
    unsigned char error         = 0;
    std::string data;
};

struct field_info_t
{
    std::string org_name;
    std::string name;
    std::string org_table;
    std::string table;
    std::string dbname;
    unsigned char field_type;
    unsigned char decimals;
    unsigned short charset;
    unsigned short flags;
    unsigned int column_length;
};

struct orm_conn_t
{
    // std::string type;
    std::string host;
    std::string port;
    std::string dbname;
    std::string user;
    std::string password;
    std::string pretable;
    std::string dbtype;

    std::string tag;
    std::string charset;

    bool isssl              = false;
    bool issock             = false;
    unsigned char link_type = 0;// 0 edit 1 select 2 backup
    unsigned char max_pool  = 0;
    unsigned char min_pool  = 0;
};

class mysql_conn_base
{
  public:
    mysql_conn_base(asio::io_context &ioc);
    ~mysql_conn_base();
    void read_server_hello(unsigned int offset, unsigned int length);
    bool server_public_key_encrypt(const std::string &password, unsigned char *data, unsigned int length);
    bool connect(const std::string &host, const std::string &port, const std::string &user, const std::string &password, const std::string &dbname, bool ssl = false);
    asio::awaitable<bool> async_connect(const std::string &host, const std::string &port, const std::string &user, const std::string &password, const std::string &dbname, bool ssl = false);

    void mysqlnd_xor_string(char *dst, const size_t dst_len, const char *xor_str, const size_t xor_str_len);
    unsigned int read_pack(unsigned char *data, unsigned int offset);
    void read_field_pack(unsigned char *data, unsigned int total_num, unsigned int &offset, pack_info_t &pack_info);
    bool pack_eof_check(const pack_info_t &temp_pack_data);
    void read_col_info(const std::string &pack_info, field_info_t &temp_field_data);
    unsigned int pack_real_num(const unsigned char *data, unsigned int &offset);

    bool ping();
    bool close();
    asio::awaitable<unsigned int> async_read_loop();
    unsigned int read_loop();

    unsigned int write();
    asio::awaitable<unsigned int> async_write();

    unsigned int write_sql(const std::string &sql);
    asio::awaitable<unsigned int> async_write_sql(const std::string &sql);

    bool is_closed();

  public:
    unsigned char *_cache_data = nullptr;

    bool isclose            = false;
    unsigned char sock_type = 0;
    unsigned char seq_next_id;
    unsigned short error_code = 0;
    asio::error_code ec;
    unsigned int client_flags = 0;

    std::string send_data;
    std::string error_msg;
    //std::string server_public_key;
    std::unique_ptr<asio::ip::tcp::socket> socket;
    std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket;
    std::unique_ptr<asio::local::stream_protocol::socket> localsocket;
    asio::io_context *io_ctx = nullptr;
    mysql_server_hello_data_t server_hello;
};
}// namespace orm
#endif