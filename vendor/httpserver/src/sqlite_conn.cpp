/*
 * @Description: SQLite connection full implementation (ENABLE_SQLITE=ON only)
 * @date 2026-08-25
 * 本文件仅在 ENABLE_SQLITE=ON 时参与编译，提供完整的 SQLite 实现。
 */

#include "sqlite_conn.h"

#include <cstdlib>
#include <set>

#ifdef ENABLE_SQLITE

// ======================== ENABLE_SQLITE=ON: 完整实现 ========================

namespace orm
{

// 每个数据库文件一条常驻线程: 串行执行该库所有连接的任务。
class sqlite_worker_t
{
  public:
    sqlite_worker_t()
    {
        std::thread([this]()
                    { this->main_loop(); })
            .swap(thread_);
    }

    ~sqlite_worker_t()
    {
        this->stop_and_drain();
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    bool submit(std::function<void()> task)
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (stop_.load())
        {
            return false;
        }
        queue_.push(std::move(task));
        queue_cv_.notify_one();
        return true;
    }

    void stop_and_drain()
    {
        bool expected = false;
        if (!stop_.compare_exchange_strong(expected, true))
        {
            if (thread_.joinable())
            {
                thread_.join();
            }
            return;
        }

        queue_cv_.notify_all();

        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    bool is_stopped() const { return stop_.load(); }
    bool is_running() const { return !stop_.load(); }

  private:
    void main_loop()
    {
        while (!stop_.load())
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait(lock, [this]()
                               { return !queue_.empty() || stop_.load(); });

                if (stop_.load() && queue_.empty())
                {
                    break;
                }

                if (queue_.empty())
                {
                    continue;
                }

                task = std::move(queue_.front());
                queue_.pop();
            }

            if (task)
            {
                task();
            }
        }
    }

    std::thread thread_;
    std::queue<std::function<void()>> queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> stop_{false};
};

namespace
{
std::mutex sqlite_worker_map_mutex;
std::map<std::string, std::shared_ptr<sqlite_worker_t>> &sqlite_worker_map()
{
    static std::map<std::string, std::shared_ptr<sqlite_worker_t>> workers;
    return workers;
}
}// namespace

void sqlite_worker_submit(const std::string &db_path, std::function<void()> task)
{
    std::shared_ptr<sqlite_worker_t> worker;
    {
        std::lock_guard<std::mutex> lock(sqlite_worker_map_mutex);
        auto &workers = sqlite_worker_map();
        auto iter     = workers.find(db_path);
        if (iter == workers.end())
        {
            iter = workers.emplace(db_path, std::make_shared<sqlite_worker_t>()).first;
        }
        worker = iter->second;
    }
    if (!worker->submit(std::move(task)))
    {
        // worker 已停止意味着程序退出，丢弃任务是合理的（析构/停机期间不应该再投递）
    }
}

void sqlite_worker_shutdown(const std::string &db_path)
{
    std::shared_ptr<sqlite_worker_t> worker;
    {
        std::lock_guard<std::mutex> lock(sqlite_worker_map_mutex);
        auto &workers = sqlite_worker_map();
        auto iter     = workers.find(db_path);
        if (iter != workers.end())
        {
            worker = iter->second;
            workers.erase(iter);
        }
    }
    if (worker)
    {
        worker->stop_and_drain();
    }
}

void sqlite_worker_shutdown_all()
{
    std::map<std::string, std::shared_ptr<sqlite_worker_t>> workers;
    {
        std::lock_guard<std::mutex> lock(sqlite_worker_map_mutex);
        workers = sqlite_worker_map();
        sqlite_worker_map().clear();
    }

    for (auto &[path, worker] : workers)
    {
        if (worker)
        {
            worker->stop_and_drain();
        }
    }
}

std::string sqlite_db_path_of(const orm_conn_t &conn_config)
{
    return conn_config.host.empty() ? conn_config.dbname : conn_config.host;
}

// ======================== 构造 / 析构 ========================

sqlite_conn_base::sqlite_conn_base()
{
}

