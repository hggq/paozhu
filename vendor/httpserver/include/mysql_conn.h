#ifndef _MYSQL_CONN_BASE_H
#define _MYSQL_CONN_BASE_H
/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2025-01-16
 */
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <tuple>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "orm_common.h"
#include "unicode.h"

namespace orm
{

// enum class wq
// {
//     qb =0,
//     bt =1,
//     be,
//     eq,
//     lt,
//     le,
//     in,
//     like,
//     notin,
//     between,
//     isnull,
//     notnull
// };
//ORM STRUCT REFLECT END
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

#define CLIENT_SECURE_CONNECTION CLIENT_RESERVED2// 16    4.1+ The authentication response adopts the format of
                                                 //       [1 byte length][data] (caching_sha2_password is required), as the password may contain 0x00

// ---------- MySQL 8.0+ capability flags ----------
// 刻意不协商 CLIENT_MULTI_RESULTS / CLIENT_PS_MULTI_RESULTS(已定):
// 它们用于"单条语句返回多个结果集"(典型 CALL 存储过程带结果集), 本库取数状态机
// 只支持单结果集 + 终结包, 不协商恰好让服务器对带结果集的 CALL 直接拒绝(1312),
// 避免发回多结果集因不排空/不续读而协议失步。此前的 1312 属刻意行为而非缺陷。
#define CLIENT_PZORM_MYSQL_FLAGS                                                                    \
    (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG |                                  \
     CLIENT_CONNECT_WITH_DB | CLIENT_PROTOCOL_41 | CLIENT_TRANSACTIONS | CLIENT_SECURE_CONNECTION | \
     CLIENT_PLUGIN_AUTH | CLIENT_DEPRECATE_EOF)

#define CLIENT_PZORM_MYSQL_SSL_FLAGS                                                                \
    (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG | CLIENT_SSL |                     \
     CLIENT_CONNECT_WITH_DB | CLIENT_PROTOCOL_41 | CLIENT_TRANSACTIONS | CLIENT_SECURE_CONNECTION | \
     CLIENT_PLUGIN_AUTH | CLIENT_DEPRECATE_EOF)

// ---------- MariaDB 12.3+ capability flags ----------
// 当前值与 MySQL 组相同，但分开定义以便未来 MariaDB 特有 flag 独立演进。
#define CLIENT_PZORM_MARIADB_FLAGS                                                                  \
    (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG |                                  \
     CLIENT_CONNECT_WITH_DB | CLIENT_PROTOCOL_41 | CLIENT_TRANSACTIONS | CLIENT_SECURE_CONNECTION | \
     CLIENT_PLUGIN_AUTH | CLIENT_DEPRECATE_EOF)

#define CLIENT_PZORM_MARIADB_SSL_FLAGS                                                              \
    (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG | CLIENT_SSL |                     \
     CLIENT_CONNECT_WITH_DB | CLIENT_PROTOCOL_41 | CLIENT_TRANSACTIONS | CLIENT_SECURE_CONNECTION | \
     CLIENT_PLUGIN_AUTH | CLIENT_DEPRECATE_EOF)

// 握手必须宣告 CLIENT_DEPRECATE_EOF：结果集终结包(OK-via-EOF)与旧协议的列段分隔包(经典 EOF)
// 首字节同为 254、载荷都 < 8 字节，解析器只有靠这一位才能无歧义区分两者；连接期还会要求
// 服务器也 advertise 该位，否则拒绝连接(见 connect / async_connect)。
static_assert((CLIENT_PZORM_MYSQL_FLAGS & CLIENT_DEPRECATE_EOF) != 0,
              "handshake flags must keep CLIENT_DEPRECATE_EOF");
static_assert((CLIENT_PZORM_MYSQL_SSL_FLAGS & CLIENT_DEPRECATE_EOF) != 0,
              "handshake ssl flags must keep CLIENT_DEPRECATE_EOF");
static_assert((CLIENT_PZORM_MARIADB_FLAGS & CLIENT_DEPRECATE_EOF) != 0,
              "mariadb handshake flags must keep CLIENT_DEPRECATE_EOF");
static_assert((CLIENT_PZORM_MARIADB_SSL_FLAGS & CLIENT_DEPRECATE_EOF) != 0,
              "mariadb handshake ssl flags must keep CLIENT_DEPRECATE_EOF");

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

// struct pack_info_t
// {
//     unsigned int length         = 0;
//     unsigned int current_length = 0;
//     unsigned char seq_id        = 0;
//     unsigned char error         = 0;
//     unsigned char padd_length   = 0;
//     unsigned char padd_str[5]   = {0x00};
//     std::string data;
// };

// struct field_info_t
// {
//     std::string org_name;
//     std::string name;
//     std::string org_table;
//     std::string table;
//     std::string dbname;
//     unsigned char field_type = 0;
//     unsigned char decimals = 0;
//     unsigned short charset = 0;
//     unsigned short flags = 0;
//     unsigned int column_length = 0;
// };

// struct orm_conn_t
// {
//     // std::string type;
//     std::string host;
//     std::string port;
//     std::string dbname;
//     std::string user;
//     std::string password;
//     std::string pretable;
//     std::string dbtype;

//     std::string tag;
//     std::string charset;

//     bool isssl                = false;
//     bool issock               = false;
//     bool isdebug              = false;
//     bool islocal              = false;
//     unsigned char link_type   = 0;// 0 edit 1 select 2 backup
//     unsigned char max_pool    = 0;
//     unsigned char min_pool    = 0;
//     unsigned char charset_val = 0;
// };

// struct orm_left_join_t
// {
//     std::string selectsql;
//     std::string join_table;
//     std::string wheresql;
//     std::string ordersql;
//     std::string limitsql;
//     std::string parbysql;
//     std::string subsql;
// };

// struct orm_where_sql_t
// {
//     bool begin_sub;
//     bool end_sub;
//     wq op_type;
//     std::string filed_name;
//     std::string filed_value;
// };

class mysql_conn_base
{
  public:
    mysql_conn_base(std::unique_ptr<orm_conn_link_t> link);
    ~mysql_conn_base();
    void read_server_hello(unsigned int offset, unsigned int length);
    bool server_public_key_encrypt(const std::string &password, unsigned char *data, unsigned int length);
    // Auth Switch Request 支持：按目标插件与新 salt 计算认证应答。
    // 仅支持 caching_sha2_password（32 字节）；禁止实现 mysql_native_password 与
    // sha256_password（MySQL 9 已移除旧插件，MariaDB 12.3+ 支持 caching_sha2_password）。
    // 密码为空返回空串（协议要求零长度应答）；不支持的插件返回空串并置 error_msg。
    std::string compute_auth_response(const std::string &plugin, const std::string &salt, const std::string &password);
    bool connect(const orm_conn_t &conn_config);
    asio::awaitable<bool> async_connect(const orm_conn_t &conn_config);

