#ifndef _ORM_CMS_SITEINFO_OPERATE_H
#define _ORM_CMS_SITEINFO_OPERATE_H
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
#include "siteinfo_base.h"
/*baseincludefile*/
namespace orm
{
// mysql Operational SQL middleware
namespace cms
{ /*tagnamespace_replace*/
    template <typename M_MODEL, typename B_BASE>
    class siteinfo_opsql : public B_BASE
    {
      public:
        siteinfo_opsql(const std::string &tag) : dbtag(tag)
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
        siteinfo_opsql() : dbtag(B_BASE::_rmstag)
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

        void assign_field_value(unsigned char index_pos, unsigned char *result_temp_data, unsigned long long value_size, siteinfo_info::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0:
             {
               data_temp.sid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.sid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.sid = 0;
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
               data_temp.agentid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.agentid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.agentid = 0;
                        }
            }
            break;
                case 3:
             {
               data_temp.languagetype = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.languagetype);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.languagetype = 0;
                        }
            }
            break;
                case 4:
            data_temp.sitename.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 5:
            data_temp.sitedomain.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 6:
            data_temp.metakeys.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 7:
            data_temp.metadesc.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 8:
            data_temp.copyright.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 9:
            data_temp.beiansn.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 10:
            data_temp.footscript.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 11:
            data_temp.headscript.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 12:
            data_temp.introduce.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 13:
            data_temp.sitelogo.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 14:
            data_temp.sitebanner.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 15:
            data_temp.contactman.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 16:
            data_temp.phone.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 17:
            data_temp.mobile.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 18:
            data_temp.email.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 19:
            data_temp.bankname.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 20:
            data_temp.banksn.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 21:
            data_temp.address.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 22:
            data_temp.zipnum.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 23:
            data_temp.taxsn.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 24:
            data_temp.companyname.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 25:
            data_temp.linkname.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 26:
            data_temp.linkmobile.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 27:
            data_temp.linkaddress.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 28:
            data_temp.theme.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 29:
            data_temp.sitepath.assign(reinterpret_cast<const char*>(result_temp_data), value_size);
            break;
                case 30:
             {
               data_temp.isopen = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.isopen);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.isopen = 0;
                        }
            }
            break;
                case 31:
             {
               data_temp.created_at = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.created_at);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.created_at = 0;
                        }
            }
            break;
                case 32:
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
                
        }
    }
    
        void assign_field_value_binary(unsigned char index_pos, col_value_variant val, siteinfo_info::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.sid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.sid = static_cast<decltype(data_temp.sid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.sid = static_cast<decltype(data_temp.sid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.sid = static_cast<decltype(data_temp.sid )>(v);
                    else
                        data_temp.sid = 0;
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
                        data_temp.agentid = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.agentid = static_cast<decltype(data_temp.agentid )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.agentid = static_cast<decltype(data_temp.agentid )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.agentid = static_cast<decltype(data_temp.agentid )>(v);
                    else
                        data_temp.agentid = 0;
                }, val);
            } break;
            case 3: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.languagetype = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.languagetype = static_cast<decltype(data_temp.languagetype )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.languagetype = static_cast<decltype(data_temp.languagetype )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.languagetype = static_cast<decltype(data_temp.languagetype )>(v);
                    else
                        data_temp.languagetype = 0;
                }, val);
            } break;
            case 4: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.sitename.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.sitename.assign(v.data(), v.size());
                    else
                        data_temp.sitename.clear();
                }, val);
            } break;
            case 5: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.sitedomain.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.sitedomain.assign(v.data(), v.size());
                    else
                        data_temp.sitedomain.clear();
                }, val);
            } break;
            case 6: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.metakeys.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.metakeys.assign(v.data(), v.size());
                    else
                        data_temp.metakeys.clear();
                }, val);
            } break;
            case 7: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.metadesc.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.metadesc.assign(v.data(), v.size());
                    else
                        data_temp.metadesc.clear();
                }, val);
            } break;
            case 8: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.copyright.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.copyright.assign(v.data(), v.size());
                    else
                        data_temp.copyright.clear();
                }, val);
            } break;
            case 9: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.beiansn.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.beiansn.assign(v.data(), v.size());
                    else
                        data_temp.beiansn.clear();
                }, val);
            } break;
            case 10: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.footscript.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.footscript.assign(v.data(), v.size());
                    else
                        data_temp.footscript.clear();
                }, val);
            } break;
            case 11: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.headscript.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.headscript.assign(v.data(), v.size());
                    else
                        data_temp.headscript.clear();
                }, val);
            } break;
            case 12: {
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
            case 13: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.sitelogo.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.sitelogo.assign(v.data(), v.size());
                    else
                        data_temp.sitelogo.clear();
                }, val);
            } break;
            case 14: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.sitebanner.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.sitebanner.assign(v.data(), v.size());
                    else
                        data_temp.sitebanner.clear();
                }, val);
            } break;
            case 15: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.contactman.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.contactman.assign(v.data(), v.size());
                    else
                        data_temp.contactman.clear();
                }, val);
            } break;
            case 16: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.phone.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.phone.assign(v.data(), v.size());
                    else
                        data_temp.phone.clear();
                }, val);
            } break;
            case 17: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.mobile.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.mobile.assign(v.data(), v.size());
                    else
                        data_temp.mobile.clear();
                }, val);
            } break;
            case 18: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.email.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.email.assign(v.data(), v.size());
                    else
                        data_temp.email.clear();
                }, val);
            } break;
            case 19: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.bankname.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.bankname.assign(v.data(), v.size());
                    else
                        data_temp.bankname.clear();
                }, val);
            } break;
            case 20: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.banksn.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.banksn.assign(v.data(), v.size());
                    else
                        data_temp.banksn.clear();
                }, val);
            } break;
            case 21: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.address.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.address.assign(v.data(), v.size());
                    else
                        data_temp.address.clear();
                }, val);
            } break;
            case 22: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.zipnum.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.zipnum.assign(v.data(), v.size());
                    else
                        data_temp.zipnum.clear();
                }, val);
            } break;
            case 23: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.taxsn.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.taxsn.assign(v.data(), v.size());
                    else
                        data_temp.taxsn.clear();
                }, val);
            } break;
            case 24: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.companyname.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.companyname.assign(v.data(), v.size());
                    else
                        data_temp.companyname.clear();
                }, val);
            } break;
            case 25: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.linkname.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.linkname.assign(v.data(), v.size());
                    else
                        data_temp.linkname.clear();
                }, val);
            } break;
            case 26: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.linkmobile.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.linkmobile.assign(v.data(), v.size());
                    else
                        data_temp.linkmobile.clear();
                }, val);
            } break;
            case 27: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.linkaddress.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.linkaddress.assign(v.data(), v.size());
                    else
                        data_temp.linkaddress.clear();
                }, val);
            } break;
            case 28: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.theme.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.theme.assign(v.data(), v.size());
                    else
                        data_temp.theme.clear();
                }, val);
            } break;
            case 29: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.sitepath.clear();
                    else if constexpr (std::is_same_v<T, std::string_view>)
                        data_temp.sitepath.assign(v.data(), v.size());
                    else
                        data_temp.sitepath.clear();
                }, val);
            } break;
            case 30: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.isopen = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.isopen = static_cast<decltype(data_temp.isopen )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.isopen = static_cast<decltype(data_temp.isopen )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.isopen = static_cast<decltype(data_temp.isopen )>(v);
                    else
                        data_temp.isopen = 0;
                }, val);
            } break;
            case 31: {
                std::visit([&](auto&& v){
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        data_temp.created_at = 0;
                    else if constexpr (std::is_same_v<T, int64_t>)
                        data_temp.created_at = static_cast<decltype(data_temp.created_at )>(v);
                    else if constexpr (std::is_same_v<T, uint64_t>)
                        data_temp.created_at = static_cast<decltype(data_temp.created_at )>(v);
                    else if constexpr (std::is_same_v<T, double>)
                        data_temp.created_at = static_cast<decltype(data_temp.created_at )>(v);
                    else
                        data_temp.created_at = 0;
                }, val);
            } break;
            case 32: {
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
        }
    }
    
        http::obj_val get_field_value(unsigned char index_pos, const siteinfo_info::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0:
                return http::obj_val(static_cast<long long>(data_temp.sid));
                break;
            case 1:
                return http::obj_val(static_cast<long long>(data_temp.userid));
                break;
            case 2:
                return http::obj_val(static_cast<long long>(data_temp.agentid));
                break;
            case 3:
                return http::obj_val(static_cast<long long>(data_temp.languagetype));
                break;
            case 4:
                return http::obj_val(std::string(data_temp.sitename));
                break;
            case 5:
                return http::obj_val(std::string(data_temp.sitedomain));
                break;
            case 6:
                return http::obj_val(std::string(data_temp.metakeys));
                break;
            case 7:
                return http::obj_val(std::string(data_temp.metadesc));
                break;
            case 8:
                return http::obj_val(std::string(data_temp.copyright));
                break;
            case 9:
                return http::obj_val(std::string(data_temp.beiansn));
                break;
            case 10:
                return http::obj_val(std::string(data_temp.footscript));
                break;
            case 11:
                return http::obj_val(std::string(data_temp.headscript));
                break;
            case 12:
                return http::obj_val(std::string(data_temp.introduce));
                break;
            case 13:
                return http::obj_val(std::string(data_temp.sitelogo));
                break;
            case 14:
                return http::obj_val(std::string(data_temp.sitebanner));
                break;
            case 15:
                return http::obj_val(std::string(data_temp.contactman));
                break;
            case 16:
                return http::obj_val(std::string(data_temp.phone));
                break;
            case 17:
                return http::obj_val(std::string(data_temp.mobile));
                break;
            case 18:
                return http::obj_val(std::string(data_temp.email));
                break;
            case 19:
                return http::obj_val(std::string(data_temp.bankname));
                break;
            case 20:
                return http::obj_val(std::string(data_temp.banksn));
                break;
            case 21:
                return http::obj_val(std::string(data_temp.address));
                break;
            case 22:
                return http::obj_val(std::string(data_temp.zipnum));
                break;
            case 23:
                return http::obj_val(std::string(data_temp.taxsn));
                break;
            case 24:
                return http::obj_val(std::string(data_temp.companyname));
                break;
            case 25:
                return http::obj_val(std::string(data_temp.linkname));
                break;
            case 26:
                return http::obj_val(std::string(data_temp.linkmobile));
                break;
            case 27:
                return http::obj_val(std::string(data_temp.linkaddress));
                break;
            case 28:
                return http::obj_val(std::string(data_temp.theme));
                break;
            case 29:
                return http::obj_val(std::string(data_temp.sitepath));
                break;
            case 30:
                return http::obj_val(static_cast<long long>(data_temp.isopen));
                break;
            case 31:
                return http::obj_val(static_cast<long long>(data_temp.created_at));
                break;
            case 32:
                return http::obj_val(static_cast<long long>(data_temp.enddate));
                break;
            default:
                return http::obj_val(nullptr);
        }
    }
    
        http::obj_val get_insert_field_value(unsigned char index_pos, const siteinfo_info::meta &data_temp)
    {
        // Auto-increment primary key sid == 0 bind NULL at runtime, and let the database automatically generate the primary key
        if (index_pos == 0 && data_temp.sid == 0)
            return http::obj_val(nullptr);
        return get_field_value(index_pos, data_temp);
    }
    

M_MODEL& eqSid(const std::string &val)
	{return where(B_BASE::cols::sid, orm::wq::eq, val);
	}

M_MODEL& nqSid(const std::string &val)
	{return where(B_BASE::cols::sid, orm::wq::nq, val);
	}

M_MODEL& btSid(const std::string &val)
	{return where(B_BASE::cols::sid, orm::wq::bt, val);
	}

M_MODEL& beSid(const std::string &val)
	{return where(B_BASE::cols::sid, orm::wq::be, val);
	}

M_MODEL& ltSid(const std::string &val)
	{return where(B_BASE::cols::sid, orm::wq::lt, val);
	}

