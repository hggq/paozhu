#ifndef _MYSQL_QUERY_ALONE_BASE_H
#define _MYSQL_QUERY_ALONE_BASE_H
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
#include "mysql_conn_pool.h"
#include "mysql_connect_mar.h"

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

    ~db_conn() {
        if(islock_conn)
        {
            islock_conn = false;
            if (select_conn)
            {
                conn_obj->back_select_conn(std::move(select_conn));
            }
            if (edit_conn)
            {
                conn_obj->back_edit_conn(std::move(edit_conn));
            }
        }
    };
    template <ResultHasSetVal T>
    unsigned int query(const std::string &rawsql, std::vector<T> &result_record)
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
                            unsigned int column_num = field_array.size();
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
    asio::awaitable<unsigned int> async_query(const std::string &rawsql, std::vector<T> result_record)
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
            unsigned int offset        = 0;
            for (; is_sql_item == false;)
            {
                n      = co_await select_conn->async_read_loop();
                offset = 0;
                if (n == 0)
                {
                    error_msg = select_conn->error_msg;
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
                            unsigned int column_num = field_array.size();
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
    void begin_commit();
    void commit();
    void rollback();

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
    std::shared_ptr<mysql_conn_base> select_conn;
    std::shared_ptr<mysql_conn_base> edit_conn;
    std::shared_ptr<orm_conn_pool> conn_obj;
};

}// namespace orm
#endif