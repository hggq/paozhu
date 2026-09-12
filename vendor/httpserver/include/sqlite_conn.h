#ifndef _SQLITE_CONN_BASE_H
#define _SQLITE_CONN_BASE_H
/*
 * @Description: SQLite connection base class
 *
 *   cmake -DENABLE_SQLITE=ON/OFF
 */

#ifndef ASIO_USE_COROUTINES_TS
#define ASIO_USE_COROUTINES_TS
#endif

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <chrono>
#include <optional>
#include <coroutine>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <string_view>

#include <asio.hpp>
#include <asio/io_context.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/async_result.hpp>

#include "orm_common.h"

#ifdef ENABLE_SQLITE
#include <sqlite3.h>
#endif

// SQLite 绑定类型常量 (sqlite3.h 之后定义回退值;
// macOS sqlite3.h 会 undef 已定义的 SQLITE_TEXT, 故必须在 include 之后)
#ifndef SQLITE_INTEGER
#define SQLITE_INTEGER 1
#endif
#ifndef SQLITE_FLOAT
#define SQLITE_FLOAT 2
#endif
#ifndef SQLITE_TEXT
#define SQLITE_TEXT 3
#endif
#ifndef SQLITE_BLOB
#define SQLITE_BLOB 4
#endif
#ifndef SQLITE_NULL
#define SQLITE_NULL 5
#endif

namespace orm
{

// ======================== Worker 线程接口 ========================
// 向指定数据库的 worker 投递任务；worker 已停止时返回 false（任务未投递）
bool sqlite_worker_submit(const std::string &db_path, std::function<void()> task);
void sqlite_init();
void sqlite_shutdown();
void sqlite_worker_shutdown_all_safe();
void sqlite_worker_shutdown(const std::string &db_path);
void sqlite_worker_shutdown_all();
std::string sqlite_db_path_of(const orm_conn_t &conn_config);

class sqlite_worker_t;

// ---- 错误值工具 (ON/OFF 共用) ----
template <typename T>
inline T sqlite_shutdown_error_value()
{
    if constexpr (std::is_same_v<T, bool>)
        return false;
    else if constexpr (std::is_integral_v<T>)
        return static_cast<T>(-1);
    else
        return T{};
}

// fetch_directly 行回调列名缓存类型：同一查询列名不变，首行构建一次后续行复用，
// 避免每行每列构造临时 std::string
using sqlite_conn_col_name_cache_t = std::vector<std::string>;

// SQLite prepared 专用 typed 返回值：替代原来的 (ptr, len) tuple，
// 让 INTEGER/FLOAT/DATETIME 等列直接以原生类型返回，避免双向字符串转换
// col_value_variant 定义在 orm_common.h

#ifdef ENABLE_SQLITE

// ---- 预编译语句缓存 ----
class sqlite_stmt_cache
{
  public:
    explicit sqlite_stmt_cache(size_t max_size = 64) : max_size_(max_size) {}
    ~sqlite_stmt_cache() { this->clear(); }

    sqlite3_stmt *get_or_create(sqlite3 *db, const std::string &sql)
    {
        auto it = cache_.find(sql);
        if (it != cache_.end())
        {
            sqlite3_reset(it->second.first);
            sqlite3_clear_bindings(it->second.first);
            order_.splice(order_.begin(), order_, it->second.second);
            return it->second.first;
        }
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            return nullptr;
        if (stmt == nullptr)
            return nullptr;// 空/纯注释 SQL：prepare 成功但无语句可执行，不缓存 NULL 语句
        if (cache_.size() >= max_size_)
            this->evict_one();
        order_.push_back(sql);
        cache_[sql] = {stmt, std::prev(order_.end())};
        return stmt;
    }

    void clear()
    {
        for (auto &[sql, pair] : cache_)
            if (pair.first)
                sqlite3_finalize(pair.first);
        cache_.clear();
        order_.clear();
    }

    void evict(const std::string &sql)
    {
        auto it = cache_.find(sql);
        if (it == cache_.end())
            return;
        if (it->second.first)
            sqlite3_finalize(it->second.first);
        order_.erase(it->second.second);
        cache_.erase(it);
    }

