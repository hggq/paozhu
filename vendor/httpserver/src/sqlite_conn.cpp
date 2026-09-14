/*
 * @Description: SQLite connection full implementation (ENABLE_SQLITE=ON only)
 * @date 2026-08-25
 * 本文件仅在 ENABLE_SQLITE=ON 时参与编译，提供完整的 SQLite 实现。
 */

#include "sqlite_conn.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <set>
#include <stdexcept>

#ifdef ENABLE_SQLITE

// ENABLE_SQLITE=ON: man content

namespace orm
{

// 每个数据库文件一条常驻线程: 串行执行该库所有连接的任务。
// One persistent thread per database file: serially execute tasks from all connections to that database.
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

// 进程级初始化/关闭标记。定义在文件顶部而非尾部，是为了让
// sqlite_worker_submit 也能读取关闭标记（见下方提交守卫）。
std::atomic<bool> g_shutdown_started{false};
std::atomic<bool> g_initialized{false};
}// namespace

bool sqlite_worker_submit(const std::string &db_path, std::function<void()> task)
{
    // 关闭守卫：已进入全局关闭流程后，不再按旧路径重新创建 worker——
    // 否则 sqlite_shutdown 之后迟到的 submit 会拉出无人 join 的常驻线程，
    // 进程退出阶段出现僵尸线程。此时直接返回 false，由
    // submit_to_worker_cached 回退到当前线程兜底执行。
    if (g_shutdown_started.load(std::memory_order_acquire))
    {
        return false;
    }

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
        // worker 已停止：任务未投递，由调用方决定（通常立即在调用线程执行，
        // 避免 submit_sync 的 future 永久阻塞，见 submit_to_worker_cached）
        return false;
    }
    return true;
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

// ---- 句柄收尾 / 存活探测工具（先于析构函数定义） ----
// 统一收尾句柄：sqlite3_close 失败（典型：仍有未 finalize 的语句且非 v2 语义）
// 时自动退回 sqlite3_close_v2 延迟回收；两者都失败才返回错误码。
// 无论结果如何都置空句柄，防止后续误用悬垂指针。
int close_db_handle(sqlite3 *&db)
{
    if (!db)
    {
        return SQLITE_OK;
    }
    int rc = sqlite3_close(db);
    if (rc != SQLITE_OK)
    {
        rc = sqlite3_close_v2(db);
    }
    db = nullptr;
    return rc;
}

// 新打开连接的存活探测：sqlite3_open 不读取文件头，对非 SQLite 文件/
// 已损坏文件会"假成功"；只有执行首条语句时才暴露 SQLITE_NOTADB /
// SQLITE_CORRUPT / SQLITE_IOERR 等致命错误。探测失败返回 false 并填出错误。
bool probe_db_liveness(sqlite3 *db, std::string &out_err)
{
    if (!db)
    {
        out_err = "database handle is null";
        return false;
    }
    sqlite3_stmt *stmt = nullptr;
    int rc             = sqlite3_prepare_v2(db, "SELECT 1", -1, &stmt, nullptr);
    if (rc == SQLITE_OK && stmt)
    {
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc == SQLITE_ROW || rc == SQLITE_DONE)
        {
            return true;
        }
    }
    if (db)
    {
        out_err = sqlite3_errmsg(db);
    }
    return false;
}

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
    // 两条路径最终都要 close db_，这里统一处理；
    // close_db_handle 会在 sqlite3_close 失败时自动退回 close_v2 并置空句柄
    stmt_cache_.clear();
    close_db_handle(db_);
}

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
// 每次打开连接都必须设置的 PRAGMA（WAL 是文件级持久设置，由 connect_impl 去重处理）
void apply_conn_pragmas(sqlite3 *db)
{
    sqlite3_exec(db, "PRAGMA synchronous=NORMAL", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA cache_size=10000", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA temp_store=MEMORY", nullptr, nullptr, nullptr);
    sqlite3_busy_timeout(db, 5000);// 跨进程/多线程写冲突等待 5 秒重试，立即返回而不立即报 SQLITE_BUSY
}

// PRAGMA 单行单值结果回调：把首行首列文本写入 user data（用于回读 journal_mode 等）
int capture_pragma_text_cb(void *data, int col_cnt, char **values, char **)
{
    if (col_cnt > 0 && values && values[0])
    {
        *static_cast<std::string *>(data) = values[0];
    }
    return 0;
}

// 判断输入是否不含任何可执行语句：空串 / 仅空白 / 仅注释（含未闭合块注释）。
// sqlite3_prepare/sqlite3_exec 对这类输入返回 SQLITE_OK 且无语句可执行，
// 若不做前置识别，执行/缓存路径会把它误当成 prepare 失败或返回陈旧结果。
bool sql_has_no_statement(const std::string &sql)
{
    size_t i       = 0;
    const size_t n = sql.size();
    while (i < n)
    {
        char c = sql[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f')
        {
            ++i;
            continue;
        }
        if (sql.compare(i, 2, "--") == 0)
        {
            i += 2;
            while (i < n && sql[i] != '\n')
            {
                ++i;
            }
            continue;
        }
        if (sql.compare(i, 2, "/*") == 0)
        {
            size_t end = sql.find("*/", i + 2);
            if (end == std::string::npos)
            {
                break;// 未闭合块注释：引擎按无语句处理
            }
            i = end + 2;
            continue;
        }
        if (c == ';')
        {
            ++i;
            continue;
        }
        return false;// 遇到真实语句起始符
    }
    return true;
}
}// namespace

// ---- 参数绑定工具：前向声明（定义见本文件后部，供 exec_bound_impl 提前调用） ----
static int _bind_params(sqlite3_stmt *stmt, const std::vector<sqlite_bind_param> &params);
static std::string _bind_error_msg(int rc, size_t given_count, size_t placeholder_count);

