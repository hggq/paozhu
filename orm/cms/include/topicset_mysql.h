#ifndef _ORM_CMS_TOPICSET_OPERATE_H
#define _ORM_CMS_TOPICSET_OPERATE_H
/*
 *  @author 黄自权 huangziquan
 *  @date 2022-05-04
 *  @update 2025-03-12
 *  @update 2026-06-14 add xxx_fetch_to, leftjoin
 *  @dest ORM MySQL中间连接层
 *  本文件自动生成 This document is automatically generated.
 *  Creation time Mon, 13 Jul 2026 09:10:59 GMT
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

#include "mysql_conn.h"
#include "mysql_conn_pool.h"
#include "orm_cache.hpp"
#include "mysql_connect_mar.h"
#include "topicset_base.h"
/*baseincludefile*/
namespace orm
{
// mysql Operational SQL middleware
namespace cms
{ /*tagnamespace_replace*/
    template <typename M_MODEL, typename B_BASE>
    class topicset_mysql : public B_BASE
    {
      public:
        topicset_mysql(const std::string &tag) : dbtag(tag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                conn_obj = iter->second;
            }
            else
            {
                conn_obj  = nullptr;
                iserror   = true;
                error_msg = "conn_pool not found " + dbtag;
            }
        }
        topicset_mysql() : dbtag(B_BASE::_rmstag)
        {
            std::map<std::string, std::shared_ptr<orm_conn_pool>> &conn_pool_obj = get_orm_conn_pool_obj();
            auto iter                                                            = conn_pool_obj.find(dbtag);
            if (iter != conn_pool_obj.end())
            {
                conn_obj = iter->second;
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
                conn_obj = iter->second;
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::size_t n = select_conn->write_sql(countsql);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                //std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                unsigned int querysql_len = 0;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int tempnum = 0;

                                unsigned int name_length = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                querysql_len             = 0;
                                // if((tempnum + name_length) >= temp_pack_data.data.size())
                                // {
                                //     error_msg = "MySQL read pack error";
                                //     return 0;
                                // }
                                for (unsigned int ik = 0; ik < name_length; ik++)
                                {
                                    if (temp_pack_data.data[tempnum] >= '0' && temp_pack_data.data[tempnum] <= '9')
                                    {
                                        querysql_len = querysql_len * 10 + (temp_pack_data.data[tempnum] - '0');
                                    }
                                    tempnum++;
                                }
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }

                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }

                if (!islock_conn)
                {
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                return querysql_len;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::size_t n = co_await select_conn->async_write_sql(countsql);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }
                //std::size_t n = co_await asio::async_write(*conn->socket, asio::buffer(conn->send_data), asio::use_awaitable);

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                //std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                unsigned int querysql_len = 0;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                unsigned int tempnum = 0;

                                unsigned int name_length = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                // if((tempnum + name_length) >= temp_pack_data.data.size())
                                // {
                                //     error_msg = "MySQL read pack error";
                                //     co_return 0;
                                // }
                                querysql_len = 0;
                                for (unsigned int ik = 0; ik < name_length; ik++)
                                {
                                    if (temp_pack_data.data[tempnum] >= '0' && temp_pack_data.data[tempnum] <= '9')
                                    {
                                        querysql_len = querysql_len * 10 + (temp_pack_data.data[tempnum] - '0');
                                    }
                                    tempnum++;
                                }
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
                }

                if (select_conn->isdebug)
                {
                    select_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = select_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_select_conn(std::move(select_conn));
                }
                co_return querysql_len;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(countsql);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = co_await edit_conn->async_write_sql(countsql);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    edit_conn.reset();
                    co_return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::size_t n = edit_conn->write_sql(countsql);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::size_t n = co_await edit_conn->async_write_sql(countsql);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(countsql, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return 0;
        }

        void assign_field_value(unsigned char index_pos, unsigned char *result_temp_data, unsigned long long value_size, topicset_info::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0:
             {
               data_temp.topicsetid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.topicsetid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.topicsetid = 0;
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
               data_temp.topicid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.topicid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.topicid = 0;
                        }
            }
            break;
                case 3:
             {
               data_temp.linktopicid = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.linktopicid);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.linktopicid = 0;
                        }
            }
            break;
                case 4:
             {
               data_temp.linkrownum = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.linkrownum);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.linkrownum = 0;
                        }
            }
            break;
                case 5:
             {
               data_temp.sidetype = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.sidetype);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.sidetype = 0;
                        }
            }
            break;
                case 6:
            data_temp.sidename.clear();
            data_temp.sidename.resize(value_size);
            
            std::memcpy(data_temp.sidename.data(), result_temp_data, value_size);
            break;
                case 7:
            data_temp.txtcontent.clear();
            data_temp.txtcontent.resize(value_size);
            
            std::memcpy(data_temp.txtcontent.data(), result_temp_data, value_size);
            break;
                case 8:
             {
               data_temp.sort = 0;
            
                    auto result = std::from_chars(
                            reinterpret_cast<const char*>(result_temp_data),
                            reinterpret_cast<const char*>(result_temp_data) + value_size,
                            data_temp.sort);
                        if (result.ec == std::errc()) {

                        }
                        else{
                            data_temp.sort = 0;
                        }
            }
            break;
                
        }
    }
    

M_MODEL& eqTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid = ");

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
    

M_MODEL& nqTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid != ");

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
    

M_MODEL& inTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inTopicsetid(const std::vector<T>& val)
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
        wheresql.append(" topicsetid IN(");

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
    

M_MODEL& inTopicsetid(const std::vector<std::string>& val)
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
        wheresql.append(" topicsetid IN(");

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
    

M_MODEL& ninTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninTopicsetid(const std::vector<T>& val)
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
        wheresql.append(" topicsetid NOT IN(");

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
    

M_MODEL& ninTopicsetid(const std::vector<std::string>& val)
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
        wheresql.append(" topicsetid NOT IN(");

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
    

M_MODEL& btTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid > ");

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
    

M_MODEL& beTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid >= ");

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
    

M_MODEL& ltTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid < ");

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
    

M_MODEL& leTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid <= ");

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
    

M_MODEL& or_eqTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid = ");

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
    

M_MODEL& or_nqTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid != ");

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
    

M_MODEL& or_inTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inTopicsetid(const std::vector<T>& val)
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
        wheresql.append(" topicsetid IN(");

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
    

M_MODEL& or_inTopicsetid(const std::vector<std::string>& val)
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
        wheresql.append(" topicsetid IN(");

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
    

M_MODEL& or_ninTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninTopicsetid(const std::vector<T>& val)
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
        wheresql.append(" topicsetid NOT IN(");

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
    

M_MODEL& or_ninTopicsetid(const std::vector<std::string>& val)
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
        wheresql.append(" topicsetid NOT IN(");

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
    

