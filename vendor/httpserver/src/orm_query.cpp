/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2026-06-18
 */
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <ctime>
#include <chrono>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#if defined(_WIN32) && !defined(_OPENSSL_Applink) && \
    defined(HAVE_OPENSSL_APPLINK_C)
#include <openssl/applink.c>
#endif
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "mysql_conn.h"
#include "pg_conn.h"
#include "sqlite_conn.h"
#include "orm_conn_pool.h"
#include "orm_connect_mar.h"
#include "orm_query.h"
namespace orm
{
void db_conn::select_db(std::string_view tag)
{
    dbtag                                                                = tag;
    std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
    auto iter                                                            = conn_pool_obj.find(dbtag);
    if (iter != conn_pool_obj.end())
    {
        conn_obj = iter->second;
        db_type  = conn_obj->conf_data[0].db_type;
    }
    else
    {
        conn_obj  = nullptr;
        iserror   = true;
        error_msg = "conn_pool not found " + dbtag;
    }
}

bool db_conn::begin_commit()
{
    if (iscommit)
    {
        error_msg = "not begin_commit";
        iserror   = true;
        return false;
    }
    islock_conn = true;
    iscommit    = true;
    if (conn_obj == nullptr)
    {
        error_msg = "Please select_db() tag";
        return false;
    }
    if (db_type == DB_TYPE::MYSQL)
    {
        return mysql_begin_commit_impl();
    }
    else if (db_type == DB_TYPE::SQLITE)
    {
        return sqlite_begin_commit_impl();
    }
    return pg_begin_commit_impl();
}

bool db_conn::mysql_begin_commit_impl()
{
    if (islock_conn)
    {
        if (!mysql_edit_conn)
        {
            mysql_edit_conn = conn_obj->get_mysql_edit_conn();
        }
    }
    else
    {
        mysql_edit_conn = conn_obj->get_mysql_edit_conn();
    }

    unsigned int affected = mysql_edit_conn->exec_dml("START TRANSACTION");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg   = mysql_edit_conn->error_msg;
        islock_conn = false;
        iscommit    = false;
        mysql_edit_conn.reset();
        return false;
    }
    return true;
}

bool db_conn::pg_begin_commit_impl()
{
    if (islock_conn)
    {
        if (!pg_edit_conn)
        {
            pg_edit_conn = conn_obj->get_pg_edit_conn();
        }
    }
    else
    {
        pg_edit_conn = conn_obj->get_pg_edit_conn();
    }

    unsigned int affected = pg_edit_conn->exec_dml("BEGIN");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg   = pg_edit_conn->error_msg;
        islock_conn = false;
        iscommit    = false;
        pg_edit_conn.reset();
        return false;
    }
    return true;
}

bool db_conn::commit()
{
    effect_num = 0;
    if (!iscommit)
    {
        error_msg = "not commit";
        iserror   = true;
        return false;
    }
    if (conn_obj == nullptr)
    {
        error_msg = "Please select_db() tag";
        iserror   = true;
        return false;
    }
    if (db_type == DB_TYPE::MYSQL)
    {
        return mysql_commit_impl();
    }
    else if (db_type == DB_TYPE::SQLITE)
    {
        return sqlite_commit_impl();
    }
    return pg_commit_impl();
}

bool db_conn::mysql_commit_impl()
{
    if (mysql_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        iserror   = true;
        return false;
    }

    unsigned int affected = mysql_edit_conn->exec_dml("COMMIT");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = mysql_edit_conn->error_msg;
        this->mysql_rollback_impl();
        return false;
    }
    conn_obj->back_mysql_edit_conn(std::move(mysql_edit_conn));
    iscommit    = false;
    islock_conn = false;
    return true;
}

bool db_conn::pg_commit_impl()
{
    if (pg_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        iserror   = true;
        return false;
    }

    unsigned int affected = pg_edit_conn->exec_dml("COMMIT");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = pg_edit_conn->error_msg;
        rollback();
        iscommit    = false;
        islock_conn = false;
        return false;
    }
    effect_num = affected;
    conn_obj->back_pg_edit_conn(std::move(pg_edit_conn));
    pg_edit_conn = nullptr;
    iscommit     = false;
    islock_conn  = false;
    return true;
}

