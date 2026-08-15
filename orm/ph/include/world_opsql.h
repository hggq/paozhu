#ifndef _ORM_PH_WORLD_OPERATE_H
#define _ORM_PH_WORLD_OPERATE_H
/*
 *  @author 黄自权 huangziquan
 *  @date 2022-05-04
 *  @update 2025-03-12
 *  @update 2026-06-14 add xxx_fetch_to, leftjoin
 *  @dest ORM PostgreSQL中间连接层
 *  本文件自动生成 This document is automatically generated.
 *  Creation time Sat, 15 Aug 2026 09:48:08 GMT
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

#include "pg_conn.h"
#include "orm_conn_pool.h"
#include "orm_cache.hpp"
#include "orm_connect_mar.h"
#include "world_base.h"
/*baseincludefile*/
namespace orm
{
// mysql Operational SQL middleware
namespace ph
{ /*tagnamespace_replace*/
    template <typename M_MODEL, typename B_BASE>
    class world_opsql : public B_BASE
    {
      public:
        world_opsql(const std::string &tag) : dbtag(tag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                //必须是 DB_TYPE::POSTGRESQL 类型链接
                if (iter->second->conf_data[0].db_type != DB_TYPE::POSTGRESQL)
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
        world_opsql() : dbtag(B_BASE::_rmstag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                //必须是 DB_TYPE::POSTGRESQL 类型链接
                if (iter->second->conf_data[0].db_type != DB_TYPE::POSTGRESQL)
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
                //必须是 DB_TYPE::POSTGRESQL 类型链接
                if (iter->second->conf_data[0].db_type != DB_TYPE::POSTGRESQL)
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
                error_msg = "conn_pool not found " + temptag;
            }
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
            std::string countsql;
            countsql = "SELECT count(*) as total_countnum  FROM ";
            countsql.append(B_BASE::tablename);
            countsql.append(" WHERE ");
            if (wheresql.empty())
            {
                countsql.append(" 1 ");
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(countsql, fields, rows, affected);
                if (err > 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                unsigned int querysql_len = 0;
                if (!rows.empty() && !rows[0].values.empty() && !rows[0].is_null[0])
                {
                    const std::string &val = rows[0].values[0];
                    for (unsigned int ik = 0; ik < val.size(); ik++)
                    {
                        if (val[ik] >= '0' && val[ik] <= '9')
                        {
                            querysql_len = querysql_len * 10 + (val[ik] - '0');
                        }
                    }
                }
                effect_num = 1;

                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_pg_select_conn(std::move(select_conn));
                }

                return querysql_len;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }

            return 0;
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
        asio::awaitable<unsigned int> async_count()
        {
            std::string countsql;
            countsql = "SELECT count(*) as total_countnum  FROM ";
            countsql.append(B_BASE::tablename);
            countsql.append(" WHERE ");
            if (wheresql.empty())
            {
                countsql.append(" 1 ");
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(countsql, fields, rows, affected);
                if (err > 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                unsigned int querysql_len = 0;
                if (!rows.empty() && !rows[0].values.empty() && !rows[0].is_null[0])
                {
                    const std::string &val = rows[0].values[0];
                    for (unsigned int ik = 0; ik < val.size(); ik++)
                    {
                        if (val[ik] >= '0' && val[ik] <= '9')
                        {
                            querysql_len = querysql_len * 10 + (val[ik] - '0');
                        }
                    }
                }
                effect_num = 1;

                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_pg_select_conn(std::move(select_conn));
                }
                co_return querysql_len;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }

            co_return 0;
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

        unsigned int update_col(std::string colname, int num, char symbol = '+')
        {
            effect_num = 0;
            std::string countsql;
            countsql = "UPDATE ";
            countsql.append(B_BASE::tablename);
            countsql.append(" SET ");
            countsql.append(colname);
            if (num > 0)
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.push_back(' ');
                countsql.push_back(symbol);
                countsql.append(std::to_string(num));
            }
            else
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.push_back(' ');
                countsql.push_back(symbol);
                countsql.push_back('(');
                countsql.push_back('-');
                countsql.append(std::to_string(std::abs(num)));
                countsql.push_back(')');
            }
            countsql.append(" where ");
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    countsql.append(tempsql.str());
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
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
                //auto conn = conn_obj->get_pg_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(countsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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

        asio::awaitable<unsigned int> async_update_col(std::string colname, int num, char symbol = '+')
        {
            effect_num = 0;
            std::string countsql;
            countsql = "UPDATE ";
            countsql.append(B_BASE::tablename);
            countsql.append(" SET ");
            countsql.append(colname);
            if (num > 0)
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.push_back(' ');
                countsql.push_back(symbol);
                countsql.append(std::to_string(num));
            }
            else
            {
                countsql.append(" = ");
                countsql.append(colname);
                countsql.push_back(' ');
                countsql.push_back(symbol);
                countsql.push_back('(');
                countsql.push_back('-');
                countsql.append(std::to_string(std::abs(num)));
                countsql.push_back(')');
            }
            countsql.append(" where ");
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    countsql.append(tempsql.str());
                }
                else
                {
                    co_return 0;
                }
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(countsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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

        int replace_col(std::string colname, const std::string &old_string, const std::string &new_string)
        {
            effect_num = 0;
            std::string countsql;
            countsql = "UPDATE ";
            countsql.append(B_BASE::tablename);
            countsql.append(" SET ");
            countsql.append(colname);

            countsql.append(" = REPLACE(");
            countsql.append(colname);
            countsql.append(",'");
            countsql.append(old_string);
            countsql.append("','");
            countsql.append(new_string);
            countsql.append("') ");

            countsql.append(" where ");
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    countsql.append(tempsql.str());
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
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
                //auto conn = conn_obj->get_pg_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(countsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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

        asio::awaitable<unsigned int> async_replace_col(std::string colname, std::string_view old_string, std::string_view new_string)
        {
            effect_num = 0;
            std::string countsql;
            countsql = "UPDATE ";
            countsql.append(B_BASE::tablename);
            countsql.append(" SET ");
            countsql.append(colname);

            countsql.append(" = REPLACE(");
            countsql.append(colname);
            countsql.append(",'");
            countsql.append(old_string);
            countsql.append("','");
            countsql.append(new_string);
            countsql.append("') ");

            countsql.append(" where ");
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    countsql.append(tempsql.str());
                }
                else
                {
                    co_return 0;
                }
            }
            else
            {
                countsql.append(wheresql);
            }
            if (!groupsql.empty())
            {
                countsql.append(groupsql);
            }
            if (!limitsql.empty())
            {
                countsql.append(limitsql);
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
                //auto conn = conn_obj->get_pg_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(countsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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

        void assign_field_value(unsigned char index_pos, unsigned char *result_temp_data, unsigned long long value_size, world_info::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0:
             {
               data_temp.id = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.id);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.id = 0;
                        }
            }
            break;
                case 1:
             {
               data_temp.randomnumber = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.randomnumber);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.randomnumber = 0;
                        }
            }
            break;
                
        }
    }
    