    void mysqlnd_xor_string(char *dst, const size_t dst_len, const char *xor_str, const size_t xor_str_len);
    unsigned int read_pack(unsigned char *data, unsigned int offset);
    void read_field_pack(unsigned char *data, unsigned int total_num, unsigned int &offset, pack_info_t &pack_info);
    bool pack_eof_check(const pack_info_t &temp_pack_data);
    void read_col_info(const std::string &pack_info, field_info_t &temp_field_data);
    unsigned long long pack_real_num(const unsigned char *data, unsigned int data_len, unsigned int &offset);

    bool ping();
    bool close();
    bool hard_close();
    asio::awaitable<bool> async_close();
    asio::awaitable<unsigned int> async_read_loop();
    unsigned int read_loop();

    unsigned int write();
    asio::awaitable<unsigned int> async_write();

    unsigned int write_sql(const std::string &sql);
    asio::awaitable<unsigned int> async_write_sql(const std::string &sql);

    /**
     * @brief 执行 DML 语句 (INSERT/UPDATE/DELETE) 并返回影响行数
     * @param sql SQL语句
     * @return 影响的行数，失败返回 (unsigned int)-1
     */
    unsigned int exec_dml(const std::string &sql);

    /**
     * @brief exec_dml 的异步版本
     */
    asio::awaitable<unsigned int> async_exec_dml(const std::string &sql);

