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
#include "mysql_conn_pool.h"
#include "mysql_connect_mar.h"
#include "mysql_query.h"
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
    }
    else
    {
        conn_obj  = nullptr;
        iserror   = true;
        error_msg = "conn_pool not found " + dbtag;
    }
}

void db_conn::begin_commit()
{
    if (iscommit)
    {
        error_msg = "not begin_commit";
        iserror   = true;
        return;
    }
    iscommit = true;
    if (conn_obj == nullptr)
    {
        error_msg = "Please select_db() tag";
        return;
    }

    edit_conn = conn_obj->get_edit_conn();

    std::string sqlstring = "start transaction";
    edit_conn->write_sql(sqlstring);
}

void db_conn::commit()
{
    if (iscommit)
    {
        error_msg = "not commit";
        iserror   = true;
        return;
    }
    if (conn_obj == nullptr)
    {
        error_msg = "Please select_db() tag";
        return;
    }

    edit_conn = conn_obj->get_edit_conn();

    std::string sqlstring = "commit";
    edit_conn->write_sql(sqlstring);

    unsigned int offset = 0;
    auto n              = edit_conn->read_loop();
    if (n == 0)
    {
        error_msg = edit_conn->error_msg;
        edit_conn.reset();
        return;
    }
    pack_info_t temp_pack_data;
    temp_pack_data.seq_id = 1;
    edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);

    if ((unsigned char)temp_pack_data.data[0] == 0xFF)
    {
        error_msg = temp_pack_data.data.substr(3);
        rollback();
    }
    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
    {

        unsigned int d_offset = 1;
        effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
        conn_obj->back_edit_conn(std::move(edit_conn));
        edit_conn = nullptr;
    }
    iscommit = false;
}

void db_conn::rollback()
{
    if (iscommit)
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

    edit_conn = conn_obj->get_edit_conn();

    std::string sqlstring = "rollback";
    edit_conn->write_sql(sqlstring);
    unsigned int offset = 0;
    auto n              = edit_conn->read_loop();
    if (n == 0)
    {
        error_msg = edit_conn->error_msg;
        edit_conn.reset();
        return;
    }
    pack_info_t temp_pack_data;
    temp_pack_data.seq_id = 1;
    edit_conn->read_field_pack(edit_conn->_cache_data, n, offset, temp_pack_data);

    if ((unsigned char)temp_pack_data.data[0] == 0xFF)
    {
        error_msg = temp_pack_data.data.substr(3);
    }
    else if ((unsigned char)temp_pack_data.data[0] == 0x00)
    {
        unsigned int d_offset = 1;
        effect_num            = edit_conn->pack_real_num((unsigned char *)&temp_pack_data.data[0], d_offset);
    }
    iscommit = false;
    edit_conn.reset();
}

unsigned int db_conn::edit_query(const std::string &rawsql)
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

asio::awaitable<unsigned int> db_conn::async_edit_query(const std::string &rawsql)
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

}// namespace orm