M_MODEL& eqId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id IN('");

        wheresql.append(B_BASE::stringaddslash(val));
        wheresql.push_back('\'');
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inId(const T &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id IN(");

        wheresql.append(std::to_string(val));
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inId(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inId(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id NOT IN('");

        wheresql.append(B_BASE::stringaddslash(val));
        wheresql.push_back('\'');
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninId(const T &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id NOT IN(");

        wheresql.append(std::to_string(val));
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninId(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninId(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id IN('");

        wheresql.append(B_BASE::stringaddslash(val));
        wheresql.push_back('\'');
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inId(const T &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id IN(");

        wheresql.append(std::to_string(val));
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inId(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inId(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id NOT IN('");

        wheresql.append(B_BASE::stringaddslash(val));
        wheresql.push_back('\'');
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninId(const T &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id NOT IN(");

        wheresql.append(std::to_string(val));
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninId(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninId(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" id <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& nqRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& inRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber IN('");

        wheresql.append(B_BASE::stringaddslash(val));
        wheresql.push_back('\'');
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inRandomnumber(const T &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber IN(");

        wheresql.append(std::to_string(val));
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inRandomnumber(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inRandomnumber(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber NOT IN('");

        wheresql.append(B_BASE::stringaddslash(val));
        wheresql.push_back('\'');
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninRandomnumber(const T &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber NOT IN(");

        wheresql.append(std::to_string(val));
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninRandomnumber(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninRandomnumber(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& btRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& beRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& ltRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& leRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_eqRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber = ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_nqRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber != ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_inRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber IN('");

        wheresql.append(B_BASE::stringaddslash(val));
        wheresql.push_back('\'');
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inRandomnumber(const T &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber IN(");

        wheresql.append(std::to_string(val));
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inRandomnumber(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inRandomnumber(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber NOT IN('");

        wheresql.append(B_BASE::stringaddslash(val));
        wheresql.push_back('\'');
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninRandomnumber(const T &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber NOT IN(");

        wheresql.append(std::to_string(val));
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninRandomnumber(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }
            wheresql.append(std::to_string(val[i]));
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninRandomnumber(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i > 0)
            {
                wheresql.push_back(',');
            }

            try
            {
                wheresql.append(std::to_string(std::stoll(val[i])));
            }
            catch (std::invalid_argument const& ex)
            {
                wheresql.push_back('0');
            }
            catch (std::out_of_range const& ex)
            {
                wheresql.push_back('0');
            }
        }
        wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_btRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber > ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_beRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber >= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_ltRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber < ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

M_MODEL& or_leRandomnumber(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber <= ");

        try
        {
            wheresql.append(std::to_string(std::stoll(val)));
        }
        catch (std::invalid_argument const& ex)
        {
           wheresql.push_back('0');
        }
        catch (std::out_of_range const& ex)
        {
           wheresql.push_back('0');
        }
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& eqRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leRandomnumber(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" OR ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" OR ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" randomnumber <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
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

        M_MODEL &where(std::string_view wq)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(wq);
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &where(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('=');
            wheresql.append(std::to_string(val));
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &where(std::string_view wq, char bi, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back(bi);
            wheresql.append(std::to_string(val));
            return *mod;
        }

        M_MODEL &where(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('=');

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &whereBT(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('>');

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &whereBE(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(">=");

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &whereLT(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" < ");

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &whereLE(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" <= ");

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &betWeen(std::string_view &wq, _SQL_Value a, _SQL_Value b)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(" ");
            wheresql.append(wq);
            wheresql.append(" BETWEEN ");
            wheresql.append(a);
            wheresql.append(" AND ");
            wheresql.append(b);
            wheresql.append(" ");
            return *mod;
        }

        M_MODEL &orBetWeen(std::string_view wq, std::string_view a, std::string_view b)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(" ");
            wheresql.append(wq);
            wheresql.append(" BETWEEN ");
            wheresql.append(a);
            wheresql.append(" AND ");
            wheresql.append(b);
            wheresql.append(" ");
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &orBetWeen(std::string_view wq, _SQL_Value a, _SQL_Value b)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(" ");
            wheresql.append(wq);
            wheresql.append(" BETWEEN ");
            wheresql.append(std::to_string(a));
            wheresql.append(" AND ");
            wheresql.append(std::to_string(b));
            wheresql.append(" ");
            return *mod;
        }
        M_MODEL &whereLike(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" like '");
            if (val.size() > 0 && (val[0] == '%' || val.back() == '%'))
            {
                wheresql.append(B_BASE::stringaddslash(val));
                wheresql.append("' ");
            }
            else
            {
                wheresql.push_back('%');
                wheresql.append(B_BASE::stringaddslash(val));
                wheresql.append("%' ");
            }
            return *mod;
        }
        M_MODEL &whereLikeLeft(std::string_view wq, std::string_view val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" like '");
            wheresql.push_back('%');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.append("' ");
            return *mod;
        }
        M_MODEL &whereLikeRight(std::string_view wq, std::string_view val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" like '");
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.append("%' ");
            return *mod;
        }
        M_MODEL &whereOrLike(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" like '");
            if (val[0] == '%' || val.back() == '%')
            {
                wheresql.append(B_BASE::stringaddslash(val));
                wheresql.append("' ");
            }
            else
            {
                wheresql.push_back('%');
                wheresql.append(B_BASE::stringaddslash(val));
                wheresql.append("%' ");
            }
            return *mod;
        }
        M_MODEL &whereAnd(std::string_view wq)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereAnd(std::string_view wq, _SQL_Value val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('=');
            wheresql.append(std::to_string(val));
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereBT(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" > ");
            wheresql.append(std::to_string(val));
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereBE(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" >= ");
            wheresql.append(std::to_string(val));
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereLT(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" < ");
            wheresql.append(std::to_string(val));
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereLE(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" <= ");
            wheresql.append(std::to_string(val));
            return *mod;
        }
        //where and
        M_MODEL &whereEQ(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('=');
            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');

            return *mod;
        }

        M_MODEL &whereAnd(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('=');
            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');

            return *mod;
        }
        //where string or

        M_MODEL &whereOrBT(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('>');

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &whereOrBE(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(">=");

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &whereOrLT(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" < ");

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &whereOrLE(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" <= ");

            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }

        //where or
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereOrBT(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" > ");
            wheresql.append(val);
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereOrBE(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" >= ");
            wheresql.append(val);
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereOrLT(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" < ");
            wheresql.append(val);
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereOrLE(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.append(" <= ");
            wheresql.append(val);
            return *mod;
        }

        M_MODEL &whereOr(std::string_view wq)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereOr(std::string_view wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('=');
            wheresql.append(std::to_string(val));
            return *mod;
        }
        M_MODEL &whereOr(std::string_view wq, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(wq);
            wheresql.push_back('=');
            wheresql.push_back('\'');
            wheresql.append(B_BASE::stringaddslash(val));
            wheresql.push_back('\'');
            return *mod;
        }
        M_MODEL &whereIn(std::string_view k)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(k);
            return *mod;
        }
        M_MODEL &whereIn(std::string_view k, std::string_view val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" IN(");
            {
                std::string _in_part;
                for (char _c : val)
                {
                    if (_c == ',')
                    {
                        wheresql.append(B_BASE::stringaddslash(_in_part));
                        wheresql.push_back('\'');
                        wheresql.push_back(',');
                        _in_part.clear();
                    }
                    else
                        _in_part.push_back(_c);
                }
                wheresql.append(B_BASE::stringaddslash(_in_part));
                wheresql.push_back('\'');
            }
            wheresql.append(") ");
            return *mod;
        }

        M_MODEL &whereIn(std::string_view k, const std::vector<std::string> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",\'");
                }
                else
                {
                    wheresql.append("\'");
                }
                wheresql.append(key);
                wheresql.append("\'");
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        M_MODEL &whereNotIn(std::string_view k, const std::vector<std::string> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",\'");
                }
                else
                {
                    wheresql.append("\'");
                }
                wheresql.append(key);
                wheresql.append("\'");
                i++;
            }
            wheresql.append(") ");
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereIn(std::string_view k, const std::list<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" in(");
            int i = 0;
            std::stringstream _stream;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                _stream << key;
                wheresql.append(_stream.str());
                i++;
                _stream.str("");
            }
            wheresql.append(") ");
            return *mod;
        }

        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereIn(std::string_view k, const std::vector<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" IN(");
            int i = 0;
            std::stringstream _stream;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                _stream << key;
                wheresql.append(_stream.str());
                i++;
                _stream.str("");
            }
            wheresql.append(") ");
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &whereNotIn(std::string_view k, const std::vector<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(k);
            wheresql.append(" NOT IN(");
            int i = 0;
            std::stringstream _stream;
            for (auto &key : a)
            {
                if (i > 0)
                {
                    wheresql.append(",");
                }
                _stream << key;
                wheresql.append(_stream.str());
                i++;
                _stream.str("");
            }
            wheresql.append(") ");
            return *mod;
        }

        template <typename T2>
        M_MODEL &where(std::string_view field1, orm::wq opwq, T2 &&field2)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(field1);

            if (opwq == orm::wq::in)
            {
                wheresql.append(" IN (");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    wheresql.append(std::string_view(field2));
                }
                wheresql.append(") ");
                return *mod;
            }
            else if (opwq == orm::wq::like)
            {
                wheresql.append(" like '%");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    wheresql.append(std::string_view(field2));
                }
                wheresql.append("%' ");
                return *mod;
            }

            switch (opwq)
            {
            case orm::wq::bt:
                wheresql.append(" > ");
                break;
            case orm::wq::be:
                wheresql.append(" >= ");
                break;
            case orm::wq::eq:
                wheresql.append(" = ");
                break;
            case orm::wq::lt:
                wheresql.append(" < ");
                break;
            case orm::wq::le:
                wheresql.append(" <= ");
                break;
            default:
                wheresql.append(" = ");
                break;
            }

            wheresql.append(to_sql_value(std::forward<T2>(field2)));
            wheresql.append(" ");
            return *mod;
        }

        template <typename T2>
        M_MODEL &whereOr(std::string_view field1, orm::wq opwq, T2 &&field2)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            wheresql.append(field1);

            if (opwq == orm::wq::in)
            {
                wheresql.append(" IN (");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    wheresql.append(std::string_view(field2));
                }
                wheresql.append(") ");
                return *mod;
            }
            else if (opwq == orm::wq::like)
            {
                wheresql.append(" like '%");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    wheresql.append(std::string_view(field2));
                }
                wheresql.append("%' ");
                return *mod;
            }

            switch (opwq)
            {
            case orm::wq::bt:
                wheresql.append(" > ");
                break;
            case orm::wq::be:
                wheresql.append(" >= ");
                break;
            case orm::wq::eq:
                wheresql.append(" = ");
                break;
            case orm::wq::lt:
                wheresql.append(" < ");
                break;
            case orm::wq::le:
                wheresql.append(" <= ");
                break;
            default:
                wheresql.append(" = ");
                break;
            }

            wheresql.append(to_sql_value(std::forward<T2>(field2)));
            wheresql.append(" ");
            return *mod;
        }

        template <typename T2>
        M_MODEL &where(world_info::cols field1, orm::wq opwq, T2 &&field2)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            switch (field1)
            {
            
			case world_info::cols::id:
				wheresql.append("id");
				break;
			case world_info::cols::randomnumber:
				wheresql.append("randomnumber");
				break;
            default:
                return *mod;
                break;
            }

            if (opwq == orm::wq::in)
            {
                wheresql.append(" IN (");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    wheresql.append(std::string_view(field2));
                }
                wheresql.append(") ");
                return *mod;
            }
            else if (opwq == orm::wq::like)
            {
                wheresql.append(" like '%");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    wheresql.append(std::string_view(field2));
                }
                wheresql.append("%' ");
                return *mod;
            }

            switch (opwq)
            {
            case orm::wq::bt:
                wheresql.append(" > ");
                break;
            case orm::wq::be:
                wheresql.append(" >= ");
                break;
            case orm::wq::eq:
                wheresql.append(" = ");
                break;
            case orm::wq::lt:
                wheresql.append(" < ");
                break;
            case orm::wq::le:
                wheresql.append(" <= ");
                break;
            default:
                wheresql.append(" = ");
                break;
            }

            wheresql.append(to_sql_value(std::forward<T2>(field2)));
            wheresql.append(" ");
            return *mod;
        }

        template <typename T2>
        M_MODEL &whereOr(world_info::cols field1, orm::wq opwq, T2 &&field2)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" OR ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" OR ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }

            switch (field1)
            {
            
			case world_info::cols::id:
				wheresql.append("id");
				break;
			case world_info::cols::randomnumber:
				wheresql.append("randomnumber");
				break;
            default:
                return *mod;
                break;
            }

            if (opwq == orm::wq::in)
            {
                wheresql.append(" IN (");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    wheresql.append(std::string_view(field2));
                }
                wheresql.append(") ");
                return *mod;
            }
            else if (opwq == orm::wq::like)
            {
                wheresql.append(" like '%");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    wheresql.append(std::string_view(field2));
                }
                wheresql.append("%' ");
                return *mod;
            }

            switch (opwq)
            {
            case orm::wq::bt:
                wheresql.append(" > ");
                break;
            case orm::wq::be:
                wheresql.append(" >= ");
                break;
            case orm::wq::eq:
                wheresql.append(" = ");
                break;
            case orm::wq::lt:
                wheresql.append(" < ");
                break;
            case orm::wq::le:
                wheresql.append(" <= ");
                break;
            default:
                wheresql.append(" = ");
                break;
            }

            wheresql.append(to_sql_value(std::forward<T2>(field2)));
            wheresql.append(" ");
            return *mod;
        }

        M_MODEL &order(std::string_view wq)
        {
            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            return *mod;
        }
        M_MODEL &asc(std::string_view wq)
        {

            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            ordersql.append(" ASC ");
            return *mod;
        }

        M_MODEL &desc(std::string_view wq)
        {

            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            ordersql.append(" DESC ");
            return *mod;
        }

        M_MODEL &having(std::string_view wq)
        {

            groupsql.append(" HAVING by ");
            groupsql.append(wq);
            return *mod;
        }

        M_MODEL &group(std::string_view wq)
        {

            groupsql.append(" GROUP BY ");
            groupsql.append(wq);
            return *mod;
        }

        M_MODEL &orsub()
        {

            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            else
            {
                wheresql.append(" OR (");
                iskuohao     = true;
                ishascontent = false;
            }
            return *mod;
        }
        M_MODEL &andsub()
        {

            if (iskuohao == true)
            {
                iskuohao = false;
                wheresql.append(" )");
                ishascontent = false;
            }
            else
            {
                wheresql.append(" AND (");
                iskuohao     = true;
                ishascontent = false;
            }

            return *mod;
        }

        M_MODEL &endsub()
        {
            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            return *mod;
        }

        M_MODEL &or_b()
        {

            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            else
            {
                wheresql.append(" OR (");
                iskuohao     = true;
                ishascontent = false;
            }
            return *mod;
        }
        M_MODEL &and_b()
        {

            if (iskuohao == true)
            {
                iskuohao = false;
                wheresql.append(" )");
                ishascontent = false;
            }
            else
            {
                wheresql.append(" AND (");
                iskuohao     = true;
                ishascontent = false;
            }

            return *mod;
        }

        M_MODEL &or_e()
        {
            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
            return *mod;
        }

        M_MODEL &and_e()
        {
            if (iskuohao == true)
            {
                iskuohao     = false;
                ishascontent = false;
                wheresql.append(" )");
            }
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
            limitsql.append(std::to_string(endnum));
            limitsql.append(" OFFSET ");
            limitsql.append(std::to_string(num));
            return *mod;
        }

        std::vector<std::map<std::string, std::string>> fetch_obj()
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return temprecord;
                }

                for (auto &row : rows)
                {
                    std::map<std::string, std::string> data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (fields[ij].name.size() > 0)
                        {
                            data_temp.insert({fields[ij].name, row.values[ij]});
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            data_temp.insert({fields[ij].org_name, row.values[ij]});
                        }
                    }
                    temprecord.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            return temprecord;
        }
        std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>
        fetch_row()
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                temprecord = temp_cache.get(sqlhashid);
                if (temprecord.size() > 0)
                {
                    iscache                                                 = false;
                    model_meta_cache<std::vector<std::string>> &table_cache = model_meta_cache<std::vector<std::string>>::getinstance();
                    table_fieldname                                         = table_cache.get(sqlhashid);

                    model_meta_cache<std::map<std::string, unsigned int>> &tablemap_cache =
                        model_meta_cache<std::map<std::string, unsigned int>>::getinstance();
                    table_fieldmap = tablemap_cache.get(sqlhashid);

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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }

                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    table_fieldmap.emplace(fields[ii].org_name, table_fieldname.size());
                    table_fieldname.push_back(fields[ii].org_name);
                }

                for (auto &row : rows)
                {
                    std::vector<std::string> temp_v_record;
                    for (unsigned int ij = 0; ij < row.values.size(); ij++)
                    {
                        temp_v_record.push_back(row.values[ij]);
                    }
                    temprecord.push_back(std::move(temp_v_record));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
            }

            return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }

        template <typename T, RecordLineCallback<T> Callback>
        unsigned int fetch_to(std::vector<T> &custom_record, Callback &&callback)
        {
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                parse_wheresql();
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                for (auto &row : rows)
                {
                    T data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            std::invoke(std::forward<Callback>(callback), data_temp, fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type, 1);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            std::invoke(std::forward<Callback>(callback), data_temp, fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type, 1);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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

        template <typename T, RecordLineCallback<T> Callback>
        asio::awaitable<unsigned int> async_fetch_to(std::vector<T> &custom_record, Callback &&callback)
        {
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                parse_wheresql();
                sqlstring.append(wheresql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                for (auto &row : rows)
                {
                    T data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            std::invoke(std::forward<Callback>(callback), data_temp, fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type, 1);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            std::invoke(std::forward<Callback>(callback), data_temp, fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type, 1);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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

        template <ResultHasSetVal T>
        unsigned int fetch_to(std::vector<T> &custom_record)
        {
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
            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                parse_wheresql();
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                for (auto &row : rows)
                {
                    T data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            data_temp.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            data_temp.set_val(fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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

        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_fetch_to(std::vector<T> &custom_record)
        {
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                parse_wheresql();
                sqlstring.append(wheresql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                for (auto &row : rows)
                {
                    T data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            data_temp.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            data_temp.set_val(fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                    }
                    custom_record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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

        unsigned int fetch()
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                std::vector<unsigned char> field_pos;
                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    field_pos.push_back(B_BASE::findcolpos(fields[ii].org_name));
                }

                for (auto &row : rows)
                {
                    world_info::meta data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), data_temp);
                    }
                    B_BASE::record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
                return 0;
            }

            return 0;
        }

        asio::awaitable<unsigned int> async_fetch()
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                std::vector<unsigned char> field_pos;
                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    field_pos.push_back(B_BASE::findcolpos(fields[ii].org_name));
                }

                for (auto &row : rows)
                {
                    world_info::meta data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), data_temp);
                    }
                    B_BASE::record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
                co_return 0;
            }

            co_return 0;
        }
        M_MODEL &fetch_append()
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                    return *mod;
                }
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return *mod;
                }

                std::vector<unsigned char> field_pos;
                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    field_pos.push_back(B_BASE::findcolpos(fields[ii].org_name));
                }

                for (auto &row : rows)
                {
                    world_info::meta data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), data_temp);
                    }
                    B_BASE::record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
                return *mod;
            }

            return *mod;
        }

        asio::awaitable<unsigned int> async_fetch_append()
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                std::vector<unsigned char> field_pos;
                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    field_pos.push_back(B_BASE::findcolpos(fields[ii].org_name));
                }

                for (auto &row : rows)
                {
                    world_info::meta data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), data_temp);
                    }
                    B_BASE::record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
                co_return 0;
            }

            co_return 0;
        }

        template <typename T, RecordLineCallback<T> Callback>
        unsigned int fetch_one_to(T &custom_record, Callback &&callback)
        {
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                parse_wheresql();
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                for (auto &row : rows)
                {
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            std::invoke(std::forward<Callback>(callback), custom_record, fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type, 1);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            std::invoke(std::forward<Callback>(callback), custom_record, fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type, 1);
                        }
                    }
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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

        template <typename T, RecordLineCallback<T> Callback>
        asio::awaitable<unsigned int> async_fetch_one_to(T &custom_record, Callback &&callback)
        {
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                parse_wheresql();
                sqlstring.append(wheresql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                for (auto &row : rows)
                {
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            std::invoke(std::forward<Callback>(callback), custom_record, fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type, 1);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            std::invoke(std::forward<Callback>(callback), custom_record, fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type, 1);
                        }
                    }
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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

        template <ResultHasSetVal T>
        unsigned int fetch_one_to(T &custom_struct)
        {
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                parse_wheresql();
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                for (auto &row : rows)
                {
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            custom_struct.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            custom_struct.set_val(fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                    }
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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

        template <ResultHasSetVal T>
        asio::awaitable<unsigned int> async_fetch_one_to(T &custom_struct)
        {
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                parse_wheresql();
                sqlstring.append(wheresql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                for (auto &row : rows)
                {
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            custom_struct.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            custom_struct.set_val(fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                    }
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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

        unsigned int fetch_one(bool isappend = false)
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                std::vector<unsigned char> field_pos;
                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    field_pos.push_back(B_BASE::findcolpos(fields[ii].org_name));
                }

                for (auto &row : rows)
                {
                    if (isappend)
                    {
                        world_info::meta data_temp;
                        for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                        {
                            if (row.is_null[ij])
                            {
                                continue;
                            }
                            assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), data_temp);
                        }
                        B_BASE::record.emplace_back(std::move(data_temp));
                        effect_num++;
                    }
                    else
                    {
                        for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                        {
                            if (row.is_null[ij])
                            {
                                continue;
                            }
                            assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), B_BASE::data);
                        }
                        effect_num++;
                    }
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
                return 0;
            }

            return 0;
        }

        asio::awaitable<unsigned int> async_fetch_one(bool isappend = false)
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                std::vector<unsigned char> field_pos;
                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    field_pos.push_back(B_BASE::findcolpos(fields[ii].org_name));
                }

                for (auto &row : rows)
                {
                    if (isappend)
                    {
                        world_info::meta data_temp;
                        for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                        {
                            if (row.is_null[ij])
                            {
                                continue;
                            }
                            assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), data_temp);
                        }
                        B_BASE::record.emplace_back(std::move(data_temp));
                        effect_num++;
                    }
                    else
                    {
                        for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                        {
                            if (row.is_null[ij])
                            {
                                continue;
                            }
                            assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), B_BASE::data);
                        }
                        effect_num++;
                    }
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
            model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
            temp_cache.remove_exptime();
        }
        void clear_cache()
        {
            model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
            temp_cache.clear();
        }
        bool remove_cache()
        {
            model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            return temp_cache.remove(sqlhashid);
        }
        bool remove_cache(std::size_t cache_key_name)
        {
            model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
            return temp_cache.remove(cache_key_name);
        }
        int check_cache(std::size_t cache_key_name)
        {
            model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
            return temp_cache.check(cache_key_name);
        }

        bool get_data_cache(std::size_t cache_key_name)
        {
            try
            {
                model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
                B_BASE::data                                        = temp_cache.get(cache_key_name);
                return true;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            B_BASE::data_reset();
            return false;
        }
        int update_cache(int exp_time = 0)
        {
            model_meta_cache<std::vector<world_info::meta>> &temp_cache = model_meta_cache<std::vector<world_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            return temp_cache.update(sqlhashid, exp_time);
        }
        int update_cache(std::size_t cache_key_name, int exp_time)
        {
            model_meta_cache<std::vector<world_info::meta>> &temp_cache = model_meta_cache<std::vector<world_info::meta>>::getinstance();
            return temp_cache.update(cache_key_name, exp_time);
        }
        bool save_cache(int exp_time = 0)
        {
            model_meta_cache<std::vector<world_info::meta>> &temp_cache = model_meta_cache<std::vector<world_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::record, exp_time);
            return true;
        }

        bool save_data_cache(int exp_time = 0)
        {
            model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::data, exp_time);
            return true;
        }

        bool save_data_cache(const std::string &cache_key_name, const world_info::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }

        bool save_cache(std::size_t cache_key_name, const std::vector<world_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<world_info::meta>> &temp_cache = model_meta_cache<std::vector<world_info::meta>>::getinstance();
            temp_cache.save(cache_key_name, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const std::vector<world_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<world_info::meta>> &temp_cache = model_meta_cache<std::vector<world_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_vector_cache(const std::string cache_key_name, const std::vector<world_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<world_info::meta>> &temp_cache = model_meta_cache<std::vector<world_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const world_info::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        const world_info::meta &get_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<world_info::meta> &temp_cache = model_meta_cache<world_info::meta>::getinstance();
                std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
                return temp_cache.get(sqlhashid);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            throw std::runtime_error("Not in cache");
        }

        const std::vector<world_info::meta> &get_vector_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<std::vector<world_info::meta>> &temp_cache = model_meta_cache<std::vector<world_info::meta>>::getinstance();
                std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
                return temp_cache.get(sqlhashid);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            throw std::runtime_error("Not in cache");
        }

        bool get_record_cache(std::size_t cache_key_name)
        {
            try
            {
                model_meta_cache<std::vector<world_info::meta>> &temp_cache = model_meta_cache<std::vector<world_info::meta>>::getinstance();
                B_BASE::record                                                   = temp_cache.get(cache_key_name);
                return true;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            B_BASE::record.clear();
            return false;
        }
        http::obj_val fetch_json()
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                for (auto &row : rows)
                {
                    http::obj_val json_temp_v;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            json_temp_v[fields[ij].name] = row.values[ij];
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            json_temp_v[fields[ij].org_name] = row.values[ij];
                        }
                    }
                    valuetemp.push(json_temp_v);
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
                }
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            return valuetemp;
        }

        asio::awaitable<http::obj_val> async_fetch_json()
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

            if (wheresql.empty())
            {
                sqlstring.append(" 1 ");
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return valuetemp;
                }

                for (auto &row : rows)
                {
                    http::obj_val json_temp_v;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            json_temp_v[fields[ij].name] = row.values[ij];
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            json_temp_v[fields[ij].org_name] = row.values[ij];
                        }
                    }
                    valuetemp.push(json_temp_v);
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
                }
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                std::vector<unsigned char> field_pos;
                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    field_pos.push_back(B_BASE::findcolpos(fields[ii].org_name));
                }

                for (auto &row : rows)
                {
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), B_BASE::data);
                    }
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
                return 0;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }

            return 0;
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
                //auto conn = co_await conn_obj->async_get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                std::vector<unsigned char> field_pos;
                for (unsigned int ii = 0; ii < fields.size(); ii++)
                {
                    field_pos.push_back(B_BASE::findcolpos(fields[ii].org_name));
                }

                for (auto &row : rows)
                {
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        assign_field_value(field_pos[ij], (unsigned char *)row.values[ij].data(), row.values[ij].size(), B_BASE::data);
                    }
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
                co_return 0;
            }

            co_return 0;
        }

        int update()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }
            sqlstring = B_BASE::make_update_sql("");
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
        int update(const std::string &fieldname)
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    return 0;
                }
            }

            sqlstring = B_BASE::make_update_sql(fieldname);
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }
                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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

        asio::awaitable<int> async_update(const std::string &fieldname)
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    co_return 0;
                }
            }

            sqlstring = B_BASE::make_update_sql(fieldname);
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(wheresql);
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
        asio::awaitable<int> async_update()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    co_return 0;
                }
            }

            sqlstring = B_BASE::make_update_sql("");
            sqlstring.append(" where ");
            if (wheresql.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(wheresql);
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
                //auto conn = conn_obj->get_pg_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
        int remove()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    return 0;
                }
            }

            sqlstring = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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

        asio::awaitable<unsigned int> async_remove()
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    co_return 0;
                }
            }

            sqlstring = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                co_return 0;
            }
            else
            {
                sqlstring.append(wheresql);
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
                //auto conn = conn_obj->get_pg_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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

        int soft_remove(const std::string &fieldsql)
        {
            effect_num = 0;
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
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
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
        int soft_remove()
        {
            effect_num = 0;
            if (wheresql.empty())
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
                    wheresql = tempsql.str();
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
            if (wheresql.empty())
            {
                return 0;
            }
            else
            {
                sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
        std::tuple<unsigned int, unsigned long long> insert(world_info::meta &insert_data)
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
                //auto conn = conn_obj->get_pg_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::string insertsql = sqlstring;
                insertsql.append(" RETURNING ");
                insertsql.append(B_BASE::getPKname());
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(insertsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(insertsql, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (err > 0)
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!rows.empty() && !rows.back().values.empty() && !rows.back().is_null[0])
                    {
                        insert_last_id = std::strtoll(rows.back().values[0].c_str(), nullptr, 10);
                    }
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
                }
                //return insert_last_id;
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(world_info::meta &insert_data)
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::string insertsql = sqlstring;
                insertsql.append(" RETURNING ");
                insertsql.append(B_BASE::getPKname());
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(insertsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(insertsql, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (err > 0)
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!rows.empty() && !rows.back().values.empty() && !rows.back().is_null[0])
                    {
                        insert_last_id = std::strtoll(rows.back().values[0].c_str(), nullptr, 10);
                    }
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
                }
                //co_return insert_last_id;
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            co_return std::make_tuple(0, 0);
        }

        std::tuple<unsigned int, unsigned long long> insert(std::vector<world_info::meta> &insert_data)
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
                //auto conn = conn_obj->get_pg_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::string insertsql = sqlstring;
                insertsql.append(" RETURNING ");
                insertsql.append(B_BASE::getPKname());
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(insertsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(insertsql, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (err > 0)
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!rows.empty() && !rows.back().values.empty() && !rows.back().is_null[0])
                    {
                        insert_last_id = std::strtoll(rows.back().values[0].c_str(), nullptr, 10);
                    }
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
                }
                //return insert_last_id;
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(std::vector<world_info::meta> &insert_data)
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::string insertsql = sqlstring;
                insertsql.append(" RETURNING ");
                insertsql.append(B_BASE::getPKname());
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(insertsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(insertsql, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (err > 0)
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!rows.empty() && !rows.back().values.empty() && !rows.back().is_null[0])
                    {
                        insert_last_id = std::strtoll(rows.back().values[0].c_str(), nullptr, 10);
                    }
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
                }
                //co_return insert_last_id;
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
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
                //auto conn = conn_obj->get_pg_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::string insertsql = sqlstring;
                insertsql.append(" RETURNING ");
                insertsql.append(B_BASE::getPKname());
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(insertsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(insertsql, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (err > 0)
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!rows.empty() && !rows.back().values.empty() && !rows.back().is_null[0])
                    {
                        insert_last_id = std::strtoll(rows.back().values[0].c_str(), nullptr, 10);
                    }
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
                }
                //return insert_last_id;
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::string insertsql = sqlstring;
                insertsql.append(" RETURNING ");
                insertsql.append(B_BASE::getPKname());
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(insertsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(insertsql, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (err > 0)
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!rows.empty() && !rows.back().values.empty() && !rows.back().is_null[0])
                    {
                        insert_last_id = std::strtoll(rows.back().values[0].c_str(), nullptr, 10);
                    }
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
                }
                //co_return insert_last_id;
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }

            co_return std::make_tuple(0, 0);
        }

        std::tuple<unsigned int, unsigned long long> save(bool isrealnew = false)
        {
            effect_num = 0;
            if (B_BASE::getPK() > 0 && isrealnew == false)
            {
                if (wheresql.empty())
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                sqlstring = B_BASE::make_update_sql("");
                sqlstring.append(" where ");
                if (wheresql.empty())
                {
                    return std::make_tuple(0, 0);
                }
                else
                {
                    sqlstring.append(wheresql);
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
                //auto conn = conn_obj->get_pg_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(sqlstring, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
                }
                //return effect_num;
                return std::make_tuple(effect_num, 0);
            }
            else
            {
                sqlstring = B_BASE::make_data_insert_sql();
                if (conn_empty())
                {
                    return std::make_tuple(0, 0);
                }
                //auto conn = conn_obj->get_pg_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::string insertsql = sqlstring;
                insertsql.append(" RETURNING ");
                insertsql.append(B_BASE::getPKname());
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(insertsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(insertsql, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if (err > 0)
                {
                    error_msg = edit_conn->error_msg;
                    iserror   = true;
                    edit_conn.reset();
                }
                else
                {
                    effect_num = affected;
                    if (!rows.empty() && !rows.back().values.empty() && !rows.back().is_null[0])
                    {
                        insert_last_id = std::strtoll(rows.back().values[0].c_str(), nullptr, 10);
                    }
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
                }
                //return insert_last_id;
                return std::make_tuple(effect_num, insert_last_id);
            }
            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_save(bool isrealnew = false)
        {
            effect_num = 0;
            if (B_BASE::getPK() > 0 && isrealnew == false)
            {
                if (wheresql.empty())
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                sqlstring = B_BASE::make_update_sql("");
                sqlstring.append(" where ");
                if (wheresql.empty())
                {
                    co_return std::make_tuple(0, 0);
                }
                else
                {
                    sqlstring.append(wheresql);
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
                        if (!edit_conn)
                        {
                            edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                        }
                    }
                    else
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }

                    if (edit_conn->isdebug)
                    {
                        edit_conn->begin_time();
                    }
                    std::vector<field_info_t> fields;
                    std::vector<pg_row_data_t> rows;
                    unsigned int affected = 0;
                    unsigned int err      = co_await edit_conn->async_execute_and_fetch(sqlstring, fields, rows, affected);
                    if (edit_conn->isdebug)
                    {
                        edit_conn->finish_time();
                        auto &conn_mar    = get_orm_connect_mar();
                        long long du_time = edit_conn->count_time();
                        conn_mar.push_log(sqlstring, std::to_string(du_time));
                    }
                    if (err > 0)
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
                            conn_obj->back_pg_edit_conn(std::move(edit_conn));
                        }
                    }
                    co_return std::make_tuple(effect_num, 0);
                    //co_return effect_num;
                }
                catch (const std::exception &e)
                {
                    error_msg = std::string(e.what());
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
                        if (!edit_conn)
                        {
                            edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                        }
                    }
                    else
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }

                    if (edit_conn->isdebug)
                    {
                        edit_conn->begin_time();
                    }
                    std::string insertsql = sqlstring;
                    insertsql.append(" RETURNING ");
                    insertsql.append(B_BASE::getPKname());
                    std::vector<field_info_t> fields;
                    std::vector<pg_row_data_t> rows;
                    unsigned int affected = 0;
                    unsigned int err      = co_await edit_conn->async_execute_and_fetch(insertsql, fields, rows, affected);
                    if (edit_conn->isdebug)
                    {
                        edit_conn->finish_time();
                        auto &conn_mar    = get_orm_connect_mar();
                        long long du_time = edit_conn->count_time();
                        conn_mar.push_log(insertsql, std::to_string(du_time));
                    }

                    long long insert_last_id = 0;
                    if (err > 0)
                    {
                        error_msg = edit_conn->error_msg;
                        iserror   = true;
                        edit_conn.reset();
                    }
                    else
                    {
                        effect_num = affected;
                        if (!rows.empty() && !rows.back().values.empty() && !rows.back().is_null[0])
                        {
                            insert_last_id = std::strtoll(rows.back().values[0].c_str(), nullptr, 10);
                        }
                        B_BASE::setPK(insert_last_id);
                        if (!islock_conn)
                        {
                            conn_obj->back_pg_edit_conn(std::move(edit_conn));
                        }
                    }
                    co_return std::make_tuple(effect_num, insert_last_id);
                    //co_return insert_last_id;
                }
                catch (const std::exception &e)
                {
                    error_msg = std::string(e.what());
                }

                co_return std::make_tuple(0, 0);
            }
            co_return std::make_tuple(0, 0);
        }

        unsigned int query(const std::string &rawsql)
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

            return 0;
        }

        asio::awaitable<unsigned int> async_query(const std::string &rawsql)
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

            co_return 0;
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
                //auto conn = conn_obj->get_pg_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = select_conn->execute_and_fetch(rawsql, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                for (auto &row : rows)
                {
                    T data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            data_temp.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            data_temp.set_val(fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                    }
                    result_record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
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
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_pg_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_pg_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await select_conn->async_execute_and_fetch(rawsql, fields, rows, affected);
                if (err > 0)
                {
                    iserror   = true;
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                for (auto &row : rows)
                {
                    T data_temp;
                    for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                    {
                        if (row.is_null[ij])
                        {
                            continue;
                        }
                        if (fields[ij].name.size() > 0)
                        {
                            //or alias name
                            data_temp.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                        else if (fields[ij].org_name.size() > 0)
                        {
                            data_temp.set_val(fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                        }
                    }
                    result_record.emplace_back(std::move(data_temp));
                    effect_num++;
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
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
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = edit_conn->execute_and_fetch(rawsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_pg_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::vector<field_info_t> fields;
                std::vector<pg_row_data_t> rows;
                unsigned int affected = 0;
                unsigned int err      = co_await edit_conn->async_execute_and_fetch(rawsql, fields, rows, affected);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }
                if (err > 0)
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
                        conn_obj->back_pg_edit_conn(std::move(edit_conn));
                    }
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

        unsigned int parse_value(unsigned int i)
        {
            i++;
            if (i >= wheresql.size())
            {
                return i;
            }
            if (wheresql[i] == '>')
            {
                i++;
            }

            if (i >= wheresql.size())
            {
                return i;
            }

            if (wheresql[i] == '=')
            {
                i++;
                if (i >= wheresql.size())
                {
                    return i;
                }

                if (wheresql[i] == '>')
                {
                    i++;
                }

                if (i >= wheresql.size())
                {
                    return i;
                }

                // spache
                if (wheresql[i] == ' ')
                {
                    i++;
                    for (; i < wheresql.size(); i++)
                    {
                        if (wheresql[i] == ' ')
                        {
                            continue;
                        }
                        break;
                    }
                }

                if (i >= wheresql.size())
                {
                    return i;
                }
            }
            else if (wheresql[i] == ' ')
            {
                i++;
                for (; i < wheresql.size(); i++)
                {
                    if (wheresql[i] == ' ')
                    {
                        continue;
                    }
                    break;
                }
            }

            //begin value
            if (wheresql[i] == '\'')
            {
                i++;
                for (; i < wheresql.size(); i++)
                {
                    if (wheresql[i] == '\'')
                    {
                        if (wheresql[i - 1] == '\\')
                        {
                            continue;
                        }
                        i++;
                        break;
                    }
                }
            }
            else
            {
                for (; i < wheresql.size(); i++)
                {
                    if (wheresql[i] == ' ')
                    {
                        break;
                    }
                }
            }
            //end value
            return i;
        }

        unsigned int parse_between(unsigned int i)
        {
            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            //1 value
            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            //and
            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }
            return i;
        }
        unsigned int parse_like(unsigned int i)
        {
            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            i++;
            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == '\'')
                {
                    if (wheresql[i - 1] == '\\')
                    {
                        continue;
                    }

                    i++;
                    break;
                }
            }
            return i;
        }

        unsigned int parse_in(unsigned int i)
        {
            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            i++;
            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ')')
                {
                    i++;
                    break;
                }
            }
            return i;
        }

        unsigned int parse_exists(unsigned int i)
        {
            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            i++;
            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ')')
                {
                    i++;
                    break;
                }
            }
            return i;
        }

        unsigned int parse_notexists(unsigned int i)
        {
            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            for (; i < wheresql.size(); i++)
            {
                //find space
                if (wheresql[i] == ' ')
                {
                    i++;
                    break;
                }
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            i++;
            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ')')
                {
                    i++;
                    break;
                }
            }
            return i;
        }

        void parse_wheresql()
        {
            if (join_ptr == nullptr)
            {
                return;
            }
            bool ishastabname = false;
            unsigned int i    = 0;
            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == '.')
                {
                    ishastabname = true;
                    break;
                }
                else if (wheresql[i] == ' ' || wheresql[i] == '>' || wheresql[i] == '!' || wheresql[i] == '<' || wheresql[i] == '=' || wheresql[i] == '(')
                {
                    break;
                }
            }
            if (ishastabname)
            {
                return;
            }

            std::string newwheresql_;
            newwheresql_.append(B_BASE::tablename);
            newwheresql_.append(".");
            i = 0;
            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ')
                {
                    continue;
                }
                break;
            }

            for (; i < wheresql.size(); i++)
            {
                if (wheresql[i] == ' ' || wheresql[i] == '>' || wheresql[i] == '!' || wheresql[i] == '<' || wheresql[i] == '=' || wheresql[i] == '(')
                {
                    unsigned begin_offset = i;
                    if (wheresql[i] == ' ')
                    {
                        newwheresql_.push_back(' ');
                        for (; i < wheresql.size(); i++)
                        {
                            if (wheresql[i] != ' ')
                            {
                                break;
                            }
                        }
                    }
                    // > >= = < <= is null, in , not in , like , exists , not exists, between and
                    //value area

                    for (; i < wheresql.size(); i++)
                    {
                        if (wheresql[i] == '>')
                        {
                            i = parse_value(i);
                            break;
                        }
                        else if (wheresql[i] == '<')
                        {
                            i = parse_value(i);
                        }
                        else if (wheresql[i] == '=')
                        {
                            i = parse_value(i);
                        }
                        else if (wheresql[i] == '!')
                        {
                            i = parse_value(i);
                        }
                        else if (wheresql[i] == 'b' || wheresql[i] == 'B')
                        {
                            i = parse_between(i);
                        }
                        else if (wheresql[i] == 'l' || wheresql[i] == 'L')
                        {
                            i = parse_like(i);
                        }
                        else if (wheresql[i] == 'e' || wheresql[i] == 'E')
                        {
                            i = parse_exists(i);
                        }
                        else if (wheresql[i] == 'n' || wheresql[i] == 'N')
                        {
                            i = parse_notexists(i);
                        }
                        else if (wheresql[i] == 'i' || wheresql[i] == 'I')
                        {
                            if ((i + 1) < wheresql.size() && (wheresql[i] == 'n' || wheresql[i] == 'N'))
                            {
                                i = parse_in(i);
                            }
                            else
                            {
                                if ((i + 1) < wheresql.size() && (wheresql[i] == 's' || wheresql[i] == 'S'))
                                {
                                    i += 2;
                                    for (; i < wheresql.size(); i++)
                                    {
                                        if (wheresql[i] != ' ')
                                        {
                                            break;
                                        }
                                    }

                                    if ((i + 3) < wheresql.size() && (((wheresql[i] == 'N' || wheresql[i + 1] == 'O' || wheresql[i + 2] == 'T')) || (wheresql[i] == 'n' || wheresql[i + 1] == 'o' || wheresql[i + 2] == 't')))
                                    {
                                        i += 3;

                                        for (; i < wheresql.size(); i++)
                                        {
                                            if (wheresql[i] != ' ')
                                            {
                                                break;
                                            }
                                        }

                                        for (; i < wheresql.size(); i++)
                                        {
                                            if (wheresql[i] == ' ')
                                            {
                                                break;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        for (; i < wheresql.size(); i++)
                                        {
                                            if (wheresql[i] == ' ')
                                            {
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    for (; i < wheresql.size(); i++)
                    {
                        if (wheresql[i] != ' ')
                        {
                            break;
                        }
                    }
                    //in and or AND OR xor
                    for (; i < wheresql.size(); i++)
                    {
                        if (wheresql[i] == ' ')
                        {
                            break;
                        }
                    }
                    //pass
                    //and or AND OR xor
                    for (; i < wheresql.size(); i++)
                    {
                        if (wheresql[i] == ' ')
                        {
                            continue;
                        }
                        break;
                    }
                    newwheresql_.append(wheresql.substr(begin_offset, (i - begin_offset)));

                    if (i < wheresql.size())
                    {
                        newwheresql_.append(B_BASE::tablename);
                        newwheresql_.append(".");
                    }
                    else
                    {
                        break;
                    }
                }
                newwheresql_.push_back(wheresql[i]);
            }
            wheresql = newwheresql_;
        }
        void parse_leftjion()
        {
            if (join_ptr == nullptr)
            {
                return;
            }

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
        M_MODEL &leftJoin(std::string_view table1)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }
            join_ptr->join_table = table1;
            return *mod;
        }
        void get_join_table()
        {
            if (join_ptr == nullptr)
            {
                return;
            }

            sqlstring.append(" LEFT JOIN ");

            if (join_ptr->limitsql.empty())
            {
                sqlstring.append(join_ptr->join_table);
                sqlstring.append(" ON ");
                sqlstring.append(join_ptr->wheresql);
            }
            else
            {
                sqlstring.append(" ( SELECT ");
                if (join_ptr->selectsql.empty())
                {
                    sqlstring.append(" *,");
                }
                else
                {

                    sqlstring.append(trip_as_field(join_ptr->selectsql));

                    sqlstring.append(", ROW_NUMBER() OVER(PARTITION BY ");
                    sqlstring.append(join_ptr->parbysql);
                    sqlstring.append(" ");
                    sqlstring.append(join_ptr->ordersql);
                    sqlstring.append(") AS rn FROM ");
                    sqlstring.append(join_ptr->join_table);

                    if (join_ptr->subsql.size() > 0)
                    {
                        sqlstring.append(" WHERE ");
                        sqlstring.append(join_ptr->subsql);
                    }
                }
                sqlstring.append(" ) ");
                sqlstring.append(join_ptr->join_table);
                sqlstring.append(" ON ");
                sqlstring.append(join_ptr->wheresql);
                sqlstring.append(" AND ");
                sqlstring.append(join_ptr->join_table);
                sqlstring.append(".rn <= ");
                sqlstring.append(join_ptr->limitsql);
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
        M_MODEL &joinOn(std::string_view field1, std::string_view field2)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            if (join_ptr->wheresql.size() > 0)
            {
                join_ptr->wheresql.append(" AND ");
            }
            join_ptr->wheresql.append(join_ptr->join_table);
            join_ptr->wheresql.append(".");
            join_ptr->wheresql.append(field1);
            join_ptr->wheresql.append(" = ");
            join_ptr->wheresql.append(B_BASE::tablename);
            join_ptr->wheresql.append(".");
            join_ptr->wheresql.append(field2);

            if (join_ptr->parbysql.empty())
            {
                join_ptr->parbysql.append(field1);
            }
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
        M_MODEL &joinWhere(std::string_view field1, T2 &&field2)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            if (!join_ptr->subsql.empty())
            {
                join_ptr->subsql.append(" AND ");
            }

            join_ptr->subsql.append(field1);
            join_ptr->subsql.append(" = ");
            join_ptr->subsql.append(to_sql_value(std::forward<T2>(field2)));
            return *mod;
        }

        template <typename T2>
        M_MODEL &joinWhere(std::string_view field1, orm::wq opwq, T2 &&field2)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            if (!join_ptr->subsql.empty())
            {
                join_ptr->subsql.append(" AND ");
            }

            join_ptr->subsql.append(field1);

            if (opwq == orm::wq::in)
            {
                join_ptr->subsql.append(" IN (");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    join_ptr->subsql.append(std::string_view(field2));
                }
                join_ptr->subsql.append(") ");
                return *mod;
            }
            switch (opwq)
            {
            case orm::wq::bt:
                join_ptr->subsql.append(" > ");
                break;
            case orm::wq::be:
                join_ptr->subsql.append(" >= ");
                break;
            case orm::wq::eq:
                join_ptr->subsql.append(" = ");
                break;
            case orm::wq::lt:
                join_ptr->subsql.append(" < ");
                break;
            case orm::wq::le:
                join_ptr->subsql.append(" <= ");
                break;
            case orm::wq::like:
                join_ptr->subsql.append(" LIKE ");
                break;
            default:
                join_ptr->subsql.append(" = ");
                break;
            }

            join_ptr->subsql.append(to_sql_value(std::forward<T2>(field2)));
            wheresql.append(" ");
            return *mod;
        }

        template <typename T2>
        M_MODEL &joinWhereOr(std::string_view field1, orm::wq opwq, T2 &&field2)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            if (!join_ptr->subsql.empty())
            {
                join_ptr->subsql.append(" OR ");
            }

            join_ptr->subsql.append(field1);

            if (opwq == orm::wq::in)
            {
                join_ptr->subsql.append(" IN (");
                if constexpr (std::is_convertible_v<decltype(field2), std::string_view>)
                {
                    join_ptr->subsql.append(std::string_view(field2));
                }
                join_ptr->subsql.append(") ");
                return *mod;
            }
            switch (opwq)
            {
            case orm::wq::bt:
                join_ptr->subsql.append(" > ");
                break;
            case orm::wq::be:
                join_ptr->subsql.append(" >= ");
                break;
            case orm::wq::eq:
                join_ptr->subsql.append(" = ");
                break;
            case orm::wq::lt:
                join_ptr->subsql.append(" < ");
                break;
            case orm::wq::le:
                join_ptr->subsql.append(" <= ");
                break;
            case orm::wq::like:
                join_ptr->subsql.append(" LIKE ");
                break;
            default:
                join_ptr->subsql.append(" = ");
                break;
            }

            join_ptr->subsql.append(to_sql_value(std::forward<T2>(field2)));
            wheresql.append(" ");
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
        M_MODEL &joinParAppend(std::string_view field1)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            if (!join_ptr->parbysql.empty())
            {
                join_ptr->parbysql.append(",");
            }
            join_ptr->parbysql.append(field1);
            return *mod;
        }

        //分组
        M_MODEL &joinGroup(std::string_view field1)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            join_ptr->parbysql = field1;
            return *mod;
        }

        M_MODEL &joinDesc(std::string_view field1)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }
            if (join_ptr->ordersql.empty())
            {
                join_ptr->ordersql = " ORDER BY ";
                join_ptr->ordersql.append(field1);
                join_ptr->ordersql.append(" DESC ");
            }
            else
            {
                join_ptr->ordersql.append(" , ");
                join_ptr->ordersql.append(field1);
                join_ptr->ordersql.append(" DESC ");
            }
            return *mod;
        }

        M_MODEL &joinAsc(std::string_view field1)
        {
            if (join_ptr == nullptr)
            {
                join_ptr = std::make_unique<orm::orm_left_join_t>();
            }

            if (join_ptr->ordersql.empty())
            {
                join_ptr->ordersql = " ORDER BY ";
                join_ptr->ordersql.append(field1);
                join_ptr->ordersql.append(" ASC ");
            }
            else
            {
                join_ptr->ordersql.append(" , ");
                join_ptr->ordersql.append(field1);
                join_ptr->ordersql.append(" ASC ");
            }
            return *mod;
        }

        std::string commit_insert(world_info::meta &insert_data)
        {
            return B_BASE::make_data_insert_sql(insert_data);
        }

        std::string commit_insert()
        {
            return B_BASE::make_data_insert_sql();
        }
        std::string commit_update(const std::string &fieldname)
        {
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    error_msg = "warning empty where sql!";
                    return "";
                }
            }

            sqlstring = B_BASE::make_update_sql(fieldname);
            sqlstring.append(" WHERE ");
            if (wheresql.empty())
            {
                return "";
            }
            else
            {
                sqlstring.append(wheresql);
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
            if (wheresql.empty())
            {
                if (B_BASE::getPK() > 0)
                {
                    std::ostringstream tempsql;
                    tempsql << " ";
                    tempsql << B_BASE::getPKname();
                    tempsql << " = '";
                    tempsql << B_BASE::getPK();
                    tempsql << "' ";
                    wheresql = tempsql.str();
                }
                else
                {
                    return "";
                }
            }

            sqlstring = "DELETE FROM  ";
            sqlstring.append(B_BASE::tablename);
            sqlstring.append(" WHERE ");

            if (wheresql.empty())
            {
                return "";
            }
            else
            {
                sqlstring.append(wheresql);
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
            wheresql.clear();
            ordersql.clear();
            groupsql.clear();
            limitsql.clear();
            sqlstring.clear();
            error_msg.clear();

            join_ptr.reset();

            iskuohao     = false;
            ishascontent = false;
            iscache      = false;
            iserror      = false;
            effect_num   = 0;
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
            wheresql.clear();
            ordersql.clear();
            groupsql.clear();
            limitsql.clear();
            sqlstring.clear();
            error_msg.clear();

            iskuohao     = false;
            ishascontent = false;
            iscache      = false;
            iserror      = false;
            effect_num   = 0;
            join_ptr.reset();
            return *mod;
        }
        M_MODEL &set_data(world_info::meta indata)
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
                    conn_obj->back_pg_select_conn(std::move(select_conn));
                }
                if (edit_conn)
                {
                    conn_obj->back_pg_edit_conn(std::move(edit_conn));
                }
            }
        }

        DB_TYPE get_db_type()
        {
            return DB_TYPE::POSTGRESQL;
        }

      public:
        std::string selectsql;
        std::string wheresql;
        std::string ordersql;
        std::string groupsql;
        std::string limitsql;
        std::string sqlstring;
        std::string dbtag;
        std::string error_msg;

        // std::list<std::string> commit_sqllist;
        bool iskuohao           = false;
        bool ishascontent       = false;
        bool iscache            = false;
        bool iserror            = false;
        bool islock_conn        = false;
        int exptime             = 0;
        unsigned int effect_num = 0;

        M_MODEL *mod;

        std::unique_ptr<orm::orm_left_join_t> join_ptr = nullptr;

        std::shared_ptr<pg_conn_base> select_conn;
        std::shared_ptr<pg_conn_base> edit_conn;
        std::shared_ptr<orm_conn_pool> conn_obj;
        static constexpr DB_TYPE db_type = DB_TYPE::POSTGRESQL;
    };
} /*tagnamespace_replace*/
}// namespace orm
#endif