    /**
     * @brief 直接从数据库获取数据并通过回调处理，避免中间态拷贝
     * @param sql SQL查询语句
     * @param handler 回调函数，签名为 bool handler(int col_count, char** col_names, std::function<std::tuple<unsigned char*, size_t>(int)> get_data)
     *                col_count: 列数
     *                col_names: 列名数组
     *                get_data: 获取指定列原始数据的函数，返回 (指针, 长度)，若为 NULL 则指针为 nullptr
     *                返回值: true 继续下一行，false 中止查询
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
    // MVP: 每次 prepare → execute → close 闭环，不做 LRU 缓存
    // 使用 http::obj_val 传递参数，Conn 层 switch(get_type()) 精确绑定

    /**
     * @brief 预编译查询（同步）
     * @param sql SQL 语句（含 ? 占位符）
     * @param params 参数列表（obj_val 保留类型信息）
     * @param handler 回调，签名同 fetch_directly
     * @return 处理的行数
     */
    template <typename RowHandler>
    unsigned int fetch_prepared(const std::string &sql,
                                const std::vector<http::obj_val> &params,
                                RowHandler handler);

    /**
     * @brief 预编译查询（异步）
     */
    template <typename RowHandler>
    asio::awaitable<unsigned int> async_fetch_prepared(const std::string &sql,
                                                       const std::vector<http::obj_val> &params,
                                                       RowHandler handler);

    // ===== MySQL prepared binary =====
    template <typename H>
        requires BinaryRowHandler<H>
    unsigned int fetch_prepared_binary(const std::string &, const std::vector<http::obj_val> &, H);
    template <typename H>
        requires BinaryRowHandler<H>
    asio::awaitable<unsigned int> async_fetch_prepared_binary(const std::string &, const std::vector<http::obj_val> &, H);

    /**
     * @brief 预编译 DML（同步）
     * @param sql SQL 语句（含 ? 占位符）
     * @param params 参数列表
     * @return 影响行数，失败返回 (unsigned int)-1
     */
    unsigned int exec_dml_prepared(const std::string &sql,
                                   const std::vector<http::obj_val> &params);

    /**
     * @brief 预编译 DML（异步）
     */
    asio::awaitable<unsigned int> async_exec_dml_prepared(const std::string &sql,
                                                          const std::vector<http::obj_val> &params);

    unsigned int last_insert_id() const { return last_insert_id_; }
    void clear_error()
    {
        error_msg.clear();
        error_code = 0;
    }

    // 连接层跟踪的事务状态：exec_dml 成功执行 BEGIN/START TRANSACTION 置位，
    // COMMIT/ROLLBACK 清除；连接池归还路径据此判断是否需要自动 ROLLBACK 清理
    bool in_transaction() const { return in_transaction_.load(); }

    // 最近一次 fetch_prepared / async_fetch_prepared 结果集的"原始(物理)列名"，
    // 下标与 handler 回调收到的列名数组一一对应（长度等于列数）。
    // handler 回调拿到的是显示列名（SELECT id AS myid → "myid"），而上层把结果集
    // 映射回本表字段时需要物理名（"id"）；表达式/聚合列的物理名为空字符串。
    // 语义范围仅限预编译路径：文本路径 fetch_directly 不填本数组，每次预编译
    // 查询开始时清空，因此 0 行/失败结果集不会残留上一次的列名。
    const std::vector<std::string> &prepared_col_org_names() const { return col_org_names_; }

    void reset_data();
    bool is_closed();

    void begin_time();
    void finish_time();
    long long count_time();

  public:
    unsigned char *_cache_data = nullptr;