void db_conn::rollback()
{
    if (!iscommit)
    {
        error_msg = "not begin_commit";
        iserror   = true;
        return;
    }
    if (conn_obj == nullptr)
    {
        error_msg = "Please select_db() tag";
        return;
    }
    if (db_type == DB_TYPE::MYSQL)
    {
        mysql_rollback_impl();
        return;
    }
    else if (db_type == DB_TYPE::SQLITE)
    {
        sqlite_rollback_impl();
        return;
    }
    pg_rollback_impl();
}

void db_conn::mysql_rollback_impl()
{
    if (mysql_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        return;
    }

    unsigned int affected = mysql_edit_conn->exec_dml("ROLLBACK");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = mysql_edit_conn->error_msg;
    }
    iscommit    = false;
    islock_conn = false;
    conn_obj->back_mysql_edit_conn(std::move(mysql_edit_conn));
}

void db_conn::pg_rollback_impl()
{
    if (pg_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        return;
    }

    unsigned int affected = pg_edit_conn->exec_dml("ROLLBACK");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = pg_edit_conn->error_msg;
    }
    else
    {
        effect_num = affected;
    }
    iscommit    = false;
    islock_conn = false;
    pg_edit_conn.reset();
}

asio::awaitable<bool> db_conn::async_begin_commit()
{
    if (iscommit)
    {
        error_msg = "begin_commit has begin";
        iserror   = true;
        co_return false;
    }
    islock_conn = true;
    iscommit    = true;
    if (conn_obj == nullptr)
    {
        error_msg = "Please select_db() tag";
        iserror   = true;
        co_return false;
    }
    if (db_type == DB_TYPE::MYSQL)
    {
        co_return co_await mysql_async_begin_commit_impl();
    }
    else if (db_type == DB_TYPE::SQLITE)
    {
        co_return co_await sqlite_async_begin_commit_impl();
    }
    co_return co_await pg_async_begin_commit_impl();
}

asio::awaitable<bool> db_conn::mysql_async_begin_commit_impl()
{
    if (islock_conn)
    {
        if (!mysql_edit_conn)
        {
            mysql_edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
        }
    }
    else
    {
        mysql_edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
    }

    unsigned int affected = co_await mysql_edit_conn->async_exec_dml("START TRANSACTION");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg   = mysql_edit_conn->error_msg;
        islock_conn = false;
        iscommit    = false;
        mysql_edit_conn.reset();
        co_return false;
    }
    co_return true;
}

asio::awaitable<bool> db_conn::pg_async_begin_commit_impl()
{
    if (islock_conn)
    {
        if (!pg_edit_conn)
        {
            pg_edit_conn = conn_obj->get_pg_edit_conn();
        }
    }
    else
    {
        pg_edit_conn = conn_obj->get_pg_edit_conn();
    }

    unsigned int affected = co_await pg_edit_conn->async_exec_dml("BEGIN");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg   = pg_edit_conn->error_msg;
        islock_conn = false;
        iscommit    = false;
        pg_edit_conn.reset();
        co_return false;
    }
    co_return true;
}

asio::awaitable<bool> db_conn::async_commit()
{
    effect_num = 0;
    if (!iscommit)
    {
        error_msg = "not commit";
        iserror   = true;
        co_return false;
    }
    if (conn_obj == nullptr)
    {
        error_msg = "Please select_db() tag";
        iserror   = true;
        co_return false;
    }
    if (db_type == DB_TYPE::MYSQL)
    {
        co_return co_await mysql_async_commit_impl();
    }
    else if (db_type == DB_TYPE::SQLITE)
    {
        co_return co_await sqlite_async_commit_impl();
    }
    co_return co_await pg_async_commit_impl();
}