bool sqlite_conn_base::connect_impl(const orm_conn_t &conn_config)
{
    this->clear_error();
    conn_config_ = conn_config;

    std::string path = sqlite_db_path_of(conn_config);
    if (path.empty())
    {
        this->set_error("SQLite database file path is empty (host/dbname field)");
        return false;
    }

    // 重复 connect 防护：若上次 connect 后未 close（或中途失败残留半开句柄），
    // 先完整走一遍 close 流程（回滚未提交事务、归还事务权、释放语句缓存），
    // 再打开新句柄，避免句柄泄漏与新老连接的事务状态串扰。
    if (db_ || !isclose_.load())
    {
        this->close_impl();
    }

    db_file_path_ = path;

    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK)
    {
        this->set_error(db_ ? sqlite3_errmsg(db_) : sqlite3_errstr(rc));
        close_db_handle(db_);
        isclose_.store(true);
        isclose = true;
        return false;
    }

    // 存活探测：sqlite3_open 不读文件头，对"非 SQLite 文件 / 文件头损坏 /
    // 权限不足"的路径只会在首条语句时报 NOTADB/CORRUPT/IOERR。
    // 这里立刻暴露这类连接失败，不让半死连接进入服务状态。
    {
        std::string liveness_err;
        if (!probe_db_liveness(db_, liveness_err))
        {
            this->set_error(liveness_err.empty() ? "connection liveness probe failed" : liveness_err);
            close_db_handle(db_);
            isclose_.store(true);
            isclose = true;
            return false;
        }
    }

    // PRAGMA 去重（journal_mode=WAL 为文件级持久设置，进程内每条路径只设一次）
    {
        static std::mutex init_mutex;
        static std::set<std::string> initialized_dbs;
        std::lock_guard<std::mutex> lock(init_mutex);

        if (initialized_dbs.find(path) == initialized_dbs.end())
        {
            char *errmsg = nullptr;
            int exec_rc  = sqlite3_exec(db_, "PRAGMA journal_mode=WAL", nullptr, nullptr, &errmsg);
            if (exec_rc != SQLITE_OK)
            {
                std::string err = errmsg ? errmsg : sqlite3_errmsg(db_);
                sqlite3_free(errmsg);
                // WAL 切换失败视为连接失败：journal 模式差异会带来完全不同的
                // 锁/恢复语义，不允许带着半初始化状态继续对外提供服务
                this->set_error("set WAL journal mode failed: " + err);
                close_db_handle(db_);
                isclose_.store(true);
                isclose = true;
                return false;
            }
            // 极少数文件系统不支持 WAL 时引擎可能不报错但回退为 delete 模式，
            // 回读确认模式真实生效，避免静默降级
            std::string mode;
            sqlite3_exec(db_, "PRAGMA journal_mode", capture_pragma_text_cb, &mode, nullptr);
            if (mode == "delete")
            {
                this->set_error("WAL journal mode unsupported on this database file (silently fell back to delete)");
                close_db_handle(db_);
                isclose_.store(true);
                isclose = true;
                return false;
            }
            initialized_dbs.insert(path);
        }
    }

    apply_conn_pragmas(db_);

    isclose_.store(false);
    isclose = false;
    return true;
}

bool sqlite_conn_base::close_impl()
{
    if (db_)
    {
        // 兜底回滚：以引擎真实状态为准——即便内部事务标记与引擎脱节
        //（标记为 false 但引擎仍有打开事务），也先回滚，避免把调用方
        // 遗留的未提交写入静默丢弃，或让句柄带着写锁关闭
        if (sqlite3_get_autocommit(db_) == 0)
        {
            sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        }

        stmt_cache_.clear();

        int rc = close_db_handle(db_);
        if (rc != SQLITE_OK)
        {
            // 缓存已清空后 close_v2 仍失败属极端情形；记录错误供排查，
            // 不影响"连接已关闭"语义（句柄已置空，不会被误用）
            this->set_error(std::string("sqlite3_close failed: ") + sqlite3_errstr(rc));
        }
    }

    in_transaction_.store(false);
    transaction_log_.clear();
    // 关闭连接时归还事务权，避免残留锁影响后续连接
    txn_lock_.clear(std::memory_order_release);
    isclose_.store(true);
    isclose = true;
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
        consecutive_errors_.store(0);
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
    this->clear_error();

    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return -1;
    }

    // 空串 / 纯注释：无语句可执行，视为成功且未产生任何变更。
    // 若不前置拦截，sqlite3_exec 虽返回 OK，但无法区分"没执行任何东西"，
    // 后续会误报或返回陈旧的影响行数。
    if (sql_has_no_statement(sql))
    {
        consecutive_errors_.store(0);
        return 0;
    }

    // 执行前快照 total_changes，用于判定本次调用是否真的产生了 DML
    sqlite3_int64 total_before = sqlite3_total_changes(db_);

    char *errmsg = nullptr;
    int rc       = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK)
    {
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(errmsg ? errmsg : sqlite3_errmsg(db_));
        sqlite3_free(errmsg);

        this->check_and_attempt_recovery(sqlite_err_code);
        // 失败也可能已改变 autocommit（如多语句批内含 BEGIN/COMMIT），
        // 以引擎实际状态校正事务标记，并在检测到悬挂事务时尽力回滚
        this->reconcile_transaction_state_after_error();
        return -1;
    }

    consecutive_errors_.store(0);

    // 以 sqlite3_get_autocommit 为准同步事务标记与事务锁：
    // 本次调用打开/关闭了事务窗口（裸 BEGIN/COMMIT/ROLLBACK）时，
    // 引擎状态是唯一权威，事务锁必须与之一致，防止跨调用方事务串扰
    this->sync_transaction_state_after_exec();

    // 返回语义修正：仅当本次执行确实产生了 DML（total_changes 有增量）时
    // 才返回影响行数；纯 SELECT / DDL / 事务控制语句返回 0。
    // 旧实现直接返回 sqlite3_changes(db_)，会带出上一次 DML 的陈旧计数。
    if (sqlite3_total_changes(db_) > total_before)
    {
        return sqlite3_changes(db_);
    }
    return 0;
}