sqlite_conn_base::~sqlite_conn_base()
{
    if (worker_running_.load())
    {
        this->stop_worker_thread();
        // 必须由析构函数自身负责 close（db_ 是本对象成员，worker 无法替我们关闭）
    }
    // 两条路径最终都要 close db_，这里统一处理
    stmt_cache_.clear();
    if (db_)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

// ======================== 错误处理 ========================

void sqlite_conn_base::set_error(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(error_mutex_);
    error_msg = msg;
}

void sqlite_conn_base::clear_error()
{
    std::lock_guard<std::mutex> lock(error_mutex_);
    error_msg.clear();
}

namespace
{
// 仅检查语句开头的非空白字符，大小写不敏感匹配事务关键字，
// 避免对大 SQL（如批量插入）做整串 toupper 复制，开销降为 O(1)
bool sql_starts_with(const std::string &sql, const char *keyword)
{
    size_t i = sql.find_first_not_of(" \t\n\r");
    if (i == std::string::npos)
    {
        return false;
    }
    for (const char *k = keyword; *k != '\0'; ++k, ++i)
    {
        if (i >= sql.size())
        {
            return false;
        }
        if (std::toupper(static_cast<unsigned char>(sql[i])) != *k)
        {
            return false;
        }
    }
    return true;
}

// 每次打开连接都必须设置的 PRAGMA（WAL 是文件级持久设置，由 connect_impl 去重处理）
void apply_conn_pragmas(sqlite3 *db)
{
    sqlite3_exec(db, "PRAGMA synchronous=NORMAL", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA cache_size=10000", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA temp_store=MEMORY", nullptr, nullptr, nullptr);
    sqlite3_busy_timeout(db, 5000);  // 跨进程/多线程写冲突等待 5 秒重试，立即返回而不立即报 SQLITE_BUSY
}
}// namespace

// ======================== 内部实现 ========================

bool sqlite_conn_base::connect_impl(const orm_conn_t &conn_config)
{
    conn_config_ = conn_config;

    std::string path = sqlite_db_path_of(conn_config);
    if (path.empty())
    {
        this->set_error("SQLite database file path is empty (host/dbname field)");
        return false;
    }

    db_file_path_ = path;

    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK)
    {
        this->set_error(db_ ? sqlite3_errmsg(db_) : sqlite3_errstr(rc));
        if (db_)
        {
            sqlite3_close(db_);
            db_ = nullptr;
        }
        isclose_.store(true);
        return false;
    }

    // PRAGMA 去重
    {
        static std::mutex init_mutex;
        static std::set<std::string> initialized_dbs;
        std::lock_guard<std::mutex> lock(init_mutex);

        if (initialized_dbs.find(path) == initialized_dbs.end())
        {
            sqlite3_exec(db_, "PRAGMA journal_mode=WAL", nullptr, nullptr, nullptr);
            initialized_dbs.insert(path);
        }
    }

    apply_conn_pragmas(db_);

    isclose_.store(false);
    return true;
}

bool sqlite_conn_base::close_impl()
{
    if (in_transaction_.load() && db_)
    {
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        in_transaction_.store(false);
        transaction_log_.clear();
    }

    stmt_cache_.clear();

    if (db_)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }
    isclose_.store(true);
    return true;
}

bool sqlite_conn_base::ping_impl()
{
    if (!db_ || isclose_.load())
    {
        return false;
    }

    static const std::string ping_sql = "SELECT 1";
    sqlite3_stmt *stmt                = stmt_cache_.get_or_create(db_, ping_sql);
    if (!stmt)
    {
        this->set_error(sqlite3_errmsg(db_));
        return false;
    }

    int rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);// 必须 reset，否则常驻读快照钉住 WAL
    if (rc == SQLITE_ROW || rc == SQLITE_DONE)
    {
        return true;
    }
    else
    {
        this->set_error(sqlite3_errmsg(db_));
        return false;
    }
}