asio::awaitable<bool> db_conn::mysql_async_commit_impl()
{
    if (mysql_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        iserror   = true;
        co_return false;
    }

    unsigned int affected = co_await mysql_edit_conn->async_exec_dml("COMMIT");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = mysql_edit_conn->error_msg;
        co_await this->mysql_async_rollback_impl();
        co_return false;
    }
    conn_obj->back_mysql_edit_conn(std::move(mysql_edit_conn));
    iscommit    = false;
    islock_conn = false;
    co_return true;
}

asio::awaitable<bool> db_conn::pg_async_commit_impl()
{
    if (pg_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        iserror   = true;
        co_return false;
    }

    unsigned int affected = co_await pg_edit_conn->async_exec_dml("COMMIT");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = pg_edit_conn->error_msg;
        co_await async_rollback();
        iserror = true;
        co_return false;
    }
    effect_num = affected;
    conn_obj->back_pg_edit_conn(std::move(pg_edit_conn));
    pg_edit_conn = nullptr;
    iscommit     = false;
    islock_conn  = false;
    co_return true;
}

asio::awaitable<void> db_conn::async_rollback()
{
    if (!iscommit)
    {
        error_msg = "not begin_commit";
        iserror   = true;
        co_return;
    }
    if (conn_obj == nullptr)
    {
        error_msg = "Please select_db() tag";
        co_return;
    }
    if (db_type == DB_TYPE::MYSQL)
    {
        co_await mysql_async_rollback_impl();
        co_return;
    }
    else if (db_type == DB_TYPE::SQLITE)
    {
        co_await sqlite_async_rollback_impl();
        co_return;
    }
    co_await pg_async_rollback_impl();
}

asio::awaitable<void> db_conn::mysql_async_rollback_impl()
{
    if (mysql_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        co_return;
    }

    unsigned int affected = co_await mysql_edit_conn->async_exec_dml("ROLLBACK");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = mysql_edit_conn->error_msg;
    }
    iscommit    = false;
    islock_conn = false;
    conn_obj->back_mysql_edit_conn(std::move(mysql_edit_conn));
}

asio::awaitable<void> db_conn::pg_async_rollback_impl()
{
    if (pg_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        co_return;
    }

    unsigned int affected = co_await pg_edit_conn->async_exec_dml("ROLLBACK");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg   = pg_edit_conn->error_msg;
        islock_conn = false;
        iscommit    = false;
        iserror     = true;
    }
    else
    {
        effect_num = affected;
    }

    iscommit    = false;
    islock_conn = false;
    pg_edit_conn.reset();
}

// ======================== SQLite transaction implementations ========================

bool db_conn::sqlite_begin_commit_impl()
{
    if (islock_conn)
    {
        if (!sqlite_edit_conn)
        {
            sqlite_edit_conn = conn_obj->get_sqlite_edit_conn();
        }
    }
    else
    {
        sqlite_edit_conn = conn_obj->get_sqlite_edit_conn();
    }

    unsigned int affected = sqlite_edit_conn->exec_dml("BEGIN");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg   = sqlite_edit_conn->error_msg;
        islock_conn = false;
        iscommit    = false;
        sqlite_edit_conn.reset();
        return false;
    }
    return true;
}

bool db_conn::sqlite_commit_impl()
{
    if (sqlite_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        iserror   = true;
        return false;
    }

    unsigned int affected = sqlite_edit_conn->exec_dml("COMMIT");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = sqlite_edit_conn->error_msg;
        this->sqlite_rollback_impl();
        return false;
    }
    conn_obj->back_sqlite_edit_conn(std::move(sqlite_edit_conn));
    iscommit    = false;
    islock_conn = false;
    return true;
}

void db_conn::sqlite_rollback_impl()
{
    if (sqlite_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        return;
    }

    unsigned int affected = sqlite_edit_conn->exec_dml("ROLLBACK");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = sqlite_edit_conn->error_msg;
    }
    iscommit    = false;
    islock_conn = false;
    conn_obj->back_sqlite_edit_conn(std::move(sqlite_edit_conn));
}