unsigned int sqlite_conn_base::exec_dml_impl(const std::string &sql)
{
    this->clear_error();

    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return static_cast<unsigned int>(-1);
    }

    if (sql_has_no_statement(sql))
    {
        consecutive_errors_.store(0);
        return 0;
    }

    sqlite3_int64 total_before = sqlite3_total_changes(db_);

    char *errmsg = nullptr;
    int rc       = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK)
    {
        int sqlite_err_code = sqlite3_errcode(db_);
        this->set_error(errmsg ? errmsg : sqlite3_errmsg(db_));
        sqlite3_free(errmsg);

        this->check_and_attempt_recovery(sqlite_err_code);
        this->reconcile_transaction_state_after_error();
        return static_cast<unsigned int>(-1);
    }

    consecutive_errors_.store(0);
    this->sync_transaction_state_after_exec();

    if (sqlite3_total_changes(db_) > total_before)
    {
        return static_cast<unsigned int>(sqlite3_changes(db_));
    }
    return 0;
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

    // 空串/纯注释：无语句可执行，成功返回空结果（结果集已在函数头清空）。
    // 若不前置拦截，prepare 得到 NULL 语句会被当成失败并误报陈旧错误
    if (sql_has_no_statement(sql))
    {
        consecutive_errors_.store(0);
        return true;
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

        // 列数/列名必须等第一次 sqlite3_step() 之后再取：本连接解析过表 T、另一连接
        // DROP/CREATE 过 T 之后，首次 step 会内部 re-prepare，step 前拿到的列数与列名
        // 都还是旧 schema 的 —— 新表列多变会静默截断首行，列少变会按旧列数越界读、
        // 给首行补出一个假的 NULL 列
        int col_count           = 0;
        const auto capture_cols = [stmt, &result]
        {
            const int n = sqlite3_column_count(stmt);
            result.column_names.clear();
            result.column_names.reserve(static_cast<size_t>(n));
            for (int i = 0; i < n; i++)
            {
                const char *name = sqlite3_column_name(stmt, i);
                result.column_names.push_back(name ? name : "");
            }
            return n;
        };

        result.rows.clear();
        result.is_null.clear();
        result.rows.reserve(16);
        result.is_null.reserve(16);

        step_rc = SQLITE_OK;
        while ((step_rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            if (result.rows.empty())
                col_count = capture_cols();

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
                else if (type == SQLITE_FLOAT)
                {
                    // SQLite 默认只给 15 位有效数字，用 column_double + %.17g 完整还原
                    char buf[64];
                    double d = sqlite3_column_double(stmt, i);
                    int n    = snprintf(buf, sizeof(buf), "%.17g", d);
                    row[i].assign(buf, static_cast<size_t>(n > 0 ? n : 0));
                }
                else
                {
                    // TEXT/INTEGER 统一按 column_bytes 显式长度拷贝：
                    // column_text 返回的 C 字符串遇内嵌 '\0' 会提前截断，
                    // 改为与 BLOB 分支 assign(ptr,len) 相同的长度语义
                    const char *val = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                    if (val)
                    {
                        int len = sqlite3_column_bytes(stmt, i);
                        row[i].assign(val, static_cast<size_t>(len));
                    }
                    else
                    {
                        row[i].clear();
                    }
                }
            }
            result.rows.push_back(std::move(row));
            result.is_null.push_back(std::move(nulls));
        }

        // 0 行结果集没进过循环 ⇒ 补抓一次列名。SQLITE_DONE 之后列数/列名仍完整
        // 但必须在下面的 reset 之前做；PRAGMA/表结构反射的
        // 调用点按名字取列，不能让"零行"把列名一起清掉
        if (step_rc == SQLITE_DONE && result.column_names.empty())
            capture_cols();

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
    // 成功路径统一归零：保证 consecutive_errors_ 严格表达"连续错误"语义，
    // 避免零散瞬态错误在查询型负载下（从不归零）累积触发误恢复
    consecutive_errors_.store(0);
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

    // 空串/纯注释：无语句可执行，直接成功返回，不触发缓存/绑定路径
    if (sql_has_no_statement(sql))
    {
        consecutive_errors_.store(0);
        return true;
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

        int bind_rc = _bind_params(stmt, params);
        if (bind_rc != SQLITE_OK)
        {
            this->set_error(_bind_error_msg(bind_rc, params.size(), static_cast<size_t>(sqlite3_bind_parameter_count(stmt))));
            return false;
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
    // 预编译执行若以事务语句（BEGIN 等）提交到引擎，同样同步事务锁/标记，
    // 使本路径与 exec_sql/exec_dml 的事务窗口追踪保持一致
    this->sync_transaction_state_after_exec();
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

    // 重连前仍处于未提交事务的留痕信息（成功重连时写入 error_msg）
    std::string discarded_txn_note;

    if (db_)
    {
        // 重连前引擎仍有打开事务：这类事务不可能迁移到新连接上，必须显式回滚。
        // 旧实现只依据内部标记 in_transaction_ 判断；当标记与引擎真实状态脱节
        //（标记 false、引擎 autocommit=0）时会无提示地连同未提交写一起丢弃。
        if (sqlite3_get_autocommit(db_) == 0)
        {
            int rb = sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
            if (rb == SQLITE_OK)
            {
                discarded_txn_note = "reconnect: rolled back the uncommitted transaction left on the previous connection";
            }
            else
            {
                discarded_txn_note = "reconnect: failed to roll back the leftover transaction (" +
                                     std::string(sqlite3_errmsg(db_)) + "); it was discarded with the old connection";
            }
        }

        int close_rc = close_db_handle(db_);
        if (close_rc != SQLITE_OK)
        {
            this->set_error(std::string("sqlite3_close during reconnect failed: ") + sqlite3_errstr(close_rc));
        }
    }

    isclose_.store(true);
    isclose = true;
    // 重连后底层连接已更换，必须归还旧连接上的事务权并清空全部事务状态，
    // 否则恢复逻辑会基于陈旧状态误判（事务锁、标记、日志一并重置）
    txn_lock_.clear(std::memory_order_release);
    in_transaction_.store(false);
    transaction_log_.clear();

    for (int attempt = 0; attempt < max_retries; attempt++)
    {
        if (attempt > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(k_reconnect_delay_ms * (1 << attempt)));
        }

        int rc = sqlite3_open(db_file_path_.c_str(), &db_);
        if (rc != SQLITE_OK)
        {
            if (db_)
            {
                this->set_error(sqlite3_errmsg(db_));
            }
            else
            {
                this->set_error(sqlite3_errstr(rc));
            }
            close_db_handle(db_);
            continue;
        }

        // 存活探测：sqlite3_open 对"文件已损坏 / 已被替换为非库文件"仍返回 OK，
        // 只有首条语句才暴露 NOTADB/CORRUPT 等致命错误；在重连循环内即时
        // 判定失败并进入下一轮重试，而不是把半死连接当作重连成功返回
        std::string liveness_err;
        if (!probe_db_liveness(db_, liveness_err))
        {
            this->set_error(liveness_err.empty() ? "reconnect liveness probe failed" : liveness_err);
            close_db_handle(db_);
            continue;
        }

        apply_conn_pragmas(db_);// 重开是新连接，foreign_keys/busy_timeout 等 PRAGMA 必须全部重设
        // journal_mode 为文件级持久设置，首连已置 WAL，重开自动继承，无需重设

        isclose_.store(false);
        isclose = false;
        consecutive_errors_.store(0);
        in_transaction_.store(false);
        transaction_log_.clear();

        // 重连成功后的引擎一致性校验：新连接必须处于 autocommit=1（无事务）。
        // 理论上不应违背；一旦出现就显式回滚并提示，不静默带病运行
        if (sqlite3_get_autocommit(db_) != 1)
        {
            sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
            this->set_error("reconnect: new connection unexpectedly opened inside a transaction (rolled back)");
        }
        else if (!discarded_txn_note.empty())
        {
            this->set_error(discarded_txn_note);
        }
        return true;
    }

    std::string msg = "Reconnection failed after " + std::to_string(max_retries) + " attempts";
    if (!discarded_txn_note.empty())
    {
        msg += " (" + discarded_txn_note + ")";
    }
    this->set_error(msg);
    return false;
}

bool sqlite_conn_base::rollback_unfinished_impl()
{
    if (!db_ || isclose_.load())
    {
        return true;
    }

    // 以引擎真实状态判断是否存在需要回滚的事务，而非依赖内部标记：
    // 标记与引擎脱节时（如事务经裸 BEGIN 打开、标记尚未校正）也能正确发现
    if (sqlite3_get_autocommit(db_) != 0)
    {
        if (in_transaction_.load())
        {
            // 引擎不在事务但标记残留：校正标记并归还事务权，防状态泄漏
            in_transaction_.store(false);
            transaction_log_.clear();
            txn_lock_.clear(std::memory_order_release);
        }
        return true;
    }

    int rc = sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
    // 事务已结束，归还事务权并清空状态（无论引擎是否真正完成了回滚）
    txn_lock_.clear(std::memory_order_release);
    if (rc == SQLITE_OK)
    {
        in_transaction_.store(false);
        transaction_log_.clear();
        return true;
    }

    // 回滚失败：以引擎真实状态校正标记后上报
    in_transaction_.store(sqlite3_get_autocommit(db_) == 0);
    this->set_error(sqlite3_errmsg(db_));
    return false;
}

// 语句执行成功后调用：把事务标记/日志/锁同步到引擎真实状态。
// 裸 BEGIN/COMMIT/ROLLBACK（含多语句批、带注释前缀的 BEGIN）经由 exec_* 到达
// 引擎时同样会打开/关闭事务窗口，本函数让这些路径与事务 API 共享 txn_lock_，
// 杜绝"引擎已在事务中、锁却未持有"的跨调用方串扰
void sqlite_conn_base::sync_transaction_state_after_exec()
{
    if (!db_)
    {
        return;
    }

    bool was_in_txn = in_transaction_.load();
    bool now_in_txn = (sqlite3_get_autocommit(db_) == 0);
    if (now_in_txn == was_in_txn)
    {
        return;
    }

    transaction_log_.clear();
    in_transaction_.store(now_in_txn);

    if (now_in_txn)
    {
        // 本次执行打开了事务窗口：事务已独占底层连接，取锁与引擎状态对齐
        txn_lock_.test_and_set(std::memory_order_acquire);
    }
    else
    {
        // 事务窗口已在本调用内结束（COMMIT/ROLLBACK）：归还事务权
        txn_lock_.clear(std::memory_order_release);
    }
}

void sqlite_conn_base::reconcile_transaction_state_after_error()
{
    if (!db_)
    {
        return;
    }

    bool was_in_txn = in_transaction_.load();
    bool now_in_txn = (sqlite3_get_autocommit(db_) == 0);
    if (now_in_txn == was_in_txn)
    {
        return;
    }

    transaction_log_.clear();

    if (now_in_txn)
    {
        // 调用方此前并不在事务中（was=false），本次失败语句却留下未提交事务
        // （典型：多语句批含 BEGIN、随后语句失败）。尽力回滚，防止悬挂事务
        // 把后续无关写入卷入；回滚失败则保持引擎真实状态，由 close/reconnect 兜底。
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
    }

    // 校正后以引擎真实状态更新标记
    now_in_txn = (sqlite3_get_autocommit(db_) == 0);
    in_transaction_.store(now_in_txn);

    if (now_in_txn)
    {
        // 回滚未成功、事务仍在引擎层打开：事务已实际独占底层连接，取锁对齐
        txn_lock_.test_and_set(std::memory_order_acquire);
    }
    else
    {
        // 悬挂事务已回滚 / 错误语句结束了原有事务窗口：归还事务权
        txn_lock_.clear(std::memory_order_release);
    }
}

bool sqlite_conn_base::check_and_attempt_recovery(int error_code)
{
    // BUSY/LOCKED 属并发瞬态（busy_timeout 内已等待），不累计也不触发整库
    // 体检，交由调用方重试即可；仅对库级损坏类错误进入恢复流程。
    if (error_code != SQLITE_CORRUPT &&
        error_code != SQLITE_NOTADB &&
        error_code != SQLITE_IOERR)
    {
        return false;
    }

    // 防递归：integrity_check_impl 内部同样走 query_fetch_impl，当数据库
    // 确实损坏时体检自身也会失败并再次进入本函数；进行中直接返回避免栈溢出。
    if (recovery_in_progress_.exchange(true))
    {
        return false;
    }
    // 离开恢复流程时归还标记（含内部体检再失败提前返回的所有路径）
    struct recovery_flag_guard_t
    {
        std::atomic<bool> &flag_;
        ~recovery_flag_guard_t() { flag_.store(false); }
    } flag_guard(recovery_in_progress_);

    consecutive_errors_.fetch_add(1);
    if (consecutive_errors_.load() < k_error_threshold)
    {
        return false;
    }

    // 先清零再体检：体检/修复内部查询再失败时不会再次把计数推过阈值造成递归
    consecutive_errors_.store(0);

    std::string report;
    if (!this->integrity_check_impl(report))
    {
        this->repair_database_impl();
    }
    return true;
}

// ======================== 事务管理 ========================

bool sqlite_conn_base::begin_transaction_impl()
{
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return false;
    }

    // 以引擎真实状态前置校验：即使事务锁未被占用，只要引擎已处于事务中
    //（其他实例经裸 BEGIN 开启、sync 后锁已置位），也必须拒绝 begin，
    // 否则 SQLite 会返回 "cannot start a transaction within a transaction"，
    // 而且会让调用方误以为 begin 成功后自己掌握了事务
    if (sqlite3_get_autocommit(db_) == 0)
    {
        this->set_error("begin transaction failed: the connection is already inside a transaction");
        return false;
    }

    // 单连接模式下同一底层连接同时只能有一个事务拥有者：
    // test_and_set 成功(原子置位并返回旧值 false)才取得事务权，
    // 阻止其他 db_conn 实例重复 begin / 意外 commit 破坏本事务
    if (txn_lock_.test_and_set(std::memory_order_acquire))
    {
        this->set_error("begin transaction failed: another db_conn owns the transaction on this shared connection");
        return false;
    }

    int rc = sqlite3_exec(db_, "BEGIN", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK)
    {
        in_transaction_.store(true);
        transaction_log_.clear();
        return true;
    }

    // BEGIN 失败必须归还事务权
    txn_lock_.clear(std::memory_order_release);
    this->set_error(sqlite3_errmsg(db_));
    return false;
}