int sqlite_conn_base::exec_sql_impl(const std::string &sql)
{
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return -1;
    }

    char *errmsg = nullptr;
    int rc       = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK)
    {
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(errmsg ? errmsg : sqlite3_errmsg(db_));
        sqlite3_free(errmsg);

        this->check_and_attempt_recovery(sqlite_err_code);
        return -1;
    }

    consecutive_errors_.store(0);

    // 检测事务相关语句（前缀匹配，不复制整串）
    if (sql_starts_with(sql, "BEGIN") || sql_starts_with(sql, "SAVEPOINT"))
    {
        in_transaction_.store(true);
        transaction_log_.clear();
    }
    else if (sql_starts_with(sql, "COMMIT") || sql_starts_with(sql, "RELEASE"))
    {
        in_transaction_.store(false);
        transaction_log_.clear();
    }
    else if (sql_starts_with(sql, "ROLLBACK"))
    {
        in_transaction_.store(false);
        transaction_log_.clear();
    }

    return sqlite3_changes(db_);
}

unsigned int sqlite_conn_base::exec_dml_impl(const std::string &sql)
{
    this->clear_error();

    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return static_cast<unsigned int>(-1);
    }

    char *errmsg = nullptr;
    int rc       = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK)
    {
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(errmsg ? errmsg : sqlite3_errmsg(db_));
        sqlite3_free(errmsg);

        this->check_and_attempt_recovery(sqlite_err_code);
        return static_cast<unsigned int>(-1);
    }

    consecutive_errors_.store(0);

    // 检测事务相关语句（前缀匹配，不复制整串）
    if (sql_starts_with(sql, "BEGIN") || sql_starts_with(sql, "SAVEPOINT"))
    {
        in_transaction_.store(true);
        transaction_log_.clear();
    }
    else if (sql_starts_with(sql, "COMMIT") || sql_starts_with(sql, "RELEASE"))
    {
        in_transaction_.store(false);
        transaction_log_.clear();
    }
    else if (sql_starts_with(sql, "ROLLBACK"))
    {
        in_transaction_.store(false);
        transaction_log_.clear();
    }

    return static_cast<unsigned int>(sqlite3_changes(db_));
}

long long sqlite_conn_base::last_insert_rowid_impl()
{
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return 0;
    }
    return static_cast<long long>(sqlite3_last_insert_rowid(db_));
}

bool sqlite_conn_base::query_fetch_impl(const std::string &sql, sqlite_query_result &result)
{
    this->clear_error();

    result.column_names.clear();
    result.rows.clear();
    result.is_null.clear();
    result.affected_rows = 0;

    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return false;
    }

    // 最多两轮：命中 SQLITE_SCHEMA（外部 DDL 变更）则驱逐缓存语句，重新 prepare 后重试一次
    int step_rc = SQLITE_ERROR;
    for (int attempt = 0; attempt < 2; attempt++)
    {
        sqlite3_stmt *stmt = stmt_cache_.get_or_create(db_, sql);
        if (!stmt)
        {
            int sqlite_err_code = sqlite3_errcode(db_);
            this->set_error(sqlite3_errmsg(db_));
            this->check_and_attempt_recovery(sqlite_err_code);
            return false;
        }

        int col_count = sqlite3_column_count(stmt);

        result.column_names.clear();
        result.column_names.reserve(col_count);
        for (int i = 0; i < col_count; i++)
        {
            const char *name = sqlite3_column_name(stmt, i);
            result.column_names.push_back(name ? name : "");
        }

        result.rows.clear();
        result.is_null.clear();
        result.rows.reserve(16);
        result.is_null.reserve(16);

        step_rc = SQLITE_OK;
        while ((step_rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            std::vector<std::string> row(col_count);
            std::vector<bool> nulls(col_count, false);
            for (int i = 0; i < col_count; i++)
            {
                int type = sqlite3_column_type(stmt, i);
                if (type == SQLITE_NULL)
                {
                    nulls[i] = true;
                }
                else if (type == SQLITE_BLOB)
                {
                    const void *blob = sqlite3_column_blob(stmt, i);
                    int len          = sqlite3_column_bytes(stmt, i);
                    if (blob && len > 0)
                    {
                        row[i].assign(static_cast<const char *>(blob), static_cast<size_t>(len));
                    }
                }
                else
                {
                    const char *val = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                    row[i]          = val ? val : "";
                }
            }
            result.rows.push_back(std::move(row));
            result.is_null.push_back(std::move(nulls));
        }

        // 必须 reset：未 reset 的缓存语句会保持 WAL 读事务/快照，
        // 钉住常驻连接后续查询；缓存只在同 SQL 再次命中时才 reset，不够
        sqlite3_reset(stmt);

        if (step_rc == SQLITE_SCHEMA)
        {
            stmt_cache_.evict(sql);
            continue;
        }
        break;
    }

    if (step_rc != SQLITE_DONE && step_rc != SQLITE_ROW)
    {
        // 中途出错（BUSY/CORRUPT 等）：不把部分结果当成功返回
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(sqlite3_errmsg(db_));
        this->check_and_attempt_recovery(sqlite_err_code);
        result.rows.clear();
        result.is_null.clear();
        result.column_names.clear();
        return false;
    }

    result.affected_rows = sqlite3_changes(db_);
    return true;
}