asio::awaitable<bool> db_conn::sqlite_async_begin_commit_impl()
{
    if (islock_conn)
    {
        if (!sqlite_edit_conn)
        {
            sqlite_edit_conn = co_await conn_obj->async_get_sqlite_edit_conn();
        }
    }
    else
    {
        sqlite_edit_conn = co_await conn_obj->async_get_sqlite_edit_conn();
    }

    unsigned int affected = co_await sqlite_edit_conn->async_exec_dml("BEGIN");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg   = sqlite_edit_conn->error_msg;
        islock_conn = false;
        iscommit    = false;
        sqlite_edit_conn.reset();
        co_return false;
    }
    co_return true;
}

asio::awaitable<bool> db_conn::sqlite_async_commit_impl()
{
    if (sqlite_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        iserror   = true;
        co_return false;
    }

    unsigned int affected = co_await sqlite_edit_conn->async_exec_dml("COMMIT");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = sqlite_edit_conn->error_msg;
        co_await this->sqlite_async_rollback_impl();
        co_return false;
    }
    conn_obj->back_sqlite_edit_conn(std::move(sqlite_edit_conn));
    iscommit    = false;
    islock_conn = false;
    co_return true;
}

asio::awaitable<void> db_conn::sqlite_async_rollback_impl()
{
    if (sqlite_edit_conn == nullptr)
    {
        error_msg = "edit_conn error, must begin_commit() first";
        co_return;
    }

    unsigned int affected = co_await sqlite_edit_conn->async_exec_dml("ROLLBACK");
    if (affected == static_cast<unsigned int>(-1))
    {
        error_msg = sqlite_edit_conn->error_msg;
    }
    iscommit    = false;
    islock_conn = false;
    conn_obj->back_sqlite_edit_conn(std::move(sqlite_edit_conn));
}

unsigned int db_conn::edit_query(const std::string &rawsql)
{
    if (db_type == DB_TYPE::MYSQL)
    {
        return mysql_edit_query_impl(rawsql);
    }
    else if (db_type == DB_TYPE::SQLITE)
    {
        return sqlite_edit_query_impl(rawsql);
    }
    return pg_edit_query_impl(rawsql);
}

unsigned int db_conn::mysql_edit_query_impl(const std::string &rawsql)
{
    effect_num = 0;
    if (iserror)
    {
        return 0;
    }

    try
    {
        if (conn_obj == nullptr)
        {
            error_msg = "Please select_db() tag";
            return 0;
        }

        if (islock_conn)
        {
            if (!mysql_edit_conn)
            {
                mysql_edit_conn = conn_obj->get_mysql_edit_conn();
            }
        }
        else
        {
            mysql_edit_conn = conn_obj->get_mysql_edit_conn();
        }

        if (mysql_edit_conn->isdebug)
        {
            mysql_edit_conn->begin_time();
        }

        unsigned int affected = mysql_edit_conn->exec_dml(rawsql);
        if (affected == static_cast<unsigned int>(-1))
        {
            error_msg = mysql_edit_conn->error_msg;
            mysql_edit_conn.reset();
            return 0;
        }
        effect_num = affected;

        if (mysql_edit_conn->isdebug)
        {
            mysql_edit_conn->finish_time();
            auto &conn_mar    = get_orm_connect_mar();
            long long du_time = mysql_edit_conn->count_time();
            conn_mar.push_log(rawsql, std::to_string(du_time));
        }
        if (!islock_conn)
        {
            conn_obj->back_mysql_edit_conn(std::move(mysql_edit_conn));
        }
        return effect_num;
    }
    catch (const std::exception &e)
    {
        error_msg = std::string(e.what());
        return 0;
    }

    return 0;
}