bool sqlite_conn_base::commit_transaction_impl()
{
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return false;
    }

    int rc = sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
    // SQLite 语义：COMMIT 失败时事务通常仍处于打开状态（可继续 ROLLBACK），
    // 不能无条件归还事务权——否则悬挂事务会静默吸收后续调用方的写入。
    // 因此以引擎真实状态为准决定是否归还锁与清空标记。
    bool still_in_txn = (sqlite3_get_autocommit(db_) == 0);
    if (!still_in_txn)
    {
        // 事务已结束（无论 COMMIT 成功与否）：归还事务权并清空状态
        txn_lock_.clear(std::memory_order_release);
        in_transaction_.store(false);
        transaction_log_.clear();
    }

    if (rc == SQLITE_OK)
    {
        return true;
    }

    if (still_in_txn)
    {
        // 提交失败且事务仍在打开：错误信息明确提示可回滚收尾
        this->set_error(std::string("commit failed, transaction still open (call rollback): ") + sqlite3_errmsg(db_));
    }
    else
    {
        this->set_error(sqlite3_errmsg(db_));
    }
    return false;
}

bool sqlite_conn_base::rollback_transaction_impl()
{
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return false;
    }

    int rc            = sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
    bool still_in_txn = (sqlite3_get_autocommit(db_) == 0);
    if (!still_in_txn)
    {
        // 事务已结束：归还事务权并清空状态
        txn_lock_.clear(std::memory_order_release);
        in_transaction_.store(false);
        transaction_log_.clear();
    }

    if (rc == SQLITE_OK)
    {
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

bool sqlite_conn_base::begin_transaction()
{
    return this->submit_sync<bool>([this]()
                                   { return this->begin_transaction_impl(); });
}

bool sqlite_conn_base::commit_transaction()
{
    return this->submit_sync<bool>([this]()
                                   { return this->commit_transaction_impl(); });
}

bool sqlite_conn_base::rollback_transaction()
{
    return this->submit_sync<bool>([this]()
                                   { return this->rollback_transaction_impl(); });
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
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    this->clear_error();

    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return 0;
    }

    // 空串/纯注释：无语句可执行，返回 0 行，不进入缓存/错误路径
    if (sql_has_no_statement(sql))
    {
        consecutive_errors_.store(0);
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

    // 列数/列名必须等第一次 sqlite3_step() 之后再取：sqlite3_column_name() 的指针在语句被
    // 内部 re-prepare（本连接解析过该表、其它连接 DROP/CREATE 过）时当场失效，step 前抓就会
    // 让首行的列名落在已释放内存上（值仍正确）。栈/数组声明留在循环外，指针要跨行存活。
    int col_count = 0;

    // 列名数组优先用栈上小数组，超限时回退堆分配，避免每次查询 new/delete
    char *col_names_stack[64];
    std::vector<char *> col_names_heap;
    char **col_names = col_names_stack;

    // get_data 捕获 stmt + this（REAL 列精度还原走成员 float_buf_）
    auto get_data = [stmt, this](int col_idx) -> std::tuple<unsigned char *, size_t>
    {
        int col_type = sqlite3_column_type(stmt, col_idx);
        if (col_type == SQLITE_NULL)
        {
            return {nullptr, 0};
        }
        else if (col_type == SQLITE_BLOB)
        {
            const void *blob = sqlite3_column_blob(stmt, col_idx);
            int len          = sqlite3_column_bytes(stmt, col_idx);
            if (blob && len > 0)
            {
                return {const_cast<unsigned char *>(static_cast<const unsigned char *>(blob)), static_cast<size_t>(len)};
            }
            return {nullptr, 0};
        }
        else if (col_type == SQLITE_FLOAT)
        {
            // SQLite 默认只给 15 位有效数字，用 column_double + %.17g 完整还原
            double d = sqlite3_column_double(stmt, col_idx);
            int n    = snprintf(float_buf_, sizeof(float_buf_), "%.17g", d);
            return {reinterpret_cast<unsigned char *>(float_buf_), static_cast<size_t>(n > 0 ? n : 0)};
        }
        else
        {
            const char *val = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_idx));
            if (val)
            {
                int len = sqlite3_column_bytes(stmt, col_idx);
                return {const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(val)), static_cast<size_t>(len)};
            }
            return {nullptr, 0};
        }
    };

    unsigned int row_num = 0;
    int rc               = SQLITE_OK;

    // 说明：此处不再做 stmt_busy 预重置（死代码）——语句全部来自
    // stmt_cache_：命中缓存时 get_or_create 已 reset，新 prepare 必然空闲；
    // 且本连接语句只在单 worker 上串行 step，不存在跨线程复用残留快照的场景。
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        if (row_num == 0)
        {
            col_count = sqlite3_column_count(stmt);
            if (col_count > static_cast<int>(sizeof(col_names_stack) / sizeof(col_names_stack[0])))
            {
                col_names_heap.resize(static_cast<size_t>(col_count));
                col_names = col_names_heap.data();
            }
            for (int i = 0; i < col_count; i++)
            {
                col_names[i] = const_cast<char *>(sqlite3_column_name(stmt, i));
            }
        }
        bool should_continue = handler(col_count, col_names, get_data);
        row_num++;

        if (!should_continue)
        {
            rc = SQLITE_DONE;// 调用方主动中止，非错误；统一在循环后 reset 释放读事务
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
        // 走统一事务入口 begin_transaction_impl()：自动取得 txn_lock_ 并保持
        // in_transaction_ 与引擎 autocommit 一致；若连接当前已处于事务中
        //（其他调用方正在使用底层连接），begin 会返回失败，避免引擎级
        // 嵌套 BEGIN 破坏外层事务（旧实现直接用裸 sqlite3_exec 发 BEGIN）
        if (!this->begin_transaction_impl())
        {
            return -1;
        }

        for (auto &sql : sqls)
        {
            char *errmsg = nullptr;
            int rc       = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
            if (rc != SQLITE_OK)
            {
                std::string err = errmsg ? errmsg : sqlite3_errmsg(db_);
                sqlite3_free(errmsg);
                this->rollback_transaction_impl();
                this->set_error(err);
                return -1;
            }
        }

        if (!this->commit_transaction_impl())
        {
            // COMMIT 失败且事务仍在打开时，尽力回滚收尾；
            // 根因错误已由 commit_transaction_impl 设置，rollback 不覆盖它
            this->rollback_transaction_impl();
            return -1;
        }

        consecutive_errors_.store(0);
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

std::vector<std::string> sqlite_conn_base::get_index_list(const std::string &table)
{
    return this->submit_sync<std::vector<std::string>>([this, &table]()
                                                       {
        std::vector<std::string> names;
        std::string quoted;
        quoted.reserve(table.size() + 2);
        quoted.push_back('"');
        for (char c : table) { if (c == '"') quoted += """"; else quoted += c; }
        quoted.push_back('"');

        sqlite_query_result qr;
        if (this->query_fetch_impl("PRAGMA index_list(" + quoted + ")", qr))
        {
            // 跳过自动索引 (origin=pk 由 is_pk 覆盖, origin=u 也由 table_info 覆盖)
            // 全部收集——is_indexed 只要"在任何索引里"就算
            std::set<std::string> seen;
            for (auto &r : qr.rows)
            {
                // index_list 返回: seq, name, unique, origin, partial
                // 只取 origin='c' 的用户创建索引——pk/u 已由 PRAGMA table_info 覆盖
                if (r.size() >= 4)
                {
                    const std::string &origin = r[3];
                    if (origin != "c") continue;  // 只要用户 CREATE INDEX 的
                    const std::string &idx_name = r[1];
                    if (seen.insert(idx_name).second) names.push_back(idx_name);
                }
            }
        }
        return names; });
}

std::vector<std::string> sqlite_conn_base::get_index_info(const std::string &index_name)
{
    return this->submit_sync<std::vector<std::string>>([this, &index_name]()
                                                       {
        std::vector<std::string> col_names;
        std::string quoted;
        quoted.reserve(index_name.size() + 2);
        quoted.push_back('"');
        for (char c : index_name) { if (c == '"') quoted += """"; else quoted += c; }
        quoted.push_back('"');

        sqlite_query_result qr;
        if (this->query_fetch_impl("PRAGMA index_info(" + quoted + ")", qr))
        {
            for (auto &r : qr.rows)
            {
                // index_info 返回: seqno, cid, name
                if (r.size() >= 3) col_names.push_back(r[2]);
            }
        }
        return col_names; });
}

// ======================== 协程接口 ========================

asio::awaitable<bool> sqlite_conn_base::async_connect(const orm_conn_t &conn_config)
{
    co_return co_await this->run_on_worker<bool>([this, conn_config]()
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

asio::awaitable<bool> sqlite_conn_base::async_begin_transaction()
{
    co_return co_await this->run_on_worker<bool>([this]()
                                                 { return this->begin_transaction_impl(); });
}

asio::awaitable<bool> sqlite_conn_base::async_commit_transaction()
{
    co_return co_await this->run_on_worker<bool>([this]()
                                                 { return this->commit_transaction_impl(); });
}

asio::awaitable<bool> sqlite_conn_base::async_rollback_transaction()
{
    co_return co_await this->run_on_worker<bool>([this]()
                                                 { return this->rollback_transaction_impl(); });
}

asio::awaitable<int> sqlite_conn_base::async_exec_sql(const std::string &sql)
{
    // 按值捕获 sql：协程可能在 worker 上稍后才执行 lambda，
    // 若调用方传入的是临时对象，引用捕获会在调用点销毁后悬垂
    co_return co_await this->run_on_worker<int>([this, sql]()
                                                { return this->exec_sql_impl(sql); });
}

asio::awaitable<unsigned int> sqlite_conn_base::async_exec_dml(const std::string &sql)
{
    co_return co_await this->run_on_worker<unsigned int>([this, sql]()
                                                         { return this->exec_dml_impl(sql); });
}

asio::awaitable<long long> sqlite_conn_base::async_last_insert_rowid()
{
    co_return co_await this->run_on_worker<long long>([this]()
                                                      { return this->last_insert_rowid_impl(); });
}

asio::awaitable<bool> sqlite_conn_base::async_query_fetch(const std::string &sql, sqlite_query_result &result)
{
    // sql 按值捕获（防临时对象悬垂）；result 是调用方持有的输出参数，
    // 必须由调用方保证其生命周期跨越本协程
    co_return co_await this->run_on_worker<bool>([this, sql, &result]()
                                                 { return this->query_fetch_impl(sql, result); });
}

asio::awaitable<bool> sqlite_conn_base::async_query_scalar(const std::string &sql, sqlite_scalar_result &result)
{
    co_return co_await this->run_on_worker<bool>([this, sql, &result]()
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

    if (!sqlite_worker_submit(db_path, [&ready_promise]()
                              { ready_promise.set_value(); }))
    {
        ready_promise.set_value();// worker 已停止，避免 ready_future 永久阻塞
    }

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
    if (!sqlite_worker_submit(worker_key_, task))
    {
        // worker 已停止（程序关闭/停服）：直接在调用线程执行任务，
        // 否则 submit_sync 的 future.get() 将永久阻塞
        task();
    }
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
// g_shutdown_started / g_initialized 定义在本文件顶部的匿名命名空间中
//（sqlite_worker_submit 需读取关闭标记），此处只使用它们。

void sqlite_init()
{
    g_initialized.store(true);
    // 允许"关闭后重新初始化"：清掉一次性关闭标记，使后续 submit
    // 能再次创建 worker（应用二次启动 / 热重启场景）
    g_shutdown_started.store(false, std::memory_order_release);
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

// ===== 预编译语句实现 =====

// obj_val → sqlite_bind_param 转换
static std::vector<sqlite_bind_param> _obj_vals_to_bind_params(const std::vector<http::obj_val> &params)
{
    std::vector<sqlite_bind_param> out;
    out.reserve(params.size());
    for (const auto &v : params)
    {
        sqlite_bind_param bp;
        switch (v.get_type())
        {
        case http::obj_type::NIL:
            bp.type = SQLITE_NULL;
            break;
        case http::obj_type::INT:
        case http::obj_type::LONG:
            bp.type = SQLITE_INTEGER;
            bp.data = std::to_string(v.lval);
            break;
        case http::obj_type::UINT:
        case http::obj_type::ULONG:
            bp.type = SQLITE_INTEGER;
            bp.data = std::to_string(v.uval);
            break;
        case http::obj_type::DOUBLE:
            bp.type = SQLITE_FLOAT;
            {
                // std::to_string 固定 6 位小数会丢精度（<1e-6 归零、尾部截断），
                // 改用 %.17g 保证二进制 double 文本往返无损
                char num_buf[64];
                std::snprintf(num_buf, sizeof(num_buf), "%.17g", v.dval);
                bp.data = num_buf;
            }
            break;
        case http::obj_type::BOOL:
            bp.type = SQLITE_INTEGER;
            bp.data = v.lval ? "1" : "0";
            break;
        case http::obj_type::STRING:
        default:
            bp.type = SQLITE_TEXT;
            bp.data = v.str_;
            break;
        }
        out.push_back(std::move(bp));
    }
    return out;
}

// 给 stmt 绑定 sqlite_bind_param。
// 返回：
//   SQLITE_OK      全部绑定成功
//   SQLITE_RANGE   参数个数与 SQL 占位符数不一致（多绑定/少绑定都会漏绑成 NULL）
//   SQLITE_MISMATCH 数值文本非法或超出可表示范围（如 UINT/ULONG > INT64_MAX）
//   其它            绑定接口返回的错误码
static int _bind_params(sqlite3_stmt *stmt, const std::vector<sqlite_bind_param> &params)
{
    int max_param = sqlite3_bind_parameter_count(stmt);
    if (params.size() != static_cast<size_t>(max_param))
    {
        return SQLITE_RANGE;
    }

    for (size_t i = 0; i < params.size(); ++i)
    {
        int idx       = static_cast<int>(i) + 1;
        const auto &p = params[i];
        int rc        = SQLITE_OK;

        // 长度防护：SQLite 绑定长度参数为 int 型，超过 INT_MAX 的值无法表示，
        // 负长度又会被解释为"到首个 NUL 为止"，必须先拒绝
        if (p.type == SQLITE_TEXT || p.type == SQLITE_BLOB)
        {
            if (p.data.size() > static_cast<size_t>(INT_MAX))
            {
                return SQLITE_TOOBIG;
            }
        }

        switch (p.type)
        {
        case SQLITE_INTEGER:
            try
            {
                rc = sqlite3_bind_int64(stmt, idx, std::stoll(p.data));
            }
            catch (const std::exception &)
            {
                return SQLITE_MISMATCH;
            }
            break;
        case SQLITE_FLOAT:
            try
            {
                rc = sqlite3_bind_double(stmt, idx, std::stod(p.data));
            }
            catch (const std::exception &)
            {
                return SQLITE_MISMATCH;
            }
            break;
        case SQLITE_TEXT:
            // 显式传字节长度而非 -1：-1 只绑定到首个 '\0'，含内嵌 NUL 的
            // 字符串会被静默截断，与读取端的 column_bytes 语义对应
            rc = sqlite3_bind_text(stmt, idx, p.data.c_str(), static_cast<int>(p.data.size()), SQLITE_TRANSIENT);
            break;
        case SQLITE_BLOB:
            // data() 在空串实现上可能返回 nullptr，bind_blob 遇 NULL 指针会
            // 绑成 SQL NULL；空 BLOB 需要有效指针 + 长度 0 才绑定为空值
            rc = sqlite3_bind_blob(stmt, idx, p.data.empty() ? "" : p.data.data(), static_cast<int>(p.data.size()), SQLITE_TRANSIENT);
            break;
        case SQLITE_NULL:
        default:
            rc = sqlite3_bind_null(stmt, idx);
            break;
        }

        if (rc != SQLITE_OK)
        {
            return rc;
        }
    }
    return SQLITE_OK;
}

// 将绑定失败 rc 转换为可读的错误文本
static std::string _bind_error_msg(int rc, size_t given_count, size_t placeholder_count)
{
    if (rc == SQLITE_RANGE)
    {
        return "bind parameter count mismatch: SQL has " + std::to_string(placeholder_count) +
               " placeholder(s) but got " + std::to_string(given_count) + " value(s)";
    }
    if (rc == SQLITE_MISMATCH)
    {
        return "bind parameter value invalid/out-of-range (unsigned integer > INT64_MAX or non-numeric text)";
    }
    if (rc == SQLITE_TOOBIG)
    {
        return "bind parameter too large (length exceeds INT_MAX/2^31-1 bytes)";
    }
    return "sqlite bind error rc=" + std::to_string(rc);
}

unsigned int sqlite_conn_base::fetch_prepared_impl(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<std::tuple<unsigned char *, size_t>(int)>)> handler)
{
    this->clear_error();
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return 0;
    }

    // 空串/纯注释：无语句可执行，返回 0 行（成功），不进入缓存/错误路径
    if (sql_has_no_statement(sql))
    {
        consecutive_errors_.store(0);
        return 0;
    }

    auto bind_params = _obj_vals_to_bind_params(params);

    // 最多两轮：命中 SQLITE_SCHEMA 则驱逐缓存重试
    for (int attempt = 0; attempt < 2; attempt++)
    {
        sqlite3_stmt *stmt = stmt_cache_.get_or_create(db_, sql);
        if (!stmt)
        {
            this->set_error(sqlite3_errmsg(db_));
            return 0;
        }

        // 清除上次的绑定（如果有的话）
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

        int bind_rc = _bind_params(stmt, bind_params);
        if (bind_rc != SQLITE_OK)
        {
            this->set_error(_bind_error_msg(bind_rc, bind_params.size(), static_cast<size_t>(sqlite3_bind_parameter_count(stmt))));
            return 0;
        }

        // 列数/列名留到首行 step 之后再抓，理由同 fetch_directly_impl
        int col_count = 0;

        char *col_names_stack[64];
        std::vector<char *> col_names_heap;
        char **col_names = col_names_stack;

        // get_data 捕获 stmt + this（REAL 列精度还原走成员 float_buf_）
        auto get_data = [stmt, this](int col_idx) -> std::tuple<unsigned char *, size_t>
        {
            int col_type = sqlite3_column_type(stmt, col_idx);
            if (col_type == SQLITE_NULL)
                return {nullptr, 0};
            if (col_type == SQLITE_BLOB)
            {
                const void *blob = sqlite3_column_blob(stmt, col_idx);
                int len          = sqlite3_column_bytes(stmt, col_idx);
                if (blob && len > 0)
                    return {const_cast<unsigned char *>(static_cast<const unsigned char *>(blob)), (size_t)len};
                return {nullptr, 0};
            }
            if (col_type == SQLITE_FLOAT)
            {
                // SQLite 默认只给 15 位有效数字，用 column_double + %.17g 完整还原
                double d = sqlite3_column_double(stmt, col_idx);
                int n    = snprintf(float_buf_, sizeof(float_buf_), "%.17g", d);
                return {reinterpret_cast<unsigned char *>(float_buf_), (size_t)(n > 0 ? n : 0)};
            }
            const char *val = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_idx));
            if (val)
            {
                int len = sqlite3_column_bytes(stmt, col_idx);
                return {const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(val)), (size_t)len};
            }
            return {nullptr, 0};
        };

        unsigned int row_num = 0;
        int rc               = SQLITE_OK;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            if (row_num == 0)
            {
                col_count = sqlite3_column_count(stmt);
                if (col_count > static_cast<int>(sizeof(col_names_stack) / sizeof(col_names_stack[0])))
                {
                    col_names_heap.resize(static_cast<size_t>(col_count));
                    col_names = col_names_heap.data();
                }
                for (int i = 0; i < col_count; i++)
                    col_names[i] = const_cast<char *>(sqlite3_column_name(stmt, i));
            }
            row_num++;
            if (!handler(col_count, col_names, get_data))
            {
                sqlite3_reset(stmt);
                consecutive_errors_.store(0);
                return row_num;
            }
        }
        sqlite3_reset(stmt);

        if (rc == SQLITE_SCHEMA)
        {
            stmt_cache_.evict(sql);
            continue;
        }
        if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
            this->set_error(sqlite3_errmsg(db_));
            return row_num;
        }
        // 成功路径统一归零错误计数
        consecutive_errors_.store(0);
        return row_num;
    }
    return 0;
}

