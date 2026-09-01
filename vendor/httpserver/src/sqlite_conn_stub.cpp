/*
 * @Description: SQLite 存根实现 (ENABLE_SQLITE=OFF 时使用)
 * SQLite stub implementation (used when ENABLE_SQLITE=OFF)
 * 
 * 本文件始终参与编译，提供 ENABLE_SQLITE=OFF 时的所有存根函数定义。
 * 当 ENABLE_SQLITE=ON 时，本文件中的实现被 #ifndef ENABLE_SQLITE 守卫排除，
 * 实际实现由 sqlite_conn.cpp 提供。
 *
 * 设计目标:
 *   - ENABLE_SQLITE=OFF 时，不需要安装 sqlite3 库
 *   - 不查找 sqlite3.h、不链接 libsqlite3
 *   - 所有 SQLite 相关调用安全返回错误值，不会崩溃
 */

#include "sqlite_conn.h"

#include <chrono>
#include <functional>
#include <string>

namespace orm
{

#ifndef ENABLE_SQLITE

static const char *SQLITE_NOT_COMPILED_MSG =
    "SQLite support not compiled. Rebuild with -DENABLE_SQLITE=ON";

// ======================== Worker 存根 ========================

bool sqlite_worker_submit(const std::string & /*db_path*/, std::function<void()> task)
{
    if (task)
    {
        task();
    }
    return true;
}

void sqlite_init()
{
}

void sqlite_shutdown()
{
}

void sqlite_worker_shutdown_all_safe()
{
}

void sqlite_worker_shutdown(const std::string & /*db_path*/)
{
}

void sqlite_worker_shutdown_all()
{
}

std::string sqlite_db_path_of(const orm_conn_t &conn_config)
{
    return conn_config.host.empty() ? conn_config.dbname : conn_config.host;
}

// ======================== sqlite_conn_base 存根 ========================

sqlite_conn_base::sqlite_conn_base()
{
}

sqlite_conn_base::~sqlite_conn_base()
{
}

void sqlite_conn_base::set_error(const std::string &msg)
{
    error_msg = msg;
}

bool sqlite_conn_base::connect(const orm_conn_t &conn_config)
{
    (void)conn_config;
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

bool sqlite_conn_base::close()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

bool sqlite_conn_base::ping()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

bool sqlite_conn_base::is_closed()
{
    return true;
}

int sqlite_conn_base::exec_sql(const std::string & /*sql*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return -1;
}

bool sqlite_conn_base::query_fetch(const std::string & /*sql*/, sqlite_query_result & /*result*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

bool sqlite_conn_base::query_scalar(const std::string & /*sql*/, sqlite_scalar_result & /*result*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

unsigned int sqlite_conn_base::exec_dml(const std::string & /*sql*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return static_cast<unsigned int>(-1);
}

long long sqlite_conn_base::last_insert_rowid()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return 0;
}

int sqlite_conn_base::exec_batch(const std::vector<std::string> & /*sqls*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return -1;
}

bool sqlite_conn_base::exec_bound(const std::string & /*sql*/, const std::vector<sqlite_bind_param> & /*params*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

std::vector<std::string> sqlite_conn_base::get_table_list()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return {};
}

std::vector<std::map<std::string, std::string>> sqlite_conn_base::get_table_info(const std::string & /*table*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return {};
}

asio::awaitable<bool> sqlite_conn_base::async_connect(const orm_conn_t &conn_config)
{
    (void)conn_config;
    error_msg = SQLITE_NOT_COMPILED_MSG;
    co_return false;
}

asio::awaitable<bool> sqlite_conn_base::async_close()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    co_return false;
}

asio::awaitable<bool> sqlite_conn_base::async_ping()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    co_return false;
}

asio::awaitable<int> sqlite_conn_base::async_exec_sql(const std::string & /*sql*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    co_return -1;
}

asio::awaitable<bool> sqlite_conn_base::async_query_fetch(const std::string & /*sql*/, sqlite_query_result & /*result*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    co_return false;
}

asio::awaitable<bool> sqlite_conn_base::async_query_scalar(const std::string & /*sql*/, sqlite_scalar_result & /*result*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    co_return false;
}

asio::awaitable<unsigned int> sqlite_conn_base::async_exec_dml(const std::string & /*sql*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    co_return static_cast<unsigned int>(-1);
}

asio::awaitable<long long> sqlite_conn_base::async_last_insert_rowid()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    co_return 0;
}

void sqlite_conn_base::start_worker_thread(asio::io_context & /*ioc*/, const std::string & /*db_path*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
}

void sqlite_conn_base::stop_worker_thread()
{
}

bool sqlite_conn_base::integrity_check(std::string &report)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    report    = SQLITE_NOT_COMPILED_MSG;
    return false;
}

bool sqlite_conn_base::repair_database()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

bool sqlite_conn_base::reconnect(int /*max_retries*/)
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

bool sqlite_conn_base::rollback_unfinished_transactions()
{
    error_msg = SQLITE_NOT_COMPILED_MSG;
    return false;
}

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
    return "SQLite support not compiled";
}

#endif// !ENABLE_SQLITE (i.e. ENABLE_SQLITE not defined)

}// namespace orm