unsigned int db_conn::pg_edit_query_impl(const std::string &rawsql)
{
    effect_num = 0;
    if (iserror)
    {
        return 0;
    }

    try
    {
        if (conn_obj == nullptr)
        {
            error_msg = "Please select_db() tag";
            return 0;
        }

        if (islock_conn)
        {
            if (!pg_edit_conn)
            {
                pg_edit_conn = conn_obj->get_pg_edit_conn();
            }
        }
        else
        {
            pg_edit_conn = conn_obj->get_pg_edit_conn();
        }

        if (pg_edit_conn->isdebug)
        {
            pg_edit_conn->begin_time();
        }

        unsigned int affected = pg_edit_conn->exec_dml(rawsql);
        if (affected == static_cast<unsigned int>(-1))
        {
            error_msg = pg_edit_conn->error_msg;
            pg_edit_conn.reset();
            return 0;
        }
        effect_num = affected;

        if (pg_edit_conn->isdebug)
        {
            pg_edit_conn->finish_time();
            auto &conn_mar    = get_orm_connect_mar();
            long long du_time = pg_edit_conn->count_time();
            conn_mar.push_log(rawsql, std::to_string(du_time));
        }
        if (!islock_conn)
        {
            conn_obj->back_pg_edit_conn(std::move(pg_edit_conn));
        }
        return effect_num;
    }
    catch (const std::exception &e)
    {
        error_msg = std::string(e.what());
        return 0;
    }

    return 0;
}

unsigned int db_conn::sqlite_edit_query_impl(const std::string &rawsql)
{
    effect_num = 0;
    if (iserror)
    {
        return 0;
    }

    try
    {
        if (conn_obj == nullptr)
        {
            error_msg = "Please select_db() tag";
            return 0;
        }

        if (islock_conn)
        {
            if (!sqlite_edit_conn)
            {
                sqlite_edit_conn = conn_obj->get_sqlite_edit_conn();
            }
        }
        else
        {
            sqlite_edit_conn = conn_obj->get_sqlite_edit_conn();
        }

        if (sqlite_edit_conn->isdebug)
        {
            sqlite_edit_conn->begin_time();
        }

        unsigned int affected = sqlite_edit_conn->exec_dml(rawsql);
        if (affected == static_cast<unsigned int>(-1))
        {
            error_msg = sqlite_edit_conn->error_msg;
            sqlite_edit_conn.reset();
            return 0;
        }
        effect_num = affected;

        if (sqlite_edit_conn->isdebug)
        {
            sqlite_edit_conn->finish_time();
            auto &conn_mar    = get_orm_connect_mar();
            long long du_time = sqlite_edit_conn->count_time();
            conn_mar.push_log(rawsql, std::to_string(du_time));
        }
        if (!islock_conn)
        {
            conn_obj->back_sqlite_edit_conn(std::move(sqlite_edit_conn));
        }
        return effect_num;
    }
    catch (const std::exception &e)
    {
        error_msg = std::string(e.what());
        return 0;
    }

    return 0;
}

asio::awaitable<unsigned int> db_conn::async_edit_query(const std::string &rawsql)
{
    if (db_type == DB_TYPE::MYSQL)
    {
        co_return co_await mysql_async_edit_query_impl(rawsql);
    }
    else if (db_type == DB_TYPE::SQLITE)
    {
        co_return co_await sqlite_async_edit_query_impl(rawsql);
    }
    co_return co_await pg_async_edit_query_impl(rawsql);
}

