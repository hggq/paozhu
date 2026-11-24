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
#include <utility>
#include <tuple>
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
        // 事务未 commit/rollback 就析构：先回滚，否则会把"还开着事务"的连接
        // 归还给连接池，下一个借用者继承这个事务（悬挂事务）
        if (iscommit && conn_obj)
        {
            rollback();
        }
        // 残留连接一律走归还口，不再只在 lock_conn 模式下归还：异常/提前返回的路径
        // 以前直接销毁连接，既不复用也不唤醒本槽等待者。
        islock_conn = false;
        if (conn_obj)
        {
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
    //// 语句级失败收尾：把底层连接的错误搬到 db_conn 上，并置 iserror
    //// 调用方即可用 if (ulink->iserror) { ulink->rollback(); } 判定。
    //// iserror 语义：最近一条语句失败且尚未处理；rollback() 成功或 clear() 后复位。
    template <typename ConnPtr>
    void mark_edit_failed(ConnPtr &conn)
    {
        if (conn && !conn->error_msg.empty())
        {
            error_msg = conn->error_msg;
        }
        iserror = true;
        if (islock_conn)
        {
            // 事务中必须留着这条连接，交给上层 rollback()/commit() 收尾；
            // 直接销毁会让后续 rollback 打在另一条新连接上，变成悬挂事务
            return;
        }
        conn.reset();
        return;
    }

    // PG 专用：SQL 出错后协议已排空到 ReadyForQuery，连接仍可用。交回归还口，由池
    // 按 isclose 决定复用还是丢弃，丢弃时统一唤醒等待者；不能像模板那样直接 reset。
    void mark_select_failed(std::shared_ptr<pg_conn_base> &conn)
    {
        if (conn && !conn->error_msg.empty())
        {
            error_msg = conn->error_msg;
        }
        iserror = true;
        if (!conn)
        {
            return;
        }
        if (islock_conn || !conn_obj)
        {
            return;// 锁模式：留给 unlock_conn()/析构统一归还
        }
        conn_obj->back_pg_select_conn(std::move(conn));
    }

    // 异常中断（分配失败等）：协议状态未知，按断开处理；仍走归还口，保证等待者被唤醒。
    void drop_select_after_exception()
    {
        iserror = true;
        if (!pg_select_conn)
        {
            return;
        }
        if (islock_conn || !conn_obj)
        {
            return;
        }
        pg_select_conn->isclose = true;
        conn_obj->back_pg_select_conn(std::move(pg_select_conn));
    }

    //// 协程帧被直接销毁（取消不会进 catch）时的兜底：查询可能仍在途，服务端还有
    //// 没读完的响应，协议状态未知。必须先 isclose 再交还池，否则下一个借用者会把
    //// 旧响应当成当前查询结果（串数据）。正常/错误/异常路径都已把 pg_select_conn
    //// 移出成员，能命中这里的只有帧销毁一种情况。
    void release_select_on_frame_destroy()
    {
        if (!pg_select_conn || islock_conn || !conn_obj)
        {
            return;
        }
        pg_select_conn->isclose = true;
        conn_obj->back_pg_select_conn(std::move(pg_select_conn));
    }

    // 声明在协程帧作用域（try 之前）：帧正常走完时成员已被移走，析构是空操作；
    // 帧被取消销毁时析构执行 release_select_on_frame_destroy()。
    class select_frame_guard
    {
      public:
        explicit select_frame_guard(db_conn *self) : self_(self) {}
        ~select_frame_guard() { self_->release_select_on_frame_destroy(); }
        select_frame_guard(const select_frame_guard &)            = delete;
        select_frame_guard &operator=(const select_frame_guard &) = delete;

      private:
        db_conn *self_;
    };

    template <typename ConnPtr>
    void mark_select_failed(ConnPtr &conn)
    {
        if (conn && !conn->error_msg.empty())
        {
            error_msg = conn->error_msg;
        }
        iserror = true;
        conn.reset();
        return;
    }

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
                mark_select_failed(mysql_select_conn);
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
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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

    // 预处理查询同步版，与 async_query_prepared 对称；委托 exec_query（MySQL/SQLite 用 ?，PG 直接用 $1..$n）。
    template <ResultHasSetVal T>
    unsigned int query_prepared(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        return exec_query(rawsql, params, result_record);
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
                mark_select_failed(mysql_select_conn);
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
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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

    // prepared 协议专用入口，与 async_query 对称；由调用点显式选用，async_query 的 simple 默认路径不变。
    // 等价于 async_exec_query：只允许 SELECT（MySQL/SQLite 用 ?，PG 直接用 $1..$n），
    // 命名语句缓存按 SQL 文本做 key（pg_conn_base::prepared_meta_），不同参数值复用同一条语句。
    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_query_prepared(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record)
    {
        co_return co_await async_exec_query(rawsql, params, result_record);
    }

    // PG 二进制结果预编译查询：handler 直接按列拿值，跳过 set_val 与行对象，列值只在
    // handler 返回前有效（文本列指向本行 backing，不能保存指针）。handler 返回 true
    // 继续、false 中止；返回处理的行数，失败时 iserror / error_msg 置位。
    // 只有 PG 有二进制结果格式，其他 db_type 直接报错。
    template <BinaryRowHandler H>
    asio::awaitable<unsigned int> async_query_prepared_binary(const std::string &rawsql,
                                                              const std::vector<http::obj_val> &params,
                                                              H handler)
    {
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();
        select_frame_guard frame_guard{this};

        try
        {
            if (db_type != DB_TYPE::POSTGRESQL)
            {
                error_msg = "async_query_prepared_binary: only PostgreSQL has binary row values";
                co_return 0;
            }
            if (!exec_bind_params_guard(params, error_msg))
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

            effect_num = co_await pg_select_conn->async_fetch_prepared_binary(rawsql, params, std::move(handler));

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
                effect_num = 0;
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
            drop_select_after_exception();// 异常退出也要归还，否则连接随对象析构被扔掉且不唤醒等待者
        }

        co_return 0;
    }

    // PG 二进制结果整取：所有行先读进 out，连接在 co_await 返回前已归还，之后再用
    // out.decode_row() 逐行取。与 async_query_prepared_binary 的差别是值不再随行回调失效，
    // 适合"先整取、再排序/渲染"；代价是行载荷多一次拷贝。失败时 out 不可用，返回 0。
    asio::awaitable<unsigned int> async_query_prepared_all(const std::string &rawsql,
                                                           const std::vector<http::obj_val> &params,
                                                           pg_result_set &out)
    {
        out.clear();
        effect_num = 0;
        if (iserror)
        {
            co_return 0;
        }
        error_msg.clear();
        select_frame_guard frame_guard{this};

        try
        {
            if (db_type != DB_TYPE::POSTGRESQL)
            {
                error_msg = "async_query_prepared_all: only PostgreSQL has binary row values";
                co_return 0;
            }
            if (!exec_bind_params_guard(params, error_msg))
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

            effect_num = co_await pg_select_conn->async_fetch_prepared_collect(rawsql, params, out);

            const bool query_failed = !pg_select_conn->error_msg.empty();
            std::string query_error;
            if (query_failed)
            {
                query_error = std::move(pg_select_conn->error_msg);
            }
            const bool debug = pg_select_conn->isdebug;
            long long query_time = 0;
            if (!query_failed && debug)
            {
                pg_select_conn->finish_time();
                query_time = pg_select_conn->count_time();
            }

            // 结果已归 out，后续错误清理和调试日志都不能继续占用 select 连接。
            if (!islock_conn)
            {
                conn_obj->back_pg_select_conn(std::move(pg_select_conn));
            }

            if (query_failed)
            {
                error_msg = std::move(query_error);
                iserror = true;
                effect_num = 0;
                out.clear();
                co_return 0;
            }
            if (debug)
            {
                auto &conn_mar = get_orm_connect_mar();
                conn_mar.push_log(rawsql, std::to_string(query_time));
            }
            co_return effect_num;
        }
        catch (const std::exception &e)
        {
            error_msg = std::string(e.what());
            drop_select_after_exception();// 异常退出也要归还，否则连接随对象析构被扔掉且不唤醒等待者
        }

        co_return 0;
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
                mark_select_failed(mysql_select_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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

    // prepared 协议专用入口（只取首行），语义与上游 async_query 的单行版对称，门禁同 async_exec_query。
    template <ResultHasSetVal T>
    asio::awaitable<unsigned int> async_query_prepared(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record)
    {
        co_return co_await async_exec_query(rawsql, params, result_record);
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
                mark_select_failed(mysql_select_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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

    // prepared 协议专用入口（带逐行回调），与上游 async_query 的回调版对称，门禁同 async_exec_query。
    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> async_query_prepared(const std::string &rawsql, const std::vector<http::obj_val> &params, std::vector<T> &result_record, Callback &&callback)
    {
        co_return co_await async_exec_query(rawsql, params, result_record, std::forward<Callback>(callback));
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
                mark_select_failed(mysql_select_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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
                mark_select_failed(mysql_select_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [callback = std::forward<Callback>(callback)](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int ij) mutable
                {
                    std::invoke(callback, data_temp, name, ptr, len, ij % 255, 1);
                });

            if (!pg_select_conn->error_msg.empty())
            {
                mark_select_failed(pg_select_conn);
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
                mark_select_failed(sqlite_select_conn);
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

    // prepared 协议专用入口（单行 + 回调），与上游 async_query 的单行回调版对称，门禁同 async_exec_query。
    template <typename T, RecordLineCallback<T> Callback>
    asio::awaitable<unsigned int> async_query_prepared(const std::string &rawsql, const std::vector<http::obj_val> &params, T &result_record, Callback &&callback)
    {
        co_return co_await async_exec_query(rawsql, params, result_record, std::forward<Callback>(callback));
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
                mark_edit_failed(mysql_edit_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_edit_conn->error_msg.empty())
            {
                mark_edit_failed(pg_edit_conn);
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
                mark_edit_failed(sqlite_edit_conn);
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
                mark_edit_failed(mysql_edit_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_edit_conn->error_msg.empty())
            {
                mark_edit_failed(pg_edit_conn);
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
                mark_edit_failed(sqlite_edit_conn);
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
                mark_edit_failed(mysql_edit_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_edit_conn->error_msg.empty())
            {
                mark_edit_failed(pg_edit_conn);
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
                mark_edit_failed(sqlite_edit_conn);
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
                mark_edit_failed(mysql_edit_conn);
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
            if (!exec_bind_params_guard(params, error_msg))
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
                rawsql,
                params,
                result_record,
                effect_num,
                [](T &data_temp, const std::string &name, const unsigned char *ptr, std::size_t len, int)
                {
                    data_temp.set_val(name, ptr, len, 0);
                });

            if (!pg_edit_conn->error_msg.empty())
            {
                mark_edit_failed(pg_edit_conn);
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
                mark_edit_failed(sqlite_edit_conn);
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
                mark_edit_failed(mysql_edit_conn);
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
                mark_edit_failed(pg_edit_conn);
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
                mark_edit_failed(sqlite_edit_conn);
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
                mark_edit_failed(mysql_edit_conn);
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
                mark_edit_failed(pg_edit_conn);
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
                mark_edit_failed(sqlite_edit_conn);
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
                mark_edit_failed(mysql_edit_conn);
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
                mark_edit_failed(pg_edit_conn);
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
                mark_edit_failed(sqlite_edit_conn);
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
                mark_edit_failed(mysql_edit_conn);
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
                mark_edit_failed(pg_edit_conn);
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
                mark_edit_failed(sqlite_edit_conn);
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

    //// 事务从 begin_commit() 开始，必须在同一个 db_conn 对象上走完 commit() 或 rollback()。
    //// MySQL/PostgreSQL 的事务只属于这条 db_conn 借到的连接，别的调用方看不到。
    //// SQLite 一个数据库文件只有一条常驻连接：begin_commit() 开的是这条共享会话的事务，
    //// 期间其他调用方（含 ORM 模型自己借连接发出的写）也落在这个事务里，
    //// 由 hold 事务的那条线 commit 或 rollback 一起收尾。
    //// 因此 SQLite 侧要让模型写进同一个事务，必须 lock_conn(sqlite_edit_conn) 借这条连接，
    //// 且事务里只放必要的少量语句，尽快 commit 或 rollback。
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

    //// insert + 返回 {影响行数, 自增主键} impl（移植自各 *orm.hpp 的 save() INSERT 分支）
    std::tuple<unsigned int, unsigned long long> mysql_insert_query_impl(const std::string &rawsql);
    std::tuple<unsigned int, unsigned long long> pg_insert_query_impl(const std::string &rawsql);
    std::tuple<unsigned int, unsigned long long> sqlite_insert_query_impl(const std::string &rawsql);
    asio::awaitable<std::tuple<unsigned int, unsigned long long>> mysql_async_insert_query_impl(const std::string &rawsql);
    asio::awaitable<std::tuple<unsigned int, unsigned long long>> pg_async_insert_query_impl(const std::string &rawsql);
    asio::awaitable<std::tuple<unsigned int, unsigned long long>> sqlite_async_insert_query_impl(const std::string &rawsql);

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

    //// insert + 返回 {影响行数, 自增主键}，等价于各 *orm.hpp save() 的 INSERT 分支。
    //// 只吃裸 SQL、不耦合 ORM 对象；lastid 由调用方手动回填（如 user_m.setPK(lastid)）。
    //// - MySQL/SQLite：自增 id 来自连接隐式状态，rawsql 为普通 INSERT 即可。
    //// - PostgreSQL：rawsql 需自带 "RETURNING <pk>"（可用 commit_insert_returning()），
    ////               否则 lastid 返回 0，需调用方另行取回。
    std::tuple<unsigned int, unsigned long long> insert_query(const std::string &rawsql);
    asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert_query(const std::string &rawsql);

  public:
    //// exec prepared DML-only（src/orm_query.cpp）
    unsigned int exec_edit_query(const std::string &rawsql, const std::vector<http::obj_val> &params);
    asio::awaitable<unsigned int> async_exec_edit_query(const std::string &rawsql,
                                                        const std::vector<http::obj_val> &params);

  public:
    //// iserror：最近一条语句执行失败（query/edit_query/exec_*）或事务 API 误用。
    //// 失败后本对象进入错误态，所有后续语句直接返回 0（不再发 SQL），
    //// 必须先处理：事务场景 rollback()，非事务场景 clear()。
    //// rollback() 执行成功后自动复位，典型写法：
    ////   ulink->begin_commit();
    ////   ulink->edit_query(sql);
    ////   if (ulink->iserror) { ulink->rollback(); } else { ulink->commit(); }
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