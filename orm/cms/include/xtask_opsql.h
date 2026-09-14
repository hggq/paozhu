#ifndef _ORM_CMS_XTASK_OPERATE_H
#define _ORM_CMS_XTASK_OPERATE_H
/*
 *  @author 黄自权 huangziquan
 *  @date 2022-05-04
 *  @update 2025-03-12
 *  @update 2026-06-14 add xxx_fetch_to, leftjoin
 *  @dest ORM MySQL中间连接层
 *  本文件自动生成 This document is automatically generated.
 *  Creation time Mon, 14 Sep 2026 13:39:42 GMT
 */
#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include <set>
#include <string_view>
#include <system_error>
#include <charconv>
#include <thread>
#include "request.h"

#include "unicode.h"
#include "datetime.h"
#include <stdexcept>
#include <iostream>
#include <functional>
#include <tuple>
#include <typeinfo>
#include <memory>
#include <list>
#include <queue>
#include <cmath>
#include <condition_variable>
#include <sstream>
#include <algorithm>
#include <vector>

#include "orm_common.h"
#include "mysql_conn.h"
#include "orm_conn_pool.h"
#include "orm_cache.hpp"
#include "orm_connect_mar.h"
#include "xtask_base.h"
/*baseincludefile*/
namespace orm
{
// mysql Operational SQL middleware
namespace cms
{ /*tagnamespace_replace*/
    template <typename M_MODEL, typename B_BASE>
    class xtask_opsql : public B_BASE
    {
      public:
        xtask_opsql(const std::string &tag) : dbtag(tag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                //必须是 DB_TYPE::MYSQL 类型链接
                if (iter->second->conf_data[0].db_type != DB_TYPE::MYSQL)
                {
                    conn_obj  = nullptr;
                    iserror   = true;
                    error_msg = "conn_pool db type error " + dbtag;
                }
                else
                {
                    conn_obj = iter->second;
                }
            }
            else
            {
                conn_obj  = nullptr;
                iserror   = true;
                error_msg = "conn_pool not found " + dbtag;
            }
        }
        xtask_opsql() : dbtag(B_BASE::_rmstag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                //必须是 DB_TYPE::MYSQL 类型链接
                if (iter->second->conf_data[0].db_type != DB_TYPE::MYSQL)
                {
                    conn_obj  = nullptr;
                    iserror   = true;
                    error_msg = "conn_pool db type error " + dbtag;
                }
                else
                {
                    conn_obj = iter->second;
                }
            }
            else
            {
                conn_obj  = nullptr;
                iserror   = true;
                error_msg = "conn_pool not found " + dbtag;
            }
        }
        M_MODEL &switchDB(const std::string &temptag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(temptag);
            if (iter != conn_pool_obj.end())
            {
                //必须是 DB_TYPE::MYSQL 类型链接
                if (iter->second->conf_data[0].db_type != DB_TYPE::MYSQL)
                {
                    conn_obj  = nullptr;
                    iserror   = true;
                    error_msg = "conn_pool db type error " + temptag;
                }
                else
                {
                    conn_obj = iter->second;
                    dbtag    = temptag;
                }
            }
            else
            {
                conn_obj  = nullptr;
                iserror   = true;
                error_msg = "conn_pool not found " + temptag;
            }
            return *mod;
        }

        M_MODEL &resetDB()
        {
            dbtag = B_BASE::_rmstag;
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                //必须是 DB_TYPE::MYSQL 类型链接
                if (iter->second->conf_data[0].db_type != DB_TYPE::MYSQL)
                {
                    conn_obj  = nullptr;
                    iserror   = true;
                    error_msg = "conn_pool db type error " + dbtag;
                }
                else
                {
                    conn_obj = iter->second;
                }
            }
            else
            {
                conn_obj  = nullptr;
                iserror   = true;
                error_msg = "conn_pool not found " + dbtag;
            }
            return *mod;
        }

        M_MODEL &set_table(const std::string &table_name)
        {
            if (table_name.size() > 0)
            {
                B_BASE::tablename = table_name;
            }
            return *mod;
        }
        M_MODEL &reset_table()
        {
            B_BASE::tablename = B_BASE::org_tablename;
            return *mod;
        }
        unsigned int count()
        {
            std::string where_clause;
            build_text_where(where_clause);

            sqlstring = "SELECT count(*) as total_countnum  FROM ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");
            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                unsigned int querysql_len = 0;
                unsigned int fetch_count  = select_conn->fetch_directly(sqlstring,
                                                                       [&querysql_len](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           (void)col_count;
                                                                           (void)col_names;
                                                                           auto [ptr, len] = get_data(0);
                                                                           if (ptr != nullptr && len > 0)
                                                                           {
                                                                               querysql_len = 0;
                                                                               for (unsigned int ik = 0; ik < len; ik++)
                                                                               {
                                                                                   if (ptr[ik] >= '0' && ptr[ik] <= '9')
                                                                                   {
                                                                                       querysql_len = querysql_len * 10 + (ptr[ik] - '0');
                                                                                   }
                                                                               }
                                                                           }
                                                                           return false;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                effect_num = querysql_len;

                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                return querysql_len;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        // ===== exec_count / exec_page：预编译 COUNT(*) + 分页 =====
        unsigned int exec_count()
        {
            effect_num = 0;
            std::vector<http::obj_val> params;
            std::string sql = "SELECT count(*) as total_countnum FROM ";
            sql.append(B_BASE::tablename);

            std::string where_clause;
            build_prepared_where(where_clause, params);
            sql.append(" WHERE ").append(where_clause);

            if (!groupsql.empty())
                sql.append(groupsql);
            if (!limitsql.empty())
                sql.append(limitsql);

            if (iserror)
                return 0;
            if (conn_empty())
                return 0;

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int count_val = 0;
            conn->fetch_prepared(sql, params, [&count_val]([[maybe_unused]] int col_count, [[maybe_unused]] char **col_names, auto get_data) -> bool
                                 {
                    auto [ptr, len] = get_data(0);
                    if (ptr) {
                        for (size_t i = 0; i < len; i++)
                            if (ptr[i] >= '0' && ptr[i] <= '9')
                                count_val = count_val * 10 + (ptr[i] - '0');
                    }
                    return false; });

            if (conn->isdebug)
                conn->finish_time();
            if (count_val == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
                if (!islock_conn)
                    conn_obj->back_mysql_select_conn(std::move(conn));
                return 0;
            }

            effect_num = count_val;
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(conn));
            return count_val;
        }

        std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>
        page(unsigned int page, unsigned int per_page = 10, unsigned int list_num = 5)
        {
            unsigned int total_page = count();
            if (per_page == 0)
            {
                per_page = 10;
            }
            if (list_num < 1)
            {
                list_num = 1;
            }
            total_page = std::ceil((float)total_page / per_page);

            if (total_page < 1)
            {
                total_page = 1;
            }
            if (page > total_page)
            {
                page = total_page;
            }
            if (page < 1)
            {
                page = 1;
            }
            unsigned int mid_num  = std::floor(list_num / 2);
            unsigned int last_num = list_num - 1;

            int temp_num = page - mid_num;

            unsigned int minpage = temp_num < 1 ? 1 : temp_num;
            unsigned int maxpage = minpage + last_num;

            if (maxpage > total_page)
            {
                maxpage  = total_page;
                temp_num = (maxpage - last_num);
                if (temp_num < 1)
                {
                    minpage = 1;
                }
                else
                {
                    minpage = temp_num;
                }
            }
            limit((page - 1) * per_page, per_page);
            return std::make_tuple(minpage, maxpage, page, total_page);
        }

        // --- exec_page：预编译分页（内部调 exec_count + limit）---
        auto exec_page(unsigned int page, unsigned int per_page = 10, unsigned int list_num = 5)
        {
            unsigned int total_page = exec_count();
            if (per_page == 0)
                per_page = 10;
            if (list_num < 1)
                list_num = 1;
            total_page = std::ceil((float)total_page / per_page);
            if (total_page < 1)
                total_page = 1;
            if (page > total_page)
                page = total_page;
            if (page < 1)
                page = 1;

            unsigned int mid_num  = std::floor(list_num / 2);
            unsigned int last_num = list_num - 1;
            int temp_num          = page - mid_num;
            unsigned int minpage  = temp_num < 1 ? 1 : temp_num;
            unsigned int maxpage  = minpage + last_num;

            if (maxpage > total_page)
            {
                maxpage  = total_page;
                temp_num = (int)(maxpage - last_num);
                if (temp_num < 1)
                    minpage = 1;
                else
                    minpage = temp_num;
            }
            limit((page - 1) * per_page, per_page);
            return std::make_tuple(minpage, maxpage, page, total_page);
        }

        asio::awaitable<unsigned int> async_count()
        {
            std::string where_clause;
            build_text_where(where_clause);
            sqlstring = "SELECT count(*) as total_countnum  FROM ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");
            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                unsigned int querysql_len = 0;
                unsigned int fetch_count  = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [&querysql_len](int col_count, char **col_names, auto get_data) -> bool
                                                                                      {
                                                                                          (void)col_count;
                                                                                          (void)col_names;
                                                                                          auto [ptr, len] = get_data(0);
                                                                                          if (ptr != nullptr && len > 0)
                                                                                          {
                                                                                              querysql_len = 0;
                                                                                              for (unsigned int ik = 0; ik < len; ik++)
                                                                                              {
                                                                                                  if (ptr[ik] >= '0' && ptr[ik] <= '9')
                                                                                                  {
                                                                                                      querysql_len = querysql_len * 10 + (ptr[ik] - '0');
                                                                                                  }
                                                                                              }
                                                                                          }
                                                                                          return false;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                effect_num = querysql_len;

                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
                co_return querysql_len;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        // --- async_exec_count：异步预编译 COUNT(*) ---
        asio::awaitable<unsigned int> async_exec_count()
        {
            effect_num = 0;
            std::vector<http::obj_val> params;
            std::string sql = "SELECT count(*) as total_countnum FROM ";
            sql.append(B_BASE::tablename);

            std::string where_clause;
            build_prepared_where(where_clause, params);
            sql.append(" WHERE ").append(where_clause);

            if (!groupsql.empty())
                sql.append(groupsql);
            if (!limitsql.empty())
                sql.append(limitsql);

            if (iserror)
                co_return 0;

            std::shared_ptr<mysql_conn_base> select_conn_l;
            if (islock_conn)
            {
                if (!this->select_conn || this->select_conn->isclose)
                    this->select_conn = co_await conn_obj->async_get_mysql_select_conn();
                select_conn_l = this->select_conn;
            }
            else
            {
                select_conn_l = co_await conn_obj->async_get_mysql_select_conn();
            }

            if (select_conn_l->isdebug)
                select_conn_l->begin_time();

            unsigned int count_val = 0;
            co_await select_conn_l->async_fetch_prepared(sql, params, [&count_val]([[maybe_unused]] int col_count, [[maybe_unused]] char **col_names, auto get_data) -> bool
                                                         {
                    auto [ptr, len] = get_data(0);
                    if (ptr) {
                        for (size_t i = 0; i < len; i++)
                            if (ptr[i] >= '0' && ptr[i] <= '9')
                                count_val = count_val * 10 + (ptr[i] - '0');
                    }
                    return false; });

            if (select_conn_l->isdebug)
                select_conn_l->finish_time();
            if (count_val == 0 && !select_conn_l->error_msg.empty())
            {
                iserror   = true;
                error_msg = select_conn_l->error_msg;
                if (!islock_conn)
                    conn_obj->back_mysql_select_conn(std::move(select_conn_l));
                co_return 0;
            }

            effect_num = count_val;
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn_l));
            co_return count_val;
        }

        asio::awaitable<std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>>
        async_page(unsigned int page, unsigned int per_page = 10, unsigned int list_num = 5)
        {
            unsigned int total_page = co_await async_count();
            if (per_page == 0)
            {
                per_page = 10;
            }
            if (list_num < 1)
            {
                list_num = 1;
            }
            total_page = std::ceil((float)total_page / per_page);

            if (total_page < 1)
            {
                total_page = 1;
            }
            if (page > total_page)
            {
                page = total_page;
            }
            if (page < 1)
            {
                page = 1;
            }
            unsigned int mid_num  = std::floor(list_num / 2);
            unsigned int last_num = list_num - 1;

            int temp_num = page - mid_num;

            unsigned int minpage = temp_num < 1 ? 1 : temp_num;
            unsigned int maxpage = minpage + last_num;

            if (maxpage > total_page)
            {
                maxpage  = total_page;
                temp_num = (maxpage - last_num);
                if (temp_num < 1)
                {
                    minpage = 1;
                }
                else
                {
                    minpage = temp_num;
                }
            }
            limit((page - 1) * per_page, per_page);
            co_return std::make_tuple(minpage, maxpage, page, total_page);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>>
        async_exec_page(unsigned int page, unsigned int per_page = 10, unsigned int list_num = 5)
        {
            unsigned int total_page = co_await async_exec_count();
            if (per_page == 0)
                per_page = 10;
            if (list_num < 1)
                list_num = 1;
            total_page = std::ceil((float)total_page / per_page);
            if (total_page < 1)
                total_page = 1;
            if (page > total_page)
                page = total_page;
            if (page < 1)
                page = 1;

            unsigned int mid_num  = std::floor(list_num / 2);
            unsigned int last_num = list_num - 1;
            int temp_num          = page - mid_num;
            unsigned int minpage  = temp_num < 1 ? 1 : temp_num;
            unsigned int maxpage  = minpage + last_num;

            if (maxpage > total_page)
            {
                maxpage  = total_page;
                temp_num = (int)(maxpage - last_num);
                if (temp_num < 1)
                    minpage = 1;
                else
                    minpage = temp_num;
            }
            limit((page - 1) * per_page, per_page);
            co_return std::make_tuple(minpage, maxpage, page, total_page);
        }

        unsigned int update_col(std::string colname, int num, char symbol = '+')
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            sqlstring  = "UPDATE ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" SET ");
            sqlstring.append(colname);
            if (num > 0)
            {
                sqlstring.append(" = ");
                sqlstring.append(colname);
                sqlstring.push_back(' ');
                sqlstring.push_back(symbol);
                sqlstring.append(std::to_string(num));
            }
            else
            {
                sqlstring.append(" = ");
                sqlstring.append(colname);
                sqlstring.push_back(' ');
                sqlstring.push_back(symbol);
                sqlstring.push_back('(');
                sqlstring.push_back('-');
                sqlstring.append(std::to_string(std::abs(num)));
                sqlstring.push_back(')');
            }
            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    sqlstring.append(tempsql.str());
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        // ===== exec_update_col / exec_replace_col：预编译 UPDATE =====
        unsigned int exec_update_col(std::string colname, int num, char symbol = '+')
        {
            effect_num = 0;
            std::vector<http::obj_val> params;
            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");
            sql.append(colname);
            sql.append(" = ");
            sql.append(colname);
            sql.push_back(' ');
            sql.push_back(symbol);
            sql.append(std::to_string(num > 0 ? num : -num));

            sql.append(" WHERE ");
            std::string where_clause;
            build_prepared_where(where_clause, params);
            sql.append(where_clause);

            // 缺 WHERE 守卫
            if (where_clause.empty())
            {
                sqlstring = "exec_update_col: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition";
                iserror   = true;
                return 0;
            }
            if (!groupsql.empty())
                sql.append(groupsql);
            if (!limitsql.empty())
                sql.append(limitsql);

            if (iserror)
                return 0;
            if (conn_empty())
                return 0;

            auto conn = _get_prepared_edit_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int affected = conn->exec_dml_prepared(sql, params);
            effect_num            = affected;

            if (conn->isdebug)
                conn->finish_time();
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = conn->error_msg;
                iserror   = true;
                if (!islock_conn)
                    conn_obj->back_mysql_edit_conn(std::move(conn));
                return affected;
            }
            if (!islock_conn)
                conn_obj->back_mysql_edit_conn(std::move(conn));
            return affected;
        }

        asio::awaitable<unsigned int> async_update_col(std::string colname, int num, char symbol = '+')
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            sqlstring  = "UPDATE ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" SET ");
            sqlstring.append(colname);
            if (num > 0)
            {
                sqlstring.append(" = ");
                sqlstring.append(colname);
                sqlstring.push_back(' ');
                sqlstring.push_back(symbol);
                sqlstring.append(std::to_string(num));
            }
            else
            {
                sqlstring.append(" = ");
                sqlstring.append(colname);
                sqlstring.push_back(' ');
                sqlstring.push_back(symbol);
                sqlstring.push_back('(');
                sqlstring.push_back('-');
                sqlstring.append(std::to_string(std::abs(num)));
                sqlstring.push_back(')');
            }
            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    sqlstring.append(tempsql.str());
                }
                else
                {
                    co_return 0;
                }
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {

                if (conn_empty())
                {
                    co_return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        // --- async_exec_update_col：异步预编译计数器 UPDATE ---
        asio::awaitable<unsigned int> async_exec_update_col(std::string colname, int num, char symbol = '+')
        {
            effect_num = 0;
            std::vector<http::obj_val> params;
            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");
            sql.append(colname);
            sql.append(" = ");
            sql.append(colname);
            sql.push_back(' ');
            sql.push_back(symbol);
            sql.append(std::to_string(num > 0 ? num : -num));

            sql.append(" WHERE ");
            std::string where_clause;
            build_prepared_where(where_clause, params);
            sql.append(where_clause);

            if (where_clause.empty())
            {
                sqlstring = "async_exec_update_col: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition";
                iserror   = true;
                co_return 0;
            }
            if (!groupsql.empty())
                sql.append(groupsql);
            if (!limitsql.empty())
                sql.append(limitsql);

            if (iserror)
                co_return 0;

            std::shared_ptr<mysql_conn_base> edit_conn_l;
            if (islock_conn)
            {
                if (!this->edit_conn || this->edit_conn->isclose)
                    this->edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                edit_conn_l = this->edit_conn;
            }
            else
            {
                edit_conn_l = co_await conn_obj->async_get_mysql_edit_conn();
            }

            if (edit_conn_l->isdebug)
                edit_conn_l->begin_time();

            unsigned int affected = co_await edit_conn_l->async_exec_dml_prepared(sql, params);
            effect_num            = affected;

            if (edit_conn_l->isdebug)
                edit_conn_l->finish_time();
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = edit_conn_l->error_msg;
                iserror   = true;
                if (!islock_conn)
                    conn_obj->back_mysql_edit_conn(std::move(edit_conn_l));
                co_return affected;
            }
            if (!islock_conn)
                conn_obj->back_mysql_edit_conn(std::move(edit_conn_l));
            co_return affected;
        }

        int replace_col(std::string colname, const std::string &old_string, const std::string &new_string)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            sqlstring  = "UPDATE ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" SET ");
            sqlstring.append(colname);

            sqlstring.append(" = REPLACE(");
            sqlstring.append(colname);
            sqlstring.append(",'");
            sqlstring.append(B_BASE::stringaddslash(old_string));
            sqlstring.append("','");
            sqlstring.append(B_BASE::stringaddslash(new_string));
            sqlstring.append("') ");

            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    sqlstring.append(tempsql.str());
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        // --- exec_replace_col：预编译 REPLACE(col, old, new) UPDATE ---
        unsigned int exec_replace_col(std::string colname, std::string_view old_str, std::string_view new_str)
        {
            effect_num = 0;
            std::vector<http::obj_val> params;
            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");
            sql.append(colname);
            sql.append(" = REPLACE(");
            sql.append(colname);
            sql.append(", ?, ?)");

            params.push_back(http::obj_val(std::string(old_str)));
            params.push_back(http::obj_val(std::string(new_str)));

            sql.append(" WHERE ");
            std::string where_clause;
            build_prepared_where(where_clause, params);
            sql.append(where_clause);

            if (where_clause.empty())
            {
                sqlstring = "exec_replace_col: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition";
                iserror   = true;
                return 0;
            }
            if (!groupsql.empty())
                sql.append(groupsql);
            if (!limitsql.empty())
                sql.append(limitsql);

            if (iserror)
                return 0;
            if (conn_empty())
                return 0;

            auto conn = _get_prepared_edit_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int affected = conn->exec_dml_prepared(sql, params);
            effect_num            = affected;

            if (conn->isdebug)
                conn->finish_time();
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = conn->error_msg;
                iserror   = true;
                if (!islock_conn)
                    conn_obj->back_mysql_edit_conn(std::move(conn));
                return affected;
            }
            if (!islock_conn)
                conn_obj->back_mysql_edit_conn(std::move(conn));
            return affected;
        }

        asio::awaitable<unsigned int> async_replace_col(std::string colname, std::string_view old_string, std::string_view new_string)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            sqlstring  = "UPDATE ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" SET ");
            sqlstring.append(colname);

            sqlstring.append(" = REPLACE(");
            sqlstring.append(colname);
            sqlstring.append(",'");
            sqlstring.append(B_BASE::stringaddslash(old_string));
            sqlstring.append("','");
            sqlstring.append(B_BASE::stringaddslash(new_string));
            sqlstring.append("') ");

            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    sqlstring.append(tempsql.str());
                }
                else
                {
                    co_return 0;
                }
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        // --- async_exec_replace_col：异步预编译 REPLACE(col, old, new) ---
        asio::awaitable<unsigned int> async_exec_replace_col(std::string colname, std::string_view old_str, std::string_view new_str)
        {
            effect_num = 0;
            std::vector<http::obj_val> params;
            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");
            sql.append(colname);
            sql.append(" = REPLACE(");
            sql.append(colname);
            sql.append(", ?, ?)");

            params.push_back(http::obj_val(std::string(old_str)));
            params.push_back(http::obj_val(std::string(new_str)));

            sql.append(" WHERE ");
            std::string where_clause;
            build_prepared_where(where_clause, params);
            sql.append(where_clause);

            if (where_clause.empty())
            {
                sqlstring = "async_exec_replace_col: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition";
                iserror   = true;
                co_return 0;
            }
            if (!groupsql.empty())
                sql.append(groupsql);
            if (!limitsql.empty())
                sql.append(limitsql);

            if (iserror)
                co_return 0;

            std::shared_ptr<mysql_conn_base> edit_conn_l;
            if (islock_conn)
            {
                if (!this->edit_conn || this->edit_conn->isclose)
                    this->edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                edit_conn_l = this->edit_conn;
            }
            else
            {
                edit_conn_l = co_await conn_obj->async_get_mysql_edit_conn();
            }

            if (edit_conn_l->isdebug)
                edit_conn_l->begin_time();

            unsigned int affected = co_await edit_conn_l->async_exec_dml_prepared(sql, params);
            effect_num            = affected;

            if (edit_conn_l->isdebug)
                edit_conn_l->finish_time();
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = edit_conn_l->error_msg;
                iserror   = true;
                if (!islock_conn)
                    conn_obj->back_mysql_edit_conn(std::move(edit_conn_l));
                co_return affected;
            }
            if (!islock_conn)
                conn_obj->back_mysql_edit_conn(std::move(edit_conn_l));
            co_return affected;
        }

        void assign_field_value(unsigned char index_pos, unsigned char *result_temp_data, unsigned long long value_size, xtask_info::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0:
             {
               data_temp.xtaskid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.xtaskid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.xtaskid = 0;
                        }
            }
            break;
                case 1:
             {
               data_temp.userid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.userid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.userid = 0;
                        }
            }
            break;
                case 2:
             {
               data_temp.xpjid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.xpjid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.xpjid = 0;
                        }
            }
            break;
                case 3:
             {
               data_temp.adminid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.adminid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.adminid = 0;
                        }
            }
            break;
                case 4:
             {
               data_temp.parentid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.parentid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.parentid = 0;
                        }
            }
            break;
                case 5:
             {
               data_temp.begindate = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.begindate);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.begindate = 0;
                        }
            }
            break;
                case 6:
             {
               data_temp.enddate = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.enddate);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.enddate = 0;
                        }
            }
            break;
                case 7:
             {
               data_temp.expectbegindate = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.expectbegindate);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.expectbegindate = 0;
                        }
            }
            break;
                case 8:
             {
               data_temp.expectenddate = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.expectenddate);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.expectenddate = 0;
                        }
            }
            break;
                case 9:
             {
               data_temp.milestone = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.milestone);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.milestone = 0;
                        }
            }
            break;
                case 10:
             {
               data_temp.subxpjid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.subxpjid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.subxpjid = 0;
                        }
            }
            break;
                case 11:
             {
               data_temp.depxtaskid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.depxtaskid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.depxtaskid = 0;
                        }
            }
            break;
                case 12:
             {
               data_temp.referdocid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.referdocid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.referdocid = 0;
                        }
            }
            break;
                case 13:
             {
               data_temp.isfinish = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.isfinish);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.isfinish = 0;
                        }
            }
            break;
                case 14:
             {
               data_temp.updatedate = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.updatedate);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.updatedate = 0;
                        }
            }
            break;
                case 15:
             {
               data_temp.finishdate = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.finishdate);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.finishdate = 0;
                        }
            }
            break;
                case 16:
             {
               data_temp.iscore = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.iscore);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.iscore = 0;
                        }
            }
            break;
                case 17:
                {

                #if defined(_LIBCPP_VERSION) && \
                    (!defined(__cpp_lib_to_chars) || __cpp_lib_to_chars < 201611L || \
                    (defined(__apple_build_version__) && __clang_major__ < 21))

                    data_temp.xvalue = 0.0;
                    try {
                        const char* p = reinterpret_cast<const char*>(result_temp_data);

                        if (value_size == 0 || *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
                            data_temp.xvalue = 0.0;
                        } else {
                            std::string tmp(p, value_size);
                            size_t idx = 0;
                            long double parsed = std::stold(tmp, &idx);
                            if (idx > 0 && idx <= value_size) {
                                data_temp.xvalue = static_cast<double>(parsed);
                            } else {
                                data_temp.xvalue = 0.0;
                            }
                        }
                    } catch (...) {
                        data_temp.xvalue = 0.0;
                    }

                #else

                data_temp.xvalue=0.0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.xvalue);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.xvalue = 0.0;
                        }
                #endif
            }  
            break;
                case 18:
                {

                #if defined(_LIBCPP_VERSION) && \
                    (!defined(__cpp_lib_to_chars) || __cpp_lib_to_chars < 201611L || \
                    (defined(__apple_build_version__) && __clang_major__ < 21))

                    data_temp.expectday = 0.0;
                    try {
                        const char* p = reinterpret_cast<const char*>(result_temp_data);

                        if (value_size == 0 || *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
                            data_temp.expectday = 0.0;
                        } else {
                            std::string tmp(p, value_size);
                            size_t idx = 0;
                            long double parsed = std::stold(tmp, &idx);
                            if (idx > 0 && idx <= value_size) {
                                data_temp.expectday = static_cast<double>(parsed);
                            } else {
                                data_temp.expectday = 0.0;
                            }
                        }
                    } catch (...) {
                        data_temp.expectday = 0.0;
                    }

                #else

                data_temp.expectday=0.0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.expectday);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.expectday = 0.0;
                        }
                #endif
            }  
            break;
                case 19:
                {

                #if defined(_LIBCPP_VERSION) && \
                    (!defined(__cpp_lib_to_chars) || __cpp_lib_to_chars < 201611L || \
                    (defined(__apple_build_version__) && __clang_major__ < 21))

                    data_temp.realday = 0.0;
                    try {
                        const char* p = reinterpret_cast<const char*>(result_temp_data);

                        if (value_size == 0 || *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
                            data_temp.realday = 0.0;
                        } else {
                            std::string tmp(p, value_size);
                            size_t idx = 0;
                            long double parsed = std::stold(tmp, &idx);
                            if (idx > 0 && idx <= value_size) {
                                data_temp.realday = static_cast<double>(parsed);
                            } else {
                                data_temp.realday = 0.0;
                            }
                        }
                    } catch (...) {
                        data_temp.realday = 0.0;
                    }

                #else

                data_temp.realday=0.0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.realday);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.realday = 0.0;
                        }
                #endif
            }  
            break;
                case 20:
                {

                #if defined(_LIBCPP_VERSION) && \
                    (!defined(__cpp_lib_to_chars) || __cpp_lib_to_chars < 201611L || \
                    (defined(__apple_build_version__) && __clang_major__ < 21))

                    data_temp.pricevalue = 0.0;
                    try {
                        const char* p = reinterpret_cast<const char*>(result_temp_data);

                        if (value_size == 0 || *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
                            data_temp.pricevalue = 0.0;
                        } else {
                            std::string tmp(p, value_size);
                            size_t idx = 0;
                            long double parsed = std::stold(tmp, &idx);
                            if (idx > 0 && idx <= value_size) {
                                data_temp.pricevalue = static_cast<double>(parsed);
                            } else {
                                data_temp.pricevalue = 0.0;
                            }
                        }
                    } catch (...) {
                        data_temp.pricevalue = 0.0;
                    }

                #else

                data_temp.pricevalue=0.0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.pricevalue);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.pricevalue = 0.0;
                        }
                #endif
            }  
            break;
                case 21:
            data_temp.title.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 22:
            data_temp.introduce.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 23:
            data_temp.xlogo.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 24:
            data_temp.xcolor.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 25:
            data_temp.pullurl.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 26:
            data_temp.pulltitle.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 27:
            data_temp.pullauthor.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 28:
            data_temp.note.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 29:
            data_temp.itemnote.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                
        }
    }
    
        void assign_field_value_binary(unsigned char index_pos, col_value_variant val, xtask_info::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.xtaskid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.xtaskid = static_cast<decltype(data_temp.xtaskid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.xtaskid = static_cast<decltype(data_temp.xtaskid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.xtaskid = static_cast<decltype(data_temp.xtaskid )>(v);
                    else
                        data_temp.xtaskid = 0;
                }, val);
            } break;
            case 1: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.userid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.userid = static_cast<decltype(data_temp.userid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.userid = static_cast<decltype(data_temp.userid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.userid = static_cast<decltype(data_temp.userid )>(v);
                    else
                        data_temp.userid = 0;
                }, val);
            } break;
            case 2: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.xpjid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.xpjid = static_cast<decltype(data_temp.xpjid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.xpjid = static_cast<decltype(data_temp.xpjid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.xpjid = static_cast<decltype(data_temp.xpjid )>(v);
                    else
                        data_temp.xpjid = 0;
                }, val);
            } break;
            case 3: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.adminid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.adminid = static_cast<decltype(data_temp.adminid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.adminid = static_cast<decltype(data_temp.adminid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.adminid = static_cast<decltype(data_temp.adminid )>(v);
                    else
                        data_temp.adminid = 0;
                }, val);
            } break;
            case 4: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.parentid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.parentid = static_cast<decltype(data_temp.parentid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.parentid = static_cast<decltype(data_temp.parentid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.parentid = static_cast<decltype(data_temp.parentid )>(v);
                    else
                        data_temp.parentid = 0;
                }, val);
            } break;
            case 5: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.begindate = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.begindate = static_cast<decltype(data_temp.begindate )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.begindate = static_cast<decltype(data_temp.begindate )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.begindate = static_cast<decltype(data_temp.begindate )>(v);
                    else
                        data_temp.begindate = 0;
                }, val);
            } break;
            case 6: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.enddate = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.enddate = static_cast<decltype(data_temp.enddate )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.enddate = static_cast<decltype(data_temp.enddate )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.enddate = static_cast<decltype(data_temp.enddate )>(v);
                    else
                        data_temp.enddate = 0;
                }, val);
            } break;
            case 7: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.expectbegindate = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.expectbegindate = static_cast<decltype(data_temp.expectbegindate )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.expectbegindate = static_cast<decltype(data_temp.expectbegindate )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.expectbegindate = static_cast<decltype(data_temp.expectbegindate )>(v);
                    else
                        data_temp.expectbegindate = 0;
                }, val);
            } break;
            case 8: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.expectenddate = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.expectenddate = static_cast<decltype(data_temp.expectenddate )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.expectenddate = static_cast<decltype(data_temp.expectenddate )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.expectenddate = static_cast<decltype(data_temp.expectenddate )>(v);
                    else
                        data_temp.expectenddate = 0;
                }, val);
            } break;
            case 9: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.milestone = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.milestone = static_cast<decltype(data_temp.milestone )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.milestone = static_cast<decltype(data_temp.milestone )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.milestone = static_cast<decltype(data_temp.milestone )>(v);
                    else
                        data_temp.milestone = 0;
                }, val);
            } break;
            case 10: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.subxpjid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.subxpjid = static_cast<decltype(data_temp.subxpjid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.subxpjid = static_cast<decltype(data_temp.subxpjid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.subxpjid = static_cast<decltype(data_temp.subxpjid )>(v);
                    else
                        data_temp.subxpjid = 0;
                }, val);
            } break;
            case 11: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.depxtaskid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.depxtaskid = static_cast<decltype(data_temp.depxtaskid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.depxtaskid = static_cast<decltype(data_temp.depxtaskid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.depxtaskid = static_cast<decltype(data_temp.depxtaskid )>(v);
                    else
                        data_temp.depxtaskid = 0;
                }, val);
            } break;
            case 12: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.referdocid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.referdocid = static_cast<decltype(data_temp.referdocid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.referdocid = static_cast<decltype(data_temp.referdocid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.referdocid = static_cast<decltype(data_temp.referdocid )>(v);
                    else
                        data_temp.referdocid = 0;
                }, val);
            } break;
            case 13: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.isfinish = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.isfinish = static_cast<decltype(data_temp.isfinish )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.isfinish = static_cast<decltype(data_temp.isfinish )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.isfinish = static_cast<decltype(data_temp.isfinish )>(v);
                    else
                        data_temp.isfinish = 0;
                }, val);
            } break;
            case 14: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.updatedate = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.updatedate = static_cast<decltype(data_temp.updatedate )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.updatedate = static_cast<decltype(data_temp.updatedate )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.updatedate = static_cast<decltype(data_temp.updatedate )>(v);
                    else
                        data_temp.updatedate = 0;
                }, val);
            } break;
            case 15: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.finishdate = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.finishdate = static_cast<decltype(data_temp.finishdate )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.finishdate = static_cast<decltype(data_temp.finishdate )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.finishdate = static_cast<decltype(data_temp.finishdate )>(v);
                    else
                        data_temp.finishdate = 0;
                }, val);
            } break;
            case 16: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.iscore = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.iscore = static_cast<decltype(data_temp.iscore )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.iscore = static_cast<decltype(data_temp.iscore )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.iscore = static_cast<decltype(data_temp.iscore )>(v);
                    else
                        data_temp.iscore = 0;
                }, val);
            } break;
            case 17: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.xvalue = 0.0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.xvalue = static_cast<double>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.xvalue = static_cast<double>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.xvalue = v;
                    else
                        data_temp.xvalue = 0.0;
                }, val);
            } break;
            case 18: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.expectday = 0.0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.expectday = static_cast<double>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.expectday = static_cast<double>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.expectday = v;
                    else
                        data_temp.expectday = 0.0;
                }, val);
            } break;
            case 19: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.realday = 0.0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.realday = static_cast<double>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.realday = static_cast<double>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.realday = v;
                    else
                        data_temp.realday = 0.0;
                }, val);
            } break;
            case 20: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.pricevalue = 0.0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.pricevalue = static_cast<double>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.pricevalue = static_cast<double>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.pricevalue = v;
                    else
                        data_temp.pricevalue = 0.0;
                }, val);
            } break;
            case 21: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.title.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.title.assign(v.data(), v.size());
                    else
                        data_temp.title.clear();
                }, val);
            } break;
            case 22: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.introduce.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.introduce.assign(v.data(), v.size());
                    else
                        data_temp.introduce.clear();
                }, val);
            } break;
            case 23: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.xlogo.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.xlogo.assign(v.data(), v.size());
                    else
                        data_temp.xlogo.clear();
                }, val);
            } break;
            case 24: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.xcolor.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.xcolor.assign(v.data(), v.size());
                    else
                        data_temp.xcolor.clear();
                }, val);
            } break;
            case 25: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.pullurl.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.pullurl.assign(v.data(), v.size());
                    else
                        data_temp.pullurl.clear();
                }, val);
            } break;
            case 26: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.pulltitle.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.pulltitle.assign(v.data(), v.size());
                    else
                        data_temp.pulltitle.clear();
                }, val);
            } break;
            case 27: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.pullauthor.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.pullauthor.assign(v.data(), v.size());
                    else
                        data_temp.pullauthor.clear();
                }, val);
            } break;
            case 28: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.note.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.note.assign(v.data(), v.size());
                    else
                        data_temp.note.clear();
                }, val);
            } break;
            case 29: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.itemnote.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.itemnote.assign(v.data(), v.size());
                    else
                        data_temp.itemnote.clear();
                }, val);
            } break;
        }
    }
    
        http::obj_val get_field_value(unsigned char index_pos, const xtask_info::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0:
                return http::obj_val(static_cast<long long>(data_temp.xtaskid));
                break;
            case 1:
                return http::obj_val(static_cast<long long>(data_temp.userid));
                break;
            case 2:
                return http::obj_val(static_cast<long long>(data_temp.xpjid));
                break;
            case 3:
                return http::obj_val(static_cast<long long>(data_temp.adminid));
                break;
            case 4:
                return http::obj_val(static_cast<long long>(data_temp.parentid));
                break;
            case 5:
                return http::obj_val(static_cast<long long>(data_temp.begindate));
                break;
            case 6:
                return http::obj_val(static_cast<long long>(data_temp.enddate));
                break;
            case 7:
                return http::obj_val(static_cast<long long>(data_temp.expectbegindate));
                break;
            case 8:
                return http::obj_val(static_cast<long long>(data_temp.expectenddate));
                break;
            case 9:
                return http::obj_val(static_cast<long long>(data_temp.milestone));
                break;
            case 10:
                return http::obj_val(static_cast<long long>(data_temp.subxpjid));
                break;
            case 11:
                return http::obj_val(static_cast<long long>(data_temp.depxtaskid));
                break;
            case 12:
                return http::obj_val(static_cast<long long>(data_temp.referdocid));
                break;
            case 13:
                return http::obj_val(static_cast<long long>(data_temp.isfinish));
                break;
            case 14:
                return http::obj_val(static_cast<long long>(data_temp.updatedate));
                break;
            case 15:
                return http::obj_val(static_cast<long long>(data_temp.finishdate));
                break;
            case 16:
                return http::obj_val(static_cast<long long>(data_temp.iscore));
                break;
            case 17:
                return http::obj_val(static_cast<double>(data_temp.xvalue));
                break;
            case 18:
                return http::obj_val(static_cast<double>(data_temp.expectday));
                break;
            case 19:
                return http::obj_val(static_cast<double>(data_temp.realday));
                break;
            case 20:
                return http::obj_val(static_cast<double>(data_temp.pricevalue));
                break;
            case 21:
                return http::obj_val(std::string(data_temp.title));
                break;
            case 22:
                return http::obj_val(std::string(data_temp.introduce));
                break;
            case 23:
                return http::obj_val(std::string(data_temp.xlogo));
                break;
            case 24:
                return http::obj_val(std::string(data_temp.xcolor));
                break;
            case 25:
                return http::obj_val(std::string(data_temp.pullurl));
                break;
            case 26:
                return http::obj_val(std::string(data_temp.pulltitle));
                break;
            case 27:
                return http::obj_val(std::string(data_temp.pullauthor));
                break;
            case 28:
                return http::obj_val(std::string(data_temp.note));
                break;
            case 29:
                return http::obj_val(std::string(data_temp.itemnote));
                break;
            default:
                return http::obj_val(nullptr);
        }
    }
    
        http::obj_val get_insert_field_value(unsigned char index_pos, const xtask_info::meta &data_temp)
    {
        // Auto-increment primary key xtaskid == 0 bind NULL at runtime, and let the database automatically generate the primary key
        if (index_pos == 0 && data_temp.xtaskid == 0)
            return http::obj_val(nullptr);
        return get_field_value(index_pos, data_temp);
    }
    