  private:
    void evict_one()
    {
        if (order_.empty())
            return;
        const std::string &oldest = order_.back();
        auto it                   = cache_.find(oldest);
        if (it != cache_.end() && it->second.first)
            sqlite3_finalize(it->second.first);
        cache_.erase(oldest);
        order_.pop_back();
    }
    size_t max_size_;
    std::unordered_map<std::string, std::pair<sqlite3_stmt *, std::list<std::string>::iterator>> cache_;
    std::list<std::string> order_;
};

// ---- 连接基类 (完整) ----
class sqlite_conn_base
{
  public:
    sqlite_conn_base();
    ~sqlite_conn_base();
    sqlite_conn_base(const sqlite_conn_base &)            = delete;
    sqlite_conn_base &operator=(const sqlite_conn_base &) = delete;

    bool connect(const orm_conn_t &conn_config);
    bool close();
    bool ping();
    bool is_closed();

    int exec_sql(const std::string &sql);

    /**
     * @brief 执行 DML 语句 (INSERT/UPDATE/DELETE) 并返回影响行数
     * @param sql SQL语句
     * @return 影响的行数，失败返回 (unsigned int)-1
     */
    unsigned int exec_dml(const std::string &sql);

    /**
     * @brief 获取本连接最近一次 INSERT 产生的自增主键 (rowid)
     * @return rowid，未连接或无插入时返回 0
     */
    long long last_insert_rowid();

    bool query_fetch(const std::string &sql, sqlite_query_result &result);
    bool query_scalar(const std::string &sql, sqlite_scalar_result &result);

    /**
     * @brief 直接从数据库获取数据并通过回调处理，避免中间态拷贝
     * @param sql SQL查询语句
     * @param handler 回调函数，签名为 bool handler(int col_count, char** col_names, std::function<std::tuple<unsigned char*, size_t>(int)> get_data)
     *                col_count: 列数
     *                col_names: 列名数组
     *                get_data: 获取指定列原始数据的函数，返回 (指针, 长度)，若为 NULL 则指针为 nullptr
     *                生命周期: 返回的指针只在本次 handler 调用期间有效，必须在下次取列前拷贝走；
     *                          不得把 (指针, 长度) 存起来跨列或跨行再回读（REAL 列共用本连接的单格还原缓冲）
     *                          handler 内不得再用同一连接发起其它查询，否则该格会被后一次查询覆写
     *                返回值: true 继续下一行，false 中止查询
     * @return 处理的行数
     */
    template <typename RowHandler>
    unsigned int fetch_directly(const std::string &sql, RowHandler handler);

    int exec_batch(const std::vector<std::string> &sqls);
    bool exec_bound(const std::string &sql, const std::vector<sqlite_bind_param> &params);

    /**
     * @brief 开启事务。单连接模式下同一底层连接同时只能有一个事务，
     *        若本连接引擎已有未提交事务(含其它调用方经裸 BEGIN 开启的)则返回 false
     * @return 成功 true；连接已关闭 / 已有事务进行中 false
     */
    bool begin_transaction();

    /**
     * @brief 提交事务。提交失败且事务仍在引擎层打开时不会归还事务权，
     *        调用方应随后 rollback 收尾
     * @return 成功 true；连接已关闭 / 无活动事务 / 提交失败 false
     */
    bool commit_transaction();

    /**
     * @brief 回滚事务
     * @return 成功 true；连接已关闭 / 无活动事务 / 回滚失败 false
     */
    bool rollback_transaction();

    // ===== 预编译语句（Prepared Statements）接口 =====
    template <typename RowHandler>
    unsigned int fetch_prepared(const std::string &sql,
                                const std::vector<http::obj_val> &params,
                                RowHandler handler);

    unsigned int exec_dml_prepared(const std::string &sql,
                                   const std::vector<http::obj_val> &params);

    // 预编译异步版
    template <typename RowHandler>
    asio::awaitable<unsigned int> async_fetch_prepared(const std::string &sql,
                                                       const std::vector<http::obj_val> &params,
                                                       RowHandler handler);
    asio::awaitable<unsigned int> async_exec_dml_prepared(const std::string &sql,
                                                          const std::vector<http::obj_val> &params);

    // ===== SQLite prepared 二进制版：driver 返回 typed variant，
    //       INTEGER/FLOAT 等列直接以原生类型返回，避免双向字符串转换 =====
    template <typename RowHandler>
    unsigned int fetch_prepared_binary(const std::string &sql,
                                       const std::vector<http::obj_val> &params,
                                       RowHandler handler);