M_MODEL& or_btTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid > ");

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
    

M_MODEL& or_beTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid >= ");

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
    

M_MODEL& or_ltTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid < ");

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
    

M_MODEL& or_leTopicsetid(const std::string &val)
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
        wheresql.append(" topicsetid <= ");

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
M_MODEL& eqTopicsetid(T val)
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
        wheresql.append(" topicsetid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqTopicsetid(T val)
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
        wheresql.append(" topicsetid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btTopicsetid(T val)
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
        wheresql.append(" topicsetid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beTopicsetid(T val)
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
        wheresql.append(" topicsetid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltTopicsetid(T val)
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
        wheresql.append(" topicsetid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leTopicsetid(T val)
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
        wheresql.append(" topicsetid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqTopicsetid(T val)
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
        wheresql.append(" topicsetid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqTopicsetid(T val)
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
        wheresql.append(" topicsetid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btTopicsetid(T val)
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
        wheresql.append(" topicsetid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beTopicsetid(T val)
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
        wheresql.append(" topicsetid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltTopicsetid(T val)
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
        wheresql.append(" topicsetid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leTopicsetid(T val)
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
        wheresql.append(" topicsetid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqUserid(const std::string &val)
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
        wheresql.append(" userid = ");

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
    

M_MODEL& nqUserid(const std::string &val)
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
        wheresql.append(" userid != ");

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
    

M_MODEL& inUserid(const std::string &val)
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
        wheresql.append(" userid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inUserid(const std::vector<T>& val)
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
        wheresql.append(" userid IN(");

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
    

M_MODEL& inUserid(const std::vector<std::string>& val)
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
        wheresql.append(" userid IN(");

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
    

M_MODEL& ninUserid(const std::string &val)
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
        wheresql.append(" userid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninUserid(const std::vector<T>& val)
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
        wheresql.append(" userid NOT IN(");

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
    

M_MODEL& ninUserid(const std::vector<std::string>& val)
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
        wheresql.append(" userid NOT IN(");

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
    

M_MODEL& btUserid(const std::string &val)
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
        wheresql.append(" userid > ");

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
    

M_MODEL& beUserid(const std::string &val)
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
        wheresql.append(" userid >= ");

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
    

M_MODEL& ltUserid(const std::string &val)
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
        wheresql.append(" userid < ");

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
    

M_MODEL& leUserid(const std::string &val)
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
        wheresql.append(" userid <= ");

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
    

M_MODEL& or_eqUserid(const std::string &val)
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
        wheresql.append(" userid = ");

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
    

M_MODEL& or_nqUserid(const std::string &val)
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
        wheresql.append(" userid != ");

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
    

M_MODEL& or_inUserid(const std::string &val)
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
        wheresql.append(" userid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inUserid(const std::vector<T>& val)
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
        wheresql.append(" userid IN(");

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
    

M_MODEL& or_inUserid(const std::vector<std::string>& val)
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
        wheresql.append(" userid IN(");

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
    

M_MODEL& or_ninUserid(const std::string &val)
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
        wheresql.append(" userid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninUserid(const std::vector<T>& val)
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
        wheresql.append(" userid NOT IN(");

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
    

M_MODEL& or_ninUserid(const std::vector<std::string>& val)
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
        wheresql.append(" userid NOT IN(");

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
    

M_MODEL& or_btUserid(const std::string &val)
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
        wheresql.append(" userid > ");

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
    

M_MODEL& or_beUserid(const std::string &val)
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
        wheresql.append(" userid >= ");

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
    

M_MODEL& or_ltUserid(const std::string &val)
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
        wheresql.append(" userid < ");

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
    

M_MODEL& or_leUserid(const std::string &val)
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
        wheresql.append(" userid <= ");

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
M_MODEL& eqUserid(T val)
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
        wheresql.append(" userid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqUserid(T val)
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
        wheresql.append(" userid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btUserid(T val)
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
        wheresql.append(" userid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beUserid(T val)
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
        wheresql.append(" userid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltUserid(T val)
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
        wheresql.append(" userid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leUserid(T val)
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
        wheresql.append(" userid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqUserid(T val)
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
        wheresql.append(" userid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqUserid(T val)
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
        wheresql.append(" userid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btUserid(T val)
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
        wheresql.append(" userid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beUserid(T val)
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
        wheresql.append(" userid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltUserid(T val)
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
        wheresql.append(" userid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leUserid(T val)
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
        wheresql.append(" userid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqTopicid(const std::string &val)
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
        wheresql.append(" topicid = ");

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
    

M_MODEL& nqTopicid(const std::string &val)
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
        wheresql.append(" topicid != ");

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
    

M_MODEL& inTopicid(const std::string &val)
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
        wheresql.append(" topicid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inTopicid(const std::vector<T>& val)
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
        wheresql.append(" topicid IN(");

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
    

M_MODEL& inTopicid(const std::vector<std::string>& val)
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
        wheresql.append(" topicid IN(");

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
    

M_MODEL& ninTopicid(const std::string &val)
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
        wheresql.append(" topicid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninTopicid(const std::vector<T>& val)
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
        wheresql.append(" topicid NOT IN(");

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
    

M_MODEL& ninTopicid(const std::vector<std::string>& val)
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
        wheresql.append(" topicid NOT IN(");

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
    

M_MODEL& btTopicid(const std::string &val)
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
        wheresql.append(" topicid > ");

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
    

M_MODEL& beTopicid(const std::string &val)
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
        wheresql.append(" topicid >= ");

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
    

M_MODEL& ltTopicid(const std::string &val)
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
        wheresql.append(" topicid < ");

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
    

M_MODEL& leTopicid(const std::string &val)
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
        wheresql.append(" topicid <= ");

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
    

M_MODEL& or_eqTopicid(const std::string &val)
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
        wheresql.append(" topicid = ");

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
    

M_MODEL& or_nqTopicid(const std::string &val)
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
        wheresql.append(" topicid != ");

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
    

M_MODEL& or_inTopicid(const std::string &val)
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
        wheresql.append(" topicid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inTopicid(const std::vector<T>& val)
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
        wheresql.append(" topicid IN(");

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
    

M_MODEL& or_inTopicid(const std::vector<std::string>& val)
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
        wheresql.append(" topicid IN(");

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
    

M_MODEL& or_ninTopicid(const std::string &val)
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
        wheresql.append(" topicid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninTopicid(const std::vector<T>& val)
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
        wheresql.append(" topicid NOT IN(");

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
    

M_MODEL& or_ninTopicid(const std::vector<std::string>& val)
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
        wheresql.append(" topicid NOT IN(");

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
    

M_MODEL& or_btTopicid(const std::string &val)
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
        wheresql.append(" topicid > ");

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
    

M_MODEL& or_beTopicid(const std::string &val)
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
        wheresql.append(" topicid >= ");

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
    

M_MODEL& or_ltTopicid(const std::string &val)
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
        wheresql.append(" topicid < ");

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
    

M_MODEL& or_leTopicid(const std::string &val)
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
        wheresql.append(" topicid <= ");

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
M_MODEL& eqTopicid(T val)
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
        wheresql.append(" topicid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqTopicid(T val)
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
        wheresql.append(" topicid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btTopicid(T val)
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
        wheresql.append(" topicid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beTopicid(T val)
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
        wheresql.append(" topicid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltTopicid(T val)
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
        wheresql.append(" topicid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leTopicid(T val)
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
        wheresql.append(" topicid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqTopicid(T val)
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
        wheresql.append(" topicid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqTopicid(T val)
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
        wheresql.append(" topicid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btTopicid(T val)
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
        wheresql.append(" topicid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beTopicid(T val)
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
        wheresql.append(" topicid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltTopicid(T val)
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
        wheresql.append(" topicid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leTopicid(T val)
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
        wheresql.append(" topicid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid = ");

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
    

M_MODEL& nqLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid != ");

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
    

M_MODEL& inLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inLinktopicid(const std::vector<T>& val)
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
        wheresql.append(" linktopicid IN(");

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
    

M_MODEL& inLinktopicid(const std::vector<std::string>& val)
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
        wheresql.append(" linktopicid IN(");

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
    

M_MODEL& ninLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninLinktopicid(const std::vector<T>& val)
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
        wheresql.append(" linktopicid NOT IN(");

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
    

M_MODEL& ninLinktopicid(const std::vector<std::string>& val)
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
        wheresql.append(" linktopicid NOT IN(");

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
    

M_MODEL& btLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid > ");

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
    

M_MODEL& beLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid >= ");

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
    

M_MODEL& ltLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid < ");

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
    

M_MODEL& leLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid <= ");

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
    

M_MODEL& or_eqLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid = ");

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
    

M_MODEL& or_nqLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid != ");

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
    

M_MODEL& or_inLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inLinktopicid(const std::vector<T>& val)
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
        wheresql.append(" linktopicid IN(");

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
    

M_MODEL& or_inLinktopicid(const std::vector<std::string>& val)
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
        wheresql.append(" linktopicid IN(");

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
    

M_MODEL& or_ninLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninLinktopicid(const std::vector<T>& val)
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
        wheresql.append(" linktopicid NOT IN(");

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
    

M_MODEL& or_ninLinktopicid(const std::vector<std::string>& val)
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
        wheresql.append(" linktopicid NOT IN(");

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
    

M_MODEL& or_btLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid > ");

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
    

M_MODEL& or_beLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid >= ");

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
    

M_MODEL& or_ltLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid < ");

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
    

M_MODEL& or_leLinktopicid(const std::string &val)
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
        wheresql.append(" linktopicid <= ");

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
M_MODEL& eqLinktopicid(T val)
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
        wheresql.append(" linktopicid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqLinktopicid(T val)
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
        wheresql.append(" linktopicid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btLinktopicid(T val)
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
        wheresql.append(" linktopicid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beLinktopicid(T val)
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
        wheresql.append(" linktopicid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltLinktopicid(T val)
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
        wheresql.append(" linktopicid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leLinktopicid(T val)
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
        wheresql.append(" linktopicid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqLinktopicid(T val)
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
        wheresql.append(" linktopicid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqLinktopicid(T val)
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
        wheresql.append(" linktopicid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btLinktopicid(T val)
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
        wheresql.append(" linktopicid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beLinktopicid(T val)
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
        wheresql.append(" linktopicid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltLinktopicid(T val)
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
        wheresql.append(" linktopicid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leLinktopicid(T val)
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
        wheresql.append(" linktopicid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum = ");

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
    

M_MODEL& nqLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum != ");

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
    

M_MODEL& inLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inLinkrownum(const std::vector<T>& val)
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
        wheresql.append(" linkrownum IN(");

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
    

M_MODEL& inLinkrownum(const std::vector<std::string>& val)
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
        wheresql.append(" linkrownum IN(");

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
    

M_MODEL& ninLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninLinkrownum(const std::vector<T>& val)
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
        wheresql.append(" linkrownum NOT IN(");

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
    

M_MODEL& ninLinkrownum(const std::vector<std::string>& val)
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
        wheresql.append(" linkrownum NOT IN(");

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
    

M_MODEL& btLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum > ");

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
    

M_MODEL& beLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum >= ");

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
    

M_MODEL& ltLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum < ");

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
    

M_MODEL& leLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum <= ");

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
    

M_MODEL& or_eqLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum = ");

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
    

M_MODEL& or_nqLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum != ");

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
    

M_MODEL& or_inLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inLinkrownum(const std::vector<T>& val)
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
        wheresql.append(" linkrownum IN(");

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
    

M_MODEL& or_inLinkrownum(const std::vector<std::string>& val)
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
        wheresql.append(" linkrownum IN(");

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
    

M_MODEL& or_ninLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninLinkrownum(const std::vector<T>& val)
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
        wheresql.append(" linkrownum NOT IN(");

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
    

M_MODEL& or_ninLinkrownum(const std::vector<std::string>& val)
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
        wheresql.append(" linkrownum NOT IN(");

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
    

M_MODEL& or_btLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum > ");

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
    

M_MODEL& or_beLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum >= ");

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
    

M_MODEL& or_ltLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum < ");

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
    

M_MODEL& or_leLinkrownum(const std::string &val)
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
        wheresql.append(" linkrownum <= ");

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
M_MODEL& eqLinkrownum(T val)
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
        wheresql.append(" linkrownum = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqLinkrownum(T val)
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
        wheresql.append(" linkrownum != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btLinkrownum(T val)
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
        wheresql.append(" linkrownum > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beLinkrownum(T val)
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
        wheresql.append(" linkrownum >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltLinkrownum(T val)
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
        wheresql.append(" linkrownum < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leLinkrownum(T val)
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
        wheresql.append(" linkrownum <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqLinkrownum(T val)
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
        wheresql.append(" linkrownum = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqLinkrownum(T val)
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
        wheresql.append(" linkrownum != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btLinkrownum(T val)
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
        wheresql.append(" linkrownum > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beLinkrownum(T val)
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
        wheresql.append(" linkrownum >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltLinkrownum(T val)
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
        wheresql.append(" linkrownum < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leLinkrownum(T val)
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
        wheresql.append(" linkrownum <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqSidetype(const std::string &val)
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
        wheresql.append(" sidetype = ");

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
    

M_MODEL& nqSidetype(const std::string &val)
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
        wheresql.append(" sidetype != ");

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
    

M_MODEL& inSidetype(const std::string &val)
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
        wheresql.append(" sidetype IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inSidetype(const std::vector<T>& val)
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
        wheresql.append(" sidetype IN(");

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
    

M_MODEL& inSidetype(const std::vector<std::string>& val)
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
        wheresql.append(" sidetype IN(");

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
    

M_MODEL& ninSidetype(const std::string &val)
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
        wheresql.append(" sidetype NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninSidetype(const std::vector<T>& val)
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
        wheresql.append(" sidetype NOT IN(");

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
    

M_MODEL& ninSidetype(const std::vector<std::string>& val)
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
        wheresql.append(" sidetype NOT IN(");

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
    

M_MODEL& btSidetype(const std::string &val)
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
        wheresql.append(" sidetype > ");

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
    

M_MODEL& beSidetype(const std::string &val)
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
        wheresql.append(" sidetype >= ");

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
    

M_MODEL& ltSidetype(const std::string &val)
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
        wheresql.append(" sidetype < ");

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
    

M_MODEL& leSidetype(const std::string &val)
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
        wheresql.append(" sidetype <= ");

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
    

M_MODEL& or_eqSidetype(const std::string &val)
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
        wheresql.append(" sidetype = ");

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
    

M_MODEL& or_nqSidetype(const std::string &val)
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
        wheresql.append(" sidetype != ");

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
    

M_MODEL& or_inSidetype(const std::string &val)
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
        wheresql.append(" sidetype IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inSidetype(const std::vector<T>& val)
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
        wheresql.append(" sidetype IN(");

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
    

M_MODEL& or_inSidetype(const std::vector<std::string>& val)
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
        wheresql.append(" sidetype IN(");

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
    

M_MODEL& or_ninSidetype(const std::string &val)
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
        wheresql.append(" sidetype NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninSidetype(const std::vector<T>& val)
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
        wheresql.append(" sidetype NOT IN(");

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
    

M_MODEL& or_ninSidetype(const std::vector<std::string>& val)
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
        wheresql.append(" sidetype NOT IN(");

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
    

M_MODEL& or_btSidetype(const std::string &val)
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
        wheresql.append(" sidetype > ");

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
    

M_MODEL& or_beSidetype(const std::string &val)
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
        wheresql.append(" sidetype >= ");

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
    

M_MODEL& or_ltSidetype(const std::string &val)
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
        wheresql.append(" sidetype < ");

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
    

M_MODEL& or_leSidetype(const std::string &val)
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
        wheresql.append(" sidetype <= ");

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
M_MODEL& eqSidetype(T val)
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
        wheresql.append(" sidetype = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqSidetype(T val)
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
        wheresql.append(" sidetype != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btSidetype(T val)
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
        wheresql.append(" sidetype > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beSidetype(T val)
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
        wheresql.append(" sidetype >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltSidetype(T val)
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
        wheresql.append(" sidetype < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leSidetype(T val)
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
        wheresql.append(" sidetype <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqSidetype(T val)
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
        wheresql.append(" sidetype = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqSidetype(T val)
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
        wheresql.append(" sidetype != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btSidetype(T val)
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
        wheresql.append(" sidetype > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beSidetype(T val)
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
        wheresql.append(" sidetype >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltSidetype(T val)
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
        wheresql.append(" sidetype < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leSidetype(T val)
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
        wheresql.append(" sidetype <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& nullSidename()
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
        wheresql.append(" sidename = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullSidename()
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
        wheresql.append(" sidename != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqSidename(const std::string &val)
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
        wheresql.append(" sidename = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqSidename(const std::string &val)
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
        wheresql.append(" sidename != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inSidename(const std::string &val)
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
        wheresql.append(" sidename IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inSidename(const std::vector<std::string> &val)
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
        wheresql.append(" sidename IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inSidename(const std::vector<T> &val)
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
        wheresql.append(" sidename IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninSidename(const std::string &val)
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
        wheresql.append(" sidename NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninSidename(const std::vector<std::string> &val)
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
        wheresql.append(" sidename NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninSidename(const std::vector<T> &val)
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
        wheresql.append(" sidename NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeSidename(const std::string &val)
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
        wheresql.append(" sidename LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeSidename(const std::string &val)
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
        wheresql.append(" sidename LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeSidename(const std::string &val)
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
        wheresql.append(" sidename LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btSidename(const std::string &val)
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
        wheresql.append(" sidename > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beSidename(const std::string &val)
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
        wheresql.append(" sidename >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltSidename(const std::string &val)
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
        wheresql.append(" sidename < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leSidename(const std::string &val)
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
        wheresql.append(" sidename <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullSidename()
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
        wheresql.append(" sidename = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullSidename()
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
        wheresql.append(" sidename != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqSidename(const std::string &val)
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
        wheresql.append(" sidename = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqSidename(const std::string &val)
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
        wheresql.append(" sidename != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inSidename(const std::string &val)
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
        wheresql.append(" sidename IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inSidename(const std::vector<std::string> &val)
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
        wheresql.append(" sidename IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inSidename(const std::vector<T> &val)
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
        wheresql.append(" sidename IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninSidename(const std::string &val)
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
        wheresql.append(" sidename NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninSidename(const std::vector<std::string> &val)
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
        wheresql.append(" sidename NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninSidename(const std::vector<T> &val)
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
        wheresql.append(" sidename NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeSidename(const std::string &val)
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
        wheresql.append(" sidename LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeSidename(const std::string &val)
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
        wheresql.append(" sidename LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeSidename(const std::string &val)
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
        wheresql.append(" sidename LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btSidename(const std::string &val)
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
        wheresql.append(" sidename > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beSidename(const std::string &val)
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
        wheresql.append(" sidename >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltSidename(const std::string &val)
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
        wheresql.append(" sidename < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leSidename(const std::string &val)
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
        wheresql.append(" sidename <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqSidename(T val)
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
        wheresql.append(" sidename = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqSidename(T val)
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
        wheresql.append(" sidename != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btSidename(T val)
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
        wheresql.append(" sidename > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beSidename(T val)
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
        wheresql.append(" sidename >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltSidename(T val)
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
        wheresql.append(" sidename < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leSidename(T val)
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
        wheresql.append(" sidename <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqSidename(T val)
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
        wheresql.append(" sidename = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqSidename(T val)
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
        wheresql.append(" sidename != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btSidename(T val)
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
        wheresql.append(" sidename > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beSidename(T val)
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
        wheresql.append(" sidename >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltSidename(T val)
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
        wheresql.append(" sidename < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leSidename(T val)
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
        wheresql.append(" sidename <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nullTxtcontent()
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
        wheresql.append(" txtcontent = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullTxtcontent()
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
        wheresql.append(" txtcontent != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inTxtcontent(const std::vector<std::string> &val)
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
        wheresql.append(" txtcontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inTxtcontent(const std::vector<T> &val)
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
        wheresql.append(" txtcontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& ninTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninTxtcontent(const std::vector<std::string> &val)
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
        wheresql.append(" txtcontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninTxtcontent(const std::vector<T> &val)
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
        wheresql.append(" txtcontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& likeTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullTxtcontent()
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
        wheresql.append(" txtcontent = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullTxtcontent()
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
        wheresql.append(" txtcontent != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inTxtcontent(const std::vector<std::string> &val)
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
        wheresql.append(" txtcontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inTxtcontent(const std::vector<T> &val)
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
        wheresql.append(" txtcontent IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_ninTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninTxtcontent(const std::vector<std::string> &val)
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
        wheresql.append(" txtcontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(B_BASE::stringaddslash(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninTxtcontent(const std::vector<T> &val)
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
        wheresql.append(" txtcontent NOT IN(");

        for(unsigned int i=0;i<val.size(); i++)
        {
            if(i>0)
            {
                wheresql.push_back(',');  
            }
            wheresql.push_back('\'');  
            wheresql.append(std::to_string(val[i]));
            wheresql.push_back('\'');    
        }
    

    
        return *mod;   
    }   
    

M_MODEL& or_likeTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leTxtcontent(const std::string &val)
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
        wheresql.append(" txtcontent <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqTxtcontent(T val)
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
        wheresql.append(" txtcontent = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqTxtcontent(T val)
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
        wheresql.append(" txtcontent != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btTxtcontent(T val)
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
        wheresql.append(" txtcontent > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beTxtcontent(T val)
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
        wheresql.append(" txtcontent >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltTxtcontent(T val)
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
        wheresql.append(" txtcontent < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leTxtcontent(T val)
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
        wheresql.append(" txtcontent <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqTxtcontent(T val)
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
        wheresql.append(" txtcontent = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqTxtcontent(T val)
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
        wheresql.append(" txtcontent != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btTxtcontent(T val)
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
        wheresql.append(" txtcontent > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beTxtcontent(T val)
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
        wheresql.append(" txtcontent >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltTxtcontent(T val)
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
        wheresql.append(" txtcontent < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leTxtcontent(T val)
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
        wheresql.append(" txtcontent <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& eqSort(const std::string &val)
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
        wheresql.append(" sort = ");

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
    

M_MODEL& nqSort(const std::string &val)
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
        wheresql.append(" sort != ");

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
    

M_MODEL& inSort(const std::string &val)
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
        wheresql.append(" sort IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inSort(const std::vector<T>& val)
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
        wheresql.append(" sort IN(");

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
    

M_MODEL& inSort(const std::vector<std::string>& val)
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
        wheresql.append(" sort IN(");

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
    

M_MODEL& ninSort(const std::string &val)
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
        wheresql.append(" sort NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninSort(const std::vector<T>& val)
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
        wheresql.append(" sort NOT IN(");

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
    

M_MODEL& ninSort(const std::vector<std::string>& val)
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
        wheresql.append(" sort NOT IN(");

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
    

M_MODEL& btSort(const std::string &val)
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
        wheresql.append(" sort > ");

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
    

M_MODEL& beSort(const std::string &val)
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
        wheresql.append(" sort >= ");

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
    

M_MODEL& ltSort(const std::string &val)
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
        wheresql.append(" sort < ");

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
    

M_MODEL& leSort(const std::string &val)
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
        wheresql.append(" sort <= ");

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
    

M_MODEL& or_eqSort(const std::string &val)
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
        wheresql.append(" sort = ");

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
    

M_MODEL& or_nqSort(const std::string &val)
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
        wheresql.append(" sort != ");

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
    

M_MODEL& or_inSort(const std::string &val)
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
        wheresql.append(" sort IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inSort(const std::vector<T>& val)
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
        wheresql.append(" sort IN(");

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
    

M_MODEL& or_inSort(const std::vector<std::string>& val)
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
        wheresql.append(" sort IN(");

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
    

M_MODEL& or_ninSort(const std::string &val)
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
        wheresql.append(" sort NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninSort(const std::vector<T>& val)
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
        wheresql.append(" sort NOT IN(");

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
    

M_MODEL& or_ninSort(const std::vector<std::string>& val)
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
        wheresql.append(" sort NOT IN(");

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
    

M_MODEL& or_btSort(const std::string &val)
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
        wheresql.append(" sort > ");

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
    

M_MODEL& or_beSort(const std::string &val)
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
        wheresql.append(" sort >= ");

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
    

M_MODEL& or_ltSort(const std::string &val)
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
        wheresql.append(" sort < ");

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
    

M_MODEL& or_leSort(const std::string &val)
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
        wheresql.append(" sort <= ");

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
M_MODEL& eqSort(T val)
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
        wheresql.append(" sort = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqSort(T val)
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
        wheresql.append(" sort != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btSort(T val)
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
        wheresql.append(" sort > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beSort(T val)
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
        wheresql.append(" sort >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltSort(T val)
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
        wheresql.append(" sort < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leSort(T val)
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
        wheresql.append(" sort <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqSort(T val)
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
        wheresql.append(" sort = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqSort(T val)
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
        wheresql.append(" sort != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btSort(T val)
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
        wheresql.append(" sort > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beSort(T val)
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
        wheresql.append(" sort >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltSort(T val)
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
        wheresql.append(" sort < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leSort(T val)
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
        wheresql.append(" sort <= ");

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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
                wheresql.append(val);
                wheresql.append("' ");
            }
            else
            {
                wheresql.push_back('%');
                wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
                wheresql.append(val);
                wheresql.append("' ");
            }
            else
            {
                wheresql.push_back('%');
                wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
            wheresql.append(val);
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
        M_MODEL &where(topicset_info::cols field1, orm::wq opwq, T2 &&field2)
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
            
			case topicset_info::cols::topicsetid:
				wheresql.append("topicsetid");
				break;
			case topicset_info::cols::userid:
				wheresql.append("userid");
				break;
			case topicset_info::cols::topicid:
				wheresql.append("topicid");
				break;
			case topicset_info::cols::linktopicid:
				wheresql.append("linktopicid");
				break;
			case topicset_info::cols::linkrownum:
				wheresql.append("linkrownum");
				break;
			case topicset_info::cols::sidetype:
				wheresql.append("sidetype");
				break;
			case topicset_info::cols::sidename:
				wheresql.append("sidename");
				break;
			case topicset_info::cols::txtcontent:
				wheresql.append("txtcontent");
				break;
			case topicset_info::cols::sort:
				wheresql.append("sort");
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
        M_MODEL &whereOr(topicset_info::cols field1, orm::wq opwq, T2 &&field2)
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
            
			case topicset_info::cols::topicsetid:
				wheresql.append("topicsetid");
				break;
			case topicset_info::cols::userid:
				wheresql.append("userid");
				break;
			case topicset_info::cols::topicid:
				wheresql.append("topicid");
				break;
			case topicset_info::cols::linktopicid:
				wheresql.append("linktopicid");
				break;
			case topicset_info::cols::linkrownum:
				wheresql.append("linkrownum");
				break;
			case topicset_info::cols::sidetype:
				wheresql.append("sidetype");
				break;
			case topicset_info::cols::sidename:
				wheresql.append("sidename");
				break;
			case topicset_info::cols::txtcontent:
				wheresql.append("txtcontent");
				break;
			case topicset_info::cols::sort:
				wheresql.append("sort");
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
            limitsql.append(std::to_string(num));
            limitsql.push_back(',');
            limitsql.append(std::to_string(endnum));
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return temprecord;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return temprecord;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return temprecord;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::map<std::string, std::string> data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return temprecord;
                                    // }
                                    std::string temp_str;
                                    temp_str.resize(name_length);
                                    std::memcpy(temp_str.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        data_temp.insert({field_array[ij].name, std::move(temp_str)});
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        data_temp.insert({field_array[ij].org_name, std::move(temp_str)});
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                temprecord.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                // std::vector<std::vector<std::string>> field_value;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                        table_fieldmap.emplace(field_array[ii].org_name, table_fieldname.size());
                                        table_fieldname.push_back(field_array[ii].org_name);
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::vector<std::string> temp_v_record;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                                    // }
                                    std::string tempstr;
                                    tempstr.resize(name_length);
                                    std::memcpy(tempstr.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    temp_v_record.push_back(std::move(tempstr));
                                    tempnum = tempnum + name_length;
                                }
                                temprecord.push_back(temp_v_record);
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        iserror   = true;
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                T data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        std::invoke(std::forward<Callback>(callback), data_temp, field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type, 1);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        std::invoke(std::forward<Callback>(callback), data_temp, field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type, 1);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                custom_record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = co_await select_conn->async_write_sql(sqlstring);

                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                T data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        std::invoke(std::forward<Callback>(callback), data_temp, field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type, 1);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        std::invoke(std::forward<Callback>(callback), data_temp, field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type, 1);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                custom_record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        iserror   = true;
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                T data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        data_temp.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        data_temp.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }

                                    tempnum = tempnum + name_length;
                                }
                                custom_record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = co_await select_conn->async_write_sql(sqlstring);

                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                T data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        data_temp.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        data_temp.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                custom_record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        iserror   = true;
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                topicset_info::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return 0;
                                    // }
                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                    tempnum = tempnum + name_length;
                                }
                                B_BASE::record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = co_await select_conn->async_write_sql(sqlstring);

                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                topicset_info::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                    tempnum = tempnum + name_length;
                                }
                                B_BASE::record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                    return 0;
                }
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                topicset_info::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return *mod;
                                    // }
                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                    tempnum = tempnum + name_length;
                                }
                                B_BASE::record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::size_t n = co_await select_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                // std::vector<std::vector<std::string>> field_value;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;
                // std::map<unsigned char, std::string> other_col;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                topicset_info::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                    tempnum = tempnum + name_length;
                                }
                                effect_num++;
                                B_BASE::record.emplace_back(std::move(data_temp));
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        std::invoke(std::forward<Callback>(callback), custom_record, field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type, 1);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        std::invoke(std::forward<Callback>(callback), custom_record, field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type, 1);
                                    }

                                    tempnum = tempnum + name_length;
                                }

                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = co_await select_conn->async_write_sql(sqlstring);

                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        std::invoke(std::forward<Callback>(callback), custom_record, field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type, 1);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        std::invoke(std::forward<Callback>(callback), custom_record, field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type, 1);
                                    }

                                    tempnum = tempnum + name_length;
                                }

                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        custom_struct.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        custom_struct.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }

                                    tempnum = tempnum + name_length;
                                }

                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = co_await select_conn->async_write_sql(sqlstring);

                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        custom_struct.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        custom_struct.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }

                                    tempnum = tempnum + name_length;
                                }

                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;
                unsigned char action_setup = 0;
                unsigned int column_num    = 0;
                unsigned int offset        = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                if (isappend)
                                {
                                    topicset_info::meta data_temp;
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                        // if((tempnum + name_length) >= temp_pack_data.data.size())
                                        // {
                                        //     error_msg = "MySQL read pack error";
                                        //     return 0;
                                        // }
                                        assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                        tempnum = tempnum + name_length;
                                    }
                                    B_BASE::record.emplace_back(std::move(data_temp));
                                    effect_num++;
                                }
                                else
                                {
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                        // if((tempnum + name_length) >= temp_pack_data.data.size())
                                        // {
                                        //     error_msg = "MySQL read pack error";
                                        //     return 0;
                                        // }
                                        assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, B_BASE::data);
                                        tempnum = tempnum + name_length;
                                    }
                                    effect_num++;
                                }
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }
                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = co_await select_conn->async_write_sql(sqlstring);

                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                if (isappend)
                                {
                                    topicset_info::meta data_temp;
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                        // if((tempnum + name_length) >= temp_pack_data.data.size())
                                        // {
                                        //     error_msg = "MySQL read pack error";
                                        //     co_return 0;
                                        // }
                                        assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, data_temp);
                                        tempnum = tempnum + name_length;
                                    }
                                    B_BASE::record.emplace_back(std::move(data_temp));
                                    effect_num++;
                                }
                                else
                                {
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                        // if((tempnum + name_length) >= temp_pack_data.data.size())
                                        // {
                                        //     error_msg = "MySQL read pack error";
                                        //     co_return 0;
                                        // }
                                        assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, B_BASE::data);
                                        tempnum = tempnum + name_length;
                                    }
                                    effect_num++;
                                }
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
            model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
            temp_cache.remove_exptime();
        }
        void clear_cache()
        {
            model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
            temp_cache.clear();
        }
        bool remove_cache()
        {
            model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            return temp_cache.remove(sqlhashid);
        }
        bool remove_cache(std::size_t cache_key_name)
        {
            model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
            return temp_cache.remove(cache_key_name);
        }
        int check_cache(std::size_t cache_key_name)
        {
            model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
            return temp_cache.check(cache_key_name);
        }

        bool get_data_cache(std::size_t cache_key_name)
        {
            try
            {
                model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
                B_BASE::data                                        = temp_cache.get(cache_key_name);
                return true;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (const char *e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            B_BASE::data_reset();
            return false;
        }
        int update_cache(int exp_time = 0)
        {
            model_meta_cache<std::vector<topicset_info::meta>> &temp_cache = model_meta_cache<std::vector<topicset_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            return temp_cache.update(sqlhashid, exp_time);
        }
        int update_cache(std::size_t cache_key_name, int exp_time)
        {
            model_meta_cache<std::vector<topicset_info::meta>> &temp_cache = model_meta_cache<std::vector<topicset_info::meta>>::getinstance();
            return temp_cache.update(cache_key_name, exp_time);
        }
        bool save_cache(int exp_time = 0)
        {
            model_meta_cache<std::vector<topicset_info::meta>> &temp_cache = model_meta_cache<std::vector<topicset_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::record, exp_time);
            return true;
        }

        bool save_data_cache(int exp_time = 0)
        {
            model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::data, exp_time);
            return true;
        }

        bool save_data_cache(const std::string &cache_key_name, const topicset_info::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }

        bool save_cache(std::size_t cache_key_name, const std::vector<topicset_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<topicset_info::meta>> &temp_cache = model_meta_cache<std::vector<topicset_info::meta>>::getinstance();
            temp_cache.save(cache_key_name, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const std::vector<topicset_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<topicset_info::meta>> &temp_cache = model_meta_cache<std::vector<topicset_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_vector_cache(const std::string cache_key_name, const std::vector<topicset_info::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<topicset_info::meta>> &temp_cache = model_meta_cache<std::vector<topicset_info::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const topicset_info::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        const topicset_info::meta &get_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<topicset_info::meta> &temp_cache = model_meta_cache<topicset_info::meta>::getinstance();
                std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
                return temp_cache.get(sqlhashid);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (const char *e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            throw "Not in cache";
        }

        const std::vector<topicset_info::meta> &get_vector_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<std::vector<topicset_info::meta>> &temp_cache = model_meta_cache<std::vector<topicset_info::meta>>::getinstance();
                std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
                return temp_cache.get(sqlhashid);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (const char *e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            throw "Not in cache";
        }

        bool get_record_cache(std::size_t cache_key_name)
        {
            try
            {
                model_meta_cache<std::vector<topicset_info::meta>> &temp_cache = model_meta_cache<std::vector<topicset_info::meta>>::getinstance();
                B_BASE::record                                                   = temp_cache.get(cache_key_name);
                return true;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (const char *e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                http::obj_val json_temp_v;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return 0;
                                    // }
                                    std::string temp_str;
                                    temp_str.resize(name_length);
                                    std::memcpy(temp_str.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        json_temp_v[field_array[ij].name] = std::move(temp_str);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        json_temp_v[field_array[ij].org_name] = std::move(temp_str);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                valuetemp.push(json_temp_v);
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::size_t n = co_await select_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return valuetemp;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return valuetemp;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                http::obj_val json_temp_v;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    std::string temp_str;
                                    temp_str.resize(name_length);
                                    std::memcpy(temp_str.data(), (unsigned char *)&temp_pack_data.data[tempnum], name_length);
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        json_temp_v[field_array[ij].name] = std::move(temp_str);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        json_temp_v[field_array[ij].org_name] = std::move(temp_str);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                valuetemp.push(json_temp_v);
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }

                std::size_t n = select_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     return 0;
                                    // }
                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, B_BASE::data);
                                    tempnum = tempnum + name_length;
                                }

                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = co_await conn_obj->async_get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::size_t n = co_await select_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    {
                                        field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    assign_field_value(field_pos[ij], (unsigned char *)&temp_pack_data.data[tempnum], name_length, B_BASE::data);
                                    tempnum = tempnum + name_length;
                                }

                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
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
                    conn_obj->back_select_conn(std::move(select_conn));
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
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);

                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }
                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);

                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    edit_conn.reset();
                    co_return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror   = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    edit_conn.reset();
                    co_return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror   = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    edit_conn.reset();
                    co_return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    edit_conn.reset();
                    co_return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return 0;
        }
        std::tuple<unsigned int, unsigned long long> insert(topicset_info::meta &insert_data)
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
                //auto conn = conn_obj->get_edit_conn();

                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                //return insert_last_id;
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(topicset_info::meta &insert_data)
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                //co_return insert_last_id;
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            co_return std::make_tuple(0, 0);
        }

        std::tuple<unsigned int, unsigned long long> insert(std::vector<topicset_info::meta> &insert_data)
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
                //auto conn = conn_obj->get_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                //return insert_last_id;
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
            }
            return std::make_tuple(0, 0);
        }

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(std::vector<topicset_info::meta> &insert_data)
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                //co_return insert_last_id;
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                //return insert_last_id;
                return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    edit_conn.reset();
                    co_return std::make_tuple(0, 0);
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                //co_return insert_last_id;
                co_return std::make_tuple(effect_num, insert_last_id);
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                //auto conn = conn_obj->get_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
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
                //auto conn = conn_obj->get_edit_conn();
                if (islock_conn)
                {
                    if (!edit_conn)
                    {
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }
                std::size_t n = edit_conn->write_sql(sqlstring);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return std::make_tuple(0, 0);
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(sqlstring, std::to_string(du_time));
                }

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
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
                            edit_conn = co_await conn_obj->async_get_edit_conn();
                        }
                    }
                    else
                    {
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }

                    if (edit_conn->isdebug)
                    {
                        edit_conn->begin_time();
                    }
                    std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                    if (n == 0)
                    {
                        error_msg = edit_conn->error_msg;
                        edit_conn.reset();
                        co_return std::make_tuple(0, 0);
                    }

                    unsigned int offset = 0;
                    n                   = co_await edit_conn->async_read_loop();
                    if (n == 0)
                    {
                        edit_conn.reset();
                        co_return std::make_tuple(0, 0);
                    }
                    pack_info_t temp_pack_data;
                    temp_pack_data.seq_id = 1;
                    edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                    if (edit_conn->isdebug)
                    {
                        edit_conn->finish_time();
                        auto &conn_mar    = get_orm_connect_mar();
                        long long du_time = edit_conn->count_time();
                        conn_mar.push_log(sqlstring, std::to_string(du_time));
                    }

                    if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                    {
                        error_msg = temp_pack_data.data.substr(3);
                        iserror   = true;
                        edit_conn.reset();
                    }
                    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                    {
                        unsigned int d_offset = 1;
                        effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                        if (!islock_conn)
                        {
                            conn_obj->back_edit_conn(std::move(edit_conn));
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
                catch (const std::string &e)
                {
                    error_msg = e;
                }
                catch (...)
                {
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
                            edit_conn = co_await conn_obj->async_get_edit_conn();
                        }
                    }
                    else
                    {
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }

                    if (edit_conn->isdebug)
                    {
                        edit_conn->begin_time();
                    }
                    std::size_t n = co_await edit_conn->async_write_sql(sqlstring);
                    if (n == 0)
                    {
                        error_msg = edit_conn->error_msg;
                        edit_conn.reset();
                        co_return std::make_tuple(0, 0);
                    }

                    unsigned int offset = 0;
                    n                   = co_await edit_conn->async_read_loop();
                    if (n == 0)
                    {
                        edit_conn.reset();
                        co_return std::make_tuple(0, 0);
                    }
                    pack_info_t temp_pack_data;
                    temp_pack_data.seq_id = 1;
                    edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                    if (edit_conn->isdebug)
                    {
                        edit_conn->finish_time();
                        auto &conn_mar    = get_orm_connect_mar();
                        long long du_time = edit_conn->count_time();
                        conn_mar.push_log(sqlstring, std::to_string(du_time));
                    }

                    long long insert_last_id = 0;
                    if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                    {
                        error_msg = temp_pack_data.data.substr(3);
                        iserror   = true;
                        edit_conn.reset();
                    }
                    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                    {

                        unsigned int d_offset = 1;
                        effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                        insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                        B_BASE::setPK(insert_last_id);
                        if (!islock_conn)
                        {
                            conn_obj->back_edit_conn(std::move(edit_conn));
                        }
                    }
                    co_return std::make_tuple(effect_num, insert_last_id);
                    //co_return insert_last_id;
                }
                catch (const std::exception &e)
                {
                    error_msg = std::string(e.what());
                }
                catch (const std::string &e)
                {
                    error_msg = e;
                }
                catch (...)
                {
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
                //auto conn = conn_obj->get_select_conn();
                if (islock_conn)
                {
                    if (!select_conn)
                    {
                        select_conn = conn_obj->get_select_conn();
                    }
                }
                else
                {
                    select_conn = conn_obj->get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::size_t n = select_conn->write_sql(rawsql);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = select_conn->read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        error_msg = select_conn->error_msg;
                        select_conn.reset();
                        return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    // for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    // {
                                    //     field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    //     table_fieldmap.emplace(field_array[ii].org_name, table_fieldname.size());
                                    //     table_fieldname.push_back(field_array[ii].org_name);
                                    // }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                T data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        data_temp.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        data_temp.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                result_record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        select_conn = co_await conn_obj->async_get_select_conn();
                    }
                }
                else
                {
                    select_conn = co_await conn_obj->async_get_select_conn();
                }

                if (select_conn->isdebug)
                {
                    select_conn->begin_time();
                }
                std::size_t n = co_await select_conn->async_write_sql(rawsql);
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
                    select_conn.reset();
                    co_return 0;
                }

                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                bool is_sql_item      = false;
                std::vector<field_info_t> field_array;

                unsigned char action_setup = 0;
                unsigned int column_num    = 0;

                unsigned int offset = 0;

                // std::vector<unsigned char> field_pos;

                for (; is_sql_item == false;)
                {
                    n      = co_await select_conn->async_read_loop();
                    offset = 0;
                    if (n == 0)
                    {
                        select_conn.reset();
                        co_return 0;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return 0;
                        }
                        if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                        {
                            if (select_conn->pack_eof_check(temp_pack_data))
                            {
                                is_sql_item = true;
                                break;
                            }

                            if (action_setup == 0)
                            {
                                if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                                {
                                    action_setup = 1;
                                    column_num   = (unsigned char)temp_pack_data.data[0];
                                }
                            }
                            else if (action_setup == 1)
                            {
                                field_info_t temp_filed_col;
                                select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                                field_array.emplace_back(std::move(temp_filed_col));
                                column_num--;
                                if (column_num == 0)
                                {
                                    action_setup = 2;
                                    // for (unsigned int ii = 0; ii < field_array.size(); ii++)
                                    // {
                                    //     field_pos.push_back(B_BASE::findcolpos(field_array[ii].org_name));
                                    //     table_fieldmap.emplace(field_array[ii].org_name, table_fieldname.size());
                                    //     table_fieldname.push_back(field_array[ii].org_name);
                                    // }
                                }
                            }
                            else if (action_setup == 2)
                            {
                                column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                T data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
                                    // if((tempnum + name_length) >= temp_pack_data.data.size())
                                    // {
                                    //     error_msg = "MySQL read pack error";
                                    //     co_return 0;
                                    // }
                                    if (field_array[ij].name.size() > 0)
                                    {
                                        //or alias name
                                        data_temp.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    else if (field_array[ij].org_name.size() > 0)
                                    {
                                        data_temp.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                    }
                                    tempnum = tempnum + name_length;
                                }
                                result_record.emplace_back(std::move(data_temp));
                                effect_num++;
                            }
                        }
                        else
                        {
                            if (offset >= n)
                            {
                                break;
                            }
                            is_sql_item = true;
                            break;
                        }
                    }
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }
                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        edit_conn = conn_obj->get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = conn_obj->get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::size_t n = edit_conn->write_sql(rawsql);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }

                unsigned int offset = 0;
                n                   = edit_conn->read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }
                return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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
                        edit_conn = co_await conn_obj->async_get_edit_conn();
                    }
                }
                else
                {
                    edit_conn = co_await conn_obj->async_get_edit_conn();
                }

                if (edit_conn->isdebug)
                {
                    edit_conn->begin_time();
                }

                std::size_t n = co_await edit_conn->async_write_sql(rawsql);
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }

                unsigned int offset = 0;
                n                   = co_await edit_conn->async_read_loop();
                if (n == 0)
                {
                    error_msg = edit_conn->error_msg;
                    edit_conn.reset();
                    co_return 0;
                }
                pack_info_t temp_pack_data;
                temp_pack_data.seq_id = 1;
                edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);
                if (edit_conn->isdebug)
                {
                    edit_conn->finish_time();
                    auto &conn_mar    = get_orm_connect_mar();
                    long long du_time = edit_conn->count_time();
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }

                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                    iserror = true;
                    edit_conn.reset();
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
                    }
                }

                co_return effect_num;
            }
            catch (const std::exception &e)
            {
                error_msg = std::string(e.what());
                co_return 0;
            }
            catch (const std::string &e)
            {
                error_msg = e;
            }
            catch (...)
            {
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

        std::string commit_insert(topicset_info::meta &insert_data)
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
        M_MODEL &set_data(topicset_info::meta indata)
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
                    conn_obj->back_select_conn(std::move(select_conn));
                }
                if (edit_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
                }
            }
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

        std::shared_ptr<mysql_conn_base> select_conn;
        std::shared_ptr<mysql_conn_base> edit_conn;
        std::shared_ptr<orm_conn_pool> conn_obj;
    };
} /*tagnamespace_replace*/
}// namespace orm
#endif