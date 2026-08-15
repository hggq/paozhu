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
            std::size_t n = mysql_select_conn->write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
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
                n      = mysql_select_conn->read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_select_conn->error_msg;
                    mysql_select_conn.reset();
                    return 0;
                }
                for (; offset < n;)
                {
                    mysql_select_conn->read_field_pack(mysql_select_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_select_conn.reset();
                        return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_select_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            column_num           = field_array.size();
                            unsigned int tempnum = 0;

                            T data_temp;
                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    return 0;
                                }
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = pg_select_conn->execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            for (auto &row : rows)
            {
                T data_temp;
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (!row.is_null[ij] && fields[ij].name.size() > 0)
                    {
                        data_temp.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                    }
                }
                result_record.emplace_back(std::move(data_temp));
                effect_num++;
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
    unsigned int query(const std::string &rawsql, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_query_vec_impl(rawsql, result_record);
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
            std::size_t n = co_await mysql_select_conn->async_write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
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
                n      = co_await mysql_select_conn->async_read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_select_conn->error_msg;
                    mysql_select_conn.reset();
                    co_return 0;
                }
                for (; offset < n;)
                {
                    mysql_select_conn->read_field_pack(mysql_select_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_select_conn.reset();
                        co_return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_select_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            column_num           = field_array.size();
                            unsigned int tempnum = 0;

                            T data_temp;
                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    co_return 0;
                                }
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = co_await pg_select_conn->async_execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            for (auto &row : rows)
            {
                T data_temp;
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (!row.is_null[ij] && fields[ij].name.size() > 0)
                    {
                        data_temp.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                    }
                }
                result_record.emplace_back(std::move(data_temp));
                effect_num++;
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
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_query_vec_impl(rawsql, result_record);
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
            std::size_t n = mysql_select_conn->write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
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
                n      = mysql_select_conn->read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_select_conn->error_msg;
                    mysql_select_conn.reset();
                    return 0;
                }
                for (; offset < n;)
                {
                    mysql_select_conn->read_field_pack(mysql_select_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_select_conn.reset();
                        return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_select_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            column_num           = field_array.size();
                            unsigned int tempnum = 0;

                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    return 0;
                                }
                                if (field_array[ij].name.size() > 0)
                                {
                                    result_record.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                }
                                else if (field_array[ij].org_name.size() > 0)
                                {
                                    result_record.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = pg_select_conn->execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (!rows.empty())
            {
                auto &row = rows.front();
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (!row.is_null[ij] && fields[ij].name.size() > 0)
                    {
                        result_record.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                    }
                }
                effect_num++;
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
    unsigned int query(const std::string &rawsql, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_query_single_impl(rawsql, result_record);
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
            std::size_t n = co_await mysql_select_conn->async_write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
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
                n      = co_await mysql_select_conn->async_read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_select_conn->error_msg;
                    mysql_select_conn.reset();
                    co_return 0;
                }
                for (; offset < n;)
                {
                    mysql_select_conn->read_field_pack(mysql_select_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_select_conn.reset();
                        co_return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_select_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            column_num           = field_array.size();
                            unsigned int tempnum = 0;

                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    co_return 0;
                                }
                                if (field_array[ij].name.size() > 0)
                                {
                                    result_record.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                }
                                else if (field_array[ij].org_name.size() > 0)
                                {
                                    result_record.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = co_await pg_select_conn->async_execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (!rows.empty())
            {
                auto &row = rows.front();
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (!row.is_null[ij] && fields[ij].name.size() > 0)
                    {
                        result_record.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                    }
                }
                effect_num++;
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
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_query_single_impl(rawsql, result_record);
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
            std::size_t n = mysql_select_conn->write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
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
                n      = mysql_select_conn->read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_select_conn->error_msg;
                    mysql_select_conn.reset();
                    return 0;
                }
                for (; offset < n;)
                {
                    mysql_select_conn->read_field_pack(mysql_select_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_select_conn.reset();
                        return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_select_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            unsigned int tempnum = 0;

                            T data_temp;
                            for (unsigned int ij = 0; ij < field_array.size(); ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    return 0;
                                }
                                if (field_array[ij].name.size() > 0)
                                {
                                    std::invoke(std::forward<Callback>(callback), data_temp, field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, ij % 255, 1);
                                }
                                else if (field_array[ij].org_name.size() > 0)
                                {
                                    std::invoke(std::forward<Callback>(callback), data_temp, field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, ij % 255, 1);
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = pg_select_conn->execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
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
                        std::invoke(std::forward<Callback>(callback), data_temp, fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), ij % 255, 1);
                    }
                    else if (fields[ij].org_name.size() > 0)
                    {
                        std::invoke(std::forward<Callback>(callback), data_temp, fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), ij % 255, 1);
                    }
                }
                result_record.emplace_back(std::move(data_temp));
                effect_num++;
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
    unsigned int query(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_query_vec_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
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
            std::size_t n = co_await mysql_select_conn->async_write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
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
                n      = co_await mysql_select_conn->async_read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_select_conn->error_msg;
                    mysql_select_conn.reset();
                    co_return 0;
                }
                for (; offset < n;)
                {
                    mysql_select_conn->read_field_pack(mysql_select_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_select_conn.reset();
                        co_return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_select_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            unsigned int tempnum = 0;

                            T data_temp;
                            for (unsigned int ij = 0; ij < field_array.size(); ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    co_return 0;
                                }
                                if (field_array[ij].name.size() > 0)
                                {
                                    std::invoke(std::forward<Callback>(callback), data_temp, field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, ij % 255, 1);
                                }
                                else if (field_array[ij].org_name.size() > 0)
                                {
                                    std::invoke(std::forward<Callback>(callback), data_temp, field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, ij % 255, 1);
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = co_await pg_select_conn->async_execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
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
                        std::invoke(std::forward<Callback>(callback), data_temp, fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), ij % 255, 1);
                    }
                    else if (fields[ij].org_name.size() > 0)
                    {
                        std::invoke(std::forward<Callback>(callback), data_temp, fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), ij % 255, 1);
                    }
                }
                result_record.emplace_back(std::move(data_temp));
                effect_num++;
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
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, std::vector<T> &result_record, Callback &&callback)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_query_vec_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
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
            std::size_t n = mysql_select_conn->write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
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
                n      = mysql_select_conn->read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_select_conn->error_msg;
                    mysql_select_conn.reset();
                    return 0;
                }
                for (; offset < n;)
                {
                    mysql_select_conn->read_field_pack(mysql_select_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_select_conn.reset();
                        return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_select_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            unsigned int tempnum = 0;

                            for (unsigned int ij = 0; ij < field_array.size(); ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    return 0;
                                }
                                if (field_array[ij].name.size() > 0)
                                {
                                    std::invoke(std::forward<Callback>(callback), result_record, field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, ij % 255, 1);
                                }
                                else if (field_array[ij].org_name.size() > 0)
                                {
                                    std::invoke(std::forward<Callback>(callback), result_record, field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, ij % 255, 1);
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = pg_select_conn->execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                return 0;
            }

            if (!rows.empty())
            {
                auto &row = rows.front();
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (row.is_null[ij])
                    {
                        continue;
                    }
                    if (fields[ij].name.size() > 0)
                    {
                        std::invoke(std::forward<Callback>(callback), result_record, fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), ij % 255, 1);
                    }
                    else if (fields[ij].org_name.size() > 0)
                    {
                        std::invoke(std::forward<Callback>(callback), result_record, fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), ij % 255, 1);
                    }
                }
                effect_num++;
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
    unsigned int query(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
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
            std::size_t n = co_await mysql_select_conn->async_write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_select_conn->error_msg;
                mysql_select_conn.reset();
                co_return 0;
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
                n      = co_await mysql_select_conn->async_read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_select_conn->error_msg;
                    mysql_select_conn.reset();
                    co_return 0;
                }
                for (; offset < n;)
                {
                    mysql_select_conn->read_field_pack(mysql_select_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_select_conn.reset();
                        co_return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_select_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_select_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            unsigned int tempnum = 0;

                            for (unsigned int ij = 0; ij < field_array.size(); ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_select_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    co_return 0;
                                }
                                if (field_array[ij].name.size() > 0)
                                {
                                    std::invoke(std::forward<Callback>(callback), result_record, field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, ij % 255, 1);
                                }
                                else if (field_array[ij].org_name.size() > 0)
                                {
                                    std::invoke(std::forward<Callback>(callback), result_record, field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, ij % 255, 1);
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = co_await pg_select_conn->async_execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_select_conn->error_msg;
                pg_select_conn.reset();
                co_return 0;
            }

            if (!rows.empty())
            {
                auto &row = rows.front();
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (row.is_null[ij])
                    {
                        continue;
                    }
                    if (fields[ij].name.size() > 0)
                    {
                        std::invoke(std::forward<Callback>(callback), result_record, fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), ij % 255, 1);
                    }
                    else if (fields[ij].org_name.size() > 0)
                    {
                        std::invoke(std::forward<Callback>(callback), result_record, fields[ij].org_name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), ij % 255, 1);
                    }
                }
                effect_num++;
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
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, T &result_record, Callback &&callback)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
        }
        co_return co_await pg_async_query_single_cb_impl(rawsql, result_record, std::forward<Callback>(callback));
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

            std::size_t n = mysql_edit_conn->write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
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
                n      = mysql_edit_conn->read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_edit_conn->error_msg;
                    mysql_edit_conn.reset();
                    return 0;
                }
                for (; offset < n;)
                {
                    mysql_edit_conn->read_field_pack(mysql_edit_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_edit_conn.reset();
                        return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_edit_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_edit_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            column_num           = field_array.size();
                            unsigned int tempnum = 0;

                            T data_temp;
                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    return 0;
                                }
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = pg_edit_conn->execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                return 0;
            }

            for (auto &row : rows)
            {
                T data_temp;
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (!row.is_null[ij] && fields[ij].name.size() > 0)
                    {
                        data_temp.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                    }
                }
                result_record.emplace_back(std::move(data_temp));
                effect_num++;
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
    unsigned int edit_query(const std::string &rawsql, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_edit_query_vec_impl(rawsql, result_record);
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
            std::size_t n = co_await mysql_edit_conn->async_write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                co_return 0;
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
                n      = co_await mysql_edit_conn->async_read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_edit_conn->error_msg;
                    mysql_edit_conn.reset();
                    co_return 0;
                }
                for (; offset < n;)
                {
                    mysql_edit_conn->read_field_pack(mysql_edit_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_edit_conn.reset();
                        co_return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_edit_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_edit_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            column_num           = field_array.size();
                            unsigned int tempnum = 0;

                            T data_temp;
                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    co_return 0;
                                }
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = co_await pg_edit_conn->async_execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                co_return 0;
            }

            for (auto &row : rows)
            {
                T data_temp;
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (!row.is_null[ij] && fields[ij].name.size() > 0)
                    {
                        data_temp.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                    }
                }
                result_record.emplace_back(std::move(data_temp));
                effect_num++;
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
    asio::awaitable<unsigned int> async_edit_query(const std::string &rawsql, std::vector<T> &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_edit_query_vec_impl(rawsql, result_record);
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

            std::size_t n = mysql_edit_conn->write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
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
                n      = mysql_edit_conn->read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_edit_conn->error_msg;
                    mysql_edit_conn.reset();
                    return 0;
                }
                for (; offset < n;)
                {
                    mysql_edit_conn->read_field_pack(mysql_edit_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_edit_conn.reset();
                        return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_edit_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_edit_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            column_num           = field_array.size();
                            unsigned int tempnum = 0;

                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    return 0;
                                }
                                if (field_array[ij].name.size() > 0)
                                {
                                    result_record.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                }
                                else if (field_array[ij].org_name.size() > 0)
                                {
                                    result_record.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = pg_edit_conn->execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                return 0;
            }

            if (!rows.empty())
            {
                auto &row = rows.front();
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (!row.is_null[ij] && fields[ij].name.size() > 0)
                    {
                        result_record.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                    }
                }
                effect_num++;
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
    unsigned int edit_query(const std::string &rawsql, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            return mysql_edit_query_single_impl(rawsql, result_record);
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

            std::size_t n = co_await mysql_edit_conn->async_write_sql(rawsql);
            if (n == 0)
            {
                error_msg = mysql_edit_conn->error_msg;
                mysql_edit_conn.reset();
                co_return 0;
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
                n      = co_await mysql_edit_conn->async_read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = mysql_edit_conn->error_msg;
                    mysql_edit_conn.reset();
                    co_return 0;
                }
                for (; offset < n;)
                {
                    mysql_edit_conn->read_field_pack(mysql_edit_conn->_cache_data, n, offset, temp_pack_data);
                    if (temp_pack_data.error > 0)
                    {
                        iserror   = true;
                        error_msg = temp_pack_data.data;
                        mysql_edit_conn.reset();
                        co_return 0;
                    }
                    if (temp_pack_data.length == temp_pack_data.current_length && temp_pack_data.current_length > 0)
                    {
                        if (mysql_edit_conn->pack_eof_check(temp_pack_data))
                        {
                            is_sql_item = true;
                            break;
                        }

                        if (action_setup == 0)
                        {
                            if (temp_pack_data.data.empty())
                                break;
                            if (temp_pack_data.length == 2 && (unsigned char)temp_pack_data.data[0] < 251 && (unsigned char)temp_pack_data.data[0] > 0)
                            {
                                action_setup = 1;
                                column_num   = (unsigned char)temp_pack_data.data[0];
                            }
                        }
                        else if (action_setup == 1)
                        {
                            field_info_t temp_filed_col;
                            mysql_edit_conn->read_col_info(temp_pack_data.data, temp_filed_col);

                            field_array.emplace_back(std::move(temp_filed_col));
                            column_num--;
                            if (column_num == 0)
                            {
                                action_setup = 2;
                            }
                        }
                        else if (action_setup == 2)
                        {
                            column_num           = field_array.size();
                            unsigned int tempnum = 0;

                            for (unsigned int ij = 0; ij < column_num; ij++)
                            {
                                unsigned long long name_length = 0;
                                name_length                    = mysql_edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], temp_pack_data.data.size(), tempnum);
                                if (name_length > 16 * 1024 * 1024 || (tempnum + name_length) > temp_pack_data.data.size())
                                {
                                    error_msg = "MySQL read pack error";
                                    co_return 0;
                                }
                                if (field_array[ij].name.size() > 0)
                                {
                                    result_record.set_val(field_array[ij].name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
                                }
                                else if (field_array[ij].org_name.size() > 0)
                                {
                                    result_record.set_val(field_array[ij].org_name, (unsigned char *)&temp_pack_data.data[tempnum], name_length, field_array[ij].field_type);
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

            std::vector<field_info_t> fields;
            std::vector<pg_row_data_t> rows;
            unsigned int affected = 0;
            unsigned int err      = co_await pg_edit_conn->async_execute_and_fetch(rawsql, fields, rows, affected);
            if (err > 0)
            {
                error_msg = pg_edit_conn->error_msg;
                pg_edit_conn.reset();
                co_return 0;
            }

            if (!rows.empty())
            {
                auto &row = rows.front();
                for (unsigned int ij = 0; ij < fields.size() && ij < row.values.size(); ij++)
                {
                    if (!row.is_null[ij] && fields[ij].name.size() > 0)
                    {
                        result_record.set_val(fields[ij].name, reinterpret_cast<const unsigned char *>(row.values[ij].data()), row.values[ij].size(), fields[ij].field_type);
                    }
                }
                effect_num++;
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
    asio::awaitable<unsigned int> async_edit_query(const std::string &rawsql, T &result_record)
    {
        if (db_type == DB_TYPE::MYSQL)
        {
            co_return co_await mysql_async_edit_query_single_impl(rawsql, result_record);
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
    //// transaction impl (mysql / pg)
    bool mysql_begin_commit_impl();
    bool pg_begin_commit_impl();
    bool mysql_commit_impl();
    bool pg_commit_impl();
    void mysql_rollback_impl();
    void pg_rollback_impl();

    asio::awaitable<bool> mysql_async_begin_commit_impl();
    asio::awaitable<bool> pg_async_begin_commit_impl();
    asio::awaitable<bool> mysql_async_commit_impl();
    asio::awaitable<bool> pg_async_commit_impl();
    asio::awaitable<void> mysql_async_rollback_impl();
    asio::awaitable<void> pg_async_rollback_impl();

    //// raw edit sql impl (mysql / pg)
    unsigned int mysql_edit_query_impl(const std::string &rawsql);
    unsigned int pg_edit_query_impl(const std::string &rawsql);
    asio::awaitable<unsigned int> mysql_async_edit_query_impl(const std::string &rawsql);
    asio::awaitable<unsigned int> pg_async_edit_query_impl(const std::string &rawsql);

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
    }

  public:
    unsigned int edit_query(const std::string &);
    asio::awaitable<unsigned int> async_edit_query(const std::string &);

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
    std::shared_ptr<orm_conn_pool> conn_obj;
    DB_TYPE db_type = DB_TYPE::MYSQL;
};

}// namespace orm
#endif
