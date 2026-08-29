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
void sqlite_worker_submit(const std::string &db_path, std::function<void()> task);
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
     *                返回值: true 继续下一行，false 中止查询
     * @return 处理的行数
     */
    template <typename RowHandler>
    unsigned int fetch_directly(const std::string &sql, RowHandler handler);

    int exec_batch(const std::vector<std::string> &sqls);
    bool exec_bound(const std::string &sql, const std::vector<sqlite_bind_param> &params);

    std::vector<std::string> get_table_list();
    std::vector<std::map<std::string, std::string>> get_table_info(const std::string &table);

    asio::awaitable<bool> async_connect(const orm_conn_t &conn_config);
    asio::awaitable<bool> async_close();
    asio::awaitable<bool> async_ping();
    asio::awaitable<int> async_exec_sql(const std::string &sql);

    /**
     * @brief exec_dml 的异步版本
     */
    asio::awaitable<unsigned int> async_exec_dml(const std::string &sql);

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
    sqlite_stmt_cache stmt_cache_{64};

    std::atomic<bool> in_transaction_{false};
    std::atomic<int> consecutive_errors_{0};
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
    return this->run_on_worker<unsigned int>([this, &sql, handler = std::move(handler)]() mutable
                                             {
        std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)> func = std::move(handler);
        return this->fetch_directly_impl(sql, std::move(func)); });
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
     *                返回值: true 继续下一行，false 中止查询
     * @return 处理的行数
     */
    template <typename RowHandler>
    unsigned int fetch_directly(const std::string &sql, RowHandler handler);

    int exec_batch(const std::vector<std::string> &sqls);
    bool exec_bound(const std::string &sql, const std::vector<sqlite_bind_param> &params);

    std::vector<std::string> get_table_list();
    std::vector<std::map<std::string, std::string>> get_table_info(const std::string &table);

    asio::awaitable<bool> async_connect(const orm_conn_t &conn_config);
    asio::awaitable<bool> async_close();
    asio::awaitable<bool> async_ping();
    asio::awaitable<int> async_exec_sql(const std::string &sql);

    /**
     * @brief exec_dml 的异步版本
     */
    asio::awaitable<unsigned int> async_exec_dml(const std::string &sql);

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

#endif// !def(ENABLE_SQLITE)

}// namespace orm

#endif// _SQLITE_CONN_BASE_H