// ===== SQLite prepared 二进制版：get_data 返回 typed variant =====
// 和 fetch_prepared_impl 逻辑完全一致，唯一差别是 get_data 返回 col_value_variant
// 而不是 tuple<ptr,len>。这样 INTEGER/FLOAT 列可以直接以 int64_t/double 返回，
// 避免 SQLite driver 先转字符串、handler 再 from_chars 转回来的双向浪费。
unsigned int sqlite_conn_base::fetch_prepared_impl_binary(
    const std::string &sql,
    const std::vector<http::obj_val> &params,
    std::function<bool(int, char **, std::function<col_value_variant(int)>)> handler)
{
    this->clear_error();
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return 0;
    }

    if (sql_has_no_statement(sql))
    {
        consecutive_errors_.store(0);
        return 0;
    }

    auto bind_params = _obj_vals_to_bind_params(params);

    for (int attempt = 0; attempt < 2; attempt++)
    {
        sqlite3_stmt *stmt = stmt_cache_.get_or_create(db_, sql);
        if (!stmt)
        {
            this->set_error(sqlite3_errmsg(db_));
            return 0;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

        int bind_rc = _bind_params(stmt, bind_params);
        if (bind_rc != SQLITE_OK)
        {
            this->set_error(_bind_error_msg(bind_rc, bind_params.size(), static_cast<size_t>(sqlite3_bind_parameter_count(stmt))));
            return 0;
        }

        // 列数/列名留到首行 step 之后再抓，理由同 fetch_directly_impl
        int col_count = 0;

        char *col_names_stack[64];
        std::vector<char *> col_names_heap;
        char **col_names = col_names_stack;

        // ===== 核心差别：get_data 返回 col_value_variant =====
        // INTEGER → int64_t, FLOAT → double, TEXT/BLOB → string_view, NULL → monostate
        auto get_data = [stmt](int col_idx) -> col_value_variant
        {
            int t = sqlite3_column_type(stmt, col_idx);
            switch (t)
            {
            case SQLITE_NULL: return std::monostate{};
            case SQLITE_INTEGER: return sqlite3_column_int64(stmt, col_idx);
            case SQLITE_FLOAT: return sqlite3_column_double(stmt, col_idx);
            case SQLITE_TEXT:
            {
                const char *p = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_idx));
                if (!p)
                    return std::monostate{};
                int len = sqlite3_column_bytes(stmt, col_idx);
                return std::string_view(p, static_cast<size_t>(len > 0 ? len : 0));
            }
            case SQLITE_BLOB:
            {
                const void *b = sqlite3_column_blob(stmt, col_idx);
                if (!b)
                    return std::monostate{};
                int len = sqlite3_column_bytes(stmt, col_idx);
                return std::string_view(reinterpret_cast<const char *>(b),
                                        static_cast<size_t>(len > 0 ? len : 0));
            }
            default: return std::monostate{};
            }
        };

        unsigned int row_num = 0;
        int rc               = SQLITE_OK;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            if (row_num == 0)
            {
                col_count = sqlite3_column_count(stmt);
                if (col_count > static_cast<int>(sizeof(col_names_stack) / sizeof(col_names_stack[0])))
                {
                    col_names_heap.resize(static_cast<size_t>(col_count));
                    col_names = col_names_heap.data();
                }
                for (int i = 0; i < col_count; i++)
                    col_names[i] = const_cast<char *>(sqlite3_column_name(stmt, i));
            }
            row_num++;
            if (!handler(col_count, col_names, get_data))
            {
                sqlite3_reset(stmt);
                consecutive_errors_.store(0);
                return row_num;
            }
        }
        sqlite3_reset(stmt);

        if (rc == SQLITE_SCHEMA)
        {
            stmt_cache_.evict(sql);
            continue;
        }
        if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
            this->set_error(sqlite3_errmsg(db_));
            return row_num;
        }
        consecutive_errors_.store(0);
        return row_num;
    }
    return 0;
}

