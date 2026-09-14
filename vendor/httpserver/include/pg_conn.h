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
#include <cstdint>
#include <unordered_map>
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
#define PG_AUTH_SSPI 9
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

    // ===== 预编译语句（Prepared Statements）接口 =====
    // 使用 PostgreSQL Extended Query 协议，全部 text format

    /**
     * @brief 预编译查询（同步）
     */
    template <typename RowHandler>
    unsigned int fetch_prepared(const std::string &sql,
                                const std::vector<http::obj_val> &params,
                                RowHandler handler);

    /**
     * @brief 预编译 DML（同步）
     */
    unsigned int exec_dml_prepared(const std::string &sql,
                                   const std::vector<http::obj_val> &params);

    /**
     * @brief 预编译查询（异步）
     */
    template <typename RowHandler>
    asio::awaitable<unsigned int> async_fetch_prepared(const std::string &sql,
                                                       const std::vector<http::obj_val> &params,
                                                       RowHandler handler);

    /**
     * @brief 预编译查询（同步, binary 协议）— 与 fetch_prepared 并行的新入口
     *
     * 行值有效期（硬约束）：handler 收到的列名数组与 getter 返回的 col_value_variant
     * **只到 handler() 返回为止有效**（文本/numeric/时间列指向连接内部的行缓冲，下一行即复用）。
     * handler 若要留下数据必须当场拷走（如 `s.assign(p, n)`），不得持有 string_view / char*。
     */
    template <typename H>
        requires orm::BinaryRowHandler<H>
    unsigned int fetch_prepared_binary(const std::string &sql,
                                       const std::vector<http::obj_val> &params,
                                       H handler);

    /**
     * @brief 预编译查询（异步, binary 协议）
     *
     * 行值有效期同 fetch_prepared_binary。
     */
    template <typename H>
        requires orm::BinaryRowHandler<H>
    asio::awaitable<unsigned int> async_fetch_prepared_binary(const std::string &sql,
                                                              const std::vector<http::obj_val> &params,
                                                              H handler);

    /**
     * @brief 预编译 DML（异步）
     */
    asio::awaitable<unsigned int> async_exec_dml_prepared(const std::string &sql,
                                                          const std::vector<http::obj_val> &params);

    void clear_error()
    {
        error_msg.clear();
        error_code = 0;
    }

    // 连接层跟踪的事务状态：exec_dml 成功执行 BEGIN 置位，COMMIT/ROLLBACK/END 清除；
    // 连接池归还路径据此判断是否需要自动 ROLLBACK 清理
    bool in_transaction() const { return in_transaction_.load(); }

    // 诊断用：本连接 prepared 结果列元数据缓存的当前条目数（§17.4 判据 4 的容量断言）
    size_t prepared_meta_size() const { return prepared_meta_.size(); }
    uint64_t prepared_hits() const { return prepared_meta_.hits(); }
    uint64_t prepared_misses() const { return prepared_meta_.misses(); }
    double prepared_hit_rate() const { return prepared_meta_.hit_rate(); }
    void prepared_reset_stats() { prepared_meta_.reset_stats(); }
    void prepared_clear_cache() { prepared_meta_.clear_cache(); }

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

    // fetch_directly / exec_dml 的内部实现（与 MySQL 侧同形态：模板包装在头文件，实现在 cpp）
    unsigned int fetch_directly_impl(const std::string &sql,
                                     std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);
    asio::awaitable<unsigned int> async_fetch_directly_impl(const std::string &sql,
                                                            std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);
    unsigned int exec_dml_impl(const std::string &sql);

    // Extended Query 内部实现
    unsigned int fetch_prepared_impl(
        const std::string &sql,
        const std::vector<http::obj_val> &params,
        std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);
    unsigned int exec_dml_prepared_impl(const std::string &sql,
                                        const std::vector<http::obj_val> &params);
    asio::awaitable<unsigned int> async_fetch_prepared_impl(
        const std::string &sql,
        const std::vector<http::obj_val> &params,
        std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);
    asio::awaitable<unsigned int> async_exec_dml_prepared_impl(
        const std::string &sql,
        const std::vector<http::obj_val> &params);

    // ===== PostgreSQL prepared binary (新路径,不干预原版) =====
    unsigned int fetch_prepared_impl_binary(
        const std::string &sql,
        const std::vector<http::obj_val> &params,
        std::function<bool(int, char **, std::function<orm::col_value_variant(int)>)> handler);
    asio::awaitable<unsigned int> async_fetch_prepared_impl_binary(
        const std::string &sql,
        const std::vector<http::obj_val> &params,
        std::function<bool(int, char **, std::function<orm::col_value_variant(int)>)> handler);

    // ===== 连接级 SQL→结果列元数据 + 命名 statement 缓存 =====
    // 终版：完全对齐 MySQL COM_STMT 架构 —— 命名 Parse 持久化 + 热路径跳过 Parse + 跳过 Describe
    struct pg_stmt_meta_t
    {
        std::string stmt_name;             // 命名语句名（如 "pg_42"），服务器端持久化键
        std::vector<uint32_t> col_oids;    // 上一轮 'T' 给的结果列类型 OID（解码真值）
        std::vector<uint16_t> col_fmt;     // 上一轮 'T' 给的逐列格式码（解码真值）
        std::vector<std::string> col_names;// 上一轮解析出的列名（热路径服务器不返回 RowDescription，客户端直接用缓存）
        bool needs_reparse = false;        // DDL 后标记，下次 attempt 强制冷路径 Close+ReParse
        uint64_t lru_seq   = 0;
    };
    // 成员所有权即隔离：只有本连接能查本连接的缓存，不需要全局 id。
    // 同一连接不得被两个线程同时使用（沿用池既有约束），内部无需原子。
    class pg_stmt_meta_cache
    {
      public:
        explicit pg_stmt_meta_cache(size_t cap) : cap_(cap) {}

        // 命中即刷新 lru_seq；hits_/misses_ 仅用于调试验证，非线程安全
        const pg_stmt_meta_t *find(const std::string &sql);

        // upsert: 新 SQL 插入（evicted_out 带出被淘汰条目用于 Close），已存在刷新 lru_seq
        pg_stmt_meta_t &upsert(const std::string &sql, pg_stmt_meta_t *evicted_out = nullptr);

        // erase: 移除条目（removed_out 带出条目用于 Close）
        void erase(const std::string &sql, pg_stmt_meta_t *removed_out = nullptr);

        // 标记需要重 Parse（ALTER/DROP 后 Execute 报 SQLSTATE 时调用）
        void mark_needs_reparse(const std::string &sql);

        // DDL 经文本路径执行成功后，粗暴标记全部 entry 需重 Parse（保守全失效，只影响性能不影响正确性）
        void mark_all_needs_reparse();

        // 生成唯一 stmt_name（"pg_1", "pg_2", ...）
        std::string next_stmt_name();

        // close_all: hard_close 前调用，批量发 Close('S', name)+Sync 让服务器释放所有命名语句
        // closer 参数签名: void(const std::string &stmt_name)
        template <typename Closer>
        void close_all(Closer &&closer)
        {
            for (auto &kv : map_)
            {
                if (!kv.second.stmt_name.empty())
                    closer(kv.second.stmt_name);
            }
        }

        // 纯清空 map（hard_close 收尾时用）
        void clear()
        {
            map_.clear();
            seq_ = 0;
            // 注意：name_counter_ 故意不重置，保持单调递增。
            // 否则复用连接时生成的新语句名（pg_N）会与服务器上仍残留的旧同名语句撞名（42P07 already exists）。
            hits_   = 0;
            misses_ = 0;
        }
        size_t size() const { return map_.size(); }
        uint64_t hits() const { return hits_; }
        uint64_t misses() const { return misses_; }
        double hit_rate() const
        {
            uint64_t tot = hits_ + misses_;
            return tot ? double(hits_) / double(tot) : 0.0;
        }
        void reset_stats()
        {
            hits_   = 0;
            misses_ = 0;
        }
        // 彻底清空 map（用于测试隔离；生产靠 hard_close 自然清理）
        void clear_cache()
        {
            map_.clear();
            seq_ = 0;
            // name_counter_ 故意不重置（同 clear()），保持单调递增以免复用连接撞名。
            hits_   = 0;
            misses_ = 0;
        }

      private:
        size_t cap_;
        uint64_t seq_          = 0;
        uint64_t name_counter_ = 0;
        uint64_t hits_         = 0;
        uint64_t misses_       = 0;
        std::unordered_map<std::string, pg_stmt_meta_t> map_;
    };
    // 128条缓存不知有没有性能问题，不过符合大多数项目了
    pg_stmt_meta_cache prepared_meta_{128};

    // async_drain_until_ready 的同步孪生：同样先消费调用方累积缓冲里的残帧再按批读，
    // 否则主循环已读进缓冲、尚未处理的部分会被裸读越过，导致协议乱序
    void drain_until_ready_sync(std::vector<uint8_t> &accum_buf, unsigned int &consumed);

    // 读掉 Extended Query 收尾 Close+Sync 的响应直到 ReadyForQuery,复用调用方已有的累积缓冲
    asio::awaitable<void> async_drain_until_ready(std::vector<uint8_t> &accum_buf, unsigned int &consumed);

    // E/N 消息载荷解析：提取 'M' 字段写入 error_msg 并置 error_code
    void parse_error_from_payload(const unsigned char *data, unsigned int len);

    // Message building methods
    void build_startup_message(const orm_conn_t &conn_config);
    void build_ssl_request();

    // ===== 断线自动重连支持 =====
    orm_conn_t last_conn_config_;
    bool has_conn_config_                       = false;
    static constexpr unsigned int kMaxReconnect = 1;

    bool is_last_error_reconnectable() const;
    bool try_reconnect();
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
    int backend_pid_   = 0;             // BackendKeyData: 后端进程 PID
    int cancel_secret_ = 0;             // BackendKeyData: 取消密钥
    std::string server_client_encoding_;// ParameterStatus: 服务器声明的 client_encoding
    std::vector<std::string> notices_;  // NoticeResponse: 收集的警告/提示消息

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
    return this->fetch_directly_impl(sql,
                                     std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)>(handler));
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> pg_conn_base::async_fetch_directly(const std::string &sql, RowHandler handler)
{
    auto func = std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)>(handler);
    co_return co_await this->async_fetch_directly_impl(sql, func);
}