bool sqlite_conn_base::query_scalar_impl(const std::string &sql, sqlite_scalar_result &result)
{
    result.value.clear();
    result.is_null = true;

    sqlite_query_result qr;
    if (!this->query_fetch_impl(sql, qr))
    {
        return false;
    }

    if (!qr.rows.empty() && !qr.rows[0].empty())
    {
        result.value   = qr.rows[0][0];
        result.is_null = qr.is_null.empty() ? false : qr.is_null[0][0];
    }
    return true;
}

bool sqlite_conn_base::exec_bound_impl(const std::string &sql, const std::vector<sqlite_bind_param> &params)
{
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return false;
    }

    // 最多两轮：第一轮命中 SQLITE_SCHEMA（外部 DDL 变更）则驱逐缓存语句，重新 prepare+绑定重试一次
    int rc = SQLITE_ERROR;
    for (int attempt = 0; attempt < 2; attempt++)
    {
        sqlite3_stmt *stmt = stmt_cache_.get_or_create(db_, sql);
        if (!stmt)
        {
            this->set_error(sqlite3_errmsg(db_));
            return false;
        }

        for (size_t i = 0; i < params.size(); i++)
        {
            int idx       = static_cast<int>(i) + 1;
            const auto &p = params[i];

            switch (p.type)
            {
            case SQLITE_INTEGER:
            {
                long long val = std::stoll(p.data);
                sqlite3_bind_int64(stmt, idx, val);
                break;
            }
            case SQLITE_FLOAT:
            {
                double val = std::stod(p.data);
                sqlite3_bind_double(stmt, idx, val);
                break;
            }
            case SQLITE_TEXT:
            {
                sqlite3_bind_text(stmt, idx, p.data.c_str(), -1, SQLITE_TRANSIENT);
                break;
            }
            case SQLITE_BLOB:
            {
                sqlite3_bind_blob(stmt, idx, p.data.c_str(), static_cast<int>(p.data.size()), SQLITE_TRANSIENT);
                break;
            }
            case SQLITE_NULL:
            {
                sqlite3_bind_null(stmt, idx);
                break;
            }
            default:
                sqlite3_bind_null(stmt, idx);
                break;
            }
        }

        rc = sqlite3_step(stmt);
        sqlite3_reset(stmt);// 与其他 step 路径一致，释放读快照

        if (rc == SQLITE_SCHEMA)
        {
            stmt_cache_.evict(sql);
            continue;
        }
        break;
    }

    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(sqlite3_errmsg(db_));
        this->check_and_attempt_recovery(sqlite_err_code);
        return false;
    }

    consecutive_errors_.store(0);
    return true;
}

// ======================== 错误恢复实现 ========================

bool sqlite_conn_base::integrity_check_impl(std::string &report)
{
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return false;
    }

    sqlite_query_result result;
    if (!this->query_fetch_impl("PRAGMA integrity_check", result))
    {
        return false;
    }

    if (!result.rows.empty() && !result.rows[0].empty())
    {
        report = result.rows[0][0];
        return report == "ok";
    }

    report = "empty result";
    return false;
}

