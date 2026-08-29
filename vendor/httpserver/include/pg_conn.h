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
#include <functional>
#include <tuple>
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

// pg_row_data_t 已移至 orm_common.h (PG / SQLite 共用)

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

    /**
     * @brief 执行 DML 语句 (INSERT/UPDATE/DELETE) 并返回影响行数，不存储行数据。
     *        入口清陈旧错误；遇 E 消息继续排干到 'Z' 保持连接可用。
     * @param sql SQL语句
     * @return 影响的行数，失败返回 (unsigned int)-1
     */
    unsigned int exec_dml(const std::string &sql);

    /**
     * @brief exec_dml 的异步版本
     */
    asio::awaitable<unsigned int> async_exec_dml(const std::string &sql);

    /**
     * @brief 直接从数据库获取数据并通过回调处理，避免中间态拷贝（零拷贝：
     *        get_data 返回的指针指向累积缓冲内，回调返回前有效）。
     *        签名与 MySQL/SQLite 侧 fetch_directly 完全一致，可复用同一套调用模板。
     * @param sql SQL查询语句
     * @param handler 回调函数，签名为 bool handler(int col_count, char** col_names, std::function<std::tuple<unsigned char*, size_t>(int)> get_data)
     *                col_count: 列数（无 T 消息时为 0）
     *                col_names: 列名数组（org_name）
     *                get_data: 获取指定列原始数据的函数，返回 (指针, 长度)，若为 NULL 则指针为 nullptr
     *                返回值: true 继续下一行，false 中止查询（中止后排干到 'Z' 再返回）
     * @return 处理的行数
     */
    template <typename RowHandler>
    unsigned int fetch_directly(const std::string &sql, RowHandler handler);

    /**
     * @brief fetch_directly 的异步版本
     */
    template <typename RowHandler>
    asio::awaitable<unsigned int> async_fetch_directly(const std::string &sql, RowHandler handler);

    void clear_error() { error_msg.clear(); error_code = 0; }

    // 连接层跟踪的事务状态：exec_dml 成功执行 BEGIN 置位，COMMIT/ROLLBACK/END 清除；
    // 连接池归还路径据此判断是否需要自动 ROLLBACK 清理
    bool in_transaction() const { return in_transaction_.load(); }

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
    // 事务状态跟踪（见 in_transaction()）
    std::atomic_bool in_transaction_ = false;

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

    // fetch_directly / exec_dml 的内部实现（与 MySQL 侧同形态：模板包装在头文件，实现在 cpp）
    unsigned int fetch_directly_impl(const std::string &sql,
                                     std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)> handler);
    asio::awaitable<unsigned int> async_fetch_directly_impl(const std::string &sql,
                                                           std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)> handler);
    unsigned int exec_dml_impl(const std::string &sql);

    // E/N 消息载荷解析：提取 'M' 字段写入 error_msg 并置 error_code
    void parse_error_from_payload(const unsigned char *data, unsigned int len);

    // Message building methods
    void build_startup_message(const orm_conn_t &conn_config);
    void build_ssl_request();

    // ===== 断线自动重连支持 =====
    orm_conn_t    last_conn_config_;
    bool          has_conn_config_ = false;
    static constexpr unsigned int kMaxReconnect = 1;

    bool            is_last_error_reconnectable() const;
    bool            try_reconnect();
    asio::awaitable<bool> async_try_reconnect();

  public:
    // Public member variables (similar to mysql_conn_base)
    unsigned char *_cache_data = nullptr;

    bool server_enable_ssl   = false;
    bool isclose             = false;
    bool isdebug             = false;
    std::atomic_bool issynch = false;
    // 连接池在用标志：仅在池锁内读写，防止同一连接重复入池
    bool pooled_ = false;
    //unsigned char sock_type = 0;  // 0: none, 1: tcp, 2: ssl
    unsigned short error_code = 0;
    unsigned int time_start   = 0;
    unsigned int query_num    = 0;

    std::chrono::time_point<std::chrono::steady_clock> time_begin;
    std::chrono::time_point<std::chrono::steady_clock> time_finish;

    std::string send_data;
    std::string error_msg;

    // 协议级状态 (BackendKeyData / ParameterStatus / NoticeResponse)
    int backend_pid_     = 0;   // BackendKeyData: 后端进程 PID
    int cancel_secret_   = 0;   // BackendKeyData: 取消密钥
    std::string server_client_encoding_;  // ParameterStatus: 服务器声明的 client_encoding
    std::vector<std::string> notices_;     // NoticeResponse: 收集的警告/提示消息

    // std::unique_ptr<asio::ip::tcp::socket> socket;
    // std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket;
    // std::shared_ptr<asio::ssl::context> ssl_context;
    // asio::error_code ec;
    // asio::io_context *io_ctx = nullptr;
    std::unique_ptr<orm_conn_link_t> conn_link;
};

// ---- exec_dml 内联实现 ----
inline unsigned int pg_conn_base::exec_dml(const std::string &sql)
{
    return this->exec_dml_impl(sql);
}

// async_exec_dml 在 pg_conn.cpp 中实现

// ---- 模板实现 ----
template <typename RowHandler>
inline unsigned int pg_conn_base::fetch_directly(const std::string &sql, RowHandler handler)
{
    unsigned int retry = 0;
RETRY_LABEL:
    unsigned int rows = this->fetch_directly_impl(sql, std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)>(handler));

    if (rows == 0 &&
        is_last_error_reconnectable() &&
        retry < kMaxReconnect &&
        try_reconnect())
    {
        retry++;
        goto RETRY_LABEL;
    }
    return rows;
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> pg_conn_base::async_fetch_directly(const std::string &sql, RowHandler handler)
{
    auto func = std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)>(handler);
    unsigned int retry = 0;
RETRY_LABEL:
    unsigned int rows = co_await this->async_fetch_directly_impl(sql, func);

    if (rows == 0 &&
        is_last_error_reconnectable() &&
        retry < kMaxReconnect &&
        co_await async_try_reconnect())
    {
        retry++;
        goto RETRY_LABEL;
    }
    co_return rows;
}

}// namespace orm

#endif// _PG_CONN_BASE_H