// ---- 预编译语句内联实现 ----
template <typename RowHandler>
inline unsigned int pg_conn_base::fetch_prepared(const std::string &sql,
                                                 const std::vector<http::obj_val> &params,
                                                 RowHandler handler)
{
    auto func = std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)>(handler);
    return this->fetch_prepared_impl(sql, params, func);
}

inline unsigned int pg_conn_base::exec_dml_prepared(const std::string &sql,
                                                    const std::vector<http::obj_val> &params)
{
    return this->exec_dml_prepared_impl(sql, params);
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> pg_conn_base::async_fetch_prepared(
    const std::string &sql, const std::vector<http::obj_val> &params, RowHandler handler)
{
    auto func = std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)>(handler);
    co_return co_await this->async_fetch_prepared_impl(sql, params, func);
}

inline asio::awaitable<unsigned int> pg_conn_base::async_exec_dml_prepared(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    co_return co_await this->async_exec_dml_prepared_impl(sql, params);
}

// ===== PostgreSQL prepared binary (inline wrappers) =====
template <typename H>
    requires orm::BinaryRowHandler<H>
inline unsigned int pg_conn_base::fetch_prepared_binary(
    const std::string &sql, const std::vector<http::obj_val> &params, H handler)
{
    auto func = std::function<bool(int, char **, std::function<orm::col_value_variant(int)>)>(handler);
    return this->fetch_prepared_impl_binary(sql, params, func);
}

template <typename H>
    requires orm::BinaryRowHandler<H>
inline asio::awaitable<unsigned int> pg_conn_base::async_fetch_prepared_binary(
    const std::string &sql, const std::vector<http::obj_val> &params, H handler)
{
    auto func = std::function<bool(int, char **, std::function<orm::col_value_variant(int)>)>(handler);
    co_return co_await this->async_fetch_prepared_impl_binary(sql, params, func);
}

}// namespace orm

#endif// _PG_CONN_BASE_H