bool sqlite_conn_base::repair_database_impl()
{
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return false;
    }

    std::string report;
    if (this->integrity_check_impl(report))
    {
        return true;
    }

    // 尝试 VACUUM 修复
    int rc = sqlite3_exec(db_, "VACUUM", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        this->set_error(sqlite3_errmsg(db_));
        return false;
    }

    // 修复后再次检查
    return this->integrity_check_impl(report);
}

bool sqlite_conn_base::reconnect_impl(int max_retries)
{
    // 1. 先清理预编译语句缓存（所有 stmt 都绑定到当前 db_）
    stmt_cache_.clear();

    if (db_)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }

    isclose_.store(true);

    for (int attempt = 0; attempt < max_retries; attempt++)
    {
        if (attempt > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (1 << attempt)));
        }

        int rc = sqlite3_open(db_file_path_.c_str(), &db_);
        if (rc == SQLITE_OK)
        {
            apply_conn_pragmas(db_);// 重开是新连接，foreign_keys/busy_timeout 等 PRAGMA 必须全部重设
            isclose_.store(false);
            consecutive_errors_.store(0);
            return true;
        }

        if (db_)
        {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    this->set_error("Reconnection failed after " + std::to_string(max_retries) + " attempts");
    return false;
}

bool sqlite_conn_base::rollback_unfinished_impl()
{
    if (!db_ || isclose_.load())
    {
        return true;
    }

    if (!in_transaction_.load())
    {
        return true;
    }

    int rc = sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK)
    {
        in_transaction_.store(false);
        transaction_log_.clear();
        return true;
    }

    this->set_error(sqlite3_errmsg(db_));
    return false;
}

bool sqlite_conn_base::check_and_attempt_recovery(int error_code)
{
    if (error_code != SQLITE_CORRUPT &&
        error_code != SQLITE_NOTADB &&
        error_code != SQLITE_IOERR &&
        error_code != SQLITE_BUSY)
    {
        return false;
    }

    consecutive_errors_.fetch_add(1);

    if (consecutive_errors_.load() >= k_error_threshold)
    {
        std::string report;
        if (!this->integrity_check_impl(report))
        {
            this->repair_database_impl();
        }
        consecutive_errors_.store(0);
        return true;
    }

    return false;
}

// ======================== 事务管理 ========================

bool sqlite_conn_base::begin_transaction_impl()
{
    if (!db_ || isclose_.load())
    {
        return false;
    }

    int rc = sqlite3_exec(db_, "BEGIN", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK)
    {
        in_transaction_.store(true);
        transaction_log_.clear();
        return true;
    }

    this->set_error(sqlite3_errmsg(db_));
    return false;
}

bool sqlite_conn_base::commit_transaction_impl()
{
    if (!db_ || isclose_.load())
    {
        return false;
    }

    int rc = sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK)
    {
        in_transaction_.store(false);
        transaction_log_.clear();
        return true;
    }

    this->set_error(sqlite3_errmsg(db_));
    return false;
}

bool sqlite_conn_base::rollback_transaction_impl()
{
    if (!db_ || isclose_.load())
    {
        return false;
    }

    int rc = sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK)
    {
        in_transaction_.store(false);
        transaction_log_.clear();
        return true;
    }

    this->set_error(sqlite3_errmsg(db_));
    return false;
}

// ======================== 公共同步接口 ========================

bool sqlite_conn_base::connect(const orm_conn_t &conn_config)
{
    return this->submit_sync<bool>([this, &conn_config]()
                                   { return this->connect_impl(conn_config); });
}

bool sqlite_conn_base::close()
{
    return this->submit_sync<bool>([this]()
                                   { return this->close_impl(); });
}

bool sqlite_conn_base::ping()
{
    return this->submit_sync<bool>([this]()
                                   { return this->ping_impl(); });
}

bool sqlite_conn_base::is_closed()
{
    return isclose_.load();
}

int sqlite_conn_base::exec_sql(const std::string &sql)
{
    return this->submit_sync<int>([this, &sql]()
                                  { return this->exec_sql_impl(sql); });
}

unsigned int sqlite_conn_base::exec_dml(const std::string &sql)
{
    return this->submit_sync<unsigned int>([this, &sql]()
                                            { return this->exec_dml_impl(sql); });
}

