#ifndef _ORM_CMS_TESTA_OPERATE_H
#define _ORM_CMS_TESTA_OPERATE_H
/*
 *  @author 黄自权 huangziquan
 *  @date 2022-05-04
 *  @update 2025-03-12
 *  @dest ORM MySQL中间连接层
 */
#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include <set>
#include <string_view>
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
/*baseincludefile*/
namespace orm
{
// mysql Operational SQL middleware
namespace cms
{ /*tagnamespace_replace*/
    template <typename M_MODEL, typename B_BASE>
    class testa_mysql : public B_BASE
    {
      public:
        testa_mysql(const std::string &tag) : dbtag(tag)
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
        testa_mysql() : dbtag(B_BASE::_rmstag)
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
            if (original_tablename.empty())
            {
                original_tablename = B_BASE::tablename;
            }
            if (table_name.size() > 0)
            {
                B_BASE::tablename = table_name;
            }
            return *mod;
        }
        M_MODEL &reset_table()
        {
            if (original_tablename.empty())
            {
                return *mod;
            }
            B_BASE::tablename = original_tablename;
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
            unsigned int total_page = async_count();
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
            if (iscommit)
            {
                iscommit = false;
                return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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
            if (iscommit)
            {
                iscommit = false;
                co_return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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
            if (iscommit)
            {
                iscommit = false;
                return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

        void assign_field_value(unsigned char index_pos, unsigned char *result_temp_data, unsigned int value_size, typename B_BASE::meta &data_temp)
    {
        switch(index_pos)
        {
            case 0:
            data_temp.id=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.id= data_temp.id * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 1:
            data_temp.parentid=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.parentid= data_temp.parentid * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 2:
            data_temp.value_id=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.value_id= data_temp.value_id * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 3:
            data_temp.content.clear();
            data_temp.content.resize(value_size);
            
            std::memcpy(data_temp.content.data(), result_temp_data, value_size);
            break;
                case 4:
            data_temp.deleted=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.deleted= data_temp.deleted * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
                }
            }
            break;
                case 5:
            data_temp.deletetime=0;
            
            for(unsigned int i=0; i< value_size; i++)
            {
                if(result_temp_data[i]>='0'&&result_temp_data[i]<='9')
                {

                data_temp.deletetime= data_temp.deletetime * 10 + (result_temp_data[i]-'0');
                }   
                if(i>32)
                {
                    break;
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
        wheresql.append(" id IN(");

        wheresql.append(val);
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
        wheresql.append(" id NOT IN(");

        wheresql.append(val);
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
        wheresql.append(" id IN(");

        wheresql.append(val);
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
        wheresql.append(" id NOT IN(");

        wheresql.append(val);
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
    

M_MODEL& eqParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid = ");

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
    

M_MODEL& nqParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid != ");

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
    

M_MODEL& inParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inParentid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

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
    

M_MODEL& inParentid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid IN(");

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
    

M_MODEL& ninParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninParentid(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

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
    

M_MODEL& ninParentid(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid NOT IN(");

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
    

M_MODEL& btParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid > ");

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
    

M_MODEL& beParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid >= ");

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
    

M_MODEL& ltParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid < ");

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
    

M_MODEL& leParentid(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid <= ");

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
    

M_MODEL& or_eqParentid(const std::string &val)
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
        wheresql.append(" parentid = ");

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
    

M_MODEL& or_nqParentid(const std::string &val)
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
        wheresql.append(" parentid != ");

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
    

M_MODEL& or_inParentid(const std::string &val)
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
        wheresql.append(" parentid IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inParentid(const std::vector<T>& val)
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
        wheresql.append(" parentid IN(");

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
    

M_MODEL& or_inParentid(const std::vector<std::string>& val)
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
        wheresql.append(" parentid IN(");

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
    

M_MODEL& or_ninParentid(const std::string &val)
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
        wheresql.append(" parentid NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninParentid(const std::vector<T>& val)
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
        wheresql.append(" parentid NOT IN(");

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
    

M_MODEL& or_ninParentid(const std::vector<std::string>& val)
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
        wheresql.append(" parentid NOT IN(");

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
    

M_MODEL& or_btParentid(const std::string &val)
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
        wheresql.append(" parentid > ");

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
    

M_MODEL& or_beParentid(const std::string &val)
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
        wheresql.append(" parentid >= ");

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
    

M_MODEL& or_ltParentid(const std::string &val)
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
        wheresql.append(" parentid < ");

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
    

M_MODEL& or_leParentid(const std::string &val)
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
        wheresql.append(" parentid <= ");

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
M_MODEL& eqParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leParentid(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" parentid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqParentid(T val)
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
        wheresql.append(" parentid = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqParentid(T val)
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
        wheresql.append(" parentid != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btParentid(T val)
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
        wheresql.append(" parentid > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beParentid(T val)
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
        wheresql.append(" parentid >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltParentid(T val)
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
        wheresql.append(" parentid < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leParentid(T val)
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
        wheresql.append(" parentid <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqValueId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id = ");

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
    

M_MODEL& nqValueId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id != ");

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
    

M_MODEL& inValueId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inValueId(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id IN(");

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
    

M_MODEL& inValueId(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id IN(");

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
    

M_MODEL& ninValueId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninValueId(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id NOT IN(");

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
    

M_MODEL& ninValueId(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id NOT IN(");

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
    

M_MODEL& btValueId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id > ");

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
    

M_MODEL& beValueId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id >= ");

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
    

M_MODEL& ltValueId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id < ");

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
    

M_MODEL& leValueId(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id <= ");

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
    

M_MODEL& or_eqValueId(const std::string &val)
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
        wheresql.append(" value_id = ");

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
    

M_MODEL& or_nqValueId(const std::string &val)
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
        wheresql.append(" value_id != ");

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
    

M_MODEL& or_inValueId(const std::string &val)
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
        wheresql.append(" value_id IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inValueId(const std::vector<T>& val)
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
        wheresql.append(" value_id IN(");

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
    

M_MODEL& or_inValueId(const std::vector<std::string>& val)
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
        wheresql.append(" value_id IN(");

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
    

M_MODEL& or_ninValueId(const std::string &val)
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
        wheresql.append(" value_id NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninValueId(const std::vector<T>& val)
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
        wheresql.append(" value_id NOT IN(");

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
    

M_MODEL& or_ninValueId(const std::vector<std::string>& val)
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
        wheresql.append(" value_id NOT IN(");

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
    

M_MODEL& or_btValueId(const std::string &val)
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
        wheresql.append(" value_id > ");

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
    

M_MODEL& or_beValueId(const std::string &val)
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
        wheresql.append(" value_id >= ");

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
    

M_MODEL& or_ltValueId(const std::string &val)
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
        wheresql.append(" value_id < ");

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
    

M_MODEL& or_leValueId(const std::string &val)
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
        wheresql.append(" value_id <= ");

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
M_MODEL& eqValueId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqValueId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btValueId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beValueId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltValueId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leValueId(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" value_id <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqValueId(T val)
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
        wheresql.append(" value_id = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqValueId(T val)
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
        wheresql.append(" value_id != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btValueId(T val)
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
        wheresql.append(" value_id > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beValueId(T val)
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
        wheresql.append(" value_id >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltValueId(T val)
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
        wheresql.append(" value_id < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leValueId(T val)
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
        wheresql.append(" value_id <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& nullContent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content = NULL ");

        return *mod;   
    }   
    

M_MODEL& nnullContent()
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content != NULL ");

        return *mod;   
    }   
    

M_MODEL& eqContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& nqContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& inContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& inContent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content IN(");

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
M_MODEL& inContent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content IN(");

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
    

M_MODEL& ninContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& ninContent(const std::vector<std::string> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content NOT IN(");

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
M_MODEL& ninContent(const std::vector<T> &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content NOT IN(");

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
    

M_MODEL& likeContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& l_likeContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& r_likeContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& btContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& beContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& ltContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& leContent(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nullContent()
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
        wheresql.append(" content = NULL ");

        return *mod;   
    }   
    

M_MODEL& or_nnullContent()
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
        wheresql.append(" content != NULL ");

        return *mod;   
    }   
    

M_MODEL& or_eqContent(const std::string &val)
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
        wheresql.append(" content = '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_nqContent(const std::string &val)
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
        wheresql.append(" content != '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_inContent(const std::string &val)
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
        wheresql.append(" content IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_inContent(const std::vector<std::string> &val)
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
        wheresql.append(" content IN(");

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
M_MODEL& or_inContent(const std::vector<T> &val)
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
        wheresql.append(" content IN(");

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
    

M_MODEL& or_ninContent(const std::string &val)
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
        wheresql.append(" content NOT IN(");
				wheresql.append(val);
				wheresql.push_back(')');

        return *mod;   
    }   
    

M_MODEL& or_ninContent(const std::vector<std::string> &val)
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
        wheresql.append(" content NOT IN(");

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
M_MODEL& or_ninContent(const std::vector<T> &val)
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
        wheresql.append(" content NOT IN(");

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
    

M_MODEL& or_likeContent(const std::string &val)
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
        wheresql.append(" content LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& orl_likeContent(const std::string &val)
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
        wheresql.append(" content LIKE '%");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("'");

        return *mod;   
    }   
    

M_MODEL& orr_likeContent(const std::string &val)
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
        wheresql.append(" content LIKE '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.append("%'");

        return *mod;   
    }   
    

M_MODEL& or_btContent(const std::string &val)
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
        wheresql.append(" content > '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_beContent(const std::string &val)
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
        wheresql.append(" content >= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_ltContent(const std::string &val)
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
        wheresql.append(" content < '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& or_leContent(const std::string &val)
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
        wheresql.append(" content <= '");
				wheresql.append(B_BASE::stringaddslash(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& eqContent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& nqContent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& btContent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& beContent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& ltContent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content < '");
		wheresql.append(std::to_string(val));
		wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& leContent(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" content <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_eqContent(T val)
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
        wheresql.append(" content = '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_nqContent(T val)
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
        wheresql.append(" content != '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_btContent(T val)
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
        wheresql.append(" content > '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_beContent(T val)
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
        wheresql.append(" content >= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_ltContent(T val)
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
        wheresql.append(" content < '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

template <typename T>
		requires std::is_floating_point_v<T>||std::is_integral_v<T>
M_MODEL& or_leContent(T val)
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
        wheresql.append(" content <= '");
				wheresql.append(std::to_string(val));
				wheresql.push_back('\'');

        return *mod;   
    }   
    

M_MODEL& eqDeleted(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted = ");

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
    

M_MODEL& nqDeleted(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted != ");

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
    

M_MODEL& inDeleted(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inDeleted(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted IN(");

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
    

M_MODEL& inDeleted(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted IN(");

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
    

M_MODEL& ninDeleted(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninDeleted(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted NOT IN(");

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
    

M_MODEL& ninDeleted(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted NOT IN(");

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
    

M_MODEL& btDeleted(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted > ");

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
    

M_MODEL& beDeleted(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted >= ");

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
    

M_MODEL& ltDeleted(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted < ");

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
    

M_MODEL& leDeleted(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted <= ");

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
    

M_MODEL& or_eqDeleted(const std::string &val)
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
        wheresql.append(" deleted = ");

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
    

M_MODEL& or_nqDeleted(const std::string &val)
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
        wheresql.append(" deleted != ");

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
    

M_MODEL& or_inDeleted(const std::string &val)
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
        wheresql.append(" deleted IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inDeleted(const std::vector<T>& val)
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
        wheresql.append(" deleted IN(");

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
    

M_MODEL& or_inDeleted(const std::vector<std::string>& val)
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
        wheresql.append(" deleted IN(");

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
    

M_MODEL& or_ninDeleted(const std::string &val)
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
        wheresql.append(" deleted NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninDeleted(const std::vector<T>& val)
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
        wheresql.append(" deleted NOT IN(");

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
    

M_MODEL& or_ninDeleted(const std::vector<std::string>& val)
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
        wheresql.append(" deleted NOT IN(");

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
    

M_MODEL& or_btDeleted(const std::string &val)
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
        wheresql.append(" deleted > ");

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
    

M_MODEL& or_beDeleted(const std::string &val)
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
        wheresql.append(" deleted >= ");

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
    

M_MODEL& or_ltDeleted(const std::string &val)
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
        wheresql.append(" deleted < ");

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
    

M_MODEL& or_leDeleted(const std::string &val)
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
        wheresql.append(" deleted <= ");

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
M_MODEL& eqDeleted(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqDeleted(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btDeleted(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beDeleted(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltDeleted(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leDeleted(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deleted <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqDeleted(T val)
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
        wheresql.append(" deleted = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqDeleted(T val)
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
        wheresql.append(" deleted != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btDeleted(T val)
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
        wheresql.append(" deleted > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beDeleted(T val)
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
        wheresql.append(" deleted >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltDeleted(T val)
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
        wheresql.append(" deleted < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leDeleted(T val)
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
        wheresql.append(" deleted <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

M_MODEL& eqDeletetime(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime = ");

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
    

M_MODEL& nqDeletetime(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime != ");

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
    

M_MODEL& inDeletetime(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& inDeletetime(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime IN(");

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
    

M_MODEL& inDeletetime(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime IN(");

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
    

M_MODEL& ninDeletetime(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ninDeletetime(const std::vector<T>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime NOT IN(");

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
    

M_MODEL& ninDeletetime(const std::vector<std::string>& val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime NOT IN(");

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
    

M_MODEL& btDeletetime(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime > ");

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
    

M_MODEL& beDeletetime(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime >= ");

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
    

M_MODEL& ltDeletetime(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime < ");

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
    

M_MODEL& leDeletetime(const std::string &val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime <= ");

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
    

M_MODEL& or_eqDeletetime(const std::string &val)
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
        wheresql.append(" deletetime = ");

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
    

M_MODEL& or_nqDeletetime(const std::string &val)
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
        wheresql.append(" deletetime != ");

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
    

M_MODEL& or_inDeletetime(const std::string &val)
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
        wheresql.append(" deletetime IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_inDeletetime(const std::vector<T>& val)
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
        wheresql.append(" deletetime IN(");

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
    

M_MODEL& or_inDeletetime(const std::vector<std::string>& val)
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
        wheresql.append(" deletetime IN(");

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
    

M_MODEL& or_ninDeletetime(const std::string &val)
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
        wheresql.append(" deletetime NOT IN(");

        wheresql.append(val);
        wheresql.push_back(')');
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ninDeletetime(const std::vector<T>& val)
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
        wheresql.append(" deletetime NOT IN(");

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
    

M_MODEL& or_ninDeletetime(const std::vector<std::string>& val)
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
        wheresql.append(" deletetime NOT IN(");

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
    

M_MODEL& or_btDeletetime(const std::string &val)
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
        wheresql.append(" deletetime > ");

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
    

M_MODEL& or_beDeletetime(const std::string &val)
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
        wheresql.append(" deletetime >= ");

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
    

M_MODEL& or_ltDeletetime(const std::string &val)
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
        wheresql.append(" deletetime < ");

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
    

M_MODEL& or_leDeletetime(const std::string &val)
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
        wheresql.append(" deletetime <= ");

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
M_MODEL& eqDeletetime(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& nqDeletetime(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& btDeletetime(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& beDeletetime(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& ltDeletetime(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& leDeletetime(T val)
	{
        if (wheresql.empty())
        {
        }
        else
        {
            if (ishascontent)
            {
                wheresql.append(" AND ");
            }
            else
            {
                if (!iskuohao)
                {
                    wheresql.append(" AND ");
                }
            }
        }
        if (iskuohao)
        {
            ishascontent = true;
        }
        wheresql.append(" deletetime <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_eqDeletetime(T val)
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
        wheresql.append(" deletetime = ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_nqDeletetime(T val)
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
        wheresql.append(" deletetime != ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_btDeletetime(T val)
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
        wheresql.append(" deletetime > ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_beDeletetime(T val)
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
        wheresql.append(" deletetime >= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_ltDeletetime(T val)
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
        wheresql.append(" deletetime < ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

template <typename T>
	requires std::is_integral_v<T>
M_MODEL& or_leDeletetime(T val)
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
        wheresql.append(" deletetime <= ");

        wheresql.append(std::to_string(val));
        return *mod;   
    }   
    

        M_MODEL &select(const std::string &fieldname)
        {
            if (selectsql.size() > 0)
            {
                selectsql.push_back(',');
            }
            selectsql.append(fieldname);
            return *mod;
        }

        M_MODEL &where(const std::string &wq)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &where(const std::string &wq, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            std::stringstream _stream;
            _stream << val;
            wheresql.append(_stream.str());
            return *mod;
        }

        M_MODEL &where(const std::string &wq, char bi, http::obj_val &obj)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            if (obj.is_string())
            {
                wheresql.push_back('\'');
                wheresql.append(obj.as_string());
                wheresql.push_back('\'');
            }
            else
            {

                wheresql.append(obj.to_string());
            }
            return *mod;
        }
        M_MODEL &where(const std::string &wq, http::obj_val &obj)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            if (obj.is_string())
            {
                wheresql.push_back('\'');
                wheresql.append(obj.as_string());
                wheresql.push_back('\'');
            }
            else
            {

                wheresql.append(obj.to_string());
            }
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &where(const std::string &wq, char bi, _SQL_Value val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            std::stringstream _stream;
            _stream << val;
            wheresql.append(_stream.str());
            return *mod;
        }

        M_MODEL &where(const std::string &wq, char bi, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            wheresql.push_back('\'');

            wheresql.append(val);
            wheresql.push_back('\'');
            return *mod;
        }
        M_MODEL &where(const std::string &wq, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            wheresql.push_back('\'');
            wheresql.append(val);
            wheresql.push_back('\'');
            return *mod;
        }

        M_MODEL &between(const std::string &wq, const std::string &a, const std::string &b)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(" (");
            wheresql.append(wq);
            wheresql.append(" BETWEEN '");
            std::stringstream _stream;
            _stream << a;
            _stream << "' AND '";
            _stream << b;
            _stream << "' ) ";
            wheresql.append(_stream.str());
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &between(const std::string &wq, _SQL_Value a, _SQL_Value b)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
                    {
                        wheresql.append(" AND ");
                    }
                }
            }
            if (iskuohao)
            {
                ishascontent = true;
            }
            wheresql.append(" (");
            wheresql.append(wq);
            wheresql.append(" BETWEEN ");
            std::stringstream _stream;
            _stream << a;
            _stream << " AND ";
            _stream << b;
            _stream << " ) ";
            wheresql.append(_stream.str());
            return *mod;
        }
        template <typename _SQL_Value>
            requires std::is_integral_v<_SQL_Value> || std::is_floating_point_v<_SQL_Value>
        M_MODEL &orBetween(const std::string &wq, _SQL_Value a, _SQL_Value b)
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
            wheresql.append(" (");
            wheresql.append(wq);
            wheresql.append(" BETWEEN ");
            std::stringstream _stream;
            _stream << a;
            _stream << " AND ";
            _stream << b;
            _stream << " ) ";
            wheresql.append(_stream.str());
            return *mod;
        }
        M_MODEL &whereLike(const std::string &wq, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereLikeLeft(const std::string &wq, const std::string &val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereLikeRight(const std::string &wq, const std::string &val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereOrLike(const std::string &wq, const std::string &val)
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
        M_MODEL &whereAnd(const std::string &wq)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereAnd(const std::string &wq, _SQL_Value val)
        {

            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }
            std::stringstream _stream;
            _stream << val;
            wheresql.append(_stream.str());
            return *mod;
        }
        M_MODEL &whereAnd(const std::string &wq, const std::string &val)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            wheresql.push_back('\'');
            wheresql.append(val);
            wheresql.push_back('\'');

            return *mod;
        }
        M_MODEL &whereOr(const std::string &wq)
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
        M_MODEL &whereOr(const std::string &wq, _SQL_Value val)
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
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }
            std::stringstream _stream;
            _stream << val;
            wheresql.append(_stream.str());
            return *mod;
        }
        M_MODEL &whereOr(const std::string &wq, const std::string &val)
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
            char bi = wq.back();
            if (bi == '=' || bi == '>' || bi == '<')
            {
            }
            else
            {
                wheresql.push_back('=');
            }

            wheresql.push_back('\'');
            wheresql.append(val);
            wheresql.push_back('\'');
            return *mod;
        }
        M_MODEL &whereIn(const std::string &k)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereIn(const std::string &k, const std::string &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
            wheresql.append(a);
            wheresql.append(") ");
            return *mod;
        }

        M_MODEL &whereIn(const std::string &k, const std::vector<std::string> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereNotIn(const std::string &k, const std::vector<std::string> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereIn(const std::string &k, const std::list<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereIn(const std::string &k, const std::vector<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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
        M_MODEL &whereNotIn(const std::string &k, const std::vector<_SQL_Value> &a)
        {
            if (wheresql.empty())
            {
            }
            else
            {
                if (ishascontent)
                {
                    wheresql.append(" AND ");
                }
                else
                {
                    if (!iskuohao)
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

        M_MODEL &order(const std::string &wq)
        {
            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            return *mod;
        }
        M_MODEL &asc(const std::string &wq)
        {

            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            ordersql.append(" ASC ");
            return *mod;
        }

        M_MODEL &desc(const std::string &wq)
        {

            ordersql.append(" ORDER by ");
            ordersql.append(wq);
            ordersql.append(" DESC ");
            return *mod;
        }

        M_MODEL &having(const std::string &wq)
        {

            groupsql.append(" HAVING by ");
            groupsql.append(wq);
            return *mod;
        }

        M_MODEL &group(const std::string &wq)
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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::map<std::string, std::string> data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::vector<std::string> temp_v_record;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
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
        M_MODEL &fetch()
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

            B_BASE::record_reset();
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
                    return *mod;
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
                        return *mod;
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            return *mod;
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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                typename B_BASE::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                typename B_BASE::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                typename B_BASE::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                typename B_BASE::meta data_temp;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                if (isappend)
                                {
                                    typename B_BASE::meta data_temp;
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                if (isappend)
                                {
                                    typename B_BASE::meta data_temp;
                                    for (unsigned int ij = 0; ij < column_num; ij++)
                                    {
                                        unsigned long long name_length = 0;
                                        name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            temp_cache.remove_exptime();
        }
        void clear_cache()
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            temp_cache.clear();
        }
        bool remove_cache()
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            return temp_cache.remove(sqlhashid);
        }
        bool remove_cache(std::size_t cache_key_name)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            return temp_cache.remove(cache_key_name);
        }
        int check_cache(std::size_t cache_key_name)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            return temp_cache.check(cache_key_name);
        }

        bool get_data_cache(std::size_t cache_key_name)
        {
            try
            {
                model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
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
            model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            return temp_cache.update(sqlhashid, exp_time);
        }
        int update_cache(std::size_t cache_key_name, int exp_time)
        {
            model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
            return temp_cache.update(cache_key_name, exp_time);
        }
        bool save_cache(int exp_time = 0)
        {
            model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::record, exp_time);
            return true;
        }

        bool save_data_cache(int exp_time = 0)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(sqlstring);
            temp_cache.save(sqlhashid, B_BASE::data, exp_time);
            return true;
        }

        bool save_data_cache(const std::string &cache_key_name, const typename B_BASE::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }

        bool save_cache(std::size_t cache_key_name, const std::vector<typename B_BASE::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
            temp_cache.save(cache_key_name, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const std::vector<typename B_BASE::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_vector_cache(const std::string cache_key_name, const std::vector<typename B_BASE::meta> &cache_data, int exp_time = 0)
        {
            model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
            std::size_t sqlhashid                                            = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        bool save_cache(const std::string cache_key_name, const typename B_BASE::meta &cache_data, int exp_time = 0)
        {
            model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
            std::size_t sqlhashid                               = std::hash<std::string>{}(cache_key_name);
            temp_cache.save(sqlhashid, cache_data, exp_time);
            return true;
        }
        typename B_BASE::meta &get_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<typename B_BASE::meta> &temp_cache = model_meta_cache<typename B_BASE::meta>::getinstance();
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

        std::vector<typename B_BASE::meta> &get_vector_cache(const std::string &cache_key_name)
        {
            try
            {
                model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
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
                model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                http::obj_val json_temp_v;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                http::obj_val json_temp_v;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);

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
            sqlstring = B_BASE::_makeupdatesql("");
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

            if (iscommit)
            {
                iscommit = false;
                return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            sqlstring = B_BASE::_makeupdatesql(fieldname);
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

            if (iscommit)
            {
                iscommit = false;
                return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            sqlstring = B_BASE::_makeupdatesql(fieldname);
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

            if (iscommit)
            {
                iscommit = false;
                co_return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            sqlstring = B_BASE::_makeupdatesql("");
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

            if (iscommit)
            {
                iscommit = false;
                co_return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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
                sqlstring = B_BASE::_make_insert_into_sql(fieldname);
            }
            else
            {
                sqlstring = B_BASE::_make_replace_into_sql();
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            if (iscommit)
            {
                iscommit = false;
                return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            if (iscommit)
            {
                iscommit = false;
                co_return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            if (iscommit)
            {
                iscommit = false;
                return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            if (iscommit)
            {
                iscommit = false;
                co_return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            if (iscommit)
            {
                iscommit = false;
                return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

            if (iscommit)
            {
                iscommit = false;
                return 0;
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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
        std::tuple<unsigned int, unsigned long long> insert(typename B_BASE::meta &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makerecordinsertsql(insert_data);
            if (iscommit)
            {
                iscommit = false;
                return std::make_tuple(0, 0);
            }

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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(typename B_BASE::meta &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makerecordinsertsql(insert_data);
            if (iscommit)
            {
                iscommit = false;
                co_return std::make_tuple(0, 0);
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

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

        std::tuple<unsigned int, unsigned long long> insert(std::vector<typename B_BASE::meta> &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makerecordinsertsql(insert_data);
            if (iscommit)
            {
                iscommit = false;
                return std::make_tuple(0, 0);
            }

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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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

        asio::awaitable<std::tuple<unsigned int, unsigned long long>> async_insert(std::vector<typename B_BASE::meta> &insert_data)
        {
            effect_num = 0;
            sqlstring  = B_BASE::_makerecordinsertsql(insert_data);
            if (iscommit)
            {
                iscommit = false;
                co_return std::make_tuple(0, 0);
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

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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
            sqlstring  = B_BASE::_makeinsertsql();
            if (iscommit)
            {
                iscommit = false;
                return std::make_tuple(0, 0);
            }

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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {
                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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
            sqlstring  = B_BASE::_makeinsertsql();
            if (iscommit)
            {
                iscommit = false;
                co_return std::make_tuple(0, 0);
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

                long long insert_last_id = 0;
                if ((unsigned char)temp_pack_data.data[0] == 0xFF)
                {
                    error_msg = temp_pack_data.data.substr(3);
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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
                sqlstring = B_BASE::_makeupdatesql("");
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
                if (iscommit)
                {
                    iscommit = false;
                    return std::make_tuple(0, 0);
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
                }
                //return effect_num;
                return std::make_tuple(effect_num, 0);
            }
            else
            {
                sqlstring = B_BASE::_makeinsertsql();
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
                }
                else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                {

                    unsigned int d_offset = 1;
                    effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    B_BASE::setPK(insert_last_id);
                }
                if (!islock_conn)
                {
                    conn_obj->back_edit_conn(std::move(edit_conn));
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
                sqlstring = B_BASE::_makeupdatesql("");
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
                if (iscommit)
                {
                    iscommit = false;
                    co_return std::make_tuple(0, 0);
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
                    }
                    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                    {

                        unsigned int d_offset = 1;
                        effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                    }
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
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
                sqlstring = B_BASE::_makeinsertsql();
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
                    }
                    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
                    {

                        unsigned int d_offset = 1;
                        effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                        insert_last_id        = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
                        B_BASE::setPK(insert_last_id);
                    }
                    if (!islock_conn)
                    {
                        conn_obj->back_edit_conn(std::move(edit_conn));
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

        std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>
        query(const std::string &rawsql)
        {
            effect_num = 0;
            std::vector<std::vector<std::string>> temprecord;
            std::vector<std::string> table_fieldname;
            std::map<std::string, unsigned int> table_fieldmap;

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
                        error_msg = "Query sql string must be select.";
                        iserror   = true;
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
                std::size_t n = select_conn->write_sql(rawsql);
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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::vector<std::string> temp_v_record;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
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
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_select_conn(std::move(select_conn));
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

        asio::awaitable<std::tuple<std::vector<std::string>, std::map<std::string, unsigned int>, std::vector<std::vector<std::string>>>>
        async_query(const std::string &rawsql)
        {
            effect_num = 0;
            std::vector<std::vector<std::string>> temprecord;
            std::vector<std::string> table_fieldname;
            std::map<std::string, unsigned int> table_fieldmap;

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
                        error_msg = "Query sql string must be select.";
                        iserror   = true;
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
                co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
            }

            try
            {
                if (conn_empty())
                {
                    co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
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
                    co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
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
                        co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
                    }
                    for (; offset < n;)
                    {
                        select_conn->read_field_pack(select_conn->_cache_data, n, offset, temp_pack_data);
                        if (temp_pack_data.error > 0)
                        {
                            iserror   = true;
                            error_msg = temp_pack_data.data;
                            select_conn.reset();
                            co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
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
                                unsigned int column_num = field_array.size();
                                unsigned int tempnum    = 0;

                                std::vector<std::string> temp_v_record;
                                for (unsigned int ij = 0; ij < column_num; ij++)
                                {
                                    unsigned long long name_length = 0;
                                    name_length                    = select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], tempnum);
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
                    conn_mar.push_log(rawsql, std::to_string(du_time));
                }
                if (!islock_conn)
                {
                    conn_obj->back_select_conn(std::move(select_conn));
                }
                co_return std::make_tuple(std::move(table_fieldname), std::move(table_fieldmap), std::move(temprecord));
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
            co_return std::make_tuple(table_fieldname, table_fieldmap, temprecord);
        }

        // long long edit_query(const std::string &rawsql, bool isinsert = false)
        // {
        //     if (iserror)
        //     {
        //         return 0;
        //     }

        //     return 0;
        // }
        M_MODEL &clear(bool both = true)
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
            iscommit     = false;
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
            iscommit     = false;
            iscache      = false;
            iserror      = false;
            effect_num   = 0;
            return *mod;
        }
        M_MODEL &set_data(typename B_BASE::meta indata)
        {
            B_BASE::data = indata;
            return *mod;
        }
        M_MODEL &get() { return *mod; }
        std::string get_query() { return sqlstring; }

        M_MODEL &begin_commit()
        {
            if (!conn_empty())
            {
                return *mod;
            }
            islock_conn = true;
            
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
            sqlstring="start transaction";
            edit_conn->write_sql(sqlstring);
            iscommit = true;

            
            return *mod;
        }
        M_MODEL &rollback()
        {
            if(iscommit==false)
            {
                error_msg = "not begin_commit";
                iserror   = true;
                return *mod;
            }
            if (!conn_empty())
            {
                return *mod;
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
            sqlstring="rollback";
            edit_conn->write_sql(sqlstring);

            iscommit = false;
            islock_conn = false;
            return *mod;
        }
        M_MODEL &commit()
        {
            if(iscommit==false)
            {
                error_msg = "not begin_commit";
                iserror   = true;
                return *mod;
            }
            if (!conn_empty())
            {
                return *mod;
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
                edit_conn =conn_obj->get_edit_conn();
            }
            sqlstring="commit";
            edit_conn->write_sql(sqlstring);

            iscommit = false;
            islock_conn = false;
            return *mod;
        }


        asio::awaitable<bool> async_begin_commit()
        {
            if (!conn_empty())
            {
                co_return false;
            }
            islock_conn = true;

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
            sqlstring="start transaction";
            co_await edit_conn->async_write_sql(sqlstring);
            iscommit = true;

            co_return false;
        }
        asio::awaitable<bool> async_rollback()
        {
            if(iscommit==false)
            {
                error_msg = "not begin_commit";
                iserror   = true;
                co_return false;
            }
            if (!conn_empty())
            {
                co_return false;
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
            sqlstring="rollback";
            co_await edit_conn->async_write_sql(sqlstring);

            iscommit = false;
            islock_conn = false;
            co_return true;
        }

        asio::awaitable<bool> async_commit()
        {
            if(iscommit==false)
            {
                error_msg = "not begin_commit";
                iserror   = true;
                co_return false;
            }
            if (!conn_empty())
            {
                co_return false;
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
            sqlstring="commit";
            co_await edit_conn->async_write_sql(sqlstring);

            iscommit = false;
            islock_conn = false;
            co_return true;
        }

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
        std::string original_tablename;

        // std::list<std::string> commit_sqllist;
        bool iskuohao           = false;
        bool iscommit           = false;
        bool ishascontent       = false;
        bool iscache            = false;
        bool iserror            = false;
        bool islock_conn        = false;
        int exptime             = 0;
        unsigned int effect_num = 0;

        M_MODEL *mod;

        std::shared_ptr<mysql_conn_base> select_conn;
        std::shared_ptr<mysql_conn_base> edit_conn;
        std::shared_ptr<orm_conn_pool> conn_obj;
    };
} /*tagnamespace_replace*/
}// namespace orm
#endif