    template <typename RowHandler>
    asio::awaitable<unsigned int> async_fetch_prepared_binary(const std::string &sql,
                                                              const std::vector<http::obj_val> &params,
                                                              RowHandler handler);

    std::vector<std::string> get_table_list();
    std::vector<std::map<std::string, std::string>> get_table_info(const std::string &table);
    // PRAGMA index_list / index_info — 用于 modelfun.hpp 生成 col_indexed 数组
    std::vector<std::string> get_index_list(const std::string &table);
    std::vector<std::string> get_index_info(const std::string &index_name);

    asio::awaitable<bool> async_connect(const orm_conn_t &conn_config);
    asio::awaitable<bool> async_close();
    asio::awaitable<bool> async_ping();
    asio::awaitable<int> async_exec_sql(const std::string &sql);

    /**
     * @brief exec_dml 的异步版本
     */
    asio::awaitable<unsigned int> async_exec_dml(const std::string &sql);

    asio::awaitable<bool> async_begin_transaction();
    asio::awaitable<bool> async_commit_transaction();
    asio::awaitable<bool> async_rollback_transaction();

    /**
     * @brief last_insert_rowid 的异步版本
     */
    asio::awaitable<long long> async_last_insert_rowid();

    asio::awaitable<bool> async_query_fetch(const std::string &sql, sqlite_query_result &result);
    asio::awaitable<bool> async_query_scalar(const std::string &sql, sqlite_scalar_result &result);

    /**
     * @brief fetch_directly 的异步版本
     */
    template <typename RowHandler>
    asio::awaitable<unsigned int> async_fetch_directly(const std::string &sql, RowHandler handler);

    void start_worker_thread(asio::io_context &ioc, const std::string &db_path);
    void stop_worker_thread();
    bool is_worker_running() const { return worker_running_.load(); }

    bool integrity_check(std::string &report);
    bool repair_database();
    bool reconnect(int max_retries = 3);
    bool rollback_unfinished_transactions();
    bool in_transaction() const { return in_transaction_.load(); }

    void begin_time();
    void finish_time();
    long long count_time();

    sqlite3 *get_db() const { return db_; }
    const std::string &db_file_path() const { return db_file_path_; }
    static const char *sqlite_version();

  public:
    std::string error_msg;
    bool isclose            = true;// 与 isclose_ 同步，兼容 pg/mysql 的 isclose 成员变量
    bool isdebug            = false;
    unsigned int time_start = 0;
    unsigned int query_num  = 0;
    std::chrono::time_point<std::chrono::steady_clock> time_begin;
    std::chrono::time_point<std::chrono::steady_clock> time_finish;

  private:
    template <typename T>
    T submit_sync(std::function<T()> fn);
    template <typename T>
    asio::awaitable<T> run_on_worker(std::function<T()> fn);

    bool connect_impl(const orm_conn_t &conn_config);
    bool close_impl();
    bool ping_impl();
    int exec_sql_impl(const std::string &sql);
    unsigned int exec_dml_impl(const std::string &sql);
    long long last_insert_rowid_impl();
    bool query_fetch_impl(const std::string &sql, sqlite_query_result &result);
    bool query_scalar_impl(const std::string &sql, sqlite_scalar_result &result);

    /**
     * @brief fetch_directly 的内部实现，直接操作 sqlite3_stmt
     */
    unsigned int fetch_directly_impl(const std::string &sql,
                                     std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);

    bool exec_bound_impl(const std::string &sql, const std::vector<sqlite_bind_param> &params);

    unsigned int fetch_prepared_impl(
        const std::string &sql,
        const std::vector<http::obj_val> &params,
        std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler);

    // SQLite prepared 二进制版：内部 get_data 返回 col_value_variant（int64_t/double/string_view/monostate）
    // 只给 SQLite driver 用，MySQL/PG 继续走 fetch_prepared_impl（字符串协议）
    unsigned int fetch_prepared_impl_binary(
        const std::string &sql,
        const std::vector<http::obj_val> &params,
        std::function<bool(int, char **, std::function<col_value_variant(int)>)> handler);

    unsigned int exec_dml_prepared_impl(const std::string &sql,
                                        const std::vector<http::obj_val> &params);