long long sqlite_conn_base::last_insert_rowid()
{
    return this->submit_sync<long long>([this]()
                                        { return this->last_insert_rowid_impl(); });
}

bool sqlite_conn_base::query_fetch(const std::string &sql, sqlite_query_result &result)
{
    return this->submit_sync<bool>([this, &sql, &result]()
                                   { return this->query_fetch_impl(sql, result); });
}

bool sqlite_conn_base::query_scalar(const std::string &sql, sqlite_scalar_result &result)
{
    return this->submit_sync<bool>([this, &sql, &result]()
                                   { return this->query_scalar_impl(sql, result); });
}

unsigned int sqlite_conn_base::fetch_directly_impl(
    const std::string &sql,
    std::function<bool(int, char**, std::function<std::tuple<unsigned char*, size_t>(int)>)> handler)
{
    this->clear_error();

    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return 0;
    }

    sqlite3_stmt *stmt = stmt_cache_.get_or_create(db_, sql);
    if (!stmt)
    {
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(sqlite3_errmsg(db_));
        this->check_and_attempt_recovery(sqlite_err_code);
        return 0;
    }

    int col_count = sqlite3_column_count(stmt);

    // 列名数组优先用栈上小数组，超限时回退堆分配，避免每次查询 new/delete
    char              *col_names_stack[64];
    std::vector<char *> col_names_heap;
    char              **col_names = col_names_stack;
    if (col_count > static_cast<int>(sizeof(col_names_stack) / sizeof(col_names_stack[0])))
    {
        col_names_heap.resize(static_cast<size_t>(col_count));
        col_names = col_names_heap.data();
    }
    for (int i = 0; i < col_count; i++)
    {
        col_names[i] = const_cast<char *>(sqlite3_column_name(stmt, i));
    }

    // get_data 仅捕获 stmt，提到行循环外避免每行重建
    auto get_data = [stmt](int col_idx) -> std::tuple<unsigned char*, size_t>
    {
        int col_type = sqlite3_column_type(stmt, col_idx);
        if (col_type == SQLITE_NULL)
        {
            return {nullptr, 0};
        }
        else if (col_type == SQLITE_BLOB)
        {
            const void *blob = sqlite3_column_blob(stmt, col_idx);
            int len = sqlite3_column_bytes(stmt, col_idx);
            if (blob && len > 0)
            {
                return {const_cast<unsigned char*>(static_cast<const unsigned char*>(blob)), static_cast<size_t>(len)};
            }
            return {nullptr, 0};
        }
        else
        {
            const char *val = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_idx));
            if (val)
            {
                int len = sqlite3_column_bytes(stmt, col_idx);
                return {const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(val)), static_cast<size_t>(len)};
            }
            return {nullptr, 0};
        }
    };

    unsigned int row_num = 0;
    int          rc      = SQLITE_OK;

    // 复用的语句若仍停在 SQLITE_ROW（上次调用方提前中止），先强制 reset，
    // 避免其保持的旧读事务/快照影响本次查询结果。
    if (sqlite3_stmt_busy(stmt))
    {
        sqlite3_reset(stmt);
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        bool should_continue = handler(col_count, col_names, get_data);
        row_num++;

        if (!should_continue)
        {
            rc = SQLITE_DONE;  // 调用方主动中止，非错误；统一在循环后 reset 释放读事务
            break;
        }
    }

    // 必须 reset：未 reset 的预编译语句会保持 WAL 读事务/快照，
    // 导致常驻连接后续所有查询钉在旧快照上，看不到其他连接的新提交；
    // 即使语句已 step 到 DONE，不 reset 读事务也可能不释放，且下次复用 reset 时快照会回退到 prepare 时刻。
    // reset 不会丢失错误信息（errcode 仍可从 db_ 读取）。
    sqlite3_reset(stmt);

    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        // step 中途出错（BUSY/CONSTRAINT 等），上报并触发恢复机制，不能当正常结束吞掉
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(sqlite3_errmsg(db_));
        this->check_and_attempt_recovery(sqlite_err_code);
        if (rc == SQLITE_SCHEMA)
        {
            // handler 已交付过行（有副作用），不能调用内重试；驱逐缓存语句，下次调用重新 prepare
            stmt_cache_.evict(sql);
        }
    }
    else
    {
        consecutive_errors_.store(0);
    }

    return row_num;
}