unsigned int sqlite_conn_base::exec_dml_prepared_impl(
    const std::string &sql,
    const std::vector<http::obj_val> &params)
{
    this->clear_error();
    if (!db_ || isclose_.load())
    {
        this->set_error("Database not connected");
        return (unsigned int)-1;
    }

    // 空串/纯注释：无语句可执行，视为成功且影响 0 行
    if (sql_has_no_statement(sql))
    {
        consecutive_errors_.store(0);
        return 0;
    }

    sqlite3_int64 total_before = sqlite3_total_changes(db_);
    auto bind_params           = _obj_vals_to_bind_params(params);

    for (int attempt = 0; attempt < 2; attempt++)
    {
        sqlite3_stmt *stmt = stmt_cache_.get_or_create(db_, sql);
        if (!stmt)
        {
            this->set_error(sqlite3_errmsg(db_));
            return (unsigned int)-1;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int bind_rc = _bind_params(stmt, bind_params);
        if (bind_rc != SQLITE_OK)
        {
            this->set_error(_bind_error_msg(bind_rc, bind_params.size(), static_cast<size_t>(sqlite3_bind_parameter_count(stmt))));
            return (unsigned int)-1;
        }

        int rc = sqlite3_step(stmt);
        sqlite3_reset(stmt);

        if (rc == SQLITE_SCHEMA)
        {
            stmt_cache_.evict(sql);
            continue;
        }
        if (rc != SQLITE_DONE)
        {
            this->set_error(sqlite3_errmsg(db_));
            return (unsigned int)-1;
        }

        // 成功路径统一归零错误计数，并同步事务窗口（预编译语句执行 BEGIN 等场景）
        consecutive_errors_.store(0);
        this->sync_transaction_state_after_exec();

        // 与 exec_dml_impl 一致的返回语义：仅当真正产生 DML 时返回影响行数
        if (sqlite3_total_changes(db_) > total_before)
        {
            return static_cast<unsigned int>(sqlite3_changes(db_));
        }
        return 0;
    }
    return (unsigned int)-1;
}

}// namespace orm

#endif// ENABLE_SQLITE