    bool server_enable_ssl   = false;
    bool isclose             = false;
    bool isdebug             = false;
    std::atomic_bool issynch = false;
    // 连接池在用标志：仅在池锁内读写，防止同一连接重复入池
    bool pooled_ = false;
    // unsigned char sock_type  = 0;
    unsigned char seq_next_id;
    unsigned short error_code    = 0;
    unsigned int last_insert_id_ = 0;
    unsigned int client_flags    = 0;
    unsigned int time_start      = 0;
    unsigned int query_num       = 0;
    std::chrono::time_point<std::chrono::steady_clock> time_begin;
    std::chrono::time_point<std::chrono::steady_clock> time_finish;

    // fetch_directly 行数据缓存
    std::vector<_row_data_cache_ptr_t> row_data_cache_ptrs_;
    std::string send_data;
    std::string error_msg;
    // std::string server_public_key;
    // std::unique_ptr<asio::ip::tcp::socket> socket;
    // std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket;
    // std::unique_ptr<asio::local::stream_protocol::socket> localsocket;
    // std::shared_ptr<asio::ssl::context> ssl_context;
    // asio::error_code ec;
    // asio::io_context *io_ctx = nullptr;
    std::unique_ptr<orm_conn_link_t> conn_link;
    mysql_server_hello_data_t server_hello;
    // 本次认证中服务器下发的 RSA 公钥 PEM（TOFU：首次连接后可取出固化为可信公钥）
    std::string server_pubkey_pem;

  private:
    // ===== per-connection stmt_id LRU 缓存 =====
    // MySQL COM_STMT 协议：stmt_id 在 session 内持久化，重复执行同 SQL 可复用。
    // 下面的列元数据（types/unsigned/names/org_names）只是"上一轮在网上看到的值"：
    // 热路径必须先逐包用本轮 Column Definition 包校验（_definition_matches），全部一致
    // 才允许复用；任何一格不一致就回退冷路径重解析并整体刷新（见 update_meta）。
    // 原因：服务器 ALTER 后会自行 re-prepare，stmt_id 依旧有效 ⇒ 缓存收不到失效信号，
    // 直接拿缓存类型解码会静默出错值。所有格子必须来自同一轮，故不存无法校验的字段。
    struct mysql_stmt_cache_entry_t
    {
        uint32_t stmt_id;
        std::vector<uint8_t> param_types;
        std::vector<uint8_t> param_unsigned;
        std::vector<uint8_t> col_types;
        std::vector<uint8_t> col_unsigned;
        std::vector<uint8_t> col_decimals;// 分数秒位数(DATE/TIME 族打印用), 与 col_types 等长
        std::vector<std::string> col_names;
        std::vector<std::string> col_org_names;
        uint64_t lru_seq = 0;
    };
    class mysql_stmt_cache
    {
      public:
        explicit mysql_stmt_cache(size_t cap) : cap_(cap) {}
        const mysql_stmt_cache_entry_t *find(const std::string &sql);
        // closed_out 带出因本次插入被淘汰条目的 stmt_id（未淘汰则不写）。
        // 缓存层不持有 socket ⇒ COM_STMT_CLOSE 由调用方发出。
        mysql_stmt_cache_entry_t &upsert(const std::string &sql, uint32_t *closed_out = nullptr);
        // 删除条目并带出其 stmt_id：失败路径上服务端语句可能仍存活，调用方须补 close。
        void erase(const std::string &sql, uint32_t *closed_out = nullptr);
        // 用本轮 execute 响应解析出的列元数据覆盖已存在的条目。
        // 要求键已存在 ⇒ 不插入、不淘汰 ⇒ 同轮内已持有的 entry 指针不会失效。
        bool update_meta(const std::string &sql,
                         std::vector<uint8_t> types,
                         std::vector<uint8_t> uns,
                         std::vector<uint8_t> decimals,
                         std::vector<std::string> names,
                         std::vector<std::string> orgs);
        // 清理时对每个 entry 调 closer(stmt_id) 让 server 端同步释放
        void clear(std::function<void(uint32_t)> closer);
        size_t size() const { return map_.size(); }