asio::awaitable<unsigned int> db_conn::mysql_async_edit_query_impl(const std::string &rawsql)
{
    effect_num = 0;
    if (iserror)
    {
        co_return 0;
    }

    try
    {
        if (conn_obj == nullptr)
        {
            error_msg = "Please select_db() tag";
            co_return 0;
        }

        if (islock_conn)
        {
            if (!mysql_edit_conn)
            {
                mysql_edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
            }
        }
        else
        {
            mysql_edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
        }

        if (mysql_edit_conn->isdebug)
        {
            mysql_edit_conn->begin_time();
        }

        unsigned int affected = co_await mysql_edit_conn->async_exec_dml(rawsql);
        if (affected == static_cast<unsigned int>(-1))
        {
            error_msg = mysql_edit_conn->error_msg;
            mysql_edit_conn.reset();
            co_return 0;
        }
        effect_num = affected;

        if (mysql_edit_conn->isdebug)
        {
            mysql_edit_conn->finish_time();
            auto &conn_mar    = get_orm_connect_mar();
            long long du_time = mysql_edit_conn->count_time();
            conn_mar.push_log(rawsql, std::to_string(du_time));
        }
        if (!islock_conn)
        {
            conn_obj->back_mysql_edit_conn(std::move(mysql_edit_conn));
        }
        co_return effect_num;
    }
    catch (const std::exception &e)
    {
        error_msg = std::string(e.what());
        co_return 0;
    }

    co_return 0;
}

asio::awaitable<unsigned int> db_conn::pg_async_edit_query_impl(const std::string &rawsql)
{
    effect_num = 0;
    if (iserror)
    {
        co_return 0;
    }

    try
    {
        if (conn_obj == nullptr)
        {
            error_msg = "Please select_db() tag";
            co_return 0;
        }

        if (islock_conn)
        {
            if (!pg_edit_conn)
            {
                pg_edit_conn = co_await conn_obj->async_get_pg_edit_conn();
            }
        }
        else
        {
            pg_edit_conn = co_await conn_obj->async_get_pg_edit_conn();
        }

        if (pg_edit_conn->isdebug)
        {
            pg_edit_conn->begin_time();
        }

        unsigned int affected = co_await pg_edit_conn->async_exec_dml(rawsql);
        if (affected == static_cast<unsigned int>(-1))
        {
            error_msg = pg_edit_conn->error_msg;
            pg_edit_conn.reset();
            co_return 0;
        }
        effect_num = affected;

        if (pg_edit_conn->isdebug)
        {
            pg_edit_conn->finish_time();
            auto &conn_mar    = get_orm_connect_mar();
            long long du_time = pg_edit_conn->count_time();
            conn_mar.push_log(rawsql, std::to_string(du_time));
        }
        if (!islock_conn)
        {
            conn_obj->back_pg_edit_conn(std::move(pg_edit_conn));
        }
        co_return effect_num;
    }
    catch (const std::exception &e)
    {
        error_msg = std::string(e.what());
        co_return 0;
    }

    co_return 0;
}

asio::awaitable<unsigned int> db_conn::sqlite_async_edit_query_impl(const std::string &rawsql)
{
    effect_num = 0;
    if (iserror)
    {
        co_return 0;
    }

    try
    {
        if (conn_obj == nullptr)
        {
            error_msg = "Please select_db() tag";
            co_return 0;
        }

        if (islock_conn)
        {
            if (!sqlite_edit_conn)
            {
                sqlite_edit_conn = co_await conn_obj->async_get_sqlite_edit_conn();
            }
        }
        else
        {
            sqlite_edit_conn = co_await conn_obj->async_get_sqlite_edit_conn();
        }

        if (sqlite_edit_conn->isdebug)
        {
            sqlite_edit_conn->begin_time();
        }

        unsigned int affected = co_await sqlite_edit_conn->async_exec_dml(rawsql);
        if (affected == static_cast<unsigned int>(-1))
        {
            error_msg = sqlite_edit_conn->error_msg;
            sqlite_edit_conn.reset();
            co_return 0;
        }
        effect_num = affected;

        if (sqlite_edit_conn->isdebug)
        {
            sqlite_edit_conn->finish_time();
            auto &conn_mar    = get_orm_connect_mar();
            long long du_time = sqlite_edit_conn->count_time();
            conn_mar.push_log(rawsql, std::to_string(du_time));
        }
        if (!islock_conn)
        {
            conn_obj->back_sqlite_edit_conn(std::move(sqlite_edit_conn));
        }
        co_return effect_num;
    }
    catch (const std::exception &e)
    {
        error_msg = std::string(e.what());
        co_return 0;
    }

    co_return 0;
}

}// namespace orm