M_MODEL& eqXtaskid(const std::string &val)
	{return where(B_BASE::cols::xtaskid, orm::wq::eq, val);
	}

M_MODEL& nqXtaskid(const std::string &val)
	{return where(B_BASE::cols::xtaskid, orm::wq::nq, val);
	}

M_MODEL& btXtaskid(const std::string &val)
	{return where(B_BASE::cols::xtaskid, orm::wq::bt, val);
	}

M_MODEL& beXtaskid(const std::string &val)
	{return where(B_BASE::cols::xtaskid, orm::wq::be, val);
	}

M_MODEL& ltXtaskid(const std::string &val)
	{return where(B_BASE::cols::xtaskid, orm::wq::lt, val);
	}

M_MODEL& leXtaskid(const std::string &val)
	{return where(B_BASE::cols::xtaskid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqXtaskid(T val)
	{return where(B_BASE::cols::xtaskid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqXtaskid(T val)
	{return where(B_BASE::cols::xtaskid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btXtaskid(T val)
	{return where(B_BASE::cols::xtaskid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beXtaskid(T val)
	{return where(B_BASE::cols::xtaskid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltXtaskid(T val)
	{return where(B_BASE::cols::xtaskid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leXtaskid(T val)
	{return where(B_BASE::cols::xtaskid, orm::wq::le, val);
	}

M_MODEL& nullXtaskid()
	{return whereNull(B_BASE::cols::xtaskid);
	}

M_MODEL& notnullXtaskid()
	{return whereNotNull(B_BASE::cols::xtaskid);
	}

M_MODEL& oreqXtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::eq, val);
	}

M_MODEL& ornqXtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::nq, val);
	}

M_MODEL& orbtXtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::bt, val);
	}

M_MODEL& orbeXtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::be, val);
	}

M_MODEL& orltXtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::lt, val);
	}

M_MODEL& orleXtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqXtaskid(T val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqXtaskid(T val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtXtaskid(T val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeXtaskid(T val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltXtaskid(T val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleXtaskid(T val)
	{return whereOr(B_BASE::cols::xtaskid, orm::wq::le, val);
	}

M_MODEL& ornullXtaskid()
	{return whereOrNull(B_BASE::cols::xtaskid);
	}

M_MODEL& ornotnullXtaskid()
	{return whereOrNotNull(B_BASE::cols::xtaskid);
	}

M_MODEL& eqUserid(const std::string &val)
	{return where(B_BASE::cols::userid, orm::wq::eq, val);
	}

M_MODEL& nqUserid(const std::string &val)
	{return where(B_BASE::cols::userid, orm::wq::nq, val);
	}

M_MODEL& btUserid(const std::string &val)
	{return where(B_BASE::cols::userid, orm::wq::bt, val);
	}

M_MODEL& beUserid(const std::string &val)
	{return where(B_BASE::cols::userid, orm::wq::be, val);
	}

M_MODEL& ltUserid(const std::string &val)
	{return where(B_BASE::cols::userid, orm::wq::lt, val);
	}

M_MODEL& leUserid(const std::string &val)
	{return where(B_BASE::cols::userid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqUserid(T val)
	{return where(B_BASE::cols::userid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqUserid(T val)
	{return where(B_BASE::cols::userid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btUserid(T val)
	{return where(B_BASE::cols::userid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beUserid(T val)
	{return where(B_BASE::cols::userid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltUserid(T val)
	{return where(B_BASE::cols::userid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leUserid(T val)
	{return where(B_BASE::cols::userid, orm::wq::le, val);
	}

M_MODEL& nullUserid()
	{return whereNull(B_BASE::cols::userid);
	}

M_MODEL& notnullUserid()
	{return whereNotNull(B_BASE::cols::userid);
	}

M_MODEL& oreqUserid(const std::string &val)
	{return whereOr(B_BASE::cols::userid, orm::wq::eq, val);
	}

M_MODEL& ornqUserid(const std::string &val)
	{return whereOr(B_BASE::cols::userid, orm::wq::nq, val);
	}

M_MODEL& orbtUserid(const std::string &val)
	{return whereOr(B_BASE::cols::userid, orm::wq::bt, val);
	}

M_MODEL& orbeUserid(const std::string &val)
	{return whereOr(B_BASE::cols::userid, orm::wq::be, val);
	}

M_MODEL& orltUserid(const std::string &val)
	{return whereOr(B_BASE::cols::userid, orm::wq::lt, val);
	}

M_MODEL& orleUserid(const std::string &val)
	{return whereOr(B_BASE::cols::userid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqUserid(T val)
	{return whereOr(B_BASE::cols::userid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqUserid(T val)
	{return whereOr(B_BASE::cols::userid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtUserid(T val)
	{return whereOr(B_BASE::cols::userid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeUserid(T val)
	{return whereOr(B_BASE::cols::userid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltUserid(T val)
	{return whereOr(B_BASE::cols::userid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleUserid(T val)
	{return whereOr(B_BASE::cols::userid, orm::wq::le, val);
	}

M_MODEL& ornullUserid()
	{return whereOrNull(B_BASE::cols::userid);
	}

M_MODEL& ornotnullUserid()
	{return whereOrNotNull(B_BASE::cols::userid);
	}

M_MODEL& eqXpjid(const std::string &val)
	{return where(B_BASE::cols::xpjid, orm::wq::eq, val);
	}

M_MODEL& nqXpjid(const std::string &val)
	{return where(B_BASE::cols::xpjid, orm::wq::nq, val);
	}

M_MODEL& btXpjid(const std::string &val)
	{return where(B_BASE::cols::xpjid, orm::wq::bt, val);
	}

M_MODEL& beXpjid(const std::string &val)
	{return where(B_BASE::cols::xpjid, orm::wq::be, val);
	}

M_MODEL& ltXpjid(const std::string &val)
	{return where(B_BASE::cols::xpjid, orm::wq::lt, val);
	}

M_MODEL& leXpjid(const std::string &val)
	{return where(B_BASE::cols::xpjid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqXpjid(T val)
	{return where(B_BASE::cols::xpjid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqXpjid(T val)
	{return where(B_BASE::cols::xpjid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btXpjid(T val)
	{return where(B_BASE::cols::xpjid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beXpjid(T val)
	{return where(B_BASE::cols::xpjid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltXpjid(T val)
	{return where(B_BASE::cols::xpjid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leXpjid(T val)
	{return where(B_BASE::cols::xpjid, orm::wq::le, val);
	}

M_MODEL& nullXpjid()
	{return whereNull(B_BASE::cols::xpjid);
	}

M_MODEL& notnullXpjid()
	{return whereNotNull(B_BASE::cols::xpjid);
	}

M_MODEL& oreqXpjid(const std::string &val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::eq, val);
	}

M_MODEL& ornqXpjid(const std::string &val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::nq, val);
	}

M_MODEL& orbtXpjid(const std::string &val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::bt, val);
	}

M_MODEL& orbeXpjid(const std::string &val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::be, val);
	}

M_MODEL& orltXpjid(const std::string &val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::lt, val);
	}

M_MODEL& orleXpjid(const std::string &val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqXpjid(T val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqXpjid(T val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtXpjid(T val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeXpjid(T val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltXpjid(T val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleXpjid(T val)
	{return whereOr(B_BASE::cols::xpjid, orm::wq::le, val);
	}

M_MODEL& ornullXpjid()
	{return whereOrNull(B_BASE::cols::xpjid);
	}

M_MODEL& ornotnullXpjid()
	{return whereOrNotNull(B_BASE::cols::xpjid);
	}

M_MODEL& eqAdminid(const std::string &val)
	{return where(B_BASE::cols::adminid, orm::wq::eq, val);
	}

M_MODEL& nqAdminid(const std::string &val)
	{return where(B_BASE::cols::adminid, orm::wq::nq, val);
	}

M_MODEL& btAdminid(const std::string &val)
	{return where(B_BASE::cols::adminid, orm::wq::bt, val);
	}

M_MODEL& beAdminid(const std::string &val)
	{return where(B_BASE::cols::adminid, orm::wq::be, val);
	}

M_MODEL& ltAdminid(const std::string &val)
	{return where(B_BASE::cols::adminid, orm::wq::lt, val);
	}

M_MODEL& leAdminid(const std::string &val)
	{return where(B_BASE::cols::adminid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqAdminid(T val)
	{return where(B_BASE::cols::adminid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqAdminid(T val)
	{return where(B_BASE::cols::adminid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btAdminid(T val)
	{return where(B_BASE::cols::adminid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beAdminid(T val)
	{return where(B_BASE::cols::adminid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltAdminid(T val)
	{return where(B_BASE::cols::adminid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leAdminid(T val)
	{return where(B_BASE::cols::adminid, orm::wq::le, val);
	}

M_MODEL& nullAdminid()
	{return whereNull(B_BASE::cols::adminid);
	}

M_MODEL& notnullAdminid()
	{return whereNotNull(B_BASE::cols::adminid);
	}

M_MODEL& oreqAdminid(const std::string &val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::eq, val);
	}

M_MODEL& ornqAdminid(const std::string &val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::nq, val);
	}

M_MODEL& orbtAdminid(const std::string &val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::bt, val);
	}

M_MODEL& orbeAdminid(const std::string &val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::be, val);
	}

M_MODEL& orltAdminid(const std::string &val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::lt, val);
	}

M_MODEL& orleAdminid(const std::string &val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqAdminid(T val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqAdminid(T val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtAdminid(T val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeAdminid(T val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltAdminid(T val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleAdminid(T val)
	{return whereOr(B_BASE::cols::adminid, orm::wq::le, val);
	}

M_MODEL& ornullAdminid()
	{return whereOrNull(B_BASE::cols::adminid);
	}

M_MODEL& ornotnullAdminid()
	{return whereOrNotNull(B_BASE::cols::adminid);
	}

M_MODEL& eqParentid(const std::string &val)
	{return where(B_BASE::cols::parentid, orm::wq::eq, val);
	}

M_MODEL& nqParentid(const std::string &val)
	{return where(B_BASE::cols::parentid, orm::wq::nq, val);
	}

M_MODEL& btParentid(const std::string &val)
	{return where(B_BASE::cols::parentid, orm::wq::bt, val);
	}

M_MODEL& beParentid(const std::string &val)
	{return where(B_BASE::cols::parentid, orm::wq::be, val);
	}

M_MODEL& ltParentid(const std::string &val)
	{return where(B_BASE::cols::parentid, orm::wq::lt, val);
	}

M_MODEL& leParentid(const std::string &val)
	{return where(B_BASE::cols::parentid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqParentid(T val)
	{return where(B_BASE::cols::parentid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqParentid(T val)
	{return where(B_BASE::cols::parentid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btParentid(T val)
	{return where(B_BASE::cols::parentid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beParentid(T val)
	{return where(B_BASE::cols::parentid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltParentid(T val)
	{return where(B_BASE::cols::parentid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leParentid(T val)
	{return where(B_BASE::cols::parentid, orm::wq::le, val);
	}

M_MODEL& nullParentid()
	{return whereNull(B_BASE::cols::parentid);
	}

M_MODEL& notnullParentid()
	{return whereNotNull(B_BASE::cols::parentid);
	}

M_MODEL& oreqParentid(const std::string &val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::eq, val);
	}

M_MODEL& ornqParentid(const std::string &val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::nq, val);
	}

M_MODEL& orbtParentid(const std::string &val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::bt, val);
	}

M_MODEL& orbeParentid(const std::string &val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::be, val);
	}

M_MODEL& orltParentid(const std::string &val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::lt, val);
	}

M_MODEL& orleParentid(const std::string &val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqParentid(T val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqParentid(T val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtParentid(T val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeParentid(T val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltParentid(T val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleParentid(T val)
	{return whereOr(B_BASE::cols::parentid, orm::wq::le, val);
	}

M_MODEL& ornullParentid()
	{return whereOrNull(B_BASE::cols::parentid);
	}

M_MODEL& ornotnullParentid()
	{return whereOrNotNull(B_BASE::cols::parentid);
	}

M_MODEL& eqBegindate(const std::string &val)
	{return where(B_BASE::cols::begindate, orm::wq::eq, val);
	}

M_MODEL& nqBegindate(const std::string &val)
	{return where(B_BASE::cols::begindate, orm::wq::nq, val);
	}

M_MODEL& btBegindate(const std::string &val)
	{return where(B_BASE::cols::begindate, orm::wq::bt, val);
	}

M_MODEL& beBegindate(const std::string &val)
	{return where(B_BASE::cols::begindate, orm::wq::be, val);
	}

M_MODEL& ltBegindate(const std::string &val)
	{return where(B_BASE::cols::begindate, orm::wq::lt, val);
	}

M_MODEL& leBegindate(const std::string &val)
	{return where(B_BASE::cols::begindate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqBegindate(T val)
	{return where(B_BASE::cols::begindate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqBegindate(T val)
	{return where(B_BASE::cols::begindate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btBegindate(T val)
	{return where(B_BASE::cols::begindate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beBegindate(T val)
	{return where(B_BASE::cols::begindate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltBegindate(T val)
	{return where(B_BASE::cols::begindate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leBegindate(T val)
	{return where(B_BASE::cols::begindate, orm::wq::le, val);
	}

M_MODEL& nullBegindate()
	{return whereNull(B_BASE::cols::begindate);
	}

M_MODEL& notnullBegindate()
	{return whereNotNull(B_BASE::cols::begindate);
	}

M_MODEL& oreqBegindate(const std::string &val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::eq, val);
	}

M_MODEL& ornqBegindate(const std::string &val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::nq, val);
	}

M_MODEL& orbtBegindate(const std::string &val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::bt, val);
	}

M_MODEL& orbeBegindate(const std::string &val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::be, val);
	}

M_MODEL& orltBegindate(const std::string &val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::lt, val);
	}

M_MODEL& orleBegindate(const std::string &val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqBegindate(T val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqBegindate(T val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtBegindate(T val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeBegindate(T val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltBegindate(T val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleBegindate(T val)
	{return whereOr(B_BASE::cols::begindate, orm::wq::le, val);
	}

M_MODEL& ornullBegindate()
	{return whereOrNull(B_BASE::cols::begindate);
	}

M_MODEL& ornotnullBegindate()
	{return whereOrNotNull(B_BASE::cols::begindate);
	}

M_MODEL& eqEnddate(const std::string &val)
	{return where(B_BASE::cols::enddate, orm::wq::eq, val);
	}

M_MODEL& nqEnddate(const std::string &val)
	{return where(B_BASE::cols::enddate, orm::wq::nq, val);
	}

M_MODEL& btEnddate(const std::string &val)
	{return where(B_BASE::cols::enddate, orm::wq::bt, val);
	}

M_MODEL& beEnddate(const std::string &val)
	{return where(B_BASE::cols::enddate, orm::wq::be, val);
	}

M_MODEL& ltEnddate(const std::string &val)
	{return where(B_BASE::cols::enddate, orm::wq::lt, val);
	}

M_MODEL& leEnddate(const std::string &val)
	{return where(B_BASE::cols::enddate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqEnddate(T val)
	{return where(B_BASE::cols::enddate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqEnddate(T val)
	{return where(B_BASE::cols::enddate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btEnddate(T val)
	{return where(B_BASE::cols::enddate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beEnddate(T val)
	{return where(B_BASE::cols::enddate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltEnddate(T val)
	{return where(B_BASE::cols::enddate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leEnddate(T val)
	{return where(B_BASE::cols::enddate, orm::wq::le, val);
	}

M_MODEL& nullEnddate()
	{return whereNull(B_BASE::cols::enddate);
	}

M_MODEL& notnullEnddate()
	{return whereNotNull(B_BASE::cols::enddate);
	}

M_MODEL& oreqEnddate(const std::string &val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::eq, val);
	}

M_MODEL& ornqEnddate(const std::string &val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::nq, val);
	}

M_MODEL& orbtEnddate(const std::string &val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::bt, val);
	}

M_MODEL& orbeEnddate(const std::string &val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::be, val);
	}

M_MODEL& orltEnddate(const std::string &val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::lt, val);
	}

M_MODEL& orleEnddate(const std::string &val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqEnddate(T val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqEnddate(T val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtEnddate(T val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeEnddate(T val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltEnddate(T val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleEnddate(T val)
	{return whereOr(B_BASE::cols::enddate, orm::wq::le, val);
	}

M_MODEL& ornullEnddate()
	{return whereOrNull(B_BASE::cols::enddate);
	}

M_MODEL& ornotnullEnddate()
	{return whereOrNotNull(B_BASE::cols::enddate);
	}

M_MODEL& eqExpectbegindate(const std::string &val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::eq, val);
	}

M_MODEL& nqExpectbegindate(const std::string &val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::nq, val);
	}

M_MODEL& btExpectbegindate(const std::string &val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::bt, val);
	}

M_MODEL& beExpectbegindate(const std::string &val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::be, val);
	}

M_MODEL& ltExpectbegindate(const std::string &val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::lt, val);
	}

M_MODEL& leExpectbegindate(const std::string &val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqExpectbegindate(T val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqExpectbegindate(T val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btExpectbegindate(T val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beExpectbegindate(T val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltExpectbegindate(T val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leExpectbegindate(T val)
	{return where(B_BASE::cols::expectbegindate, orm::wq::le, val);
	}

M_MODEL& nullExpectbegindate()
	{return whereNull(B_BASE::cols::expectbegindate);
	}

M_MODEL& notnullExpectbegindate()
	{return whereNotNull(B_BASE::cols::expectbegindate);
	}

M_MODEL& oreqExpectbegindate(const std::string &val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::eq, val);
	}

M_MODEL& ornqExpectbegindate(const std::string &val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::nq, val);
	}

M_MODEL& orbtExpectbegindate(const std::string &val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::bt, val);
	}

M_MODEL& orbeExpectbegindate(const std::string &val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::be, val);
	}

M_MODEL& orltExpectbegindate(const std::string &val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::lt, val);
	}

M_MODEL& orleExpectbegindate(const std::string &val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqExpectbegindate(T val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqExpectbegindate(T val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtExpectbegindate(T val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeExpectbegindate(T val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltExpectbegindate(T val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleExpectbegindate(T val)
	{return whereOr(B_BASE::cols::expectbegindate, orm::wq::le, val);
	}

M_MODEL& ornullExpectbegindate()
	{return whereOrNull(B_BASE::cols::expectbegindate);
	}

M_MODEL& ornotnullExpectbegindate()
	{return whereOrNotNull(B_BASE::cols::expectbegindate);
	}

M_MODEL& eqExpectenddate(const std::string &val)
	{return where(B_BASE::cols::expectenddate, orm::wq::eq, val);
	}

M_MODEL& nqExpectenddate(const std::string &val)
	{return where(B_BASE::cols::expectenddate, orm::wq::nq, val);
	}

M_MODEL& btExpectenddate(const std::string &val)
	{return where(B_BASE::cols::expectenddate, orm::wq::bt, val);
	}

M_MODEL& beExpectenddate(const std::string &val)
	{return where(B_BASE::cols::expectenddate, orm::wq::be, val);
	}

M_MODEL& ltExpectenddate(const std::string &val)
	{return where(B_BASE::cols::expectenddate, orm::wq::lt, val);
	}

M_MODEL& leExpectenddate(const std::string &val)
	{return where(B_BASE::cols::expectenddate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqExpectenddate(T val)
	{return where(B_BASE::cols::expectenddate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqExpectenddate(T val)
	{return where(B_BASE::cols::expectenddate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btExpectenddate(T val)
	{return where(B_BASE::cols::expectenddate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beExpectenddate(T val)
	{return where(B_BASE::cols::expectenddate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltExpectenddate(T val)
	{return where(B_BASE::cols::expectenddate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leExpectenddate(T val)
	{return where(B_BASE::cols::expectenddate, orm::wq::le, val);
	}

M_MODEL& nullExpectenddate()
	{return whereNull(B_BASE::cols::expectenddate);
	}

M_MODEL& notnullExpectenddate()
	{return whereNotNull(B_BASE::cols::expectenddate);
	}

M_MODEL& oreqExpectenddate(const std::string &val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::eq, val);
	}

M_MODEL& ornqExpectenddate(const std::string &val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::nq, val);
	}

M_MODEL& orbtExpectenddate(const std::string &val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::bt, val);
	}

M_MODEL& orbeExpectenddate(const std::string &val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::be, val);
	}

M_MODEL& orltExpectenddate(const std::string &val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::lt, val);
	}

M_MODEL& orleExpectenddate(const std::string &val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqExpectenddate(T val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqExpectenddate(T val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtExpectenddate(T val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeExpectenddate(T val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltExpectenddate(T val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleExpectenddate(T val)
	{return whereOr(B_BASE::cols::expectenddate, orm::wq::le, val);
	}

M_MODEL& ornullExpectenddate()
	{return whereOrNull(B_BASE::cols::expectenddate);
	}

M_MODEL& ornotnullExpectenddate()
	{return whereOrNotNull(B_BASE::cols::expectenddate);
	}

M_MODEL& eqMilestone(const std::string &val)
	{return where(B_BASE::cols::milestone, orm::wq::eq, val);
	}

M_MODEL& nqMilestone(const std::string &val)
	{return where(B_BASE::cols::milestone, orm::wq::nq, val);
	}

M_MODEL& btMilestone(const std::string &val)
	{return where(B_BASE::cols::milestone, orm::wq::bt, val);
	}

M_MODEL& beMilestone(const std::string &val)
	{return where(B_BASE::cols::milestone, orm::wq::be, val);
	}

M_MODEL& ltMilestone(const std::string &val)
	{return where(B_BASE::cols::milestone, orm::wq::lt, val);
	}

M_MODEL& leMilestone(const std::string &val)
	{return where(B_BASE::cols::milestone, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqMilestone(T val)
	{return where(B_BASE::cols::milestone, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqMilestone(T val)
	{return where(B_BASE::cols::milestone, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btMilestone(T val)
	{return where(B_BASE::cols::milestone, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beMilestone(T val)
	{return where(B_BASE::cols::milestone, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltMilestone(T val)
	{return where(B_BASE::cols::milestone, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leMilestone(T val)
	{return where(B_BASE::cols::milestone, orm::wq::le, val);
	}

M_MODEL& nullMilestone()
	{return whereNull(B_BASE::cols::milestone);
	}

M_MODEL& notnullMilestone()
	{return whereNotNull(B_BASE::cols::milestone);
	}

M_MODEL& oreqMilestone(const std::string &val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::eq, val);
	}

M_MODEL& ornqMilestone(const std::string &val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::nq, val);
	}

M_MODEL& orbtMilestone(const std::string &val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::bt, val);
	}

M_MODEL& orbeMilestone(const std::string &val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::be, val);
	}

M_MODEL& orltMilestone(const std::string &val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::lt, val);
	}

M_MODEL& orleMilestone(const std::string &val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqMilestone(T val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqMilestone(T val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtMilestone(T val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeMilestone(T val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltMilestone(T val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleMilestone(T val)
	{return whereOr(B_BASE::cols::milestone, orm::wq::le, val);
	}

M_MODEL& ornullMilestone()
	{return whereOrNull(B_BASE::cols::milestone);
	}

M_MODEL& ornotnullMilestone()
	{return whereOrNotNull(B_BASE::cols::milestone);
	}

M_MODEL& eqSubxpjid(const std::string &val)
	{return where(B_BASE::cols::subxpjid, orm::wq::eq, val);
	}

M_MODEL& nqSubxpjid(const std::string &val)
	{return where(B_BASE::cols::subxpjid, orm::wq::nq, val);
	}

M_MODEL& btSubxpjid(const std::string &val)
	{return where(B_BASE::cols::subxpjid, orm::wq::bt, val);
	}

M_MODEL& beSubxpjid(const std::string &val)
	{return where(B_BASE::cols::subxpjid, orm::wq::be, val);
	}

M_MODEL& ltSubxpjid(const std::string &val)
	{return where(B_BASE::cols::subxpjid, orm::wq::lt, val);
	}

M_MODEL& leSubxpjid(const std::string &val)
	{return where(B_BASE::cols::subxpjid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqSubxpjid(T val)
	{return where(B_BASE::cols::subxpjid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqSubxpjid(T val)
	{return where(B_BASE::cols::subxpjid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btSubxpjid(T val)
	{return where(B_BASE::cols::subxpjid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beSubxpjid(T val)
	{return where(B_BASE::cols::subxpjid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltSubxpjid(T val)
	{return where(B_BASE::cols::subxpjid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leSubxpjid(T val)
	{return where(B_BASE::cols::subxpjid, orm::wq::le, val);
	}

M_MODEL& nullSubxpjid()
	{return whereNull(B_BASE::cols::subxpjid);
	}

M_MODEL& notnullSubxpjid()
	{return whereNotNull(B_BASE::cols::subxpjid);
	}

M_MODEL& oreqSubxpjid(const std::string &val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::eq, val);
	}

M_MODEL& ornqSubxpjid(const std::string &val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::nq, val);
	}

M_MODEL& orbtSubxpjid(const std::string &val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::bt, val);
	}

M_MODEL& orbeSubxpjid(const std::string &val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::be, val);
	}

M_MODEL& orltSubxpjid(const std::string &val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::lt, val);
	}

M_MODEL& orleSubxpjid(const std::string &val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqSubxpjid(T val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqSubxpjid(T val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtSubxpjid(T val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeSubxpjid(T val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltSubxpjid(T val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleSubxpjid(T val)
	{return whereOr(B_BASE::cols::subxpjid, orm::wq::le, val);
	}

M_MODEL& ornullSubxpjid()
	{return whereOrNull(B_BASE::cols::subxpjid);
	}

M_MODEL& ornotnullSubxpjid()
	{return whereOrNotNull(B_BASE::cols::subxpjid);
	}

M_MODEL& eqDepxtaskid(const std::string &val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::eq, val);
	}

M_MODEL& nqDepxtaskid(const std::string &val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::nq, val);
	}

M_MODEL& btDepxtaskid(const std::string &val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::bt, val);
	}

M_MODEL& beDepxtaskid(const std::string &val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::be, val);
	}

M_MODEL& ltDepxtaskid(const std::string &val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::lt, val);
	}

M_MODEL& leDepxtaskid(const std::string &val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqDepxtaskid(T val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqDepxtaskid(T val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btDepxtaskid(T val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beDepxtaskid(T val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltDepxtaskid(T val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leDepxtaskid(T val)
	{return where(B_BASE::cols::depxtaskid, orm::wq::le, val);
	}

M_MODEL& nullDepxtaskid()
	{return whereNull(B_BASE::cols::depxtaskid);
	}

M_MODEL& notnullDepxtaskid()
	{return whereNotNull(B_BASE::cols::depxtaskid);
	}

M_MODEL& oreqDepxtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::eq, val);
	}

M_MODEL& ornqDepxtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::nq, val);
	}

M_MODEL& orbtDepxtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::bt, val);
	}

M_MODEL& orbeDepxtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::be, val);
	}

M_MODEL& orltDepxtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::lt, val);
	}

M_MODEL& orleDepxtaskid(const std::string &val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqDepxtaskid(T val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqDepxtaskid(T val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtDepxtaskid(T val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeDepxtaskid(T val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltDepxtaskid(T val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleDepxtaskid(T val)
	{return whereOr(B_BASE::cols::depxtaskid, orm::wq::le, val);
	}

M_MODEL& ornullDepxtaskid()
	{return whereOrNull(B_BASE::cols::depxtaskid);
	}

M_MODEL& ornotnullDepxtaskid()
	{return whereOrNotNull(B_BASE::cols::depxtaskid);
	}

M_MODEL& eqReferdocid(const std::string &val)
	{return where(B_BASE::cols::referdocid, orm::wq::eq, val);
	}

M_MODEL& nqReferdocid(const std::string &val)
	{return where(B_BASE::cols::referdocid, orm::wq::nq, val);
	}

M_MODEL& btReferdocid(const std::string &val)
	{return where(B_BASE::cols::referdocid, orm::wq::bt, val);
	}

M_MODEL& beReferdocid(const std::string &val)
	{return where(B_BASE::cols::referdocid, orm::wq::be, val);
	}

M_MODEL& ltReferdocid(const std::string &val)
	{return where(B_BASE::cols::referdocid, orm::wq::lt, val);
	}

M_MODEL& leReferdocid(const std::string &val)
	{return where(B_BASE::cols::referdocid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqReferdocid(T val)
	{return where(B_BASE::cols::referdocid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqReferdocid(T val)
	{return where(B_BASE::cols::referdocid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btReferdocid(T val)
	{return where(B_BASE::cols::referdocid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beReferdocid(T val)
	{return where(B_BASE::cols::referdocid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltReferdocid(T val)
	{return where(B_BASE::cols::referdocid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leReferdocid(T val)
	{return where(B_BASE::cols::referdocid, orm::wq::le, val);
	}

M_MODEL& nullReferdocid()
	{return whereNull(B_BASE::cols::referdocid);
	}

M_MODEL& notnullReferdocid()
	{return whereNotNull(B_BASE::cols::referdocid);
	}

M_MODEL& oreqReferdocid(const std::string &val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::eq, val);
	}

M_MODEL& ornqReferdocid(const std::string &val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::nq, val);
	}

M_MODEL& orbtReferdocid(const std::string &val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::bt, val);
	}

M_MODEL& orbeReferdocid(const std::string &val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::be, val);
	}

M_MODEL& orltReferdocid(const std::string &val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::lt, val);
	}

M_MODEL& orleReferdocid(const std::string &val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqReferdocid(T val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqReferdocid(T val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtReferdocid(T val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeReferdocid(T val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltReferdocid(T val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleReferdocid(T val)
	{return whereOr(B_BASE::cols::referdocid, orm::wq::le, val);
	}

M_MODEL& ornullReferdocid()
	{return whereOrNull(B_BASE::cols::referdocid);
	}

M_MODEL& ornotnullReferdocid()
	{return whereOrNotNull(B_BASE::cols::referdocid);
	}

M_MODEL& eqIsfinish(const std::string &val)
	{return where(B_BASE::cols::isfinish, orm::wq::eq, val);
	}

M_MODEL& nqIsfinish(const std::string &val)
	{return where(B_BASE::cols::isfinish, orm::wq::nq, val);
	}

M_MODEL& btIsfinish(const std::string &val)
	{return where(B_BASE::cols::isfinish, orm::wq::bt, val);
	}

M_MODEL& beIsfinish(const std::string &val)
	{return where(B_BASE::cols::isfinish, orm::wq::be, val);
	}

M_MODEL& ltIsfinish(const std::string &val)
	{return where(B_BASE::cols::isfinish, orm::wq::lt, val);
	}

M_MODEL& leIsfinish(const std::string &val)
	{return where(B_BASE::cols::isfinish, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqIsfinish(T val)
	{return where(B_BASE::cols::isfinish, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqIsfinish(T val)
	{return where(B_BASE::cols::isfinish, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btIsfinish(T val)
	{return where(B_BASE::cols::isfinish, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beIsfinish(T val)
	{return where(B_BASE::cols::isfinish, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltIsfinish(T val)
	{return where(B_BASE::cols::isfinish, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leIsfinish(T val)
	{return where(B_BASE::cols::isfinish, orm::wq::le, val);
	}

M_MODEL& nullIsfinish()
	{return whereNull(B_BASE::cols::isfinish);
	}

M_MODEL& notnullIsfinish()
	{return whereNotNull(B_BASE::cols::isfinish);
	}

M_MODEL& oreqIsfinish(const std::string &val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::eq, val);
	}

M_MODEL& ornqIsfinish(const std::string &val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::nq, val);
	}

M_MODEL& orbtIsfinish(const std::string &val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::bt, val);
	}

M_MODEL& orbeIsfinish(const std::string &val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::be, val);
	}

M_MODEL& orltIsfinish(const std::string &val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::lt, val);
	}

M_MODEL& orleIsfinish(const std::string &val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqIsfinish(T val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqIsfinish(T val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtIsfinish(T val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeIsfinish(T val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltIsfinish(T val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleIsfinish(T val)
	{return whereOr(B_BASE::cols::isfinish, orm::wq::le, val);
	}

M_MODEL& ornullIsfinish()
	{return whereOrNull(B_BASE::cols::isfinish);
	}

M_MODEL& ornotnullIsfinish()
	{return whereOrNotNull(B_BASE::cols::isfinish);
	}

M_MODEL& eqUpdatedate(const std::string &val)
	{return where(B_BASE::cols::updatedate, orm::wq::eq, val);
	}

M_MODEL& nqUpdatedate(const std::string &val)
	{return where(B_BASE::cols::updatedate, orm::wq::nq, val);
	}

M_MODEL& btUpdatedate(const std::string &val)
	{return where(B_BASE::cols::updatedate, orm::wq::bt, val);
	}

M_MODEL& beUpdatedate(const std::string &val)
	{return where(B_BASE::cols::updatedate, orm::wq::be, val);
	}

M_MODEL& ltUpdatedate(const std::string &val)
	{return where(B_BASE::cols::updatedate, orm::wq::lt, val);
	}

M_MODEL& leUpdatedate(const std::string &val)
	{return where(B_BASE::cols::updatedate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqUpdatedate(T val)
	{return where(B_BASE::cols::updatedate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqUpdatedate(T val)
	{return where(B_BASE::cols::updatedate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btUpdatedate(T val)
	{return where(B_BASE::cols::updatedate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beUpdatedate(T val)
	{return where(B_BASE::cols::updatedate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltUpdatedate(T val)
	{return where(B_BASE::cols::updatedate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leUpdatedate(T val)
	{return where(B_BASE::cols::updatedate, orm::wq::le, val);
	}

M_MODEL& nullUpdatedate()
	{return whereNull(B_BASE::cols::updatedate);
	}

M_MODEL& notnullUpdatedate()
	{return whereNotNull(B_BASE::cols::updatedate);
	}

M_MODEL& oreqUpdatedate(const std::string &val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::eq, val);
	}

M_MODEL& ornqUpdatedate(const std::string &val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::nq, val);
	}

M_MODEL& orbtUpdatedate(const std::string &val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::bt, val);
	}

M_MODEL& orbeUpdatedate(const std::string &val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::be, val);
	}

M_MODEL& orltUpdatedate(const std::string &val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::lt, val);
	}

M_MODEL& orleUpdatedate(const std::string &val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqUpdatedate(T val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqUpdatedate(T val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtUpdatedate(T val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeUpdatedate(T val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltUpdatedate(T val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleUpdatedate(T val)
	{return whereOr(B_BASE::cols::updatedate, orm::wq::le, val);
	}

M_MODEL& ornullUpdatedate()
	{return whereOrNull(B_BASE::cols::updatedate);
	}

M_MODEL& ornotnullUpdatedate()
	{return whereOrNotNull(B_BASE::cols::updatedate);
	}

M_MODEL& eqFinishdate(const std::string &val)
	{return where(B_BASE::cols::finishdate, orm::wq::eq, val);
	}

M_MODEL& nqFinishdate(const std::string &val)
	{return where(B_BASE::cols::finishdate, orm::wq::nq, val);
	}

M_MODEL& btFinishdate(const std::string &val)
	{return where(B_BASE::cols::finishdate, orm::wq::bt, val);
	}

M_MODEL& beFinishdate(const std::string &val)
	{return where(B_BASE::cols::finishdate, orm::wq::be, val);
	}

M_MODEL& ltFinishdate(const std::string &val)
	{return where(B_BASE::cols::finishdate, orm::wq::lt, val);
	}

M_MODEL& leFinishdate(const std::string &val)
	{return where(B_BASE::cols::finishdate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqFinishdate(T val)
	{return where(B_BASE::cols::finishdate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqFinishdate(T val)
	{return where(B_BASE::cols::finishdate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btFinishdate(T val)
	{return where(B_BASE::cols::finishdate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beFinishdate(T val)
	{return where(B_BASE::cols::finishdate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltFinishdate(T val)
	{return where(B_BASE::cols::finishdate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leFinishdate(T val)
	{return where(B_BASE::cols::finishdate, orm::wq::le, val);
	}

M_MODEL& nullFinishdate()
	{return whereNull(B_BASE::cols::finishdate);
	}

M_MODEL& notnullFinishdate()
	{return whereNotNull(B_BASE::cols::finishdate);
	}

M_MODEL& oreqFinishdate(const std::string &val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::eq, val);
	}

M_MODEL& ornqFinishdate(const std::string &val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::nq, val);
	}

M_MODEL& orbtFinishdate(const std::string &val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::bt, val);
	}

M_MODEL& orbeFinishdate(const std::string &val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::be, val);
	}

M_MODEL& orltFinishdate(const std::string &val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::lt, val);
	}

M_MODEL& orleFinishdate(const std::string &val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqFinishdate(T val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqFinishdate(T val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtFinishdate(T val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeFinishdate(T val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltFinishdate(T val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleFinishdate(T val)
	{return whereOr(B_BASE::cols::finishdate, orm::wq::le, val);
	}

M_MODEL& ornullFinishdate()
	{return whereOrNull(B_BASE::cols::finishdate);
	}

M_MODEL& ornotnullFinishdate()
	{return whereOrNotNull(B_BASE::cols::finishdate);
	}

M_MODEL& eqIscore(const std::string &val)
	{return where(B_BASE::cols::iscore, orm::wq::eq, val);
	}

M_MODEL& nqIscore(const std::string &val)
	{return where(B_BASE::cols::iscore, orm::wq::nq, val);
	}

M_MODEL& btIscore(const std::string &val)
	{return where(B_BASE::cols::iscore, orm::wq::bt, val);
	}

M_MODEL& beIscore(const std::string &val)
	{return where(B_BASE::cols::iscore, orm::wq::be, val);
	}

M_MODEL& ltIscore(const std::string &val)
	{return where(B_BASE::cols::iscore, orm::wq::lt, val);
	}

M_MODEL& leIscore(const std::string &val)
	{return where(B_BASE::cols::iscore, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqIscore(T val)
	{return where(B_BASE::cols::iscore, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqIscore(T val)
	{return where(B_BASE::cols::iscore, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btIscore(T val)
	{return where(B_BASE::cols::iscore, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beIscore(T val)
	{return where(B_BASE::cols::iscore, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltIscore(T val)
	{return where(B_BASE::cols::iscore, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leIscore(T val)
	{return where(B_BASE::cols::iscore, orm::wq::le, val);
	}

M_MODEL& nullIscore()
	{return whereNull(B_BASE::cols::iscore);
	}

M_MODEL& notnullIscore()
	{return whereNotNull(B_BASE::cols::iscore);
	}

M_MODEL& oreqIscore(const std::string &val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::eq, val);
	}

M_MODEL& ornqIscore(const std::string &val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::nq, val);
	}

M_MODEL& orbtIscore(const std::string &val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::bt, val);
	}

M_MODEL& orbeIscore(const std::string &val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::be, val);
	}

M_MODEL& orltIscore(const std::string &val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::lt, val);
	}

M_MODEL& orleIscore(const std::string &val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqIscore(T val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqIscore(T val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtIscore(T val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeIscore(T val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltIscore(T val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleIscore(T val)
	{return whereOr(B_BASE::cols::iscore, orm::wq::le, val);
	}

M_MODEL& ornullIscore()
	{return whereOrNull(B_BASE::cols::iscore);
	}

M_MODEL& ornotnullIscore()
	{return whereOrNotNull(B_BASE::cols::iscore);
	}

M_MODEL& eqXvalue(const std::string &val)
	{return where(B_BASE::cols::xvalue, orm::wq::eq, val);
	}

M_MODEL& nqXvalue(const std::string &val)
	{return where(B_BASE::cols::xvalue, orm::wq::nq, val);
	}

M_MODEL& btXvalue(const std::string &val)
	{return where(B_BASE::cols::xvalue, orm::wq::bt, val);
	}

M_MODEL& beXvalue(const std::string &val)
	{return where(B_BASE::cols::xvalue, orm::wq::be, val);
	}

M_MODEL& ltXvalue(const std::string &val)
	{return where(B_BASE::cols::xvalue, orm::wq::lt, val);
	}

M_MODEL& leXvalue(const std::string &val)
	{return where(B_BASE::cols::xvalue, orm::wq::le, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& eqXvalue(T val)
	{return where(B_BASE::cols::xvalue, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& nqXvalue(T val)
	{return where(B_BASE::cols::xvalue, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& btXvalue(T val)
	{return where(B_BASE::cols::xvalue, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& beXvalue(T val)
	{return where(B_BASE::cols::xvalue, orm::wq::be, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& ltXvalue(T val)
	{return where(B_BASE::cols::xvalue, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& leXvalue(T val)
	{return where(B_BASE::cols::xvalue, orm::wq::le, val);
	}

M_MODEL& nullXvalue()
	{return whereNull(B_BASE::cols::xvalue);
	}

M_MODEL& notnullXvalue()
	{return whereNotNull(B_BASE::cols::xvalue);
	}

M_MODEL& oreqXvalue(const std::string &val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::eq, val);
	}

M_MODEL& ornqXvalue(const std::string &val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::nq, val);
	}

M_MODEL& orbtXvalue(const std::string &val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::bt, val);
	}

M_MODEL& orbeXvalue(const std::string &val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::be, val);
	}

M_MODEL& orltXvalue(const std::string &val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::lt, val);
	}

M_MODEL& orleXvalue(const std::string &val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::le, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& oreqXvalue(T val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& ornqXvalue(T val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orbtXvalue(T val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orbeXvalue(T val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::be, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orltXvalue(T val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orleXvalue(T val)
	{return whereOr(B_BASE::cols::xvalue, orm::wq::le, val);
	}

M_MODEL& ornullXvalue()
	{return whereOrNull(B_BASE::cols::xvalue);
	}

M_MODEL& ornotnullXvalue()
	{return whereOrNotNull(B_BASE::cols::xvalue);
	}

M_MODEL& eqExpectday(const std::string &val)
	{return where(B_BASE::cols::expectday, orm::wq::eq, val);
	}

M_MODEL& nqExpectday(const std::string &val)
	{return where(B_BASE::cols::expectday, orm::wq::nq, val);
	}

M_MODEL& btExpectday(const std::string &val)
	{return where(B_BASE::cols::expectday, orm::wq::bt, val);
	}

M_MODEL& beExpectday(const std::string &val)
	{return where(B_BASE::cols::expectday, orm::wq::be, val);
	}

M_MODEL& ltExpectday(const std::string &val)
	{return where(B_BASE::cols::expectday, orm::wq::lt, val);
	}

M_MODEL& leExpectday(const std::string &val)
	{return where(B_BASE::cols::expectday, orm::wq::le, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& eqExpectday(T val)
	{return where(B_BASE::cols::expectday, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& nqExpectday(T val)
	{return where(B_BASE::cols::expectday, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& btExpectday(T val)
	{return where(B_BASE::cols::expectday, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& beExpectday(T val)
	{return where(B_BASE::cols::expectday, orm::wq::be, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& ltExpectday(T val)
	{return where(B_BASE::cols::expectday, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& leExpectday(T val)
	{return where(B_BASE::cols::expectday, orm::wq::le, val);
	}

M_MODEL& nullExpectday()
	{return whereNull(B_BASE::cols::expectday);
	}

M_MODEL& notnullExpectday()
	{return whereNotNull(B_BASE::cols::expectday);
	}

M_MODEL& oreqExpectday(const std::string &val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::eq, val);
	}

M_MODEL& ornqExpectday(const std::string &val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::nq, val);
	}

M_MODEL& orbtExpectday(const std::string &val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::bt, val);
	}

M_MODEL& orbeExpectday(const std::string &val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::be, val);
	}

M_MODEL& orltExpectday(const std::string &val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::lt, val);
	}

M_MODEL& orleExpectday(const std::string &val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::le, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& oreqExpectday(T val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& ornqExpectday(T val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orbtExpectday(T val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orbeExpectday(T val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::be, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orltExpectday(T val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orleExpectday(T val)
	{return whereOr(B_BASE::cols::expectday, orm::wq::le, val);
	}

M_MODEL& ornullExpectday()
	{return whereOrNull(B_BASE::cols::expectday);
	}

M_MODEL& ornotnullExpectday()
	{return whereOrNotNull(B_BASE::cols::expectday);
	}

M_MODEL& eqRealday(const std::string &val)
	{return where(B_BASE::cols::realday, orm::wq::eq, val);
	}

M_MODEL& nqRealday(const std::string &val)
	{return where(B_BASE::cols::realday, orm::wq::nq, val);
	}

M_MODEL& btRealday(const std::string &val)
	{return where(B_BASE::cols::realday, orm::wq::bt, val);
	}

M_MODEL& beRealday(const std::string &val)
	{return where(B_BASE::cols::realday, orm::wq::be, val);
	}

M_MODEL& ltRealday(const std::string &val)
	{return where(B_BASE::cols::realday, orm::wq::lt, val);
	}

M_MODEL& leRealday(const std::string &val)
	{return where(B_BASE::cols::realday, orm::wq::le, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& eqRealday(T val)
	{return where(B_BASE::cols::realday, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& nqRealday(T val)
	{return where(B_BASE::cols::realday, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& btRealday(T val)
	{return where(B_BASE::cols::realday, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& beRealday(T val)
	{return where(B_BASE::cols::realday, orm::wq::be, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& ltRealday(T val)
	{return where(B_BASE::cols::realday, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& leRealday(T val)
	{return where(B_BASE::cols::realday, orm::wq::le, val);
	}

M_MODEL& nullRealday()
	{return whereNull(B_BASE::cols::realday);
	}

M_MODEL& notnullRealday()
	{return whereNotNull(B_BASE::cols::realday);
	}

M_MODEL& oreqRealday(const std::string &val)
	{return whereOr(B_BASE::cols::realday, orm::wq::eq, val);
	}

M_MODEL& ornqRealday(const std::string &val)
	{return whereOr(B_BASE::cols::realday, orm::wq::nq, val);
	}

M_MODEL& orbtRealday(const std::string &val)
	{return whereOr(B_BASE::cols::realday, orm::wq::bt, val);
	}

M_MODEL& orbeRealday(const std::string &val)
	{return whereOr(B_BASE::cols::realday, orm::wq::be, val);
	}

M_MODEL& orltRealday(const std::string &val)
	{return whereOr(B_BASE::cols::realday, orm::wq::lt, val);
	}

M_MODEL& orleRealday(const std::string &val)
	{return whereOr(B_BASE::cols::realday, orm::wq::le, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& oreqRealday(T val)
	{return whereOr(B_BASE::cols::realday, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& ornqRealday(T val)
	{return whereOr(B_BASE::cols::realday, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orbtRealday(T val)
	{return whereOr(B_BASE::cols::realday, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orbeRealday(T val)
	{return whereOr(B_BASE::cols::realday, orm::wq::be, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orltRealday(T val)
	{return whereOr(B_BASE::cols::realday, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orleRealday(T val)
	{return whereOr(B_BASE::cols::realday, orm::wq::le, val);
	}

M_MODEL& ornullRealday()
	{return whereOrNull(B_BASE::cols::realday);
	}

M_MODEL& ornotnullRealday()
	{return whereOrNotNull(B_BASE::cols::realday);
	}

M_MODEL& eqPricevalue(const std::string &val)
	{return where(B_BASE::cols::pricevalue, orm::wq::eq, val);
	}

M_MODEL& nqPricevalue(const std::string &val)
	{return where(B_BASE::cols::pricevalue, orm::wq::nq, val);
	}

M_MODEL& btPricevalue(const std::string &val)
	{return where(B_BASE::cols::pricevalue, orm::wq::bt, val);
	}

M_MODEL& bePricevalue(const std::string &val)
	{return where(B_BASE::cols::pricevalue, orm::wq::be, val);
	}

M_MODEL& ltPricevalue(const std::string &val)
	{return where(B_BASE::cols::pricevalue, orm::wq::lt, val);
	}

M_MODEL& lePricevalue(const std::string &val)
	{return where(B_BASE::cols::pricevalue, orm::wq::le, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& eqPricevalue(T val)
	{return where(B_BASE::cols::pricevalue, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& nqPricevalue(T val)
	{return where(B_BASE::cols::pricevalue, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& btPricevalue(T val)
	{return where(B_BASE::cols::pricevalue, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& bePricevalue(T val)
	{return where(B_BASE::cols::pricevalue, orm::wq::be, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& ltPricevalue(T val)
	{return where(B_BASE::cols::pricevalue, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& lePricevalue(T val)
	{return where(B_BASE::cols::pricevalue, orm::wq::le, val);
	}

M_MODEL& nullPricevalue()
	{return whereNull(B_BASE::cols::pricevalue);
	}

M_MODEL& notnullPricevalue()
	{return whereNotNull(B_BASE::cols::pricevalue);
	}

M_MODEL& oreqPricevalue(const std::string &val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::eq, val);
	}

M_MODEL& ornqPricevalue(const std::string &val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::nq, val);
	}

M_MODEL& orbtPricevalue(const std::string &val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::bt, val);
	}

M_MODEL& orbePricevalue(const std::string &val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::be, val);
	}

M_MODEL& orltPricevalue(const std::string &val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::lt, val);
	}

M_MODEL& orlePricevalue(const std::string &val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::le, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& oreqPricevalue(T val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& ornqPricevalue(T val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orbtPricevalue(T val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orbePricevalue(T val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::be, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orltPricevalue(T val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_arithmetic_v<T>
M_MODEL& orlePricevalue(T val)
	{return whereOr(B_BASE::cols::pricevalue, orm::wq::le, val);
	}

M_MODEL& ornullPricevalue()
	{return whereOrNull(B_BASE::cols::pricevalue);
	}

M_MODEL& ornotnullPricevalue()
	{return whereOrNotNull(B_BASE::cols::pricevalue);
	}

M_MODEL& eqTitle(const std::string &val)
	{return where(B_BASE::cols::title, orm::wq::eq, val);
	}

M_MODEL& nqTitle(const std::string &val)
	{return where(B_BASE::cols::title, orm::wq::nq, val);
	}

M_MODEL& btTitle(const std::string &val)
	{return where(B_BASE::cols::title, orm::wq::bt, val);
	}

M_MODEL& beTitle(const std::string &val)
	{return where(B_BASE::cols::title, orm::wq::be, val);
	}

M_MODEL& ltTitle(const std::string &val)
	{return where(B_BASE::cols::title, orm::wq::lt, val);
	}

M_MODEL& leTitle(const std::string &val)
	{return where(B_BASE::cols::title, orm::wq::le, val);
	}

M_MODEL& likeTitle(const std::string &val)
	{return where(B_BASE::cols::title, orm::wq::like, val);
	}

M_MODEL& nullTitle()
	{return whereNull(B_BASE::cols::title);
	}

M_MODEL& notnullTitle()
	{return whereNotNull(B_BASE::cols::title);
	}

M_MODEL& oreqTitle(const std::string &val)
	{return whereOr(B_BASE::cols::title, orm::wq::eq, val);
	}

M_MODEL& ornqTitle(const std::string &val)
	{return whereOr(B_BASE::cols::title, orm::wq::nq, val);
	}

M_MODEL& orbtTitle(const std::string &val)
	{return whereOr(B_BASE::cols::title, orm::wq::bt, val);
	}

M_MODEL& orbeTitle(const std::string &val)
	{return whereOr(B_BASE::cols::title, orm::wq::be, val);
	}

M_MODEL& orltTitle(const std::string &val)
	{return whereOr(B_BASE::cols::title, orm::wq::lt, val);
	}

M_MODEL& orleTitle(const std::string &val)
	{return whereOr(B_BASE::cols::title, orm::wq::le, val);
	}

M_MODEL& orlikeTitle(const std::string &val)
	{return whereOr(B_BASE::cols::title, orm::wq::like, val);
	}

M_MODEL& ornullTitle()
	{return whereOrNull(B_BASE::cols::title);
	}

M_MODEL& ornotnullTitle()
	{return whereOrNotNull(B_BASE::cols::title);
	}

M_MODEL& eqIntroduce(const std::string &val)
	{return where(B_BASE::cols::introduce, orm::wq::eq, val);
	}

M_MODEL& nqIntroduce(const std::string &val)
	{return where(B_BASE::cols::introduce, orm::wq::nq, val);
	}

M_MODEL& btIntroduce(const std::string &val)
	{return where(B_BASE::cols::introduce, orm::wq::bt, val);
	}

M_MODEL& beIntroduce(const std::string &val)
	{return where(B_BASE::cols::introduce, orm::wq::be, val);
	}

M_MODEL& ltIntroduce(const std::string &val)
	{return where(B_BASE::cols::introduce, orm::wq::lt, val);
	}

M_MODEL& leIntroduce(const std::string &val)
	{return where(B_BASE::cols::introduce, orm::wq::le, val);
	}

M_MODEL& likeIntroduce(const std::string &val)
	{return where(B_BASE::cols::introduce, orm::wq::like, val);
	}

M_MODEL& nullIntroduce()
	{return whereNull(B_BASE::cols::introduce);
	}

M_MODEL& notnullIntroduce()
	{return whereNotNull(B_BASE::cols::introduce);
	}

M_MODEL& oreqIntroduce(const std::string &val)
	{return whereOr(B_BASE::cols::introduce, orm::wq::eq, val);
	}

M_MODEL& ornqIntroduce(const std::string &val)
	{return whereOr(B_BASE::cols::introduce, orm::wq::nq, val);
	}

M_MODEL& orbtIntroduce(const std::string &val)
	{return whereOr(B_BASE::cols::introduce, orm::wq::bt, val);
	}

M_MODEL& orbeIntroduce(const std::string &val)
	{return whereOr(B_BASE::cols::introduce, orm::wq::be, val);
	}

M_MODEL& orltIntroduce(const std::string &val)
	{return whereOr(B_BASE::cols::introduce, orm::wq::lt, val);
	}

M_MODEL& orleIntroduce(const std::string &val)
	{return whereOr(B_BASE::cols::introduce, orm::wq::le, val);
	}

M_MODEL& orlikeIntroduce(const std::string &val)
	{return whereOr(B_BASE::cols::introduce, orm::wq::like, val);
	}

M_MODEL& ornullIntroduce()
	{return whereOrNull(B_BASE::cols::introduce);
	}

M_MODEL& ornotnullIntroduce()
	{return whereOrNotNull(B_BASE::cols::introduce);
	}

M_MODEL& eqXlogo(const std::string &val)
	{return where(B_BASE::cols::xlogo, orm::wq::eq, val);
	}

M_MODEL& nqXlogo(const std::string &val)
	{return where(B_BASE::cols::xlogo, orm::wq::nq, val);
	}

M_MODEL& btXlogo(const std::string &val)
	{return where(B_BASE::cols::xlogo, orm::wq::bt, val);
	}

M_MODEL& beXlogo(const std::string &val)
	{return where(B_BASE::cols::xlogo, orm::wq::be, val);
	}

M_MODEL& ltXlogo(const std::string &val)
	{return where(B_BASE::cols::xlogo, orm::wq::lt, val);
	}

M_MODEL& leXlogo(const std::string &val)
	{return where(B_BASE::cols::xlogo, orm::wq::le, val);
	}

M_MODEL& likeXlogo(const std::string &val)
	{return where(B_BASE::cols::xlogo, orm::wq::like, val);
	}

M_MODEL& nullXlogo()
	{return whereNull(B_BASE::cols::xlogo);
	}

M_MODEL& notnullXlogo()
	{return whereNotNull(B_BASE::cols::xlogo);
	}

M_MODEL& oreqXlogo(const std::string &val)
	{return whereOr(B_BASE::cols::xlogo, orm::wq::eq, val);
	}

M_MODEL& ornqXlogo(const std::string &val)
	{return whereOr(B_BASE::cols::xlogo, orm::wq::nq, val);
	}

M_MODEL& orbtXlogo(const std::string &val)
	{return whereOr(B_BASE::cols::xlogo, orm::wq::bt, val);
	}

M_MODEL& orbeXlogo(const std::string &val)
	{return whereOr(B_BASE::cols::xlogo, orm::wq::be, val);
	}

M_MODEL& orltXlogo(const std::string &val)
	{return whereOr(B_BASE::cols::xlogo, orm::wq::lt, val);
	}

M_MODEL& orleXlogo(const std::string &val)
	{return whereOr(B_BASE::cols::xlogo, orm::wq::le, val);
	}

M_MODEL& orlikeXlogo(const std::string &val)
	{return whereOr(B_BASE::cols::xlogo, orm::wq::like, val);
	}

M_MODEL& ornullXlogo()
	{return whereOrNull(B_BASE::cols::xlogo);
	}

M_MODEL& ornotnullXlogo()
	{return whereOrNotNull(B_BASE::cols::xlogo);
	}

M_MODEL& eqXcolor(const std::string &val)
	{return where(B_BASE::cols::xcolor, orm::wq::eq, val);
	}

M_MODEL& nqXcolor(const std::string &val)
	{return where(B_BASE::cols::xcolor, orm::wq::nq, val);
	}

M_MODEL& btXcolor(const std::string &val)
	{return where(B_BASE::cols::xcolor, orm::wq::bt, val);
	}

M_MODEL& beXcolor(const std::string &val)
	{return where(B_BASE::cols::xcolor, orm::wq::be, val);
	}

M_MODEL& ltXcolor(const std::string &val)
	{return where(B_BASE::cols::xcolor, orm::wq::lt, val);
	}

M_MODEL& leXcolor(const std::string &val)
	{return where(B_BASE::cols::xcolor, orm::wq::le, val);
	}

M_MODEL& likeXcolor(const std::string &val)
	{return where(B_BASE::cols::xcolor, orm::wq::like, val);
	}

M_MODEL& nullXcolor()
	{return whereNull(B_BASE::cols::xcolor);
	}

M_MODEL& notnullXcolor()
	{return whereNotNull(B_BASE::cols::xcolor);
	}

M_MODEL& oreqXcolor(const std::string &val)
	{return whereOr(B_BASE::cols::xcolor, orm::wq::eq, val);
	}

M_MODEL& ornqXcolor(const std::string &val)
	{return whereOr(B_BASE::cols::xcolor, orm::wq::nq, val);
	}

M_MODEL& orbtXcolor(const std::string &val)
	{return whereOr(B_BASE::cols::xcolor, orm::wq::bt, val);
	}

M_MODEL& orbeXcolor(const std::string &val)
	{return whereOr(B_BASE::cols::xcolor, orm::wq::be, val);
	}

M_MODEL& orltXcolor(const std::string &val)
	{return whereOr(B_BASE::cols::xcolor, orm::wq::lt, val);
	}

M_MODEL& orleXcolor(const std::string &val)
	{return whereOr(B_BASE::cols::xcolor, orm::wq::le, val);
	}

M_MODEL& orlikeXcolor(const std::string &val)
	{return whereOr(B_BASE::cols::xcolor, orm::wq::like, val);
	}

M_MODEL& ornullXcolor()
	{return whereOrNull(B_BASE::cols::xcolor);
	}

M_MODEL& ornotnullXcolor()
	{return whereOrNotNull(B_BASE::cols::xcolor);
	}

M_MODEL& eqPullurl(const std::string &val)
	{return where(B_BASE::cols::pullurl, orm::wq::eq, val);
	}

M_MODEL& nqPullurl(const std::string &val)
	{return where(B_BASE::cols::pullurl, orm::wq::nq, val);
	}

M_MODEL& btPullurl(const std::string &val)
	{return where(B_BASE::cols::pullurl, orm::wq::bt, val);
	}

M_MODEL& bePullurl(const std::string &val)
	{return where(B_BASE::cols::pullurl, orm::wq::be, val);
	}

M_MODEL& ltPullurl(const std::string &val)
	{return where(B_BASE::cols::pullurl, orm::wq::lt, val);
	}

M_MODEL& lePullurl(const std::string &val)
	{return where(B_BASE::cols::pullurl, orm::wq::le, val);
	}

M_MODEL& likePullurl(const std::string &val)
	{return where(B_BASE::cols::pullurl, orm::wq::like, val);
	}

M_MODEL& nullPullurl()
	{return whereNull(B_BASE::cols::pullurl);
	}

M_MODEL& notnullPullurl()
	{return whereNotNull(B_BASE::cols::pullurl);
	}

M_MODEL& oreqPullurl(const std::string &val)
	{return whereOr(B_BASE::cols::pullurl, orm::wq::eq, val);
	}

M_MODEL& ornqPullurl(const std::string &val)
	{return whereOr(B_BASE::cols::pullurl, orm::wq::nq, val);
	}

M_MODEL& orbtPullurl(const std::string &val)
	{return whereOr(B_BASE::cols::pullurl, orm::wq::bt, val);
	}

M_MODEL& orbePullurl(const std::string &val)
	{return whereOr(B_BASE::cols::pullurl, orm::wq::be, val);
	}

M_MODEL& orltPullurl(const std::string &val)
	{return whereOr(B_BASE::cols::pullurl, orm::wq::lt, val);
	}

M_MODEL& orlePullurl(const std::string &val)
	{return whereOr(B_BASE::cols::pullurl, orm::wq::le, val);
	}

M_MODEL& orlikePullurl(const std::string &val)
	{return whereOr(B_BASE::cols::pullurl, orm::wq::like, val);
	}

M_MODEL& ornullPullurl()
	{return whereOrNull(B_BASE::cols::pullurl);
	}

M_MODEL& ornotnullPullurl()
	{return whereOrNotNull(B_BASE::cols::pullurl);
	}

M_MODEL& eqPulltitle(const std::string &val)
	{return where(B_BASE::cols::pulltitle, orm::wq::eq, val);
	}

M_MODEL& nqPulltitle(const std::string &val)
	{return where(B_BASE::cols::pulltitle, orm::wq::nq, val);
	}

M_MODEL& btPulltitle(const std::string &val)
	{return where(B_BASE::cols::pulltitle, orm::wq::bt, val);
	}

M_MODEL& bePulltitle(const std::string &val)
	{return where(B_BASE::cols::pulltitle, orm::wq::be, val);
	}

M_MODEL& ltPulltitle(const std::string &val)
	{return where(B_BASE::cols::pulltitle, orm::wq::lt, val);
	}

M_MODEL& lePulltitle(const std::string &val)
	{return where(B_BASE::cols::pulltitle, orm::wq::le, val);
	}

M_MODEL& likePulltitle(const std::string &val)
	{return where(B_BASE::cols::pulltitle, orm::wq::like, val);
	}

M_MODEL& nullPulltitle()
	{return whereNull(B_BASE::cols::pulltitle);
	}

M_MODEL& notnullPulltitle()
	{return whereNotNull(B_BASE::cols::pulltitle);
	}

M_MODEL& oreqPulltitle(const std::string &val)
	{return whereOr(B_BASE::cols::pulltitle, orm::wq::eq, val);
	}

M_MODEL& ornqPulltitle(const std::string &val)
	{return whereOr(B_BASE::cols::pulltitle, orm::wq::nq, val);
	}

M_MODEL& orbtPulltitle(const std::string &val)
	{return whereOr(B_BASE::cols::pulltitle, orm::wq::bt, val);
	}

M_MODEL& orbePulltitle(const std::string &val)
	{return whereOr(B_BASE::cols::pulltitle, orm::wq::be, val);
	}

M_MODEL& orltPulltitle(const std::string &val)
	{return whereOr(B_BASE::cols::pulltitle, orm::wq::lt, val);
	}

M_MODEL& orlePulltitle(const std::string &val)
	{return whereOr(B_BASE::cols::pulltitle, orm::wq::le, val);
	}

M_MODEL& orlikePulltitle(const std::string &val)
	{return whereOr(B_BASE::cols::pulltitle, orm::wq::like, val);
	}

M_MODEL& ornullPulltitle()
	{return whereOrNull(B_BASE::cols::pulltitle);
	}

M_MODEL& ornotnullPulltitle()
	{return whereOrNotNull(B_BASE::cols::pulltitle);
	}

M_MODEL& eqPullauthor(const std::string &val)
	{return where(B_BASE::cols::pullauthor, orm::wq::eq, val);
	}

M_MODEL& nqPullauthor(const std::string &val)
	{return where(B_BASE::cols::pullauthor, orm::wq::nq, val);
	}

M_MODEL& btPullauthor(const std::string &val)
	{return where(B_BASE::cols::pullauthor, orm::wq::bt, val);
	}

M_MODEL& bePullauthor(const std::string &val)
	{return where(B_BASE::cols::pullauthor, orm::wq::be, val);
	}

M_MODEL& ltPullauthor(const std::string &val)
	{return where(B_BASE::cols::pullauthor, orm::wq::lt, val);
	}

M_MODEL& lePullauthor(const std::string &val)
	{return where(B_BASE::cols::pullauthor, orm::wq::le, val);
	}

M_MODEL& likePullauthor(const std::string &val)
	{return where(B_BASE::cols::pullauthor, orm::wq::like, val);
	}

M_MODEL& nullPullauthor()
	{return whereNull(B_BASE::cols::pullauthor);
	}

M_MODEL& notnullPullauthor()
	{return whereNotNull(B_BASE::cols::pullauthor);
	}

M_MODEL& oreqPullauthor(const std::string &val)
	{return whereOr(B_BASE::cols::pullauthor, orm::wq::eq, val);
	}

M_MODEL& ornqPullauthor(const std::string &val)
	{return whereOr(B_BASE::cols::pullauthor, orm::wq::nq, val);
	}

M_MODEL& orbtPullauthor(const std::string &val)
	{return whereOr(B_BASE::cols::pullauthor, orm::wq::bt, val);
	}

M_MODEL& orbePullauthor(const std::string &val)
	{return whereOr(B_BASE::cols::pullauthor, orm::wq::be, val);
	}

M_MODEL& orltPullauthor(const std::string &val)
	{return whereOr(B_BASE::cols::pullauthor, orm::wq::lt, val);
	}

M_MODEL& orlePullauthor(const std::string &val)
	{return whereOr(B_BASE::cols::pullauthor, orm::wq::le, val);
	}

M_MODEL& orlikePullauthor(const std::string &val)
	{return whereOr(B_BASE::cols::pullauthor, orm::wq::like, val);
	}

M_MODEL& ornullPullauthor()
	{return whereOrNull(B_BASE::cols::pullauthor);
	}

M_MODEL& ornotnullPullauthor()
	{return whereOrNotNull(B_BASE::cols::pullauthor);
	}

M_MODEL& eqNote(const std::string &val)
	{return where(B_BASE::cols::note, orm::wq::eq, val);
	}

M_MODEL& nqNote(const std::string &val)
	{return where(B_BASE::cols::note, orm::wq::nq, val);
	}

M_MODEL& btNote(const std::string &val)
	{return where(B_BASE::cols::note, orm::wq::bt, val);
	}

M_MODEL& beNote(const std::string &val)
	{return where(B_BASE::cols::note, orm::wq::be, val);
	}

M_MODEL& ltNote(const std::string &val)
	{return where(B_BASE::cols::note, orm::wq::lt, val);
	}

M_MODEL& leNote(const std::string &val)
	{return where(B_BASE::cols::note, orm::wq::le, val);
	}

M_MODEL& likeNote(const std::string &val)
	{return where(B_BASE::cols::note, orm::wq::like, val);
	}

M_MODEL& nullNote()
	{return whereNull(B_BASE::cols::note);
	}

M_MODEL& notnullNote()
	{return whereNotNull(B_BASE::cols::note);
	}

M_MODEL& oreqNote(const std::string &val)
	{return whereOr(B_BASE::cols::note, orm::wq::eq, val);
	}

M_MODEL& ornqNote(const std::string &val)
	{return whereOr(B_BASE::cols::note, orm::wq::nq, val);
	}

M_MODEL& orbtNote(const std::string &val)
	{return whereOr(B_BASE::cols::note, orm::wq::bt, val);
	}

M_MODEL& orbeNote(const std::string &val)
	{return whereOr(B_BASE::cols::note, orm::wq::be, val);
	}

M_MODEL& orltNote(const std::string &val)
	{return whereOr(B_BASE::cols::note, orm::wq::lt, val);
	}

M_MODEL& orleNote(const std::string &val)
	{return whereOr(B_BASE::cols::note, orm::wq::le, val);
	}

M_MODEL& orlikeNote(const std::string &val)
	{return whereOr(B_BASE::cols::note, orm::wq::like, val);
	}

M_MODEL& ornullNote()
	{return whereOrNull(B_BASE::cols::note);
	}

M_MODEL& ornotnullNote()
	{return whereOrNotNull(B_BASE::cols::note);
	}

M_MODEL& eqItemnote(const std::string &val)
	{return where(B_BASE::cols::itemnote, orm::wq::eq, val);
	}

M_MODEL& nqItemnote(const std::string &val)
	{return where(B_BASE::cols::itemnote, orm::wq::nq, val);
	}

M_MODEL& btItemnote(const std::string &val)
	{return where(B_BASE::cols::itemnote, orm::wq::bt, val);
	}

M_MODEL& beItemnote(const std::string &val)
	{return where(B_BASE::cols::itemnote, orm::wq::be, val);
	}

M_MODEL& ltItemnote(const std::string &val)
	{return where(B_BASE::cols::itemnote, orm::wq::lt, val);
	}

M_MODEL& leItemnote(const std::string &val)
	{return where(B_BASE::cols::itemnote, orm::wq::le, val);
	}

M_MODEL& likeItemnote(const std::string &val)
	{return where(B_BASE::cols::itemnote, orm::wq::like, val);
	}

M_MODEL& nullItemnote()
	{return whereNull(B_BASE::cols::itemnote);
	}

M_MODEL& notnullItemnote()
	{return whereNotNull(B_BASE::cols::itemnote);
	}

M_MODEL& oreqItemnote(const std::string &val)
	{return whereOr(B_BASE::cols::itemnote, orm::wq::eq, val);
	}

M_MODEL& ornqItemnote(const std::string &val)
	{return whereOr(B_BASE::cols::itemnote, orm::wq::nq, val);
	}

M_MODEL& orbtItemnote(const std::string &val)
	{return whereOr(B_BASE::cols::itemnote, orm::wq::bt, val);
	}

M_MODEL& orbeItemnote(const std::string &val)
	{return whereOr(B_BASE::cols::itemnote, orm::wq::be, val);
	}

M_MODEL& orltItemnote(const std::string &val)
	{return whereOr(B_BASE::cols::itemnote, orm::wq::lt, val);
	}

M_MODEL& orleItemnote(const std::string &val)
	{return whereOr(B_BASE::cols::itemnote, orm::wq::le, val);
	}

M_MODEL& orlikeItemnote(const std::string &val)
	{return whereOr(B_BASE::cols::itemnote, orm::wq::like, val);
	}

M_MODEL& ornullItemnote()
	{return whereOrNull(B_BASE::cols::itemnote);
	}

M_MODEL& ornotnullItemnote()
	{return whereOrNotNull(B_BASE::cols::itemnote);
	}

        M_MODEL &select(std::string_view fields)
        {
            if (selectsql.size() > 0)
            {
                selectsql.push_back(',');
            }
            selectsql.append(fields);
            return *mod;
        }

        // === 兼容旧版 char/string 操作符 ===
        static orm::wq char_to_wq(char op)
        {
            switch (op)
            {
            case '=': return orm::wq::eq;
            case '>': return orm::wq::bt;
            case '<': return orm::wq::lt;
            case '!': return orm::wq::nq;// '!='
            default: return orm::wq::eq;
            }
        }
        static orm::wq str_to_wq(std::string_view op)
        {
            if (op == "=" || op == "==")
                return orm::wq::eq;
            if (op == ">=")
                return orm::wq::be;
            if (op == "<=")
                return orm::wq::le;
            if (op == "!=" || op == "<>")
                return orm::wq::nq;
            if (op == ">")
                return orm::wq::bt;
            if (op == "<")
                return orm::wq::lt;
            if (op == "LIKE" || op == "like")
                return orm::wq::like;
            if (op == "NOT LIKE" || op == "not like")
                return orm::wq::nlike;
            return orm::wq::eq;
        }

        // 3 参数兼容: where(str, '>', val) = where(str, orm::wq::bt, val)
        template <typename T>
        M_MODEL &where(std::string_view wq, char op, T &&val)
        {
            return where(wq, char_to_wq(op), std::forward<T>(val));
        }
        template <typename T>
        M_MODEL &whereOr(std::string_view wq, char op, T &&val)
        {
            return whereOr(wq, char_to_wq(op), std::forward<T>(val));
        }
        // 3 参数兼容: where(str, ">=", val)
        template <typename T>
        M_MODEL &where(std::string_view wq, std::string_view op, T &&val)
        {
            return where(wq, str_to_wq(op), std::forward<T>(val));
        }
        template <typename T>
        M_MODEL &whereOr(std::string_view wq, std::string_view op, T &&val)
        {
            return whereOr(wq, str_to_wq(op), std::forward<T>(val));
        }

        // ===== where 核心入口（所有具名函数转发到此）=====

        // cols 版 — 带 switch 校验        // 2 参数兼容: where(col, val) = where(col, orm::wq::eq, val)
        template <typename T>
        M_MODEL &where(B_BASE::cols field, T &&val)
        {
            return where(field, orm::wq::eq, std::forward<T>(val));
        }

        template <typename T>
        M_MODEL &where(B_BASE::cols field, orm::wq op, T &&val)
        {
            orm_where_sql_t item;
            item.pre_op      = wheresql.empty() ? 0 : 1;
            item.op_type     = op;
            item.col_idx     = static_cast<unsigned char>(field);
            item.need_quote  = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name  = xtask_info::col_names[item.col_idx];
            item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }// 2 参数兼容: whereOr(col, val) = whereOr(col, orm::wq::eq, val)
        template <typename T>
        M_MODEL &whereOr(B_BASE::cols field, T &&val)
        {
            return whereOr(field, orm::wq::eq, std::forward<T>(val));
        }

        template <typename T>
        M_MODEL &whereOr(B_BASE::cols field, orm::wq op, T &&val)
        {
            orm_where_sql_t item;
            item.pre_op      = wheresql.empty() ? 0 : 2;
            item.op_type     = op;
            item.col_idx     = static_cast<unsigned char>(field);
            item.need_quote  = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name  = xtask_info::col_names[item.col_idx];
            item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        // 列名编译期校验版(orm::table_col) // 2 参数兼容: where(str, val) = where(str, orm::wq::eq, val)
        template <typename T>
        M_MODEL &where(orm::table_col<B_BASE, &xtask_info::col_names> wq, T &&val)
        {
            return where(wq, orm::wq::eq, std::forward<T>(val));
        }

        template <typename T>
        M_MODEL &where(std::string_view wq, orm::wq op, T &&val)
        {
            orm_where_sql_t item;
            item.pre_op  = wheresql.empty() ? 0 : 1;
            item.op_type = op;
            item.col_idx = B_BASE::findcolpos(wq);

            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.need_quote  = (item.col_idx != 255) ? B_BASE::col_need_quote[item.col_idx] : true;
            item.filed_name  = wq;
            item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }// 2 参数兼容: whereOr(str, val) = whereOr(str, orm::wq::eq, val)
        template <typename T>
        M_MODEL &whereOr(orm::table_col<B_BASE, &xtask_info::col_names> wq, T &&val)
        {
            return whereOr(wq, orm::wq::eq, std::forward<T>(val));
        }

        template <typename T>
        M_MODEL &whereOr(std::string_view wq, orm::wq op, T &&val)
        {
            orm_where_sql_t item;
            item.pre_op  = wheresql.empty() ? 0 : 2;
            item.op_type = op;
            item.col_idx = B_BASE::findcolpos(wq);

            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.need_quote  = (item.col_idx != 255) ? B_BASE::col_need_quote[item.col_idx] : true;
            item.filed_name  = wq;
            item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        // IN / NOT IN 专用（set_array）

        // IN 核心入口（接收 vector，内部 set_array + 填充）
        template <typename T>
        M_MODEL &whereIn(std::string_view wq, orm::wq op, const std::vector<T> &a)
        {
            orm_where_sql_t item;
            item.pre_op  = wheresql.empty() ? 0 : 1;
            item.op_type = op;
            item.col_idx = B_BASE::findcolpos(wq);

            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.need_quote = (item.col_idx != 255) ? B_BASE::col_need_quote[item.col_idx] : true;
            item.filed_name = wq;
            item.filed_value.set_array();
            for (auto &v : a)
                item.filed_value.push(v);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        template <typename T>
        M_MODEL &whereOrIn(std::string_view wq, orm::wq op, const std::vector<T> &a)
        {
            orm_where_sql_t item;
            item.pre_op  = wheresql.empty() ? 0 : 2;
            item.op_type = op;
            item.col_idx = B_BASE::findcolpos(wq);

            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.need_quote = (item.col_idx != 255) ? B_BASE::col_need_quote[item.col_idx] : true;
            item.filed_name = wq;
            item.filed_value.set_array();
            for (auto &v : a)
                item.filed_value.push(v);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        template <typename T>
        M_MODEL &whereIn(B_BASE::cols field, orm::wq op, const std::vector<T> &a)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 1;
            item.op_type    = op;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = xtask_info::col_names[item.col_idx];
            item.filed_value.set_array();
            for (auto &v : a)
                item.filed_value.push(v);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        template <typename T>
        M_MODEL &whereOrIn(B_BASE::cols field, orm::wq op, const std::vector<T> &a)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 2;
            item.op_type    = op;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = xtask_info::col_names[item.col_idx];
            item.filed_value.set_array();
            for (auto &v : a)
                item.filed_value.push(v);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        // ===== 核心入口结束 =====

        // ===== IN / NOT IN 具名转发 =====

        template <typename T2>
        M_MODEL &whereIn(xtask_info::cols field, const std::vector<T2> &a)
        {
            return whereIn(field, orm::wq::in, a);
        }

        template <typename T2>
        M_MODEL &whereNotIn(xtask_info::cols field, const std::vector<T2> &a)
        {
            return whereIn(field, orm::wq::notin, a);
        }

        template <typename T2>
        M_MODEL &whereOrIn(xtask_info::cols field, const std::vector<T2> &a)
        {
            return whereOrIn(field, orm::wq::in, a);
        }

        template <typename T2>
        M_MODEL &whereOrNotIn(xtask_info::cols field, const std::vector<T2> &a)
        {
            return whereOrIn(field, orm::wq::notin, a);
        }

        M_MODEL &whereIn(orm::table_col<B_BASE, &xtask_info::col_names> wq, const std::vector<std::string> &a)
        {
            return whereIn(wq, orm::wq::in, a);
        }
        // 2 参数旧版兼容: whereIn("id", "1,2,3") = whereIn("id", split_csv("1,2,3"))
        M_MODEL &whereIn(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view csv_val)
        {
            std::vector<std::string> vec;
            std::string cur;
            for (char c : csv_val)
            {
                if (c == ',')
                {
                    if (!cur.empty())
                        vec.push_back(std::move(cur));
                    cur.clear();
                }

                else
                {
                    cur.push_back(c);
                }
            }
            if (!cur.empty())
                vec.push_back(std::move(cur));
            return whereIn(wq, vec);
        }

        // 2 参数模板版: whereIn("id", vector<T>) — 把任意 T 转成 string 再转发
        template <typename T>
        M_MODEL &whereIn(orm::table_col<B_BASE, &xtask_info::col_names> wq, const std::vector<T> &a)
        {
            std::vector<std::string> str_vec;
            str_vec.reserve(a.size());
            for (const auto &v : a)
            {
                std::ostringstream oss;
                oss << v;
                str_vec.push_back(oss.str());
            }
            return whereIn(wq, str_vec);
        }

        M_MODEL &whereNotIn(orm::table_col<B_BASE, &xtask_info::col_names> wq, const std::vector<std::string> &a)
        {
            return whereIn(wq, orm::wq::notin, a);
        }

        M_MODEL &whereOrIn(orm::table_col<B_BASE, &xtask_info::col_names> wq, const std::vector<std::string> &a)
        {
            return whereOrIn(wq, orm::wq::in, a);
        }

        M_MODEL &whereOrNotIn(orm::table_col<B_BASE, &xtask_info::col_names> wq, const std::vector<std::string> &a)
        {
            return whereOrIn(wq, orm::wq::notin, a);
        }

        // ===== IN 具名转发结束 =====

        // ===== Null 条件（无 value）=====

        M_MODEL &whereNull(xtask_info::cols field)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 1;
            item.op_type    = orm::wq::isnull;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = xtask_info::col_names[item.col_idx];
            // filed_value 不设
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereNull(orm::table_col<B_BASE, &xtask_info::col_names> wq)
        {
            orm_where_sql_t item;
            item.pre_op  = wheresql.empty() ? 0 : 1;
            item.op_type = orm::wq::isnull;
            item.col_idx = B_BASE::findcolpos(wq);

            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.need_quote = (item.col_idx != 255) ? B_BASE::col_need_quote[item.col_idx] : true;
            item.filed_name = wq;
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereOrNull(xtask_info::cols field)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 2;
            item.op_type    = orm::wq::isnull;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = xtask_info::col_names[item.col_idx];
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereOrNull(orm::table_col<B_BASE, &xtask_info::col_names> wq)
        {
            orm_where_sql_t item;
            item.pre_op  = wheresql.empty() ? 0 : 2;
            item.op_type = orm::wq::isnull;
            item.col_idx = B_BASE::findcolpos(wq);

            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.need_quote = (item.col_idx != 255) ? B_BASE::col_need_quote[item.col_idx] : true;
            item.filed_name = wq;
            wheresql.push_back(std::move(item));
            return *mod;
        }

        // ===== Not Null 条件（无 value）=====

        M_MODEL &whereNotNull(xtask_info::cols field)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 1;
            item.op_type    = orm::wq::notnull;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = xtask_info::col_names[item.col_idx];
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereNotNull(orm::table_col<B_BASE, &xtask_info::col_names> wq)
        {
            orm_where_sql_t item;
            item.pre_op  = wheresql.empty() ? 0 : 1;
            item.op_type = orm::wq::notnull;
            item.col_idx = B_BASE::findcolpos(wq);

            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.need_quote = (item.col_idx != 255) ? B_BASE::col_need_quote[item.col_idx] : true;
            item.filed_name = wq;
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereOrNotNull(xtask_info::cols field)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 2;
            item.op_type    = orm::wq::notnull;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = xtask_info::col_names[item.col_idx];
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereOrNotNull(orm::table_col<B_BASE, &xtask_info::col_names> wq)
        {
            orm_where_sql_t item;
            item.pre_op  = wheresql.empty() ? 0 : 2;
            item.op_type = orm::wq::notnull;
            item.col_idx = B_BASE::findcolpos(wq);

            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.need_quote = (item.col_idx != 255) ? B_BASE::col_need_quote[item.col_idx] : true;
            item.filed_name = wq;
            wheresql.push_back(std::move(item));
            return *mod;
        }

        template <typename T2>
        M_MODEL &whereEQ(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::eq, std::forward<T2>(value));
        }

        M_MODEL &whereEQ(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::eq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereEQ(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::eq, val);
        }

        template <typename T2>
        M_MODEL &whereAnd(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::eq, std::forward<T2>(value));
        }

        M_MODEL &whereAnd(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::eq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereAnd(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::eq, val);
        }

        template <typename T2>
        M_MODEL &whereBT(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::bt, std::forward<T2>(value));
        }

        M_MODEL &whereBT(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::bt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereBT(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::bt, val);
        }

        template <typename T2>
        M_MODEL &whereGT(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::bt, std::forward<T2>(value));
        }

        M_MODEL &whereGT(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::bt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereGT(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::bt, val);
        }

        template <typename T2>
        M_MODEL &whereBE(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::be, std::forward<T2>(value));
        }

        M_MODEL &whereBE(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::be, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereBE(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::be, val);
        }

        template <typename T2>
        M_MODEL &whereGE(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::be, std::forward<T2>(value));
        }

        M_MODEL &whereGE(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::be, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereGE(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::be, val);
        }

        template <typename T2>
        M_MODEL &whereLT(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::lt, std::forward<T2>(value));
        }

        M_MODEL &whereLT(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::lt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLT(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::lt, val);
        }

        template <typename T2>
        M_MODEL &whereLE(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::le, std::forward<T2>(value));
        }

        M_MODEL &whereLE(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::le, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLE(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::le, val);
        }

        template <typename T2>
        M_MODEL &whereNQ(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::nq, std::forward<T2>(value));
        }

        M_MODEL &whereNQ(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::nq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereNQ(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::nq, val);
        }

        template <typename T2>
        M_MODEL &whereNE(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::nq, std::forward<T2>(value));
        }

        M_MODEL &whereNE(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::nq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereNE(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::nq, val);
        }

        template <typename T2>
        M_MODEL &whereLike(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::like, std::forward<T2>(value));
        }

        M_MODEL &whereLike(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::like, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLike(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::like, val);
        }

        template <typename T2>
        M_MODEL &whereLikeLeft(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::llike, std::forward<T2>(value));
        }

        M_MODEL &whereLikeLeft(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::llike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLikeLeft(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::llike, val);
        }

        template <typename T2>
        M_MODEL &whereLikeRight(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::rlike, std::forward<T2>(value));
        }

        M_MODEL &whereLikeRight(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::rlike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLikeRight(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::rlike, val);
        }

        template <typename T2>
        M_MODEL &whereNotLike(xtask_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::nlike, std::forward<T2>(value));
        }

        M_MODEL &whereNotLike(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::nlike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereNotLike(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::nlike, val);
        }

        template <typename T2>
        M_MODEL &whereOrBT(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::bt, std::forward<T2>(value));
        }

        M_MODEL &whereOrBT(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::bt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrBT(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::bt, val);
        }

        template <typename T2>
        M_MODEL &whereOrBE(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::be, std::forward<T2>(value));
        }

        M_MODEL &whereOrBE(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::be, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrBE(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::be, val);
        }

        template <typename T2>
        M_MODEL &whereOrLT(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::lt, std::forward<T2>(value));
        }

        M_MODEL &whereOrLT(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::lt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLT(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::lt, val);
        }

        template <typename T2>
        M_MODEL &whereOrLE(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::le, std::forward<T2>(value));
        }

        M_MODEL &whereOrLE(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::le, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLE(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::le, val);
        }

        template <typename T2>
        M_MODEL &whereOrNQ(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::nq, std::forward<T2>(value));
        }

        M_MODEL &whereOrNQ(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::nq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrNQ(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::nq, val);
        }

        template <typename T2>
        M_MODEL &whereOrLike(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::like, std::forward<T2>(value));
        }

        M_MODEL &whereOrLike(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::like, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLike(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::like, val);
        }

        template <typename T2>
        M_MODEL &whereOrLikeLeft(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::llike, std::forward<T2>(value));
        }

        M_MODEL &whereOrLikeLeft(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::llike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLikeLeft(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::llike, val);
        }

        template <typename T2>
        M_MODEL &whereOrLikeRight(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::rlike, std::forward<T2>(value));
        }

        M_MODEL &whereOrLikeRight(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::rlike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLikeRight(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::rlike, val);
        }

        template <typename T2>
        M_MODEL &whereOrNotLike(xtask_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::nlike, std::forward<T2>(value));
        }

        M_MODEL &whereOrNotLike(orm::table_col<B_BASE, &xtask_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::nlike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrNotLike(orm::table_col<B_BASE, &xtask_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::nlike, val);
        }

        void escape_text_value(std::string &out, const http::obj_val &v, bool need_quote)
        {
            if (!need_quote)
            {
                out.append(v.to_string());
                return;
            }
            std::string s = v.to_string();
            size_t p      = 0;
            while ((p = s.find('\'', p)) != std::string::npos)
            {
                s.insert(p, "\\'");
                p += 2;
            }
            out.append("'");

            out.append(s);
            out.append("'");
        }

        // LIKE 通配符只进值，不进 SQL 结构：
        // 文本路径拼进转义后的字面量，预编译路径拼进绑定参数
        static http::obj_val wrap_like_value(const http::obj_val &v, bool left, bool right)
        {
            std::string s = v.to_string();
            if (left)
            {
                s.insert(s.begin(), '%');
            }
            if (right)
            {
                s.push_back('%');
            }
            return http::obj_val(std::move(s));
        }

        void append_like_text(std::string &out, const http::obj_val &v, bool left, bool right)
        {
            // LIKE 模式恒为字符串字面量，不按列类型走 need_quote
            escape_text_value(out, wrap_like_value(v, left, right), true);
        }

        static http::obj_val prepared_like_bind(const orm_where_sql_t &item)
        {
            switch (item.op_type)
            {
            case orm::wq::like: return wrap_like_value(item.filed_value, true, true);
            case orm::wq::llike: return wrap_like_value(item.filed_value, true, false);
            case orm::wq::rlike: return wrap_like_value(item.filed_value, false, true);
            case orm::wq::nlike: return wrap_like_value(item.filed_value, true, true);
            default: return item.filed_value;
            }
        }

        void build_text_where(std::string &where_clause)
        {
            const bool need_table_prefix = (join_ptr != nullptr);
            auto append_field            = [&](const std::string &fname)
            {
                if (need_table_prefix && fname.find('.') == std::string::npos)
                {
                    where_clause.append(B_BASE::tablename);
                    where_clause.append(".");
                }
                where_clause.append(escape_mysql_col(fname));
            };

            for (size_t i = 0; i < wheresql.size(); ++i)
            {
                auto &item = wheresql[i];
                if (item.end_sub)
                {
                    where_clause.push_back(')');
                    continue;
                }
                // 连接词：仅当前面已有内容且不紧邻左括号（括号内首项不加 AND/OR）
                if (!where_clause.empty() && where_clause.back() != '(')
                {
                    if (item.pre_op == 2)
                        where_clause.append(" OR ");
                    else if (item.pre_op == 1)
                        where_clause.append(" AND ");
                }
                if (item.begin_sub)
                {
                    where_clause.push_back('(');
                    if (item.filed_name.empty())
                        continue;
                }

                switch (item.op_type)
                {
                case orm::wq::eq:
                    append_field(item.filed_name);
                    where_clause.append(" = ");
                    escape_text_value(where_clause, item.filed_value, item.need_quote);
                    break;
                case orm::wq::nq:
                    append_field(item.filed_name);
                    where_clause.append(" != ");
                    escape_text_value(where_clause, item.filed_value, item.need_quote);
                    break;
                case orm::wq::lt:
                    append_field(item.filed_name);
                    where_clause.append(" < ");
                    escape_text_value(where_clause, item.filed_value, item.need_quote);
                    break;
                case orm::wq::le:
                    append_field(item.filed_name);
                    where_clause.append(" <= ");
                    escape_text_value(where_clause, item.filed_value, item.need_quote);
                    break;
                case orm::wq::bt:
                    append_field(item.filed_name);
                    where_clause.append(" > ");
                    escape_text_value(where_clause, item.filed_value, item.need_quote);
                    break;
                case orm::wq::be:
                    append_field(item.filed_name);
                    where_clause.append(" >= ");
                    escape_text_value(where_clause, item.filed_value, item.need_quote);
                    break;
                case orm::wq::like:
                    append_field(item.filed_name);
                    where_clause.append(" LIKE ");
                    append_like_text(where_clause, item.filed_value, true, true);
                    break;
                case orm::wq::llike:
                    append_field(item.filed_name);
                    where_clause.append(" LIKE ");
                    append_like_text(where_clause, item.filed_value, true, false);
                    break;
                case orm::wq::rlike:
                    append_field(item.filed_name);
                    where_clause.append(" LIKE ");
                    append_like_text(where_clause, item.filed_value, false, true);
                    break;
                case orm::wq::nlike:
                    append_field(item.filed_name);
                    where_clause.append(" NOT LIKE ");
                    append_like_text(where_clause, item.filed_value, true, true);
                    break;
                case orm::wq::in:
                case orm::wq::notin:
                    append_field(item.filed_name);
                    where_clause.append(item.op_type == orm::wq::in ? " IN (" : " NOT IN (");
                    if (item.filed_value.is_array())
                    {
                        for (size_t a = 0; a < item.filed_value.size(); ++a)
                        {
                            if (a > 0)
                                where_clause.append(", ");
                            escape_text_value(where_clause, item.filed_value[a], item.need_quote);
                        }
                    }
                    else
                    {
                        escape_text_value(where_clause, item.filed_value, item.need_quote);
                    }
                    where_clause.append(")");
                    break;
                case orm::wq::isnull:
                    append_field(item.filed_name);
                    where_clause.append(" IS NULL");
                    break;
                case orm::wq::notnull:
                    append_field(item.filed_name);
                    where_clause.append(" IS NOT NULL");
                    break;
                default:
                    break;
                }
            }
        }

        M_MODEL &order(xtask_info::cols field, const std::string &asc_or_desc)
        {
            ordersql.append(" ORDER BY ");
            switch (field)
            {
            
			case xtask_info::cols::xtaskid:
				ordersql.append("xtaskid");
				break;
			case xtask_info::cols::userid:
				ordersql.append("userid");
				break;
			case xtask_info::cols::xpjid:
				ordersql.append("xpjid");
				break;
			case xtask_info::cols::adminid:
				ordersql.append("adminid");
				break;
			case xtask_info::cols::parentid:
				ordersql.append("parentid");
				break;
			case xtask_info::cols::begindate:
				ordersql.append("begindate");
				break;
			case xtask_info::cols::enddate:
				ordersql.append("enddate");
				break;
			case xtask_info::cols::expectbegindate:
				ordersql.append("expectbegindate");
				break;
			case xtask_info::cols::expectenddate:
				ordersql.append("expectenddate");
				break;
			case xtask_info::cols::milestone:
				ordersql.append("milestone");
				break;
			case xtask_info::cols::subxpjid:
				ordersql.append("subxpjid");
				break;
			case xtask_info::cols::depxtaskid:
				ordersql.append("depxtaskid");
				break;
			case xtask_info::cols::referdocid:
				ordersql.append("referdocid");
				break;
			case xtask_info::cols::isfinish:
				ordersql.append("isfinish");
				break;
			case xtask_info::cols::updatedate:
				ordersql.append("updatedate");
				break;
			case xtask_info::cols::finishdate:
				ordersql.append("finishdate");
				break;
			case xtask_info::cols::iscore:
				ordersql.append("iscore");
				break;
			case xtask_info::cols::xvalue:
				ordersql.append("xvalue");
				break;
			case xtask_info::cols::expectday:
				ordersql.append("expectday");
				break;
			case xtask_info::cols::realday:
				ordersql.append("realday");
				break;
			case xtask_info::cols::pricevalue:
				ordersql.append("pricevalue");
				break;
			case xtask_info::cols::title:
				ordersql.append("title");
				break;
			case xtask_info::cols::introduce:
				ordersql.append("introduce");
				break;
			case xtask_info::cols::xlogo:
				ordersql.append("xlogo");
				break;
			case xtask_info::cols::xcolor:
				ordersql.append("xcolor");
				break;
			case xtask_info::cols::pullurl:
				ordersql.append("pullurl");
				break;
			case xtask_info::cols::pulltitle:
				ordersql.append("pulltitle");
				break;
			case xtask_info::cols::pullauthor:
				ordersql.append("pullauthor");
				break;
			case xtask_info::cols::note:
				ordersql.append("note");
				break;
			case xtask_info::cols::itemnote:
				ordersql.append("itemnote");
				break;
            default:
                return *mod;
                break;
            }
            ordersql.append(asc_or_desc);
            return *mod;
        }

        M_MODEL &asc(xtask_info::cols field)
        {
            ordersql.append(" ORDER BY ");
            switch (field)
            {
            
			case xtask_info::cols::xtaskid:
				ordersql.append("xtaskid");
				break;
			case xtask_info::cols::userid:
				ordersql.append("userid");
				break;
			case xtask_info::cols::xpjid:
				ordersql.append("xpjid");
				break;
			case xtask_info::cols::adminid:
				ordersql.append("adminid");
				break;
			case xtask_info::cols::parentid:
				ordersql.append("parentid");
				break;
			case xtask_info::cols::begindate:
				ordersql.append("begindate");
				break;
			case xtask_info::cols::enddate:
				ordersql.append("enddate");
				break;
			case xtask_info::cols::expectbegindate:
				ordersql.append("expectbegindate");
				break;
			case xtask_info::cols::expectenddate:
				ordersql.append("expectenddate");
				break;
			case xtask_info::cols::milestone:
				ordersql.append("milestone");
				break;
			case xtask_info::cols::subxpjid:
				ordersql.append("subxpjid");
				break;
			case xtask_info::cols::depxtaskid:
				ordersql.append("depxtaskid");
				break;
			case xtask_info::cols::referdocid:
				ordersql.append("referdocid");
				break;
			case xtask_info::cols::isfinish:
				ordersql.append("isfinish");
				break;
			case xtask_info::cols::updatedate:
				ordersql.append("updatedate");
				break;
			case xtask_info::cols::finishdate:
				ordersql.append("finishdate");
				break;
			case xtask_info::cols::iscore:
				ordersql.append("iscore");
				break;
			case xtask_info::cols::xvalue:
				ordersql.append("xvalue");
				break;
			case xtask_info::cols::expectday:
				ordersql.append("expectday");
				break;
			case xtask_info::cols::realday:
				ordersql.append("realday");
				break;
			case xtask_info::cols::pricevalue:
				ordersql.append("pricevalue");
				break;
			case xtask_info::cols::title:
				ordersql.append("title");
				break;
			case xtask_info::cols::introduce:
				ordersql.append("introduce");
				break;
			case xtask_info::cols::xlogo:
				ordersql.append("xlogo");
				break;
			case xtask_info::cols::xcolor:
				ordersql.append("xcolor");
				break;
			case xtask_info::cols::pullurl:
				ordersql.append("pullurl");
				break;
			case xtask_info::cols::pulltitle:
				ordersql.append("pulltitle");
				break;
			case xtask_info::cols::pullauthor:
				ordersql.append("pullauthor");
				break;
			case xtask_info::cols::note:
				ordersql.append("note");
				break;
			case xtask_info::cols::itemnote:
				ordersql.append("itemnote");
				break;
            default:
                return *mod;
                break;
            }
            ordersql.append(" ASC ");
            return *mod;
        }

        M_MODEL &asc()
        {
            ordersql.append(" ORDER BY ");
            ordersql.append(B_BASE::getPKname());
            ordersql.append(" ASC ");
            return *mod;
        }
        M_MODEL &desc()
        {
            ordersql.append(" ORDER BY ");
            ordersql.append(B_BASE::getPKname());
            ordersql.append(" DESC ");
            return *mod;
        }
        M_MODEL &desc(xtask_info::cols field)
        {
            ordersql.append(" ORDER BY ");
            switch (field)
            {
            
			case xtask_info::cols::xtaskid:
				ordersql.append("xtaskid");
				break;
			case xtask_info::cols::userid:
				ordersql.append("userid");
				break;
			case xtask_info::cols::xpjid:
				ordersql.append("xpjid");
				break;
			case xtask_info::cols::adminid:
				ordersql.append("adminid");
				break;
			case xtask_info::cols::parentid:
				ordersql.append("parentid");
				break;
			case xtask_info::cols::begindate:
				ordersql.append("begindate");
				break;
			case xtask_info::cols::enddate:
				ordersql.append("enddate");
				break;
			case xtask_info::cols::expectbegindate:
				ordersql.append("expectbegindate");
				break;
			case xtask_info::cols::expectenddate:
				ordersql.append("expectenddate");
				break;
			case xtask_info::cols::milestone:
				ordersql.append("milestone");
				break;
			case xtask_info::cols::subxpjid:
				ordersql.append("subxpjid");
				break;
			case xtask_info::cols::depxtaskid:
				ordersql.append("depxtaskid");
				break;
			case xtask_info::cols::referdocid:
				ordersql.append("referdocid");
				break;
			case xtask_info::cols::isfinish:
				ordersql.append("isfinish");
				break;
			case xtask_info::cols::updatedate:
				ordersql.append("updatedate");
				break;
			case xtask_info::cols::finishdate:
				ordersql.append("finishdate");
				break;
			case xtask_info::cols::iscore:
				ordersql.append("iscore");
				break;
			case xtask_info::cols::xvalue:
				ordersql.append("xvalue");
				break;
			case xtask_info::cols::expectday:
				ordersql.append("expectday");
				break;
			case xtask_info::cols::realday:
				ordersql.append("realday");
				break;
			case xtask_info::cols::pricevalue:
				ordersql.append("pricevalue");
				break;
			case xtask_info::cols::title:
				ordersql.append("title");
				break;
			case xtask_info::cols::introduce:
				ordersql.append("introduce");
				break;
			case xtask_info::cols::xlogo:
				ordersql.append("xlogo");
				break;
			case xtask_info::cols::xcolor:
				ordersql.append("xcolor");
				break;
			case xtask_info::cols::pullurl:
				ordersql.append("pullurl");
				break;
			case xtask_info::cols::pulltitle:
				ordersql.append("pulltitle");
				break;
			case xtask_info::cols::pullauthor:
				ordersql.append("pullauthor");
				break;
			case xtask_info::cols::note:
				ordersql.append("note");
				break;
			case xtask_info::cols::itemnote:
				ordersql.append("itemnote");
				break;
            default:
                return *mod;
                break;
            }
            ordersql.append(" DESC ");
            return *mod;
        }

        M_MODEL &order(orm::table_col<B_BASE, &xtask_info::col_names> wq, const std::string &asc_or_desc)
        {
            ordersql.append(" ORDER BY ");
            ordersql.append(wq);
            ordersql.append(" ");
            ordersql.append(asc_or_desc);
            return *mod;
        }
        M_MODEL &asc(orm::table_col<B_BASE, &xtask_info::col_names> wq)
        {

            ordersql.append(" ORDER BY ");
            ordersql.append(wq);
            ordersql.append(" ASC ");
            return *mod;
        }

        M_MODEL &desc(orm::table_col<B_BASE, &xtask_info::col_names> wq)
        {

            ordersql.append(" ORDER BY ");
            ordersql.append(wq);
            ordersql.append(" DESC ");
            return *mod;
        }

        M_MODEL &having(orm::table_col<B_BASE, &xtask_info::col_names> wq)
        {
            groupsql.append(" HAVING ");
            groupsql.append(wq);
            return *mod;
        }

        M_MODEL &having(xtask_info::cols field)
        {
            groupsql.append(" HAVING ");
            switch (field)
            {
            
			case xtask_info::cols::xtaskid:
				groupsql.append("xtaskid");
				break;
			case xtask_info::cols::userid:
				groupsql.append("userid");
				break;
			case xtask_info::cols::xpjid:
				groupsql.append("xpjid");
				break;
			case xtask_info::cols::adminid:
				groupsql.append("adminid");
				break;
			case xtask_info::cols::parentid:
				groupsql.append("parentid");
				break;
			case xtask_info::cols::begindate:
				groupsql.append("begindate");
				break;
			case xtask_info::cols::enddate:
				groupsql.append("enddate");
				break;
			case xtask_info::cols::expectbegindate:
				groupsql.append("expectbegindate");
				break;
			case xtask_info::cols::expectenddate:
				groupsql.append("expectenddate");
				break;
			case xtask_info::cols::milestone:
				groupsql.append("milestone");
				break;
			case xtask_info::cols::subxpjid:
				groupsql.append("subxpjid");
				break;
			case xtask_info::cols::depxtaskid:
				groupsql.append("depxtaskid");
				break;
			case xtask_info::cols::referdocid:
				groupsql.append("referdocid");
				break;
			case xtask_info::cols::isfinish:
				groupsql.append("isfinish");
				break;
			case xtask_info::cols::updatedate:
				groupsql.append("updatedate");
				break;
			case xtask_info::cols::finishdate:
				groupsql.append("finishdate");
				break;
			case xtask_info::cols::iscore:
				groupsql.append("iscore");
				break;
			case xtask_info::cols::xvalue:
				groupsql.append("xvalue");
				break;
			case xtask_info::cols::expectday:
				groupsql.append("expectday");
				break;
			case xtask_info::cols::realday:
				groupsql.append("realday");
				break;
			case xtask_info::cols::pricevalue:
				groupsql.append("pricevalue");
				break;
			case xtask_info::cols::title:
				groupsql.append("title");
				break;
			case xtask_info::cols::introduce:
				groupsql.append("introduce");
				break;
			case xtask_info::cols::xlogo:
				groupsql.append("xlogo");
				break;
			case xtask_info::cols::xcolor:
				groupsql.append("xcolor");
				break;
			case xtask_info::cols::pullurl:
				groupsql.append("pullurl");
				break;
			case xtask_info::cols::pulltitle:
				groupsql.append("pulltitle");
				break;
			case xtask_info::cols::pullauthor:
				groupsql.append("pullauthor");
				break;
			case xtask_info::cols::note:
				groupsql.append("note");
				break;
			case xtask_info::cols::itemnote:
				groupsql.append("itemnote");
				break;
            default:
                return *mod;
                break;
            }
            return *mod;
        }

        M_MODEL &group(xtask_info::cols field)
        {
            groupsql.append(" GROUP BY ");
            switch (field)
            {
            
			case xtask_info::cols::xtaskid:
				groupsql.append("xtaskid");
				break;
			case xtask_info::cols::userid:
				groupsql.append("userid");
				break;
			case xtask_info::cols::xpjid:
				groupsql.append("xpjid");
				break;
			case xtask_info::cols::adminid:
				groupsql.append("adminid");
				break;
			case xtask_info::cols::parentid:
				groupsql.append("parentid");
				break;
			case xtask_info::cols::begindate:
				groupsql.append("begindate");
				break;
			case xtask_info::cols::enddate:
				groupsql.append("enddate");
				break;
			case xtask_info::cols::expectbegindate:
				groupsql.append("expectbegindate");
				break;
			case xtask_info::cols::expectenddate:
				groupsql.append("expectenddate");
				break;
			case xtask_info::cols::milestone:
				groupsql.append("milestone");
				break;
			case xtask_info::cols::subxpjid:
				groupsql.append("subxpjid");
				break;
			case xtask_info::cols::depxtaskid:
				groupsql.append("depxtaskid");
				break;
			case xtask_info::cols::referdocid:
				groupsql.append("referdocid");
				break;
			case xtask_info::cols::isfinish:
				groupsql.append("isfinish");
				break;
			case xtask_info::cols::updatedate:
				groupsql.append("updatedate");
				break;
			case xtask_info::cols::finishdate:
				groupsql.append("finishdate");
				break;
			case xtask_info::cols::iscore:
				groupsql.append("iscore");
				break;
			case xtask_info::cols::xvalue:
				groupsql.append("xvalue");
				break;
			case xtask_info::cols::expectday:
				groupsql.append("expectday");
				break;
			case xtask_info::cols::realday:
				groupsql.append("realday");
				break;
			case xtask_info::cols::pricevalue:
				groupsql.append("pricevalue");
				break;
			case xtask_info::cols::title:
				groupsql.append("title");
				break;
			case xtask_info::cols::introduce:
				groupsql.append("introduce");
				break;
			case xtask_info::cols::xlogo:
				groupsql.append("xlogo");
				break;
			case xtask_info::cols::xcolor:
				groupsql.append("xcolor");
				break;
			case xtask_info::cols::pullurl:
				groupsql.append("pullurl");
				break;
			case xtask_info::cols::pulltitle:
				groupsql.append("pulltitle");
				break;
			case xtask_info::cols::pullauthor:
				groupsql.append("pullauthor");
				break;
			case xtask_info::cols::note:
				groupsql.append("note");
				break;
			case xtask_info::cols::itemnote:
				groupsql.append("itemnote");
				break;
            default:
                return *mod;
                break;
            }
            return *mod;
        }
        M_MODEL &group(orm::table_col<B_BASE, &xtask_info::col_names> wq)
        {
            groupsql.append(" GROUP BY ");
            groupsql.append(wq);
            return *mod;
        }

        M_MODEL &limit(unsigned int num)
        {
            limitsql.clear();
            limitsql.append(" limit ");
            limitsql.append(std::to_string(num));
            return *mod;
        }
        M_MODEL &limit(unsigned int num, unsigned int endnum)
        {
            limitsql.clear();
            limitsql.append(" limit ");
            limitsql.append(std::to_string(num));
            limitsql.push_back(',');
            limitsql.append(std::to_string(endnum));
            return *mod;
        }

        std::vector<std::map<std::string, std::string>> fetch_obj()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            std::vector<std::map<std::string, std::string>> temprecord;

            if (iserror)
            {
                return temprecord;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    return temprecord;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, &temprecord](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           std::map<std::string, std::string> data_temp;
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                               {
                                                                                   data_temp.insert({col_names[ij] ? col_names[ij] : "", ""});
                                                                                   continue;
                                                                               }
                                                                               std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                               std::string value(reinterpret_cast<char *>(ptr), len);
                                                                               data_temp.insert({col_name, std::move(value)});
                                                                           }
                                                                           temprecord.emplace_back(std::move(data_temp));
                                                                           effect_num++;
                                                                           return true;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return temprecord;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            return temprecord;
        }
        std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>
        fetch_row()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            std::vector<std::vector<std::string>> temprecord;
            std::vector<std::string> table_fieldname;
            std::map<std::string, unsigned int> table_fieldmap;

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);

                model_meta_cache<std::vector<std::vector<std::string>>> &temp_cache =
                    model_meta_cache<std::vector<std::vector<std::string>>>::getinstance();
                model_meta_cache<std::vector<std::string>> &table_cache =
                    model_meta_cache<std::vector<std::string>>::getinstance();
                model_meta_cache<std::map<std::string, unsigned int>> &tablemap_cache =
                    model_meta_cache<std::map<std::string, unsigned int>>::getinstance();

                bool iscache_hit = false;
                try
                {
                    std::vector<std::vector<std::string>> cache_rows  = temp_cache.get(sqlhashid);
                    std::vector<std::string> cache_fieldname           = table_cache.get(sqlhashid);
                    std::map<std::string, unsigned int> cache_fieldmap = tablemap_cache.get(sqlhashid);

                    temprecord      = std::move(cache_rows);
                    table_fieldname = std::move(cache_fieldname);
                    table_fieldmap  = std::move(cache_fieldmap);
                    iscache_hit     = true;
                }
                catch (const std::exception &)
                {
                    iscache_hit = false;
                }
                if (iscache_hit && temprecord.size() > 0)
                {
                    iscache = false;
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
            }

            if (iserror)
            {
                return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }

            try
            {
                if (conn_empty())
                {
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, &temprecord, &table_fieldname, &table_fieldmap](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           if (table_fieldname.empty())
                                                                           {
                                                                               for (int ii = 0; ii < col_count; ii++)
                                                                               {
                                                                                   std::string col_name = col_names[ii] ? col_names[ii] : "";
                                                                                   table_fieldmap.emplace(col_name, table_fieldname.size());
                                                                                   table_fieldname.push_back(col_name);
                                                                               }
                                                                           }
                                                                           std::vector<std::string> temp_v_record;
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                               {
                                                                                   temp_v_record.push_back("");
                                                                               }
                                                                               else
                                                                               {
                                                                                   temp_v_record.emplace_back(reinterpret_cast<char *>(ptr), len);
                                                                               }
                                                                           }
                                                                           temprecord.push_back(std::move(temp_v_record));
                                                                           effect_num++;
                                                                           return true;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        if (temprecord.size() > 0)
                        {
                            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);

                            model_meta_cache<std::vector<std::vector<std::string>>> &temp_cache =
                                model_meta_cache<std::vector<std::vector<std::string>>>::getinstance();
                            temp_cache.save(sqlhashid, temprecord, exptime);

                            exptime += 1;
                            model_meta_cache<std::vector<std::string>> &table_cache = model_meta_cache<std::vector<std::string>>::getinstance();
                            table_cache.save(sqlhashid, table_fieldname, exptime);

                            model_meta_cache<std::map<std::string, unsigned int>> &tablemap_cache =
                                model_meta_cache<std::map<std::string, unsigned int>>::getinstance();
                            tablemap_cache.save(sqlhashid, table_fieldmap, exptime);
                            exptime = 0;
                            iscache = false;
                        }
                    }
                }

                return std::make_tuple(std::move(table_fieldname), std::move(table_fieldmap), std::move(temprecord));
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }

        template <typename T, RecordLineCallback<T> Callback>
        unsigned int fetch_to(std::vector<T> &custom_record, Callback &&callback)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            parse_leftjion();
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }
            sqlstring.append(B_BASE::tablename);
            get_join_table();

            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }

            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           T data_temp;
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                                   continue;
                                                                               std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                               if (!col_name.empty())
                                                                               {
                                                                                   std::invoke(std::forward<Callback>(callback), data_temp, col_name, ptr, len, 0, 1);
                                                                               }
                                                                           }
                                                                           custom_record.emplace_back(std::move(data_temp));
                                                                           effect_num++;
                                                                           return true;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        template <typename T, RecordLineCallback<T> Callback>
        asio::awaitable<unsigned int> async_fetch_to(std::vector<T> &custom_record, Callback &&callback)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            parse_leftjion();
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }
            sqlstring.append(B_BASE::tablename);
            get_join_table();
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) -> bool
                                                                                      {
                                                                                          T data_temp;
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                                  continue;
                                                                                              std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                                              if (!col_name.empty())
                                                                                              {
                                                                                                  std::invoke(std::forward<Callback>(callback), data_temp, col_name, ptr, len, 0, 1);
                                                                                              }
                                                                                          }
                                                                                          custom_record.emplace_back(std::move(data_temp));
                                                                                          effect_num++;
                                                                                          return true;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        template <ResultHasSetVal T>
        unsigned int fetch_to(std::vector<T> &custom_record)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            parse_leftjion();
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }
            sqlstring.append(B_BASE::tablename);
            get_join_table();
            sqlstring.append(" WHERE ");
            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, &custom_record](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           T data_temp;
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                                   continue;
                                                                               std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                               if (!col_name.empty())
                                                                               {
                                                                                   data_temp.set_val(col_name, ptr, len, 0);
                                                                               }
                                                                           }
                                                                           custom_record.emplace_back(std::move(data_temp));
                                                                           effect_num++;
                                                                           return true;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_fetch_to(std::vector<T> &custom_record)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            parse_leftjion();
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }
            sqlstring.append(B_BASE::tablename);
            get_join_table();
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, &custom_record](int col_count, char **col_names, auto get_data) -> bool
                                                                                      {
                                                                                          T data_temp;
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                                  continue;
                                                                                              std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                                              if (!col_name.empty())
                                                                                              {
                                                                                                  data_temp.set_val(col_name, ptr, len, 0);
                                                                                              }
                                                                                          }
                                                                                          custom_record.emplace_back(std::move(data_temp));
                                                                                          effect_num++;
                                                                                          return true;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        unsigned int fetch()
        {
            std::string where_clause;
            build_text_where(where_clause);

            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_record_cache(sqlhashid))
                {
                    iscache = false;
                    return 0;
                }
            }

            B_BASE::record_reset();
            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, col_pos_map = std::vector<int>{}, first_row = true](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                       {
                                                                           if (first_row)
                                                                           {
                                                                               col_pos_map.assign(col_count, 255);
                                                                               for (int ij = 0; ij < col_count; ij++)
                                                                               {
                                                                                   if (col_names[ij] && col_names[ij][0] != '\0')
                                                                                   {
                                                                                       col_pos_map[ij] = B_BASE::findcolpos(col_names[ij]);
                                                                                   }
                                                                               }
                                                                               first_row = false;
                                                                           }
                                                                           xtask_info::meta data_temp;
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                               {
                                                                                   static const unsigned char null_value = 0;
                                                                                   assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, data_temp);
                                                                                   continue;
                                                                               }
                                                                               assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, data_temp);
                                                                           }
                                                                           B_BASE::record.emplace_back(std::move(data_temp));
                                                                           effect_num++;
                                                                           return true;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        asio::awaitable<unsigned int> async_fetch()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_record_cache(sqlhashid))
                {
                    iscache = false;
                    co_return 0;
                }
            }

            B_BASE::record_reset();
            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, col_pos_map = std::vector<int>{}, first_row = true](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                      {
                                                                                          if (first_row)
                                                                                          {
                                                                                              col_pos_map.assign(col_count, 255);
                                                                                              for (int ij = 0; ij < col_count; ij++)
                                                                                              {
                                                                                                  if (col_names[ij] && col_names[ij][0] != '\0')
                                                                                                  {
                                                                                                      col_pos_map[ij] = B_BASE::findcolpos(col_names[ij]);
                                                                                                  }
                                                                                              }
                                                                                              first_row = false;
                                                                                          }
                                                                                          xtask_info::meta data_temp;
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                              {
                                                                                                  static const unsigned char null_value = 0;
                                                                                                  assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, data_temp);
                                                                                                  continue;
                                                                                              }
                                                                                              assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, data_temp);
                                                                                          }
                                                                                          B_BASE::record.emplace_back(std::move(data_temp));
                                                                                          effect_num++;
                                                                                          return true;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }
        M_MODEL &fetch_append()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_record_cache(sqlhashid))
                {
                    iscache = false;
                    return *mod;
                }
            }

            if (iserror)
            {
                return *mod;
            }

            try
            {

                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, col_pos_map = std::vector<int>{}, first_row = true](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                       {
                                                                           if (first_row)
                                                                           {
                                                                               col_pos_map.assign(col_count, 255);
                                                                               for (int ij = 0; ij < col_count; ij++)
                                                                               {
                                                                                   if (col_names[ij] && col_names[ij][0] != '\0')
                                                                                   {
                                                                                       col_pos_map[ij] = B_BASE::findcolpos(col_names[ij]);
                                                                                   }
                                                                               }
                                                                               first_row = false;
                                                                           }
                                                                           xtask_info::meta data_temp;
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                               {
                                                                                   static const unsigned char null_value = 0;
                                                                                   assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, data_temp);
                                                                                   continue;
                                                                               }
                                                                               assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, data_temp);
                                                                           }
                                                                           B_BASE::record.emplace_back(std::move(data_temp));
                                                                           effect_num++;
                                                                           return true;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return *mod;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return *mod;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return *mod;
            }

            return *mod;
        }

        asio::awaitable<unsigned int> async_fetch_append()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_record_cache(sqlhashid))
                {
                    iscache = false;
                    co_return 1;
                }
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                effect_num = 0;

                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, col_pos_map = std::vector<int>{}, first_row = true](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                      {
                                                                                          if (first_row)
                                                                                          {
                                                                                              col_pos_map.assign(col_count, 255);
                                                                                              for (int ij = 0; ij < col_count; ij++)
                                                                                              {
                                                                                                  if (col_names[ij] && col_names[ij][0] != '\0')
                                                                                                  {
                                                                                                      col_pos_map[ij] = B_BASE::findcolpos(col_names[ij]);
                                                                                                  }
                                                                                              }
                                                                                              first_row = false;
                                                                                          }
                                                                                          xtask_info::meta data_temp;
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                              {
                                                                                                  static const unsigned char null_value = 0;
                                                                                                  assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, data_temp);
                                                                                                  continue;
                                                                                              }
                                                                                              assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, data_temp);
                                                                                          }
                                                                                          effect_num++;
                                                                                          B_BASE::record.emplace_back(std::move(data_temp));
                                                                                          return true;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        template <typename T, RecordLineCallback<T> Callback>
        unsigned int fetch_one_to(T &custom_record, Callback &&callback)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            parse_leftjion();
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            get_join_table();
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }

            sqlstring.append(" limit 1");

            if (iserror)
            {
                return 0;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                                   continue;
                                                                               std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                               if (!col_name.empty())
                                                                               {
                                                                                   std::invoke(std::forward<Callback>(callback), custom_record, col_name, ptr, len, 0, 1);
                                                                               }
                                                                           }
                                                                           effect_num++;
                                                                           return false;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        template <typename T, RecordLineCallback<T> Callback>
        asio::awaitable<unsigned int> async_fetch_one_to(T &custom_record, Callback &&callback)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            parse_leftjion();
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            get_join_table();
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }

            sqlstring.append(" limit 1");

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                effect_num = 0;

                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) -> bool
                                                                                      {
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                                  continue;
                                                                                              std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                                              if (!col_name.empty())
                                                                                              {
                                                                                                  std::invoke(std::forward<Callback>(callback), custom_record, col_name, ptr, len, 0, 1);
                                                                                              }
                                                                                          }
                                                                                          effect_num++;
                                                                                          return false;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        template <ResultHasSetVal T>
        unsigned int fetch_one_to(T &custom_struct)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            parse_leftjion();
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            get_join_table();
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }

            sqlstring.append(" limit 1");

            if (iserror)
            {
                return 0;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, &custom_struct](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                                   continue;
                                                                               std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                               if (!col_name.empty())
                                                                               {
                                                                                   custom_struct.set_val(col_name, ptr, len, 0);
                                                                               }
                                                                           }
                                                                           effect_num++;
                                                                           return false;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_fetch_one_to(T &custom_struct)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            parse_leftjion();
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            get_join_table();
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }

            sqlstring.append(" limit 1");

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                effect_num = 0;

                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, &custom_struct](int col_count, char **col_names, auto get_data) -> bool
                                                                                      {
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                                  continue;
                                                                                              std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                                              if (!col_name.empty())
                                                                                              {
                                                                                                  custom_struct.set_val(col_name, ptr, len, 0);
                                                                                              }
                                                                                          }
                                                                                          effect_num++;
                                                                                          return false;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        unsigned int fetch_one(bool isappend = false)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }

            sqlstring.append(" limit 1");

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_data_cache(sqlhashid))
                {
                    iscache = false;
                    return 0;
                }
            }

            B_BASE::data_reset();
            if (iserror)
            {
                return 0;
            }

            try
            {
                effect_num = 0;
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, isappend, col_pos_map = std::vector<int>{}, first_row = true](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                       {
                                                                           // 列位映射仅首行构建（同一结果集列序固定），消除每行重建。
                                                                           if (first_row)
                                                                           {
                                                                               col_pos_map.assign(col_count, 255);
                                                                               for (int ii = 0; ii < col_count; ii++)
                                                                               {
                                                                                   if (col_names[ii] && col_names[ii][0] != 0x00)
                                                                                   {
                                                                                       col_pos_map[ii] = B_BASE::findcolpos(col_names[ii]);
                                                                                   }
                                                                               }
                                                                               first_row = false;
                                                                           }
                                                                           if (isappend)
                                                                           {
                                                                               xtask_info::meta data_temp;
                                                                               for (int ij = 0; ij < col_count; ij++)
                                                                               {
                                                                                   auto [ptr, len] = get_data(ij);
                                                                                   if (ptr == nullptr)
                                                                                   {
                                                                                       static const unsigned char null_value = 0;
                                                                                       assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, data_temp);
                                                                                       continue;
                                                                                   }
                                                                                   assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, data_temp);
                                                                               }
                                                                               B_BASE::record.emplace_back(std::move(data_temp));
                                                                           }
                                                                           else
                                                                           {
                                                                               for (int ij = 0; ij < col_count; ij++)
                                                                               {
                                                                                   auto [ptr, len] = get_data(ij);
                                                                                   if (ptr == nullptr)
                                                                                   {
                                                                                       static const unsigned char null_value = 0;
                                                                                       assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, B_BASE::data);
                                                                                       continue;
                                                                                   }
                                                                                   assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, B_BASE::data);
                                                                               }
                                                                           }
                                                                           effect_num++;
                                                                           return false;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_data_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        asio::awaitable<unsigned int> async_fetch_one(bool isappend = false)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }

            sqlstring.append(" limit 1");

            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_data_cache(sqlhashid))
                {
                    iscache = false;
                    co_return 0;
                }
            }

            B_BASE::data_reset();
            if (iserror)
            {
                co_return 0;
            }

            try
            {
                effect_num = 0;

                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, isappend, col_pos_map = std::vector<int>{}, first_row = true](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                      {
                                                                                          // 列位映射仅首行构建（同一结果集列序固定），消除每行重建。
                                                                                          if (first_row)
                                                                                          {
                                                                                              col_pos_map.assign(col_count, 255);
                                                                                              for (int ii = 0; ii < col_count; ii++)
                                                                                              {
                                                                                                  if (col_names[ii] && col_names[ii][0] != 0x00)
                                                                                                  {
                                                                                                      col_pos_map[ii] = B_BASE::findcolpos(col_names[ii]);
                                                                                                  }
                                                                                              }
                                                                                              first_row = false;
                                                                                          }
                                                                                          if (isappend)
                                                                                          {
                                                                                              xtask_info::meta data_temp;
                                                                                              for (int ij = 0; ij < col_count; ij++)
                                                                                              {
                                                                                                  auto [ptr, len] = get_data(ij);
                                                                                                  if (ptr == nullptr)
                                                                                                  {
                                                                                                      static const unsigned char null_value = 0;
                                                                                                      assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, data_temp);
                                                                                                      continue;
                                                                                                  }
                                                                                                  assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, data_temp);
                                                                                              }
                                                                                              B_BASE::record.emplace_back(std::move(data_temp));
                                                                                          }
                                                                                          else
                                                                                          {
                                                                                              for (int ij = 0; ij < col_count; ij++)
                                                                                              {
                                                                                                  auto [ptr, len] = get_data(ij);
                                                                                                  if (ptr == nullptr)
                                                                                                  {
                                                                                                      static const unsigned char null_value = 0;
                                                                                                      assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, B_BASE::data);
                                                                                                      continue;
                                                                                                  }
                                                                                                  assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, B_BASE::data);
                                                                                              }
                                                                                          }
                                                                                          effect_num++;
                                                                                          return false;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_data_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        M_MODEL &use_cache(int cache_time = 0)
        {
            iscache = true;
            exptime = cache_time;
            return *mod;
        }
        bool isuse_cache(bool iscachedate = false)
        {
            if (iscachedate)
            {
                return exptime == 0 && iscache == false;
            }
            return iscache;
        }
        void set_cache_state(bool isrestatus = false) { iscache = isrestatus; }
        void remove_exptime_cache()
        {
            model_meta_cache<xtask_info::meta> &data_cache = model_meta_cache<xtask_info::meta>::getinstance();
            data_cache.remove_exptime();

            model_meta_cache<std::vector<xtask_info::meta>> &record_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            record_cache.remove_exptime();
        }
        void clear_cache()
        {
            model_meta_cache<xtask_info::meta> &data_cache = model_meta_cache<xtask_info::meta>::getinstance();
            data_cache.clear();

            model_meta_cache<std::vector<xtask_info::meta>> &record_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            record_cache.clear();
        }
        bool remove_cache()
        {
            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);

            model_meta_cache<xtask_info::meta> &data_cache = model_meta_cache<xtask_info::meta>::getinstance();
            bool state = data_cache.remove(sqlhashid);

            model_meta_cache<std::vector<xtask_info::meta>> &record_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            return record_cache.remove(sqlhashid) || state;
        }
        bool remove_cache(std::size_t cache_key_name)
        {
            model_meta_cache<xtask_info::meta> &data_cache = model_meta_cache<xtask_info::meta>::getinstance();
            bool state = data_cache.remove(cache_key_name);

            model_meta_cache<std::vector<xtask_info::meta>> &record_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            return record_cache.remove(cache_key_name) || state;
        }
        int check_cache(std::size_t cache_key_name)
        {
            model_meta_cache<xtask_info::meta> &temp_cache = model_meta_cache<xtask_info::meta>::getinstance();
            return temp_cache.check(cache_key_name);
        }

        bool get_data_cache(std::size_t cache_key_name)
        {
            try
            {
                model_meta_cache<xtask_info::meta> &temp_cache = model_meta_cache<xtask_info::meta>::getinstance();
                B_BASE::data                                        = temp_cache.get(cache_key_name);
                return true;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            B_BASE::data_reset();
            return false;
        }
        int update_cache(int exp_time = 0)
        {
            model_meta_cache<std::vector<xtask_info::meta>> &temp_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            return temp_cache.update(sqlhashid, exp_time);
        }
        int update_cache(std::size_t cache_key_name, int exp_time)
        {
            model_meta_cache<std::vector<xtask_info::meta>> &temp_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            return temp_cache.update(cache_key_name, exp_time);
        }
        bool save_cache(int exp_time = 0)
        {
            model_meta_cache<std::vector<xtask_info::meta>> &temp_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::record, exp_time);
            return true;
        }

        bool save_data_cache(int exp_time = 0)
        {
            model_meta_cache<xtask_info::meta> &temp_cache = model_meta_cache<xtask_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::data, exp_time);
            return true;
        }

        bool save_data_cache(const std::string &cache_key_name, const xtask_info::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<xtask_info::meta> &temp_cache = model_meta_cache<xtask_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }

        bool save_cache(std::size_t cache_key_name, const std::vector<xtask_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<xtask_info::meta>> &temp_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            temp_cache.save(cache_key_name, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const std::vector<xtask_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<xtask_info::meta>> &temp_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_vector_cache(const std::string cache_key_name, const std::vector<xtask_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<xtask_info::meta>> &temp_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const xtask_info::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<xtask_info::meta> &temp_cache = model_meta_cache<xtask_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        const xtask_info::meta get_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<xtask_info::meta> &temp_cache = model_meta_cache<xtask_info::meta>::getinstance();
                std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
                return temp_cache.get(sqlhashid);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            throw std::runtime_error("Not in cache");
        }

        const std::vector<xtask_info::meta> get_vector_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<std::vector<xtask_info::meta>> &temp_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
                std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
                return temp_cache.get(sqlhashid);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            throw std::runtime_error("Not in cache");
        }

        bool get_record_cache(std::size_t cache_key_name)
        {
            try
            {
                model_meta_cache<std::vector<xtask_info::meta>> &temp_cache = model_meta_cache<std::vector<xtask_info::meta>>::getinstance();
                B_BASE::record                                                   = temp_cache.get(cache_key_name);
                return true;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            B_BASE::record.clear();
            return false;
        }
        http::obj_val fetch_json()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            http::obj_val valuetemp;
            valuetemp.set_array();

            if (iserror)
            {
                return valuetemp;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, &valuetemp](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           http::obj_val json_temp_v;
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                               {
                                                                                   json_temp_v[col_names[ij] ? col_names[ij] : ""] = "";
                                                                                   continue;
                                                                               }
                                                                               std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                               std::string temp_str(reinterpret_cast<char *>(ptr), len);
                                                                               if (!col_name.empty())
                                                                               {
                                                                                   json_temp_v[col_name] = std::move(temp_str);
                                                                               }
                                                                           }
                                                                           valuetemp.push(json_temp_v);
                                                                           effect_num++;
                                                                           return true;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return valuetemp;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            return valuetemp;
        }

        asio::awaitable<http::obj_val> async_fetch_json()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            http::obj_val valuetemp;
            valuetemp.set_array();

            if (iserror)
            {
                co_return valuetemp;
            }

            try
            {
                if (conn_empty())
                {
                    co_return valuetemp;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, &valuetemp](int col_count, char **col_names, auto get_data) -> bool
                                                                                      {
                                                                                          http::obj_val json_temp_v;
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len]      = get_data(ij);
                                                                                              std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                                              if (ptr == nullptr)
                                                                                              {
                                                                                                  json_temp_v[col_name] = "";
                                                                                                  continue;
                                                                                              }
                                                                                              std::string temp_str(reinterpret_cast<char *>(ptr), len);
                                                                                              if (!col_name.empty())
                                                                                              {
                                                                                                  json_temp_v[col_name] = std::move(temp_str);
                                                                                              }
                                                                                          }
                                                                                          valuetemp.push(json_temp_v);
                                                                                          effect_num++;
                                                                                          return true;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return valuetemp;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            co_return valuetemp;
        }

        long long get_one(long long id)
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(B_BASE::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));
            sqlstring.append(" limit 1");
            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_data_cache(sqlhashid))
                {
                    iscache = false;
                    return 0;
                }
            }

            B_BASE::data_reset();

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                unsigned int fetch_count = select_conn->fetch_directly(sqlstring,
                                                                       [this, col_pos_map = std::vector<int>{}, first_row = true](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                       {
                                                                           // 列位映射仅首行构建（同一结果集列序固定），消除每行重建。
                                                                           if (first_row)
                                                                           {
                                                                               col_pos_map.assign(col_count, 255);
                                                                               for (int ii = 0; ii < col_count; ii++)
                                                                               {
                                                                                   if (col_names[ii] && col_names[ii][0] != 0x00)
                                                                                   {
                                                                                       col_pos_map[ii] = B_BASE::findcolpos(col_names[ii]);
                                                                                   }
                                                                               }
                                                                               first_row = false;
                                                                           }
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                               {
                                                                                   static const unsigned char null_value = 0;
                                                                                   assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, B_BASE::data);
                                                                                   continue;
                                                                               }
                                                                               assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, B_BASE::data);
                                                                           }
                                                                           effect_num++;
                                                                           return false;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_data_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return effect_num;
        }

        asio::awaitable<long long> async_get_one(long long id)
        {
            effect_num = 0;
            if (selectsql.empty())
            {
                sqlstring = "SELECT *  FROM ";
            }
            else
            {
                sqlstring = "SELECT ";
                sqlstring.append(selectsql);
                sqlstring.append(" FROM ");
            }

            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(B_BASE::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));
            sqlstring.append(" limit 1");
            if (iscache)
            {
                std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);
                if (get_data_cache(sqlhashid))
                {
                    iscache = false;
                    co_return 0;
                }
            }

            B_BASE::data_reset();

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                //auto conn = co_await conn_obj->async_get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                unsigned int fetch_count = co_await select_conn->async_fetch_directly(sqlstring,
                                                                                      [this, col_pos_map = std::vector<int>{}, first_row = true](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                                      {
                                                                                          // 列位映射仅首行构建（同一结果集列序固定），消除每行重建。
                                                                                          if (first_row)
                                                                                          {
                                                                                              col_pos_map.assign(col_count, 255);
                                                                                              for (int ii = 0; ii < col_count; ii++)
                                                                                              {
                                                                                                  if (col_names[ii] && col_names[ii][0] != 0x00)
                                                                                                  {
                                                                                                      col_pos_map[ii] = B_BASE::findcolpos(col_names[ii]);
                                                                                                  }
                                                                                              }
                                                                                              first_row = false;
                                                                                          }
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                              {
                                                                                                  static const unsigned char null_value = 0;
                                                                                                  assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), (unsigned char *)&null_value, 0, B_BASE::data);
                                                                                                  continue;
                                                                                              }
                                                                                              assign_field_value(static_cast<unsigned char>(col_pos_map[ij]), ptr, len, B_BASE::data);
                                                                                          }
                                                                                          effect_num++;
                                                                                          return false;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
                if (iscache)
                {
                    if (exptime > 0)
                    {
                        save_data_cache(exptime);
                        exptime = 0;
                        iscache = false;
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        int update()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }
            sqlstring = B_BASE::make_update_sql("");
            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        // --- update_dirty：仅更新 __dirty_bits 标记的脏字段，文本协议 ---
        int update_dirty()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;

            // 1. 生成 dirty SQL（空则短路，避免全字段误更新）
            sqlstring = B_BASE::make_update_dirty_sql();
            if (sqlstring.empty())
            {
                return 0;
            }

            // 2. WHERE 处理（同 update()：where_clause 空则自动用 PK）
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }

            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                    // 成功才清脏
                    B_BASE::clear_dirty();
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        int update(const std::string &fieldname)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    return 0;
                }
            }

            sqlstring = B_BASE::make_update_sql(fieldname);
            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }
                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        asio::awaitable<int> async_update(const std::string &fieldname)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    co_return 0;
                }
            }

            sqlstring = B_BASE::make_update_sql(fieldname);
            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }
            try
            {

                if (conn_empty())
                {
                    co_return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }
        asio::awaitable<int> async_update()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    co_return 0;
                }
            }

            sqlstring = B_BASE::make_update_sql("");
            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }
            try
            {

                if (conn_empty())
                {
                    co_return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        // --- async_update_dirty：异步仅更新脏字段，文本协议 ---
        asio::awaitable<int> async_update_dirty()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;

            // 1. 生成 dirty SQL（空则短路）
            sqlstring = B_BASE::make_update_dirty_sql();
            if (sqlstring.empty())
            {
                co_return 0;
            }

            // 2. WHERE 处理
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    co_return 0;
                }
            }

            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }
            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                    // 成功才清脏
                    B_BASE::clear_dirty();
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        int update_batch(const std::string &fieldname)
        {
            effect_num = 0;
            if (B_BASE::record.size() == 0)
            {
                return 0;
            }
            if (fieldname.size() > 0)
            {
                sqlstring = B_BASE::make_record_into_sql(fieldname);
            }
            else
            {
                sqlstring = B_BASE::make_record_replace_sql();
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }
        int remove()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }

            sqlstring = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        asio::awaitable<unsigned int> async_remove()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    co_return 0;
                }
            }

            sqlstring = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        int remove(long long id)
        {
            effect_num = 0;
            sqlstring  = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(B_BASE::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        asio::awaitable<unsigned int> async_remove(long long id)
        {
            effect_num = 0;
            sqlstring  = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            sqlstring.append(B_BASE::getPKname());
            sqlstring.append("=");
            sqlstring.append(std::to_string(id));

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        int soft_remove(const std::string &fieldsql)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }

            sqlstring = B_BASE::soft_remove_sql(fieldsql);
            if (sqlstring.empty())
            {
                error_msg = "soft delete field empty.";
                return 0;
            }
            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }
        int soft_remove()
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    effect_num = 1;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }
            if (effect_num == 1)
            {
                sqlstring = B_BASE::soft_remove_sql(" ");
            }
            else
            {
                sqlstring = B_BASE::soft_remove_sql("");
            }
            effect_num = 0;
            if (sqlstring.empty())
            {
                error_msg = "soft delete field empty.";
                return 0;
            }
            sqlstring.append(" where ");
            if (where_clause.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }
        std::tuple<unsigned int, unsigned long long> insert(xtask_info::meta &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::make_data_insert_sql(insert_data);

            if (iserror)
            {
                return std::make_tuple(0, 0);
            }

            try
            {
                if (conn_empty())
                {
                    return std::make_tuple(0, 0);
                }
                //auto conn = conn_obj->get_mysql_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                else
                {
                    effect_num     = affected;
                    insert_last_id = edit_conn->last_insert_id();
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(xtask_info::meta &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::make_data_insert_sql(insert_data);

            if (iserror)
            {
                co_return std::make_tuple(0, 0);
            }

            try
            {
                if (conn_empty())
                {
                    co_return std::make_tuple(0, 0);
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }
                else
                {
                    effect_num     = affected;
                    insert_last_id = edit_conn->last_insert_id();
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            co_return std::make_tuple(0, 0);
        }

        std::tuple<unsigned int, unsigned long long> insert(std::vector<xtask_info::meta> &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::make_vector_insert_sql(insert_data);

            if (iserror)
            {
                return std::make_tuple(0, 0);
            }

            try
            {
                if (conn_empty())
                {
                    return std::make_tuple(0, 0);
                }
                //auto conn = conn_obj->get_mysql_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                else
                {
                    effect_num     = affected;
                    insert_last_id = edit_conn->last_insert_id();
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(std::vector<xtask_info::meta> &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::make_vector_insert_sql(insert_data);

            if (iserror)
            {
                co_return std::make_tuple(0, 0);
            }

            try
            {
                if (conn_empty())
                {
                    co_return std::make_tuple(0, 0);
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }
                else
                {
                    effect_num     = affected;
                    insert_last_id = edit_conn->last_insert_id();
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            co_return std::make_tuple(0, 0);
        }

        std::tuple<unsigned int, unsigned long long> insert()
        {
            effect_num = 0;
            sqlstring  = B_BASE::make_data_insert_sql();

            if (iserror)
            {
                return std::make_tuple(0, 0);
            }

            try
            {
                if (conn_empty())
                {
                    return std::make_tuple(0, 0);
                }
                //auto conn = conn_obj->get_mysql_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                else
                {
                    effect_num     = affected;
                    insert_last_id = edit_conn->last_insert_id();
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert()
        {
            effect_num = 0;
            sqlstring  = B_BASE::make_data_insert_sql();

            if (iserror)
            {
                co_return std::make_tuple(0, 0);
            }

            try
            {
                if (conn_empty())
                {
                    co_return std::make_tuple(0, 0);
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }
                else
                {
                    effect_num     = affected;
                    insert_last_id = edit_conn->last_insert_id();
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            co_return std::make_tuple(0, 0);
        }

        std::tuple<unsigned int, unsigned long long> save(bool isrealnew = false)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (B_BASE::getPK() > 0 && isrealnew == false)
            {
                if (where_clause.empty())
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                sqlstring = B_BASE::make_update_sql("");
                sqlstring.append(" where ");
                if (where_clause.empty())
                {
                    return std::make_tuple(0, 0);
                }
                else
                {
                    sqlstring.append(where_clause);
                }
                if (!groupsql.empty())
                {
                    sqlstring.append(groupsql);
                }
                if (!ordersql.empty())
                {
                    sqlstring.append(ordersql);
                }
                if (!limitsql.empty())
                {
                    sqlstring.append(limitsql);
                }

                if (iserror)
                {
                    return std::make_tuple(0, 0);
                }
                if (conn_empty())
                {
                    return std::make_tuple(0, 0);
                }
                //auto conn = conn_obj->get_mysql_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                return std::make_tuple(effect_num, 0);
            }
            else
            {
                sqlstring = B_BASE::make_data_insert_sql();
                if (conn_empty())
                {
                    return std::make_tuple(0, 0);
                }
                //auto conn = conn_obj->get_mysql_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = edit_conn->exec_dml(sqlstring);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                else
                {
                    effect_num     = affected;
                    insert_last_id = edit_conn->last_insert_id();
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }
                return std::make_tuple(effect_num, insert_last_id);
            }
            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_save(bool isrealnew = false)
        {
            std::string where_clause;
            build_text_where(where_clause);
            effect_num = 0;
            if (B_BASE::getPK() > 0 && isrealnew == false)
            {
                if (where_clause.empty())
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                sqlstring = B_BASE::make_update_sql("");
                sqlstring.append(" where ");
                if (where_clause.empty())
                {
                    co_return std::make_tuple(0, 0);
                }
                else
                {
                    sqlstring.append(where_clause);
                }
                if (!groupsql.empty())
                {
                    sqlstring.append(groupsql);
                }
                if (!ordersql.empty())
                {
                    sqlstring.append(ordersql);
                }
                if (!limitsql.empty())
                {
                    sqlstring.append(limitsql);
                }

                if (iserror)
                {
                    co_return std::make_tuple(0, 0);
                }

                try
                {
                    if (conn_empty())
                    {
                        co_return std::make_tuple(0, 0);
                    }

                    if (islock_conn)
                    {
                        if (!edit_conn || edit_conn->isclose)
                        {
                            edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                        }
                    }
                    else
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }

                    if (edit_conn->isdebug)
                    {
                        edit_conn->begin_time();
                    }
                    unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                    if (edit_conn->isdebug)
                    {
                        edit_conn->finish_time();
                        auto &conn_mar    = get_orm_connect_mar();
                        long long du_time = edit_conn->count_time();
                        conn_mar.push_log(sqlstring, std::to_string(du_time));
                    }

                    if (affected == static_cast<unsigned int>(-1))
                    {
                        error_msg = edit_conn->error_msg;
                        iserror   = true;
                        edit_conn.reset();
                        co_return std::make_tuple(0, 0);
                    }
                    else
                    {
                        effect_num = affected;
                        if (!islock_conn)
                        {
                            conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                        }
                    }
                    co_return std::make_tuple(effect_num, 0);
                }
                catch (const std::exception &e)
                {
                    error_msg = std::string(e.what());
                    unlock_conn();
                    co_return std::make_tuple(0, 0);
                }

                co_return std::make_tuple(0, 0);
            }
            else
            {
                sqlstring = B_BASE::make_data_insert_sql();
                try
                {
                    if (conn_empty())
                    {
                        co_return std::make_tuple(0, 0);
                    }
                    if (islock_conn)
                    {
                        if (!edit_conn || edit_conn->isclose)
                        {
                            edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                        }
                    }
                    else
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }

                    if (edit_conn->isdebug)
                    {
                        edit_conn->begin_time();
                    }
                    unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);
                    if (edit_conn->isdebug)
                    {
                        edit_conn->finish_time();
                        auto &conn_mar    = get_orm_connect_mar();
                        long long du_time = edit_conn->count_time();
                        conn_mar.push_log(sqlstring, std::to_string(du_time));
                    }

                    long long insert_last_id = 0;
                    if (affected == static_cast<unsigned int>(-1))
                    {
                        error_msg = edit_conn->error_msg;
                        iserror   = true;
                        edit_conn.reset();
                        co_return std::make_tuple(0, 0);
                    }
                    else
                    {
                        effect_num     = affected;
                        insert_last_id = edit_conn->last_insert_id();
                        B_BASE::setPK(insert_last_id);
                        if (!islock_conn)
                        {
                            conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                        }
                    }
                    co_return std::make_tuple(effect_num, insert_last_id);
                }
                catch (const std::exception &e)
                {
                    error_msg = std::string(e.what());
                    unlock_conn();
                }

                co_return std::make_tuple(0, 0);
            }
            co_return std::make_tuple(0, 0);
        }

        template <ResultHasSetVal T>
        unsigned int query(const std::string &rawsql, std::vector<T> &result_record)
        {
            effect_num = 0;
            if (rawsql.size() > 10)
            {
                unsigned int i = 0;
                for (; i < rawsql.size(); i++)
                {
                    if (rawsql[i] != 0x20)
                    {
                        break;
                    }
                }
                if (i < 5)
                {
                    //must be select
                    if (rawsql[i] != 's' && rawsql[i] != 'S')
                    {
                        effect_num = edit_query(rawsql);
                        return effect_num;
                    }
                }
                else
                {
                    iserror = true;
                }
            }
            else
            {
                iserror = true;
            }

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }
                //auto conn = conn_obj->get_mysql_select_conn();
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = conn_obj->get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                unsigned int fetch_count = select_conn->fetch_directly(rawsql,
                                                                       [this, &result_record](int col_count, char **col_names, auto get_data) -> bool
                                                                       {
                                                                           T data_temp;
                                                                           for (int ij = 0; ij < col_count; ij++)
                                                                           {
                                                                               auto [ptr, len] = get_data(ij);
                                                                               if (ptr == nullptr)
                                                                                   continue;
                                                                               std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                               if (!col_name.empty())
                                                                               {
                                                                                   data_temp.set_val(col_name, ptr, len, 0);
                                                                               }
                                                                           }
                                                                           result_record.emplace_back(std::move(data_temp));
                                                                           effect_num++;
                                                                           return true;
                                                                       });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            return 0;
        }
        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_query(const std::string &rawsql, std::vector<T> &result_record)
        {
            effect_num = 0;

            if (rawsql.size() > 10)
            {
                unsigned int i = 0;
                for (; i < rawsql.size(); i++)
                {
                    if (rawsql[i] != 0x20)
                    {
                        break;
                    }
                }
                if (i < 5)
                {
                    //must be select
                    if (rawsql[i] != 's' && rawsql[i] != 'S')
                    {
                        effect_num = co_await async_edit_query(rawsql);
                        co_return effect_num;
                    }
                }
                else
                {
                    iserror = true;
                }
            }
            else
            {
                iserror = true;
            }

            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }
                if (islock_conn)
                {
                    if (!select_conn || select_conn->isclose)
                    {
                        select_conn = co_await conn_obj->async_get_mysql_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                unsigned int fetch_count = co_await select_conn->async_fetch_directly(rawsql,
                                                                                      [this, &result_record](int col_count, char **col_names, auto get_data) -> bool
                                                                                      {
                                                                                          T data_temp;
                                                                                          for (int ij = 0; ij < col_count; ij++)
                                                                                          {
                                                                                              auto [ptr, len] = get_data(ij);
                                                                                              if (ptr == nullptr)
                                                                                                  continue;
                                                                                              std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                                                              if (!col_name.empty())
                                                                                              {
                                                                                                  data_temp.set_val(col_name, ptr, len, 0);
                                                                                              }
                                                                                          }
                                                                                          result_record.emplace_back(std::move(data_temp));
                                                                                          effect_num++;
                                                                                          return true;
                                                                                      });
                if (fetch_count == 0 && !select_conn->error_msg.empty())
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }

            co_return 0;
        }

        int edit_query(const std::string &rawsql)
        {
            effect_num = 0;

            if (iserror)
            {
                return 0;
            }

            try
            {
                if (conn_empty())
                {
                    return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = conn_obj->get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                unsigned int affected = edit_conn->exec_dml(rawsql);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                return 0;
            }

            return 0;
        }

        asio::awaitable<unsigned int> async_edit_query(const std::string &rawsql)
        {
            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }

            try
            {
                if (conn_empty())
                {
                    co_return 0;
                }

                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                unsigned int affected = co_await edit_conn->async_exec_dml(rawsql);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return 0;
                }
                else
                {
                    effect_num = affected;
                    if (!islock_conn)
                    {
                        conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                    }
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
                co_return 0;
            }

            co_return 0;
        }

        void parse_leftjion()
        {
            if (join_ptr == nullptr)
            {
                return;
            }

            if (join_ptr->parsed)
            {
                return;
            }
            join_ptr->parsed = true;

            std::string sqlselect_;
            if (selectsql.size() == 0)
            {
                if (join_ptr->selectsql.size() == 0)
                {
                    sqlselect_.append(B_BASE::tablename);
                    sqlselect_.append(".* ");
                }
            }
            else
            {
                bool ishastabname = false;
                for (unsigned int i = 0; i < selectsql.size(); i++)
                {
                    if (selectsql[i] == '.')
                    {
                        ishastabname = true;
                        sqlselect_.append(selectsql);
                        break;
                    }
                    else if (selectsql[i] == ',')
                    {
                        break;
                    }
                }
                if (!ishastabname)
                {
                    sqlselect_.append(B_BASE::tablename);
                    sqlselect_.push_back('.');
                    unsigned int i = 0;
                    for (; i < selectsql.size(); i++)
                    {
                        if (selectsql[i] == ' ')
                        {
                            continue;
                        }
                        break;
                    }
                    for (; i < selectsql.size(); i++)
                    {
                        if (selectsql[i] == ',')
                        {
                            sqlselect_.push_back(',');
                            sqlselect_.append(B_BASE::tablename);
                            sqlselect_.push_back('.');
                            bool isspace = false;
                            for (; i < selectsql.size(); i++)
                            {
                                if (selectsql[i] == ' ')
                                {
                                    isspace = true;
                                    continue;
                                }
                                break;
                            }
                            if (isspace)
                            {
                                i--;
                            }
                            continue;
                        }
                        sqlselect_.push_back(selectsql[i]);
                    }
                }
            }

            if (join_ptr->selectsql.size() == 0)
            {
                if (sqlselect_.size() > 0)
                {
                    sqlselect_.push_back(',');
                }

                sqlselect_.append(join_ptr->join_table);
                sqlselect_.append(".* ");
            }
            else
            {
                bool ishastabname = false;
                for (unsigned int i = 0; i < join_ptr->selectsql.size(); i++)
                {
                    if (join_ptr->selectsql[i] == '.')
                    {
                        ishastabname = true;
                        if (sqlselect_.size() > 0)
                        {
                            sqlselect_.push_back(',');
                        }
                        sqlselect_.append(join_ptr->selectsql);
                        break;
                    }
                    else if (join_ptr->selectsql[i] == ',')
                    {
                        break;
                    }
                }
                if (!ishastabname)
                {
                    if (sqlselect_.size() > 0)
                    {
                        sqlselect_.push_back(',');
                    }
                    sqlselect_.append(join_ptr->join_table);
                    sqlselect_.push_back('.');
                    unsigned int i = 0;
                    for (; i < join_ptr->selectsql.size(); i++)
                    {
                        if (join_ptr->selectsql[i] == ' ')
                        {
                            continue;
                        }
                        break;
                    }
                    for (; i < join_ptr->selectsql.size(); i++)
                    {
                        if (join_ptr->selectsql[i] == ',')
                        {
                            sqlselect_.push_back(',');
                            sqlselect_.append(join_ptr->join_table);
                            sqlselect_.push_back('.');
                            bool isspace = false;
                            i++;
                            for (; i < join_ptr->selectsql.size(); i++)
                            {
                                if (join_ptr->selectsql[i] == ' ')
                                {
                                    isspace = true;
                                    continue;
                                }
                                break;
                            }
                            if (isspace)
                            {
                                i--;
                            }
                            continue;
                        }
                        sqlselect_.push_back(join_ptr->selectsql[i]);
                    }
                }
            }
            selectsql = sqlselect_;
        }
        template <HasOrgTablename T>
        M_MODEL &leftJoin()
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }
            join_ptr->join_table = T::org_tablename;
            return *mod;
        }
        template <HasOrgTablename T>
        M_MODEL &leftJoin(const T &)
        {
            return leftJoin<T>();
        }

        template <HasOrgTablename T>
        M_MODEL &innerJoin()
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }
            join_ptr->join_table = T::org_tablename;
            join_ptr->isleft     = false;
            return *mod;
        }
        template <HasOrgTablename T>
        M_MODEL &innerJoin(const T &)
        {
            return innerJoin<T>();
        }

        // 从 on_sqls 统一组装 LEFT JOIN 的 ON 条件
        void build_join_on_sql(std::string &on_clause)
        {
            if (!join_ptr || join_ptr->on_sqls.empty())
                return;

            const std::string &main_tbl = B_BASE::tablename;
            const std::string &join_tbl = join_ptr->join_table;

            for (const auto &item : join_ptr->on_sqls)
            {
                if (!on_clause.empty())
                {
                    if (item.pre_op == 2)
                        on_clause.append(" OR ");
                    else if (item.pre_op == 1)
                        on_clause.append(" AND ");
                }
                on_clause.append(join_tbl);
                on_clause.append(".");
                on_clause.append(item.right_filed_name);

                switch (item.op_type)
                {
                case orm::wq::eq: on_clause.append(" = "); break;
                case orm::wq::nq: on_clause.append(" != "); break;
                case orm::wq::lt: on_clause.append(" < "); break;
                case orm::wq::le: on_clause.append(" <= "); break;
                case orm::wq::bt: on_clause.append(" > "); break;
                case orm::wq::be: on_clause.append(" >= "); break;
                case orm::wq::like: on_clause.append(" LIKE "); break;
                // ON 是列与列比较，通配符无处安放，只取关键字
                case orm::wq::llike: on_clause.append(" LIKE "); break;
                case orm::wq::rlike: on_clause.append(" LIKE "); break;
                case orm::wq::nlike: on_clause.append(" NOT LIKE "); break;
                default: on_clause.append(" = "); break;
                }

                on_clause.append(main_tbl);
                on_clause.append(".");
                on_clause.append(item.left_filed_name);
            }
        }

        // 从 sub_sqls 组装子查询内部 WHERE（joinWhere 填充）
        void build_join_sub_sql(std::string &sub_where)
        {
            if (!join_ptr || join_ptr->sub_sqls.empty())
                return;

            for (const auto &item : join_ptr->sub_sqls)
            {
                if (!sub_where.empty())
                {
                    if (item.pre_op == 2)
                        sub_where.append(" OR ");
                    else if (item.pre_op == 1)
                        sub_where.append(" AND ");
                }
                // 子查询单表，字段名不需要表名前缀
                sub_where.append(escape_mysql_col(item.left_filed_name));

                switch (item.op_type)
                {
                case orm::wq::eq:
                    sub_where.append(" = ");
                    escape_text_value(sub_where, item.filed_value, item.need_quote);
                    break;
                case orm::wq::nq:
                    sub_where.append(" != ");
                    escape_text_value(sub_where, item.filed_value, item.need_quote);
                    break;
                case orm::wq::lt:
                    sub_where.append(" < ");
                    escape_text_value(sub_where, item.filed_value, item.need_quote);
                    break;
                case orm::wq::le:
                    sub_where.append(" <= ");
                    escape_text_value(sub_where, item.filed_value, item.need_quote);
                    break;
                case orm::wq::bt:
                    sub_where.append(" > ");
                    escape_text_value(sub_where, item.filed_value, item.need_quote);
                    break;
                case orm::wq::be:
                    sub_where.append(" >= ");
                    escape_text_value(sub_where, item.filed_value, item.need_quote);
                    break;
                case orm::wq::like:
                    sub_where.append(" LIKE ");
                    append_like_text(sub_where, item.filed_value, true, true);
                    break;
                case orm::wq::llike:
                    sub_where.append(" LIKE ");
                    append_like_text(sub_where, item.filed_value, true, false);
                    break;
                case orm::wq::rlike:
                    sub_where.append(" LIKE ");
                    append_like_text(sub_where, item.filed_value, false, true);
                    break;
                case orm::wq::nlike:
                    sub_where.append(" NOT LIKE ");
                    append_like_text(sub_where, item.filed_value, true, true);
                    break;
                case orm::wq::in:
                case orm::wq::notin:
                    sub_where.append(item.op_type == orm::wq::in ? " IN (" : " NOT IN (");
                    if (item.filed_value.is_array())
                    {
                        for (size_t a = 0; a < item.filed_value.size(); ++a)
                        {
                            if (a > 0)
                                sub_where.append(", ");
                            escape_text_value(sub_where, item.filed_value[a], item.need_quote);
                        }
                    }
                    else
                    {
                        sub_where.append(item.filed_value.to_string());
                    }
                    sub_where.append(")");
                    break;
                case orm::wq::isnull: sub_where.append(" IS NULL"); break;
                case orm::wq::notnull: sub_where.append(" IS NOT NULL"); break;
                default:
                    sub_where.append(" = ");
                    escape_text_value(sub_where, item.filed_value, item.need_quote);
                    break;
                }
            }
        }

        void get_join_table()
        {
            get_join_table(sqlstring);
        }
        void get_join_table(std::string &out)
        {
            if (join_ptr == nullptr)
            {
                return;
            }

            if (join_ptr->isleft)
            {
                out.append(" LEFT JOIN ");
            }
            else
            {
                out.append(" INNER JOIN ");
            }

            std::string on_clause;
            build_join_on_sql(on_clause);

            if (on_clause.empty())
            {
                iserror = true;
                out.append(join_ptr->join_table);
                out.append(" ON 1");
                return;
            }

            if (join_ptr->limitsql.empty())
            {
                // 分支 1: 没有限制, 简单 JOIN
                out.append(join_ptr->join_table);
                out.append(" ON ");
                out.append(on_clause);
            }
            else if (join_ptr->parbysql.empty())
            {
                // 分支 2: 有 LIMIT 但无 PARTITION → 直接包一层子查询 LIMIT
                out.append(" ( SELECT ");
                if (join_ptr->selectsql.empty())
                {
                    out.append(" * ");
                }
                else
                {
                    out.append(join_ptr->selectsql);
                }
                out.append(" FROM ");
                out.append(join_ptr->join_table);

                std::string sub_where;
                build_join_sub_sql(sub_where);
                if (!sub_where.empty())
                {
                    out.append(" WHERE ");
                    out.append(sub_where);
                }
                if (!join_ptr->ordersql.empty())
                {
                    out.append(" ");
                    out.append(join_ptr->ordersql);
                }
                out.append(" LIMIT ");
                out.append(join_ptr->limitsql);

                out.append(" ) ");
                out.append(join_ptr->join_table);
                out.append(" ON ");
                out.append(on_clause);
            }
            else
            {
                // 分支 3: 有 LIMIT 且有 PARTITION → ROW_NUMBER() 窗口函数
                // parbysql 由 joinGroup/joinParAppend 设置, 这里把 joinOn 的 JOIN 表字段补上;
                // parbysql 与 joinOn 右列同为 JOIN 表字段, 只能按列名精确去重:
                // 主表的 findcolpos 按首字母哈希且属另一张表的命名空间, 会把 item_id 误判成与 id 同列
                std::vector<std::string> parby_cols;
                {
                    std::string_view par_by = join_ptr->parbysql;
                    while (!par_by.empty())
                    {
                        auto cut             = par_by.find(',');
                        std::string_view one = (cut == std::string_view::npos) ? par_by : par_by.substr(0, cut);
                        while (!one.empty() && (one.front() == ' ' || one.front() == '\t'))
                        {
                            one.remove_prefix(1);
                        }
                        while (!one.empty() && (one.back() == ' ' || one.back() == '\t'))
                        {
                            one.remove_suffix(1);
                        }
                        if (!one.empty())
                        {
                            parby_cols.emplace_back(one);
                        }
                        if (cut == std::string_view::npos)
                        {
                            break;
                        }
                        par_by.remove_prefix(cut + 1);
                    }
                }

                for (const auto &cond : join_ptr->on_sqls)
                {
                    if (cond.right_filed_name.empty())
                    {
                        continue;
                    }
                    bool exist_pos = false;
                    for (const std::string &one_col : parby_cols)
                    {
                        if (one_col == cond.right_filed_name)
                        {
                            exist_pos = true;
                            break;
                        }
                    }
                    if (exist_pos)
                    {
                        continue;
                    }
                    join_ptr->parbysql.append(",");
                    join_ptr->parbysql.append(cond.right_filed_name);
                }

                out.append(" ( SELECT ");
                std::string sub_where;
                build_join_sub_sql(sub_where);

                if (join_ptr->selectsql.empty())
                {
                    out.append(" *, ");
                }
                else
                {
                    out.append(trip_as_field(join_ptr->selectsql));
                    out.append(", ");
                }

                out.append("ROW_NUMBER() OVER(PARTITION BY ");
                out.append(join_ptr->parbysql);
                out.append(" ");
                out.append(join_ptr->ordersql);
                out.append(") AS rn FROM ");
                out.append(join_ptr->join_table);

                if (!sub_where.empty())
                {
                    out.append(" WHERE ");
                    out.append(sub_where);
                }
                out.append(" ) ");
                out.append(join_ptr->join_table);
                out.append(" ON ");
                out.append(on_clause);
                out.append(" AND ");
                out.append(join_ptr->join_table);
                out.append(".rn <= ");
                out.append(join_ptr->limitsql);
            }
        }
        std::string trip_as_field(std::string_view fields)
        {
            std::string str_tm_;
            unsigned int i = 0;
            for (; i < fields.size(); i++)
            {
                if (fields[i] == ' ')
                {
                    continue;
                }
                break;
            }

            for (; i < fields.size(); i++)
            {
                if (fields[i] == ' ')
                {
                    bool isneed = false;
                    for (; i < fields.size(); i++)
                    {
                        if (fields[i] == ' ')
                        {
                            continue;
                        }
                        isneed = true;
                        break;
                    }

                    if ((i + 3) < fields.size())
                    {
                        if (fields[i] == 'A' || fields[i] == 'a')
                        {
                            if (fields[i + 1] == 'S' || fields[i + 1] == 's')
                            {
                                if (fields[i + 2] == ' ')
                                {
                                    isneed = false;
                                    i      = i + 3;
                                    for (; i < fields.size(); i++)
                                    {
                                        if (fields[i] == ' ')
                                        {
                                            continue;
                                        }
                                        break;
                                    }
                                    //skip as name
                                    for (; i < fields.size(); i++)
                                    {
                                        if (fields[i] == ' ')
                                        {
                                            for (; i < fields.size(); i++)
                                            {
                                                if (fields[i] == ' ')
                                                {
                                                    continue;
                                                }
                                                break;
                                            }
                                            isneed = true;
                                            break;
                                        }
                                        else if (fields[i] == ',')
                                        {
                                            isneed = true;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (isneed)
                    {
                        i--;
                    }
                    continue;
                }
                str_tm_.push_back(fields[i]);
            }

            return str_tm_;
        }
        M_MODEL &joinSelect(std::string_view fields)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }
            join_ptr->selectsql = fields;

            return *mod;
        }
        // joinOn 通用版本: join_field(join表字段) = main_field(主表字段)
        M_MODEL &joinOn(std::string_view join_field, std::string_view main_field)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            orm::orm_where_join_t item;
            item.pre_op           = join_ptr->on_sqls.empty() ? 0 : 1;
            item.op_type          = orm::wq::eq;
            item.right_filed_name = join_field;
            item.left_filed_name  = main_field;
            join_ptr->on_sqls.push_back(std::move(item));

            return *mod;
        }

        // joinOn 模板版: 主表字段用 cols 枚举
        M_MODEL &joinOn(std::string_view join_field, B_BASE::cols main_field)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            orm::orm_where_join_t item;
            item.pre_op           = join_ptr->on_sqls.empty() ? 0 : 1;
            item.op_type          = orm::wq::eq;
            item.left_idx         = static_cast<unsigned char>(main_field);
            item.need_quote       = B_BASE::col_need_quote[item.left_idx];
            item.left_filed_name  = xtask_info::col_names[item.left_idx];
            item.right_filed_name = join_field;
            join_ptr->on_sqls.push_back(std::move(item));

            return *mod;
        }
        template <typename T>
        std::string to_sql_value(T &&val)
        {
            using RawType = std::decay_t<T>;

            if constexpr (std::is_same_v<RawType, bool>)
            {
                return val ? "1" : "0";
            }
            else if constexpr (std::is_arithmetic_v<RawType>)
            {
                return std::to_string(std::forward<T>(val));
            }
            else
            {
                std::string str(std::forward<T>(val));
                std::string result = "'";
                for (char c : str)
                {
                    if (c == '\'')
                        result += "''";// SQL标准单引号转义
                    else
                        result += c;
                }
                result += "'";
                return result;
            }
        }
        template <typename T2>
        M_MODEL &joinWhere(std::string_view field, T2 &&value)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            orm::orm_where_join_t item;
            item.pre_op          = join_ptr->sub_sqls.empty() ? 0 : 1;
            item.op_type         = orm::wq::eq;
            item.left_filed_name = field;

            if (join_ptr->find_join_col_indexd_ptr != nullptr)
            {
                item.left_idx = join_ptr->find_join_col_indexd_ptr(field);
                if (item.left_idx == 255)
                {
                    error_msg = "field is not join table column";
                    iserror   = true;
                }
            }

            item.filed_value = std::forward<T2>(value);
            join_ptr->sub_sqls.push_back(std::move(item));
            return *mod;
        }

        template <typename T2>
        M_MODEL &joinWhere(std::string_view field, orm::wq opwq, T2 &&value)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            orm::orm_where_join_t item;
            item.pre_op          = join_ptr->sub_sqls.empty() ? 0 : 1;
            item.op_type         = opwq;
            item.left_filed_name = field;

            if constexpr (std::is_convertible_v<decltype(value), std::string_view>)
            {
                item.filed_value = std::string(value);
            }
            else
            {
                item.filed_value = std::forward<T2>(value);
            }

            join_ptr->sub_sqls.push_back(std::move(item));
            return *mod;
        }

        template <typename T2>
        M_MODEL &joinWhereOr(std::string_view field, orm::wq opwq, T2 &&value)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            orm::orm_where_join_t item;
            item.pre_op          = join_ptr->sub_sqls.empty() ? 0 : 2;
            item.op_type         = opwq;
            item.left_filed_name = field;

            if constexpr (std::is_convertible_v<decltype(value), std::string_view>)
            {
                item.filed_value = std::string(value);
            }
            else
            {
                item.filed_value = std::forward<T2>(value);
            }

            join_ptr->sub_sqls.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &joinLimit(unsigned int n)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            join_ptr->limitsql.append(std::to_string(n));
            return *mod;
        }
        M_MODEL &joinParAppend(std::string_view field)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            if (!join_ptr->parbysql.empty())
            {
                join_ptr->parbysql.append(",");
            }
            join_ptr->parbysql.append(field);
            return *mod;
        }

        //分组
        M_MODEL &joinGroup(std::string_view field)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            join_ptr->parbysql = field;
            return *mod;
        }

        M_MODEL &joinDesc(std::string_view field)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }
            if (join_ptr->ordersql.empty())
            {
                join_ptr->ordersql = " ORDER BY ";
                join_ptr->ordersql.append(field);
                join_ptr->ordersql.append(" DESC ");
            }
            else
            {
                join_ptr->ordersql.append(" , ");
                join_ptr->ordersql.append(field);
                join_ptr->ordersql.append(" DESC ");
            }
            return *mod;
        }

        M_MODEL &joinAsc(std::string_view field)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            if (join_ptr->ordersql.empty())
            {
                join_ptr->ordersql = " ORDER BY ";
                join_ptr->ordersql.append(field);
                join_ptr->ordersql.append(" ASC ");
            }
            else
            {
                join_ptr->ordersql.append(" , ");
                join_ptr->ordersql.append(field);
                join_ptr->ordersql.append(" ASC ");
            }
            return *mod;
        }

        std::string commit_insert(xtask_info::meta &insert_data)
        {
            return B_BASE::make_data_insert_sql(insert_data);
        }

        std::string commit_insert()
        {
            return B_BASE::make_data_insert_sql();
        }
        std::string commit_update(const std::string &fieldname)
        {
            std::string where_clause;
            build_text_where(where_clause);
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    return "";
                }
            }

            sqlstring = B_BASE::make_update_sql(fieldname);
            sqlstring.append(" WHERE ");
            if (where_clause.empty())
            {
                return "";
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }
            return sqlstring;
        }

        std::string commit_remove()
        {
            std::string where_clause;
            build_text_where(where_clause);
            if (where_clause.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    where_clause = tempsql.str();
                }
                else
                {
                    return "";
                }
            }

            sqlstring = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (where_clause.empty())
            {
                return "";
            }
            else
            {
                sqlstring.append(where_clause);
            }
            if (!groupsql.empty())
            {
                sqlstring.append(groupsql);
            }
            if (!ordersql.empty())
            {
                sqlstring.append(ordersql);
            }
            if (!limitsql.empty())
            {
                sqlstring.append(limitsql);
            }
            return sqlstring;
        }

        M_MODEL &clear(bool both = true)
        {
            selectsql.clear();
            ordersql.clear();
            groupsql.clear();
            limitsql.clear();
            sqlstring.clear();
            error_msg.clear();
            join_ptr.reset();
            wheresql.clear();

            iscache    = false;
            iserror    = false;
            effect_num = 0;
            if (both)
            {
                B_BASE::record_reset();
                B_BASE::data_reset();
            }
            return *mod;
        }
        M_MODEL &clearWhere()
        {
            selectsql.clear();
            ordersql.clear();
            groupsql.clear();
            limitsql.clear();
            sqlstring.clear();
            error_msg.clear();
            wheresql.clear();

            iscache    = false;
            iserror    = false;
            effect_num = 0;
            join_ptr.reset();
            return *mod;
        }
        M_MODEL &set_data(xtask_info::meta indata)
        {
            B_BASE::data = indata;
            return *mod;
        }
        M_MODEL &get() { return *mod; }
        std::string get_query() { return sqlstring; }

        unsigned int effect()
        {
            return effect_num;
        }
        bool conn_empty()
        {
            if (conn_obj)
            {
                return false;
            }
            error_msg = "conn_obj is null";
            iserror   = true;
            return true;
        }
        void lock_conn()
        {
            islock_conn = true;
        }
        void unlock_conn()
        {
            islock_conn = false;
            if (conn_obj)
            {
                if (select_conn)
                {
                    conn_obj->back_mysql_select_conn(std::move(select_conn));
                }

                if (edit_conn)
                {
                    conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                }
            }
        }
        // lock_conn 模式下析构时自动清理连接（防忘记 unlock_conn）
        // 断连的连接直接 reset，不归还池（避免污染池）
        ~xtask_opsql()
        {
            if (islock_conn)
            {
                if (select_conn)
                    select_conn.reset();
                if (edit_conn)
                    edit_conn.reset();
                islock_conn = false;
            }
        }

        // 移动构造，unique_ptr join_ptr 又使隐式拷贝成为 deleted。
        // mod 必须重指到新对象：链式接口全部 return *mod，否则 SQL 会写进被搬空的那个临时量。
        xtask_opsql(const xtask_opsql &)            = delete;
        xtask_opsql &operator=(const xtask_opsql &) = delete;
        xtask_opsql(xtask_opsql &&o) noexcept : B_BASE(std::move(o))
        {
            selectsql     = std::move(o.selectsql);
            ordersql      = std::move(o.ordersql);
            groupsql      = std::move(o.groupsql);
            limitsql      = std::move(o.limitsql);
            sqlstring     = std::move(o.sqlstring);
            dbtag         = std::move(o.dbtag);
            error_msg     = std::move(o.error_msg);
            iscache       = o.iscache;
            iserror       = o.iserror;
            islock_conn   = o.islock_conn;
            exptime       = o.exptime;
            effect_num    = o.effect_num;
            wheresql      = std::move(o.wheresql);
            join_ptr      = std::move(o.join_ptr);
            select_conn   = std::move(o.select_conn);
            edit_conn     = std::move(o.edit_conn);
            conn_obj      = std::move(o.conn_obj);
            mod           = static_cast<M_MODEL *>(this);
            o.mod         = static_cast<M_MODEL *>(&o);
            o.islock_conn = false;
        }
        xtask_opsql &operator=(xtask_opsql &&o) noexcept
        {
            if (this != &o)
            {
                static_cast<B_BASE &>(*this) = std::move(static_cast<B_BASE &>(o));
                selectsql                    = std::move(o.selectsql);
                ordersql                     = std::move(o.ordersql);
                groupsql                     = std::move(o.groupsql);
                limitsql                     = std::move(o.limitsql);
                sqlstring                    = std::move(o.sqlstring);
                dbtag                        = std::move(o.dbtag);
                error_msg                    = std::move(o.error_msg);
                iscache                      = o.iscache;
                iserror                      = o.iserror;
                islock_conn                  = o.islock_conn;
                exptime                      = o.exptime;
                effect_num                   = o.effect_num;
                wheresql                     = std::move(o.wheresql);
                join_ptr                     = std::move(o.join_ptr);
                select_conn                  = std::move(o.select_conn);
                edit_conn                    = std::move(o.edit_conn);
                conn_obj                     = std::move(o.conn_obj);
                mod                          = static_cast<M_MODEL *>(this);
                o.mod                        = static_cast<M_MODEL *>(&o);
                o.islock_conn                = false;
            }
            return *this;
        }

        DB_TYPE get_db_type()
        {
            return DB_TYPE::MYSQL;
        }

        // ===== 预编译语句（Prepared Statements）=====
        // --- AND / OR（string_view 版本，共享）---
        template <typename T>
        M_MODEL &AND(orm::table_col<B_BASE, &xtask_info::col_names> field, orm::wq opwq, T val)
        {
            orm_where_sql_t item;
            item.pre_op      = wheresql.empty() ? 0 : 1;
            item.op_type     = opwq;
            item.col_idx     = B_BASE::findcolpos(field);
            
            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.filed_name  = std::string(field);
            item.filed_value = item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        template <typename T>
        M_MODEL &OR(orm::table_col<B_BASE, &xtask_info::col_names> field, orm::wq opwq, T val)
        {
            orm_where_sql_t item;
            item.pre_op      = wheresql.empty() ? 0 : 2;
            item.op_type     = opwq;
            item.col_idx     = B_BASE::findcolpos(field);
            
            if (item.col_idx == 255)
            {
                error_msg = "field is not table column";
                iserror   = true;
            }

            item.filed_name  = std::string(field);
            item.filed_value = item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        template <typename T>
        M_MODEL &AND(B_BASE::cols field, orm::wq opwq, T val)
        {
            orm_where_sql_t item;
            item.pre_op      = wheresql.empty() ? 0 : 1;
            item.op_type     = opwq;
            item.col_idx     = static_cast<unsigned char>(field);
            item.need_quote  = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name  = xtask_info::col_names[item.col_idx];
            item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        template <typename T>
        M_MODEL &OR(B_BASE::cols field, orm::wq opwq, T val)
        {
            orm_where_sql_t item;
            item.pre_op      = wheresql.empty() ? 0 : 2;
            item.op_type     = opwq;
            item.col_idx     = static_cast<unsigned char>(field);
            item.need_quote  = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name  = xtask_info::col_names[item.col_idx];
            item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        // --- 括号方法 ---
        M_MODEL &andsub()
        {
            orm_where_sql_t item;
            item.pre_op    = wheresql.empty() ? 0 : 1;
            item.begin_sub = true;
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &orsub()
        {
            orm_where_sql_t item;
            item.pre_op    = wheresql.empty() ? 0 : 2;
            item.begin_sub = true;
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &endsub()
        {
            orm_where_sql_t item;
            item.end_sub = true;
            wheresql.push_back(std::move(item));
            return *mod;
        }

      private:
        // --- 构建预编译 WHERE ---
        void build_prepared_where(std::string &where_clause,
                                  std::vector<http::obj_val> &params)
        {
            const bool need_table_prefix = (join_ptr != nullptr);
            auto append_field            = [&](const std::string &fname)
            {
                if (need_table_prefix && fname.find('.') == std::string::npos)
                {
                    where_clause.append(B_BASE::tablename);
                    where_clause.append(".");
                }
                where_clause.append(escape_mysql_col(fname));
            };

            for (const auto &item : wheresql)
            {
                if (item.end_sub)
                {
                    where_clause.push_back(')');
                    continue;
                }

                // 连接词：仅当前面已有内容且不紧邻左括号时才输出，
                // 避免括号内首项被多加 AND/OR（如 "AND ( AND x = ?"）
                if (!where_clause.empty() && where_clause.back() != '(')
                {
                    switch (item.pre_op)
                    {
                    case 1: where_clause.append(" AND "); break;
                    case 2: where_clause.append(" OR "); break;
                    default: break;
                    }
                }

                if (item.begin_sub)
                {
                    where_clause.push_back('(');
                    if (item.filed_name.empty())
                        continue;
                }

                append_field(item.filed_name);
                switch (item.op_type)
                {
                case orm::wq::eq: where_clause.append(" = ?"); break;
                case orm::wq::bt: where_clause.append(" > ?"); break;
                case orm::wq::be: where_clause.append(" >= ?"); break;
                case orm::wq::lt: where_clause.append(" < ?"); break;
                case orm::wq::le: where_clause.append(" <= ?"); break;
                case orm::wq::nq: where_clause.append(" != ?"); break;
                case orm::wq::like: where_clause.append(" LIKE ?"); break;
                case orm::wq::llike: where_clause.append(" LIKE ?"); break;
                case orm::wq::rlike: where_clause.append(" LIKE ?"); break;
                case orm::wq::nlike: where_clause.append(" NOT LIKE ?"); break;
                case orm::wq::in:
                case orm::wq::notin:
                {
                    where_clause.append(item.op_type == orm::wq::in ? " IN (" : " NOT IN (");
                    if (item.filed_value.is_array())
                    {
                        for (size_t a = 0; a < item.filed_value.size(); ++a)
                        {
                            if (a > 0)
                                where_clause.append(", ");
                            where_clause.append("?");
                            params.push_back(item.filed_value[a]);
                        }
                    }
                    else
                    {
                        where_clause.append("?");
                        params.push_back(item.filed_value);
                    }
                    where_clause.append(")");
                    break;
                }
                case orm::wq::isnull: where_clause.append(" IS NULL"); break;
                case orm::wq::notnull: where_clause.append(" IS NOT NULL"); break;
                default:
#ifdef _ORM_DEBUG
                    std::cerr << "[orm] mysql build_prepared_where: unknown op_type=" << int(item.op_type) << std::endl;
#endif
                    break;
                }
                if (item.op_type != orm::wq::isnull && item.op_type != orm::wq::notnull && item.op_type != orm::wq::in && item.op_type != orm::wq::notin)
                    params.push_back(prepared_like_bind(item));
            }

            // 兼容旧版 async_update/fetch：wheresql 为空时，若 pk > 0 自动用 pk 做 WHERE
            if (wheresql.empty() && B_BASE::getPK() > 0)
            {
                if (!where_clause.empty())
                    where_clause.append(" AND ");
                append_field(B_BASE::getPKname());
                where_clause.append(" = ?");
                params.push_back(http::obj_val(B_BASE::getPK()));
            }
        }

        // --- 构建完整 SELECT SQL + 参数 ---
        std::string build_prepared_select(std::vector<http::obj_val> &params, bool limit_one = false)
        {
            std::string where_clause;
            build_prepared_where(where_clause, params);

            parse_leftjion();

            std::string sql;
            if (selectsql.empty())
                sql = "SELECT * FROM ";
            else
            {
                sql = "SELECT ";
                sql.append(selectsql);
                sql.append(" FROM ");
            }
            sql.append(B_BASE::tablename);
            get_join_table(sql);
            if (!where_clause.empty())
            {
                sql.append(" WHERE ");
                sql.append(where_clause);
            }
            sql.append(groupsql);
            sql.append(ordersql);
            if (limit_one)
                sql.append(" LIMIT 1");
            else
                sql.append(limitsql);
            return sql;
        }

        // --- 获取 select_conn（预编译版复用现有连接池逻辑）---
        auto _get_prepared_select_conn()
        {
            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = conn_obj->get_mysql_select_conn();
            }
            else
            {
                select_conn = conn_obj->get_mysql_select_conn();
            }
            return select_conn;
        }

        auto _get_prepared_edit_conn()
        {
            if (islock_conn)
            {
                if (!edit_conn || edit_conn->isclose)
                    edit_conn = conn_obj->get_mysql_edit_conn();
            }
            else
            {
                edit_conn = conn_obj->get_mysql_edit_conn();
            }
            return edit_conn;
        }

      public:
        // ===== SELECT 类（参考 fetch_one / fetch_to / fetch_append 命名）=====

        // --- exec_one：预编译 SELECT LIMIT 1，结果写入 this->data ---
        unsigned int exec_one()
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            B_BASE::data_reset();
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            auto col_pos_map  = std::vector<unsigned char>();
            bool first_row    = true;
            unsigned int rows = conn->fetch_prepared(sql, params, [this, conn, &col_pos_map, &first_row](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                                                         if (first_row)
                                                         {
                                                             col_pos_map.assign(col_count, 255);
                                                             const auto &org_names = conn->prepared_col_org_names();
                                                             for (int ii = 0; ii < col_count; ii++)
                                                             {
                                                                 if (col_names[ii] && col_names[ii][0] != 0x00)
                                                                     col_pos_map[ii] = B_BASE::findcolpos(col_names[ii]);
                                                                 // 显示名是别名/表达式名，本表无同名字段时用原始(物理)列名兜底
                                                                 if (col_pos_map[ii] == 255 && ii < static_cast<int>(org_names.size()))
                                                                     col_pos_map[ii] = B_BASE::findcolpos(org_names[ii]);
                                                             }
                                                             first_row = false;
                                                         }
                                                         for (int ij = 0; ij < col_count; ij++)
                                                         {
                                                             auto [ptr, len] = get_data(ij);
                                                             if (ptr == nullptr)
                                                             {
                                                                 static const unsigned char null_value = 0;
                                                                 assign_field_value(col_pos_map[ij], (unsigned char *)&null_value, 0, B_BASE::data);
                                                                 continue;
                                                             }
                                                             assign_field_value(col_pos_map[ij], ptr, len, B_BASE::data);
                                                         }
                                                         return false;// LIMIT 1，一行后停止
                                                     });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = rows;
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_fetch_append：预编译 SELECT，追加到 record ---
        unsigned int exec_fetch_append()
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            // 追加语义：不清空 record（exec_fetch_to 会先自行 record_reset 再调用本函数）
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            // 先空跑一次拿到列名映射（fetch_prepared 内部会缓存列信息）
            auto col_pos_map = std::vector<unsigned char>();
            bool first_row   = true;

            unsigned int rows = conn->fetch_prepared(sql, params, [this, conn, &col_pos_map, &first_row](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                    if (first_row)
                    {
                        col_pos_map.assign(col_count, 255);
                        const auto &org_names = conn->prepared_col_org_names();
                        for (int ii = 0; ii < col_count; ii++)
                        {
                            if (col_names[ii] && col_names[ii][0] != 0x00)
                                col_pos_map[ii] = B_BASE::findcolpos(col_names[ii]);
                            // 显示名是别名/表达式名，本表无同名字段时用原始(物理)列名兜底
                            if (col_pos_map[ii] == 255 && ii < static_cast<int>(org_names.size()))
                                col_pos_map[ii] = B_BASE::findcolpos(org_names[ii]);
                        }
                        first_row = false;
                    }
                    xtask_info::meta data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr)
                        {
                            static const unsigned char null_value = 0;
                            assign_field_value(col_pos_map[ij], (unsigned char *)&null_value, 0, data_temp);
                            continue;
                        }
                        assign_field_value(col_pos_map[ij], ptr, len, data_temp);
                    }
                    B_BASE::record.emplace_back(std::move(data_temp));
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            // effect_num 反映追加后 record 的总行数（exec_fetch_to 因先 reset，总数即本次抓取数）
            effect_num = static_cast<unsigned int>(B_BASE::record.size());
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_fetch_to：预编译 SELECT，覆盖 record（不追加）---
        unsigned int exec_fetch()
        {
            B_BASE::record_reset();
            return exec_fetch_append();
        }
        unsigned int exec_fetch_to()
        {
            return exec_fetch();
        }

        // ===== SELECT 类：自定义结构体模板版（参照 fetch_to / fetch_one_to）=====

        // --- exec_fetch_to(cb)：预编译 SELECT，覆盖写入自定义 vector<T>（多行，RecordLineCallback<T> 版）---
        template <typename T, RecordLineCallback<T> Callback>
        unsigned int exec_fetch_to(std::vector<T> &custom_record, Callback &&callback)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            custom_record.clear();
            effect_num = 0;
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = conn->fetch_prepared(sql, params, [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            std::invoke(std::forward<Callback>(callback), data_temp, col_name, ptr, len, 0, 1);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = rows;
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_fetch_to(set_val)：预编译 SELECT，覆盖写入自定义 vector<T>（多行，ResultHasSetVal 版）---
        template <ResultHasSetVal T>
        unsigned int exec_fetch_to(std::vector<T> &custom_record)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            custom_record.clear();
            effect_num = 0;
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = conn->fetch_prepared(sql, params, [this, &custom_record](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            data_temp.set_val(col_name, ptr, len, 0);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = rows;
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_fetch_append(cb)：预编译 SELECT，追加到自定义 vector<T>（多行，RecordLineCallback<T> 版）---
        template <typename T, RecordLineCallback<T> Callback>
        unsigned int exec_fetch_append(std::vector<T> &custom_record, Callback &&callback)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            // 追加语义：不清空 custom_record
            effect_num = 0;
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = conn->fetch_prepared(sql, params, [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            std::invoke(std::forward<Callback>(callback), data_temp, col_name, ptr, len, 0, 1);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = static_cast<unsigned int>(custom_record.size());
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_fetch_append(set_val)：预编译 SELECT，追加到自定义 vector<T>（多行，ResultHasSetVal 版）---
        template <ResultHasSetVal T>
        unsigned int exec_fetch_append(std::vector<T> &custom_record)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            // 追加语义：不清空 custom_record
            effect_num = 0;
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = conn->fetch_prepared(sql, params, [this, &custom_record](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            data_temp.set_val(col_name, ptr, len, 0);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = static_cast<unsigned int>(custom_record.size());
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_one_to(cb)：预编译 SELECT LIMIT 1，写入自定义 T（单行，RecordLineCallback<T> 版）---
        template <typename T, RecordLineCallback<T> Callback>
        unsigned int exec_one_to(T &custom_record, Callback &&callback)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = conn->fetch_prepared(sql, params, [&custom_record, &callback](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                                                         for (int ij = 0; ij < col_count; ij++)
                                                         {
                                                             auto [ptr, len] = get_data(ij);
                                                             if (ptr == nullptr)
                                                                 continue;
                                                             std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                             if (!col_name.empty())
                                                             {
                                                                 std::invoke(std::forward<Callback>(callback), custom_record, col_name, ptr, len, 0, 1);
                                                             }
                                                         }
                                                         return false;// LIMIT 1，一行后停止
                                                     });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = rows;
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_one_to(set_val)：预编译 SELECT LIMIT 1，写入自定义 T（单行，ResultHasSetVal 版）---
        template <ResultHasSetVal T>
        unsigned int exec_one_to(T &custom_record)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = conn->fetch_prepared(sql, params, [&custom_record](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                                                         for (int ij = 0; ij < col_count; ij++)
                                                         {
                                                             auto [ptr, len] = get_data(ij);
                                                             if (ptr == nullptr)
                                                                 continue;
                                                             std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                             if (!col_name.empty())
                                                             {
                                                                 custom_record.set_val(col_name, ptr, len, 0);
                                                             }
                                                         }
                                                         return false;// LIMIT 1，一行后停止
                                                     });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = rows;
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_one_append(cb)：预编译 SELECT LIMIT 1，追加到自定义 vector<T>（单行追加，RecordLineCallback<T> 版）---
        template <typename T, RecordLineCallback<T> Callback>
        unsigned int exec_one_append(std::vector<T> &custom_record, Callback &&callback)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = conn->fetch_prepared(sql, params, [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                                                         T data_temp;
                                                         for (int ij = 0; ij < col_count; ij++)
                                                         {
                                                             auto [ptr, len] = get_data(ij);
                                                             if (ptr == nullptr)
                                                                 continue;
                                                             std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                             if (!col_name.empty())
                                                             {
                                                                 std::invoke(std::forward<Callback>(callback), data_temp, col_name, ptr, len, 0, 1);
                                                             }
                                                         }
                                                         custom_record.emplace_back(std::move(data_temp));
                                                         effect_num++;
                                                         return false;// LIMIT 1，一行后停止
                                                     });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = static_cast<unsigned int>(custom_record.size());
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // --- exec_one_append(set_val)：预编译 SELECT LIMIT 1，追加到自定义 vector<T>（单行追加，ResultHasSetVal 版）---
        template <ResultHasSetVal T>
        unsigned int exec_one_append(std::vector<T> &custom_record)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                return 0;
            }
            if (conn_empty())
            {
                return 0;
            }

            auto conn = _get_prepared_select_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = conn->fetch_prepared(sql, params, [this, &custom_record](int col_count, char **col_names, auto get_data) mutable -> bool
                                                     {
                                                         T data_temp;
                                                         for (int ij = 0; ij < col_count; ij++)
                                                         {
                                                             auto [ptr, len] = get_data(ij);
                                                             if (ptr == nullptr)
                                                                 continue;
                                                             std::string col_name = col_names[ij] ? col_names[ij] : "";
                                                             if (!col_name.empty())
                                                             {
                                                                 data_temp.set_val(col_name, ptr, len, 0);
                                                             }
                                                         }
                                                         custom_record.emplace_back(std::move(data_temp));
                                                         effect_num++;
                                                         return false;// LIMIT 1，一行后停止
                                                     });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            effect_num = static_cast<unsigned int>(custom_record.size());
            if (!islock_conn)
            {
                conn_obj->back_mysql_select_conn(std::move(conn));
            }
            return rows;
        }

        // ===== SELECT 类：异步协程版 =====

        // --- async_exec_one：预编译 SELECT LIMIT 1 异步版 ---
        asio::awaitable<unsigned int> async_exec_one()
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            B_BASE::data_reset();
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            auto col_pos_map  = std::vector<unsigned char>();
            bool first_row    = true;
            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [this, conn, &col_pos_map, &first_row](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    if (first_row)
                    {
                        col_pos_map.assign(col_count, 255);
                        const auto &org_names = conn->prepared_col_org_names();
                        for (int ii = 0; ii < col_count; ii++)
                        {
                            if (col_names[ii] && col_names[ii][0] != 0x00)
                                col_pos_map[ii] = B_BASE::findcolpos(col_names[ii]);
                            // 显示名是别名/表达式名，本表无同名字段时用原始(物理)列名兜底 mysql特有
                            if (col_pos_map[ii] == 255 && ii < static_cast<int>(org_names.size()))
                                col_pos_map[ii] = B_BASE::findcolpos(org_names[ii]);
                        }
                        first_row = false;
                    }
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr)
                        {
                            static const unsigned char null_value = 0;
                            assign_field_value(col_pos_map[ij], (unsigned char *)&null_value, 0, B_BASE::data);
                            continue;
                        }
                        assign_field_value(col_pos_map[ij], ptr, len, B_BASE::data);
                    }
                    return false; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = rows;
            co_return rows;
        }

        // --- async_exec_fetch_append：预编译 SELECT 异步版（追加到 record）---
        asio::awaitable<unsigned int> async_exec_fetch_append()
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            auto col_pos_map  = std::vector<unsigned char>();
            bool first_row    = true;
            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [this, conn, &col_pos_map, &first_row](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    if (first_row)
                    {
                        col_pos_map.assign(col_count, 255);
                        const auto &org_names = conn->prepared_col_org_names();
                        for (int ii = 0; ii < col_count; ii++)
                        {
                            if (col_names[ii] && col_names[ii][0] != 0x00)
                                col_pos_map[ii] = B_BASE::findcolpos(col_names[ii]);
                            // 显示名是别名/表达式名，本表无同名字段时用原始(物理)列名兜底
                            if (col_pos_map[ii] == 255 && ii < static_cast<int>(org_names.size()))
                                col_pos_map[ii] = B_BASE::findcolpos(org_names[ii]);
                        }
                        first_row = false;
                    }
                    xtask_info::meta data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr)
                        {
                            static const unsigned char null_value = 0;
                            assign_field_value(col_pos_map[ij], (unsigned char *)&null_value, 0, data_temp);
                            continue;
                        }
                        assign_field_value(col_pos_map[ij], ptr, len, data_temp);
                    }
                    B_BASE::record.emplace_back(std::move(data_temp));
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = static_cast<unsigned int>(B_BASE::record.size());
            co_return rows;
        }

        // --- async_exec_fetch / async_exec_fetch_to：预编译 SELECT 异步版（覆盖 record）---
        asio::awaitable<unsigned int> async_exec_fetch()
        {
            B_BASE::record_reset();
            co_return co_await async_exec_fetch_append();
        }
        asio::awaitable<unsigned int> async_exec_fetch_to() { co_return co_await async_exec_fetch(); }

        // ===== SELECT 异步模板版 =====

        // --- async_exec_fetch_to(cb) ---
        template <typename T, RecordLineCallback<T> Callback>
        asio::awaitable<unsigned int> async_exec_fetch_to(std::vector<T> &custom_record, Callback &&callback)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            custom_record.clear();
            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            std::invoke(std::forward<Callback>(callback), data_temp, col_name, ptr, len, 0, 1);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = rows;
            co_return rows;
        }

        // --- async_exec_fetch_to(set_val) ---
        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_exec_fetch_to(std::vector<T> &custom_record)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            custom_record.clear();
            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [this, &custom_record](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            data_temp.set_val(col_name, ptr, len, 0);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = rows;
            co_return rows;
        }

        // --- async_exec_fetch_append(cb) ---
        template <typename T, RecordLineCallback<T> Callback>
        asio::awaitable<unsigned int> async_exec_fetch_append(std::vector<T> &custom_record, Callback &&callback)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            std::invoke(std::forward<Callback>(callback), data_temp, col_name, ptr, len, 0, 1);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = static_cast<unsigned int>(custom_record.size());
            co_return rows;
        }

        // --- async_exec_fetch_append(set_val) ---
        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_exec_fetch_append(std::vector<T> &custom_record)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [this, &custom_record](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            data_temp.set_val(col_name, ptr, len, 0);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return true; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = static_cast<unsigned int>(custom_record.size());
            co_return rows;
        }

        // --- async_exec_one_to(cb) ---
        template <typename T, RecordLineCallback<T> Callback>
        asio::awaitable<unsigned int> async_exec_one_to(T &custom_record, Callback &&callback)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [&custom_record, &callback](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            std::invoke(std::forward<Callback>(callback), custom_record, col_name, ptr, len, 0, 1);
                        }
                    }
                    return false; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = rows;
            co_return rows;
        }

        // --- async_exec_one_to(set_val) ---
        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_exec_one_to(T &custom_record)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [&custom_record](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            custom_record.set_val(col_name, ptr, len, 0);
                        }
                    }
                    return false; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = rows;
            co_return rows;
        }

        // --- async_exec_one_append(cb) ---
        template <typename T, RecordLineCallback<T> Callback>
        asio::awaitable<unsigned int> async_exec_one_append(std::vector<T> &custom_record, Callback &&callback)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [this, &custom_record, &callback](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            std::invoke(std::forward<Callback>(callback), data_temp, col_name, ptr, len, 0, 1);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return false; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = static_cast<unsigned int>(custom_record.size());
            co_return rows;
        }

        // --- async_exec_one_append(set_val) ---
        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_exec_one_append(std::vector<T> &custom_record)
        {
            std::vector<http::obj_val> params;
            std::string sql = build_prepared_select(params, true);
            sqlstring       = sql;

            effect_num = 0;
            if (iserror)
            {
                co_return 0;
            }
            if (conn_empty())
            {
                co_return 0;
            }

            if (islock_conn)
            {
                if (!select_conn || select_conn->isclose)
                    select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            else
            {
                select_conn = co_await conn_obj->async_get_mysql_select_conn();
            }
            auto conn = select_conn;
            if (conn->isdebug)
                conn->begin_time();

            unsigned int rows = co_await conn->async_fetch_prepared(sql, params, [this, &custom_record](int col_count, char **col_names, auto get_data) mutable -> bool
                                                                    {
                    T data_temp;
                    for (int ij = 0; ij < col_count; ij++)
                    {
                        auto [ptr, len] = get_data(ij);
                        if (ptr == nullptr) continue;
                        std::string col_name = col_names[ij] ? col_names[ij] : "";
                        if (!col_name.empty())
                        {
                            data_temp.set_val(col_name, ptr, len, 0);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
                    return false; });

            if (conn->isdebug)
                conn->finish_time();
            if (rows == 0 && !conn->error_msg.empty())
            {
                iserror   = true;
                error_msg = conn->error_msg;
            }
            if (!islock_conn)
                conn_obj->back_mysql_select_conn(std::move(select_conn));
            effect_num = static_cast<unsigned int>(custom_record.size());
            co_return rows;
        }

        // ===== DML 类 =====

        // --- _split_fields_csv：把 "字段1,字段2" 切成字段列表（去首尾空格、跳过空段）---
        // 供 exec_update(fields_csv) / async_exec_update(fields_csv) 共用，避免两处解析逻辑漂移
        static std::vector<std::string_view> _split_fields_csv(std::string_view fields_csv)
        {
            std::vector<std::string_view> fields;
            std::string::size_type start = 0;
            for (auto pos = fields_csv.find(','); pos != std::string_view::npos; pos = fields_csv.find(',', start))
            {
                auto f = fields_csv.substr(start, pos - start);
                while (!f.empty() && f.front() == ' ')
                    f.remove_prefix(1);
                while (!f.empty() && f.back() == ' ')
                    f.remove_suffix(1);
                if (!f.empty())
                    fields.push_back(f);
                start = pos + 1;
            }
            auto last = fields_csv.substr(start);
            while (!last.empty() && last.front() == ' ')
                last.remove_prefix(1);
            while (!last.empty() && last.back() == ' ')
                last.remove_suffix(1);
            if (!last.empty())
                fields.push_back(last);
            return fields;
        }

        // --- exec_update：预编译 UPDATE，全量 SET = 从 this->data 取 ---
        // 依赖 paozhu_cli 生成 get_field_value(unsigned char idx, const meta &data) -> http::obj_val
        unsigned int exec_update()
        {
            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");

            std::vector<http::obj_val> params;
            bool first = true;
            for (unsigned char i = 0; i < xtask_info::col_names.size(); i++)
            {
                // 跳过自增主键：全字段更新不修改身份列（“自增主键跳过”约定），避免把主键误改为 0
                if (static_cast<int>(i) == xtask_info::auto_pk_index)
                    continue;
                if (!first)
                    sql.append(", ");
                sql.append(xtask_info::col_names[i]);
                sql.append(" = ?");
                params.push_back(get_field_value(i, B_BASE::data));
                first = false;
            }
            if (first)
            {
                sqlstring = "exec_update: no columns";
                iserror   = true;
                return (unsigned int)-1;
            }

            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 UPDATE 会把整表所有行改成同一组值。条件来源只有 wheresql 与 pk，
            // 两者皆无时拒绝发出语句（与 exec_remove 的守卫同构）。
            if (where_clause.empty())
            {
                sqlstring = "exec_update: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                return (unsigned int)-1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                return (unsigned int)-1;
            }
            if (conn_empty())
            {
                return (unsigned int)-1;
            }

            auto conn             = _get_prepared_edit_conn();
            unsigned int affected = conn->exec_dml_prepared(sql, params);
            effect_num            = affected;
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = conn->error_msg;
                iserror   = true;
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_edit_conn(std::move(conn));
            }
            return affected;
        }

        // --- async_exec_update：异步预编译 UPDATE，全量 SET = 从 this->data 取 ---
        // 与同步 exec_update() 同轨（跳过自增主键、缺 WHERE 拒绝执行），仅取连接与执行改为协程
        asio::awaitable<unsigned int> async_exec_update()
        {
            effect_num = 0;

            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");

            std::vector<http::obj_val> params;
            bool first = true;
            for (unsigned char i = 0; i < xtask_info::col_names.size(); i++)
            {
                // 跳过自增主键：全字段更新不修改身份列（“自增主键跳过”约定），避免把主键误改为 0
                if (static_cast<int>(i) == xtask_info::auto_pk_index)
                    continue;
                if (!first)
                    sql.append(", ");
                sql.append(xtask_info::col_names[i]);
                sql.append(" = ?");
                params.push_back(get_field_value(i, B_BASE::data));
                first = false;
            }
            if (first)
            {
                sqlstring = "async_exec_update: no columns";
                iserror   = true;
                co_return (unsigned int) - 1;
            }

            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 UPDATE 会把整表所有行改成同一组值。条件来源只有 wheresql 与 pk，
            // 两者皆无时拒绝发出语句（与 async_exec_remove 的守卫同构）。
            if (where_clause.empty())
            {
                sqlstring = "async_exec_update: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                co_return (unsigned int) - 1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                co_return (unsigned int) - 1;
            }
            if (conn_empty())
            {
                co_return (unsigned int) - 1;
            }

            try
            {
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml_prepared(sql, params);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return (unsigned int) - 1;
                }

                effect_num = affected;
                if (!islock_conn)
                {
                    conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                }
                co_return affected;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }
            co_return (unsigned int) - 1;
        }

        // --- exec_update_dirty：预编译仅更新脏字段 ---
        unsigned int exec_update_dirty()
        {
            effect_num = 0;

            // 1. 拿脏 idx（源头已禁 auto_pk 标脏，无需额外过滤）
            auto dirty_indices = B_BASE::get_dirty_indices();
            if (dirty_indices.empty())
            {
                sqlstring = "exec_update_dirty: no dirty fields";
                iserror   = true;
                return (unsigned int)-1;
            }

            // 2. 拼 prepared SQL + params（直接用 idx，跳过 findcolpos）
            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");

            std::vector<http::obj_val> params;
            bool first = true;
            for (auto idx : dirty_indices)
            {
                if (!first)
                    sql.append(", ");
                sql.append(xtask_info::col_names[idx]);
                sql.append(" = ?");
                params.push_back(get_field_value(idx, B_BASE::data));
                first = false;
            }

            // 3. WHERE（复用 exec_update 的 build_prepared_where）
            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 UPDATE 会把整表所有行改成同一组值。条件来源只有 wheresql 与 pk，
            // 两者皆无时拒绝发出语句（与 exec_remove 的守卫同构）。
            if (where_clause.empty())
            {
                sqlstring = "exec_update_dirty: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                return (unsigned int)-1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                return (unsigned int)-1;
            }
            if (conn_empty())
            {
                return (unsigned int)-1;
            }

            auto conn             = _get_prepared_edit_conn();
            unsigned int affected = conn->exec_dml_prepared(sql, params);
            effect_num            = affected;
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = conn->error_msg;
                iserror   = true;
            }
            if (affected != static_cast<unsigned int>(-1))
                B_BASE::clear_dirty();
            if (!islock_conn)
            {
                conn_obj->back_mysql_edit_conn(std::move(conn));
            }
            return affected;
        }

        // --- async_exec_update_dirty：异步预编译仅更新脏字段 ---
        asio::awaitable<unsigned int> async_exec_update_dirty()
        {
            effect_num = 0;

            // 1. 拿脏 idx
            auto dirty_indices = B_BASE::get_dirty_indices();
            if (dirty_indices.empty())
            {
                sqlstring = "async_exec_update_dirty: no dirty fields";
                iserror   = true;
                co_return (unsigned int) - 1;
            }

            // 2. 拼 prepared SQL + params
            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");

            std::vector<http::obj_val> params;
            bool first = true;
            for (auto idx : dirty_indices)
            {
                if (!first)
                    sql.append(", ");
                sql.append(xtask_info::col_names[idx]);
                sql.append(" = ?");
                params.push_back(get_field_value(idx, B_BASE::data));
                first = false;
            }

            // 3. WHERE
            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 UPDATE 会把整表所有行改成同一组值。条件来源只有 wheresql 与 pk，
            // 两者皆无时拒绝发出语句（与 exec_remove 的守卫同构）。
            if (where_clause.empty())
            {
                sqlstring = "async_exec_update_dirty: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                co_return (unsigned int) - 1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                co_return (unsigned int) - 1;
            }
            if (conn_empty())
            {
                co_return (unsigned int) - 1;
            }

            // 4. 异步拿 conn + 执行预编译
            if (islock_conn)
            {
                if (!edit_conn || edit_conn->isclose)
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }
            }
            else
            {
                edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
            }

            unsigned int affected = co_await edit_conn->async_exec_dml_prepared(sql, params);
            effect_num            = affected;
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = edit_conn->error_msg;
                iserror   = true;
            }
            if (affected != static_cast<unsigned int>(-1))
                B_BASE::clear_dirty();
            co_return affected;
        }

        // --- exec_update(fields)：指定字段更新，值从 this->data 取 ---
        // 参数格式: "字段1,字段2,字段3"（逗号分隔）
        unsigned int exec_update(std::string_view fields_csv)
        {
            auto fields = _split_fields_csv(fields_csv);
            if (fields.empty())
            {
                sqlstring = "exec_update: empty fields";
                iserror   = true;
                return (unsigned int)-1;
            }

            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");

            std::vector<http::obj_val> params;
            bool first = true;
            for (auto &f : fields)
            {
                if (!first)
                    sql.append(", ");
                sql.append(f);
                sql.append(" = ?");
                unsigned char idx = B_BASE::findcolpos(std::string(f));
                if (idx == 255)
                {
                    sqlstring = std::string("exec_update: field not found: ") + std::string(f);
                    iserror   = true;
                    return (unsigned int)-1;
                }
                params.push_back(get_field_value(idx, B_BASE::data));
                first = false;
            }

            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 UPDATE 会把整表所有行改成同一组值。条件来源只有 wheresql 与 pk，
            // 两者皆无时拒绝发出语句（与 exec_remove 的守卫同构）。
            if (where_clause.empty())
            {
                sqlstring = "exec_update: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                return (unsigned int)-1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                return (unsigned int)-1;
            }
            if (conn_empty())
            {
                return (unsigned int)-1;
            }

            auto conn             = _get_prepared_edit_conn();
            unsigned int affected = conn->exec_dml_prepared(sql, params);
            effect_num            = affected;
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = conn->error_msg;
                iserror   = true;
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_edit_conn(std::move(conn));
            }
            return affected;
        }

        // --- async_exec_update(fields)：异步预编译指定字段更新，值从 this->data 取 ---
        // 参数格式: "字段1,字段2,字段3"（逗号分隔）
        asio::awaitable<unsigned int> async_exec_update(std::string_view fields_csv)
        {
            effect_num = 0;

            auto fields = _split_fields_csv(fields_csv);
            if (fields.empty())
            {
                sqlstring = "async_exec_update: empty fields";
                iserror   = true;
                co_return (unsigned int) - 1;
            }

            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");

            std::vector<http::obj_val> params;
            bool first = true;
            for (auto &f : fields)
            {
                if (!first)
                    sql.append(", ");
                sql.append(f);
                sql.append(" = ?");
                unsigned char idx = B_BASE::findcolpos(std::string(f));
                if (idx == 255)
                {
                    sqlstring = std::string("async_exec_update: field not found: ") + std::string(f);
                    iserror   = true;
                    co_return (unsigned int) - 1;
                }
                params.push_back(get_field_value(idx, B_BASE::data));
                first = false;
            }

            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 UPDATE 会把整表所有行改成同一组值。条件来源只有 wheresql 与 pk，
            // 两者皆无时拒绝发出语句（与 async_exec_remove 的守卫同构）。
            if (where_clause.empty())
            {
                sqlstring = "async_exec_update: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                co_return (unsigned int) - 1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                co_return (unsigned int) - 1;
            }
            if (conn_empty())
            {
                co_return (unsigned int) - 1;
            }

            try
            {
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml_prepared(sql, params);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return (unsigned int) - 1;
                }

                effect_num = affected;
                if (!islock_conn)
                {
                    conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                }
                co_return affected;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }
            co_return (unsigned int) - 1;
        }

        // --- exec_update_fields：预编译 UPDATE，显式传入 SET 字段 ---
        unsigned int exec_update_fields(const std::vector<std::pair<std::string, http::obj_val>> &sets)
        {
            if (sets.empty())
            {
                sqlstring = "exec_update_fields: empty sets";
                iserror   = true;
                return (unsigned int)-1;
            }

            std::string sql = "UPDATE ";
            sql.append(B_BASE::tablename);
            sql.append(" SET ");
            std::vector<http::obj_val> params;
            for (size_t i = 0; i < sets.size(); i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(sets[i].first);
                sql.append(" = ?");
                params.push_back(sets[i].second);
            }

            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 UPDATE 会把整表所有行改成同一组值。条件来源只有 wheresql 与 pk，
            // 两者皆无时拒绝发出语句（与 exec_remove 的守卫同构）。
            if (where_clause.empty())
            {
                sqlstring = "exec_update_fields: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                return (unsigned int)-1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                return (unsigned int)-1;
            }
            if (conn_empty())
            {
                return (unsigned int)-1;
            }

            auto conn             = _get_prepared_edit_conn();
            unsigned int affected = conn->exec_dml_prepared(sql, params);
            effect_num            = affected;
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = conn->error_msg;
                iserror   = true;
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_edit_conn(std::move(conn));
            }
            return affected;
        }

        // --- exec_remove：预编译 DELETE ---
        unsigned int exec_remove()
        {
            std::string sql = "DELETE FROM ";
            sql.append(B_BASE::tablename);

            std::vector<http::obj_val> params;
            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 DELETE 会清空整表。条件来源只有 wheresql 与 pk，两者皆无时
            // 拒绝发出语句（与文本路径 build_remove_sql 的"返回空串"约定同构）。
            if (where_clause.empty())
            {
                sqlstring = "exec_remove: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                return (unsigned int)-1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                return (unsigned int)-1;
            }
            if (conn_empty())
            {
                return (unsigned int)-1;
            }

            auto conn             = _get_prepared_edit_conn();
            unsigned int affected = conn->exec_dml_prepared(sql, params);
            effect_num            = affected;
            if (affected == static_cast<unsigned int>(-1))
            {
                error_msg = conn->error_msg;
                iserror   = true;
            }
            if (!islock_conn)
            {
                conn_obj->back_mysql_edit_conn(std::move(conn));
            }
            return affected;
        }

        // --- async_exec_remove：异步预编译 DELETE ---
        // 与同步 exec_remove() 同轨：WHERE 来自 wheresql / pk，两者皆空则拒绝执行（防整表清空）
        asio::awaitable<unsigned int> async_exec_remove()
        {
            effect_num = 0;

            std::string sql = "DELETE FROM ";
            sql.append(B_BASE::tablename);

            std::vector<http::obj_val> params;
            std::string where_clause;
            build_prepared_where(where_clause, params);
            // 缺 WHERE 的 DELETE 会清空整表。条件来源只有 wheresql 与 pk，两者皆无时
            // 拒绝发出语句（与同步 exec_remove 的守卫同构）。
            if (where_clause.empty())
            {
                sqlstring = "async_exec_remove: wheresql is empty and the primary key is less than or equal to 0, lacking a WHERE condition，拒绝执行";
                error_msg = sqlstring;
                iserror   = true;
                co_return (unsigned int) - 1;
            }
            sql.append(" WHERE ");
            sql.append(where_clause);

            sqlstring = sql;
            if (iserror)
            {
                co_return (unsigned int) - 1;
            }
            if (conn_empty())
            {
                co_return (unsigned int) - 1;
            }

            try
            {
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml_prepared(sql, params);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return (unsigned int) - 1;
                }

                effect_num = affected;
                if (!islock_conn)
                {
                    conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                }
                co_return affected;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }
            co_return (unsigned int) - 1;
        }

        // --- exec_insert：预编译 INSERT，值从 this->data 全字段取 ---
        // 返回 tuple<effect_num, last_insert_id>
        std::tuple<unsigned int, unsigned long long> exec_insert()
        {
            std::string sql = "INSERT INTO ";
            sql.append(B_BASE::tablename);
            sql.append(" (");

            std::vector<http::obj_val> params;
            for (unsigned char i = 0; i < xtask_info::col_names.size(); i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(xtask_info::col_names[i]);
                params.push_back(get_insert_field_value(i, B_BASE::data));
            }
            sql.append(") VALUES (");
            for (unsigned char i = 0; i < xtask_info::col_names.size(); i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append("?");
            }
            sql.append(")");

            sqlstring = sql;
            if (iserror)
            {
                return std::make_tuple(0, 0ULL);
            }
            if (conn_empty())
            {
                return std::make_tuple(0, 0ULL);
            }

            auto conn = _get_prepared_edit_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int affected = conn->exec_dml_prepared(sql, params);

            if (conn->isdebug)
                conn->finish_time();

            long long last_id = 0;
            if (affected != static_cast<unsigned int>(-1))
            {
                last_id = conn->last_insert_id();
                B_BASE::setPK(last_id);
            }
            else
            {
                error_msg = conn->error_msg;
                iserror   = true;
            }

            effect_num = affected;
            if (!islock_conn)
            {
                conn_obj->back_mysql_edit_conn(std::move(conn));
            }
            return std::make_tuple(effect_num, static_cast<unsigned long long>(last_id));
        }

        // --- async_exec_insert：异步预编译 INSERT，值从 this->data 全字段取 ---
        // 返回 tuple<effect_num, last_insert_id>
        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_exec_insert()
        {
            effect_num      = 0;
            std::string sql = "INSERT INTO ";
            sql.append(B_BASE::tablename);
            sql.append(" (");
            std::vector<http::obj_val> params;
            for (unsigned char i = 0; i < xtask_info::col_names.size(); i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(xtask_info::col_names[i]);
                params.push_back(get_insert_field_value(i, B_BASE::data));
            }
            sql.append(") VALUES (");
            for (unsigned char i = 0; i < xtask_info::col_names.size(); i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append("?");
            }
            sql.append(")");

            sqlstring = sql;
            if (iserror)
            {
                co_return std::make_tuple(0, 0ULL);
            }
            if (conn_empty())
            {
                co_return std::make_tuple(0, 0ULL);
            }

            try
            {
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml_prepared(sql, params);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0ULL);
                }

                long long last_id = edit_conn->last_insert_id();
                B_BASE::setPK(last_id);
                effect_num = affected;
                if (!islock_conn)
                {
                    conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                }
                co_return std::make_tuple(effect_num, static_cast<unsigned long long>(last_id));
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }
            co_return std::make_tuple(0, 0ULL);
        }

        // ========================================================================
        // exec_insert_batch —— 预编译批量 INSERT
        // ------------------------------------------------------------------------
        // 数据源: this->record (std::vector<meta>) —— 每行 = 一个 meta 结构体
        // SQL:    INSERT INTO tablename (c1, c2, ...) VALUES (?, ?), (?, ?), ...
        // 返回:   tuple<影响行数, 第一个 last_insert_id>

        std::tuple<unsigned int, unsigned long long> exec_insert_batch()
        {
            if (B_BASE::record.empty())
            {
                sqlstring = "exec_insert_batch: record is empty";
                iserror   = true;
                return std::make_tuple(0, 0ULL);
            }

            const unsigned char ncols = xtask_info::col_names.size();
            const size_t nrows        = B_BASE::record.size();

            std::string sql = "INSERT INTO ";
            sql.append(B_BASE::tablename);
            sql.append(" (");
            for (unsigned char i = 0; i < ncols; i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(xtask_info::col_names[i]);
            }
            sql.append(") VALUES ");

            std::vector<http::obj_val> params;
            params.reserve(ncols * nrows);
            for (size_t r = 0; r < nrows; r++)
            {
                if (r > 0)
                    sql.append(", ");
                sql.append("(");
                for (unsigned char c = 0; c < ncols; c++)
                {
                    if (c > 0)
                        sql.append(", ");
                    sql.append("?");
                    params.push_back(get_insert_field_value(c, B_BASE::record[r]));
                }
                sql.append(")");
            }

            sqlstring = sql;
            if (iserror)
            {
                return std::make_tuple(0, 0ULL);
            }
            if (conn_empty())
            {
                return std::make_tuple(0, 0ULL);
            }

            auto conn = _get_prepared_edit_conn();
            if (conn->isdebug)
                conn->begin_time();

            unsigned int affected = conn->exec_dml_prepared(sql, params);

            if (conn->isdebug)
                conn->finish_time();

            long long first_id = 0;
            if (affected != static_cast<unsigned int>(-1))
            {
                first_id = conn->last_insert_id();
            }
            else
            {
                error_msg = conn->error_msg;
                iserror   = true;
            }

            effect_num = affected;
            if (!islock_conn)
            {
                conn_obj->back_mysql_edit_conn(std::move(conn));
            }
            return std::make_tuple(effect_num, static_cast<unsigned long long>(first_id));
        }

        // ========================================================================
        // async_exec_insert_batch —— 异步预编译批量 INSERT
        // ------------------------------------------------------------------------
        // 数据源: this->record (std::vector<meta>)；每行 = 一个 meta 结构体
        // SQL:    INSERT INTO tablename (c1, c2, ...) VALUES (?, ?), (?, ?), ...
        // 返回:   tuple<影响行数, 第一个 last_insert_id>
        // ========================================================================
        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_exec_insert_batch()
        {
            if (B_BASE::record.empty())
            {
                sqlstring = "async_exec_insert_batch: record is empty";
                iserror   = true;
                co_return std::make_tuple(0, 0ULL);
            }

            const unsigned char ncols = xtask_info::col_names.size();
            const size_t nrows        = B_BASE::record.size();

            std::string sql = "INSERT INTO ";
            sql.append(B_BASE::tablename);
            sql.append(" (");
            for (unsigned char i = 0; i < ncols; i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(xtask_info::col_names[i]);
            }
            sql.append(") VALUES ");

            std::vector<http::obj_val> params;
            params.reserve(ncols * nrows);
            for (size_t r = 0; r < nrows; r++)
            {
                if (r > 0)
                    sql.append(", ");
                sql.append("(");
                for (unsigned char c = 0; c < ncols; c++)
                {
                    if (c > 0)
                        sql.append(", ");
                    sql.append("?");
                    params.push_back(get_insert_field_value(c, B_BASE::record[r]));
                }
                sql.append(")");
            }

            sqlstring = sql;
            if (iserror)
            {
                co_return std::make_tuple(0, 0ULL);
            }
            if (conn_empty())
            {
                co_return std::make_tuple(0, 0ULL);
            }

            try
            {
                if (islock_conn)
                {
                    if (!edit_conn || edit_conn->isclose)
                    {
                        edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_mysql_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                unsigned int affected = co_await edit_conn->async_exec_dml_prepared(sql, params);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if (affected == static_cast<unsigned int>(-1))
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0ULL);
                }

                long long first_id = edit_conn->last_insert_id();
                effect_num         = affected;
                if (!islock_conn)
                {
                    conn_obj->back_mysql_edit_conn(std::move(edit_conn));
                }
                co_return std::make_tuple(effect_num, static_cast<unsigned long long>(first_id));
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                unlock_conn();
            }
            co_return std::make_tuple(0, 0ULL);
        }

      public:
            // no foreign keys

      public:
        std::string selectsql;
        std::string ordersql;
        std::string groupsql;
        std::string limitsql;
        std::string sqlstring;
        std::string dbtag;
        std::string error_msg;

        // std::list<std::string> commit_sqllist;
        bool iscache            = false;
        bool iserror            = false;
        bool islock_conn        = false;
        int exptime             = 0;
        unsigned int effect_num = 0;

        std::vector<orm_where_sql_t> wheresql;
        M_MODEL *mod;

        std::unique_ptr<orm::orm_left_join_t> join_ptr = nullptr;

        std::shared_ptr<mysql_conn_base> select_conn;
        std::shared_ptr<mysql_conn_base> edit_conn;
        std::shared_ptr<orm_conn_pool> conn_obj;

        static constexpr DB_TYPE db_type = DB_TYPE::MYSQL;
    };
} /*tagnamespace_replace*/
}// namespace orm
#endif