      private:
        size_t cap_;
        uint64_t seq_ = 0;
        std::unordered_map<std::string, mysql_stmt_cache_entry_t> map_;
    };
    mysql_stmt_cache stmt_cache_{128};

    // 事务状态跟踪（见 in_transaction()）
    std::atomic_bool in_transaction_ = false;

    // fetch_directly / exec_dml 的内部实现
    unsigned int fetch_directly_impl(const std::string &sql,
                                     std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);
    asio::awaitable<unsigned int> async_fetch_directly_impl(const std::string &sql,
                                                            std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);
    unsigned int exec_dml_impl(const std::string &sql);

    // ===== COM_STMT 预编译协议内部实现 =====
    // 返回 stmt_id（0 表示失败），写错误到 error_msg/error_code
    unsigned int stmt_prepare_impl(const std::string &sql);
    // 关闭 stmt（忽略错误，因为是闭环）
    void stmt_close_impl(unsigned int stmt_id);
    // 丢弃一条缓存语句并在句柄可能仍存活时补发 COM_STMT_CLOSE（失败路径不泄漏）
    void stmt_cache_drop(const std::string &sql);
    // async 版：stmt_prepare / stmt_close（使用真正的 async I/O）
    asio::awaitable<unsigned int> async_stmt_prepare(const std::string &sql);
    asio::awaitable<void> async_stmt_close(unsigned int stmt_id);
    // async 版缓存淘汰收口：erase 拿出被丢弃条目后 co_await 真异步关闭（避免协程内阻塞写）
    asio::awaitable<void> async_stmt_cache_drop(const std::string &sql);
    // async 版 hard_close：先收集合并 co_await 遍历关闭缓存 stmt，再走同步 socket 关闭
    asio::awaitable<void> async_hard_close();
    // 执行 SELECT 预编译（内部完整闭环）
    unsigned int fetch_prepared_impl(const std::string &sql,
                                     const std::vector<http::obj_val> &params,
                                     std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);
    asio::awaitable<unsigned int> async_fetch_prepared_impl(
        const std::string &sql,
        const std::vector<http::obj_val> &params,
        std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);
    unsigned int fetch_prepared_impl_binary(const std::string &, const std::vector<http::obj_val> &, std::function<bool(int, char **, std::function<col_value_variant(int)>)>);
    asio::awaitable<unsigned int> async_fetch_prepared_impl_binary(const std::string &, const std::vector<http::obj_val> &, std::function<bool(int, char **, std::function<col_value_variant(int)>)>);
    // 执行 DML 预编译（内部完整闭环）
    unsigned int exec_dml_prepared_impl(const std::string &sql,
                                        const std::vector<http::obj_val> &params);
    asio::awaitable<unsigned int> async_exec_dml_prepared_impl(
        const std::string &sql,
        const std::vector<http::obj_val> &params);

    // COM_STMT 辅助：写 execute packet 的参数部分
    // param_types / param_unsigned 由调用方传入（热路径直接传 cache entry，冷路径传成员变量）
    void write_stmt_execute_params(const std::vector<http::obj_val> &params,
                                   const std::vector<uint8_t> &param_types,
                                   const std::vector<uint8_t> &param_unsigned,
                                   std::vector<unsigned char> &out);

    // stmt_prepare 冷路径缓存：每次 prepare 后更新，热路径不再读这些成员（改读 stmt_cache_ entry）
    // 保留以支持调试和向后兼容
    std::vector<uint8_t> stmt_param_types_;
    std::vector<uint8_t> stmt_param_unsigned_;
    std::vector<uint8_t> stmt_col_types_;
    std::vector<uint8_t> stmt_col_unsigned_;
    std::vector<uint8_t> stmt_col_decimals_;
    // COM_STMT_PREPARE 响应 Column Definition 段一次性提取的列名（冷路径写 cache entry 用）
    std::vector<std::string> stmt_col_names_;    // 显示名（含别名）
    std::vector<std::string> stmt_col_org_names_;// 物理名（底层字段名）

