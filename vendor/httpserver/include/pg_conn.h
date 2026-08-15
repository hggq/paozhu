#ifndef _PG_CONN_BASE_H
#define _PG_CONN_BASE_H
/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 * @Description: PostgreSQL connection base class, similar structure to mysql_conn.h
 */

#ifndef ASIO_USE_COROUTINES_TS
#define ASIO_USE_COROUTINES_TS
#endif

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <cstring>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/use_awaitable.hpp>
#include "orm_common.h"

// PostgreSQL protocol constants
#define PG_PROTOCOL_3_0 196608// (3 << 16) | 0
#define PG_SSL_REQUEST_CODE 80877103

// PostgreSQL message types (frontend)
#define PG_MSG_QUERY 'Q'
#define PG_MSG_PARSE 'P'
#define PG_MSG_BIND 'B'
#define PG_MSG_EXECUTE 'E'
#define PG_MSG_DESCRIBE 'D'
#define PG_MSG_CLOSE 'C'
#define PG_MSG_SYNC 'S'
#define PG_MSG_FLUSH 'H'
#define PG_MSG_TERMINATE 'X'
#define PG_MSG_PASSWORD 'p'

// PostgreSQL message types (backend)
#define PG_MSG_AUTH 'R'
#define PG_MSG_BACKEND_KEY 'K'
#define PG_MSG_PARAMETER_STATUS 'S'
#define PG_MSG_READY_FOR_QUERY 'Z'
#define PG_MSG_ERROR 'E'
#define PG_MSG_NOTICE 'N'
#define PG_MSG_ROW_DESCRIPTION 'T'
#define PG_MSG_DATA_ROW 'D'
#define PG_MSG_COMMAND_COMPLETE 'C'
#define PG_MSG_EMPTY_QUERY 'I'

// PostgreSQL authentication types
#define PG_AUTH_OK 0
#define PG_AUTH_KERBEROS_V5 2
#define PG_AUTH_CLEARTEXT_PASSWORD 3
#define PG_AUTH_MD5_PASSWORD 5
#define PG_AUTH_SCM_CREDENTIAL 6
#define PG_AUTH_GSS 7
#define PG_AUTH_GSS_CONTINUE 8
#define PG_AUTH_SASL 10
#define PG_AUTH_SASL_CONTINUE 11
#define PG_AUTH_SASL_FINAL 12

namespace orm
{

// ======================== Utility Functions ========================

inline void int32_to_buf(int32_t val, unsigned char *buf)
{
    buf[0] = (val >> 24) & 0xFF;
    buf[1] = (val >> 16) & 0xFF;
    buf[2] = (val >> 8) & 0xFF;
    buf[3] = val & 0xFF;
}

inline int32_t buf_to_int32(const unsigned char *buf)
{
    return (static_cast<int32_t>(buf[0]) << 24) |
           (static_cast<int32_t>(buf[1]) << 16) |
           (static_cast<int32_t>(buf[2]) << 8) |
           static_cast<int32_t>(buf[3]);
}

inline void int16_to_buf(int16_t val, unsigned char *buf)
{
    buf[0] = (val >> 8) & 0xFF;
    buf[1] = val & 0xFF;
}

inline int16_t buf_to_int16(const unsigned char *buf)
{
    return (static_cast<int16_t>(buf[0]) << 8) | static_cast<int16_t>(buf[1]);
}

// ======================== PG Query Result Types ========================

struct pg_row_data_t
{
    std::vector<std::string> values;// 每列的值（NULL 用空字符串+is_null标记）
    std::vector<bool> is_null;
};

// ======================== PostgreSQL Connection Base Class ========================

class pg_conn_base
{
  public:
    pg_conn_base(std::unique_ptr<orm_conn_link_t> link);
    ~pg_conn_base();

    // Connection methods
    bool connect(const orm_conn_t &conn_config);
    asio::awaitable<bool> async_connect(const orm_conn_t &conn_config);

    // Protocol parsing methods
    unsigned int read_pack(unsigned char *data, unsigned int offset);
    void read_col_info(const std::string &pack_info, field_info_t &temp_field_data);
    bool pack_eof_check(const pack_info_t &temp_pack_data);

    // PG query result parsing
    std::vector<field_info_t> parse_row_description(const unsigned char *data, unsigned int len);
    pg_row_data_t parse_data_row(const unsigned char *data, unsigned int len);
    unsigned int parse_command_complete(const unsigned char *data, unsigned int len);

    // Execute SQL and collect all results (sync)
    // Returns: 0=success, >0=error
    unsigned int execute_and_fetch(
        const std::string &sql,
        std::vector<field_info_t> &fields_out,
        std::vector<pg_row_data_t> &rows_out,
        unsigned int &affected_rows_out);

    // Execute SQL and collect all results (async)
    asio::awaitable<unsigned int> async_execute_and_fetch(
        const std::string &sql,
        std::vector<field_info_t> &fields_out,
        std::vector<pg_row_data_t> &rows_out,
        unsigned int &affected_rows_out);

    // Connection control
    bool ping();
    bool close();
    bool hard_close();
    asio::awaitable<bool> async_close();

    // Read/Write operations
    unsigned int read_loop();
    asio::awaitable<unsigned int> async_read_loop();

    unsigned int write_sql(const std::string &sql);
    asio::awaitable<unsigned int> async_write_sql(const std::string &sql);

    unsigned int write();
    asio::awaitable<unsigned int> async_write();

    // Utility methods
    void reset_data();
    bool is_closed();

    void begin_time();
    void finish_time();
    long long count_time();

  private:
    // Synchronous helper methods
    bool read_full_sync(unsigned char *buf, size_t len);
    bool read_message_sync(unsigned char &msg_type, std::string &payload);
    bool send_message_sync(unsigned char type, const std::string &payload);
    void parse_error(const std::string &payload);
    bool handle_auth(const std::string &payload, const orm_conn_t &conn_config);
    bool sasl_scram_sha256_sync(const std::string &mechanisms, const orm_conn_t &conn_config);
    bool process_server_messages_until_ready_sync(const orm_conn_t &conn_config);

    // Asynchronous helper methods
    bool sasl_scram_sha256_async(const std::string &mechanisms, const orm_conn_t &conn_config);

    // Message building methods
    void build_startup_message(const orm_conn_t &conn_config);
    void build_ssl_request();

  public:
    // Public member variables (similar to mysql_conn_base)
    unsigned char *_cache_data = nullptr;

    bool server_enable_ssl   = false;
    bool isclose             = false;
    bool isdebug             = false;
    std::atomic_bool issynch = false;
    //unsigned char sock_type = 0;  // 0: none, 1: tcp, 2: ssl
    unsigned short error_code = 0;
    unsigned int time_start   = 0;
    unsigned int query_num    = 0;

    std::chrono::time_point<std::chrono::steady_clock> time_begin;
    std::chrono::time_point<std::chrono::steady_clock> time_finish;

    std::string send_data;
    std::string error_msg;

    // std::unique_ptr<asio::ip::tcp::socket> socket;
    // std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket;
    // std::shared_ptr<asio::ssl::context> ssl_context;
    // asio::error_code ec;
    // asio::io_context *io_ctx = nullptr;
    std::unique_ptr<orm_conn_link_t> conn_link;
};

}// namespace orm

#endif// _PG_CONN_BASE_H