M_MODEL& leSid(const std::string &val)
	{return where(B_BASE::cols::sid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqSid(T val)
	{return where(B_BASE::cols::sid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqSid(T val)
	{return where(B_BASE::cols::sid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btSid(T val)
	{return where(B_BASE::cols::sid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beSid(T val)
	{return where(B_BASE::cols::sid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltSid(T val)
	{return where(B_BASE::cols::sid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leSid(T val)
	{return where(B_BASE::cols::sid, orm::wq::le, val);
	}

M_MODEL& nullSid()
	{return whereNull(B_BASE::cols::sid);
	}

M_MODEL& notnullSid()
	{return whereNotNull(B_BASE::cols::sid);
	}

M_MODEL& oreqSid(const std::string &val)
	{return whereOr(B_BASE::cols::sid, orm::wq::eq, val);
	}

M_MODEL& ornqSid(const std::string &val)
	{return whereOr(B_BASE::cols::sid, orm::wq::nq, val);
	}

M_MODEL& orbtSid(const std::string &val)
	{return whereOr(B_BASE::cols::sid, orm::wq::bt, val);
	}

M_MODEL& orbeSid(const std::string &val)
	{return whereOr(B_BASE::cols::sid, orm::wq::be, val);
	}

M_MODEL& orltSid(const std::string &val)
	{return whereOr(B_BASE::cols::sid, orm::wq::lt, val);
	}

M_MODEL& orleSid(const std::string &val)
	{return whereOr(B_BASE::cols::sid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqSid(T val)
	{return whereOr(B_BASE::cols::sid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqSid(T val)
	{return whereOr(B_BASE::cols::sid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtSid(T val)
	{return whereOr(B_BASE::cols::sid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeSid(T val)
	{return whereOr(B_BASE::cols::sid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltSid(T val)
	{return whereOr(B_BASE::cols::sid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleSid(T val)
	{return whereOr(B_BASE::cols::sid, orm::wq::le, val);
	}

M_MODEL& ornullSid()
	{return whereOrNull(B_BASE::cols::sid);
	}

M_MODEL& ornotnullSid()
	{return whereOrNotNull(B_BASE::cols::sid);
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

M_MODEL& eqAgentid(const std::string &val)
	{return where(B_BASE::cols::agentid, orm::wq::eq, val);
	}

M_MODEL& nqAgentid(const std::string &val)
	{return where(B_BASE::cols::agentid, orm::wq::nq, val);
	}

M_MODEL& btAgentid(const std::string &val)
	{return where(B_BASE::cols::agentid, orm::wq::bt, val);
	}

M_MODEL& beAgentid(const std::string &val)
	{return where(B_BASE::cols::agentid, orm::wq::be, val);
	}

M_MODEL& ltAgentid(const std::string &val)
	{return where(B_BASE::cols::agentid, orm::wq::lt, val);
	}

M_MODEL& leAgentid(const std::string &val)
	{return where(B_BASE::cols::agentid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqAgentid(T val)
	{return where(B_BASE::cols::agentid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqAgentid(T val)
	{return where(B_BASE::cols::agentid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btAgentid(T val)
	{return where(B_BASE::cols::agentid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beAgentid(T val)
	{return where(B_BASE::cols::agentid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltAgentid(T val)
	{return where(B_BASE::cols::agentid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leAgentid(T val)
	{return where(B_BASE::cols::agentid, orm::wq::le, val);
	}

M_MODEL& nullAgentid()
	{return whereNull(B_BASE::cols::agentid);
	}

M_MODEL& notnullAgentid()
	{return whereNotNull(B_BASE::cols::agentid);
	}

M_MODEL& oreqAgentid(const std::string &val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::eq, val);
	}

M_MODEL& ornqAgentid(const std::string &val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::nq, val);
	}

M_MODEL& orbtAgentid(const std::string &val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::bt, val);
	}

M_MODEL& orbeAgentid(const std::string &val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::be, val);
	}

M_MODEL& orltAgentid(const std::string &val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::lt, val);
	}

M_MODEL& orleAgentid(const std::string &val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqAgentid(T val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqAgentid(T val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtAgentid(T val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeAgentid(T val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltAgentid(T val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleAgentid(T val)
	{return whereOr(B_BASE::cols::agentid, orm::wq::le, val);
	}

M_MODEL& ornullAgentid()
	{return whereOrNull(B_BASE::cols::agentid);
	}

M_MODEL& ornotnullAgentid()
	{return whereOrNotNull(B_BASE::cols::agentid);
	}

M_MODEL& eqLanguagetype(const std::string &val)
	{return where(B_BASE::cols::languagetype, orm::wq::eq, val);
	}

M_MODEL& nqLanguagetype(const std::string &val)
	{return where(B_BASE::cols::languagetype, orm::wq::nq, val);
	}

M_MODEL& btLanguagetype(const std::string &val)
	{return where(B_BASE::cols::languagetype, orm::wq::bt, val);
	}

M_MODEL& beLanguagetype(const std::string &val)
	{return where(B_BASE::cols::languagetype, orm::wq::be, val);
	}

M_MODEL& ltLanguagetype(const std::string &val)
	{return where(B_BASE::cols::languagetype, orm::wq::lt, val);
	}

M_MODEL& leLanguagetype(const std::string &val)
	{return where(B_BASE::cols::languagetype, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqLanguagetype(T val)
	{return where(B_BASE::cols::languagetype, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqLanguagetype(T val)
	{return where(B_BASE::cols::languagetype, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btLanguagetype(T val)
	{return where(B_BASE::cols::languagetype, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beLanguagetype(T val)
	{return where(B_BASE::cols::languagetype, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltLanguagetype(T val)
	{return where(B_BASE::cols::languagetype, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leLanguagetype(T val)
	{return where(B_BASE::cols::languagetype, orm::wq::le, val);
	}

M_MODEL& nullLanguagetype()
	{return whereNull(B_BASE::cols::languagetype);
	}

M_MODEL& notnullLanguagetype()
	{return whereNotNull(B_BASE::cols::languagetype);
	}

M_MODEL& oreqLanguagetype(const std::string &val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::eq, val);
	}

M_MODEL& ornqLanguagetype(const std::string &val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::nq, val);
	}

M_MODEL& orbtLanguagetype(const std::string &val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::bt, val);
	}

M_MODEL& orbeLanguagetype(const std::string &val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::be, val);
	}

M_MODEL& orltLanguagetype(const std::string &val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::lt, val);
	}

M_MODEL& orleLanguagetype(const std::string &val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqLanguagetype(T val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqLanguagetype(T val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtLanguagetype(T val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeLanguagetype(T val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltLanguagetype(T val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleLanguagetype(T val)
	{return whereOr(B_BASE::cols::languagetype, orm::wq::le, val);
	}

M_MODEL& ornullLanguagetype()
	{return whereOrNull(B_BASE::cols::languagetype);
	}

M_MODEL& ornotnullLanguagetype()
	{return whereOrNotNull(B_BASE::cols::languagetype);
	}

M_MODEL& eqSitename(const std::string &val)
	{return where(B_BASE::cols::sitename, orm::wq::eq, val);
	}

M_MODEL& nqSitename(const std::string &val)
	{return where(B_BASE::cols::sitename, orm::wq::nq, val);
	}

M_MODEL& btSitename(const std::string &val)
	{return where(B_BASE::cols::sitename, orm::wq::bt, val);
	}

M_MODEL& beSitename(const std::string &val)
	{return where(B_BASE::cols::sitename, orm::wq::be, val);
	}

M_MODEL& ltSitename(const std::string &val)
	{return where(B_BASE::cols::sitename, orm::wq::lt, val);
	}

M_MODEL& leSitename(const std::string &val)
	{return where(B_BASE::cols::sitename, orm::wq::le, val);
	}

M_MODEL& likeSitename(const std::string &val)
	{return where(B_BASE::cols::sitename, orm::wq::like, val);
	}

M_MODEL& nullSitename()
	{return whereNull(B_BASE::cols::sitename);
	}

M_MODEL& notnullSitename()
	{return whereNotNull(B_BASE::cols::sitename);
	}

M_MODEL& oreqSitename(const std::string &val)
	{return whereOr(B_BASE::cols::sitename, orm::wq::eq, val);
	}

M_MODEL& ornqSitename(const std::string &val)
	{return whereOr(B_BASE::cols::sitename, orm::wq::nq, val);
	}

M_MODEL& orbtSitename(const std::string &val)
	{return whereOr(B_BASE::cols::sitename, orm::wq::bt, val);
	}

M_MODEL& orbeSitename(const std::string &val)
	{return whereOr(B_BASE::cols::sitename, orm::wq::be, val);
	}

M_MODEL& orltSitename(const std::string &val)
	{return whereOr(B_BASE::cols::sitename, orm::wq::lt, val);
	}

M_MODEL& orleSitename(const std::string &val)
	{return whereOr(B_BASE::cols::sitename, orm::wq::le, val);
	}

M_MODEL& orlikeSitename(const std::string &val)
	{return whereOr(B_BASE::cols::sitename, orm::wq::like, val);
	}

M_MODEL& ornullSitename()
	{return whereOrNull(B_BASE::cols::sitename);
	}

M_MODEL& ornotnullSitename()
	{return whereOrNotNull(B_BASE::cols::sitename);
	}

M_MODEL& eqSitedomain(const std::string &val)
	{return where(B_BASE::cols::sitedomain, orm::wq::eq, val);
	}

M_MODEL& nqSitedomain(const std::string &val)
	{return where(B_BASE::cols::sitedomain, orm::wq::nq, val);
	}

M_MODEL& btSitedomain(const std::string &val)
	{return where(B_BASE::cols::sitedomain, orm::wq::bt, val);
	}

M_MODEL& beSitedomain(const std::string &val)
	{return where(B_BASE::cols::sitedomain, orm::wq::be, val);
	}

M_MODEL& ltSitedomain(const std::string &val)
	{return where(B_BASE::cols::sitedomain, orm::wq::lt, val);
	}

M_MODEL& leSitedomain(const std::string &val)
	{return where(B_BASE::cols::sitedomain, orm::wq::le, val);
	}

M_MODEL& likeSitedomain(const std::string &val)
	{return where(B_BASE::cols::sitedomain, orm::wq::like, val);
	}

M_MODEL& nullSitedomain()
	{return whereNull(B_BASE::cols::sitedomain);
	}

M_MODEL& notnullSitedomain()
	{return whereNotNull(B_BASE::cols::sitedomain);
	}

M_MODEL& oreqSitedomain(const std::string &val)
	{return whereOr(B_BASE::cols::sitedomain, orm::wq::eq, val);
	}

M_MODEL& ornqSitedomain(const std::string &val)
	{return whereOr(B_BASE::cols::sitedomain, orm::wq::nq, val);
	}

M_MODEL& orbtSitedomain(const std::string &val)
	{return whereOr(B_BASE::cols::sitedomain, orm::wq::bt, val);
	}

M_MODEL& orbeSitedomain(const std::string &val)
	{return whereOr(B_BASE::cols::sitedomain, orm::wq::be, val);
	}

M_MODEL& orltSitedomain(const std::string &val)
	{return whereOr(B_BASE::cols::sitedomain, orm::wq::lt, val);
	}

M_MODEL& orleSitedomain(const std::string &val)
	{return whereOr(B_BASE::cols::sitedomain, orm::wq::le, val);
	}

M_MODEL& orlikeSitedomain(const std::string &val)
	{return whereOr(B_BASE::cols::sitedomain, orm::wq::like, val);
	}

M_MODEL& ornullSitedomain()
	{return whereOrNull(B_BASE::cols::sitedomain);
	}

M_MODEL& ornotnullSitedomain()
	{return whereOrNotNull(B_BASE::cols::sitedomain);
	}

M_MODEL& eqMetakeys(const std::string &val)
	{return where(B_BASE::cols::metakeys, orm::wq::eq, val);
	}

M_MODEL& nqMetakeys(const std::string &val)
	{return where(B_BASE::cols::metakeys, orm::wq::nq, val);
	}

M_MODEL& btMetakeys(const std::string &val)
	{return where(B_BASE::cols::metakeys, orm::wq::bt, val);
	}

M_MODEL& beMetakeys(const std::string &val)
	{return where(B_BASE::cols::metakeys, orm::wq::be, val);
	}

M_MODEL& ltMetakeys(const std::string &val)
	{return where(B_BASE::cols::metakeys, orm::wq::lt, val);
	}

M_MODEL& leMetakeys(const std::string &val)
	{return where(B_BASE::cols::metakeys, orm::wq::le, val);
	}

M_MODEL& likeMetakeys(const std::string &val)
	{return where(B_BASE::cols::metakeys, orm::wq::like, val);
	}

M_MODEL& nullMetakeys()
	{return whereNull(B_BASE::cols::metakeys);
	}

M_MODEL& notnullMetakeys()
	{return whereNotNull(B_BASE::cols::metakeys);
	}

M_MODEL& oreqMetakeys(const std::string &val)
	{return whereOr(B_BASE::cols::metakeys, orm::wq::eq, val);
	}

M_MODEL& ornqMetakeys(const std::string &val)
	{return whereOr(B_BASE::cols::metakeys, orm::wq::nq, val);
	}

M_MODEL& orbtMetakeys(const std::string &val)
	{return whereOr(B_BASE::cols::metakeys, orm::wq::bt, val);
	}

M_MODEL& orbeMetakeys(const std::string &val)
	{return whereOr(B_BASE::cols::metakeys, orm::wq::be, val);
	}

M_MODEL& orltMetakeys(const std::string &val)
	{return whereOr(B_BASE::cols::metakeys, orm::wq::lt, val);
	}

M_MODEL& orleMetakeys(const std::string &val)
	{return whereOr(B_BASE::cols::metakeys, orm::wq::le, val);
	}

M_MODEL& orlikeMetakeys(const std::string &val)
	{return whereOr(B_BASE::cols::metakeys, orm::wq::like, val);
	}

M_MODEL& ornullMetakeys()
	{return whereOrNull(B_BASE::cols::metakeys);
	}

M_MODEL& ornotnullMetakeys()
	{return whereOrNotNull(B_BASE::cols::metakeys);
	}

M_MODEL& eqMetadesc(const std::string &val)
	{return where(B_BASE::cols::metadesc, orm::wq::eq, val);
	}

M_MODEL& nqMetadesc(const std::string &val)
	{return where(B_BASE::cols::metadesc, orm::wq::nq, val);
	}

M_MODEL& btMetadesc(const std::string &val)
	{return where(B_BASE::cols::metadesc, orm::wq::bt, val);
	}

M_MODEL& beMetadesc(const std::string &val)
	{return where(B_BASE::cols::metadesc, orm::wq::be, val);
	}

M_MODEL& ltMetadesc(const std::string &val)
	{return where(B_BASE::cols::metadesc, orm::wq::lt, val);
	}

M_MODEL& leMetadesc(const std::string &val)
	{return where(B_BASE::cols::metadesc, orm::wq::le, val);
	}

M_MODEL& likeMetadesc(const std::string &val)
	{return where(B_BASE::cols::metadesc, orm::wq::like, val);
	}

M_MODEL& nullMetadesc()
	{return whereNull(B_BASE::cols::metadesc);
	}

M_MODEL& notnullMetadesc()
	{return whereNotNull(B_BASE::cols::metadesc);
	}

M_MODEL& oreqMetadesc(const std::string &val)
	{return whereOr(B_BASE::cols::metadesc, orm::wq::eq, val);
	}

M_MODEL& ornqMetadesc(const std::string &val)
	{return whereOr(B_BASE::cols::metadesc, orm::wq::nq, val);
	}

M_MODEL& orbtMetadesc(const std::string &val)
	{return whereOr(B_BASE::cols::metadesc, orm::wq::bt, val);
	}

M_MODEL& orbeMetadesc(const std::string &val)
	{return whereOr(B_BASE::cols::metadesc, orm::wq::be, val);
	}

M_MODEL& orltMetadesc(const std::string &val)
	{return whereOr(B_BASE::cols::metadesc, orm::wq::lt, val);
	}

M_MODEL& orleMetadesc(const std::string &val)
	{return whereOr(B_BASE::cols::metadesc, orm::wq::le, val);
	}

M_MODEL& orlikeMetadesc(const std::string &val)
	{return whereOr(B_BASE::cols::metadesc, orm::wq::like, val);
	}

M_MODEL& ornullMetadesc()
	{return whereOrNull(B_BASE::cols::metadesc);
	}

M_MODEL& ornotnullMetadesc()
	{return whereOrNotNull(B_BASE::cols::metadesc);
	}

M_MODEL& eqCopyright(const std::string &val)
	{return where(B_BASE::cols::copyright, orm::wq::eq, val);
	}

M_MODEL& nqCopyright(const std::string &val)
	{return where(B_BASE::cols::copyright, orm::wq::nq, val);
	}

M_MODEL& btCopyright(const std::string &val)
	{return where(B_BASE::cols::copyright, orm::wq::bt, val);
	}

M_MODEL& beCopyright(const std::string &val)
	{return where(B_BASE::cols::copyright, orm::wq::be, val);
	}

M_MODEL& ltCopyright(const std::string &val)
	{return where(B_BASE::cols::copyright, orm::wq::lt, val);
	}

M_MODEL& leCopyright(const std::string &val)
	{return where(B_BASE::cols::copyright, orm::wq::le, val);
	}

M_MODEL& likeCopyright(const std::string &val)
	{return where(B_BASE::cols::copyright, orm::wq::like, val);
	}

M_MODEL& nullCopyright()
	{return whereNull(B_BASE::cols::copyright);
	}

M_MODEL& notnullCopyright()
	{return whereNotNull(B_BASE::cols::copyright);
	}

M_MODEL& oreqCopyright(const std::string &val)
	{return whereOr(B_BASE::cols::copyright, orm::wq::eq, val);
	}

M_MODEL& ornqCopyright(const std::string &val)
	{return whereOr(B_BASE::cols::copyright, orm::wq::nq, val);
	}

M_MODEL& orbtCopyright(const std::string &val)
	{return whereOr(B_BASE::cols::copyright, orm::wq::bt, val);
	}

M_MODEL& orbeCopyright(const std::string &val)
	{return whereOr(B_BASE::cols::copyright, orm::wq::be, val);
	}

M_MODEL& orltCopyright(const std::string &val)
	{return whereOr(B_BASE::cols::copyright, orm::wq::lt, val);
	}

M_MODEL& orleCopyright(const std::string &val)
	{return whereOr(B_BASE::cols::copyright, orm::wq::le, val);
	}

M_MODEL& orlikeCopyright(const std::string &val)
	{return whereOr(B_BASE::cols::copyright, orm::wq::like, val);
	}

M_MODEL& ornullCopyright()
	{return whereOrNull(B_BASE::cols::copyright);
	}

M_MODEL& ornotnullCopyright()
	{return whereOrNotNull(B_BASE::cols::copyright);
	}

M_MODEL& eqBeiansn(const std::string &val)
	{return where(B_BASE::cols::beiansn, orm::wq::eq, val);
	}

M_MODEL& nqBeiansn(const std::string &val)
	{return where(B_BASE::cols::beiansn, orm::wq::nq, val);
	}

M_MODEL& btBeiansn(const std::string &val)
	{return where(B_BASE::cols::beiansn, orm::wq::bt, val);
	}

M_MODEL& beBeiansn(const std::string &val)
	{return where(B_BASE::cols::beiansn, orm::wq::be, val);
	}

M_MODEL& ltBeiansn(const std::string &val)
	{return where(B_BASE::cols::beiansn, orm::wq::lt, val);
	}

M_MODEL& leBeiansn(const std::string &val)
	{return where(B_BASE::cols::beiansn, orm::wq::le, val);
	}

M_MODEL& likeBeiansn(const std::string &val)
	{return where(B_BASE::cols::beiansn, orm::wq::like, val);
	}

M_MODEL& nullBeiansn()
	{return whereNull(B_BASE::cols::beiansn);
	}

M_MODEL& notnullBeiansn()
	{return whereNotNull(B_BASE::cols::beiansn);
	}

M_MODEL& oreqBeiansn(const std::string &val)
	{return whereOr(B_BASE::cols::beiansn, orm::wq::eq, val);
	}

M_MODEL& ornqBeiansn(const std::string &val)
	{return whereOr(B_BASE::cols::beiansn, orm::wq::nq, val);
	}

M_MODEL& orbtBeiansn(const std::string &val)
	{return whereOr(B_BASE::cols::beiansn, orm::wq::bt, val);
	}

M_MODEL& orbeBeiansn(const std::string &val)
	{return whereOr(B_BASE::cols::beiansn, orm::wq::be, val);
	}

M_MODEL& orltBeiansn(const std::string &val)
	{return whereOr(B_BASE::cols::beiansn, orm::wq::lt, val);
	}

M_MODEL& orleBeiansn(const std::string &val)
	{return whereOr(B_BASE::cols::beiansn, orm::wq::le, val);
	}

M_MODEL& orlikeBeiansn(const std::string &val)
	{return whereOr(B_BASE::cols::beiansn, orm::wq::like, val);
	}

M_MODEL& ornullBeiansn()
	{return whereOrNull(B_BASE::cols::beiansn);
	}

M_MODEL& ornotnullBeiansn()
	{return whereOrNotNull(B_BASE::cols::beiansn);
	}

M_MODEL& eqFootscript(const std::string &val)
	{return where(B_BASE::cols::footscript, orm::wq::eq, val);
	}

M_MODEL& nqFootscript(const std::string &val)
	{return where(B_BASE::cols::footscript, orm::wq::nq, val);
	}

M_MODEL& btFootscript(const std::string &val)
	{return where(B_BASE::cols::footscript, orm::wq::bt, val);
	}

M_MODEL& beFootscript(const std::string &val)
	{return where(B_BASE::cols::footscript, orm::wq::be, val);
	}

M_MODEL& ltFootscript(const std::string &val)
	{return where(B_BASE::cols::footscript, orm::wq::lt, val);
	}

M_MODEL& leFootscript(const std::string &val)
	{return where(B_BASE::cols::footscript, orm::wq::le, val);
	}

M_MODEL& likeFootscript(const std::string &val)
	{return where(B_BASE::cols::footscript, orm::wq::like, val);
	}

M_MODEL& nullFootscript()
	{return whereNull(B_BASE::cols::footscript);
	}

M_MODEL& notnullFootscript()
	{return whereNotNull(B_BASE::cols::footscript);
	}

M_MODEL& oreqFootscript(const std::string &val)
	{return whereOr(B_BASE::cols::footscript, orm::wq::eq, val);
	}

M_MODEL& ornqFootscript(const std::string &val)
	{return whereOr(B_BASE::cols::footscript, orm::wq::nq, val);
	}

M_MODEL& orbtFootscript(const std::string &val)
	{return whereOr(B_BASE::cols::footscript, orm::wq::bt, val);
	}

M_MODEL& orbeFootscript(const std::string &val)
	{return whereOr(B_BASE::cols::footscript, orm::wq::be, val);
	}

M_MODEL& orltFootscript(const std::string &val)
	{return whereOr(B_BASE::cols::footscript, orm::wq::lt, val);
	}

M_MODEL& orleFootscript(const std::string &val)
	{return whereOr(B_BASE::cols::footscript, orm::wq::le, val);
	}

M_MODEL& orlikeFootscript(const std::string &val)
	{return whereOr(B_BASE::cols::footscript, orm::wq::like, val);
	}

M_MODEL& ornullFootscript()
	{return whereOrNull(B_BASE::cols::footscript);
	}

M_MODEL& ornotnullFootscript()
	{return whereOrNotNull(B_BASE::cols::footscript);
	}

M_MODEL& eqHeadscript(const std::string &val)
	{return where(B_BASE::cols::headscript, orm::wq::eq, val);
	}

M_MODEL& nqHeadscript(const std::string &val)
	{return where(B_BASE::cols::headscript, orm::wq::nq, val);
	}

M_MODEL& btHeadscript(const std::string &val)
	{return where(B_BASE::cols::headscript, orm::wq::bt, val);
	}

M_MODEL& beHeadscript(const std::string &val)
	{return where(B_BASE::cols::headscript, orm::wq::be, val);
	}

M_MODEL& ltHeadscript(const std::string &val)
	{return where(B_BASE::cols::headscript, orm::wq::lt, val);
	}

M_MODEL& leHeadscript(const std::string &val)
	{return where(B_BASE::cols::headscript, orm::wq::le, val);
	}

M_MODEL& likeHeadscript(const std::string &val)
	{return where(B_BASE::cols::headscript, orm::wq::like, val);
	}

M_MODEL& nullHeadscript()
	{return whereNull(B_BASE::cols::headscript);
	}

M_MODEL& notnullHeadscript()
	{return whereNotNull(B_BASE::cols::headscript);
	}

M_MODEL& oreqHeadscript(const std::string &val)
	{return whereOr(B_BASE::cols::headscript, orm::wq::eq, val);
	}

M_MODEL& ornqHeadscript(const std::string &val)
	{return whereOr(B_BASE::cols::headscript, orm::wq::nq, val);
	}

M_MODEL& orbtHeadscript(const std::string &val)
	{return whereOr(B_BASE::cols::headscript, orm::wq::bt, val);
	}

M_MODEL& orbeHeadscript(const std::string &val)
	{return whereOr(B_BASE::cols::headscript, orm::wq::be, val);
	}

M_MODEL& orltHeadscript(const std::string &val)
	{return whereOr(B_BASE::cols::headscript, orm::wq::lt, val);
	}

M_MODEL& orleHeadscript(const std::string &val)
	{return whereOr(B_BASE::cols::headscript, orm::wq::le, val);
	}

M_MODEL& orlikeHeadscript(const std::string &val)
	{return whereOr(B_BASE::cols::headscript, orm::wq::like, val);
	}

M_MODEL& ornullHeadscript()
	{return whereOrNull(B_BASE::cols::headscript);
	}

M_MODEL& ornotnullHeadscript()
	{return whereOrNotNull(B_BASE::cols::headscript);
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

M_MODEL& eqSitelogo(const std::string &val)
	{return where(B_BASE::cols::sitelogo, orm::wq::eq, val);
	}

M_MODEL& nqSitelogo(const std::string &val)
	{return where(B_BASE::cols::sitelogo, orm::wq::nq, val);
	}

M_MODEL& btSitelogo(const std::string &val)
	{return where(B_BASE::cols::sitelogo, orm::wq::bt, val);
	}

M_MODEL& beSitelogo(const std::string &val)
	{return where(B_BASE::cols::sitelogo, orm::wq::be, val);
	}

M_MODEL& ltSitelogo(const std::string &val)
	{return where(B_BASE::cols::sitelogo, orm::wq::lt, val);
	}

M_MODEL& leSitelogo(const std::string &val)
	{return where(B_BASE::cols::sitelogo, orm::wq::le, val);
	}

M_MODEL& likeSitelogo(const std::string &val)
	{return where(B_BASE::cols::sitelogo, orm::wq::like, val);
	}

M_MODEL& nullSitelogo()
	{return whereNull(B_BASE::cols::sitelogo);
	}

M_MODEL& notnullSitelogo()
	{return whereNotNull(B_BASE::cols::sitelogo);
	}

M_MODEL& oreqSitelogo(const std::string &val)
	{return whereOr(B_BASE::cols::sitelogo, orm::wq::eq, val);
	}

M_MODEL& ornqSitelogo(const std::string &val)
	{return whereOr(B_BASE::cols::sitelogo, orm::wq::nq, val);
	}

M_MODEL& orbtSitelogo(const std::string &val)
	{return whereOr(B_BASE::cols::sitelogo, orm::wq::bt, val);
	}

M_MODEL& orbeSitelogo(const std::string &val)
	{return whereOr(B_BASE::cols::sitelogo, orm::wq::be, val);
	}

M_MODEL& orltSitelogo(const std::string &val)
	{return whereOr(B_BASE::cols::sitelogo, orm::wq::lt, val);
	}

M_MODEL& orleSitelogo(const std::string &val)
	{return whereOr(B_BASE::cols::sitelogo, orm::wq::le, val);
	}

M_MODEL& orlikeSitelogo(const std::string &val)
	{return whereOr(B_BASE::cols::sitelogo, orm::wq::like, val);
	}

M_MODEL& ornullSitelogo()
	{return whereOrNull(B_BASE::cols::sitelogo);
	}

M_MODEL& ornotnullSitelogo()
	{return whereOrNotNull(B_BASE::cols::sitelogo);
	}

M_MODEL& eqSitebanner(const std::string &val)
	{return where(B_BASE::cols::sitebanner, orm::wq::eq, val);
	}

M_MODEL& nqSitebanner(const std::string &val)
	{return where(B_BASE::cols::sitebanner, orm::wq::nq, val);
	}

M_MODEL& btSitebanner(const std::string &val)
	{return where(B_BASE::cols::sitebanner, orm::wq::bt, val);
	}

M_MODEL& beSitebanner(const std::string &val)
	{return where(B_BASE::cols::sitebanner, orm::wq::be, val);
	}

M_MODEL& ltSitebanner(const std::string &val)
	{return where(B_BASE::cols::sitebanner, orm::wq::lt, val);
	}

M_MODEL& leSitebanner(const std::string &val)
	{return where(B_BASE::cols::sitebanner, orm::wq::le, val);
	}

M_MODEL& likeSitebanner(const std::string &val)
	{return where(B_BASE::cols::sitebanner, orm::wq::like, val);
	}

M_MODEL& nullSitebanner()
	{return whereNull(B_BASE::cols::sitebanner);
	}

M_MODEL& notnullSitebanner()
	{return whereNotNull(B_BASE::cols::sitebanner);
	}

M_MODEL& oreqSitebanner(const std::string &val)
	{return whereOr(B_BASE::cols::sitebanner, orm::wq::eq, val);
	}

M_MODEL& ornqSitebanner(const std::string &val)
	{return whereOr(B_BASE::cols::sitebanner, orm::wq::nq, val);
	}

M_MODEL& orbtSitebanner(const std::string &val)
	{return whereOr(B_BASE::cols::sitebanner, orm::wq::bt, val);
	}

M_MODEL& orbeSitebanner(const std::string &val)
	{return whereOr(B_BASE::cols::sitebanner, orm::wq::be, val);
	}

M_MODEL& orltSitebanner(const std::string &val)
	{return whereOr(B_BASE::cols::sitebanner, orm::wq::lt, val);
	}

M_MODEL& orleSitebanner(const std::string &val)
	{return whereOr(B_BASE::cols::sitebanner, orm::wq::le, val);
	}

M_MODEL& orlikeSitebanner(const std::string &val)
	{return whereOr(B_BASE::cols::sitebanner, orm::wq::like, val);
	}

M_MODEL& ornullSitebanner()
	{return whereOrNull(B_BASE::cols::sitebanner);
	}

M_MODEL& ornotnullSitebanner()
	{return whereOrNotNull(B_BASE::cols::sitebanner);
	}

M_MODEL& eqContactman(const std::string &val)
	{return where(B_BASE::cols::contactman, orm::wq::eq, val);
	}

M_MODEL& nqContactman(const std::string &val)
	{return where(B_BASE::cols::contactman, orm::wq::nq, val);
	}

M_MODEL& btContactman(const std::string &val)
	{return where(B_BASE::cols::contactman, orm::wq::bt, val);
	}

M_MODEL& beContactman(const std::string &val)
	{return where(B_BASE::cols::contactman, orm::wq::be, val);
	}

M_MODEL& ltContactman(const std::string &val)
	{return where(B_BASE::cols::contactman, orm::wq::lt, val);
	}

M_MODEL& leContactman(const std::string &val)
	{return where(B_BASE::cols::contactman, orm::wq::le, val);
	}

M_MODEL& likeContactman(const std::string &val)
	{return where(B_BASE::cols::contactman, orm::wq::like, val);
	}

M_MODEL& nullContactman()
	{return whereNull(B_BASE::cols::contactman);
	}

M_MODEL& notnullContactman()
	{return whereNotNull(B_BASE::cols::contactman);
	}

M_MODEL& oreqContactman(const std::string &val)
	{return whereOr(B_BASE::cols::contactman, orm::wq::eq, val);
	}

M_MODEL& ornqContactman(const std::string &val)
	{return whereOr(B_BASE::cols::contactman, orm::wq::nq, val);
	}

M_MODEL& orbtContactman(const std::string &val)
	{return whereOr(B_BASE::cols::contactman, orm::wq::bt, val);
	}

M_MODEL& orbeContactman(const std::string &val)
	{return whereOr(B_BASE::cols::contactman, orm::wq::be, val);
	}

M_MODEL& orltContactman(const std::string &val)
	{return whereOr(B_BASE::cols::contactman, orm::wq::lt, val);
	}

M_MODEL& orleContactman(const std::string &val)
	{return whereOr(B_BASE::cols::contactman, orm::wq::le, val);
	}

M_MODEL& orlikeContactman(const std::string &val)
	{return whereOr(B_BASE::cols::contactman, orm::wq::like, val);
	}

M_MODEL& ornullContactman()
	{return whereOrNull(B_BASE::cols::contactman);
	}

M_MODEL& ornotnullContactman()
	{return whereOrNotNull(B_BASE::cols::contactman);
	}

M_MODEL& eqPhone(const std::string &val)
	{return where(B_BASE::cols::phone, orm::wq::eq, val);
	}

M_MODEL& nqPhone(const std::string &val)
	{return where(B_BASE::cols::phone, orm::wq::nq, val);
	}

M_MODEL& btPhone(const std::string &val)
	{return where(B_BASE::cols::phone, orm::wq::bt, val);
	}

M_MODEL& bePhone(const std::string &val)
	{return where(B_BASE::cols::phone, orm::wq::be, val);
	}

M_MODEL& ltPhone(const std::string &val)
	{return where(B_BASE::cols::phone, orm::wq::lt, val);
	}

M_MODEL& lePhone(const std::string &val)
	{return where(B_BASE::cols::phone, orm::wq::le, val);
	}

M_MODEL& likePhone(const std::string &val)
	{return where(B_BASE::cols::phone, orm::wq::like, val);
	}

M_MODEL& nullPhone()
	{return whereNull(B_BASE::cols::phone);
	}

M_MODEL& notnullPhone()
	{return whereNotNull(B_BASE::cols::phone);
	}

M_MODEL& oreqPhone(const std::string &val)
	{return whereOr(B_BASE::cols::phone, orm::wq::eq, val);
	}

M_MODEL& ornqPhone(const std::string &val)
	{return whereOr(B_BASE::cols::phone, orm::wq::nq, val);
	}

M_MODEL& orbtPhone(const std::string &val)
	{return whereOr(B_BASE::cols::phone, orm::wq::bt, val);
	}

M_MODEL& orbePhone(const std::string &val)
	{return whereOr(B_BASE::cols::phone, orm::wq::be, val);
	}

M_MODEL& orltPhone(const std::string &val)
	{return whereOr(B_BASE::cols::phone, orm::wq::lt, val);
	}

M_MODEL& orlePhone(const std::string &val)
	{return whereOr(B_BASE::cols::phone, orm::wq::le, val);
	}

M_MODEL& orlikePhone(const std::string &val)
	{return whereOr(B_BASE::cols::phone, orm::wq::like, val);
	}

M_MODEL& ornullPhone()
	{return whereOrNull(B_BASE::cols::phone);
	}

M_MODEL& ornotnullPhone()
	{return whereOrNotNull(B_BASE::cols::phone);
	}

M_MODEL& eqMobile(const std::string &val)
	{return where(B_BASE::cols::mobile, orm::wq::eq, val);
	}

M_MODEL& nqMobile(const std::string &val)
	{return where(B_BASE::cols::mobile, orm::wq::nq, val);
	}

M_MODEL& btMobile(const std::string &val)
	{return where(B_BASE::cols::mobile, orm::wq::bt, val);
	}

M_MODEL& beMobile(const std::string &val)
	{return where(B_BASE::cols::mobile, orm::wq::be, val);
	}

M_MODEL& ltMobile(const std::string &val)
	{return where(B_BASE::cols::mobile, orm::wq::lt, val);
	}

M_MODEL& leMobile(const std::string &val)
	{return where(B_BASE::cols::mobile, orm::wq::le, val);
	}

M_MODEL& likeMobile(const std::string &val)
	{return where(B_BASE::cols::mobile, orm::wq::like, val);
	}

M_MODEL& nullMobile()
	{return whereNull(B_BASE::cols::mobile);
	}

M_MODEL& notnullMobile()
	{return whereNotNull(B_BASE::cols::mobile);
	}

M_MODEL& oreqMobile(const std::string &val)
	{return whereOr(B_BASE::cols::mobile, orm::wq::eq, val);
	}

M_MODEL& ornqMobile(const std::string &val)
	{return whereOr(B_BASE::cols::mobile, orm::wq::nq, val);
	}

M_MODEL& orbtMobile(const std::string &val)
	{return whereOr(B_BASE::cols::mobile, orm::wq::bt, val);
	}

M_MODEL& orbeMobile(const std::string &val)
	{return whereOr(B_BASE::cols::mobile, orm::wq::be, val);
	}

M_MODEL& orltMobile(const std::string &val)
	{return whereOr(B_BASE::cols::mobile, orm::wq::lt, val);
	}

M_MODEL& orleMobile(const std::string &val)
	{return whereOr(B_BASE::cols::mobile, orm::wq::le, val);
	}

M_MODEL& orlikeMobile(const std::string &val)
	{return whereOr(B_BASE::cols::mobile, orm::wq::like, val);
	}

M_MODEL& ornullMobile()
	{return whereOrNull(B_BASE::cols::mobile);
	}

M_MODEL& ornotnullMobile()
	{return whereOrNotNull(B_BASE::cols::mobile);
	}

M_MODEL& eqEmail(const std::string &val)
	{return where(B_BASE::cols::email, orm::wq::eq, val);
	}

M_MODEL& nqEmail(const std::string &val)
	{return where(B_BASE::cols::email, orm::wq::nq, val);
	}

M_MODEL& btEmail(const std::string &val)
	{return where(B_BASE::cols::email, orm::wq::bt, val);
	}

M_MODEL& beEmail(const std::string &val)
	{return where(B_BASE::cols::email, orm::wq::be, val);
	}

M_MODEL& ltEmail(const std::string &val)
	{return where(B_BASE::cols::email, orm::wq::lt, val);
	}

M_MODEL& leEmail(const std::string &val)
	{return where(B_BASE::cols::email, orm::wq::le, val);
	}

M_MODEL& likeEmail(const std::string &val)
	{return where(B_BASE::cols::email, orm::wq::like, val);
	}

M_MODEL& nullEmail()
	{return whereNull(B_BASE::cols::email);
	}

M_MODEL& notnullEmail()
	{return whereNotNull(B_BASE::cols::email);
	}

M_MODEL& oreqEmail(const std::string &val)
	{return whereOr(B_BASE::cols::email, orm::wq::eq, val);
	}

M_MODEL& ornqEmail(const std::string &val)
	{return whereOr(B_BASE::cols::email, orm::wq::nq, val);
	}

M_MODEL& orbtEmail(const std::string &val)
	{return whereOr(B_BASE::cols::email, orm::wq::bt, val);
	}

M_MODEL& orbeEmail(const std::string &val)
	{return whereOr(B_BASE::cols::email, orm::wq::be, val);
	}

M_MODEL& orltEmail(const std::string &val)
	{return whereOr(B_BASE::cols::email, orm::wq::lt, val);
	}

M_MODEL& orleEmail(const std::string &val)
	{return whereOr(B_BASE::cols::email, orm::wq::le, val);
	}

M_MODEL& orlikeEmail(const std::string &val)
	{return whereOr(B_BASE::cols::email, orm::wq::like, val);
	}

M_MODEL& ornullEmail()
	{return whereOrNull(B_BASE::cols::email);
	}

M_MODEL& ornotnullEmail()
	{return whereOrNotNull(B_BASE::cols::email);
	}

M_MODEL& eqBankname(const std::string &val)
	{return where(B_BASE::cols::bankname, orm::wq::eq, val);
	}

M_MODEL& nqBankname(const std::string &val)
	{return where(B_BASE::cols::bankname, orm::wq::nq, val);
	}

M_MODEL& btBankname(const std::string &val)
	{return where(B_BASE::cols::bankname, orm::wq::bt, val);
	}

M_MODEL& beBankname(const std::string &val)
	{return where(B_BASE::cols::bankname, orm::wq::be, val);
	}

M_MODEL& ltBankname(const std::string &val)
	{return where(B_BASE::cols::bankname, orm::wq::lt, val);
	}

M_MODEL& leBankname(const std::string &val)
	{return where(B_BASE::cols::bankname, orm::wq::le, val);
	}

M_MODEL& likeBankname(const std::string &val)
	{return where(B_BASE::cols::bankname, orm::wq::like, val);
	}

M_MODEL& nullBankname()
	{return whereNull(B_BASE::cols::bankname);
	}

M_MODEL& notnullBankname()
	{return whereNotNull(B_BASE::cols::bankname);
	}

M_MODEL& oreqBankname(const std::string &val)
	{return whereOr(B_BASE::cols::bankname, orm::wq::eq, val);
	}

M_MODEL& ornqBankname(const std::string &val)
	{return whereOr(B_BASE::cols::bankname, orm::wq::nq, val);
	}

M_MODEL& orbtBankname(const std::string &val)
	{return whereOr(B_BASE::cols::bankname, orm::wq::bt, val);
	}

M_MODEL& orbeBankname(const std::string &val)
	{return whereOr(B_BASE::cols::bankname, orm::wq::be, val);
	}

M_MODEL& orltBankname(const std::string &val)
	{return whereOr(B_BASE::cols::bankname, orm::wq::lt, val);
	}

M_MODEL& orleBankname(const std::string &val)
	{return whereOr(B_BASE::cols::bankname, orm::wq::le, val);
	}

M_MODEL& orlikeBankname(const std::string &val)
	{return whereOr(B_BASE::cols::bankname, orm::wq::like, val);
	}

M_MODEL& ornullBankname()
	{return whereOrNull(B_BASE::cols::bankname);
	}

M_MODEL& ornotnullBankname()
	{return whereOrNotNull(B_BASE::cols::bankname);
	}

M_MODEL& eqBanksn(const std::string &val)
	{return where(B_BASE::cols::banksn, orm::wq::eq, val);
	}

M_MODEL& nqBanksn(const std::string &val)
	{return where(B_BASE::cols::banksn, orm::wq::nq, val);
	}

M_MODEL& btBanksn(const std::string &val)
	{return where(B_BASE::cols::banksn, orm::wq::bt, val);
	}

M_MODEL& beBanksn(const std::string &val)
	{return where(B_BASE::cols::banksn, orm::wq::be, val);
	}

M_MODEL& ltBanksn(const std::string &val)
	{return where(B_BASE::cols::banksn, orm::wq::lt, val);
	}

M_MODEL& leBanksn(const std::string &val)
	{return where(B_BASE::cols::banksn, orm::wq::le, val);
	}

M_MODEL& likeBanksn(const std::string &val)
	{return where(B_BASE::cols::banksn, orm::wq::like, val);
	}

M_MODEL& nullBanksn()
	{return whereNull(B_BASE::cols::banksn);
	}

M_MODEL& notnullBanksn()
	{return whereNotNull(B_BASE::cols::banksn);
	}

M_MODEL& oreqBanksn(const std::string &val)
	{return whereOr(B_BASE::cols::banksn, orm::wq::eq, val);
	}

M_MODEL& ornqBanksn(const std::string &val)
	{return whereOr(B_BASE::cols::banksn, orm::wq::nq, val);
	}

M_MODEL& orbtBanksn(const std::string &val)
	{return whereOr(B_BASE::cols::banksn, orm::wq::bt, val);
	}

M_MODEL& orbeBanksn(const std::string &val)
	{return whereOr(B_BASE::cols::banksn, orm::wq::be, val);
	}

M_MODEL& orltBanksn(const std::string &val)
	{return whereOr(B_BASE::cols::banksn, orm::wq::lt, val);
	}

M_MODEL& orleBanksn(const std::string &val)
	{return whereOr(B_BASE::cols::banksn, orm::wq::le, val);
	}

M_MODEL& orlikeBanksn(const std::string &val)
	{return whereOr(B_BASE::cols::banksn, orm::wq::like, val);
	}

M_MODEL& ornullBanksn()
	{return whereOrNull(B_BASE::cols::banksn);
	}

M_MODEL& ornotnullBanksn()
	{return whereOrNotNull(B_BASE::cols::banksn);
	}

M_MODEL& eqAddress(const std::string &val)
	{return where(B_BASE::cols::address, orm::wq::eq, val);
	}

M_MODEL& nqAddress(const std::string &val)
	{return where(B_BASE::cols::address, orm::wq::nq, val);
	}

M_MODEL& btAddress(const std::string &val)
	{return where(B_BASE::cols::address, orm::wq::bt, val);
	}

M_MODEL& beAddress(const std::string &val)
	{return where(B_BASE::cols::address, orm::wq::be, val);
	}

M_MODEL& ltAddress(const std::string &val)
	{return where(B_BASE::cols::address, orm::wq::lt, val);
	}

M_MODEL& leAddress(const std::string &val)
	{return where(B_BASE::cols::address, orm::wq::le, val);
	}

M_MODEL& likeAddress(const std::string &val)
	{return where(B_BASE::cols::address, orm::wq::like, val);
	}

M_MODEL& nullAddress()
	{return whereNull(B_BASE::cols::address);
	}

M_MODEL& notnullAddress()
	{return whereNotNull(B_BASE::cols::address);
	}

M_MODEL& oreqAddress(const std::string &val)
	{return whereOr(B_BASE::cols::address, orm::wq::eq, val);
	}

M_MODEL& ornqAddress(const std::string &val)
	{return whereOr(B_BASE::cols::address, orm::wq::nq, val);
	}

M_MODEL& orbtAddress(const std::string &val)
	{return whereOr(B_BASE::cols::address, orm::wq::bt, val);
	}

M_MODEL& orbeAddress(const std::string &val)
	{return whereOr(B_BASE::cols::address, orm::wq::be, val);
	}

M_MODEL& orltAddress(const std::string &val)
	{return whereOr(B_BASE::cols::address, orm::wq::lt, val);
	}

M_MODEL& orleAddress(const std::string &val)
	{return whereOr(B_BASE::cols::address, orm::wq::le, val);
	}

M_MODEL& orlikeAddress(const std::string &val)
	{return whereOr(B_BASE::cols::address, orm::wq::like, val);
	}

M_MODEL& ornullAddress()
	{return whereOrNull(B_BASE::cols::address);
	}

M_MODEL& ornotnullAddress()
	{return whereOrNotNull(B_BASE::cols::address);
	}

M_MODEL& eqZipnum(const std::string &val)
	{return where(B_BASE::cols::zipnum, orm::wq::eq, val);
	}

M_MODEL& nqZipnum(const std::string &val)
	{return where(B_BASE::cols::zipnum, orm::wq::nq, val);
	}

M_MODEL& btZipnum(const std::string &val)
	{return where(B_BASE::cols::zipnum, orm::wq::bt, val);
	}

M_MODEL& beZipnum(const std::string &val)
	{return where(B_BASE::cols::zipnum, orm::wq::be, val);
	}

M_MODEL& ltZipnum(const std::string &val)
	{return where(B_BASE::cols::zipnum, orm::wq::lt, val);
	}

M_MODEL& leZipnum(const std::string &val)
	{return where(B_BASE::cols::zipnum, orm::wq::le, val);
	}

M_MODEL& likeZipnum(const std::string &val)
	{return where(B_BASE::cols::zipnum, orm::wq::like, val);
	}

M_MODEL& nullZipnum()
	{return whereNull(B_BASE::cols::zipnum);
	}

M_MODEL& notnullZipnum()
	{return whereNotNull(B_BASE::cols::zipnum);
	}

M_MODEL& oreqZipnum(const std::string &val)
	{return whereOr(B_BASE::cols::zipnum, orm::wq::eq, val);
	}

M_MODEL& ornqZipnum(const std::string &val)
	{return whereOr(B_BASE::cols::zipnum, orm::wq::nq, val);
	}

M_MODEL& orbtZipnum(const std::string &val)
	{return whereOr(B_BASE::cols::zipnum, orm::wq::bt, val);
	}

M_MODEL& orbeZipnum(const std::string &val)
	{return whereOr(B_BASE::cols::zipnum, orm::wq::be, val);
	}

M_MODEL& orltZipnum(const std::string &val)
	{return whereOr(B_BASE::cols::zipnum, orm::wq::lt, val);
	}

M_MODEL& orleZipnum(const std::string &val)
	{return whereOr(B_BASE::cols::zipnum, orm::wq::le, val);
	}

M_MODEL& orlikeZipnum(const std::string &val)
	{return whereOr(B_BASE::cols::zipnum, orm::wq::like, val);
	}

M_MODEL& ornullZipnum()
	{return whereOrNull(B_BASE::cols::zipnum);
	}

M_MODEL& ornotnullZipnum()
	{return whereOrNotNull(B_BASE::cols::zipnum);
	}

M_MODEL& eqTaxsn(const std::string &val)
	{return where(B_BASE::cols::taxsn, orm::wq::eq, val);
	}

M_MODEL& nqTaxsn(const std::string &val)
	{return where(B_BASE::cols::taxsn, orm::wq::nq, val);
	}

M_MODEL& btTaxsn(const std::string &val)
	{return where(B_BASE::cols::taxsn, orm::wq::bt, val);
	}

M_MODEL& beTaxsn(const std::string &val)
	{return where(B_BASE::cols::taxsn, orm::wq::be, val);
	}

M_MODEL& ltTaxsn(const std::string &val)
	{return where(B_BASE::cols::taxsn, orm::wq::lt, val);
	}

M_MODEL& leTaxsn(const std::string &val)
	{return where(B_BASE::cols::taxsn, orm::wq::le, val);
	}

M_MODEL& likeTaxsn(const std::string &val)
	{return where(B_BASE::cols::taxsn, orm::wq::like, val);
	}

M_MODEL& nullTaxsn()
	{return whereNull(B_BASE::cols::taxsn);
	}

M_MODEL& notnullTaxsn()
	{return whereNotNull(B_BASE::cols::taxsn);
	}

M_MODEL& oreqTaxsn(const std::string &val)
	{return whereOr(B_BASE::cols::taxsn, orm::wq::eq, val);
	}

M_MODEL& ornqTaxsn(const std::string &val)
	{return whereOr(B_BASE::cols::taxsn, orm::wq::nq, val);
	}

M_MODEL& orbtTaxsn(const std::string &val)
	{return whereOr(B_BASE::cols::taxsn, orm::wq::bt, val);
	}

M_MODEL& orbeTaxsn(const std::string &val)
	{return whereOr(B_BASE::cols::taxsn, orm::wq::be, val);
	}

M_MODEL& orltTaxsn(const std::string &val)
	{return whereOr(B_BASE::cols::taxsn, orm::wq::lt, val);
	}

M_MODEL& orleTaxsn(const std::string &val)
	{return whereOr(B_BASE::cols::taxsn, orm::wq::le, val);
	}

M_MODEL& orlikeTaxsn(const std::string &val)
	{return whereOr(B_BASE::cols::taxsn, orm::wq::like, val);
	}

M_MODEL& ornullTaxsn()
	{return whereOrNull(B_BASE::cols::taxsn);
	}

M_MODEL& ornotnullTaxsn()
	{return whereOrNotNull(B_BASE::cols::taxsn);
	}

M_MODEL& eqCompanyname(const std::string &val)
	{return where(B_BASE::cols::companyname, orm::wq::eq, val);
	}

M_MODEL& nqCompanyname(const std::string &val)
	{return where(B_BASE::cols::companyname, orm::wq::nq, val);
	}

M_MODEL& btCompanyname(const std::string &val)
	{return where(B_BASE::cols::companyname, orm::wq::bt, val);
	}

M_MODEL& beCompanyname(const std::string &val)
	{return where(B_BASE::cols::companyname, orm::wq::be, val);
	}

M_MODEL& ltCompanyname(const std::string &val)
	{return where(B_BASE::cols::companyname, orm::wq::lt, val);
	}

M_MODEL& leCompanyname(const std::string &val)
	{return where(B_BASE::cols::companyname, orm::wq::le, val);
	}

M_MODEL& likeCompanyname(const std::string &val)
	{return where(B_BASE::cols::companyname, orm::wq::like, val);
	}

M_MODEL& nullCompanyname()
	{return whereNull(B_BASE::cols::companyname);
	}

M_MODEL& notnullCompanyname()
	{return whereNotNull(B_BASE::cols::companyname);
	}

M_MODEL& oreqCompanyname(const std::string &val)
	{return whereOr(B_BASE::cols::companyname, orm::wq::eq, val);
	}

M_MODEL& ornqCompanyname(const std::string &val)
	{return whereOr(B_BASE::cols::companyname, orm::wq::nq, val);
	}

M_MODEL& orbtCompanyname(const std::string &val)
	{return whereOr(B_BASE::cols::companyname, orm::wq::bt, val);
	}

M_MODEL& orbeCompanyname(const std::string &val)
	{return whereOr(B_BASE::cols::companyname, orm::wq::be, val);
	}

M_MODEL& orltCompanyname(const std::string &val)
	{return whereOr(B_BASE::cols::companyname, orm::wq::lt, val);
	}

M_MODEL& orleCompanyname(const std::string &val)
	{return whereOr(B_BASE::cols::companyname, orm::wq::le, val);
	}

M_MODEL& orlikeCompanyname(const std::string &val)
	{return whereOr(B_BASE::cols::companyname, orm::wq::like, val);
	}

M_MODEL& ornullCompanyname()
	{return whereOrNull(B_BASE::cols::companyname);
	}

M_MODEL& ornotnullCompanyname()
	{return whereOrNotNull(B_BASE::cols::companyname);
	}

M_MODEL& eqLinkname(const std::string &val)
	{return where(B_BASE::cols::linkname, orm::wq::eq, val);
	}

M_MODEL& nqLinkname(const std::string &val)
	{return where(B_BASE::cols::linkname, orm::wq::nq, val);
	}

M_MODEL& btLinkname(const std::string &val)
	{return where(B_BASE::cols::linkname, orm::wq::bt, val);
	}

M_MODEL& beLinkname(const std::string &val)
	{return where(B_BASE::cols::linkname, orm::wq::be, val);
	}

M_MODEL& ltLinkname(const std::string &val)
	{return where(B_BASE::cols::linkname, orm::wq::lt, val);
	}

M_MODEL& leLinkname(const std::string &val)
	{return where(B_BASE::cols::linkname, orm::wq::le, val);
	}

M_MODEL& likeLinkname(const std::string &val)
	{return where(B_BASE::cols::linkname, orm::wq::like, val);
	}

M_MODEL& nullLinkname()
	{return whereNull(B_BASE::cols::linkname);
	}

M_MODEL& notnullLinkname()
	{return whereNotNull(B_BASE::cols::linkname);
	}

M_MODEL& oreqLinkname(const std::string &val)
	{return whereOr(B_BASE::cols::linkname, orm::wq::eq, val);
	}

M_MODEL& ornqLinkname(const std::string &val)
	{return whereOr(B_BASE::cols::linkname, orm::wq::nq, val);
	}

M_MODEL& orbtLinkname(const std::string &val)
	{return whereOr(B_BASE::cols::linkname, orm::wq::bt, val);
	}

M_MODEL& orbeLinkname(const std::string &val)
	{return whereOr(B_BASE::cols::linkname, orm::wq::be, val);
	}

M_MODEL& orltLinkname(const std::string &val)
	{return whereOr(B_BASE::cols::linkname, orm::wq::lt, val);
	}

M_MODEL& orleLinkname(const std::string &val)
	{return whereOr(B_BASE::cols::linkname, orm::wq::le, val);
	}

M_MODEL& orlikeLinkname(const std::string &val)
	{return whereOr(B_BASE::cols::linkname, orm::wq::like, val);
	}

M_MODEL& ornullLinkname()
	{return whereOrNull(B_BASE::cols::linkname);
	}

M_MODEL& ornotnullLinkname()
	{return whereOrNotNull(B_BASE::cols::linkname);
	}

M_MODEL& eqLinkmobile(const std::string &val)
	{return where(B_BASE::cols::linkmobile, orm::wq::eq, val);
	}

M_MODEL& nqLinkmobile(const std::string &val)
	{return where(B_BASE::cols::linkmobile, orm::wq::nq, val);
	}

M_MODEL& btLinkmobile(const std::string &val)
	{return where(B_BASE::cols::linkmobile, orm::wq::bt, val);
	}

M_MODEL& beLinkmobile(const std::string &val)
	{return where(B_BASE::cols::linkmobile, orm::wq::be, val);
	}

M_MODEL& ltLinkmobile(const std::string &val)
	{return where(B_BASE::cols::linkmobile, orm::wq::lt, val);
	}

M_MODEL& leLinkmobile(const std::string &val)
	{return where(B_BASE::cols::linkmobile, orm::wq::le, val);
	}

M_MODEL& likeLinkmobile(const std::string &val)
	{return where(B_BASE::cols::linkmobile, orm::wq::like, val);
	}

M_MODEL& nullLinkmobile()
	{return whereNull(B_BASE::cols::linkmobile);
	}

M_MODEL& notnullLinkmobile()
	{return whereNotNull(B_BASE::cols::linkmobile);
	}

M_MODEL& oreqLinkmobile(const std::string &val)
	{return whereOr(B_BASE::cols::linkmobile, orm::wq::eq, val);
	}

M_MODEL& ornqLinkmobile(const std::string &val)
	{return whereOr(B_BASE::cols::linkmobile, orm::wq::nq, val);
	}

M_MODEL& orbtLinkmobile(const std::string &val)
	{return whereOr(B_BASE::cols::linkmobile, orm::wq::bt, val);
	}

M_MODEL& orbeLinkmobile(const std::string &val)
	{return whereOr(B_BASE::cols::linkmobile, orm::wq::be, val);
	}

M_MODEL& orltLinkmobile(const std::string &val)
	{return whereOr(B_BASE::cols::linkmobile, orm::wq::lt, val);
	}

M_MODEL& orleLinkmobile(const std::string &val)
	{return whereOr(B_BASE::cols::linkmobile, orm::wq::le, val);
	}

M_MODEL& orlikeLinkmobile(const std::string &val)
	{return whereOr(B_BASE::cols::linkmobile, orm::wq::like, val);
	}

M_MODEL& ornullLinkmobile()
	{return whereOrNull(B_BASE::cols::linkmobile);
	}

M_MODEL& ornotnullLinkmobile()
	{return whereOrNotNull(B_BASE::cols::linkmobile);
	}

M_MODEL& eqLinkaddress(const std::string &val)
	{return where(B_BASE::cols::linkaddress, orm::wq::eq, val);
	}

M_MODEL& nqLinkaddress(const std::string &val)
	{return where(B_BASE::cols::linkaddress, orm::wq::nq, val);
	}

M_MODEL& btLinkaddress(const std::string &val)
	{return where(B_BASE::cols::linkaddress, orm::wq::bt, val);
	}

M_MODEL& beLinkaddress(const std::string &val)
	{return where(B_BASE::cols::linkaddress, orm::wq::be, val);
	}

M_MODEL& ltLinkaddress(const std::string &val)
	{return where(B_BASE::cols::linkaddress, orm::wq::lt, val);
	}

M_MODEL& leLinkaddress(const std::string &val)
	{return where(B_BASE::cols::linkaddress, orm::wq::le, val);
	}

M_MODEL& likeLinkaddress(const std::string &val)
	{return where(B_BASE::cols::linkaddress, orm::wq::like, val);
	}

M_MODEL& nullLinkaddress()
	{return whereNull(B_BASE::cols::linkaddress);
	}

M_MODEL& notnullLinkaddress()
	{return whereNotNull(B_BASE::cols::linkaddress);
	}

M_MODEL& oreqLinkaddress(const std::string &val)
	{return whereOr(B_BASE::cols::linkaddress, orm::wq::eq, val);
	}

M_MODEL& ornqLinkaddress(const std::string &val)
	{return whereOr(B_BASE::cols::linkaddress, orm::wq::nq, val);
	}

M_MODEL& orbtLinkaddress(const std::string &val)
	{return whereOr(B_BASE::cols::linkaddress, orm::wq::bt, val);
	}

M_MODEL& orbeLinkaddress(const std::string &val)
	{return whereOr(B_BASE::cols::linkaddress, orm::wq::be, val);
	}

M_MODEL& orltLinkaddress(const std::string &val)
	{return whereOr(B_BASE::cols::linkaddress, orm::wq::lt, val);
	}

M_MODEL& orleLinkaddress(const std::string &val)
	{return whereOr(B_BASE::cols::linkaddress, orm::wq::le, val);
	}

M_MODEL& orlikeLinkaddress(const std::string &val)
	{return whereOr(B_BASE::cols::linkaddress, orm::wq::like, val);
	}

M_MODEL& ornullLinkaddress()
	{return whereOrNull(B_BASE::cols::linkaddress);
	}

M_MODEL& ornotnullLinkaddress()
	{return whereOrNotNull(B_BASE::cols::linkaddress);
	}

M_MODEL& eqTheme(const std::string &val)
	{return where(B_BASE::cols::theme, orm::wq::eq, val);
	}

M_MODEL& nqTheme(const std::string &val)
	{return where(B_BASE::cols::theme, orm::wq::nq, val);
	}

M_MODEL& btTheme(const std::string &val)
	{return where(B_BASE::cols::theme, orm::wq::bt, val);
	}

M_MODEL& beTheme(const std::string &val)
	{return where(B_BASE::cols::theme, orm::wq::be, val);
	}

M_MODEL& ltTheme(const std::string &val)
	{return where(B_BASE::cols::theme, orm::wq::lt, val);
	}

M_MODEL& leTheme(const std::string &val)
	{return where(B_BASE::cols::theme, orm::wq::le, val);
	}

M_MODEL& likeTheme(const std::string &val)
	{return where(B_BASE::cols::theme, orm::wq::like, val);
	}

M_MODEL& nullTheme()
	{return whereNull(B_BASE::cols::theme);
	}

M_MODEL& notnullTheme()
	{return whereNotNull(B_BASE::cols::theme);
	}

M_MODEL& oreqTheme(const std::string &val)
	{return whereOr(B_BASE::cols::theme, orm::wq::eq, val);
	}

M_MODEL& ornqTheme(const std::string &val)
	{return whereOr(B_BASE::cols::theme, orm::wq::nq, val);
	}

M_MODEL& orbtTheme(const std::string &val)
	{return whereOr(B_BASE::cols::theme, orm::wq::bt, val);
	}

M_MODEL& orbeTheme(const std::string &val)
	{return whereOr(B_BASE::cols::theme, orm::wq::be, val);
	}

M_MODEL& orltTheme(const std::string &val)
	{return whereOr(B_BASE::cols::theme, orm::wq::lt, val);
	}

M_MODEL& orleTheme(const std::string &val)
	{return whereOr(B_BASE::cols::theme, orm::wq::le, val);
	}

M_MODEL& orlikeTheme(const std::string &val)
	{return whereOr(B_BASE::cols::theme, orm::wq::like, val);
	}

M_MODEL& ornullTheme()
	{return whereOrNull(B_BASE::cols::theme);
	}

M_MODEL& ornotnullTheme()
	{return whereOrNotNull(B_BASE::cols::theme);
	}

M_MODEL& eqSitepath(const std::string &val)
	{return where(B_BASE::cols::sitepath, orm::wq::eq, val);
	}

M_MODEL& nqSitepath(const std::string &val)
	{return where(B_BASE::cols::sitepath, orm::wq::nq, val);
	}

M_MODEL& btSitepath(const std::string &val)
	{return where(B_BASE::cols::sitepath, orm::wq::bt, val);
	}

M_MODEL& beSitepath(const std::string &val)
	{return where(B_BASE::cols::sitepath, orm::wq::be, val);
	}

M_MODEL& ltSitepath(const std::string &val)
	{return where(B_BASE::cols::sitepath, orm::wq::lt, val);
	}

M_MODEL& leSitepath(const std::string &val)
	{return where(B_BASE::cols::sitepath, orm::wq::le, val);
	}

M_MODEL& likeSitepath(const std::string &val)
	{return where(B_BASE::cols::sitepath, orm::wq::like, val);
	}

M_MODEL& nullSitepath()
	{return whereNull(B_BASE::cols::sitepath);
	}

M_MODEL& notnullSitepath()
	{return whereNotNull(B_BASE::cols::sitepath);
	}

M_MODEL& oreqSitepath(const std::string &val)
	{return whereOr(B_BASE::cols::sitepath, orm::wq::eq, val);
	}

M_MODEL& ornqSitepath(const std::string &val)
	{return whereOr(B_BASE::cols::sitepath, orm::wq::nq, val);
	}

M_MODEL& orbtSitepath(const std::string &val)
	{return whereOr(B_BASE::cols::sitepath, orm::wq::bt, val);
	}

M_MODEL& orbeSitepath(const std::string &val)
	{return whereOr(B_BASE::cols::sitepath, orm::wq::be, val);
	}

M_MODEL& orltSitepath(const std::string &val)
	{return whereOr(B_BASE::cols::sitepath, orm::wq::lt, val);
	}

M_MODEL& orleSitepath(const std::string &val)
	{return whereOr(B_BASE::cols::sitepath, orm::wq::le, val);
	}

M_MODEL& orlikeSitepath(const std::string &val)
	{return whereOr(B_BASE::cols::sitepath, orm::wq::like, val);
	}

M_MODEL& ornullSitepath()
	{return whereOrNull(B_BASE::cols::sitepath);
	}

M_MODEL& ornotnullSitepath()
	{return whereOrNotNull(B_BASE::cols::sitepath);
	}

M_MODEL& eqIsopen(const std::string &val)
	{return where(B_BASE::cols::isopen, orm::wq::eq, val);
	}

M_MODEL& nqIsopen(const std::string &val)
	{return where(B_BASE::cols::isopen, orm::wq::nq, val);
	}

M_MODEL& btIsopen(const std::string &val)
	{return where(B_BASE::cols::isopen, orm::wq::bt, val);
	}

M_MODEL& beIsopen(const std::string &val)
	{return where(B_BASE::cols::isopen, orm::wq::be, val);
	}

M_MODEL& ltIsopen(const std::string &val)
	{return where(B_BASE::cols::isopen, orm::wq::lt, val);
	}

M_MODEL& leIsopen(const std::string &val)
	{return where(B_BASE::cols::isopen, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqIsopen(T val)
	{return where(B_BASE::cols::isopen, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqIsopen(T val)
	{return where(B_BASE::cols::isopen, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btIsopen(T val)
	{return where(B_BASE::cols::isopen, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beIsopen(T val)
	{return where(B_BASE::cols::isopen, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltIsopen(T val)
	{return where(B_BASE::cols::isopen, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leIsopen(T val)
	{return where(B_BASE::cols::isopen, orm::wq::le, val);
	}

M_MODEL& nullIsopen()
	{return whereNull(B_BASE::cols::isopen);
	}

M_MODEL& notnullIsopen()
	{return whereNotNull(B_BASE::cols::isopen);
	}

M_MODEL& oreqIsopen(const std::string &val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::eq, val);
	}

M_MODEL& ornqIsopen(const std::string &val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::nq, val);
	}

M_MODEL& orbtIsopen(const std::string &val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::bt, val);
	}

M_MODEL& orbeIsopen(const std::string &val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::be, val);
	}

M_MODEL& orltIsopen(const std::string &val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::lt, val);
	}

M_MODEL& orleIsopen(const std::string &val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqIsopen(T val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqIsopen(T val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtIsopen(T val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeIsopen(T val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltIsopen(T val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleIsopen(T val)
	{return whereOr(B_BASE::cols::isopen, orm::wq::le, val);
	}

M_MODEL& ornullIsopen()
	{return whereOrNull(B_BASE::cols::isopen);
	}

M_MODEL& ornotnullIsopen()
	{return whereOrNotNull(B_BASE::cols::isopen);
	}

M_MODEL& eqCreatedAt(const std::string &val)
	{return where(B_BASE::cols::created_at, orm::wq::eq, val);
	}

M_MODEL& nqCreatedAt(const std::string &val)
	{return where(B_BASE::cols::created_at, orm::wq::nq, val);
	}

M_MODEL& btCreatedAt(const std::string &val)
	{return where(B_BASE::cols::created_at, orm::wq::bt, val);
	}

M_MODEL& beCreatedAt(const std::string &val)
	{return where(B_BASE::cols::created_at, orm::wq::be, val);
	}

M_MODEL& ltCreatedAt(const std::string &val)
	{return where(B_BASE::cols::created_at, orm::wq::lt, val);
	}

M_MODEL& leCreatedAt(const std::string &val)
	{return where(B_BASE::cols::created_at, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqCreatedAt(T val)
	{return where(B_BASE::cols::created_at, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqCreatedAt(T val)
	{return where(B_BASE::cols::created_at, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btCreatedAt(T val)
	{return where(B_BASE::cols::created_at, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beCreatedAt(T val)
	{return where(B_BASE::cols::created_at, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltCreatedAt(T val)
	{return where(B_BASE::cols::created_at, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leCreatedAt(T val)
	{return where(B_BASE::cols::created_at, orm::wq::le, val);
	}

M_MODEL& nullCreatedAt()
	{return whereNull(B_BASE::cols::created_at);
	}

M_MODEL& notnullCreatedAt()
	{return whereNotNull(B_BASE::cols::created_at);
	}

M_MODEL& oreqCreatedAt(const std::string &val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::eq, val);
	}

M_MODEL& ornqCreatedAt(const std::string &val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::nq, val);
	}

M_MODEL& orbtCreatedAt(const std::string &val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::bt, val);
	}

M_MODEL& orbeCreatedAt(const std::string &val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::be, val);
	}

M_MODEL& orltCreatedAt(const std::string &val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::lt, val);
	}

M_MODEL& orleCreatedAt(const std::string &val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::le, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& oreqCreatedAt(T val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::eq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ornqCreatedAt(T val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::nq, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbtCreatedAt(T val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::bt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orbeCreatedAt(T val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::be, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orltCreatedAt(T val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::lt, val);
	}

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& orleCreatedAt(T val)
	{return whereOr(B_BASE::cols::created_at, orm::wq::le, val);
	}

M_MODEL& ornullCreatedAt()
	{return whereOrNull(B_BASE::cols::created_at);
	}

M_MODEL& ornotnullCreatedAt()
	{return whereOrNotNull(B_BASE::cols::created_at);
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
            item.filed_name  = siteinfo_info::col_names[item.col_idx];
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
            item.filed_name  = siteinfo_info::col_names[item.col_idx];
            item.filed_value = std::forward<T>(val);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        // 列名编译期校验版(orm::table_col) // 2 参数兼容: where(str, val) = where(str, orm::wq::eq, val)
        template <typename T>
        M_MODEL &where(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, T &&val)
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
        M_MODEL &whereOr(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, T &&val)
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
            item.filed_name = siteinfo_info::col_names[item.col_idx];
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
            item.filed_name = siteinfo_info::col_names[item.col_idx];
            item.filed_value.set_array();
            for (auto &v : a)
                item.filed_value.push(v);
            wheresql.push_back(std::move(item));
            return *mod;
        }

        // ===== 核心入口结束 =====

        // ===== IN / NOT IN 具名转发 =====

        template <typename T2>
        M_MODEL &whereIn(siteinfo_info::cols field, const std::vector<T2> &a)
        {
            return whereIn(field, orm::wq::in, a);
        }

        template <typename T2>
        M_MODEL &whereNotIn(siteinfo_info::cols field, const std::vector<T2> &a)
        {
            return whereIn(field, orm::wq::notin, a);
        }

        template <typename T2>
        M_MODEL &whereOrIn(siteinfo_info::cols field, const std::vector<T2> &a)
        {
            return whereOrIn(field, orm::wq::in, a);
        }

        template <typename T2>
        M_MODEL &whereOrNotIn(siteinfo_info::cols field, const std::vector<T2> &a)
        {
            return whereOrIn(field, orm::wq::notin, a);
        }

        M_MODEL &whereIn(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, const std::vector<std::string> &a)
        {
            return whereIn(wq, orm::wq::in, a);
        }
        // 2 参数旧版兼容: whereIn("id", "1,2,3") = whereIn("id", split_csv("1,2,3"))
        M_MODEL &whereIn(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view csv_val)
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
        M_MODEL &whereIn(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, const std::vector<T> &a)
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

        M_MODEL &whereNotIn(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, const std::vector<std::string> &a)
        {
            return whereIn(wq, orm::wq::notin, a);
        }

        M_MODEL &whereOrIn(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, const std::vector<std::string> &a)
        {
            return whereOrIn(wq, orm::wq::in, a);
        }

        M_MODEL &whereOrNotIn(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, const std::vector<std::string> &a)
        {
            return whereOrIn(wq, orm::wq::notin, a);
        }

        // ===== IN 具名转发结束 =====

        // ===== Null 条件（无 value）=====

        M_MODEL &whereNull(siteinfo_info::cols field)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 1;
            item.op_type    = orm::wq::isnull;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = siteinfo_info::col_names[item.col_idx];
            // filed_value 不设
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereNull(orm::table_col<B_BASE, &siteinfo_info::col_names> wq)
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

        M_MODEL &whereOrNull(siteinfo_info::cols field)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 2;
            item.op_type    = orm::wq::isnull;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = siteinfo_info::col_names[item.col_idx];
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereOrNull(orm::table_col<B_BASE, &siteinfo_info::col_names> wq)
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

        M_MODEL &whereNotNull(siteinfo_info::cols field)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 1;
            item.op_type    = orm::wq::notnull;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = siteinfo_info::col_names[item.col_idx];
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereNotNull(orm::table_col<B_BASE, &siteinfo_info::col_names> wq)
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

        M_MODEL &whereOrNotNull(siteinfo_info::cols field)
        {
            orm_where_sql_t item;
            item.pre_op     = wheresql.empty() ? 0 : 2;
            item.op_type    = orm::wq::notnull;
            item.col_idx    = static_cast<unsigned char>(field);
            item.need_quote = B_BASE::col_need_quote[static_cast<unsigned char>(field)];
            item.filed_name = siteinfo_info::col_names[item.col_idx];
            wheresql.push_back(std::move(item));
            return *mod;
        }

        M_MODEL &whereOrNotNull(orm::table_col<B_BASE, &siteinfo_info::col_names> wq)
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
        M_MODEL &whereEQ(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::eq, std::forward<T2>(value));
        }

        M_MODEL &whereEQ(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::eq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereEQ(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::eq, val);
        }

        template <typename T2>
        M_MODEL &whereAnd(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::eq, std::forward<T2>(value));
        }

        M_MODEL &whereAnd(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::eq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereAnd(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::eq, val);
        }

        template <typename T2>
        M_MODEL &whereBT(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::bt, std::forward<T2>(value));
        }

        M_MODEL &whereBT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::bt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereBT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::bt, val);
        }

        template <typename T2>
        M_MODEL &whereGT(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::bt, std::forward<T2>(value));
        }

        M_MODEL &whereGT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::bt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereGT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::bt, val);
        }

        template <typename T2>
        M_MODEL &whereBE(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::be, std::forward<T2>(value));
        }

        M_MODEL &whereBE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::be, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereBE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::be, val);
        }

        template <typename T2>
        M_MODEL &whereGE(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::be, std::forward<T2>(value));
        }

        M_MODEL &whereGE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::be, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereGE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::be, val);
        }

        template <typename T2>
        M_MODEL &whereLT(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::lt, std::forward<T2>(value));
        }

        M_MODEL &whereLT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::lt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::lt, val);
        }

        template <typename T2>
        M_MODEL &whereLE(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::le, std::forward<T2>(value));
        }

        M_MODEL &whereLE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::le, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::le, val);
        }

        template <typename T2>
        M_MODEL &whereNQ(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::nq, std::forward<T2>(value));
        }

        M_MODEL &whereNQ(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::nq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereNQ(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::nq, val);
        }

        template <typename T2>
        M_MODEL &whereNE(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::nq, std::forward<T2>(value));
        }

        M_MODEL &whereNE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::nq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereNE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::nq, val);
        }

        template <typename T2>
        M_MODEL &whereLike(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::like, std::forward<T2>(value));
        }

        M_MODEL &whereLike(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::like, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLike(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::like, val);
        }

        template <typename T2>
        M_MODEL &whereLikeLeft(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::llike, std::forward<T2>(value));
        }

        M_MODEL &whereLikeLeft(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::llike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLikeLeft(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::llike, val);
        }

        template <typename T2>
        M_MODEL &whereLikeRight(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::rlike, std::forward<T2>(value));
        }

        M_MODEL &whereLikeRight(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::rlike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereLikeRight(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::rlike, val);
        }

        template <typename T2>
        M_MODEL &whereNotLike(siteinfo_info::cols field, T2 &&value)
        {
            return where(field, orm::wq::nlike, std::forward<T2>(value));
        }

        M_MODEL &whereNotLike(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return where(wq, orm::wq::nlike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereNotLike(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return where(wq, orm::wq::nlike, val);
        }

        template <typename T2>
        M_MODEL &whereOrBT(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::bt, std::forward<T2>(value));
        }

        M_MODEL &whereOrBT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::bt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrBT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::bt, val);
        }

        template <typename T2>
        M_MODEL &whereOrBE(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::be, std::forward<T2>(value));
        }

        M_MODEL &whereOrBE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::be, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrBE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::be, val);
        }

        template <typename T2>
        M_MODEL &whereOrLT(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::lt, std::forward<T2>(value));
        }

        M_MODEL &whereOrLT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::lt, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLT(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::lt, val);
        }

        template <typename T2>
        M_MODEL &whereOrLE(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::le, std::forward<T2>(value));
        }

        M_MODEL &whereOrLE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::le, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLE(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::le, val);
        }

        template <typename T2>
        M_MODEL &whereOrNQ(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::nq, std::forward<T2>(value));
        }

        M_MODEL &whereOrNQ(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::nq, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrNQ(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::nq, val);
        }

        template <typename T2>
        M_MODEL &whereOrLike(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::like, std::forward<T2>(value));
        }

        M_MODEL &whereOrLike(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::like, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLike(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::like, val);
        }

        template <typename T2>
        M_MODEL &whereOrLikeLeft(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::llike, std::forward<T2>(value));
        }

        M_MODEL &whereOrLikeLeft(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::llike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLikeLeft(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::llike, val);
        }

        template <typename T2>
        M_MODEL &whereOrLikeRight(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::rlike, std::forward<T2>(value));
        }

        M_MODEL &whereOrLikeRight(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::rlike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrLikeRight(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
        {
            return whereOr(wq, orm::wq::rlike, val);
        }

        template <typename T2>
        M_MODEL &whereOrNotLike(siteinfo_info::cols field, T2 &&value)
        {
            return whereOr(field, orm::wq::nlike, std::forward<T2>(value));
        }

        M_MODEL &whereOrNotLike(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, std::string_view val)
        {
            return whereOr(wq, orm::wq::nlike, val);
        }

        template <typename _SQL_Value>
        M_MODEL &whereOrNotLike(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, _SQL_Value val)
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

        M_MODEL &order(siteinfo_info::cols field, const std::string &asc_or_desc)
        {
            ordersql.append(" ORDER BY ");
            switch (field)
            {
            
			case siteinfo_info::cols::sid:
				ordersql.append("sid");
				break;
			case siteinfo_info::cols::userid:
				ordersql.append("userid");
				break;
			case siteinfo_info::cols::agentid:
				ordersql.append("agentid");
				break;
			case siteinfo_info::cols::languagetype:
				ordersql.append("languagetype");
				break;
			case siteinfo_info::cols::sitename:
				ordersql.append("sitename");
				break;
			case siteinfo_info::cols::sitedomain:
				ordersql.append("sitedomain");
				break;
			case siteinfo_info::cols::metakeys:
				ordersql.append("metakeys");
				break;
			case siteinfo_info::cols::metadesc:
				ordersql.append("metadesc");
				break;
			case siteinfo_info::cols::copyright:
				ordersql.append("copyright");
				break;
			case siteinfo_info::cols::beiansn:
				ordersql.append("beiansn");
				break;
			case siteinfo_info::cols::footscript:
				ordersql.append("footscript");
				break;
			case siteinfo_info::cols::headscript:
				ordersql.append("headscript");
				break;
			case siteinfo_info::cols::introduce:
				ordersql.append("introduce");
				break;
			case siteinfo_info::cols::sitelogo:
				ordersql.append("sitelogo");
				break;
			case siteinfo_info::cols::sitebanner:
				ordersql.append("sitebanner");
				break;
			case siteinfo_info::cols::contactman:
				ordersql.append("contactman");
				break;
			case siteinfo_info::cols::phone:
				ordersql.append("phone");
				break;
			case siteinfo_info::cols::mobile:
				ordersql.append("mobile");
				break;
			case siteinfo_info::cols::email:
				ordersql.append("email");
				break;
			case siteinfo_info::cols::bankname:
				ordersql.append("bankname");
				break;
			case siteinfo_info::cols::banksn:
				ordersql.append("banksn");
				break;
			case siteinfo_info::cols::address:
				ordersql.append("address");
				break;
			case siteinfo_info::cols::zipnum:
				ordersql.append("zipnum");
				break;
			case siteinfo_info::cols::taxsn:
				ordersql.append("taxsn");
				break;
			case siteinfo_info::cols::companyname:
				ordersql.append("companyname");
				break;
			case siteinfo_info::cols::linkname:
				ordersql.append("linkname");
				break;
			case siteinfo_info::cols::linkmobile:
				ordersql.append("linkmobile");
				break;
			case siteinfo_info::cols::linkaddress:
				ordersql.append("linkaddress");
				break;
			case siteinfo_info::cols::theme:
				ordersql.append("theme");
				break;
			case siteinfo_info::cols::sitepath:
				ordersql.append("sitepath");
				break;
			case siteinfo_info::cols::isopen:
				ordersql.append("isopen");
				break;
			case siteinfo_info::cols::created_at:
				ordersql.append("created_at");
				break;
			case siteinfo_info::cols::enddate:
				ordersql.append("enddate");
				break;
            default:
                return *mod;
                break;
            }
            ordersql.append(asc_or_desc);
            return *mod;
        }

        M_MODEL &asc(siteinfo_info::cols field)
        {
            ordersql.append(" ORDER BY ");
            switch (field)
            {
            
			case siteinfo_info::cols::sid:
				ordersql.append("sid");
				break;
			case siteinfo_info::cols::userid:
				ordersql.append("userid");
				break;
			case siteinfo_info::cols::agentid:
				ordersql.append("agentid");
				break;
			case siteinfo_info::cols::languagetype:
				ordersql.append("languagetype");
				break;
			case siteinfo_info::cols::sitename:
				ordersql.append("sitename");
				break;
			case siteinfo_info::cols::sitedomain:
				ordersql.append("sitedomain");
				break;
			case siteinfo_info::cols::metakeys:
				ordersql.append("metakeys");
				break;
			case siteinfo_info::cols::metadesc:
				ordersql.append("metadesc");
				break;
			case siteinfo_info::cols::copyright:
				ordersql.append("copyright");
				break;
			case siteinfo_info::cols::beiansn:
				ordersql.append("beiansn");
				break;
			case siteinfo_info::cols::footscript:
				ordersql.append("footscript");
				break;
			case siteinfo_info::cols::headscript:
				ordersql.append("headscript");
				break;
			case siteinfo_info::cols::introduce:
				ordersql.append("introduce");
				break;
			case siteinfo_info::cols::sitelogo:
				ordersql.append("sitelogo");
				break;
			case siteinfo_info::cols::sitebanner:
				ordersql.append("sitebanner");
				break;
			case siteinfo_info::cols::contactman:
				ordersql.append("contactman");
				break;
			case siteinfo_info::cols::phone:
				ordersql.append("phone");
				break;
			case siteinfo_info::cols::mobile:
				ordersql.append("mobile");
				break;
			case siteinfo_info::cols::email:
				ordersql.append("email");
				break;
			case siteinfo_info::cols::bankname:
				ordersql.append("bankname");
				break;
			case siteinfo_info::cols::banksn:
				ordersql.append("banksn");
				break;
			case siteinfo_info::cols::address:
				ordersql.append("address");
				break;
			case siteinfo_info::cols::zipnum:
				ordersql.append("zipnum");
				break;
			case siteinfo_info::cols::taxsn:
				ordersql.append("taxsn");
				break;
			case siteinfo_info::cols::companyname:
				ordersql.append("companyname");
				break;
			case siteinfo_info::cols::linkname:
				ordersql.append("linkname");
				break;
			case siteinfo_info::cols::linkmobile:
				ordersql.append("linkmobile");
				break;
			case siteinfo_info::cols::linkaddress:
				ordersql.append("linkaddress");
				break;
			case siteinfo_info::cols::theme:
				ordersql.append("theme");
				break;
			case siteinfo_info::cols::sitepath:
				ordersql.append("sitepath");
				break;
			case siteinfo_info::cols::isopen:
				ordersql.append("isopen");
				break;
			case siteinfo_info::cols::created_at:
				ordersql.append("created_at");
				break;
			case siteinfo_info::cols::enddate:
				ordersql.append("enddate");
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
        M_MODEL &desc(siteinfo_info::cols field)
        {
            ordersql.append(" ORDER BY ");
            switch (field)
            {
            
			case siteinfo_info::cols::sid:
				ordersql.append("sid");
				break;
			case siteinfo_info::cols::userid:
				ordersql.append("userid");
				break;
			case siteinfo_info::cols::agentid:
				ordersql.append("agentid");
				break;
			case siteinfo_info::cols::languagetype:
				ordersql.append("languagetype");
				break;
			case siteinfo_info::cols::sitename:
				ordersql.append("sitename");
				break;
			case siteinfo_info::cols::sitedomain:
				ordersql.append("sitedomain");
				break;
			case siteinfo_info::cols::metakeys:
				ordersql.append("metakeys");
				break;
			case siteinfo_info::cols::metadesc:
				ordersql.append("metadesc");
				break;
			case siteinfo_info::cols::copyright:
				ordersql.append("copyright");
				break;
			case siteinfo_info::cols::beiansn:
				ordersql.append("beiansn");
				break;
			case siteinfo_info::cols::footscript:
				ordersql.append("footscript");
				break;
			case siteinfo_info::cols::headscript:
				ordersql.append("headscript");
				break;
			case siteinfo_info::cols::introduce:
				ordersql.append("introduce");
				break;
			case siteinfo_info::cols::sitelogo:
				ordersql.append("sitelogo");
				break;
			case siteinfo_info::cols::sitebanner:
				ordersql.append("sitebanner");
				break;
			case siteinfo_info::cols::contactman:
				ordersql.append("contactman");
				break;
			case siteinfo_info::cols::phone:
				ordersql.append("phone");
				break;
			case siteinfo_info::cols::mobile:
				ordersql.append("mobile");
				break;
			case siteinfo_info::cols::email:
				ordersql.append("email");
				break;
			case siteinfo_info::cols::bankname:
				ordersql.append("bankname");
				break;
			case siteinfo_info::cols::banksn:
				ordersql.append("banksn");
				break;
			case siteinfo_info::cols::address:
				ordersql.append("address");
				break;
			case siteinfo_info::cols::zipnum:
				ordersql.append("zipnum");
				break;
			case siteinfo_info::cols::taxsn:
				ordersql.append("taxsn");
				break;
			case siteinfo_info::cols::companyname:
				ordersql.append("companyname");
				break;
			case siteinfo_info::cols::linkname:
				ordersql.append("linkname");
				break;
			case siteinfo_info::cols::linkmobile:
				ordersql.append("linkmobile");
				break;
			case siteinfo_info::cols::linkaddress:
				ordersql.append("linkaddress");
				break;
			case siteinfo_info::cols::theme:
				ordersql.append("theme");
				break;
			case siteinfo_info::cols::sitepath:
				ordersql.append("sitepath");
				break;
			case siteinfo_info::cols::isopen:
				ordersql.append("isopen");
				break;
			case siteinfo_info::cols::created_at:
				ordersql.append("created_at");
				break;
			case siteinfo_info::cols::enddate:
				ordersql.append("enddate");
				break;
            default:
                return *mod;
                break;
            }
            ordersql.append(" DESC ");
            return *mod;
        }

        M_MODEL &order(orm::table_col<B_BASE, &siteinfo_info::col_names> wq, const std::string &asc_or_desc)
        {
            ordersql.append(" ORDER BY ");
            ordersql.append(wq);
            ordersql.append(" ");
            ordersql.append(asc_or_desc);
            return *mod;
        }
        M_MODEL &asc(orm::table_col<B_BASE, &siteinfo_info::col_names> wq)
        {

            ordersql.append(" ORDER BY ");
            ordersql.append(wq);
            ordersql.append(" ASC ");
            return *mod;
        }

        M_MODEL &desc(orm::table_col<B_BASE, &siteinfo_info::col_names> wq)
        {

            ordersql.append(" ORDER BY ");
            ordersql.append(wq);
            ordersql.append(" DESC ");
            return *mod;
        }

        M_MODEL &having(orm::table_col<B_BASE, &siteinfo_info::col_names> wq)
        {
            groupsql.append(" HAVING ");
            groupsql.append(wq);
            return *mod;
        }

        M_MODEL &having(siteinfo_info::cols field)
        {
            groupsql.append(" HAVING ");
            switch (field)
            {
            
			case siteinfo_info::cols::sid:
				groupsql.append("sid");
				break;
			case siteinfo_info::cols::userid:
				groupsql.append("userid");
				break;
			case siteinfo_info::cols::agentid:
				groupsql.append("agentid");
				break;
			case siteinfo_info::cols::languagetype:
				groupsql.append("languagetype");
				break;
			case siteinfo_info::cols::sitename:
				groupsql.append("sitename");
				break;
			case siteinfo_info::cols::sitedomain:
				groupsql.append("sitedomain");
				break;
			case siteinfo_info::cols::metakeys:
				groupsql.append("metakeys");
				break;
			case siteinfo_info::cols::metadesc:
				groupsql.append("metadesc");
				break;
			case siteinfo_info::cols::copyright:
				groupsql.append("copyright");
				break;
			case siteinfo_info::cols::beiansn:
				groupsql.append("beiansn");
				break;
			case siteinfo_info::cols::footscript:
				groupsql.append("footscript");
				break;
			case siteinfo_info::cols::headscript:
				groupsql.append("headscript");
				break;
			case siteinfo_info::cols::introduce:
				groupsql.append("introduce");
				break;
			case siteinfo_info::cols::sitelogo:
				groupsql.append("sitelogo");
				break;
			case siteinfo_info::cols::sitebanner:
				groupsql.append("sitebanner");
				break;
			case siteinfo_info::cols::contactman:
				groupsql.append("contactman");
				break;
			case siteinfo_info::cols::phone:
				groupsql.append("phone");
				break;
			case siteinfo_info::cols::mobile:
				groupsql.append("mobile");
				break;
			case siteinfo_info::cols::email:
				groupsql.append("email");
				break;
			case siteinfo_info::cols::bankname:
				groupsql.append("bankname");
				break;
			case siteinfo_info::cols::banksn:
				groupsql.append("banksn");
				break;
			case siteinfo_info::cols::address:
				groupsql.append("address");
				break;
			case siteinfo_info::cols::zipnum:
				groupsql.append("zipnum");
				break;
			case siteinfo_info::cols::taxsn:
				groupsql.append("taxsn");
				break;
			case siteinfo_info::cols::companyname:
				groupsql.append("companyname");
				break;
			case siteinfo_info::cols::linkname:
				groupsql.append("linkname");
				break;
			case siteinfo_info::cols::linkmobile:
				groupsql.append("linkmobile");
				break;
			case siteinfo_info::cols::linkaddress:
				groupsql.append("linkaddress");
				break;
			case siteinfo_info::cols::theme:
				groupsql.append("theme");
				break;
			case siteinfo_info::cols::sitepath:
				groupsql.append("sitepath");
				break;
			case siteinfo_info::cols::isopen:
				groupsql.append("isopen");
				break;
			case siteinfo_info::cols::created_at:
				groupsql.append("created_at");
				break;
			case siteinfo_info::cols::enddate:
				groupsql.append("enddate");
				break;
            default:
                return *mod;
                break;
            }
            return *mod;
        }

        M_MODEL &group(siteinfo_info::cols field)
        {
            groupsql.append(" GROUP BY ");
            switch (field)
            {
            
			case siteinfo_info::cols::sid:
				groupsql.append("sid");
				break;
			case siteinfo_info::cols::userid:
				groupsql.append("userid");
				break;
			case siteinfo_info::cols::agentid:
				groupsql.append("agentid");
				break;
			case siteinfo_info::cols::languagetype:
				groupsql.append("languagetype");
				break;
			case siteinfo_info::cols::sitename:
				groupsql.append("sitename");
				break;
			case siteinfo_info::cols::sitedomain:
				groupsql.append("sitedomain");
				break;
			case siteinfo_info::cols::metakeys:
				groupsql.append("metakeys");
				break;
			case siteinfo_info::cols::metadesc:
				groupsql.append("metadesc");
				break;
			case siteinfo_info::cols::copyright:
				groupsql.append("copyright");
				break;
			case siteinfo_info::cols::beiansn:
				groupsql.append("beiansn");
				break;
			case siteinfo_info::cols::footscript:
				groupsql.append("footscript");
				break;
			case siteinfo_info::cols::headscript:
				groupsql.append("headscript");
				break;
			case siteinfo_info::cols::introduce:
				groupsql.append("introduce");
				break;
			case siteinfo_info::cols::sitelogo:
				groupsql.append("sitelogo");
				break;
			case siteinfo_info::cols::sitebanner:
				groupsql.append("sitebanner");
				break;
			case siteinfo_info::cols::contactman:
				groupsql.append("contactman");
				break;
			case siteinfo_info::cols::phone:
				groupsql.append("phone");
				break;
			case siteinfo_info::cols::mobile:
				groupsql.append("mobile");
				break;
			case siteinfo_info::cols::email:
				groupsql.append("email");
				break;
			case siteinfo_info::cols::bankname:
				groupsql.append("bankname");
				break;
			case siteinfo_info::cols::banksn:
				groupsql.append("banksn");
				break;
			case siteinfo_info::cols::address:
				groupsql.append("address");
				break;
			case siteinfo_info::cols::zipnum:
				groupsql.append("zipnum");
				break;
			case siteinfo_info::cols::taxsn:
				groupsql.append("taxsn");
				break;
			case siteinfo_info::cols::companyname:
				groupsql.append("companyname");
				break;
			case siteinfo_info::cols::linkname:
				groupsql.append("linkname");
				break;
			case siteinfo_info::cols::linkmobile:
				groupsql.append("linkmobile");
				break;
			case siteinfo_info::cols::linkaddress:
				groupsql.append("linkaddress");
				break;
			case siteinfo_info::cols::theme:
				groupsql.append("theme");
				break;
			case siteinfo_info::cols::sitepath:
				groupsql.append("sitepath");
				break;
			case siteinfo_info::cols::isopen:
				groupsql.append("isopen");
				break;
			case siteinfo_info::cols::created_at:
				groupsql.append("created_at");
				break;
			case siteinfo_info::cols::enddate:
				groupsql.append("enddate");
				break;
            default:
                return *mod;
                break;
            }
            return *mod;
        }
        M_MODEL &group(orm::table_col<B_BASE, &siteinfo_info::col_names> wq)
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
                                                                           siteinfo_info::meta data_temp;
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
                                                                                          siteinfo_info::meta data_temp;
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
                                                                           siteinfo_info::meta data_temp;
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
                                                                                          siteinfo_info::meta data_temp;
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
                                                                               siteinfo_info::meta data_temp;
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
                                                                                              siteinfo_info::meta data_temp;
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
            model_meta_cache<siteinfo_info::meta> &data_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
            data_cache.remove_exptime();

            model_meta_cache<std::vector<siteinfo_info::meta>> &record_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            record_cache.remove_exptime();
        }
        void clear_cache()
        {
            model_meta_cache<siteinfo_info::meta> &data_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
            data_cache.clear();

            model_meta_cache<std::vector<siteinfo_info::meta>> &record_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            record_cache.clear();
        }
        bool remove_cache()
        {
            std::size_t sqlhashid = std::hash<std::string>{}(sqlstring);

            model_meta_cache<siteinfo_info::meta> &data_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
            bool state = data_cache.remove(sqlhashid);

            model_meta_cache<std::vector<siteinfo_info::meta>> &record_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            return record_cache.remove(sqlhashid) || state;
        }
        bool remove_cache(std::size_t cache_key_name)
        {
            model_meta_cache<siteinfo_info::meta> &data_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
            bool state = data_cache.remove(cache_key_name);

            model_meta_cache<std::vector<siteinfo_info::meta>> &record_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            return record_cache.remove(cache_key_name) || state;
        }
        int check_cache(std::size_t cache_key_name)
        {
            model_meta_cache<siteinfo_info::meta> &temp_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
            return temp_cache.check(cache_key_name);
        }

        bool get_data_cache(std::size_t cache_key_name)
        {
            try
            {
                model_meta_cache<siteinfo_info::meta> &temp_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
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
            model_meta_cache<std::vector<siteinfo_info::meta>> &temp_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            return temp_cache.update(sqlhashid, exp_time);
        }
        int update_cache(std::size_t cache_key_name, int exp_time)
        {
            model_meta_cache<std::vector<siteinfo_info::meta>> &temp_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            return temp_cache.update(cache_key_name, exp_time);
        }
        bool save_cache(int exp_time = 0)
        {
            model_meta_cache<std::vector<siteinfo_info::meta>> &temp_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::record, exp_time);
            return true;
        }

        bool save_data_cache(int exp_time = 0)
        {
            model_meta_cache<siteinfo_info::meta> &temp_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::data, exp_time);
            return true;
        }

        bool save_data_cache(const std::string &cache_key_name, const siteinfo_info::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<siteinfo_info::meta> &temp_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }

        bool save_cache(std::size_t cache_key_name, const std::vector<siteinfo_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<siteinfo_info::meta>> &temp_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            temp_cache.save(cache_key_name, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const std::vector<siteinfo_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<siteinfo_info::meta>> &temp_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_vector_cache(const std::string cache_key_name, const std::vector<siteinfo_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<siteinfo_info::meta>> &temp_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const siteinfo_info::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<siteinfo_info::meta> &temp_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        const siteinfo_info::meta get_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<siteinfo_info::meta> &temp_cache = model_meta_cache<siteinfo_info::meta>::getinstance();
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

        const std::vector<siteinfo_info::meta> get_vector_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<std::vector<siteinfo_info::meta>> &temp_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
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
                model_meta_cache<std::vector<siteinfo_info::meta>> &temp_cache = model_meta_cache<std::vector<siteinfo_info::meta>>::getinstance();
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
        std::tuple<unsigned int, unsigned long long> insert(siteinfo_info::meta &insert_data)
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

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(siteinfo_info::meta &insert_data)
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

        std::tuple<unsigned int, unsigned long long> insert(std::vector<siteinfo_info::meta> &insert_data)
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

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(std::vector<siteinfo_info::meta> &insert_data)
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
            item.left_filed_name  = siteinfo_info::col_names[item.left_idx];
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

        std::string commit_insert(siteinfo_info::meta &insert_data)
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
        M_MODEL &set_data(siteinfo_info::meta indata)
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
        ~siteinfo_opsql()
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
        siteinfo_opsql(const siteinfo_opsql &)            = delete;
        siteinfo_opsql &operator=(const siteinfo_opsql &) = delete;
        siteinfo_opsql(siteinfo_opsql &&o) noexcept : B_BASE(std::move(o))
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
        siteinfo_opsql &operator=(siteinfo_opsql &&o) noexcept
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
        M_MODEL &AND(orm::table_col<B_BASE, &siteinfo_info::col_names> field, orm::wq opwq, T val)
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
        M_MODEL &OR(orm::table_col<B_BASE, &siteinfo_info::col_names> field, orm::wq opwq, T val)
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
            item.filed_name  = siteinfo_info::col_names[item.col_idx];
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
            item.filed_name  = siteinfo_info::col_names[item.col_idx];
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
                    siteinfo_info::meta data_temp;
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
                    siteinfo_info::meta data_temp;
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
            for (unsigned char i = 0; i < siteinfo_info::col_names.size(); i++)
            {
                // 跳过自增主键：全字段更新不修改身份列（“自增主键跳过”约定），避免把主键误改为 0
                if (static_cast<int>(i) == siteinfo_info::auto_pk_index)
                    continue;
                if (!first)
                    sql.append(", ");
                sql.append(siteinfo_info::col_names[i]);
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
            for (unsigned char i = 0; i < siteinfo_info::col_names.size(); i++)
            {
                // 跳过自增主键：全字段更新不修改身份列（“自增主键跳过”约定），避免把主键误改为 0
                if (static_cast<int>(i) == siteinfo_info::auto_pk_index)
                    continue;
                if (!first)
                    sql.append(", ");
                sql.append(siteinfo_info::col_names[i]);
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
                sql.append(siteinfo_info::col_names[idx]);
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
                sql.append(siteinfo_info::col_names[idx]);
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
            for (unsigned char i = 0; i < siteinfo_info::col_names.size(); i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(siteinfo_info::col_names[i]);
                params.push_back(get_insert_field_value(i, B_BASE::data));
            }
            sql.append(") VALUES (");
            for (unsigned char i = 0; i < siteinfo_info::col_names.size(); i++)
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
            for (unsigned char i = 0; i < siteinfo_info::col_names.size(); i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(siteinfo_info::col_names[i]);
                params.push_back(get_insert_field_value(i, B_BASE::data));
            }
            sql.append(") VALUES (");
            for (unsigned char i = 0; i < siteinfo_info::col_names.size(); i++)
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

            const unsigned char ncols = siteinfo_info::col_names.size();
            const size_t nrows        = B_BASE::record.size();

            std::string sql = "INSERT INTO ";
            sql.append(B_BASE::tablename);
            sql.append(" (");
            for (unsigned char i = 0; i < ncols; i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(siteinfo_info::col_names[i]);
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

            const unsigned char ncols = siteinfo_info::col_names.size();
            const size_t nrows        = B_BASE::record.size();

            std::string sql = "INSERT INTO ";
            sql.append(B_BASE::tablename);
            sql.append(" (");
            for (unsigned char i = 0; i < ncols; i++)
            {
                if (i > 0)
                    sql.append(", ");
                sql.append(siteinfo_info::col_names[i]);
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