    bool integrity_check_impl(std::string &report);
    bool repair_database_impl();
    bool reconnect_impl(int max_retries);
    bool rollback_unfinished_impl();
    bool check_and_attempt_recovery(int error_code);
    bool begin_transaction_impl();
    bool commit_transaction_impl();
    bool rollback_transaction_impl();
    void set_error(const std::string &msg);
    void clear_error();
    // 语句执行失败后以 sqlite3_get_autocommit 校正事务状态；
    // 发现调用前不在事务却遗留未提交事务时尽力回滚，防悬挂事务
    void reconcile_transaction_state_after_error();
    // 语句执行成功后以 sqlite3_get_autocommit 校正事务标记/日志，
    // 并按事务窗口(打开/关闭)同步 txn_lock_，使裸 BEGIN/COMMIT 路径与
    // 事务 API 共享同一把锁，杜绝锁态与引擎真实状态脱节
    void sync_transaction_state_after_exec();
    void drain_worker_tasks();
    void submit_to_worker_cached(std::function<void()> task);

    std::string worker_key_;
    std::atomic<bool> worker_running_{false};
    std::atomic<bool> shutdown_error_{false};
    asio::io_context *io_ctx_ = nullptr;
    std::shared_ptr<sqlite_worker_t> cached_worker_;

    sqlite3 *db_ = nullptr;
    std::string db_file_path_;
    std::atomic<bool> isclose_{true};
    sqlite_stmt_cache stmt_cache_{128};

    // REAL 列精度还原缓冲（%.17g 最长 24 字符，64 留足余量）
    // 每连接一格：单 worker 串行操作，无跨线程竞争
    // 契约：get_data 返回的指针只在本次 handler 调用内有效，必须立即拷贝走
    char float_buf_[64];

    std::atomic<bool> in_transaction_{false};
    // 事务互斥锁：SQLite 单连接模式下多个 db_conn 实例共享同一底层连接，
    // 通过原子锁保证同一时刻只有一个事务拥有者，防止跨实例事务串扰
    // （A 开启事务后 B 再次 begin 将被拒绝，B 的 commit 无法破坏 A 的事务）
    std::atomic_flag txn_lock_ = ATOMIC_FLAG_INIT;
    std::atomic<int> consecutive_errors_{0};
    // 恢复流程进行中标记：integrity_check_impl 内部查询失败会再次进入
    // check_and_attempt_recovery，用该标记阻断递归（数据库损坏时防栈溢出）
    std::atomic<bool> recovery_in_progress_{false};
    std::chrono::steady_clock::time_point last_error_time_;
    static constexpr int k_max_reconnect_retries = 3;
    static constexpr int k_reconnect_delay_ms    = 100;
    static constexpr int k_error_threshold       = 5;
    struct transaction_log_t
    {
        std::string sql;
        std::chrono::steady_clock::time_point timestamp;
    };
    std::vector<transaction_log_t> transaction_log_;
    std::mutex error_mutex_;
    orm_conn_t conn_config_;
};

// ---- 模板实现 ----
template <typename T>
inline T sqlite_conn_base::submit_sync(std::function<T()> fn)
{
    if (!worker_running_.load())
    {
        try
        {
            return fn();
        }
        catch (const std::exception &e)
        {
            this->set_error(std::string("exception in sqlite task: ") + e.what());
            return sqlite_shutdown_error_value<T>();
        }
        catch (...)
        {
            this->set_error("unknown exception in sqlite task");
            return sqlite_shutdown_error_value<T>();
        }
    }
    if (shutdown_error_.load())
    {
        this->set_error("sqlite_conn is shutting down");
        return sqlite_shutdown_error_value<T>();
    }

    auto promise = std::make_shared<std::promise<T>>();
    auto future  = promise->get_future();
    submit_to_worker_cached([fn, promise, this]()
                            {
        try { promise->set_value(fn()); }
        catch (const std::exception &e) { this->set_error(std::string("exception in sqlite task: ") + e.what()); promise->set_value(sqlite_shutdown_error_value<T>()); }
        catch (...) { this->set_error("unknown exception in sqlite task"); promise->set_value(sqlite_shutdown_error_value<T>()); } });
    try
    {
        return future.get();
    }
    catch (...)
    {
        return sqlite_shutdown_error_value<T>();
    }
}

template <typename T>
inline asio::awaitable<T> sqlite_conn_base::run_on_worker(std::function<T()> fn)
{
    if (!worker_running_.load())
        co_return fn();
    if (shutdown_error_.load())
    {
        this->set_error("sqlite_conn is shutting down");
        co_return sqlite_shutdown_error_value<T>();
    }

    auto caller_executor = co_await asio::this_coro::executor;
    asio::use_awaitable_t<> token;
    auto awaitable = asio::async_initiate<void(T)>(
        [this, fn, caller_executor](auto handler) mutable
        {
            auto hp = std::make_shared<std::decay_t<decltype(handler)>>(std::move(handler));
            submit_to_worker_cached([fn = std::move(fn), hp, caller_executor, this]() mutable
                                    {
                T value{};
                try { value = fn(); }
                catch (const std::exception& e) { value = sqlite_shutdown_error_value<T>(); this->set_error(std::string("exception in sqlite task: ") + e.what()); }
                catch (...) { value = sqlite_shutdown_error_value<T>(); this->set_error("unknown exception in sqlite task"); }
                asio::post(caller_executor, [hp, value = std::move(value)]() mutable { (*hp)(std::move(value)); }); });
        },
        token);
    co_return co_await std::move(awaitable);
}

template <typename RowHandler>
inline unsigned int sqlite_conn_base::fetch_directly(const std::string &sql, RowHandler handler)
{
    return this->submit_sync<unsigned int>([this, &sql, handler = std::move(handler)]() mutable
                                           {
        std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)> func = std::move(handler);
        return this->fetch_directly_impl(sql, std::move(func)); });
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> sqlite_conn_base::async_fetch_directly(const std::string &sql, RowHandler handler)
{
    return this->run_on_worker<unsigned int>([this, sql, handler = std::move(handler)]() mutable
                                             {
        std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)> func = std::move(handler);
        return this->fetch_directly_impl(sql, std::move(func)); });
}