int sqlite_conn_base::exec_batch(const std::vector<std::string> &sqls)
{
    return this->submit_sync<int>([this, &sqls]()
                                  {
        if (!db_ || isclose_.load())
        {
            this->set_error("Database not connected");
            return -1;
        }

        // 开启事务
        if (sqlite3_exec(db_, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            this->set_error(sqlite3_errmsg(db_));
            return -1;
        }

        for (auto &sql : sqls)
        {
            if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
            {
                std::string err = sqlite3_errmsg(db_);
                sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
                this->set_error(err);
                return -1;
            }
        }

        if (sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            std::string err = sqlite3_errmsg(db_);
            sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
            this->set_error(err);
            return -1;
        }

        return 0; });
}

bool sqlite_conn_base::exec_bound(const std::string &sql, const std::vector<sqlite_bind_param> &params)
{
    return this->submit_sync<bool>([this, &sql, &params]()
                                   { return this->exec_bound_impl(sql, params); });
}

std::vector<std::string> sqlite_conn_base::get_table_list()
{
    return this->submit_sync<std::vector<std::string>>([this]()
                                                       {
        std::vector<std::string> tables;
        sqlite_query_result qr;
        if (this->query_fetch_impl(
                "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name", qr))
        {
            for (auto &r : qr.rows)
            {
                if (!r.empty())
                {
                    tables.push_back(r[0]);
                }
            }
        }
        return tables; });
}

std::vector<std::map<std::string, std::string>> sqlite_conn_base::get_table_info(const std::string &table)
{
    return this->submit_sync<std::vector<std::map<std::string, std::string>>>([this, &table]()
                                                                              {
        std::vector<std::map<std::string, std::string>> columns;

        std::string quoted;
        quoted.reserve(table.size() + 2);
        quoted.push_back('"');
        for (char c : table)
        {
            if (c == '"')
                quoted += "\"\"";
            else
                quoted += c;
        }
        quoted.push_back('"');

        sqlite_query_result qr;
        if (this->query_fetch_impl("PRAGMA table_info(" + quoted + ")", qr))
        {
            for (auto &r : qr.rows)
            {
                if (r.size() >= 6)
                {
                    std::map<std::string, std::string> col;
                    col["cid"]        = r[0];
                    col["name"]       = r[1];
                    col["type"]       = r[2];
                    col["notnull"]    = r[3];
                    col["dflt_value"] = r[4];
                    col["pk"]         = r[5];
                    columns.push_back(std::move(col));
                }
            }
        }
        return columns; });
}

// ======================== 协程接口 ========================

asio::awaitable<bool> sqlite_conn_base::async_connect(const orm_conn_t &conn_config)
{
    co_return co_await this->run_on_worker<bool>([this, &conn_config]()
                                                 { return this->connect_impl(conn_config); });
}

asio::awaitable<bool> sqlite_conn_base::async_close()
{
    co_return co_await this->run_on_worker<bool>([this]()
                                                 { return this->close_impl(); });
}

asio::awaitable<bool> sqlite_conn_base::async_ping()
{
    co_return co_await this->run_on_worker<bool>([this]()
                                                 { return this->ping_impl(); });
}

asio::awaitable<int> sqlite_conn_base::async_exec_sql(const std::string &sql)
{
    co_return co_await this->run_on_worker<int>([this, &sql]()
                                                { return this->exec_sql_impl(sql); });
}

asio::awaitable<unsigned int> sqlite_conn_base::async_exec_dml(const std::string &sql)
{
    co_return co_await this->run_on_worker<unsigned int>([this, &sql]()
                                                         { return this->exec_dml_impl(sql); });
}

asio::awaitable<long long> sqlite_conn_base::async_last_insert_rowid()
{
    co_return co_await this->run_on_worker<long long>([this]()
                                                      { return this->last_insert_rowid_impl(); });
}

asio::awaitable<bool> sqlite_conn_base::async_query_fetch(const std::string &sql, sqlite_query_result &result)
{
    co_return co_await this->run_on_worker<bool>([this, &sql, &result]()
                                                 { return this->query_fetch_impl(sql, result); });
}

asio::awaitable<bool> sqlite_conn_base::async_query_scalar(const std::string &sql, sqlite_scalar_result &result)
{
    co_return co_await this->run_on_worker<bool>([this, &sql, &result]()
                                                 { return this->query_scalar_impl(sql, result); });
}

// ======================== Worker 模式 ========================

void sqlite_conn_base::start_worker_thread(asio::io_context &ioc, const std::string &db_path)
{
    io_ctx_     = &ioc;
    worker_key_ = db_path;

    // 预创建 worker (通过 sqlite_worker_submit 机制)
    std::promise<void> ready_promise;
    auto ready_future = ready_promise.get_future();

    sqlite_worker_submit(db_path, [&ready_promise]()
                         { ready_promise.set_value(); });

    ready_future.get();

    // 缓存 worker 指针以跳过全局锁查表
    {
        std::lock_guard<std::mutex> lock(sqlite_worker_map_mutex);
        auto &workers = sqlite_worker_map();
        auto iter     = workers.find(db_path);
        if (iter != workers.end())
        {
            cached_worker_ = iter->second;
        }
    }

    worker_running_.store(true);
}

void sqlite_conn_base::stop_worker_thread()
{
    if (!worker_running_.load())
    {
        return;
    }

    // 排空本连接的任务
    this->drain_worker_tasks();

    worker_running_.store(false);
    cached_worker_.reset();
}

void sqlite_conn_base::submit_to_worker_cached(std::function<void()> task)
{
    if (cached_worker_ && !cached_worker_->is_stopped())
    {
        if (cached_worker_->submit(std::move(task)))
        {
            return;
        }
    }

    // 缓存失效或提交失败, 回退到全局 map
    sqlite_worker_submit(worker_key_, std::move(task));
}

void sqlite_conn_base::drain_worker_tasks()
{
    auto done     = std::make_shared<std::promise<void>>();
    auto done_fut = done->get_future();
    this->submit_to_worker_cached([done]()
                                  { done->set_value(); });
    done_fut.get();
}

// ======================== 错误恢复公共接口 ========================

bool sqlite_conn_base::integrity_check(std::string &report)
{
    return this->submit_sync<bool>([this, &report]()
                                   { return this->integrity_check_impl(report); });
}

bool sqlite_conn_base::repair_database()
{
    return this->submit_sync<bool>([this]()
                                   { return this->repair_database_impl(); });
}

bool sqlite_conn_base::reconnect(int max_retries)
{
    return this->submit_sync<bool>([this, max_retries]()
                                   { return this->reconnect_impl(max_retries); });
}

bool sqlite_conn_base::rollback_unfinished_transactions()
{
    return this->submit_sync<bool>([this]()
                                   { return this->rollback_unfinished_impl(); });
}

// ======================== Debug / 工具 ========================

void sqlite_conn_base::begin_time()
{
    time_begin = std::chrono::steady_clock::now();
}

void sqlite_conn_base::finish_time()
{
    time_finish = std::chrono::steady_clock::now();
}

long long sqlite_conn_base::count_time()
{
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_finish - time_begin);
    return diff.count();
}

const char *sqlite_conn_base::sqlite_version()
{
    return sqlite3_libversion();
}

// ======================== 全局初始化与关闭 ========================

namespace
{
static std::atomic<bool> g_shutdown_started{false};
static std::atomic<bool> g_initialized{false};
}// namespace

void sqlite_init()
{
    g_initialized.store(true);
}

void sqlite_shutdown()
{
    bool expected = false;
    if (!g_shutdown_started.compare_exchange_strong(expected, true))
    {
        return;
    }

    try
    {
        sqlite_worker_shutdown_all();
    }
    catch (...)
    {
    }
}

void sqlite_worker_shutdown_all_safe()
{
    sqlite_shutdown();
}

}// namespace orm

#endif// ENABLE_SQLITE