    // fetch_prepared 结果集各列的原始(物理)列名，见 prepared_col_org_names()
    std::vector<std::string> col_org_names_;

    // MySQL 协议辅助解析函数
    unsigned int parse_affected_rows_fast(const unsigned char *data, size_t data_len, unsigned int *insert_id = nullptr);
    void parse_error_packet(const unsigned char *data, unsigned int pkt_len);
    // caching_sha2_password 非 SSL 认证时校验服务器下发的 RSA 公钥（防中间人换公钥骗密码）
    bool check_server_public_key(const orm_conn_t &conn_config, const unsigned char *data, unsigned int length);

    // ===== 断线自动重连支持 =====
    orm_conn_t last_conn_config_;
    bool has_conn_config_                       = false;
    static constexpr unsigned int kMaxReconnect = 1;

    bool is_last_error_reconnectable() const;
    bool try_reconnect();
    asio::awaitable<bool> async_try_reconnect();
};

// ---- exec_dml 内联实现 ----
inline unsigned int mysql_conn_base::exec_dml(const std::string &sql)
{
    return this->exec_dml_impl(sql);
}

// async_exec_dml 在 mysql_conn.cpp 中实现

// ---- 预编译语句内联实现 ----
template <typename RowHandler>
inline unsigned int mysql_conn_base::fetch_directly(const std::string &sql, RowHandler handler)
{
    // 只转发：重连只发生在 *_impl 内部 write 失败的那一处（包没上 wire 才可能重发）。
    // 这里不能按 rows==0 再重连重跑一次——SELECT 返回 0 行和"读中途断连"无法区分，
    // 重跑会把已经交给 handler 的结果再投一遍，也会让带副作用的语句落库两次。
    return this->fetch_directly_impl(sql, std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)>(handler));
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> mysql_conn_base::async_fetch_directly(const std::string &sql, RowHandler handler)
{
    auto func = std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)>(handler);
    return this->async_fetch_directly_impl(sql, func);
}

template <typename RowHandler>
inline unsigned int mysql_conn_base::fetch_prepared(const std::string &sql,
                                                    const std::vector<http::obj_val> &params,
                                                    RowHandler handler)
{
    auto func = std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)>(handler);
    return this->fetch_prepared_impl(sql, params, func);
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> mysql_conn_base::async_fetch_prepared(
    const std::string &sql, const std::vector<http::obj_val> &params, RowHandler handler)
{
    auto func = std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)>(handler);
    return this->async_fetch_prepared_impl(sql, params, func);
}

inline unsigned int mysql_conn_base::exec_dml_prepared(const std::string &sql,
                                                       const std::vector<http::obj_val> &params)
{
    return this->exec_dml_prepared_impl(sql, params);
}
template <typename H>
    requires BinaryRowHandler<H>
inline unsigned int mysql_conn_base::fetch_prepared_binary(const std::string &sql, const std::vector<http::obj_val> &params, H h)
{
    auto func = std::function<bool(int, char **, std::function<col_value_variant(int)>)>(h);
    return this->fetch_prepared_impl_binary(sql, params, func);
}
template <typename H>
    requires BinaryRowHandler<H>
inline asio::awaitable<unsigned int> mysql_conn_base::async_fetch_prepared_binary(const std::string &sql, const std::vector<http::obj_val> &params, H h)
{
    auto func = std::function<bool(int, char **, std::function<col_value_variant(int)>)>(h);
    return this->async_fetch_prepared_impl_binary(sql, params, func);
}

// async_exec_dml_prepared 在 mysql_conn.cpp 中实现

class mysql_charset_store
{
  public:
    void mysql_charset_init();
    void mysql_charset_clear();
    unsigned int mysql_charset_find(const std::string &str);
    ~mysql_charset_store();

  public:
    std::map<std::string, unsigned int> mysql_charset;
};

mysql_charset_store &get_orm_mysql_charset();

}// namespace orm
#endif