// ---- 预编译语句内联包装 ----
template <typename RowHandler>
inline unsigned int sqlite_conn_base::fetch_prepared(const std::string &sql,
                                                     const std::vector<http::obj_val> &params,
                                                     RowHandler handler)
{
    return this->submit_sync<unsigned int>([this, &sql, &params, handler = std::move(handler)]() mutable
                                           {
        std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)> func = std::move(handler);
        return this->fetch_prepared_impl(sql, params, std::move(func)); });
}

inline unsigned int sqlite_conn_base::exec_dml_prepared(const std::string &sql,
                                                        const std::vector<http::obj_val> &params)
{
    return this->submit_sync<unsigned int>([this, &sql, &params]()
                                           { return this->exec_dml_prepared_impl(sql, params); });
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> sqlite_conn_base::async_fetch_prepared(
    const std::string &sql, const std::vector<http::obj_val> &params, RowHandler handler)
{
    return this->run_on_worker<unsigned int>([this, sql, params, handler = std::move(handler)]() mutable
                                             {
        std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)> func = std::move(handler);
        return this->fetch_prepared_impl(sql, params, std::move(func)); });
}

// ===== SQLite prepared 二进制版模板 =====
template <typename RowHandler>
inline unsigned int sqlite_conn_base::fetch_prepared_binary(
    const std::string &sql, const std::vector<http::obj_val> &params, RowHandler handler)
{
    return this->submit_sync<unsigned int>([this, &sql, &params, handler = std::move(handler)]() mutable
                                           {
        std::function<bool(int, char**, std::function<col_value_variant(int)>)> func = std::move(handler);
        return this->fetch_prepared_impl_binary(sql, params, std::move(func)); });
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> sqlite_conn_base::async_fetch_prepared_binary(
    const std::string &sql, const std::vector<http::obj_val> &params, RowHandler handler)
{
    return this->run_on_worker<unsigned int>([this, sql, params, handler = std::move(handler)]() mutable
                                             {
        std::function<bool(int, char**, std::function<col_value_variant(int)>)> func = std::move(handler);
        return this->fetch_prepared_impl_binary(sql, params, std::move(func)); });
}

inline asio::awaitable<unsigned int> sqlite_conn_base::async_exec_dml_prepared(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    return this->run_on_worker<unsigned int>([this, sql, params]()
                                             { return this->exec_dml_prepared_impl(sql, params); });
}

#endif// def(ENABLE_SQLITE)

#ifndef ENABLE_SQLITE

class sqlite_conn_base
{
  public:
    sqlite_conn_base();
    ~sqlite_conn_base();
    sqlite_conn_base(const sqlite_conn_base &)            = delete;
    sqlite_conn_base &operator=(const sqlite_conn_base &) = delete;

    bool connect(const orm_conn_t &conn_config);
    bool close();
    bool ping();
    bool is_closed();

    int exec_sql(const std::string &sql);

    /**
     * @brief 执行 DML 语句 (INSERT/UPDATE/DELETE) 并返回影响行数
     * @param sql SQL语句
     * @return 影响的行数，失败返回 (unsigned int)-1
     */
    unsigned int exec_dml(const std::string &sql);

    /**
     * @brief 获取本连接最近一次 INSERT 产生的自增主键 (rowid)
     * @return rowid，未连接或无插入时返回 0
     */
    long long last_insert_rowid();

    bool query_fetch(const std::string &sql, sqlite_query_result &result);
    bool query_scalar(const std::string &sql, sqlite_scalar_result &result);

    /**
     * @brief 直接从数据库获取数据并通过回调处理，避免中间态拷贝
     * @param sql SQL查询语句
     * @param handler 回调函数，签名为 bool handler(int col_count, char** col_names, std::function<std::tuple<unsigned char*, size_t>(int)> get_data)
     *                col_count: 列数
     *                col_names: 列名数组
     *                get_data: 获取指定列原始数据的函数，返回 (指针, 长度)，若为 NULL 则指针为 nullptr
     *                生命周期: 返回的指针只在本次 handler 调用期间有效，必须在下次取列前拷贝走；
     *                          不得把 (指针, 长度) 存起来跨列或跨行再回读（REAL 列共用本连接的单格还原缓冲）
     *                          handler 内不得再用同一连接发起其它查询，否则该格会被后一次查询覆写
     *                返回值: true 继续下一行，false 中止查询
     * @return 处理的行数
     */
    template <typename RowHandler>
    unsigned int fetch_directly(const std::string &sql, RowHandler handler);

    int exec_batch(const std::vector<std::string> &sqls);
    bool exec_bound(const std::string &sql, const std::vector<sqlite_bind_param> &params);

    /**
     * @brief 开启事务。单连接模式下同一底层连接同时只能有一个事务，
     *        若本连接引擎已有未提交事务(含其它调用方经裸 BEGIN 开启的)则返回 false
     * @return 成功 true；连接已关闭 / 已有事务进行中 false
     */
    bool begin_transaction();

    /**
     * @brief 提交事务。提交失败且事务仍在引擎层打开时不会归还事务权，
     *        调用方应随后 rollback 收尾
     * @return 成功 true；连接已关闭 / 无活动事务 / 提交失败 false
     */
    bool commit_transaction();

    /**
     * @brief 回滚事务
     * @return 成功 true；连接已关闭 / 无活动事务 / 回滚失败 false
     */
    bool rollback_transaction();

    // ===== 预编译语句（Prepared Statements）接口 =====
    template <typename RowHandler>
    unsigned int fetch_prepared(const std::string &sql,
                                const std::vector<http::obj_val> &params,
                                RowHandler handler);

    unsigned int exec_dml_prepared(const std::string &sql,
                                   const std::vector<http::obj_val> &params);

    // 预编译异步版
    template <typename RowHandler>
    asio::awaitable<unsigned int> async_fetch_prepared(const std::string &sql,
                                                       const std::vector<http::obj_val> &params,
                                                       RowHandler handler);
    asio::awaitable<unsigned int> async_exec_dml_prepared(const std::string &sql,
                                                          const std::vector<http::obj_val> &params);

    std::vector<std::string> get_table_list();
    std::vector<std::map<std::string, std::string>> get_table_info(const std::string &table);
    // PRAGMA index_list / index_info — 用于 modelfun.hpp 生成 col_indexed 数组
    std::vector<std::string> get_index_list(const std::string &table);
    std::vector<std::string> get_index_info(const std::string &index_name);

    asio::awaitable<bool> async_connect(const orm_conn_t &conn_config);
    asio::awaitable<bool> async_close();
    asio::awaitable<bool> async_ping();
    asio::awaitable<int> async_exec_sql(const std::string &sql);

    /**
     * @brief exec_dml 的异步版本
     */
    asio::awaitable<unsigned int> async_exec_dml(const std::string &sql);

    asio::awaitable<bool> async_begin_transaction();
    asio::awaitable<bool> async_commit_transaction();
    asio::awaitable<bool> async_rollback_transaction();

    /**
     * @brief last_insert_rowid 的异步版本
     */
    asio::awaitable<long long> async_last_insert_rowid();

    asio::awaitable<bool> async_query_fetch(const std::string &sql, sqlite_query_result &result);
    asio::awaitable<bool> async_query_scalar(const std::string &sql, sqlite_scalar_result &result);

    /**
     * @brief fetch_directly 的异步版本
     */
    template <typename RowHandler>
    asio::awaitable<unsigned int> async_fetch_directly(const std::string &sql, RowHandler handler);

    void start_worker_thread(asio::io_context &ioc, const std::string &db_path);
    void stop_worker_thread();
    bool is_worker_running() const { return false; }

    bool integrity_check(std::string &report);
    bool repair_database();
    bool reconnect(int max_retries = 3);
    bool rollback_unfinished_transactions();
    bool in_transaction() const { return false; }

    void begin_time();
    void finish_time();
    long long count_time();

    void *get_db() const { return nullptr; }
    const std::string &db_file_path() const { return db_file_path_; }
    static const char *sqlite_version();

    template <typename T>
    T submit_sync(std::function<T()> fn);
    template <typename T>
    asio::awaitable<T> run_on_worker(std::function<T()> fn);

  public:
    std::string error_msg;
    bool isclose            = true;// 存根永不成功连接，恒为已关闭；与 ON 模式同名成员对齐
    bool isdebug            = false;
    unsigned int time_start = 0;
    unsigned int query_num  = 0;
    std::chrono::time_point<std::chrono::steady_clock> time_begin;
    std::chrono::time_point<std::chrono::steady_clock> time_finish;

  private:
    void set_error(const std::string &msg);
    std::string db_file_path_;
};

// ---- 存根模板实现 (OFF 模式: 直接在当前线程执行) ----
template <typename T>
inline T sqlite_conn_base::submit_sync(std::function<T()> fn)
{
    if (fn)
    {
        try
        {
            return fn();
        }
        catch (...)
        {
        }
    }
    return sqlite_shutdown_error_value<T>();
}

template <typename T>
inline asio::awaitable<T> sqlite_conn_base::run_on_worker(std::function<T()> fn)
{
    if (fn)
    {
        try
        {
            co_return fn();
        }
        catch (...)
        {
        }
    }
    co_return sqlite_shutdown_error_value<T>();
}

template <typename RowHandler>
inline unsigned int sqlite_conn_base::fetch_directly(const std::string &sql, RowHandler handler)
{
    (void)sql;
    (void)handler;
    this->set_error("SQLite support is not compiled. Use -DENABLE_SQLITE=ON when building.");
    return 0;
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> sqlite_conn_base::async_fetch_directly(const std::string &sql, RowHandler handler)
{
    (void)sql;
    (void)handler;
    this->set_error("SQLite support is not compiled. Use -DENABLE_SQLITE=ON when building.");
    co_return 0;
}

template <typename RowHandler>
inline unsigned int sqlite_conn_base::fetch_prepared(const std::string &sql,
                                                     const std::vector<http::obj_val> &params,
                                                     RowHandler handler)
{
    (void)sql;
    (void)params;
    (void)handler;
    this->set_error("SQLite support is not compiled. Use -DENABLE_SQLITE=ON when building.");
    return 0;
}

template <typename RowHandler>
inline asio::awaitable<unsigned int> sqlite_conn_base::async_fetch_prepared(
    const std::string &sql, const std::vector<http::obj_val> &params, RowHandler handler)
{
    (void)sql;
    (void)params;
    (void)handler;
    this->set_error("SQLite support is not compiled. Use -DENABLE_SQLITE=ON when building.");
    co_return 0;
}

#endif// !def(ENABLE_SQLITE)

}// namespace orm

#endif// _SQLITE_CONN_BASE_H
