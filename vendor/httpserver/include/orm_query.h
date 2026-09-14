#ifndef _ORM_QUERY_H
#define _ORM_QUERY_H
/*
 * @Author: 黄自权 Huang ziqun
 * @Date:   2026-06-18
 */
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include "unicode.h"
#include "mysql_conn.h"
#include "pg_conn.h"
#include "sqlite_conn.h"
#include "orm_conn_pool.h"
#include "orm_connect_mar.h"

namespace orm
{

class db_conn : std::enable_shared_from_this<db_conn>
{
  public:
    db_conn() {};
    db_conn(std::string_view tag) : dbtag(tag)
    {
        select_db(tag);
    };
    void select_db(std::string_view tag);
    void lock_conn(bool islock)
    {
        islock_conn = islock;
    }

    ~db_conn()
    {
        if (islock_conn)
        {
            islock_conn = false;
            if (db_type == DB_TYPE::MYSQL)
            {
                if (mysql_select_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
                }
                if (mysql_edit_conn)
                {
                    conn_obj->back_mysql_edit_conn(std::move(mysql_edit_conn));
                }
            }
            else if (db_type == DB_TYPE::SQLITE)
            {
                if (sqlite_select_conn)
                {
                    conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
                }
                if (sqlite_edit_conn)
                {
                    conn_obj->back_sqlite_edit_conn(std::move(sqlite_edit_conn));
                }
            }
            else
            {
                if (pg_select_conn)
                {
                    conn_obj->back_pg_select_conn(std::move(pg_select_conn));
                }
                if (pg_edit_conn)
                {
                    conn_obj->back_pg_edit_conn(std::move(pg_edit_conn));
                }
            }
        }
    };
    ////1111 not callback
    template <ResultHasSetVal T>
    unsigned int mysql_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = conn_obj->get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = conn_obj->get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            unsigned int fetch_count = mysql_select_conn->fetch_directly(rawsql,
                                                                         [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                         {
                                                                             T data_temp;
                                                                             // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                             if (col_cache.empty() && col_count > 0)
                                                                             {
                                                                                 col_cache.reserve(col_count);
                                                                                 for (int k = 0; k < col_count; k++)
                                                                                 {
                                                                                     col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                 }
                                                                             }
                                                                             for (int ij = 0; ij < col_count; ij++)
                                                                             {
                                                                                 auto [ptr, len] = get_data(ij);
                                                                                 if (ptr == nullptr)
                                                                                 {
                                                                                     continue;
                                                                                 }
                                                                                 if (!col_cache[ij].empty())
                                                                                 {
                                                                                     data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                 }
                                                                             }
                                                                             result_record.emplace_back(std::move(data_temp));
                                                                             effect_num++;
                                                                             return true;
                                                                         });

            if (fetch_count == 0 && !mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int pg_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = conn_obj->get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = conn_obj->get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            unsigned int fetch_count = pg_select_conn->fetch_directly(rawsql,
                                                                      [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                      {
                                                                          T data_temp;
                                                                          // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                          if (col_cache.empty() && col_count > 0)
                                                                          {
                                                                              col_cache.reserve(col_count);
                                                                              for (int k = 0; k < col_count; k++)
                                                                              {
                                                                                  col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                              }
                                                                          }
                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                          {
                                                                              auto [ptr, len] = get_data(ij);
                                                                              if (ptr == nullptr)
                                                                              {
                                                                                  continue;
                                                                              }
                                                                              if (!col_cache[ij].empty())
                                                                              {
                                                                                  data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                              }
                                                                          }
                                                                          result_record.emplace_back(std::move(data_temp));
                                                                          effect_num++;
                                                                          return true;
                                                                      });

            if (fetch_count == 0 && !pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int sqlite_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = conn_obj->get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = conn_obj->get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            unsigned int fetch_count = sqlite_select_conn->fetch_directly(rawsql,
                                                                          [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                          {
                                                                              T data_temp;
                                                                              // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                              if (col_cache.empty() && col_count > 0)
                                                                              {
                                                                                  col_cache.reserve(col_count);
                                                                                  for (int k = 0; k < col_count; k++)
                                                                                  {
                                                                                      col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                  }
                                                                              }
                                                                              for (int ij = 0; ij < col_count; ij++)
                                                                              {
                                                                                  auto [ptr, len] = get_data(ij);
                                                                                  if (ptr == nullptr)
                                                                                  {
                                                                                      continue;
                                                                                  }
                                                                                  if (!col_cache[ij].empty())
                                                                                  {
                                                                                      data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                  }
                                                                              }
                                                                              result_record.emplace_back(std::move(data_temp));
                                                                              effect_num++;
                                                                              return true;
                                                                          });

            if (fetch_count == 0 && !sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int query(const std::string &rawsql, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_query_vec_impl(rawsql, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_query_vec_impl(rawsql, result_record);
        }
        return pg_query_vec_impl(rawsql, result_record);
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> mysql_async_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }
            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await mysql_select_conn->async_fetch_directly(rawsql,
                                                                                        [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                        {
                                                                                            T data_temp;
                                                                                            // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                            if (col_cache.empty() && col_count > 0)
                                                                                            {
                                                                                                col_cache.reserve(col_count);
                                                                                                for (int k = 0; k < col_count; k++)
                                                                                                {
                                                                                                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                                }
                                                                                            }
                                                                                            for (int ij = 0; ij < col_count; ij++)
                                                                                            {
                                                                                                auto [ptr, len] = get_data(ij);
                                                                                                if (ptr == nullptr)
                                                                                                {
                                                                                                    continue;
                                                                                                }
                                                                                                if (!col_cache[ij].empty())
                                                                                                {
                                                                                                    data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                                }
                                                                                            }
                                                                                            result_record.emplace_back(std::move(data_temp));
                                                                                            effect_num++;
                                                                                            return true;
                                                                                        });

            if (fetch_count == 0 && !mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> pg_async_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await pg_select_conn->async_fetch_directly(rawsql,
                                                                                     [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                     {
                                                                                         T data_temp;
                                                                                         // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                         if (col_cache.empty() && col_count > 0)
                                                                                         {
                                                                                             col_cache.reserve(col_count);
                                                                                             for (int k = 0; k < col_count; k++)
                                                                                             {
                                                                                                 col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                             }
                                                                                         }
                                                                                         for (int ij = 0; ij < col_count; ij++)
                                                                                         {
                                                                                             auto [ptr, len] = get_data(ij);
                                                                                             if (ptr == nullptr)
                                                                                             {
                                                                                                 continue;
                                                                                             }
                                                                                             if (!col_cache[ij].empty())
                                                                                             {
                                                                                                 data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                             }
                                                                                         }
                                                                                         result_record.emplace_back(std::move(data_temp));
                                                                                         effect_num++;
                                                                                         return true;
                                                                                     });

            if (fetch_count == 0 && !pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> sqlite_async_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await sqlite_select_conn->async_fetch_directly(rawsql,
                                                                                         [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                         {
                                                                                             T data_temp;
                                                                                             // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                             if (col_cache.empty() && col_count > 0)
                                                                                             {
                                                                                                 col_cache.reserve(col_count);
                                                                                                 for (int k = 0; k < col_count; k++)
                                                                                                 {
                                                                                                     col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                                 }
                                                                                             }
                                                                                             for (int ij = 0; ij < col_count; ij++)
                                                                                             {
                                                                                                 auto [ptr, len] = get_data(ij);
                                                                                                 if (ptr == nullptr)
                                                                                                 {
                                                                                                     continue;
                                                                                                 }
                                                                                                 if (!col_cache[ij].empty())
                                                                                                 {
                                                                                                     data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                                 }
                                                                                             }
                                                                                             result_record.emplace_back(std::move(data_temp));
                                                                                             effect_num++;
                                                                                             return true;
                                                                                         });

            if (fetch_count == 0 && !sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                co_return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_query_vec_impl(rawsql, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_query_vec_impl(rawsql, result_record);
        }
        co_return co_await pg_async_query_vec_impl(rawsql, result_record);
    }

    template <ResultHasSetVal T>
    unsigned int mysql_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = conn_obj->get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = conn_obj->get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            unsigned int fetch_count = mysql_select_conn->fetch_directly(rawsql,
                                                                         [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                         {
                                                                             // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                             if (col_cache.empty() && col_count > 0)
                                                                             {
                                                                                 col_cache.reserve(col_count);
                                                                                 for (int k = 0; k < col_count; k++)
                                                                                 {
                                                                                     col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                 }
                                                                             }
                                                                             for (int ij = 0; ij < col_count; ij++)
                                                                             {
                                                                                 auto [ptr, len] = get_data(ij);
                                                                                 if (ptr == nullptr)
                                                                                 {
                                                                                     continue;
                                                                                 }
                                                                                 if (!col_cache[ij].empty())
                                                                                 {
                                                                                     result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                 }
                                                                             }
                                                                             effect_num++;
                                                                             return false;
                                                                         });

            if (fetch_count == 0 && !mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int pg_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = conn_obj->get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = conn_obj->get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            unsigned int fetch_count = pg_select_conn->fetch_directly(rawsql,
                                                                      [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                      {
                                                                          // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                          if (col_cache.empty() && col_count > 0)
                                                                          {
                                                                              col_cache.reserve(col_count);
                                                                              for (int k = 0; k < col_count; k++)
                                                                              {
                                                                                  col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                              }
                                                                          }
                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                          {
                                                                              auto [ptr, len] = get_data(ij);
                                                                              if (ptr == nullptr)
                                                                              {
                                                                                  continue;
                                                                              }
                                                                              if (!col_cache[ij].empty())
                                                                              {
                                                                                  result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                              }
                                                                          }
                                                                          effect_num++;
                                                                          return false;// 只取首行
                                                                      });

            if (fetch_count == 0 && !pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int sqlite_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = conn_obj->get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = conn_obj->get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            unsigned int fetch_count = sqlite_select_conn->fetch_directly(rawsql,
                                                                          [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                          {
                                                                              // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                              if (col_cache.empty() && col_count > 0)
                                                                              {
                                                                                  col_cache.reserve(col_count);
                                                                                  for (int k = 0; k < col_count; k++)
                                                                                  {
                                                                                      col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                  }
                                                                              }
                                                                              for (int ij = 0; ij < col_count; ij++)
                                                                              {
                                                                                  auto [ptr, len] = get_data(ij);
                                                                                  if (ptr == nullptr)
                                                                                  {
                                                                                      continue;
                                                                                  }
                                                                                  if (!col_cache[ij].empty())
                                                                                  {
                                                                                      result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                  }
                                                                              }
                                                                              effect_num++;
                                                                              return false;// only fetch first row
                                                                          });

            if (fetch_count == 0 && !sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int query(const std::string &rawsql, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_query_single_impl(rawsql, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_query_single_impl(rawsql, result_record);
        }
        return pg_query_single_impl(rawsql, result_record);
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> mysql_async_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }
            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await mysql_select_conn->async_fetch_directly(rawsql,
                                                                                        [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                        {
                                                                                            // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                            if (col_cache.empty() && col_count > 0)
                                                                                            {
                                                                                                col_cache.reserve(col_count);
                                                                                                for (int k = 0; k < col_count; k++)
                                                                                                {
                                                                                                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                                }
                                                                                            }
                                                                                            for (int ij = 0; ij < col_count; ij++)
                                                                                            {
                                                                                                auto [ptr, len] = get_data(ij);
                                                                                                if (ptr == nullptr)
                                                                                                {
                                                                                                    continue;
                                                                                                }
                                                                                                if (!col_cache[ij].empty())
                                                                                                {
                                                                                                    result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                                }
                                                                                            }
                                                                                            effect_num++;
                                                                                            return false;
                                                                                        });

            if (fetch_count == 0 && !mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> pg_async_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await pg_select_conn->async_fetch_directly(rawsql,
                                                                                     [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                     {
                                                                                         // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                         if (col_cache.empty() && col_count > 0)
                                                                                         {
                                                                                             col_cache.reserve(col_count);
                                                                                             for (int k = 0; k < col_count; k++)
                                                                                             {
                                                                                                 col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                             }
                                                                                         }
                                                                                         for (int ij = 0; ij < col_count; ij++)
                                                                                         {
                                                                                             auto [ptr, len] = get_data(ij);
                                                                                             if (ptr == nullptr)
                                                                                             {
                                                                                                 continue;
                                                                                             }
                                                                                             if (!col_cache[ij].empty())
                                                                                             {
                                                                                                 result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                             }
                                                                                         }
                                                                                         effect_num++;
                                                                                         return false;// 只取首行
                                                                                     });

            if (fetch_count == 0 && !pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> sqlite_async_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await sqlite_select_conn->async_fetch_directly(rawsql,
                                                                                         [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                         {
                                                                                             // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                             if (col_cache.empty() && col_count > 0)
                                                                                             {
                                                                                                 col_cache.reserve(col_count);
                                                                                                 for (int k = 0; k < col_count; k++)
                                                                                                 {
                                                                                                     col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                                 }
                                                                                             }
                                                                                             for (int ij = 0; ij < col_count; ij++)
                                                                                             {
                                                                                                 auto [ptr, len] = get_data(ij);
                                                                                                 if (ptr == nullptr)
                                                                                                 {
                                                                                                     continue;
                                                                                                 }
                                                                                                 if (!col_cache[ij].empty())
                                                                                                 {
                                                                                                     result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                                 }
                                                                                             }
                                                                                             effect_num++;
                                                                                             return false;// only fetch first row
                                                                                         });

            if (fetch_count == 0 && !sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                co_return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_query_single_impl(rawsql, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_query_single_impl(rawsql, result_record);
        }
        co_return co_await pg_async_query_single_impl(rawsql, result_record);
    }

    ////2222 callback
    template <typename T, RecordLineCallback<T> Callback>
    unsigned int mysql_query_vec_cb_impl(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = conn_obj->get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = conn_obj->get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            unsigned int fetch_count = mysql_select_conn->fetch_directly(rawsql,
                                                                         [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                         {
                                                                             T data_temp;
                                                                             // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                             if (col_cache.empty() && col_count > 0)
                                                                             {
                                                                                 col_cache.reserve(col_count);
                                                                                 for (int k = 0; k < col_count; k++)
                                                                                 {
                                                                                     col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                 }
                                                                             }
                                                                             for (int ij = 0; ij < col_count; ij++)
                                                                             {
                                                                                 auto [ptr, len] = get_data(ij);
                                                                                 if (ptr == nullptr)
                                                                                 {
                                                                                     continue;
                                                                                 }
                                                                                 if (!col_cache[ij].empty())
                                                                                 {
                                                                                     std::invoke(callback, data_temp, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                 }
                                                                             }
                                                                             result_record.emplace_back(std::move(data_temp));
                                                                             effect_num++;
                                                                             return true;
                                                                         });

            if (fetch_count == 0 && !mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int pg_query_vec_cb_impl(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = conn_obj->get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = conn_obj->get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            unsigned int fetch_count = pg_select_conn->fetch_directly(rawsql,
                                                                      [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                      {
                                                                          T data_temp;
                                                                          // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                          if (col_cache.empty() && col_count > 0)
                                                                          {
                                                                              col_cache.reserve(col_count);
                                                                              for (int k = 0; k < col_count; k++)
                                                                              {
                                                                                  col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                              }
                                                                          }
                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                          {
                                                                              auto [ptr, len] = get_data(ij);
                                                                              if (ptr == nullptr)
                                                                              {
                                                                                  continue;
                                                                              }
                                                                              if (!col_cache[ij].empty())
                                                                              {
                                                                                  std::invoke(callback, data_temp, col_cache[ij], ptr, len, ij % 255, 1);
                                                                              }
                                                                          }
                                                                          result_record.emplace_back(std::move(data_temp));
                                                                          effect_num++;
                                                                          return true;
                                                                      });

            if (fetch_count == 0 && !pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int sqlite_query_vec_cb_impl(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = conn_obj->get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = conn_obj->get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            unsigned int fetch_count = sqlite_select_conn->fetch_directly(rawsql,
                                                                          [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                          {
                                                                              T data_temp;
                                                                              // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                              if (col_cache.empty() && col_count > 0)
                                                                              {
                                                                                  col_cache.reserve(col_count);
                                                                                  for (int k = 0; k < col_count; k++)
                                                                                  {
                                                                                      col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                  }
                                                                              }
                                                                              for (int ij = 0; ij < col_count; ij++)
                                                                              {
                                                                                  auto [ptr, len] = get_data(ij);
                                                                                  if (ptr == nullptr)
                                                                                  {
                                                                                      continue;
                                                                                  }
                                                                                  if (!col_cache[ij].empty())
                                                                                  {
                                                                                      std::invoke(callback, data_temp, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                  }
                                                                              }
                                                                              result_record.emplace_back(std::move(data_temp));
                                                                              effect_num++;
                                                                              return true;
                                                                          });

            if (fetch_count == 0 && !sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int query(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_query_vec_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_query_vec_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        return pg_query_vec_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> mysql_async_query_vec_cb_impl(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }
            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await mysql_select_conn->async_fetch_directly(rawsql,
                                                                                        [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                        {
                                                                                            T data_temp;
                                                                                            // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                            if (col_cache.empty() && col_count > 0)
                                                                                            {
                                                                                                col_cache.reserve(col_count);
                                                                                                for (int k = 0; k < col_count; k++)
                                                                                                {
                                                                                                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                                }
                                                                                            }
                                                                                            for (int ij = 0; ij < col_count; ij++)
                                                                                            {
                                                                                                auto [ptr, len] = get_data(ij);
                                                                                                if (ptr == nullptr)
                                                                                                {
                                                                                                    continue;
                                                                                                }
                                                                                                if (!col_cache[ij].empty())
                                                                                                {
                                                                                                    std::invoke(callback, data_temp, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                                }
                                                                                            }
                                                                                            result_record.emplace_back(std::move(data_temp));
                                                                                            effect_num++;
                                                                                            return true;
                                                                                        });

            if (fetch_count == 0 && !mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> pg_async_query_vec_cb_impl(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await pg_select_conn->async_fetch_directly(rawsql,
                                                                                     [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                     {
                                                                                         T data_temp;
                                                                                         // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                         if (col_cache.empty() && col_count > 0)
                                                                                         {
                                                                                             col_cache.reserve(col_count);
                                                                                             for (int k = 0; k < col_count; k++)
                                                                                             {
                                                                                                 col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                             }
                                                                                         }
                                                                                         for (int ij = 0; ij < col_count; ij++)
                                                                                         {
                                                                                             auto [ptr, len] = get_data(ij);
                                                                                             if (ptr == nullptr)
                                                                                             {
                                                                                                 continue;
                                                                                             }
                                                                                             if (!col_cache[ij].empty())
                                                                                             {
                                                                                                 std::invoke(callback, data_temp, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                             }
                                                                                         }
                                                                                         result_record.emplace_back(std::move(data_temp));
                                                                                         effect_num++;
                                                                                         return true;
                                                                                     });

            if (fetch_count == 0 && !pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> sqlite_async_query_vec_cb_impl(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await sqlite_select_conn->async_fetch_directly(rawsql,
                                                                                         [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                         {
                                                                                             T data_temp;
                                                                                             // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                             if (col_cache.empty() && col_count > 0)
                                                                                             {
                                                                                                 col_cache.reserve(col_count);
                                                                                                 for (int k = 0; k < col_count; k++)
                                                                                                 {
                                                                                                     col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                                 }
                                                                                             }
                                                                                             for (int ij = 0; ij < col_count; ij++)
                                                                                             {
                                                                                                 auto [ptr, len] = get_data(ij);
                                                                                                 if (ptr == nullptr)
                                                                                                 {
                                                                                                     continue;
                                                                                                 }
                                                                                                 if (!col_cache[ij].empty())
                                                                                                 {
                                                                                                     std::invoke(callback, data_temp, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                                 }
                                                                                             }
                                                                                             result_record.emplace_back(std::move(data_temp));
                                                                                             effect_num++;
                                                                                             return true;
                                                                                         });

            if (fetch_count == 0 && !sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                co_return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_query_vec_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_query_vec_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        co_return co_await pg_async_query_vec_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int mysql_query_single_cb_impl(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = conn_obj->get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = conn_obj->get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            unsigned int fetch_count = mysql_select_conn->fetch_directly(rawsql,
                                                                         [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                         {
                                                                             // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                             if (col_cache.empty() && col_count > 0)
                                                                             {
                                                                                 col_cache.reserve(col_count);
                                                                                 for (int k = 0; k < col_count; k++)
                                                                                 {
                                                                                     col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                 }
                                                                             }
                                                                             for (int ij = 0; ij < col_count; ij++)
                                                                             {
                                                                                 auto [ptr, len] = get_data(ij);
                                                                                 if (ptr == nullptr)
                                                                                 {
                                                                                     continue;
                                                                                 }
                                                                                 if (!col_cache[ij].empty())
                                                                                 {
                                                                                     std::invoke(callback, result_record, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                 }
                                                                             }
                                                                             effect_num++;
                                                                             return false;
                                                                         });

            if (fetch_count == 0 && !mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int pg_query_single_cb_impl(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = conn_obj->get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = conn_obj->get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            unsigned int fetch_count = pg_select_conn->fetch_directly(rawsql,
                                                                      [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                      {
                                                                          // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                          if (col_cache.empty() && col_count > 0)
                                                                          {
                                                                              col_cache.reserve(col_count);
                                                                              for (int k = 0; k < col_count; k++)
                                                                              {
                                                                                  col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                              }
                                                                          }
                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                          {
                                                                              auto [ptr, len] = get_data(ij);
                                                                              if (ptr == nullptr)
                                                                              {
                                                                                  continue;
                                                                              }
                                                                              if (!col_cache[ij].empty())
                                                                              {
                                                                                  std::invoke(callback, result_record, col_cache[ij], ptr, len, ij % 255, 1);
                                                                              }
                                                                          }
                                                                          effect_num++;
                                                                          return false;// 只取首行
                                                                      });

            if (fetch_count == 0 && !pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int sqlite_query_single_cb_impl(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = conn_obj->get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = conn_obj->get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            unsigned int fetch_count = sqlite_select_conn->fetch_directly(rawsql,
                                                                          [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                          {
                                                                              // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                              if (col_cache.empty() && col_count > 0)
                                                                              {
                                                                                  col_cache.reserve(col_count);
                                                                                  for (int k = 0; k < col_count; k++)
                                                                                  {
                                                                                      col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                  }
                                                                              }
                                                                              for (int ij = 0; ij < col_count; ij++)
                                                                              {
                                                                                  auto [ptr, len] = get_data(ij);
                                                                                  if (ptr == nullptr)
                                                                                  {
                                                                                      continue;
                                                                                  }
                                                                                  if (!col_cache[ij].empty())
                                                                                  {
                                                                                      std::invoke(callback, result_record, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                  }
                                                                              }
                                                                              effect_num++;
                                                                              return false;// only fetch first row
                                                                          });

            if (fetch_count == 0 && !sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int query(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        return pg_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> mysql_async_query_single_cb_impl(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }
            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await mysql_select_conn->async_fetch_directly(rawsql,
                                                                                        [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                        {
                                                                                            // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                            if (col_cache.empty() && col_count > 0)
                                                                                            {
                                                                                                col_cache.reserve(col_count);
                                                                                                for (int k = 0; k < col_count; k++)
                                                                                                {
                                                                                                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                                }
                                                                                            }
                                                                                            for (int ij = 0; ij < col_count; ij++)
                                                                                            {
                                                                                                auto [ptr, len] = get_data(ij);
                                                                                                if (ptr == nullptr)
                                                                                                {
                                                                                                    continue;
                                                                                                }
                                                                                                if (!col_cache[ij].empty())
                                                                                                {
                                                                                                    std::invoke(callback, result_record, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                                }
                                                                                            }
                                                                                            effect_num++;
                                                                                            return false;
                                                                                        });

            if (fetch_count == 0 && !mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> pg_async_query_single_cb_impl(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await pg_select_conn->async_fetch_directly(rawsql,
                                                                                     [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                     {
                                                                                         // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                         if (col_cache.empty() && col_count > 0)
                                                                                         {
                                                                                             col_cache.reserve(col_count);
                                                                                             for (int k = 0; k < col_count; k++)
                                                                                             {
                                                                                                 col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                             }
                                                                                         }
                                                                                         for (int ij = 0; ij < col_count; ij++)
                                                                                         {
                                                                                             auto [ptr, len] = get_data(ij);
                                                                                             if (ptr == nullptr)
                                                                                             {
                                                                                                 continue;
                                                                                             }
                                                                                             if (!col_cache[ij].empty())
                                                                                             {
                                                                                                 std::invoke(callback, result_record, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                             }
                                                                                         }
                                                                                         effect_num++;
                                                                                         return false;// 只取首行
                                                                                     });

            if (fetch_count == 0 && !pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> sqlite_async_query_single_cb_impl(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            unsigned int fetch_count = co_await sqlite_select_conn->async_fetch_directly(rawsql,
                                                                                         [this, &result_record, callback = std::forward<Callback>(callback), col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                         {
                                                                                             // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                             if (col_cache.empty() && col_count > 0)
                                                                                             {
                                                                                                 col_cache.reserve(col_count);
                                                                                                 for (int k = 0; k < col_count; k++)
                                                                                                 {
                                                                                                     col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                                 }
                                                                                             }
                                                                                             for (int ij = 0; ij < col_count; ij++)
                                                                                             {
                                                                                                 auto [ptr, len] = get_data(ij);
                                                                                                 if (ptr == nullptr)
                                                                                                 {
                                                                                                     continue;
                                                                                                 }
                                                                                                 if (!col_cache[ij].empty())
                                                                                                 {
                                                                                                     std::invoke(callback, result_record, col_cache[ij], ptr, len, ij % 255, 1);
                                                                                                 }
                                                                                             }
                                                                                             effect_num++;
                                                                                             return false;// only fetch first row
                                                                                         });

            if (fetch_count == 0 && !sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                co_return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        co_return co_await pg_async_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
    }

    ////5555 exec prepared SELECT: 预编译 + 参数绑定，值仍按文本落地
    //  所有 exec_* / async_exec_* / query / edit_query / begin / commit 在函数开头
    //  入口 error_msg.clear()，成功路径不写 error_msg，故调用返回后
    // 行循环本身在 orm_common.h 的 4 个共享模板里，这里只注入落地句：

    template <ResultHasSetVal T>
    unsigned int mysql_exec_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = conn_obj->get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = conn_obj->get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *mysql_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int pg_exec_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = conn_obj->get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = conn_obj->get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *pg_select_conn,
                sql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int sqlite_exec_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = conn_obj->get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = conn_obj->get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *sqlite_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int exec_query(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        error_msg.clear();
        if (!sql_is_select(rawsql))
        {
            error_msg  = "exec_query: only SELECT is allowed, use exec_edit_query for this statement";
            effect_num = 0;
            return 0;
        }
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_exec_query_vec_impl(rawsql, params, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_exec_query_vec_impl(rawsql, params, result_record);
        }
        return pg_exec_query_vec_impl(rawsql, params, result_record);
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> mysql_async_exec_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *mysql_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> pg_async_exec_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *pg_select_conn,
                sql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> sqlite_async_exec_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *sqlite_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                co_return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_exec_query(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        error_msg.clear();
        if (!sql_is_select(rawsql))
        {
            error_msg  = "exec_query: only SELECT is allowed, use exec_edit_query for this statement";
            effect_num = 0;
            co_return 0;
        }
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_exec_query_vec_impl(rawsql, params, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_exec_query_vec_impl(rawsql, params, result_record);
        }
        co_return co_await pg_async_exec_query_vec_impl(rawsql, params, result_record);
    }

    template <ResultHasSetVal T>
    unsigned int mysql_exec_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = conn_obj->get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = conn_obj->get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *mysql_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int pg_exec_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = conn_obj->get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = conn_obj->get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *pg_select_conn,
                sql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int sqlite_exec_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = conn_obj->get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = conn_obj->get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *sqlite_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int exec_query(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        error_msg.clear();
        if (!sql_is_select(rawsql))
        {
            error_msg  = "exec_query: only SELECT is allowed, use exec_edit_query for this statement";
            effect_num = 0;
            return 0;
        }
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_exec_query_single_impl(rawsql, params, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_exec_query_single_impl(rawsql, params, result_record);
        }
        return pg_exec_query_single_impl(rawsql, params, result_record);
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> mysql_async_exec_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *mysql_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> pg_async_exec_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *pg_select_conn,
                sql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> sqlite_async_exec_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *sqlite_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                co_return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_exec_query(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        error_msg.clear();
        if (!sql_is_select(rawsql))
        {
            error_msg  = "exec_query: only SELECT is allowed, use exec_edit_query for this statement";
            effect_num = 0;
            co_return 0;
        }
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_exec_query_single_impl(rawsql, params, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_exec_query_single_impl(rawsql, params, result_record);
        }
        co_return co_await pg_async_exec_query_single_impl(rawsql, params, result_record);
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int mysql_exec_query_vec_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = conn_obj->get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = conn_obj->get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *mysql_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int pg_exec_query_vec_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = conn_obj->get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = conn_obj->get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *pg_select_conn,
                sql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int sqlite_exec_query_vec_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = conn_obj->get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = conn_obj->get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *sqlite_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int exec_query(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        error_msg.clear();
        if (!sql_is_select(rawsql))
        {
            error_msg  = "exec_query: only SELECT is allowed, use exec_edit_query for this statement";
            effect_num = 0;
            return 0;
        }
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_exec_query_vec_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_exec_query_vec_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
        }
        return pg_exec_query_vec_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> mysql_async_exec_query_vec_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *mysql_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> pg_async_exec_query_vec_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *pg_select_conn,
                sql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> sqlite_async_exec_query_vec_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *sqlite_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                co_return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> async_exec_query(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        error_msg.clear();
        if (!sql_is_select(rawsql))
        {
            error_msg  = "exec_query: only SELECT is allowed, use exec_edit_query for this statement";
            effect_num = 0;
            co_return 0;
        }
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_exec_query_vec_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_exec_query_vec_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
        }
        co_return co_await pg_async_exec_query_vec_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int mysql_exec_query_single_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = conn_obj->get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = conn_obj->get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *mysql_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int pg_exec_query_single_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = conn_obj->get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = conn_obj->get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *pg_select_conn,
                sql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int sqlite_exec_query_single_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = conn_obj->get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = conn_obj->get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *sqlite_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    unsigned int exec_query(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        error_msg.clear();
        if (!sql_is_select(rawsql))
        {
            error_msg  = "exec_query: only SELECT is allowed, use exec_edit_query for this statement";
            effect_num = 0;
            return 0;
        }
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_exec_query_single_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_exec_query_single_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
        }
        return pg_exec_query_single_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> mysql_async_exec_query_single_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!mysql_select_conn)
                {
                    mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
            }
            else
            {
                mysql_select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *mysql_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!mysql_select_conn->error_msg.empty())
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
            }

            if (mysql_select_conn->isdebug)
            {
                mysql_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = mysql_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(mysql_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> pg_async_exec_query_single_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!pg_select_conn)
                {
                    pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
            }
            else
            {
                pg_select_conn = co_await conn_obj->async_get_pg_select_conn();
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *pg_select_conn,
                sql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (pg_select_conn->isdebug)
            {
                pg_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = pg_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> sqlite_async_exec_query_single_cb_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

            if (conn_obj == nullptr)
            {
                error_msg = "Please select_db() tag";
                co_return 0;
            }

            if (islock_conn)
            {
                if (!sqlite_select_conn)
                {
                    sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
                }
            }
            else
            {
                sqlite_select_conn = co_await conn_obj->async_get_sqlite_select_conn();
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->begin_time();
            }

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *sqlite_select_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!sqlite_select_conn->error_msg.empty())
            {
                error_msg = sqlite_select_conn->error_msg;
                sqlite_select_conn.reset();
                co_return 0;
            }

            if (sqlite_select_conn->isdebug)
            {
                sqlite_select_conn->finish_time();
                auto &conn_mar    = get_orm_connect_mar();
                long long du_time = sqlite_select_conn->count_time();
                conn_mar.push_log(rawsql, std::to_string(du_time));
            }
            if (!islock_conn)
            {
                conn_obj->back_sqlite_select_conn(std::move(sqlite_select_conn));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
        }

        co_return 0;
    }

    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> async_exec_query(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        error_msg.clear();
        if (!sql_is_select(rawsql))
        {
            error_msg  = "exec_query: only SELECT is allowed, use exec_edit_query for this statement";
            effect_num = 0;
            co_return 0;
        }
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_exec_query_single_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_exec_query_single_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
        }
        co_return co_await pg_async_exec_query_single_cb_impl(rawsql, params, result_record, std::forward<Callback>(callback));
    }

    ////5555 exec prepared DML 带结果集，预编译 + 参数绑定，走 **edit 连接**

    template <ResultHasSetVal T>
    unsigned int mysql_exec_edit_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *mysql_edit_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!mysql_edit_conn->error_msg.empty())
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int pg_exec_edit_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *pg_edit_conn,
                sql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_edit_conn->error_msg.empty())
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int sqlite_exec_edit_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_rows(
                *sqlite_edit_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!sqlite_edit_conn->error_msg.empty())
            {
                error_msg = sqlite_edit_conn->error_msg;
                sqlite_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int exec_edit_query(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_exec_edit_query_vec_impl(rawsql, params, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_exec_edit_query_vec_impl(rawsql, params, result_record);
        }
        return pg_exec_edit_query_vec_impl(rawsql, params, result_record);
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> mysql_async_exec_edit_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *mysql_edit_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!mysql_edit_conn->error_msg.empty())
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> pg_async_exec_edit_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                co_return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *pg_edit_conn,
                sql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_edit_conn->error_msg.empty())
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> sqlite_async_exec_edit_query_vec_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_rows(
                *sqlite_edit_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!sqlite_edit_conn->error_msg.empty())
            {
                error_msg = sqlite_edit_conn->error_msg;
                sqlite_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_exec_edit_query(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_exec_edit_query_vec_impl(rawsql, params, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_exec_edit_query_vec_impl(rawsql, params, result_record);
        }
        co_return co_await pg_async_exec_edit_query_vec_impl(rawsql, params, result_record);
    }

    template <ResultHasSetVal T>
    unsigned int mysql_exec_edit_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *mysql_edit_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!mysql_edit_conn->error_msg.empty())
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int pg_exec_edit_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *pg_edit_conn,
                sql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_edit_conn->error_msg.empty())
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int sqlite_exec_edit_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = exec_fetch_single(
                *sqlite_edit_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!sqlite_edit_conn->error_msg.empty())
            {
                error_msg = sqlite_edit_conn->error_msg;
                sqlite_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int exec_edit_query(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_exec_edit_query_single_impl(rawsql, params, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_exec_edit_query_single_impl(rawsql, params, result_record);
        }
        return pg_exec_edit_query_single_impl(rawsql, params, result_record);
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> mysql_async_exec_edit_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *mysql_edit_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!mysql_edit_conn->error_msg.empty())
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> pg_async_exec_edit_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            unsigned int ph_count = 0;
            const std::string sql = pg_qmark_to_dollar(rawsql, ph_count);
            if (!exec_bind_guard(ph_count, params, error_msg))
            {
                co_return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *pg_edit_conn,
                sql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_edit_conn->error_msg.empty())
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> sqlite_async_exec_edit_query_single_impl(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

        try
        {
            if (!exec_bind_guard(count_qmark(rawsql), params, error_msg))
            {
                co_return 0;
            }

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

            [[maybe_unused]] unsigned int fetch_count = co_await async_exec_fetch_single(
                *sqlite_edit_conn,
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!sqlite_edit_conn->error_msg.empty())
            {
                error_msg = sqlite_edit_conn->error_msg;
                sqlite_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_exec_edit_query(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_exec_edit_query_single_impl(rawsql, params, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_exec_edit_query_single_impl(rawsql, params, result_record);
        }
        co_return co_await pg_async_exec_edit_query_single_impl(rawsql, params, result_record);
    }

    ////3333 commit

    template <ResultHasSetVal T>
    unsigned int mysql_edit_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = mysql_edit_conn->fetch_directly(rawsql,
                                                                       [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                       {
                                                                           T data_temp;
                                                                           // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                           if (col_cache.empty() && col_count > 0)
                                                                           {
                                                                               col_cache.reserve(col_count);
                                                                               for (int k = 0; k < col_count; k++)
                                                                               {
                                                                                   col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                               }
                                                                           }
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                               {
                                                                                   continue;
                                                                               }
                                                                               if (!col_cache[ij].empty())
                                                                               {
                                                                                   data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                               }
                                                                           }
                                                                           result_record.emplace_back(std::move(data_temp));
                                                                           effect_num++;
                                                                           return true;
                                                                       });

            if (fetch_count == 0 && !mysql_edit_conn->error_msg.empty())
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                return 0;
            }
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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int pg_edit_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = pg_edit_conn->fetch_directly(rawsql,
                                                                    [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                                                                        T data_temp;
                                                                        // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                        if (col_cache.empty() && col_count > 0)
                                                                        {
                                                                            col_cache.reserve(col_count);
                                                                            for (int k = 0; k < col_count; k++)
                                                                            {
                                                                                col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                            }
                                                                        }
                                                                        for (int ij = 0; ij < col_count; ij++)
                                                                        {
                                                                            auto [ptr, len] = get_data(ij);
                                                                            if (ptr == nullptr)
                                                                            {
                                                                                continue;
                                                                            }
                                                                            if (!col_cache[ij].empty())
                                                                            {
                                                                                data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                            }
                                                                        }
                                                                        result_record.emplace_back(std::move(data_temp));
                                                                        effect_num++;
                                                                        return true;
                                                                    });

            if (fetch_count == 0 && !pg_edit_conn->error_msg.empty())
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int sqlite_edit_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = sqlite_edit_conn->fetch_directly(rawsql,
                                                                        [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                        {
                                                                            T data_temp;
                                                                            // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                            if (col_cache.empty() && col_count > 0)
                                                                            {
                                                                                col_cache.reserve(col_count);
                                                                                for (int k = 0; k < col_count; k++)
                                                                                {
                                                                                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                }
                                                                            }
                                                                            for (int ij = 0; ij < col_count; ij++)
                                                                            {
                                                                                auto [ptr, len] = get_data(ij);
                                                                                if (ptr == nullptr)
                                                                                {
                                                                                    continue;
                                                                                }
                                                                                if (!col_cache[ij].empty())
                                                                                {
                                                                                    data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                }
                                                                            }
                                                                            result_record.emplace_back(std::move(data_temp));
                                                                            effect_num++;
                                                                            return true;
                                                                        });

            if (fetch_count == 0 && !sqlite_edit_conn->error_msg.empty())
            {
                error_msg = sqlite_edit_conn->error_msg;
                sqlite_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int edit_query(const std::string &rawsql, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_edit_query_vec_impl(rawsql, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_edit_query_vec_impl(rawsql, result_record);
        }
        return pg_edit_query_vec_impl(rawsql, result_record);
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> mysql_async_edit_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

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
            unsigned int fetch_count = co_await mysql_edit_conn->async_fetch_directly(rawsql,
                                                                                      [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                      {
                                                                                          T data_temp;
                                                                                          // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                          if (col_cache.empty() && col_count > 0)
                                                                                          {
                                                                                              col_cache.reserve(col_count);
                                                                                              for (int k = 0; k < col_count; k++)
                                                                                              {
                                                                                                  col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                              }
                                                                                          }
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                              {
                                                                                                  continue;
                                                                                              }
                                                                                              if (!col_cache[ij].empty())
                                                                                              {
                                                                                                  data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                              }
                                                                                          }
                                                                                          result_record.emplace_back(std::move(data_temp));
                                                                                          effect_num++;
                                                                                          return true;
                                                                                      });

            if (fetch_count == 0 && !mysql_edit_conn->error_msg.empty())
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                co_return 0;
            }
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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> pg_async_edit_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = co_await pg_edit_conn->async_fetch_directly(rawsql,
                                                                                   [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                   {
                                                                                       T data_temp;
                                                                                       // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                       if (col_cache.empty() && col_count > 0)
                                                                                       {
                                                                                           col_cache.reserve(col_count);
                                                                                           for (int k = 0; k < col_count; k++)
                                                                                           {
                                                                                               col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                           }
                                                                                       }
                                                                                       for (int ij = 0; ij < col_count; ij++)
                                                                                       {
                                                                                           auto [ptr, len] = get_data(ij);
                                                                                           if (ptr == nullptr)
                                                                                           {
                                                                                               continue;
                                                                                           }
                                                                                           if (!col_cache[ij].empty())
                                                                                           {
                                                                                               data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                           }
                                                                                       }
                                                                                       result_record.emplace_back(std::move(data_temp));
                                                                                       effect_num++;
                                                                                       return true;
                                                                                   });

            if (fetch_count == 0 && !pg_edit_conn->error_msg.empty())
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> sqlite_async_edit_query_vec_impl(const std::string &rawsql, std::vector<T> &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = co_await sqlite_edit_conn->async_fetch_directly(rawsql,
                                                                                       [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                       {
                                                                                           T data_temp;
                                                                                           // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                           if (col_cache.empty() && col_count > 0)
                                                                                           {
                                                                                               col_cache.reserve(col_count);
                                                                                               for (int k = 0; k < col_count; k++)
                                                                                               {
                                                                                                   col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                               }
                                                                                           }
                                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                                           {
                                                                                               auto [ptr, len] = get_data(ij);
                                                                                               if (ptr == nullptr)
                                                                                               {
                                                                                                   continue;
                                                                                               }
                                                                                               if (!col_cache[ij].empty())
                                                                                               {
                                                                                                   data_temp.set_val(col_cache[ij], ptr, len, 0);
                                                                                               }
                                                                                           }
                                                                                           result_record.emplace_back(std::move(data_temp));
                                                                                           effect_num++;
                                                                                           return true;
                                                                                       });

            if (fetch_count == 0 && !sqlite_edit_conn->error_msg.empty())
            {
                error_msg = sqlite_edit_conn->error_msg;
                sqlite_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_edit_query(const std::string &rawsql, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_edit_query_vec_impl(rawsql, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_edit_query_vec_impl(rawsql, result_record);
        }
        co_return co_await pg_async_edit_query_vec_impl(rawsql, result_record);
    }

    template <ResultHasSetVal T>
    unsigned int mysql_edit_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = mysql_edit_conn->fetch_directly(rawsql,
                                                                       [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                       {
                                                                           // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                           if (col_cache.empty() && col_count > 0)
                                                                           {
                                                                               col_cache.reserve(col_count);
                                                                               for (int k = 0; k < col_count; k++)
                                                                               {
                                                                                   col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                               }
                                                                           }
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                               {
                                                                                   continue;
                                                                               }
                                                                               if (!col_cache[ij].empty())
                                                                               {
                                                                                   result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                               }
                                                                           }
                                                                           effect_num++;
                                                                           return false;
                                                                       });

            if (fetch_count == 0 && !mysql_edit_conn->error_msg.empty())
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                return 0;
            }
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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int pg_edit_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = pg_edit_conn->fetch_directly(rawsql,
                                                                    [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                                                                        // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                        if (col_cache.empty() && col_count > 0)
                                                                        {
                                                                            col_cache.reserve(col_count);
                                                                            for (int k = 0; k < col_count; k++)
                                                                            {
                                                                                col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                            }
                                                                        }
                                                                        for (int ij = 0; ij < col_count; ij++)
                                                                        {
                                                                            auto [ptr, len] = get_data(ij);
                                                                            if (ptr == nullptr)
                                                                            {
                                                                                continue;
                                                                            }
                                                                            if (!col_cache[ij].empty())
                                                                            {
                                                                                result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                            }
                                                                        }
                                                                        effect_num++;
                                                                        return false;// 只取首行
                                                                    });

            if (fetch_count == 0 && !pg_edit_conn->error_msg.empty())
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int sqlite_edit_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = sqlite_edit_conn->fetch_directly(rawsql,
                                                                        [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                        {
                                                                            // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                            if (col_cache.empty() && col_count > 0)
                                                                            {
                                                                                col_cache.reserve(col_count);
                                                                                for (int k = 0; k < col_count; k++)
                                                                                {
                                                                                    col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                }
                                                                            }
                                                                            for (int ij = 0; ij < col_count; ij++)
                                                                            {
                                                                                auto [ptr, len] = get_data(ij);
                                                                                if (ptr == nullptr)
                                                                                {
                                                                                    continue;
                                                                                }
                                                                                if (!col_cache[ij].empty())
                                                                                {
                                                                                    result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                }
                                                                            }
                                                                            effect_num++;
                                                                            return false;// only fetch first row
                                                                        });

            if (fetch_count == 0 && !sqlite_edit_conn->error_msg.empty())
            {
                error_msg = sqlite_edit_conn->error_msg;
                sqlite_edit_conn.reset();
                return 0;
            }

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
        }

        return 0;
    }

    template <ResultHasSetVal T>
    unsigned int edit_query(const std::string &rawsql, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_edit_query_single_impl(rawsql, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            return sqlite_edit_query_single_impl(rawsql, result_record);
        }
        return pg_edit_query_single_impl(rawsql, result_record);
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> mysql_async_edit_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = co_await mysql_edit_conn->async_fetch_directly(rawsql,
                                                                                      [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                      {
                                                                                          // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                          if (col_cache.empty() && col_count > 0)
                                                                                          {
                                                                                              col_cache.reserve(col_count);
                                                                                              for (int k = 0; k < col_count; k++)
                                                                                              {
                                                                                                  col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                              }
                                                                                          }
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                              {
                                                                                                  continue;
                                                                                              }
                                                                                              if (!col_cache[ij].empty())
                                                                                              {
                                                                                                  result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                              }
                                                                                          }
                                                                                          effect_num++;
                                                                                          return false;
                                                                                      });

            if (fetch_count == 0 && !mysql_edit_conn->error_msg.empty())
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                co_return 0;
            }
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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> pg_async_edit_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = co_await pg_edit_conn->async_fetch_directly(rawsql,
                                                                                   [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                   {
                                                                                       // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                       if (col_cache.empty() && col_count > 0)
                                                                                       {
                                                                                           col_cache.reserve(col_count);
                                                                                           for (int k = 0; k < col_count; k++)
                                                                                           {
                                                                                               col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                           }
                                                                                       }
                                                                                       for (int ij = 0; ij < col_count; ij++)
                                                                                       {
                                                                                           auto [ptr, len] = get_data(ij);
                                                                                           if (ptr == nullptr)
                                                                                           {
                                                                                               continue;
                                                                                           }
                                                                                           if (!col_cache[ij].empty())
                                                                                           {
                                                                                               result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                           }
                                                                                       }
                                                                                       effect_num++;
                                                                                       return false;// 只取首行
                                                                                   });

            if (fetch_count == 0 && !pg_edit_conn->error_msg.empty())
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> sqlite_async_edit_query_single_impl(const std::string &rawsql, T &result_record)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();

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

            unsigned int fetch_count = co_await sqlite_edit_conn->async_fetch_directly(rawsql,
                                                                                       [this, &result_record, col_cache = sqlite_conn_col_name_cache_t{}](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                       {
                                                                                           // 列名缓存：同一查询列名不变，首行构建一次，避免每行每列构造临时 std::string
                                                                                           if (col_cache.empty() && col_count > 0)
                                                                                           {
                                                                                               col_cache.reserve(col_count);
                                                                                               for (int k = 0; k < col_count; k++)
                                                                                               {
                                                                                                   col_cache.emplace_back(col_names[k] ? col_names[k] : "");
                                                                                               }
                                                                                           }
                                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                                           {
                                                                                               auto [ptr, len] = get_data(ij);
                                                                                               if (ptr == nullptr)
                                                                                               {
                                                                                                   continue;
                                                                                               }
                                                                                               if (!col_cache[ij].empty())
                                                                                               {
                                                                                                   result_record.set_val(col_cache[ij], ptr, len, 0);
                                                                                               }
                                                                                           }
                                                                                           effect_num++;
                                                                                           return false;// only fetch first row
                                                                                       });

            if (fetch_count == 0 && !sqlite_edit_conn->error_msg.empty())
            {
                error_msg = sqlite_edit_conn->error_msg;
                sqlite_edit_conn.reset();
                co_return 0;
            }

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
        }

        co_return 0;
    }

    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_edit_query(const std::string &rawsql, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_edit_query_single_impl(rawsql, result_record);
        }
        else if (db_type == DB_TYPE::SQLITE)
        {
            co_return co_await sqlite_async_edit_query_single_impl(rawsql, result_record);
        }
        co_return co_await pg_async_edit_query_single_impl(rawsql, result_record);
    }

    bool begin_commit();
    bool commit();
    void rollback();

    asio::awaitable<bool> async_begin_commit();
    asio::awaitable<bool> async_commit();
    asio::awaitable<void> async_rollback();

  private:
    //// transaction impl (mysql / pg / sqlite)
    bool mysql_begin_commit_impl();
    bool pg_begin_commit_impl();
    bool sqlite_begin_commit_impl();
    bool mysql_commit_impl();
    bool pg_commit_impl();
    bool sqlite_commit_impl();
    void mysql_rollback_impl();
    void pg_rollback_impl();
    void sqlite_rollback_impl();

    asio::awaitable<bool> mysql_async_begin_commit_impl();
    asio::awaitable<bool> pg_async_begin_commit_impl();
    asio::awaitable<bool> sqlite_async_begin_commit_impl();
    asio::awaitable<bool> mysql_async_commit_impl();
    asio::awaitable<bool> pg_async_commit_impl();
    asio::awaitable<bool> sqlite_async_commit_impl();
    asio::awaitable<void> mysql_async_rollback_impl();
    asio::awaitable<void> pg_async_rollback_impl();
    asio::awaitable<void> sqlite_async_rollback_impl();

    //// raw edit sql impl (mysql / pg / sqlite)
    unsigned int mysql_edit_query_impl(const std::string &rawsql);
    unsigned int pg_edit_query_impl(const std::string &rawsql);
    unsigned int sqlite_edit_query_impl(const std::string &rawsql);
    asio::awaitable<unsigned int> mysql_async_edit_query_impl(const std::string &rawsql);
    asio::awaitable<unsigned int> pg_async_edit_query_impl(const std::string &rawsql);
    asio::awaitable<unsigned int> sqlite_async_edit_query_impl(const std::string &rawsql);

    //// exec prepared DML-only impl (mysql / pg / sqlite)
    unsigned int mysql_exec_edit_query_impl(const std::string &rawsql, const std::vector<http::obj_val> &params);
    unsigned int pg_exec_edit_query_impl(const std::string &rawsql, const std::vector<http::obj_val> &params);
    unsigned int sqlite_exec_edit_query_impl(const std::string &rawsql, const std::vector<http::obj_val> &params);
    asio::awaitable<unsigned int> mysql_async_exec_edit_query_impl(const std::string &rawsql,
                                                                   const std::vector<http::obj_val> &params);
    asio::awaitable<unsigned int> pg_async_exec_edit_query_impl(const std::string &rawsql,
                                                                const std::vector<http::obj_val> &params);
    asio::awaitable<unsigned int> sqlite_async_exec_edit_query_impl(const std::string &rawsql,
                                                                    const std::vector<http::obj_val> &params);

  public:
    void clear()
    {
        iserror     = false;
        iscommit    = false;
        islock_conn = false;
        effect_num  = 0;
        error_msg.clear();
        mysql_select_conn.reset();
        mysql_edit_conn.reset();
        pg_select_conn.reset();
        pg_edit_conn.reset();
        sqlite_select_conn.reset();
        sqlite_edit_conn.reset();
    }

  public:
    unsigned int edit_query(const std::string &);
    asio::awaitable<unsigned int> async_edit_query(const std::string &);

  public:
    //// exec prepared DML-only（src/orm_query.cpp）
    unsigned int exec_edit_query(const std::string &rawsql, const std::vector<http::obj_val> &params);
    asio::awaitable<unsigned int> async_exec_edit_query(const std::string &rawsql,
                                                        const std::vector<http::obj_val> &params);

  public:
    bool iserror            = false;
    bool iscommit           = false;
    bool islock_conn        = false;
    unsigned int effect_num = 0;
    std::string dbtag;
    std::string error_msg;
    std::shared_ptr<mysql_conn_base> mysql_select_conn;
    std::shared_ptr<mysql_conn_base> mysql_edit_conn;
    std::shared_ptr<pg_conn_base> pg_select_conn;
    std::shared_ptr<pg_conn_base> pg_edit_conn;
    std::shared_ptr<sqlite_conn_base> sqlite_select_conn;
    std::shared_ptr<sqlite_conn_base> sqlite_edit_conn;
    std::shared_ptr<orm_conn_pool> conn_obj;
    DB_TYPE db_type = DB_TYPE::MYSQL;
};

}// namespace orm
#endif